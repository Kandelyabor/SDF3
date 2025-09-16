/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   nocmapping.h
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
 * $Id: nocmapping.h,v 1.1 2008-03-20 16:16:19 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_MAPPING_NOCMAPPING_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_MAPPING_NOCMAPPING_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "sdf/resource_allocation/flow/types.h"
#include "sdf/resource_allocation/mpsoc_arch/graph.h"
#include "sdf/resource_allocation/noc_allocation/scheduler/noc_scheduler.h"


namespace SDF {
/**
 * NoCMapping
 * Container class NoC-based interconnect scheduling.
 */
class NoCMapping {
public:
    // Constructor
    NoCMapping(NoCScheduler *schedulingAlgo, SDFflowType type);

    // Destructor
    ~NoCMapping();

    // Delete copy and move constructors and assignment operators
    NoCMapping(const NoCMapping&) = delete;
    NoCMapping& operator=(const NoCMapping&) = delete;
    NoCMapping(NoCMapping&&) = delete;
    NoCMapping& operator=(NoCMapping&&) = delete;

    // NoC routing and scheduling algorithms
    bool extractCommunicationConstraints();
    bool scheduleCommunication();
    bool updateBandwidthAllocations();

    // Initialization
    void init(TimedSDFgraph* app, PlatformGraph* platform, CNode *archGraph, CNode *systemUsage);

    // Communication scheduling problem (messages)
    CNode *getMessages() { return xmlMessagesSet; };
    [[nodiscard]] SetOfNoCScheduleProblems *getSetOfNoCScheduleProblems() const {
        return scheduleProblems.get();
    };

    // Mapping results
    CNode *createNetworkNode();
    CNode *createMappingNode();
    CNode *createUsageNode();
    CNode *addUsageNode(CNode *systemUsageNode);

    // Slot-table size
    uint getSlotTableSize();

    // Flow
    [[nodiscard]] SDFflowType getFlowType() const { return flowType; };

private:
    // Application graph
    TimedSDFgraph* getAppGraph() { return appGraph; };
    void setAppGraph(TimedSDFgraph* g) { appGraph = g; };

    // Platform graph
    PlatformGraph* getPlatformGraph() { return platformGraph; };
    void setPlatformGraph(PlatformGraph* g) { platformGraph = g; };

    // Network node in (XML) architecture graph
    CNode *getNetworkNode();

    // Flow
    SDFflowType flowType;

    // Scheduling algorithm
    NoCScheduler *schedulingAlgo;

    // Communication scheduling problem
    CNode *xmlMessagesSet;
    std::unique_ptr<SetOfNoCScheduleProblems> scheduleProblems;

    // Application graph
    TimedSDFgraph* appGraph;

    // Platform graph
    PlatformGraph* platformGraph;

    // XML description of the architecture
    CNode *xmlArchGraph;
    CNode *xmlSystemUsage;

    // Occupied bandwidth in NIs according to schedule
    std::unique_ptr<double[]> occupiedInBandwidthTile;
    std::unique_ptr<double[]> occupiedOutBandwidthTile;
};
} // namespace SDF
#endif
