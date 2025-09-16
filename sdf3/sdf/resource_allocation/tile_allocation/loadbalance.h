/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   loadbalance.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 11, 2006
 *
 *  Function        :   Load-balance algorithm
 *
 *  History         :
 *      11-04-06    :   Initial version.
 *
 * $Id: loadbalance.h,v 1.3 2008-03-06 10:49:45 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_TILE_ALLOCATION_LOADBALANCE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_TILE_ALLOCATION_LOADBALANCE_H_INCLUDED

#include "binding.h"
#include <memory>
namespace SDF
{


    /**
     * LoadBalanceBinding ()
     * Load balance resource assignment algorithm.
     */
    class LoadBalanceBinding : public Binding
    {
        public:
            // Constructor
            explicit LoadBalanceBinding(SDFflowType flowType);

            // Destructor
            ~LoadBalanceBinding() override;

            // Prevent copying and moving
            LoadBalanceBinding(const LoadBalanceBinding&) = delete;
            LoadBalanceBinding& operator=(const LoadBalanceBinding&) = delete;
            LoadBalanceBinding(LoadBalanceBinding&&) = delete;
            LoadBalanceBinding& operator=(LoadBalanceBinding&&) = delete;

            // Binding algorithm
            bool bind() override;
            bool bindSDFGtoTiles() override;
            bool constructStaticOrderScheduleTiles() override;
            bool allocateTDMAtimeSlices() override;
            bool optimizeStorageSpaceAllocations() override;

            // Binding check
            bool bindingCheck();

            // Constants used in tile cost function
            void getConstantsTileCostFunction(double &a, double &b,
                                              double &c, double &d, double &e, double &f, double &g, double &k,
                                              double &l, double &m, double &n, double &o, double &p, double &q) const;
            void setConstantsTileCostFunction(double a, double b,
                                              double c, double d, double e, double f, double g, double k, double l,
                                              double m, double n, double o, double p, double q);

            // Application graph (overload)
            void setAppGraph(TimedSDFgraph* g) override;

        private:
            // Check bindings
            [[nodiscard]] bool isActorBound(const SDFactor& a) const;
            [[nodiscard]] bool isChannelBound(const SDFchannel& c) const;
            [[nodiscard]] bool isChannelBoundToTile(const SDFchannel& c) const;
            [[nodiscard]] bool isChannelBoundToConnection(const SDFchannel& c) const;

            // Resource management
            void releaseResources() override;
            void releaseConnectionResources(const TimedSDFchannel& c);
            void releaseResources(const TimedSDFactor& a, const Tile &t);
            bool allocateConnectionResources(const TimedSDFchannel& c);
            bool allocateResources(const TimedSDFactor& a,const Tile& t);

            // Actor binding
            bool bindActorsToTiles();
            void optimizeActorToTileBindings();
            bool moveActorBinding(const TimedSDFactor& a, bool allowExistingTile);

            // Channel bandwidth allocation
            bool changeBandwidthAllocation(const TimedSDFchannel& c, double bw);

            // Static order scheduling
            void constructStaticOrderSchedules();
            void reconstructStaticOrderSchedules();

            // Time slice allocation
            static bool changeSlotAllocation(const Tile& , CSize sz);
            void reserveTimeSlices(double fraction);
            void releaseTimeSlices();
            bool minimizeTimeSlices(double step,  double minStep);
            void optimizeTimeSlices();
            bool optimizeTimeSlices(std::vector<CSize> minSlice, std::vector<CSize> maxSlice);

            // Storage space allocation
            void minimizeStorageSpace();
            void updateStorageSpaceAllocation(const TimedSDFgraph& mappedAppGraph,
                                              const std::unique_ptr<StorageDistribution>& d);

            // Resource estimation
            void estimateMaxCycleMean();
            double computeLoadOfChannelToConnectionBinding(const SDFactor& a, const Tile& );
            double bwChannelsMappedToInConnection(const SDFactor& a, const Tile& );
            double bwChannelsMappedToOutConnection(const SDFactor& a, const Tile& );
            CSize memLoadChannelsOnTile(const SDFactor& a, const Tile& );
            int nrChannelsMappedToConnection(const SDFactor& a, const Tile& );
            SDFactorRefs sortActorsOnCriticality();
            void sortTilesOnCommunicationOverhead(const TimedSDFactor& a, TileRefs &tiles);
            void sortTilesOnLoad(const TimedSDFactor& a, TileRefs &tiles,
                                 double const_a, double const_b, double const_c, double const_d,
                                 double const_e, double const_f, double const_g, double const_k,
                                 double const_l, double const_m, double const_n, double const_o,
                                 double const_p, double const_q);

            // Tile load
            void initTileLoad();
            double actorLoadOnTile(const TimedSDFactor& a, const Tile& t);
            void increaseLoadTile(const TimedSDFactor& a, const Tile& t);
            void decreaseLoadTile(const TimedSDFactor& a, const Tile& t);


            // Binding of actors to tiles
            ActorTileBinding actorTileBinding;

            // Allowed bindings of actors to tiles
            std::map<CId, TileCollection> actorTileBindingOptions;

            // Estimated cycle mean of actors
            std::unique_ptr<double[]> maxCycleMean;

            // Computation load of tiles
            std::unique_ptr<double[]> tileLoad;

            // Repetition vector application graph
            RepetitionVector repVec;

            // Constants used in tile sort function
            double cnst_a, cnst_b, cnst_c, cnst_d, cnst_e, cnst_f, cnst_g;
            double cnst_k, cnst_l, cnst_m, cnst_n, cnst_o, cnst_p, cnst_q;
    };

}//namespace SDF
#endif

