/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   create-detector.cc
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   September 15, 2015
 *
 *  Function        :   Create scenario detector scenario that contains the scenario detection actor
 *
 *  History         :
 *      15-09-15    :   Initial version.
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

#include "create-detector.h"
#include "fsmsadf/analysis/analysis.h" // IWYU pragma: keep
#include "fsmsadf/base/fsm.h"
#include "fsmsadf/base/port.h"
#include <memory>

namespace FSMSADF {

/**
 * createDetectorScenario ()
 * The transformation adds an additional detector scenario that contains
 * the detector actor, whose in- and outgoing edges are connected to the
 * other scenarios by means of switch/select actors.
 */
void createDetectorScenario(FSM *fsm, Scenarios& scenarios, ScenarioGraphs& scenarioGraphs) {
    // Check that channel names, number and rates are identical in each scenario
    checkDetectorActor(fsm, scenarios, scenarioGraphs);

    // Create detector scenario graph
    GraphComponent *ag = scenarioGraphs.front()->getGraph();
    auto dsg = std::make_unique<ScenarioGraph>(
            GraphComponent(ag, static_cast<CId>(scenarioGraphs.size()), "detector"));
    dsg->setType("scenariograph");

    // Create detector scenario, add 1 to skip default scenario
    auto ds =
            std::make_unique<Scenario>(GraphComponent(ag, static_cast<CId>(scenarios.size()) + 1, "detector"));
    ds->setScenarioGraph(dsg.get());
    ds->setReward(0.0);

    // Update FSM
    addDetectorScenarioToFSM(fsm, ds.get());

    // Get detector name
    CString detectorName = fsm->getDetectorName();

    // Copy detector actor with self-edges to detector scenario
    extractDetector(detectorName, *(*(scenarios.begin())), *ds);

    // List with persistent tokens per scenario
    std::map<CString, std::list<Scenario *>> ptInScenarios;

    // Find and transform detector actor in each scenario
    for (const auto& s : scenarios) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Get detector
        ActorRef oldDetector = sg->getActor(detectorName);

        // Set execution time and memory settings to zero
        // The old converter will be transformed to a switch and ET and mem and
        // are thus architecture dependent and should be added in the binding aware graph
        resetActorProperties(s.get(), oldDetector);

        // Split detector in switches and selects
        splitDetector(oldDetector, s.get());

        // Add switches and selects to detector scenario
        copySwitchesAndSelects(detectorName, s.get(), ds.get());

        // Add synchronization tokens
        addSynchronizationTokens(s.get(), ds.get());
    }
    // Isolate, just to be sure
    dsg->isolateScenario(*ds);

    // Set repetition vectors
    RepetitionVector repetitionVector = ds->computeRepetitionVector();
    ds->setRepetitionVector(repetitionVector);

    // Add detector scenario and graph
    scenarioGraphs.push_back(std::move(dsg));
    scenarios.insert(std::move(ds));
}

/**
 * checkDetectorActor ()
 * This function performs the following basic sanity checks regarding detectors:
 * -    check if each scenario has indeed an actor with the detector name;
 * -    check if that actor was properly set as detector;
 * -    check if the number of ports on the detector is equal among scenarios;
 * -    check if the port names, rates and channel names are equal among scenarios.
 */
