/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 15, 2011
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      15-04-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_BINDING_AWARE_GRAPH_BINDING_AWARE_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_BINDING_AWARE_GRAPH_BINDING_AWARE_GRAPH_H_INCLUDED


#include "fsmsadf/resource_allocation/flow/base_platform/binding_aware_graph/binding_aware_graph.h"
#include "fsmsadf/resource_allocation/platform_binding/graph.h"

namespace FSMSADF
{

    /**
     * BindingAwareGraph ()
     * Binding-aware Graph.
     */
    class BindingAwareGraphCompSoCPlatform : public BindingAwareGraphBase
    {

        public:
            // Constructor
            explicit BindingAwareGraphCompSoCPlatform(const PlatformBinding *pb);

            // Destructor
            ~BindingAwareGraphCompSoCPlatform() override;

            BindingAwareGraphCompSoCPlatform(const BindingAwareGraphCompSoCPlatform&) = delete;
            BindingAwareGraphCompSoCPlatform& operator=(const BindingAwareGraphCompSoCPlatform&) = delete;
            BindingAwareGraphCompSoCPlatform(BindingAwareGraphCompSoCPlatform&&) = delete;
            BindingAwareGraphCompSoCPlatform& operator=(BindingAwareGraphCompSoCPlatform&&) = delete;

        protected:
            // Transform application graph to include binding decisions
            bool hasNonZeroNonWcActors(ScenarioGraph *sg, Scenario *s, ActorRefs wcActors);
            void initWorstCaseActors() override;
            void modelActorToTileBinding(Actor *a, Processor *p, Scenario *s) override;
            void modelChannelToConnectionBinding(Channel *c,
                                                 ChannelBindingConstraint *cbc, Connection *co, Scenario *s) override;
            void insertTileToTileConnectionModel(Channel *c,
                                                 ChannelBindingConstraint *cbc, Connection *co, Scenario *s);
            void insertSharedMemoryConnectionModel(Channel *c,
                                                 ChannelBindingConstraint *cbc, Connection *co, Scenario *s);
            void modelChannelToTileBinding(Channel *c,
                                                 ChannelBindingConstraint *bc, Scenario *s) override;

            void interModelOrdering(Scenario *s, ScenarioGraph *sg) override;
            static void enforceDmaOrder(Scenario *s, ScenarioGraph *sg, Actor *a, std::list <CString>& dmaActorNames);
            void modelStaticOrderScheduleOnProcessor(Scenario *s) override;
            StaticOrderSchedule *addDmaOrdering(Scenario *s, ScenarioGraph *sg);
            static void removeSwitchesAndSelects(StaticOrderSchedule *sos);
            static std::unique_ptr<StaticOrderSchedule> extractDmaSchedule(ScenarioGraph *sg, StaticOrderSchedule& sos);
    };

} // namespace FSMSADF

#endif

