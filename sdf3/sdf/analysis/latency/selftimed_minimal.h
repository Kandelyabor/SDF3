/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed_minimal.h
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
 * $Id: selftimed_minimal.h,v 1.1 2008-03-06 10:49:44 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_LATENCY_SELFTIMED_MINIMAL_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_LATENCY_SELFTIMED_MINIMAL_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "sdf/basic_types.h"
#include <memory>
namespace SDF {
/**
 * Selftimed latency analysis
 * Compute the minimal latency an SDF graph under maximal throughput.
 */
class SDFstateSpaceSelfTimedMinimalLatencyAnalysis {
public:
    // Constructor
    SDFstateSpaceSelfTimedMinimalLatencyAnalysis() = default;

    // Destructor
    ~SDFstateSpaceSelfTimedMinimalLatencyAnalysis() = default;

    SDFstateSpaceSelfTimedMinimalLatencyAnalysis(
            const SDFstateSpaceSelfTimedMinimalLatencyAnalysis &) = delete;
    SDFstateSpaceSelfTimedMinimalLatencyAnalysis &
    operator=(const SDFstateSpaceSelfTimedMinimalLatencyAnalysis &) = delete;
    SDFstateSpaceSelfTimedMinimalLatencyAnalysis(SDFstateSpaceSelfTimedMinimalLatencyAnalysis &&) =
            delete;
    SDFstateSpaceSelfTimedMinimalLatencyAnalysis &
    operator=(SDFstateSpaceSelfTimedMinimalLatencyAnalysis &&) = delete;

    // Analyze latency of the graph
    static void analyze(const TimedSDFgraph &g,
                        SDFactor &srcActor,
                        SDFactor &dstActor,
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

        // Timing constraint on a firing
        using TimingConstraintFiring = struct TimingConstraintFiring {
            CId id;
            SDFdiscreteTime timeTillNextFiring;
            uint nrFiringsEnabled;
            TransitionSystem::State lastStoredState;
            bool visit;
            std::shared_ptr<TimingConstraintFiring> next;
        };

        // Constructor
        TransitionSystem(const TimedSDFgraph &gr, SDFactor *outActor) : g(&gr) {
            initOutputActor(outActor);
        };

        // Destructor
        ~TransitionSystem() = default;

        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        std::shared_ptr<TimingConstraintFiring> execSDFgraph(const SDFactor &dstActor);

        // Execute the SDFG following the timing constraints
        TDtime execConstrainedSDFgraph(const SDFactor &srcActor,
                                       std::shared_ptr<TimingConstraintFiring> &timeSrcFire,
                                       double latency,
                                       const SDFactor &dstActor,
                                       std::vector<SDFdiscreteTime> &srcTimeFire,
                                       std::vector<SDFdiscreteTime> &dstTimeFire,
                                       uint distance);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter &cycleIter);

        // State transitions
        bool actorReadyToFire(const SDFactor &a);
        void startActorFiring(const TimedSDFactor &a);
        bool actorReadyToEnd(const SDFactor &a);
        void endActorFiring(const SDFactor &a);
        SDFdiscreteTime clockStep(uint maxStep = UINT_MAX);

        // Output actor
        void initOutputActor(SDFactor *outActor);

        // SDF graph and output actor
        const TimedSDFgraph *g;
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
