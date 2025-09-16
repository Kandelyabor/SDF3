/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 27, 2008
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      27-02-08    :   Initial version.
 *
 * $Id: binding_aware_sdfg.h,v 1.3 2008-03-06 13:59:05 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_BINDING_AWARE_SDFG_BINDING_AWARE_SDFG_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_BINDING_AWARE_SDFG_BINDING_AWARE_SDFG_H_INCLUDED

#include "sdf/resource_allocation/flow/types.h"
#include "sdf/base/timed/graph.h"
#include "sdf/resource_allocation/mpsoc_arch/graph.h"


namespace SDF
{
    /**
     * BindingAwareSDFG ()
     * Binding-aware SDFG.
     */
    class BindingAwareSDFG : public TimedSDFgraph
    {
        public:
            // Constructor
            BindingAwareSDFG(TimedSDFgraph *constrainedSDFG,
                             PlatformGraph *platformGraph, SDFflowType flowType);

            // Destructor
            ~BindingAwareSDFG() override = default;

            // Delete copy/move constructors and assignment operators
            BindingAwareSDFG(const BindingAwareSDFG&) = delete;
            BindingAwareSDFG& operator=(const BindingAwareSDFG&) = delete;
            BindingAwareSDFG(BindingAwareSDFG&&) = delete;
            BindingAwareSDFG& operator=(BindingAwareSDFG&&) = delete;

            // Binding of actor to tile
            [[nodiscard]] CId getBindingOfActorToTile(const SDFactor& a) const
            {
                if (a.getId() >= actorBinding.size()) {
                    return ACTOR_NOT_BOUND;
                }
                return actorBinding[a.getId()];
            };

            // Binding of channels to connections
            [[nodiscard]] CId getBindingOfChannelToConnection(const SDFchannel& c) const
            {
                if (c.getId() >= channelBinding.size()) {
                    return CHANNEL_NOT_BOUND;
                }
                return channelBinding[c.getId()];
            };

            // Static-order schedule on processor in tile
            [[nodiscard]] const StaticOrderSchedule &getScheduleOnTile(CId tileId) const
            {
                return schedule.at(tileId);
            };

            // Static-order schedule on processor in tile
            [[nodiscard]] StaticOrderSchedule &getScheduleOnTile(CId tileId) {
                return schedule.at(tileId);
            };

            // Static-order schedule on processor in tile
            [[nodiscard]] const StaticOrderSchedule &getConstScheduleOnTile(CId tileId) const
            {
                return schedule.at(tileId);
            };

            void setScheduleOnTile(CId tileId, StaticOrderSchedule &s)
            {
                schedule[tileId] = s;
            };

            // TDMA scheduler setting on processor in tile
            [[nodiscard]] SDFdiscreteTime getTDMAsizeOnTile(CId tileId) const
            {
                return tdmaSize[tileId];
            };
            [[nodiscard]] SDFdiscreteTime getTDMAsliceOnTile(CId tileId) const
            {
                return tdmaSlice[tileId];
            };

            // Number of tiles in the platform graph
            [[nodiscard]] uint nrTilesInPlatformGraph() const
            {
                return nrTiles;
            };

        private:
            // Extraction of actor and channel related binding and scheduling properties
            void extractActorMapping(TimedSDFgraph *constrainedSDFG,
                                     PlatformGraph *platformGraph);
            void extractChannelMapping(TimedSDFgraph *constrainedSDFG,
                                       PlatformGraph *platformGraph);

            // Model binding of application to platform in an SDFG
            void constructBindingAwareSDFG(TimedSDFgraph *constrainedSDFG,
                                           PlatformGraph *platformGraph, SDFflowType flowType);

            // The actual modeling functions for the NSoC flow
            static void createMappedActorNSoC(TimedSDFactor *a, Tile *t);
            static void createMappedChannelToTileNSoC(TimedSDFchannel *c, Tile *t);
            static void createMappedChannelToConnectionNSoC(TimedSDFchannel *ch, Connection *cn);
            void modelBindingInNSoCFlow(TimedSDFgraph *g, PlatformGraph *pg);

            // The actual modeling functions for the MPARM flow
            static SDFtime getLatencyAMBAbusSemaphore();
            static SDFtime computeLatencyAMBAbus(CSize tokenSize);
            static void createMappedActorMPFlow(TimedSDFactor *a, Tile *t);
            static void createMappedChannelToTileMPFlow(TimedSDFchannel *c, Tile *t);
            void createMappedChannelToConnectionMPFlow(TimedSDFchannel *ch, Connection *cn);
            void modelBindingInMPFlow(TimedSDFgraph *g, PlatformGraph *pg);

            // Binding of channels to connections
            std::vector< CId > channelBinding;

            // Binding of actors to tiles
            std::vector< CId > actorBinding;

            // Number of tiles
            uint nrTiles;

            // Static-order schedules
            std::vector< StaticOrderSchedule > schedule;

            // Timewheels and slices
            std::vector< SDFdiscreteTime > tdmaSize;
            std::vector< SDFdiscreteTime > tdmaSlice;
    };
}//namespace SDF
#endif

