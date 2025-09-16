/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 12, 2008
 *
 *  Function        :   State-space based latency analysis
 *
 *  History         :
 *      12-02-08    :   Initial version.
 *
 * $Id: selftimed.h,v 1.1 2008-03-06 10:49:43 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_LATENCY_SELFTIMED_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_LATENCY_SELFTIMED_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "sdf/basic_types.h"

namespace SDF {
/**
 * Selftimed latency analysis
 * Compute the latency an SDF graph for unconstrained buffer sizes and
 * auto-concurrency using a state-space traversal.
 */
class SDFstateSpaceSelfTimedLatencyAnalysis {
public:
    // Constructor
    SDFstateSpaceSelfTimedLatencyAnalysis() = default;

    // Destructor
    ~SDFstateSpaceSelfTimedLatencyAnalysis() = default;

    // Prevent copying and moving
    SDFstateSpaceSelfTimedLatencyAnalysis(const SDFstateSpaceSelfTimedLatencyAnalysis&) = delete;
    SDFstateSpaceSelfTimedLatencyAnalysis& operator=(const SDFstateSpaceSelfTimedLatencyAnalysis&) = delete;
    SDFstateSpaceSelfTimedLatencyAnalysis(SDFstateSpaceSelfTimedLatencyAnalysis&&) = delete;
    SDFstateSpaceSelfTimedLatencyAnalysis& operator=(SDFstateSpaceSelfTimedLatencyAnalysis&&) = delete;


    // Analyze latency of the graph
    static void analyze(TimedSDFgraph *g,
                 SDFactor *srcActor,
                 SDFactor *dstActor,
                 TDtime &latency,
                 TDtime &throughput);

private:
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
        explicit TransitionSystem(TimedSDFgraph *gr) : g(gr) { initOutputActor(); };

        // Destructor
        ~TransitionSystem() = default;

        // Prevent copying and moving
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(SDFactor *srcActor,
                            SDFactor *dstActor,
                            std::vector<SDFdiscreteTime> &timeSrcFire,
                            std::vector<SDFdiscreteTime> &timeDstFire,
                            uint distance);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter& cycleIter);

        // State transitions
        bool actorReadyToFire(const SDFactor& a);
        void startActorFiring(const TimedSDFactor& a);
        bool actorReadyToEnd(const SDFactor& a);
        void endActorFiring(const SDFactor& a);
        SDFdiscreteTime clockStep();

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        TimedSDFgraph *g;
        CId outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };
};
} // namespace SDF
#endif
