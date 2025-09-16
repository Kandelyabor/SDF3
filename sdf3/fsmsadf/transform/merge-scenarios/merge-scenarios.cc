/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   merge-scenarios.h
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   March 16, 2015
 *
 *  Function        :   Merge all scenarios for execution on CompSoC platform
 *
 *  History         :
 *      16-03-16    :   Initial version.
 *
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

#include "merge-scenarios.h"
#include "fsmsadf/base/actor.h"
#include "fsmsadf/base/fsm.h"
#include "fsmsadf/base/port.h"

#include "fsmsadf/resource_allocation/platform_binding/memory.h"
#include <memory>

namespace FSMSADF {

/**
 * mergeAllScenarios ()
 * Merge all scenarios into one graph to be executed
 * on a CompSOC platform
 */
std::unique_ptr<Graph> mergeAllScenarios(PlatformGraph *pg, PlatformBindings *pbs) {
    auto pbs_it = pbs->begin();

    if (pbs->empty()) {
        throw CException("No platform bindings specified for merging scenarios.");
    }

    if (std::distance(pbs->begin(), pbs->end()) <= 2) {
        // Use the last mapping
        pbs_it = pbs->end();
        pbs_it--;
    } else {
        // Use the second mapping after initial
        std::advance(pbs_it, 2);
    }

    std::unique_ptr<PlatformBinding> pb = (pbs_it->get())->clone(*(*pbs_it));

    // Get original application graph
    Graph *ag = pb->getApplicationGraph();

    // Check that switches and selects are identical in each scenario
    checkDetectorScenario(ag, pb.get());

    // The actual merge: create one application graph with all scenarios
    std::unique_ptr<Graph> newAg = reduceApplicationGraph(ag);

    // The application graph has only one scenario now
    const Scenario* newSc = (newAg->getScenarios().begin())->get();
    ScenarioGraph *newScGraph = newSc->getScenarioGraph();

    // Clear the storage distributions: these are invalid for the new graph
    newScGraph->clearStorageDistributions();

    // Clear the platform bindings
    pbs->clear();
    // Merge the mapping
    pbs->push_back(reduceMapping(ag, newAg.get(), pg, pb.get()));

    return newAg;
}

/**
 * checkDetectorScenario ()
 * This function performs basic sanity checks on the detector scenario:
 * -    Detector set correctly?
 * -    Switch/select actors identical in all scenarios?
 * -    Rates and channels of switches/selects match in non-detector scenarios?
 */
void checkDetectorScenario(Graph *ag, PlatformBinding * /*pb*/) {
    FSM *fsm = ag->getFSM();
    Scenarios *scenarios = &ag->getScenarios();
    ScenarioGraphs *scenarioGraphs = &ag->getScenarioGraphs();

    if ((scenarios->size() != scenarioGraphs->size()) || scenarios->empty()) {
        throw CException("Error checking detector scenario: inconsistent scenario amount.");
    }

    // Get detector scenario and actor
    ScenarioRef ds = fsm->getInitialState()->getScenario();
    ScenarioGraph *dsg = fsm->getInitialState()->getScenario()->getScenarioGraph();
    ActorRef detector = dsg->getActor(fsm->getDetectorName());
    // Check if the detector actor was properly set;
    if (!detector->isDetectorActor()) {
        throw CException("Error: detector actor not set properly.");
    }

    // Create list of switches and selects in a non-detector scenario and
    // check if each scenario has the same switch/select actors as the detector
    ActorRefs switchesSelects;
    auto sc_it = scenarios->begin();
    while ((*sc_it)->getName() == "detector") {
        sc_it++;
    }
    ScenarioGraph *sg = (*sc_it)->getScenarioGraph();
    for (const auto& a : sg->getActors()) {
        if ((a->getName().compare(0, 7, "switch_") == 0) // NOLINT(*magic-numbers)
            || (a->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*magic-numbers)
            // Check if switch/select exists in detector scenario
            if (!dsg->hasActor(a->getName())) {
                throw CException("Error: " + a->getName() + " not found in detector scenario.");
            }
            switchesSelects.push_back(a.get());
        }
    }

    // Extra reverse check to see if all switches/ selects from the
    // detector scenario also in the list
    for (const auto& da : dsg->getActors()) {
        if ((da->getName().compare(0, 7, "switch_") == 0) // NOLINT(*magic-numbers)
            || (da->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*magic-numbers)
            bool found = false;
            for (const auto& a : switchesSelects) {
                if (da->getName() == a->getName()) {
                    found = true;
                }
            }
            if (!found) {
                throw CException("Error: " + da->getName() + " not found in scenario "
                                 + (*sc_it)->getName() + ".");
            }
        }
    }

    // Check all switches/selects in each scenario
    for (const auto& s : *scenarios) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Except detector scenario
        if (s->getName() != ds->getName()) {
            for (const auto& swSel : switchesSelects) {
                // Actor exists?
                if (!sg->hasActor(swSel->getName())) {
                    throw CException("Error: could not find " + swSel->getName() + "in "
                                     + s->getName());
                }

                ActorRef currentSwSel = sg->getActor(swSel->getName());

                // Check if the ports of switch/select actors are consistent among scenarios
                for (const auto& dp : swSel->getPorts()) {
                    PortRef cp = currentSwSel->getPort(dp->getName());

                    // Check rates
                    if (dp->getRateOfScenario(*s) != cp->getRateOfScenario(*s)) {
                        throw CException("Rate of detector '" + (swSel->getName())
                                         + "' in scenario '" + (s->getName())
                                         + "' differs from other scenarios.");
                    }
                }
            }
        }
    }
}