void checkDetectorActor(FSM *fsm, const Scenarios& scenarios, ScenarioGraphs& scenarioGraphs) {
    if ((scenarios.size() != scenarioGraphs.size()) || (scenarios.empty())) {
        throw CException("Error creating detector scenario: inconsistent scenarios.");
    }

    // Get  detector
    CString detectorName = fsm->getDetectorName();

    ActorRef defaultDetector = nullptr;
    uint defaultPortsAmount = 0;

    // Find and check detector actor in each scenario
    for (const auto& s : scenarios) {
        ScenarioGraph *sg = s->getScenarioGraph();

        if (!sg->hasActor(detectorName)) {
            throw CException("Scenario '" + (s->getName()) + "' has no detector named '"
                             + detectorName + "'.");
        }
        if (!sg->getActor(detectorName)->isDetectorActor()) {
            throw CException("Actor '" + CString(detectorName)
                             + "' was not set as detector in scenario '" + (s->getName()) + "'.");
        }

        if (defaultDetector == nullptr) {
            defaultDetector = sg->getActor(detectorName);
            defaultPortsAmount = static_cast<uint>(std::distance(
                    defaultDetector->getPorts().begin(), defaultDetector->getPorts().end()));
        } else {
            ActorRef currentDetector = sg->getActor(detectorName);

            uint currentPortsAmount = static_cast<uint>(std::distance(
                    currentDetector->getPorts().begin(), currentDetector->getPorts().end()));
            if (defaultPortsAmount != currentPortsAmount) {
                throw CException("Number of detector ports of '" + CString(detectorName)
                                 + "' in scenario '" + (s->getName()) + "' is '"
                                 + CString(currentPortsAmount)
                                 + "' and differs from other scenarios.");
            }

            for (const auto& dp : defaultDetector->getPorts()) {
                PortRef cp = currentDetector->getPort(dp->getName());
                if (dp->getRateOfScenario(*s) != cp->getRateOfScenario(*s)) {
                    throw CException("Rate of detector '" + CString(detectorName)
                                     + "' in scenario '" + (s->getName())
                                     + "' differs from other scenarios.");
                }
                if (dp->getChannel()->getName() != cp->getChannel()->getName()) {
                    throw CException("Channel name '" + (cp->getChannel()->getName())
                                     + "' of port '" + (cp->getName())
                                     + "' differs from other scenarios.");
                }
            }
        }
    }
}

/**
 * addDetectorScenarioToFSM ()
 * This function adds the new detector scenario to the FSM in each transition.
 * It is assumed that every state starts with a detector actor.
 */
void addDetectorScenarioToFSM(FSM *fsm, Scenario *ds) {
    auto detectorState =
            std::make_unique<FSMstate>(GraphComponent(fsm, static_cast<CId>(fsm->getStates().size()), "detector"));
    detectorState->setScenario(ds);

    for (const auto& state: fsm->getStates()) {
        for (auto j = state->getTransitions().begin();
             j != state->getTransitions().end();) {
            FSMtransition *trans = *j;

            if (trans->getDstState() != detectorState.get()) {
                // Insert detector state
                // Avoid inserting double transitions
                if (!state->hasTransitionTo(detectorState.get())) {
                    auto newTrans = std::make_unique<FSMtransition>(GraphComponent(fsm->getGraph(), 0));
                    newTrans->connect(state.get(), detectorState.get());
                    fsm->addTransition(std::move(newTrans));
                }

                // Avoid inserting double transitions
                if (!detectorState->hasTransitionTo(trans->getDstState())) {
                    auto newTrans = std::make_unique<FSMtransition>(GraphComponent(fsm->getGraph(), 0));
                    newTrans->connect(detectorState.get(), trans->getDstState());
                    fsm->addTransition(std::move(newTrans));
                }

                // Remove original transition
                j = state->removeTransition(trans);
            } else {
                j++;
            }
        }
    }
    auto* detectorStatePtr = detectorState.get();
    fsm->addState(std::move(detectorState));
    fsm->setInitialState(detectorStatePtr);
}

/**
 * extractDetector ()
 * This function extracts the detector from the first regular scenario and
 * copies it to the new detector scenario, along with all self-edges.
 */
