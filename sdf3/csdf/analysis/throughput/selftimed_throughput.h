/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed_throughput.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   October 10, 2007
 *
 *  Function        :   State-space based throughput analysis
 *
 *  History         :
 *      10-10-07    :   Initial version.
 *
 * $Id: selftimed_throughput.h,v 1.1 2008-03-22 14:24:21 sander Exp $
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

#ifndef CSDF_ANALYSIS_STATESPACE_SELFTIMED_THROUGHPUT_H_INCLUDED
#define CSDF_ANALYSIS_STATESPACE_SELFTIMED_THROUGHPUT_H_INCLUDED

#include "csdf/base/timed/graph.h"
#include "sdf/basic_types.h"
namespace CSDF {
/**
 * Throughput analysis
 * Compute the throughput of an CSDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
class CSDFstateSpaceThroughputAnalysis {
public:
    // Constructor
    CSDFstateSpaceThroughputAnalysis() = default;

    // Destructor

    // Prevent copying and moving
    CSDFstateSpaceThroughputAnalysis(const CSDFstateSpaceThroughputAnalysis&) = delete;
    CSDFstateSpaceThroughputAnalysis& operator=(const CSDFstateSpaceThroughputAnalysis&) = delete;
    CSDFstateSpaceThroughputAnalysis(CSDFstateSpaceThroughputAnalysis&&) = delete;
    CSDFstateSpaceThroughputAnalysis& operator=(CSDFstateSpaceThroughputAnalysis&&) = delete;

    ~CSDFstateSpaceThroughputAnalysis() = default;

    // Analyze throughput of the graph
    static TDtime analyze(const TimedCSDFgraph& g);

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

            // copying and moving
            State(const State&) = default;
            State& operator=(const State&) = default;
            State(State&&) = default;
            State& operator=(State&&) = default;

            // Initialize the state
            void init(const uint nrActors, const uint nrChannels) {
                actSeqPos.resize(nrActors);
                actClk.resize(nrActors);
                ch.resize(nrChannels);
            };

            // Compare states
            bool operator==(const State &s);

            // Clear state
            void clear();

            // set the initial tokens
            void setInitialTokens(const TimedCSDFgraph &g);

            // Output state
            void print(std::ostream &out);

            // State information
            std::vector<std::list<CSDFdiscreteTime>> actClk;
            std::vector<uint> actSeqPos;
            std::vector<TBufSize> ch;
            uint32_t glbClk{};
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(const TimedCSDFgraph& gr) : g(&gr) { initOutputActor(); };

        static bool actorReadyToFire(const State &s, const CSDFactor& a);
        static void fireActor(State& s, const TimedCSDFactor& a);

        // Destructor
        ~TransitionSystem() = default;

        // Prevent copying and moving
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the CSDFG
        TDtime execCSDFgraph();

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter& cycleIter);

        // State transitions
        bool actorReadyToFire(const CSDFactor& a);
        void startActorFiring(const TimedCSDFactor& a);
        static void startActorFiring(State& s, const TimedCSDFactor& a);
        bool actorReadyToEnd(const CSDFactor& a);
        void endActorFiring(const CSDFactor& a);
        static void endActorFiring(State& s, const CSDFactor& a);
        CSDFdiscreteTime clockStep();

        // Output actor
        void initOutputActor();

        // CSDF graph and output actor
        const TimedCSDFgraph *g;
        CId outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };
};
} // namespace CSDF
#endif
