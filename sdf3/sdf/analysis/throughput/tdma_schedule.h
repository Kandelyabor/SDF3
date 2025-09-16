/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tdma_schedule.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Throughput calculation for SDFG mapped to an MPSoC
 *                      with TDMA arbitration on the processors and a
 *                      static-order schedule for actors running on the same
 *                      processor.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: tdma_schedule.h,v 1.1 2008-03-06 10:49:44 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_TDMA_SCHEDULE_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_TDMA_SCHEDULE_H_INCLUDED

#include "sdf/resource_allocation/binding_aware_sdfg/binding_aware_sdfg.h"

namespace SDF {
/**
 * Binding-aware throughput analysis
 * Computes the throughput of an SDFG mapped to an architecture platform. It
 * is assumed that the platform uses a TDMA resource arbitration mechanism on
 * the processors. An Actor is scheduled on a processor if its input tokens are
 * available and the static-order schedule indicates that the actor is allowed
 * to fire. Actors which are not mapped to a processor, only wait for their
 * input tokens.
 */
class SDFstateSpaceBindingAwareThroughputAnalysis {
public:
    // Constructor
    SDFstateSpaceBindingAwareThroughputAnalysis() = default;

    // Destructor
    ~SDFstateSpaceBindingAwareThroughputAnalysis() = default;

    // Delete copy/move constructors and assignment operators
    SDFstateSpaceBindingAwareThroughputAnalysis(const SDFstateSpaceBindingAwareThroughputAnalysis&) = delete;
    SDFstateSpaceBindingAwareThroughputAnalysis& operator=(const SDFstateSpaceBindingAwareThroughputAnalysis&) = delete;
    SDFstateSpaceBindingAwareThroughputAnalysis(SDFstateSpaceBindingAwareThroughputAnalysis&&) = delete;
    SDFstateSpaceBindingAwareThroughputAnalysis& operator=(SDFstateSpaceBindingAwareThroughputAnalysis&&) = delete;

    // Analyze throughput of the graph
    static TDtime analyze(const BindingAwareSDFG& bg, std::vector<double> &tileUtilization);

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
            explicit State(const uint nrActors = 0,
                           const uint nrChannels = 0,
                           const uint nrTiles = 0) {
                init(nrActors, nrChannels, nrTiles);
            };

            // Destructor
            ~State() = default;

            // Copy/move constructors and assignment operators
            State(const State&) = default;
            State& operator=(const State&) = default;
            State(State&&) = default;
            State& operator=(State&&) = default;

            // Initialize the state
            void init(const uint nrActors, const uint nrChannels, const uint nrTiles) {
                actClk.resize(nrActors);
                ch.resize(nrChannels);
                schedulePos.resize(nrTiles);
                tdmaPos.resize(nrTiles);
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
            std::vector<uint> schedulePos;
            std::vector<SDFdiscreteTime> tdmaPos;
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(const BindingAwareSDFG& bg) : bindingAwareSDFG(&bg) {
            checkBindingAwareSDFG();
            initOutputActor();
        };

        // Destructor
        ~TransitionSystem() = default;

        // Delete copy/move constructors and assignment operators
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(std::vector<double> &tileUtilization);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter& cycleIter);

        // Utilization of tiles in the platform by the application
        void computeTileUtilization(const StatesIter& recurrentState,
                                    std::vector<double> &tileUtilization);

        // State transitions
        bool actorReadyToFire(const SDFactor& a);
        void startActorFiring(const TimedSDFactor& a);
        bool actorReadyToEnd(const SDFactor& a);
        void endActorFiring(const SDFactor& a);
        SDFdiscreteTime clockStep();

        // Output actor
        void initOutputActor();

        // Sanity checks on the binding-aware SDFG
        void checkBindingAwareSDFG();

        // Binding-aware SDFG
        const BindingAwareSDFG *bindingAwareSDFG;

        // SDF graph and output actor
        SDFactor *outputActor{};
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