void extractDetector(const CString& detectorName, const Scenario& s, const Scenario& ds) {
    // Get detector scenario graph
    ScenarioGraph *dsg = ds.getScenarioGraph();
    ScenarioGraph *sg = s.getScenarioGraph();

    ActorRef oldDetector = sg->getActor(detectorName);

    // Add the old detector to the detector scenario
    std::unique_ptr<Actor> newDetector = oldDetector->createCopy(GraphComponent(dsg, static_cast<CId>(dsg->getActors().size())));
    // Copy execution time and memory properties
    newDetector->setExecutionTimeOfScenario(
            ds,
            newDetector->getDefaultProcessorType(),
            oldDetector->getExecutionTimeOfScenario(s, oldDetector->getDefaultProcessorType()));
    newDetector->setMemoriesOfScenario(
            ds,
            newDetector->getDefaultProcessorType(),
            oldDetector->getMemoriesOfScenario(s, oldDetector->getDefaultProcessorType()));
    auto* newDetectorPtr = newDetector.get();
    dsg->addActor(std::move(newDetector));
    newDetectorPtr->setDetectorActor(true);

    // Find and copy all self-edges on detector to detector scenario
    for (const auto& p: oldDetector->getPorts()) {
        Channel *ch = p->getChannel();
        Port *oldSrcPort = (p->getType() == Port::PortType::Out) ? (p.get()) : (ch->getSrcPort());
        Port *oldDstPort = (p->getType() == Port::PortType::In) ? (p.get()) : (ch->getDstPort());

        // Self-edge?
        if ((p->getType() == Port::PortType::Out)
            && (ch->getSrcActor()->getName() == ch->getDstActor()->getName())) {
            // Copy any self-edge to new detector in detector scenario
            std::unique_ptr<Channel> newSelfEdge =
                    ch->createCopy(GraphComponent(dsg, static_cast<CId>(dsg->getChannels().size())));
            newSelfEdge->isolateScenario(ds);
            newSelfEdge->setTokenSizeOfScenario(ds, ch->getTokenSizeOfScenario(s));

            std::unique_ptr<Port> newSrcPort = oldSrcPort->createCopy(
                    GraphComponent(newDetectorPtr, static_cast<CId>(newDetectorPtr->getPorts().size())));
            std::unique_ptr<Port> newDstPort = oldDstPort->createCopy(
                    GraphComponent(newDetectorPtr, static_cast<CId>(newDetectorPtr->getPorts().size()) + 1));

            auto *newSrcPortPtr = newSrcPort.get();
            newDetectorPtr->addPort(std::move(newSrcPort));
            auto *newDstPortPtr = newDstPort.get();
            newDetectorPtr->addPort(std::move(newDstPort));

            newSelfEdge->connectSrc(newSrcPortPtr);
            newSelfEdge->connectDst(newDstPortPtr);

            dsg->addChannel(std::move(newSelfEdge));
        }
    }
}

/**
 * splitDetector ()
 * This function removes all self-edges from the old detector,
 * and then splits it in switches and selects for each remaining port.
 */
void splitDetector(Actor *detector, Scenario *s) {
    ScenarioGraph *sg = s->getScenarioGraph();

    // Split up old detector, one switch for each outgoing channel, one select for each incoming
    // channel
    uint switches = 0;
    uint selects = 0;
    for (auto j = detector->getPorts().begin(); j != detector->getPorts().end();) {
        Port* p = j->get();
        Channel *ch = p->getChannel();

        Port *otherPort =
                (p->getType() == Port::PortType::Out) ? (ch->getDstPort()) : (ch->getSrcPort());

        // Remove all self-edges on detector
        if (ch->getSrcActor()->getName() == ch->getDstActor()->getName()) {
            // Remove self-edge on old detector actor
            detector->removePort(otherPort->getName());
            j = detector->removePort(p->getName());
            sg->removeChannel(ch->getName());
            // Re-set ID of all remaining channels
            for (auto i = sg->getChannels().begin();
                 i != sg->getChannels().end();
                 i++) {
                ChannelRef c = i->get();
                c->setId(static_cast<CId>(std::distance(sg->getChannels().begin(), i)));
            }
        }
        // Duplicate and connect switches and selects in detector scenario
        else if (p->getType() == Port::PortType::Out) {
            // If out port, create switch
            CString switchName = ("switch_" + ch->getName());

            if (switches == 0) {
                // The first switch is created by rebranding the old detector
                detector->setName(switchName);
                detector->setType(switchName);
                ++j;
            }
            // Create new switch for additional out ports
            else {
                std::unique_ptr<Actor> newSwitch =
                        detector->createCopy(GraphComponent(sg, static_cast<CId>(sg->getActors().size())));
                newSwitch->setName(switchName);
                newSwitch->setType(switchName);
                auto *newSwitchPtr = newSwitch.get();
                sg->addActor(std::move(newSwitch));

                std::unique_ptr<Port> newPort = p->clone(GraphComponent(
                        newSwitchPtr, static_cast<CId>(newSwitchPtr->getPorts().size())));
                // Disconnect and remove old port
                ch->connectSrc(nullptr);
                j = detector->removePort(p->getName());
                // Connect channel to new switch
                auto* newPortPtr = newPort.get();
                newSwitchPtr->addPort(std::move(newPort));
                ch->connectSrc(newPortPtr);
            }

            // Rename channel because the destination is different in each scenario
            ch->setName(ch->getName() + "_" + CString(s->getId()));
            switches++;
        } else {
            // If in port, create new select
            CString selectName = ("select_" + ch->getName());

            std::unique_ptr<Actor> newSelect =
                    detector->createCopy(GraphComponent(sg, static_cast<CId>(sg->getActors().size())));
            newSelect->setName(selectName);
            newSelect->setType(selectName);
            auto* newSelectPtr = newSelect.get();
            sg->addActor(std::move(newSelect));

            std::unique_ptr<Port> newPort = p->clone(GraphComponent(
                    newSelectPtr, static_cast<CId>(newSelectPtr->getPorts().size())));
            // Disconnect and remove old port
            ch->connectDst(nullptr);
            j = detector->removePort(p->getName());
            // Connect channel to new select
            auto* newPortPtr = newPort.get();
            newSelectPtr->addPort(std::move(newPort));
            ch->connectDst(newPortPtr);

            // There must be initial tokens in the channel
            if (ch->getInitialTokens() == 0) {
                throw CException("In-channel has no initial tokens, detector will never fire");
}

            // Rename channel because the destination is different in each scenario
            ch->setName(ch->getName() + "_" + CString(s->getId()));
            selects++;
        }
    }
}

