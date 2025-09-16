/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dsm_encode.cc
 *
 *  Author          :   Morteza Damavandpeyma (m.damavandpeyma@tue.nl)
 *
 *  Date            :   March 23, 2011
 *
 *  Function        :   Decision state modelling for
 *                       static periodic schedule encoding in (SDF) graph
 *
 *  History         :
 *      23-03-11    :   Initial version.
 *      02-11-15    :   Adaptation for use in fsm-sadf (Reinier)
 *
 * $Id: schedule_encode.cc,v 1.0.0.0 2011-03-23 10:59:47 morteza Exp $
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

#include "dsm_encode.h"
#include <memory>

namespace FSMSADF {

namespace {

ChannelRef existChannel(Scenario *s,
                      ScenarioGraph *h,
                      const CString &src,
                      Rate srcRate,
                      const CString &dst,
                      Rate dstRate,
                      uint initToken) {

    for (const auto& c : h->getChannels()) {
        if (c->getSrcActor()->getName() == src && c->getDstActor()->getName() == dst
            && c->getSrcPort()->getRateOfScenario(*s) == srcRate
            && c->getDstPort()->getRateOfScenario(*s) == dstRate
            && c->getInitialTokens() == initToken) {
            return c.get();
        }
    }

    return nullptr;
}

} // namespace

/**
 * limitInterIterationExecution ()
 * The function adds the required elements to the original scenario graph to limit
 * inter-iteration execution in the scenario graph.
 */
std::unique_ptr<ScenarioGraph> limitInterIterationExecution(Scenario *s,
                                            const CString &procName,
                                            const CString &schedName,
                                            ScenarioGraph *sg,
                                            StaticOrderSchedule sos) {
    std::unique_ptr<ScenarioGraph> newSg = nullptr;
    uint actorCount = 0;
    // Construct a new scenario graph
    newSg = sg->clone(*sg);

    if (!s->isScenarioGraphConsistent()) {
        throw CException("Error encoding schedule: graph " + newSg->getName()
                         + " is inconsistent.");
    }

    // Calculate repetition vector for the graph
    RepetitionVector repetitionVector = s->computeRepetitionVector();

    // Limit inter-iteration execution
    // Retrieve last actor in schedule
    ActorRef lastActor = newSg->getActor(sos.back().actor->getName());
    uint rvLastActor = repetitionVector[lastActor->getId()];

    // Create a scheduling actor
    auto schedulingActor =
            std::make_unique<Actor>(GraphComponent(newSg.get(), static_cast<CId>(newSg->getActors().size())));
    schedulingActor->setExecutionTimeOfScenario(*s, schedName, 0);
    schedulingActor->setDefaultProcessorType(schedName);
    schedulingActor->setName(schedName + "-start_" + s->getName() + "_" + procName + "_act"
                             + CString(actorCount));
    schedulingActor->setType(schedName);
    auto* schedulingActorPtr = schedulingActor.get();
    newSg->addActor(std::move(schedulingActor));

    // Create channel from the last actor to the scheduling actor
    ChannelRef tempChannel = newSg->createChannel(lastActor, schedulingActorPtr);
    setChannelProperties(s, tempChannel, 1, rvLastActor, 0, procName, schedName, newSg.get());

    // Retrieve last actor in schedule
    ActorRef firstActor = newSg->getActor(sos.front().actor->getName());
    uint rvFirstActor = repetitionVector[firstActor->getId()];

    // Create channel from the scheduling actor to the first actor
    tempChannel = newSg->createChannel(schedulingActorPtr, firstActor);
    setChannelProperties(s, tempChannel, rvFirstActor, 1, rvFirstActor, procName, schedName, newSg.get());

    // Set repetition vector of new actor
    schedulingActorPtr->setRepetitionsOfScenario(*s, 1);

    return newSg;
}

/**
 * addScheduleEncodingComponents ()
 * The function adds the required elements to the original scenario graph to impose
 * the desired schedule as the execution order of actors in the scenario graph.
 * The function receive an scenario graph, an static schedule and decision points
 * in the graph as inputs.
 */
std::unique_ptr<ScenarioGraph> addScheduleEncodingComponents(Scenario *s,
                                             const CString &procName,
                                             const CString &schedName,
                                             ScenarioGraph *sg,
                                             const StaticOrderSchedule &sos,
                                             std::vector<std::vector<CString>> dps) {
    std::unique_ptr<ScenarioGraph> newSg = nullptr;
    uint schedulePointer = 0;
    uint actorCount = 0;
    // Construct a new scenario graph
    GraphComponent component = GraphComponent(nullptr, 0);
    newSg = sg->clone(*sg);

    if (!s->isScenarioGraphConsistent()) {
        throw CException("Error encoding schedule: graph " + newSg->getName()
                         + " is inconsistent.");
    }

    // Calculate repetition vector for the graph
    RepetitionVector repetitionVector = s->computeRepetitionVector();

    for (std::vector<std::vector<CString>>::const_iterator iter = dps.begin(); iter != dps.end();
         iter++) {
        // Decision point ?
        if ((*iter).size() > 1) {
            // Create and add scheduling actor for each decision point
            component = GraphComponent(newSg.get(), static_cast<CId>(newSg->getActors().size()));
            auto schedulingActor = std::make_unique<Actor>(component);
            schedulingActor->setExecutionTimeOfScenario(*s, schedName, 0);
            schedulingActor->setDefaultProcessorType(schedName);
            schedulingActor->setName(schedName + "_" + s->getName() + "_" + procName + "_act"
                                     + CString(actorCount));
            schedulingActor->setType(schedName);
            auto* schedulingActorPtr = schedulingActor.get();
            newSg->addActor(std::move(schedulingActor));

            // Find first actor
            CString firstActorName = (*iter)[0];
            ActorRef firstActor = newSg->getActor(firstActorName);
            uint firstRv = repetitionVector[firstActor->getId()];
            uint firstRepeatAfter = actorAppearanceAfter(sos, firstActorName, schedulePointer);

            // Create channel from first to scheduling actor
            ChannelRef tempChannel = newSg->createChannel(firstActor, schedulingActorPtr);
            setChannelProperties(
                    s, tempChannel, 1, firstRv, firstRepeatAfter, procName, schedName, newSg.get());

            // Find other actors
            for (auto iter2 = (*iter).begin() + 1; iter2 != (*iter).end(); iter2++) {
                const CString &currentActorName = (*iter2);
                ActorRef currentActor = newSg->getActor(currentActorName);
                uint currentRv = repetitionVector[currentActor->getId()];
                uint currentRepeatBefore =
                        actorAppearanceBefore(sos, currentActorName, schedulePointer);

                // Create channel from scheduling to current actor
                tempChannel = newSg->createChannel(schedulingActorPtr, currentActor);
                setChannelProperties(s,
                                     tempChannel,
                                     currentRv,
                                     1,
                                     currentRepeatBefore,
                                     procName,
                                     schedName,
                                     newSg.get());
            }
            newSg->reduceActor(s, schedulingActorPtr);

            // Set repetition vector of new actor
            repetitionVector = s->computeRepetitionVector();
            schedulingActorPtr->setRepetitionsOfScenario(*s,
                                                      repetitionVector[schedulingActorPtr->getId()]);

            actorCount++;
        }
        schedulePointer++;
    }
    return newSg;
}

/**
 * encodeSchedule ()
 * The function encodes a static periodic schedule into an scenario graph.
 * The function receives an scenario graph and a static schedule as inputs.
 * The output of this function is an scenario graph which the given
 * schedule is the only possible actor firing order.
 */
std::unique_ptr<ScenarioGraph> encodeSchedule(Scenario *s,
                              const CString &procName,
                              const CString &schedName,
                              ScenarioGraph *sg,
                              const StaticOrderSchedule &sos) {
    // Prevent inter-iteration execution
    std::unique_ptr<ScenarioGraph> newSg = limitInterIterationExecution(s, procName, schedName, sg, sos);

    // Create decision points
    DecisionPoints scenarioDecisionPoint;

    // Find decision points
    std::vector<std::vector<CString>> dpListOrg = FSMSADF::DecisionPoints::find(s, newSg.get(), sos);
    // Optimize points
    std::vector<std::vector<CString>> dpList = optimizeDecisionPoints(dpListOrg);

    // Add components
    return addScheduleEncodingComponents(s, procName, schedName, newSg.get(), sos, dpList);
}

/**
 * setChannelProperties ()
 * This function creates a channel between given actors
 */
void setChannelProperties(Scenario *s,
                          Channel *c,
                          Rate srcRate,
                          Rate dstRate,
                          uint initialTokens,
                          const CString &procName,
                          const CString &schedName,
                          ScenarioGraph *newSg) {
    c->getSrcPort()->isolateScenario(*s);
    c->getSrcPort()->setRateOfScenario(*s, srcRate);
    c->getDstPort()->isolateScenario(*s);
    c->getDstPort()->setRateOfScenario(*s, dstRate);
    c->setInitialAndFinalTokens(initialTokens);
    newSg->reduceInitialTokens(*s, *c);
    c->setTokenSizeOfScenario(*s, 0);

    CString initialTokenNames = "";
    for (uint i = 0; i < c->getInitialTokens(); i++) {
        if (!initialTokenNames.empty()) {
            initialTokenNames += ",";
        }

        initialTokenNames = initialTokenNames + schedName + "_" + s->getName() + "_" + procName
                            + "_t" + CString(c->getId()) + "_" + CString(i);
    }
    c->setPersistentAndFinalTokenNames(initialTokenNames);
    c->setName(schedName + "_" + s->getName() + "_ch" + CString(c->getId()));
    c->setTokenSizeOfScenario(*s, 0);
}

} // End namespace FSMSADF
