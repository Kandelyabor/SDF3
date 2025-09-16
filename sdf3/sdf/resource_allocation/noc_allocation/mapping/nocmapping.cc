/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   nocmapping.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *
 * $Id: nocmapping.cc,v 1.1.2.1 2010-04-25 00:31:44 mgeilen Exp $
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

#include "nocmapping.h"
#include "sdf/analysis/analysis.h" // IWYU pragma: keep
#include <memory>
namespace SDF {
/**
 * NoCMapping ()
 * Constructor.
 */
NoCMapping::NoCMapping(NoCScheduler *schedulingAlgo, SDFflowType type) :
    flowType(type),
    schedulingAlgo(schedulingAlgo),
    xmlMessagesSet(nullptr),
    scheduleProblems(nullptr),
    appGraph(nullptr),
    platformGraph(nullptr),
    xmlArchGraph(nullptr),
    xmlSystemUsage(nullptr),
    occupiedInBandwidthTile(nullptr),
    occupiedOutBandwidthTile(nullptr) {}

/**
 * ~NoCMapping ()
 * Destructor.
 */
NoCMapping::~NoCMapping() = default;

/**
 * init ()
 * The function initializes all data structures required for the NoC mapping.
 * This function must be called prior to any of the actual NoC mapping
 * functions.
 */
void NoCMapping::init(TimedSDFgraph* app,
                      PlatformGraph* platform,
                      CNode *archGraph,
                      CNode *systemUsage) {
    // XML description of the architecture
    xmlArchGraph = archGraph;
    xmlSystemUsage = systemUsage;

    // Application graph
    setAppGraph(app);

    // Platform graph
    setPlatformGraph(platform);
}

/**
 * createNetworkNode ()
 * Create an XML node which describes the structure of the interconnect graph.
 */
CNode *NoCMapping::createNetworkNode() {
    // MPFlow has no NoC interconnect
    if (getFlowType() == SDFflowType::SDFflowTypeMPFlow) {
        return nullptr;
    }

    // Create networkNode
    InterconnectGraph g(xmlArchGraph, 1);
    CNode *networkNode = g.createInterconnectGraphNode();

    return networkNode;
}

/**
 * createMappingNode ()
 * Create an XML node which describes the mapping of messages send over
 * the interconnect to the resources in the interconnect.
 */
CNode *NoCMapping::createMappingNode() {
    if (scheduleProblems == nullptr || scheduleProblems->nrScheduleProblems() == 0) {
        return nullptr;
    }

    return scheduleProblems->createNetworkMappingNode();
}

/**
 * createUsageNode ()
 * Create an XML node which describes the usage of the links in the
 * interconnect.
 */
CNode *NoCMapping::createUsageNode() {
    if (scheduleProblems == nullptr || scheduleProblems->nrScheduleProblems() == 0) {
        return nullptr;
    }

    return scheduleProblems->createNetworkUsageNode();
}

/**
 * addUsageNode ()
 * Create an XML node which describes the usage of the links in the
 * interconnect. This node is added to the systemUsageNode supplied to
 * the function. The systemUsageNode is also returned by the function.
 * The function also updates the used incoming and outgoing bandwidth
 * of the NIs inside the tiles based on the communication schedule.
 */
CNode *NoCMapping::addUsageNode(CNode *systemUsageNode) {
    // Add a node describing the network usage
    CAddNode(systemUsageNode, createUsageNode());

    // Bandwidth usage not computed?
    if (occupiedInBandwidthTile == nullptr || occupiedOutBandwidthTile == nullptr) {
        return systemUsageNode;
    }

    // Update the bandwidth usage of the tiles
    for (CNode *tileNode = CGetChildNode(systemUsageNode, "tile"); tileNode != nullptr;
         tileNode = CNextNode(tileNode, "tile")) {
        // Find corresponding tile in the platform graph
        Tile *t = platformGraph->getTile(CGetAttribute(tileNode, "name"));

        // Update bandwidth inside network interface node
        CNode *niNode = CGetChildNode(tileNode, "networkInterface");
        CSetAttribute(niNode, "inBandwidth", CString(occupiedInBandwidthTile[t->getId()]));
        CSetAttribute(niNode, "outBandwidth", CString(occupiedOutBandwidthTile[t->getId()]));
    }

    return systemUsageNode;
}

/**
 * getNetworkNode ()
 * The function return the network node from the XML architecture description.
 */
CNode *NoCMapping::getNetworkNode() {
    if (xmlArchGraph != nullptr && CHasChildNode(xmlArchGraph, "network")) {
        return CGetChildNode(xmlArchGraph, "network");
    }

    return nullptr;
}

/**
 * getSlotTableSize ()
 * The function returns the slot-table size used in the NoC.
 */
uint NoCMapping::getSlotTableSize() {
    CNode *networkNode = getNetworkNode();

    if (networkNode != nullptr && CHasAttribute(networkNode, "slotTableSize")) {
        return static_cast<uint>(CGetAttribute(networkNode, "slotTableSize"));
    }

    return 0;
}

/**
 * extractCommunicationConstraints ()
 * Extract communication scheduling problem from the bound and scheduled
 * application SDFG.
 */
bool NoCMapping::extractCommunicationConstraints() {
    SDFstateSpaceTraceInterconnectCommunication traceCommunication;

    if (getAppGraph() == nullptr || getPlatformGraph() == nullptr || getSlotTableSize() == 0) {
        throw CException("[ERROR] NoCMapping must be initialized first.");
    }

    // Cleanup existing message when present
    if (xmlMessagesSet != nullptr) {
        CRemoveNode(xmlMessagesSet);
    }

    // Create a binding-aware SDFG
    auto bindingAwareSDFG = std::make_unique<BindingAwareSDFG>(getAppGraph(), getPlatformGraph(), getFlowType());

    // Extract communication constraints from the application
    xmlMessagesSet = SDFstateSpaceTraceInterconnectCommunication::trace(
            bindingAwareSDFG.get(), getPlatformGraph(), getSlotTableSize());

    return true;
}

/**
 * scheduleCommunication ()
 * The function tries to find a valid scheduling function for the scheduling
 * problem.
 */
bool NoCMapping::scheduleCommunication() {
    bool foundSchedule = false;

    if (xmlMessagesSet == nullptr) {
        throw CException("[ERROR] NoCMapping has no messages to be scheduled.");
    }

    // Create a set of scheduling problems
    scheduleProblems = std::make_unique<SetOfNoCScheduleProblems>(xmlMessagesSet, xmlArchGraph, xmlSystemUsage);

    // Try to solve all scheduling problems
    foundSchedule = schedulingAlgo->schedule(*scheduleProblems);

    // No schedule found?
    if (!foundSchedule) {
        //  Cleanup all existing scheduling problems
        scheduleProblems->clear();
    }

    return foundSchedule;
}

/**
 * updateBandwidthAllocations ()
 * The function updates the bandwidth allocations made in the platform graph
 * based on the actual requirements as given by the NoC scheduling function.
 */
bool NoCMapping::updateBandwidthAllocations() {

    // No scheduling problems available?
    if (scheduleProblems == nullptr) {
        return true;
    }

    // Create interconnect graph
    InterconnectGraph interconnectGraph(xmlArchGraph, 1);

    // Flit size and slot table size
    double flitSize = interconnectGraph.getFlitSize();
    double slotTableSize = interconnectGraph.getSlotTableSize();

    // Allocate memory space to store occupied in/out bandwidth of tiles
    occupiedInBandwidthTile = std::make_unique<double[]>(static_cast<unsigned int>(platformGraph->nrTiles()));
    occupiedOutBandwidthTile = std::make_unique<double[]>(static_cast<unsigned int>(platformGraph->nrTiles()));
    for (uint i = 0; i < platformGraph->nrTiles(); i++) {
        occupiedInBandwidthTile[i] = 0;
        occupiedOutBandwidthTile[i] = 0;
    }

    // Compute occupied bandwidth for every link entering and leaving a tile
    for (const auto& t : platformGraph->getTiles()) {

        // Assume all slots free
        SlotReservations slotReservations;
        slotReservations.resize(static_cast<uint>(slotTableSize));
        for (uint i = 0; i < slotTableSize; i++) {
            slotReservations[i] = false;
        }
        double nrSlotsUsed = 0;

        // Find all slots used in at least one schedule
        for (auto iter = scheduleProblems->scheduleProblemsBegin();
             iter != scheduleProblems->scheduleProblemsEnd();
             iter++) {
            NoCScheduleProblem& p = *(*iter);

            // Find corresponding node in the interconnect graph
            Node *n = p.getInterconnectGraph()->getNode(t->getName());

            for (auto iterL = n->incomingLinksBegin(); iterL != n->incomingLinksEnd(); iterL++) {
                Link *l = *iterL;

                for (auto iterS = l->slotTableSeqBegin(); iterS != l->slotTableSeqEnd(); iterS++) {
                    SlotTable &s = *iterS;

                    for (uint i = 0; i < slotTableSize; i++) {
                        if (!s.isSlotFree(i)) {
                            slotReservations[i] = true;
                        }
                    }
                }
            }
        }

        // Count number of occupied slots
        for (uint i = 0; i < slotTableSize; i++) {
            if (slotReservations[i]) {
                nrSlotsUsed++;
            }
        }

        // Update outgoing bandwidth usage of NI inside tile
        double usedBw = flitSize * (nrSlotsUsed / slotTableSize);
        occupiedInBandwidthTile[t->getId()] = usedBw;

        logMsg("Incoming bandwidth tile '" + t->getName() + "': " + CString(usedBw)
               + " bits/time-unit.");

        // Assume all slots free
        slotReservations.resize(static_cast<uint>(slotTableSize));
        for (uint i = 0; i < slotTableSize; i++) {
            slotReservations[i] = false;
}
        nrSlotsUsed = 0;

        // Find all slots used in at least one schedule
        for (auto iter = scheduleProblems->scheduleProblemsBegin();
             iter != scheduleProblems->scheduleProblemsEnd();
             iter++) {
            NoCScheduleProblem& p = *(*iter);

            // Find corresponding node in the interconnect graph
            Node *n = p.getInterconnectGraph()->getNode(t->getName());

            for (auto iterL = n->outgoingLinksBegin(); iterL != n->outgoingLinksEnd();
                 iterL++) {
                Link *l = *iterL;

                for (auto iterS = l->slotTableSeqBegin(); iterS != l->slotTableSeqEnd();
                     iterS++) {
                    SlotTable &s = *iterS;

                    for (uint i = 0; i < slotTableSize; i++) {
                        if (!s.isSlotFree(i)) {
                            slotReservations[i] = true;
}
                    }
                }
            }
        }

        // Count number of occupied slots
        for (uint i = 0; i < slotTableSize; i++) {
            if (slotReservations[i]) {
                nrSlotsUsed++;
}
}

        // Update outgoing bandwidth usage of NI inside tile
        usedBw = flitSize * (nrSlotsUsed / slotTableSize);
        occupiedOutBandwidthTile[t->getId()] = usedBw;

        logMsg("Outgoing bandwidth tile '" + t->getName() + "': " + CString(usedBw)
               + " bits/time-unit.");
    }

    return true;
}

} // namespace SDF