/**
 * copySwitchesAndSelects ()
 * This function extracts all switches and selects from each scenario and
 * copies these to the new detection scenario, then connects them to the detector actor.
 */
void copySwitchesAndSelects(const CString& detectorName, Scenario *s, Scenario *ds) {
    // Get scenario graphs
    ScenarioGraph *sg = s->getScenarioGraph();
    ScenarioGraph *dsg = ds->getScenarioGraph();

    ActorRef detector = dsg->getActor(detectorName);

    // Iterate over actors in original scenario
    for (const auto& a: sg->getActors()) {
        if (((a->getName().compare(0, 7, "switch_") == 0) // NOLINT(*-magic-numbers)
             || (a->getName().compare(0, 7, "select_") == 0)) // NOLINT(*-magic-numbers)
            && !dsg->hasActor(a->getName())) {
            Port* p = a->getPorts().begin()->get();

            // Copy switches and selects to detector scenario
            std::unique_ptr<Actor> newActor = a->createCopy(GraphComponent(dsg, static_cast<CId>(dsg->getActors().size())));
            newActor->setExecutionTimeOfScenario(
                    *ds,
                    newActor->getDefaultProcessorType(),
                    a->getExecutionTimeOfScenario(*s, a->getDefaultProcessorType()));
            newActor->setMemoriesOfScenario(
                    *ds,
                    newActor->getDefaultProcessorType(),
                    a->getMemoriesOfScenario(*s, a->getDefaultProcessorType()));
            auto* newActorPtr = newActor.get();
            dsg->addActor(std::move(newActor));

            const CString chName = newActorPtr->getName();
            // Switches/selects connected to the detector can only have one outgoing edge here
            if (std::distance(a->getPorts().begin(), a->getPorts().end()) != 1) {
                throw CException(
                        "Actor '" + (a->getName()) + "' has "
                        + CString(std::distance(a->getPorts().begin(), a->getPorts().end()))
                        + " ports, 1 allowed.");
            }

            Channel *ch = p->getChannel();
            std::unique_ptr<Channel> dataCh = ch->createCopy(GraphComponent(dsg, static_cast<CId>(dsg->getChannels().size())));
            dataCh->setName(chName + "_data");
            dataCh->setTokenSizeOfScenario(*ds, ch->getTokenSizeOfScenario(*s));
            // Add data channel
            if (a->getName().compare(0, 7, "switch_") == 0) { // NOLINT(*-magic-numbers)
                dataCh->connectSrc(createPort(
                        detector, Port::PortType::Out, chName, ds, p->getRateOfScenario(*s)));
                dataCh->connectDst(createPort(
                        newActorPtr, Port::PortType::In, chName, ds, p->getRateOfScenario(*s)));
            } else {
                dataCh->connectSrc(createPort(
                        newActorPtr, Port::PortType::Out, chName, ds, p->getRateOfScenario(*s)));
                dataCh->connectDst(createPort(
                        detector, Port::PortType::In, chName, ds, p->getRateOfScenario(*s)));
            }
            dsg->addChannel(std::move(dataCh));

            // Add control channel
            ChannelRef controlCh =
                    dsg->createChannel(GraphComponent(dsg, static_cast<CId>(dsg->getChannels().size())));
            controlCh->setName(chName + "_ctrl");
            controlCh->setTokenSizeOfScenario(*ds, stdTokenSize);

            controlCh->connectSrc(
                    createPort(detector, Port::PortType::Out, (chName + "_ctrl"), ds, stdRate));
            controlCh->connectDst(
                    createPort(newActorPtr, Port::PortType::In, (chName + "_ctrl"), ds, stdRate));
        }
    }
}