/**
 * reduceApplicationGraph ()
 * Create a new application graph with all scenarios
 */
std::unique_ptr<Graph> reduceApplicationGraph(Graph *oldAg) {
    // Get old structures
    FSM *oldFsm = oldAg->getFSM();
    Scenarios *oldScenarios = &oldAg->getScenarios();

    ScenarioRef oldDs = oldFsm->getInitialState()->getScenario();
    ScenarioGraph *oldDsg = oldDs->getScenarioGraph();

    // Create new applicationGraph
    auto newAg = std::make_unique<Graph>();
    newAg->setName(oldAg->getName());

    // Add original detector scenario
    auto newSc =
            std::make_unique<Scenario>(GraphComponent(newAg.get(), static_cast<CId>(newAg->getScenarios().size()) + 1));
    newSc->setName("detector");
    auto *newScPtr = newSc.get();
    newAg->addScenario(std::move(newSc));
    newScPtr->setReward(0);
    newAg->setThroughputConstraint(oldAg->getThroughputConstraint());
    newAg->setUnits(oldAg->getUnits());

    // Add original detector scenario graph
    std::unique_ptr<ScenarioGraph> newScGraph = oldDsg->clone(
            GraphComponent(newAg.get(), static_cast<CId>(newAg->getScenarioGraphs().size())));
    newScPtr->setScenarioGraph(newScGraph.get());
    auto* newScGraphPtr = newScGraph.get();
    newAg->addScenarioGraph(std::move(newScGraph));

    // Remove all sync channels
    auto i = newScGraphPtr->getChannels().begin();
    while (i != newScGraphPtr->getChannels().end()) {
        ChannelRef newChannel = i->get();

        // !TODO: start channelName with 'sync', remove if-if
        if (newChannel->getName().size() >= 5) { // NOLINT(*magic-numbers)
            if (newChannel->getName().compare(newChannel->getName().size() - 5, 5, "_sync") == 0) { // NOLINT(*magic-numbers)
                newChannel->getSrcActor()->removePort(newChannel->getSrcPort()->getName());
                newChannel->getDstActor()->removePort(newChannel->getDstPort()->getName());
                i = newScGraphPtr->removeChannel(newChannel->getName());
            } else {
                i++;
            }
        } else {
            i++;
        }
    }

    // Create new FSM with only one state and one transition
    auto newFsm = std::make_unique<FSM>(GraphComponent(newAg.get(), 0));
    auto newState = std::make_unique<FSMstate>(
            GraphComponent(newFsm.get(), static_cast<CId>(newFsm->getStates().size()), "q1"));
    newState->setScenario(newScPtr);
    auto *newStatePtr = newState.get();
    newFsm->addState(std::move(newState));
    auto newTransition = std::make_unique<FSMtransition>(
            GraphComponent(newStatePtr, static_cast<CId>(newStatePtr->getTransitions().size())));
    newTransition->connect(newStatePtr, newStatePtr);
    newFsm->addTransition(std::move(newTransition));
    newFsm->setInitialState(newStatePtr);
    newFsm->setDetectorName(oldFsm->getDetectorName());
    newAg->setFSM(std::move(newFsm));

    // List to keep track of the original names of all actors added
    auto addedActors = std::make_unique<std::set<CString>>();

    // Add all other scenario graphs
    for (const auto& oldSc : *oldScenarios) {
        ScenarioGraph *oldScGraph = oldSc->getScenarioGraph();

        // Exclude old detector scenario
        if (oldSc.get() != oldDs) {
            // Add actors
            for (const auto& oldActor : oldScGraph->getActors()) {
                // Switch or select?
                if ((oldActor->getName().compare(0, 7, "switch_") == 0) // NOLINT(*magic-numbers)
                    || (oldActor->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*magic-numbers)
                    // Merge with existing switch or select
                    mergeSwitchOrSelect(
                            oldActor.get(), newScGraphPtr->getActor(oldActor->getName()), oldSc->getName());
                } else {
                    // Add all other actors
                    addActorToScenario(oldActor.get(), oldSc.get(), newScPtr, addedActors.get());
                }
            }

            // Add channels, except sync
            for (const auto& oldChannel : oldScGraph->getChannels()) {
                // Skip sync channels !TODO: start channelName with 'sync', remove if-else
                if (oldChannel->getName().size() < 5) { // NOLINT(*magic-numbers,*branch-clone)
                    addChannelToScenario(oldChannel.get(), oldSc.get(), newScPtr);
                } else if (oldChannel->getName().compare(
                                   oldChannel->getName().size() - 5, 5, "_sync") // NOLINT(*magic-numbers)
                           != 0) {
                    addChannelToScenario(oldChannel.get(), oldSc.get(), newScPtr);
                }
            }
        }
    }
    return newAg;
}

