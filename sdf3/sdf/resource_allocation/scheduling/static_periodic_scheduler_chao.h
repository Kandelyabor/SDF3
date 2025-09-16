/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_periodic_scheduler_chao.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 18, 2008
 *
 *  Function        :   Construct static-periodic schedule with maximal
 *                      throughput using scheduling algorithm from L.-F. Chao
 *                      and E.H.-M. Sha in "Static scheduling of DSP algorithms
 *                      on various models".
 *
 *  History         :
 *      18-09-08    :   Initial version.
 *
 * $Id: static_periodic_scheduler_chao.h,v 1.1 2008-09-25 10:49:58 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_PERIODIC_SCHEDULER_CHAO_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_PERIODIC_SCHEDULER_CHAO_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "base/fraction/fraction.h"
#include "sdf/basic_types.h"
namespace SDF {

/**
 * Static-Periodic Scheduler based on Chao's algorithm
 * Compute a static-periodic schedule with maximal throughput for the supplied
 * SDFG.
 */
class SDFstateSpaceStaticPeriodicSchedulerChao {
public:
    // Constructor
    SDFstateSpaceStaticPeriodicSchedulerChao() = default;

    // Destructor
    ~SDFstateSpaceStaticPeriodicSchedulerChao() = default;

    // Prevent copying and moving
    SDFstateSpaceStaticPeriodicSchedulerChao(const SDFstateSpaceStaticPeriodicSchedulerChao&) = delete;
    SDFstateSpaceStaticPeriodicSchedulerChao& operator=(const SDFstateSpaceStaticPeriodicSchedulerChao&) = delete;
    SDFstateSpaceStaticPeriodicSchedulerChao(SDFstateSpaceStaticPeriodicSchedulerChao&&) = delete;
    SDFstateSpaceStaticPeriodicSchedulerChao& operator=(SDFstateSpaceStaticPeriodicSchedulerChao&&) = delete;

    // Schedule the graph
    static void schedule(TimedSDFgraph *g);

private:
    // Schedule the graph
    static void computeSchedule(const TimedSDFgraph& g,
                         std::vector<std::vector<int64_t>> &startTime,
                         int64_t c,
                         int64_t f);

    /***************************************************************************
     * Transition system
     **************************************************************************/
    class TransitionSystem {
    public:
        /***********************************************************************
         * State
         **********************************************************************/
        class State {
        public:
            // Constructor
            explicit State(const uint nrActors = 0, const uint nrChannels = 0) {
                init(nrActors, nrChannels);
            };

            // Destructor
            ~State() = default;

            // Prevent copying and moving
            State(const State&) = default;
            State& operator=(const State&) = default;
            State(State&&) = default;
            State& operator=(State&&) = default;

            // Initialize the state
            void init(const uint nrActors, const uint nrChannels) {
                actClk.resize(nrActors);
                ch.resize(nrChannels);
            };

            // Compare states
            bool operator==(const State &s);

            // Clear state
            void clear();

            // Output state
            void print(std::ostream &out);

            // State information
            std::vector<std::list<SDFdiscreteTime>> actClk;
            std::vector<TBufSize> ch;
            uint32_t glbClk{};
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(const TimedSDFgraph& gr) : g(gr) { initOutputActor(); };

        // Destructor
        ~TransitionSystem() = default;

        // Prevent copying and moving
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        CFraction execSDFgraphSelfTimed();
        CFraction
        execSDFgraphStaticPeriodic(const std::vector<std::vector<int64_t>> &startTime,
                                   int64_t period);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        CFraction computeThroughput(const StatesIter &cycleIter);

        // State transitions
        bool actorReadyToFire(const SDFactor& a);
        void startActorFiring(const TimedSDFactor& a);
        bool actorReadyToEnd(const SDFactor& a);
        void endActorFiring(const SDFactor& a);
        SDFdiscreteTime clockStep(SDFdiscreteTime step = UINT_MAX);

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        const TimedSDFgraph& g;
        CId outputActor{};
        TCnt outputActorRepCnt{};

        // Current state
        State currentState;

        // List of visited states that are stored
        States storedStates;
    };
};

} // namespace SDF
#endif