/**
 * addSynchronizationTokens ()
 * For each switch and select in scenario s, this function adds a self-edge with
 * synchronization token on the switch/select in the original *and* the detector scenario.
 */
void addSynchronizationTokens(Scenario *s, Scenario *ds) {
    // Get scenario graphs
    ScenarioGraph *sg = s->getScenarioGraph();
    ScenarioGraph *dsg = ds->getScenarioGraph();

    // Iterate over actors in original scenario
    for (const auto& a: sg->getActors()) {
        if ((a->getName().compare(0, 7, "switch_") == 0) // NOLINT(*-magic-numbers)
            || (a->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*-magic-numbers)
            Port* p = a->getPorts().begin()->get();

            // Switches/selects connected to the detector can only have one outgoing edge here
            if (std::distance(a->getPorts().begin(), a->getPorts().end()) != 1) {
                throw CException(
                        "Actor '" + (a->getName()) + "' has "
                        + CString(std::distance(a->getPorts().begin(), a->getPorts().end()))
                        + " ports, 1 allowed.");
            }

            Channel *ch = p->getChannel();
            ch->setInitialAndFinalTokens(0);
            ch->setPersistentAndFinalTokenNames("");
            ch->setFinalTokens(0);
            ch->setPersistentFinalTokenNames("");

            // Add self-edge with synchronization token on switch in original scenario
            ChannelRef syncCh = sg->createChannel(GraphComponent(sg, static_cast<CId>(sg->getChannels().size())));
            CString syncName = a->getName() + "_sync";
            syncCh->setName(syncName);
            syncCh->setTokenSizeOfScenario(*s, stdTokenSize);
            syncCh->setInitialAndFinalTokens(1);
            syncCh->setPersistentAndFinalTokenNames("t_" + syncName);

            syncCh->connectSrc(createPort(a.get(), Port::PortType::Out, syncName, s, stdRate));
            syncCh->connectDst(createPort(a.get(), Port::PortType::In, syncName, s, stdRate));

            // Add self-edge with synchronization token on switch/select in detector scenario
            if (!dsg->hasChannel(syncCh->getName())) {
                ActorRef da = dsg->getActor(a->getName());
                std::unique_ptr<Channel> syncChDetect =
                        syncCh->createCopy(GraphComponent(dsg, static_cast<CId>(dsg->getChannels().size())));
                syncChDetect->setTokenSizeOfScenario(*ds, stdTokenSize);
                syncChDetect->connectSrc(
                        createPort(da, Port::PortType::Out, syncName, ds, stdRate));
                syncChDetect->connectDst(createPort(da, Port::PortType::In, syncName, ds, stdRate));
                dsg->addChannel(std::move(syncChDetect));
            }
        }
    }
}

/**
 * resetActorProperties ()
 * This function sets the execution time and memory sizes to  zero.
 */
void resetActorProperties(Scenario *s, Actor *a) {
    a->setExecutionTimeOfScenario(*s, a->getDefaultProcessorType(), 0);
    a->clearMemoriesOfScenario(*s, a->getDefaultProcessorType());
    auto code = std::make_unique<Actor::Memory>(MemoryType(MemoryType::Value::Instr), ".code", 0);
    auto data = std::make_unique<Actor::Memory>(MemoryType(MemoryType::Value::Data), ".data", 0);
    a->addMemoryForScenario(*s, a->getDefaultProcessorType(), std::move(code));
    a->addMemoryForScenario(*s, a->getDefaultProcessorType(), std::move(data));
    a->setDetectorActor(false);
}

/**
 * createPort ()
 * This function creates a port with given type, name and rate
 */
Port* createPort(Actor *a, Port::PortType t, const CString& n, Scenario *s, Rate r) {
    Port* p = a->createPort(t);
    p->setName(n + "_" + CString(a->getPorts().size() - 1));
    p->setRateOfScenario(*s, r);
    p->isolateScenario(*s);
    return p;
}

} // namespace FSMSADF