/**
 * mergeSwitchOrSelect ()
 * Copy all ports from the src actor to the detect actor
 * except 'sync' ports. Add scenario name to port name.
 */
void mergeSwitchOrSelect(Actor *src, Actor *detect, const CString &oldScenarioName) {
    for (const auto& pp : src->getPorts()) {
        if (pp->getName().compare(pp->getName().size() - 6, 5, "sync_") != 0) { // NOLINT(*magic-numbers)
            std::unique_ptr<Port> p =
                    pp->clone(GraphComponent(detect, static_cast<CId>(detect->getPorts().size())));
            auto* pr = p.get();
            detect->addPort(std::move(p));
            pr->setName(pr->getName() + "_" + oldScenarioName);
        }
    }
}

/**
 * addActorToScenario()
 * Clone an actor into the scenariograph of another application.
 */
void addActorToScenario(Actor *oldActor,
                        Scenario *oldScenario,
                        Scenario *newScenario,
                        std::set<CString> *addedActors) {
    ScenarioGraph *newScGraph = newScenario->getScenarioGraph();

    // Create new actor
    auto newActor =
            std::make_unique<Actor>(GraphComponent(newScGraph, static_cast<CId>(newScGraph->getActors().size())));

    // Set basic properties of new actor
    newActor->setName(oldActor->getName() + "_" + oldScenario->getName());
    newActor->setType(oldActor->getType());

    // Transfer the ProcessorTypes to the new actor
    Actor::ProcessorTypes oldPts = oldActor->getProcessorTypes();
    for (Actor::ProcessorTypes::const_iterator i = oldPts.begin(); i != oldPts.end(); i++) {
        const Actor::ProcessorType& oldPt = *i;

        newActor->setExecutionTimeOfScenario(
                *newScenario,
                oldPt.type,
                oldActor->getExecutionTimeOfScenario(*oldScenario, oldPt.type));
        // If actor was already added for another scenario, remove memory requirements to avoid
        // double count
        if (addedActors->find(oldActor->getName()) != addedActors->end()) {
            newActor->clearMemoriesOfScenario(*newScenario, oldPt.type);
        } else {
            newActor->setMemoriesOfScenario(
                    *newScenario,
                    oldPt.type,
                    oldActor->getMemoriesOfScenario(*oldScenario, oldPt.type));
        }
        newActor->setImplementation(
                *newScenario, oldPt.type, oldActor->getImplementation(*oldScenario, oldPt.type));
    }

    // Set all other properties
    newActor->setDefaultProcessorType(oldActor->getDefaultProcessorType());
    newActor->setRepetitionsOfScenario(*newScenario,
                                       oldActor->getRepetitionsOfScenario(*oldScenario));
    newActor->setDetectorActor(oldActor->isDetectorActor());

    // Add new actor to scenario graph
    auto* newActorPtr = newActor.get();
    newScGraph->addActor(std::move(newActor));

    // Clone all ports to new actor
    for (auto iter = oldActor->getPorts().begin();
         iter != oldActor->getPorts().end();
         iter++) {
        Port* oldP = iter->get();
        std::unique_ptr<Port> newP = oldP->clone(GraphComponent(newActorPtr, oldP->getId()));
        newActorPtr->addPort(std::move(newP));
    }
    addedActors->insert(oldActor->getName());
}

