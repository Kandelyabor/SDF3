/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.h
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
 * $Id: flow.h,v 1.5 2008-03-20 16:16:18 sander Exp $
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

#ifndef SDF_FLOW_FLOW_H_INCLUDED
#define SDF_FLOW_FLOW_H_INCLUDED

#include "sdf/resource_allocation/tile_allocation/binding.h"
#include "sdf/resource_allocation/noc_allocation/mapping/nocmapping.h"
#include <memory>
#include <utility>
namespace SDF
{
    /**
     * Redefine names of Tile binding and NoC scheduling to get meaningful
     * names in the mapping flow.
     */
    using TileMapping = Binding;

    /**
     * SDFG mapping to MP-SoC flow
     */
    class SDF3Flow
    {
        public:
            // State of the mapping flow
            enum class FlowState { FlowStart,
                             FlowModelNonLocalMemory,
                             FlowComputeStorageDist,
                             FlowSelectStorageDist,
                             FlowEstimateStorageDist,
                             FlowEstimateLatencyConstraint,
                             FlowEstimateBandwidthConstraint,
                             FlowBindSDFGtoTile,
                             FlowStaticOrderScheduleTiles,
                             FlowAllocateTDMAtimeSlices,
                             FlowOptimizeStorageSpaceAllocations,
                             FlowExtractCommunicationConstraints,
                             FlowScheduleCommunication,
                             FlowUpdateBandwidthAllocations,
                             FlowCompleted,
                             FlowFailed
                           };


            // Constructor
            SDF3Flow(SDFflowType type, CNode *xmlAppGraph, CNode *xmlArchGraph,
                     CNode *xmlSystemUsage);

            // Destructor
            ~SDF3Flow();

            // Delete copy/move constructors and assignment operators
            SDF3Flow(const SDF3Flow&) = delete;
            SDF3Flow& operator=(const SDF3Flow&) = delete;
            SDF3Flow(SDF3Flow&&) = delete;
            SDF3Flow& operator=(SDF3Flow&&) = delete;

            // Flow type
            [[nodiscard]] SDFflowType getFlowType() const
            {
                return flowType;
            };

            // Execute design flow
            FlowState run();

            // Run flow step-by-step
            [[nodiscard]] bool getStepMode() const
            {
                return stepMode;
            };
            void setStepMode(bool flag)
            {
                stepMode = flag;
            };

            // Settings for tile binding and scheduling phase
            void setTileMappingAlgo(std::unique_ptr<TileMapping> a)
            {
                tileMapping = std::move(a);
            };

            // Settings for NoC routing and scheduling phase
            void setNoCMappingAlgo(std::unique_ptr<NoCMapping> a)
            {
                nocMapping = std::move(a);
            };

            // Mapping results
            CNode *createSDF3Node();
            void outputMappingAsXML(std::ostream &out);
            void outputMappingAsHTML();

        private:
            // Application graph
            TimedSDFgraph* getAppGraph()
            {
                return appGraph.get();
            };
            static std::unique_ptr<TimedSDFgraph> createAppGraph(CNode *xmlAppGraph);

            // Platform graph
            PlatformGraph* getPlatformGraph()
            {
                return platformGraph.get();
            };
            static std::unique_ptr<PlatformGraph> createPlatformGraph(CNode *xmlPlatformGraph);
            static void setSystemUsagePlatformGraph(PlatformGraph* g, CNode *systemUsage);

            // State of the design flow
            void setNextStateOfFlow(FlowState s)
            {
                stateOfFlow = s;
            };
            FlowState getStateOfFlow()
            {
                return stateOfFlow;
            };


            // Steps of the design flow
            void checkInputDesignFlow();
            void modelNonLocalMemoryAccesses();
            void computeStorageDistributions();
            void selectStorageDistribution();
            void estimateStorageConstraints();
            void estimateLatencyConstraints();
            void estimateBandwidthConstraints();
            void bindSDFGtoTiles();
            void constructStaticOrderScheduleTiles();
            void allocateTDMAtimeSlices();
            void optimizeStorageSpaceAllocations();
            void extractCommunicationConstraints();
            void scheduleCommunication();
            void updateBandwidthAllocations();

            // User interaction when running flow in step-by-step mode
            void handleUserInteraction();

            // Storage distributions in XML format
            CNode *createStorageDistributionsNode(
                const StorageDistributionSet& distributions);


            // Flow type
            SDFflowType flowType;

            // Run flow step-by-step
            bool stepMode;

            // XML description of the application and architecture
            CNode *xmlAppGraph;
            CNode *xmlArchGraph;
            CNode *xmlSystemUsage;

            // Application graph
            std::unique_ptr<TimedSDFgraph> appGraph;

            // Platform graph
            std::unique_ptr<PlatformGraph> platformGraph;

            // Tile binding and scheduling object
            std::unique_ptr<TileMapping> tileMapping;

            // NoC routing and scheduling object
            std::unique_ptr<NoCMapping> nocMapping;

            // State of the design flow
            FlowState stateOfFlow;

            // Storage-space / throughput exploration algorithm
            SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;

            // Minimal storage distributions
            StorageDistributionSet* minStorageDistributions;
            StorageDistributionSet* selectedStorageDistributionSet;
            StorageDistribution* selectedStorageDistribution;
    };
}//namespace SDF
#endif
