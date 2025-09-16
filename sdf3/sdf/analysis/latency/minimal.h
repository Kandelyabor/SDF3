/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   minimal.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 13, 2008
 *
 *  Function        :   State-space based latency analysis
 *
 *  History         :
 *      13-02-08    :   Initial version.
 *
 * $Id: minimal.h,v 1.1 2008-03-06 10:49:43 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_LATENCY_MINIMAL_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_LATENCY_MINIMAL_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "sdf/basic_types.h"
namespace SDF {
/**
 * Minimal latency analysis
 * Compute the minimal latency an SDF graph
 */
class SDFstateSpaceMinimalLatencyAnalysis {
public:
    // Constructor
    SDFstateSpaceMinimalLatencyAnalysis() = default;

    // Destructor
    ~SDFstateSpaceMinimalLatencyAnalysis() = default;

    // Delete copy/move constructors and assignment operators
    SDFstateSpaceMinimalLatencyAnalysis(const SDFstateSpaceMinimalLatencyAnalysis&) = delete;
    SDFstateSpaceMinimalLatencyAnalysis& operator=(const SDFstateSpaceMinimalLatencyAnalysis&) = delete;
    SDFstateSpaceMinimalLatencyAnalysis(SDFstateSpaceMinimalLatencyAnalysis&&) = delete;
    SDFstateSpaceMinimalLatencyAnalysis& operator=(SDFstateSpaceMinimalLatencyAnalysis&&) = delete;

    // Analyze latency of the graph (infinitely many compute resources)
    static void analyze(const TimedSDFgraph& g,
                        SDFactor& srcActor,
                        SDFactor& dstActor,
                        TDtime &latency,
                        TDtime &throughput);

    // Analyze latency of the graph (single compute resources)
    static void
    analyzeSingleProc(const TimedSDFgraph& g, SDFactor& srcActor, SDFactor& dstActor, TDtime &latency);

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

            // Delete copy/move constructors and assignment operators
            State(const State&) = default;
            State& operator=(const State&) = default;
            State(State&&) = delete;
            State& operator=(State&&) = delete;

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

        // Delete copy/move constructors and assignment operators
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(bool startFromInitialState = true);

        // Execute the SDFG following the demand list
        SDFdiscreteTime execSDFgraphUsingDemandList(std::vector<uint> &demandList);

        // Demand list
        void computeDemandList(const SDFchannel *ch, std::vector<uint> &demandList);

        // State
        State &getCurrentState() { return currentState; };
        void setCurrentState(State &s) { currentState = s; };
        State &getPreviousState() { return previousState; };
        void setPreviousState(State &s) { previousState = s; };

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter &cycleIter);

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