/**
 * addChannelToScenario()
 * Clone a channel into the scenariograph of another application.
 */
void addChannelToScenario(Channel *oldChannel, Scenario *oldScenario, Scenario *newScenario) {
    ScenarioGraph *newScGraph = newScenario->getScenarioGraph();

    // Create new channel
    auto newChannel = std::make_unique<Channel>(
            GraphComponent(newScGraph, static_cast<CId>(newScGraph->getChannels().size())));

    // Set basic properties of new actors
    newChannel->setName(oldChannel->getName() + "_" + oldScenario->getName());
    newChannel->setTokenSizeOfScenario(*newScenario,
                                       oldChannel->getTokenSizeOfScenario(*oldScenario));

    // Set initial tokens
    newChannel->setInitialTokens(oldChannel->getInitialTokens());

    // set final tokens
    newChannel->setFinalTokens(oldChannel->getFinalTokens());

    // Set token names
    newChannel->setPersistentAndFinalTokenNames(oldChannel->getPersistentTokenNames());
    newChannel->setPersistentFinalTokenNames(oldChannel->getPersistentFinalTokenNames());

    // Connect src of channel
    if (oldChannel->getSrcActor() != nullptr) {
        CString newSrcName;
        CString newSrcPortName;

        // Get src name and src port name
        if ((oldChannel->getSrcActor()->getName().compare(0, 7, "switch_") == 0) // NOLINT(*magic-numbers)
            || (oldChannel->getSrcActor()->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*magic-numbers)
            newSrcName = oldChannel->getSrcActor()->getName();
            newSrcPortName = oldChannel->getSrcPort()->getName() + "_" + oldScenario->getName();
        } else {
            newSrcName = oldChannel->getSrcActor()->getName() + "_" + oldScenario->getName();
            newSrcPortName = oldChannel->getSrcPort()->getName();
        }
        Port* srcP = newChannel->getScenarioGraph()->getActor(newSrcName)->getPort(newSrcPortName);
        newChannel->connectSrc(srcP);
    }

    // Connect dst of channel
    if (oldChannel->getDstActor() != nullptr) {
        CString newDstName;
        CString newDstPortName;

        // Get dst name and dst port name
        if ((oldChannel->getDstActor()->getName().compare(0, 7, "switch_") == 0) // NOLINT(*magic-numbers)
            || (oldChannel->getDstActor()->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*magic-numbers)
            newDstName = oldChannel->getDstActor()->getName();
            newDstPortName = oldChannel->getDstPort()->getName() + "_" + oldScenario->getName();
        } else {
            newDstName = oldChannel->getDstActor()->getName() + "_" + oldScenario->getName();
            newDstPortName = oldChannel->getDstPort()->getName();
        }

        Port* dstP = newChannel->getScenarioGraph()->getActor(newDstName)->getPort(newDstPortName);
        newChannel->connectDst(dstP);
    }
    // Add new channel to scenario graph
    newScGraph->addChannel(std::move(newChannel));
}

