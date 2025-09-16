/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   worst-case.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 29, 2009
 *
 *  Function        :   Transform FSM-based SADF to worst-case FSM-based SADF
 *
 *  History         :
 *      29-06-09    :   Initial version.
 *
 * $Id: worst-case.cc,v 1.1.2.2 2010-04-22 07:07:19 mgeilen Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#include "worst-case.h"
#include "fsmsadf/analysis/analysis.h" // IWYU pragma: keep#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * transformToWorstCaseGraph ()
 * The function converts the graph to a graph with one scenario which
 * represents the worst-case behavior over all scenarios in the original
 * graph.
 */
std::unique_ptr<Graph> transformToWorstCaseGraph(const Graph& g) {
    std::map<Scenario *, RepetitionVector> repetitionVectors;
    std::map<CString, int> repetitionVectorWC;
    std::map<CString, uint> channelNamesToInitialTokens;
    std::map<CString, ChannelRefs> channelNamesToChannels;
    std::map<const Channel *, ScenarioRefs> channelToScenarios;
    std::map<CString, ActorRefs> actorNamesToActors;
    std::map<const Actor *, ScenarioRefs> actorToScenarios;

    // Mapping of actor names to actors and channel names to channels
    for (const auto& s : g.getConstScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Actors
        for (const auto& a : sg->getActors()) {
            actorNamesToActors[a->getName()].push_back(a.get());
            actorToScenarios[a.get()].insert(s.get());
        }

        // Channels
        for (const auto& c : sg->getChannels()) {
            channelNamesToChannels[c->getName()].push_back(c.get());
            channelToScenarios[c.get()].insert(s.get());
        }
    }

    // Initial tokens in the channels
    for (auto &[channelName, channelList] : channelNamesToChannels) {
        // Iterate over the channels
        for (auto j = channelList.begin(); j != channelList.end(); j++) {
            Channel *c = *j;

            // First channel with this name?
            if (j == channelList.begin()) {
                channelNamesToInitialTokens[channelName] = c->getInitialTokens();
            } else {
                // Same number of initial tokens on all channels with same name?
                if (channelNamesToInitialTokens[channelName] != c->getInitialTokens()) {
                    throw CException("Channel must have equal number of initial"
                                     " tokens in all scenario graphs.");
                }

                // Same source and destination actor for all channels with the
                // same name?
                if (channelList.front()->getSrcActor()->getName() != c->getSrcActor()->getName()
                    || channelList.front()->getDstActor()->getName()
                               != c->getDstActor()->getName()) {
                    std::cerr << channelList.front()->getName() << std::endl;
                    std::cerr << c->getName() << std::endl;
                    std::cerr << channelList.front()->getSrcActor()->getName() << std::endl;
                    std::cerr << c->getSrcActor()->getName() << std::endl;
                    std::cerr << channelList.front()->getDstActor()->getName() << std::endl;
                    std::cerr << c->getDstActor()->getName() << std::endl;
                    throw CException("Channel must have same source and "
                                     "destination actor in all scenario graphs.");
                }
            }

            // Channel which contains initial tokens must have equal rate in all
            // scenarios.
            if (c->getInitialTokens() != 0) {
                // Iterate over all scenarios
                for (const auto& s : channelToScenarios[c]) {
                    Channel* cFirst = *(channelList.begin());
                    Scenario* sFirst = *(channelToScenarios[cFirst].begin());

                    // Are rate on channel c in scenario s constant?
                    if (cFirst->getSrcPort()->getRateOfScenario(*sFirst)
                                != c->getSrcPort()->getRateOfScenario(*s)
                        || cFirst->getDstPort()->getRateOfScenario(*sFirst)
                                   != c->getDstPort()->getRateOfScenario(*s)) {
                        throw CException("Source and destination port of a "
                                         "channel with initial tokens must have"
                                         " constant rate.");
                    }
                }
            }
        }
    }

    // Compute repetition vectors for all scenarios
    for (const auto& s : g.getConstScenarios()) {
        repetitionVectors[s.get()] = s->computeRepetitionVector();
    }

    // Worst-case repetition vector
    for (auto &[actorName, actorList] : actorNamesToActors) {
        int maxRepVecActor = 0;

        // Iterate over all actors with this name
        for (const auto& a : actorList) {
            // Iterate over all scenarios in which this actor appears
            for (const auto& s : actorToScenarios[a]) {
                // Repetition vector of actor a in scenario s larger than any
                // entry seen before?
                maxRepVecActor = (std::max)(maxRepVecActor, repetitionVectors[s][a->getId()]);
            }
        }

        // Set worst-case repetition vector entry of actor with this name
        repetitionVectorWC[actorName] = maxRepVecActor;
    }

    // Create a new graph
    auto gWC = std::make_unique<Graph>();

    // Create a new scenario graph in the graph
    auto sgWC = std::make_unique<ScenarioGraph>(GraphComponent(gWC.get(), 0, "wc"));
    auto* sgWCPtr = sgWC.get();
    gWC->addScenarioGraph(std::move(sgWC));

    // Create a new worst-case scenario
    auto sWC = std::make_unique<Scenario>(GraphComponent(gWC.get(), 0, "wc"));
    sWC->setScenarioGraph(sgWCPtr);
    auto * sWCPtr = sWC.get();
    gWC->addScenario(std::move(sWC));

    // Create FSM
    auto fsm = std::make_unique<FSM>(GraphComponent(gWC.get(), 0, "fsm"));
    auto* fsmPtr = fsm.get();
    gWC->setFSM(std::move(fsm));

    // Create only state of the FSM
    auto fsmState = std::make_unique<FSMstate>(GraphComponent(fsmPtr, 0, "s"));
    fsmState->setScenario(sWCPtr);

    // Add state to the FSM
    auto* fsmStatePtr = fsmState.get();
    fsmPtr->addState(std::move(fsmState));
    fsmPtr->setInitialState(fsmStatePtr);

    // Create transition from state to itself
    auto fsmTransition = std::make_unique<FSMtransition>(GraphComponent(fsmStatePtr, 0, "t"));
    fsmTransition->connect(fsmStatePtr, fsmStatePtr);
    fsmPtr->addTransition(std::move(fsmTransition));

    // Add actors to the scenario graph
    for (auto &[actorName, actorList] : actorNamesToActors) {
        ActorRef aWC = sgWCPtr->createActor();
        aWC->setName(actorName);
        aWC->setType("wc");
    }

    // Add channels to the scenario graph
    for (auto &[channelName, channelList] : channelNamesToChannels) {
        Channel* c = channelList.front();

        // Locate source and destination actor in the worst-case graph
        ActorRef srcActor = sgWCPtr->getActor(c->getSrcActor()->getName());
        ActorRef dstActor = sgWCPtr->getActor(c->getDstActor()->getName());

        // Create a new channel in the worst-case graph
        ChannelRef cWC = sgWCPtr->createChannel(srcActor, dstActor);
        cWC->setName(c->getName());

        if (channelNamesToInitialTokens[c->getName()] == 0) {
            // Set rate of src/dst port based on worst-case repetition vector
            cWC->getSrcPort()->setRateOfScenario(*sWCPtr, repetitionVectorWC[dstActor->getName()]);
            cWC->getDstPort()->setRateOfScenario(*sWCPtr, repetitionVectorWC[srcActor->getName()]);
        } else {
            // Set rate of src/dst port based on rate of random channel with this
            // name in the original graph
            cWC->getSrcPort()->setRateOfScenario(
                    *sWCPtr, c->getSrcPort()->getRateOfScenario(*(*channelToScenarios[c].begin())));
            cWC->getDstPort()->setRateOfScenario(
                    *sWCPtr, c->getDstPort()->getRateOfScenario(*(*channelToScenarios[c].begin())));

            // Add initial tokens to the channel
            cWC->setInitialAndFinalTokens(channelNamesToInitialTokens[c->getName()]);
        }
    }

    // Channel properties
    for (auto &[channelName, channelList] : channelNamesToChannels) {
        uint maxTokenSize = 0;

        // Iterate over all channels with this name
        for (const auto& c : channelList) {
            // Iterate over all scenarios in which this channel appears
            for (const auto& s : channelToScenarios[c]) {
                // Tokens size of this channel larger in scenario s?
                maxTokenSize = (std::max)(maxTokenSize, c->getTokenSizeOfScenario(*s));
            }
        }

        // Set worst-case token size of channel with this name
        sgWCPtr->getChannel(channelName)->setTokenSizeOfScenario(*sWCPtr, maxTokenSize);
    }

    // Actor properties
    for (auto &[actorName, actorList] : actorNamesToActors) {
        std::map<CString, Time> procTypeToExecTime;
        std::map<CString, Size> procTypeToStateSize;

        // Get actor in worst-case graph
        ActorRef aWC = sgWCPtr->getActor(actorName);

        // Iterate over all actors with this name
        for (const auto& a : actorList) {
            // Iterate over all processor types supported by actor a
            for (auto pt : a->getProcessorTypes()) {
                // Execution time
                for (std::map<const Scenario *, Time>::const_iterator l = pt.executionTime.begin();
                     l != pt.executionTime.end();
                     l++) {
                    procTypeToExecTime[pt.type] = (std::max)(procTypeToExecTime[pt.type], l->second);
                }
                /// @Todo fix this to new multiple memories instead of single state size.
                // State size
                /*                for (std::map<Scenario*,Size>::const_iterator
                                        l = k->stateSize.begin();
                                        l != k->stateSize.end(); l++)
                                {
                                    if (procTypeToStateSize[k->type] < l->second)
                                        procTypeToStateSize[k->type] = l->second;
                                }*/
            }
        }

        // Set worst-case execution time
        for (auto j = procTypeToExecTime.begin(); j != procTypeToExecTime.end(); j++) {
            aWC->setExecutionTimeOfScenario(*sWCPtr, j->first, j->second);

            // Set first processor type as the default type
            if (j == procTypeToExecTime.begin()) {
                aWC->setDefaultProcessorType(j->first);
            }
        }

        // Set worst-case state size
        /// @Todo fix this to new multiple memories instead of single state size.
        /*
        for (std::map<CString, Size>::iterator j = procTypeToStateSize.begin();
                j != procTypeToStateSize.end(); j++)
        {
            aWC->setStateSizeOfScenario(sWCPtr, j->first, j->second);
        }
         */
    }

    // Add dummy actor with execution time zero to the graph
    ActorRef dummyActorWC = sgWCPtr->createActor();
    dummyActorWC->setType("wc");
    dummyActorWC->setExecutionTimeOfScenario(*sWCPtr, "wc", 0);
    /// @todo fix this.
    // dummyActorWC->setStateSizeOfScenario(sWCPtr, "wc", 0);
    dummyActorWC->setDefaultProcessorType("wc");

    // Connect the dummy actor to a random actor in the graph
    ChannelRef dummyChannelWC = sgWCPtr->createChannel(dummyActorWC, (sgWCPtr->getActors().front().get()));

    // Set token size to zero
    dummyChannelWC->setTokenSizeOfScenario(*sWCPtr, 0);

    // Set rates to ensure that dummy actor has entry 1 in repetition vector
    dummyChannelWC->getDstPort()->setRateOfScenario(*sWCPtr, 1);
    dummyChannelWC->getSrcPort()->setRateOfScenario(
            *sWCPtr, repetitionVectorWC[sgWCPtr->getActors().front()->getName()]);

    // Copy throughput constraint
    gWC->setThroughputConstraint(g.getThroughputConstraint());

    return gWC;
}

} // namespace FSMSADF
