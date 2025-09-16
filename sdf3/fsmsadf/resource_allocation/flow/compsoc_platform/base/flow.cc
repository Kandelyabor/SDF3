/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.cc
 *
 *  Author          :   Martijn Koedam (M.L.P.J.Koedam@tue.nl)
 *
 *  Date            :   April 14, 2011
 *
 *  Function        :   Base Implementation of SDF3Flow
 *
 *  History         :
 *       14-4-11    :   Initial version.
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

#include "flow.h"
#include "fsmsadf/resource_allocation/flow/compsoc_platform/memory/memory.h"
#include "fsmsadf/resource_allocation/flow/compsoc_platform/tile/binding.h"
#include "fsmsadf/resource_allocation/platform_binding/constraint.h"
#include "fsmsadf/transform/create-detector/create-detector.h"


namespace FSMSADF {

/**
 * SDF3FlowDummyPlatform ()
 * Constructor.
 */
SDF3FlowCompSoCPlatform::SDF3FlowCompSoCPlatform() {
    // Create a new memory dimensioning algorithm
    setMemoryDimAlgo(std::make_unique<MemoryDimAlgoCompSoCPlatform>());

    // Create a new tile binding algorithm
    setTileBindingAlgo(std::make_unique<TileBindingAlgoCompSoCPlatform>());
}

/**
 * ~SDF3CompSoCPlatform ()
 * Destructor.
 */
SDF3FlowCompSoCPlatform::~SDF3FlowCompSoCPlatform() = default;

/**
 * platformAwareTransform ()
 * Isolate the detector actor in a separate scenario.
 */
void SDF3FlowCompSoCPlatform::platformAwareTransform() {
    Graph* ag = getApplicationGraph();

    // Transform graph: extract detector actor in separate scenario, with switch actor to other
    // scenarios
    createDetectorScenario(ag->getFSM(), ag->getScenarios(), ag->getScenarioGraphs());
    Scenario *ds = getDetectorScenario();
    moveDetectorConstraints(ds);

    ProcessorRefs allProc;
    getAllProcessors(allProc);
    addLoadScheduleActors(allProc, ds);
    setSystemCosts(allProc, ds);
    addClusterBindingConstraints(ds);
}

/**
 * moveDetectorConstraints()
 * Find all initial constraints that belong to an actor or channel
 * that is now in the detector scenario, and move it there.
 */
void SDF3FlowCompSoCPlatform::moveDetectorConstraints(Scenario *ds) {
    Scenarios *scenarios = &(getApplicationGraph()->getScenarios());
    PlatformBindings* platformBindings = getPlatformBindings();

    // There should only be the initial platformBinding
    if (platformBindings->size() != 1) {
        throw CException("Expected 1 platformBindings, found " + CString(platformBindings->size()));
    }
    PlatformBinding* ipb = platformBindings->begin()->get();

    ScenarioBindingConstraints *dsBc =
            ipb->getGraphBindingConstraints()->getConstraintsOfScenario(*ds);
    (void)dsBc->getAllChannelBindingConstraints();

    for (const auto& s : *scenarios) {
        if (s.get() == ds) {
            continue;
        }

        ScenarioBindingConstraints *sbc =
                ipb->getGraphBindingConstraints()->getConstraintsOfScenario(*s);
        ChannelBindingConstraints *cbc = sbc->getAllChannelBindingConstraints();
        std::list<CString> to_be_removed;

        for (auto& [channelName, constraint] : *cbc) {

            if (!(s->getScenarioGraph()->hasChannel(channelName))
                && (s->getScenarioGraph()->hasChannelStartingWith(channelName))) {
                ChannelRef newCh = s->getScenarioGraph()->getChannelStartingWith(channelName);

                to_be_removed.push_back(channelName);

                // Add constraint for new channel
                sbc->setConstraintsOfChannel(newCh->getName(), constraint->clone(newCh->getName()));
                sbc->getConstraintsOfChannel(newCh->getName())->setChannelName(newCh->getName());
            }

            if (ds->getScenarioGraph()->hasChannel(channelName)) {
                to_be_removed.push_back(channelName);
                dsBc->setConstraintsOfChannel(channelName, constraint->clone(channelName));
            } else if (ds->getScenarioGraph()->hasChannelStartingWith("switch_" + channelName)) {
                ChannelRef dataCh =
                        ds->getScenarioGraph()->getChannel("switch_" + channelName + "_data");
                ChannelRef ctrlCh =
                        ds->getScenarioGraph()->getChannel("switch_" + channelName + "_ctrl");

                to_be_removed.push_back(channelName);
                // Add constraint for data channel
                dsBc->setConstraintsOfChannel(dataCh->getName(),
                                              constraint->clone(dataCh->getName()));
                dsBc->getConstraintsOfChannel(dataCh->getName())->setChannelName(dataCh->getName());
                // Add constraint for control channel
                dsBc->setConstraintsOfChannel(ctrlCh->getName(),
                                              constraint->clone(ctrlCh->getName()));
                dsBc->getConstraintsOfChannel(ctrlCh->getName())->setChannelName(ctrlCh->getName());
            } else if (ds->getScenarioGraph()->hasChannelStartingWith("select_" + channelName)) {
                ChannelRef dataCh =
                        ds->getScenarioGraph()->getChannel("select_" + channelName + "_data");
                ChannelRef ctrlCh =
                        ds->getScenarioGraph()->getChannel("select_" + channelName + "_ctrl");

                to_be_removed.push_back(channelName);
                // Add constraint for data channel
                dsBc->setConstraintsOfChannel(dataCh->getName(),
                                              constraint->clone(dataCh->getName()));
                dsBc->getConstraintsOfChannel(dataCh->getName())->setChannelName(dataCh->getName());
                // Add constraint for control channel
                dsBc->setConstraintsOfChannel(ctrlCh->getName(),
                                              constraint->clone(ctrlCh->getName()));
                dsBc->getConstraintsOfChannel(ctrlCh->getName())->setChannelName(ctrlCh->getName());
            }
        }

        for (const auto &j : to_be_removed) {
            sbc->removeConstraintsOfChannel(j);
        }
    }
}

/**
 * getAllProcessors()
 * Compile list of all processors on all tiles.
 */
void SDF3FlowCompSoCPlatform::getAllProcessors(ProcessorRefs& ap) {
    for (const auto& t : getPlatformGraph()->getTiles()) {
        for (const auto& j : t->getProcessors()) {
            ap.push_back(j.get());
        }
    }
    if (ap.empty()) {
        throw CException("Error: no processors found.");
    }
}

/**
 * getDetectorScenario()
 * Find the detector scenario.
 */
Scenario *SDF3FlowCompSoCPlatform::getDetectorScenario() {
    Scenarios *scenarios = &(getApplicationGraph()->getScenarios());

    for (const auto& s : *scenarios) {
        if (s->getName() == "detector") {
            return s.get();
        }
    }
    throw CException("Error: no detector scenario found.");
}

/**
 * addLoadScheduleActors()
 * Add a load-schedule actor for each processor to the 'detector' scenario.
 * Add a control channel from the detector actor to each of these loadSched actors.
 * Add a channel to enforce the scheduling from the first loadSched actor to all switches.
 * Set the memory and execution time of the loadSched actors.
 */
void SDF3FlowCompSoCPlatform::addLoadScheduleActors(ProcessorRefs& ap, Scenario *ds) {
    CString detectorName = getApplicationGraph()->getFSM()->getDetectorName();

    ScenarioGraph *sg = ds->getScenarioGraph();
    ActorRef detector = sg->getActor(detectorName);

    for (auto i = ap.begin(); i != ap.end(); i++) {
        Processor *p = (*i);
        SystemCost *pCost = p->getSystemCost("initial");

        // Create loadSched actor for each processor
        std::unique_ptr<Actor> loadSched =
                detector->createCopy(GraphComponent(sg, static_cast<CId>(sg->getActors().size())));

        uint procNr = static_cast<uint>(std::distance(ap.begin(), i));
        CString loadSchedName = "loadSched" + CString(procNr);
        loadSched->setName(loadSchedName);
        loadSched->setType(loadSchedName);
        loadSched->setDetectorActor(false);
        loadSched->isolateScenario(*ds);

        // Add execution time and memory from systemCost struct
        loadSched->setExecutionTimeOfScenario(*ds, p->getType(), pCost->getLoadScheduleTime());
        loadSched->clearMemoriesOfScenario(*ds, p->getType());
        auto iMem = std::make_shared<Actor::Memory>(
                MemoryType(MemoryType::Value::Instr), ".code", pCost->getLoadScheduleInstrMem());
        loadSched->addMemoryForScenario(*ds, p->getType(),
        std::move(iMem));
        auto dmem = std::make_shared<Actor::Memory>(
                MemoryType(MemoryType::Value::Data), ".data", pCost->getLoadScheduleDataMem());
        loadSched->addMemoryForScenario(*ds, p->getType(), std::move(dmem));

        auto* loadSchedPtr = loadSched.get();
        sg->addActor(std::move(loadSched));

        // Add control channel from detector to loadSched actor
        ChannelRef controlCh =
                sg->createChannel(GraphComponent(sg, static_cast<CId>(sg->getChannels().size())));
        controlCh->isolateScenario(*ds);
        CString controlChName = "detect_loadSched" + CString(procNr) + "_ctrl";
        controlCh->setName(controlChName);
        controlCh->setTokenSizeOfScenario(*ds, stdTokenSize);

        controlCh->connectSrc(
                createPort(detector, Port::PortType::Out, controlChName, ds, stdRate));
        controlCh->connectDst(
                createPort(loadSchedPtr, Port::PortType::In, controlChName, ds, stdRate));

        if (procNr == 0) {
            for (const auto& a : sg->getActors()) {
                // Add channel from first loadSched actor to each switch to enforce scheduling
                if (a->getName().compare(0, 7, "switch_") == 0) { // NOLINT(*magic-numbers)
                    ChannelRef schedCh = sg->createChannel(
                            GraphComponent(sg, static_cast<CId>(sg->getChannels().size())));
                    schedCh->isolateScenario(*ds);
                    CString schedChName = "loadSched" + CString(procNr) + "_" + a->getName();
                    schedCh->setName(schedChName);
                    schedCh->setTokenSizeOfScenario(*ds, stdTokenSize);

                    schedCh->connectSrc(
                            createPort(loadSchedPtr, Port::PortType::Out, schedChName, ds, stdRate));
                    schedCh->connectDst(
                            createPort(a.get(), Port::PortType::In, schedChName, ds, stdRate));
                }
            }
        }
    }
}

/**
 * setSystemCosts()
 * Model the switching costs by adding the platform-specific
 *  memory and execution time to the switch and select actors.
 */
void SDF3FlowCompSoCPlatform::setSystemCosts(ProcessorRefs &ap, Scenario *ds) {
    ScenarioGraph *sg = ds->getScenarioGraph();

    for (auto *p : ap) {
        SystemCost *pCost = p->getSystemCost("initial");

        for (const auto& a : sg->getActors()) {
            if (a->getName().compare(0, 7, "switch_") == 0) { // NOLINT(*magic-numbers)
                uint unitAmount = 0;
                // Iterate over the ports
                for (const auto& port : a->getPorts()) {
                    if (port->getType() == Port::PortType::Out) {
                        Channel *c = port->getChannel();
                        // Is it not a self-edge?
                        if (c->getDstActor() != a.get()) {
                            unitAmount +=
                                    port->getRateOfScenario(*ds) * c->getTokenSizeOfScenario(*ds);
                        }
                    }
                }
                Time execTime =
                        static_cast<Time>(pCost->getSwitchSelectTimeConst()
                                          + (pCost->getSwitchSelectTimePerUnit() * unitAmount));

                // Add execution time and memory costs to all switch actors
                a->setExecutionTimeOfScenario(*ds, p->getType(), execTime);
                a->clearMemoriesOfScenario(*ds, p->getType());
                auto iMem = std::make_shared<Actor::Memory>(MemoryType(MemoryType::Value::Instr),
                                               ".code",
                                               pCost->getSwitchSelectInstrMem());
                a->addMemoryForScenario(*ds, p->getType(), std::move(iMem));
                auto dmem = std::make_shared<Actor::Memory>(MemoryType(MemoryType::Value::Data),
                                               ".data",
                                               pCost->getSwitchSelectDataMem());
                a->addMemoryForScenario(*ds, p->getType(), std::move(dmem));
            }

            if (a->getName().compare(0, 7, "select_") == 0) { // NOLINT(*magic-numbers)
                uint unitAmount = 0;
                // Iterate over the ports
                for (const auto& port : a->getPorts()) {
                    if (port->getType() == Port::PortType::In) {
                        Channel *c = port->getChannel();
                        // Is it not a self-edge?
                        if (c->getDstActor() != a.get()) {
                            unitAmount +=
                                    port->getRateOfScenario(*ds) * c->getTokenSizeOfScenario(*ds);
                        }
                    }
                }
                Time execTime =
                        static_cast<Time>(pCost->getSwitchSelectTimeConst()
                                          + (pCost->getSwitchSelectTimePerUnit() * unitAmount));

                // Add execution time and memory costs to all select actors
                a->setExecutionTimeOfScenario(*ds, p->getType(), execTime);
                a->clearMemoriesOfScenario(*ds, p->getType());
                auto iMem= std::make_shared<Actor::Memory>(MemoryType(MemoryType::Value::Instr),
                                               ".code",
                                               pCost->getSwitchSelectInstrMem());
                a->addMemoryForScenario(*ds, p->getType(), std::move(iMem));
                auto dmem = std::make_shared<Actor::Memory>(MemoryType(MemoryType::Value::Data),
                                               ".data",
                                               pCost->getSwitchSelectDataMem());
                a->addMemoryForScenario(*ds, p->getType(), std::move(dmem));
            }
        }
    }
}

/**
 * addClusterBindingConstraints()
 * Add a constraint to bind all actors in the detector scenario to the same
 * processor, except the loadSchedule actors of which there must be
 * exactly one on each processor.
 */
void SDF3FlowCompSoCPlatform::addClusterBindingConstraints(Scenario *ds) {
    CString detectClusterName = "detector_" + getApplicationGraph()->getFSM()->getDetectorName();
    std::list<CString> exclClusters;
    exclClusters.push_back(detectClusterName);

    PlatformBindings* platformBindings = getPlatformBindings();
    // There should only be the initial platformBinding
    if (platformBindings->size() != 1) {
        throw CException("Expected 1 platformBindings, found " + CString(platformBindings->size()));
    }
    PlatformBinding* ipb = (platformBindings->begin())->get();

    // Create these constraints for each platformBinding
    ScenarioBindingConstraints *sbc =
            ipb->getGraphBindingConstraints()->getConstraintsOfScenario(*ds);
    ClusterBindingConstraints *detectBC = sbc->getClusterBindingConstraints(detectClusterName);

    for (const auto& a : ds->getScenarioGraph()->getActors()) {
        if (a->getName().compare(0, 9, "loadSched") == 0) { // NOLINT(*magic-numbers)
            if (a->getName().compare(0, 10, "loadSched0") == 0) { // NOLINT(*magic-numbers)
                // The first loadSched actor is constrained to the detector and switches/selects
                detectBC->addActorName(a->getName());
            } else
            // All others are excluded from the switches/selects and each other
            {
                // Create cluster constraint with just this actor
                CString exclClusterName = "excl_" + a->getName();
                ClusterBindingConstraints *exclBind =
                        sbc->getClusterBindingConstraints(exclClusterName);
                exclBind->addActorName(a->getName());
                exclClusters.push_back(exclClusterName);

                // Create exclusion constraint
                auto exclConstr = std::make_unique<ClusterExclusionConstraints>();
                auto* exclConstrPtr = exclConstr.get();
                sbc->addExclusionConstraint(std::move(exclConstr));

                // Exclude all clusters in list from each other
                for (const auto &l : exclClusters) {
                    exclConstrPtr->addClusterExclusionName(l);
                }
            }
        } else {
            // Add actor to detector cluster constraint
            detectBC->addActorName(a->getName());
        }
    }
}
} // End namespace FSMSADF