/**
 * reduceMapping ()
 * Create a new binding with only the detector scenario of the old
 * mapping, then merge the other scenarios in.
 */
std::unique_ptr<PlatformBinding>
reduceMapping(Graph *oldAg, Graph *newAg, PlatformGraph *pg, PlatformBinding *oldPb) {
    // Create a new platform binding
    // By default it will contain the correct tiles and processors
    auto newPb = std::make_unique<PlatformBinding>(GraphComponent(nullptr, 0),
                                      pg,
                                      newAg,
                                      BindingMethod(BindingMethod::Method::SumOfScenarios));
    newPb->setName(oldPb->getName());

    // The application graph has only one scenario now
    auto *newSc = (newAg->getScenarios().begin())->get();
    ScenarioGraph *newScGraph = newSc->getScenarioGraph();

    // Iterate over tile bindings of the original platformBinding
    for (const auto& oldTb : oldPb->getTileBindings()) {
        TileBinding *newTb = newPb->getTileBinding(oldTb->getTile());

        // Iterate over processor bindings
        for (const auto& oldProcBinding : oldTb->getProcessorBindings()) {
            ProcessorBinding* newProcBinding =
                    newTb->getProcessorBinding(*(oldProcBinding->getProcessor()));

            // We need to know the loadSched actor on each processor
            Actor* loadSched = nullptr;

            // Iterate over the actorBindings
            for (auto &[oldSc, actors] : oldProcBinding->getActorBindings()) {
                // Iterate over all actors mapped to this processor in this scenario
                for (auto *oldActor : actors) {
                    if (oldSc->getName() == "detector") {
                        // Register loadSched actor
                        if (oldActor->getName().compare(0, 9, "loadSched") == 0) { // NOLINT(*magic-numbers)
                            loadSched = newScGraph->getActor(oldActor->getName());
                        }
                        // Add this actor to new processorBinding
                        if (!newProcBinding->addActorBinding(
                                    *newSc, newScGraph->getActor(oldActor->getName()))) {
                            throw CException("Error while adding actorBinding of actor "
                                             + oldActor->getName() + " to "
                                             + newProcBinding->getName());
                        }
                    }
                    // If not detector scenario, add actors only if not switch/select
                    else if ((oldActor->getName().compare(0, 7, "switch_") != 0) // NOLINT(*magic-numbers)
                             && (oldActor->getName().compare(0, 7, "select_") != 0)) { // NOLINT(*magic-numbers)
                        // Extend name with old scenario name
                        oldActor->setName(oldActor->getName() + "_" + oldSc->getName());
                        // Add this actor to new processorBinding
                        if (!newProcBinding->addActorBinding(
                                    *newSc, newScGraph->getActor(oldActor->getName()))) {
                            throw CException("Error while adding actorBinding of actor "
                                             + oldActor->getName() + " to "
                                             + newProcBinding->getName());
                        }
                    }
                }
            }

            // Check if all WheelsizeAllocations are identical
            Time wheelsizeAllocation = oldProcBinding->getWheelsizeAllocations().begin()->second;
            for (auto [s, n] : oldProcBinding->getWheelsizeAllocations()) {
                if (n != wheelsizeAllocation) {
                    throw CException("WheelsizeAllocation for '" + s->getName()
                                     + "' does not match others.");
                }
            }
            // Set the wheelsizeAllocation for the new processorBinding
            std::map<const Scenario *, Time> newWheelsizeAllocations;
            newWheelsizeAllocations[newSc] = wheelsizeAllocation;
            newProcBinding->setWheelsizeAllocations(newWheelsizeAllocations);

            // Iterate over the original scenarios
            for (const auto& oldSc : oldAg->getScenarios()) {
                // Check if static order schedule exists in this processorBinding
                if (oldProcBinding->hasStaticOrderSchedule(*oldSc)) {
                    if (loadSched == nullptr) {
                        throw CException("No loadSched actor in '" + oldSc->getName() + "'.");
                    }

                    StaticOrderSchedule sos = oldProcBinding->getStaticOrderSchedule(*oldSc);

                    // Convert SOS into list of strings
                    std::list<CString> sosString;
                    for (const auto &l : sos) {
                        if ((oldSc->getName() == "detector")
                            || ((l.actor->getName().compare(0, 7, "switch_") != 0) // NOLINT(*magic-numbers)
                                && (l.actor->getName().compare(0, 7, "select_") != 0))) { // NOLINT(*magic-numbers)
                            sosString.push_back(l.actor->getName());
                        }
                    }
                    if (!sosString.empty()) {
                        // Add static order schedule to loadSched actor
                        newProcBinding->addCompsocScenarioSchedule(oldSc->getName(), sosString);
                    }
                }
            }
        }

        // Merge memory bindings
        MemoryBinding *oldMemBinding = oldTb->getMemoryBinding();
        MemoryBinding *newMemBinding = newTb->getMemoryBinding();

        // Iterate over all memories
        for (auto *oldMemory : oldMemBinding->getMemories()) {
            // Iterate over the original scenarios
            for (const auto& oldSc : oldAg->getScenarios()) {
                // Actor binding exists for this scenario?
                const MemoryBinding::ActorBindings *oldScActorIter =
                        oldMemBinding->findActorBinding(oldSc->getName());
                if (oldScActorIter != nullptr) {
                    // List of all actors bound to this memory
                    for (const auto &ab : *oldScActorIter) {
                        // Bound to this memory?
                        if (ab.tile_memory == oldMemory) {
                            // Add the actor binding to the new scenario
                            if (!newMemBinding->addActorBinding(
                                        *newSc,
                                        newScGraph->getActor(ab.actor->getName()),
                                        ab.processorType)) {
                                throw CException("Error while adding actorBinding of actor "
                                                 + ab.actor->getName() + " to "
                                                 + newMemBinding->getName());
                            }
                        }
                    }
                }

                // Channel binding exists for this scenario?
                const MemoryBinding::ChannelBindings *oldScChannelIter =
                        oldMemBinding->findChannelBinding(oldSc->getName());
                if (oldScChannelIter != nullptr) {
                    // List of all channels bound to this memory
                    for (auto cb : *oldScChannelIter) {
                        // Bound to this memory?
                        if (cb.memory == oldMemory) {
                            CString oldChannelName = cb.channel->getName();

                            // Skip sync channels !TODO: start channelName with 'sync', remove
                            // condition
                            if ((oldChannelName.size() < 5) // NOLINT(*magic-numbers)
                                || (oldChannelName.compare(oldChannelName.size() - 5, 5, "_sync") // NOLINT(*magic-numbers)
                                    != 0)) {
                                CString newChannelName;
                                if (oldSc->getName() != "detector") {
                                    newChannelName = oldChannelName + "_" + oldSc->getName();
                                } else {
                                    newChannelName = oldChannelName;
                                }
                                // Get old channel binding constraints
                                std::unique_ptr<ChannelBindingConstraint> oldCBC =
                                        oldMemBinding->getGraphBindingConstraints()
                                                ->getConstraintsOfScenario(*oldSc)
                                                ->getConstraintsOfChannel(oldChannelName)
                                                ->clone(newChannelName);
                                // Add them to the new memory binding
                                newMemBinding->getGraphBindingConstraints()
                                        ->getConstraintsOfScenario(*newSc)
                                        ->setConstraintsOfChannel(newChannelName, std::move(oldCBC));
                                // Finally add the channel binding to the new scenario
                                if (!newMemBinding->addChannelBinding(
                                            *newSc,
                                            newScGraph->getChannel(newChannelName),
                                            cb.bufferLocation)) {
                                    throw CException("Error while adding channelBinding of channel "
                                                     + newChannelName);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Merge network interface bindings
        NetworkInterfaceBindings& oldNIBs = oldTb->getNetworkInterfaceBindings();

        for (const auto& oldNIB : oldNIBs) {
            NetworkInterfaceBinding *newNIB =
                    newTb->getNetworkInterfaceBinding(oldNIB->getNetworkInterface());

            // Iterate over original inChannels
            for (auto &[oldSc, bindings] : oldNIB->getInChannelBindings()) {
                // Iterate over channels in current binding
                for (auto *oldChannel : bindings) {
                    CString oldChannelName = oldChannel->getName();

                    // Skip sync channels !TODO: start channelName with 'sync', remove condition
                    if ((oldChannelName.size() < 5) // NOLINT(*magic-numbers)
                        || (oldChannelName.compare(oldChannelName.size() - 5, 5, "_sync") != 0)) { // NOLINT(*magic-numbers)
                        Channel *newChannel = nullptr;

                        if (oldSc->getName() != "detector") {
                            newChannel =
                                    newScGraph->getChannel(oldChannelName + "_" + oldSc->getName());
                        } else {
                            newChannel = newScGraph->getChannel(oldChannelName);
                        }
                        // Add channel in binding to network interface binding
                        if (!newNIB->addInChannelBinding(*newSc, newChannel)) {
                            throw CException("Error while adding in-channelBinding of channel "
                                             + newChannel->getName());
                        }
                    }
                }
            }
            // Iterate over original outChannels
            for (auto &[oldSc, bindings] : oldNIB->getOutChannelBindings()) {
                // Iterate over channels in current binding
                for (auto *oldChannel : bindings) {
                    CString oldChannelName = oldChannel->getName();

                    // Skip sync channels !TODO: start channelName with 'sync', remove condition
                    if ((oldChannelName.size() < 5) // NOLINT(*magic-numbers)
                        || (oldChannelName.compare(oldChannelName.size() - 5, 5, "_sync") != 0)) { // NOLINT(*magic-numbers)
                        Channel *newChannel = nullptr;

                        if (oldSc->getName() != "detector") {
                            newChannel =
                                    newScGraph->getChannel(oldChannelName + "_" + oldSc->getName());
                        } else {
                            newChannel = newScGraph->getChannel(oldChannelName);
                        }
                        // Add channel out binding to network interface binding
                        if (!newNIB->addOutChannelBinding(*newSc, newChannel)) {
                            throw CException("Error while adding out-channelBinding of channel "
                                             + newChannel->getName());
                        }
                    }
                }
            }
        }
    }

    // Merge connection bindings
    for (const auto& oldCb: oldPb->getConnectionBindings()) {
        ConnectionBinding *newCb = newPb->getConnectionBinding(oldCb->getConnection());

        // Iterate over original scenarios
        for (auto& [oldSc, bindings]: oldCb->getChannelBindings()) {
            // Iterate over channels in current binding
            for (auto *oldChannel: bindings) {
                CString oldChannelName = oldChannel->getName();

                // Skip sync channels !TODO: start channelName with 'sync', remove condition
                if ((oldChannelName.size() < 5) // NOLINT(*magic-numbers)
                    || (oldChannelName.compare(oldChannelName.size() - 5, 5, "_sync") != 0)) { // NOLINT(*magic-numbers)
                    Channel *newChannel = nullptr;

                    if (oldSc->getName() != "detector") {
                        newChannel =
                                newScGraph->getChannel(oldChannelName + "_" + oldSc->getName());
                    } else {
                        newChannel = newScGraph->getChannel(oldChannelName);
                    }
                    // Add channel binding to network interface binding
                    newCb->addChannelBinding(*newSc, newChannel);
                }
            }
        }
    }
    return newPb;
}

} // namespace FSMSADF
