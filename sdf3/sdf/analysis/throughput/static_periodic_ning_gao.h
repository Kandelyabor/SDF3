/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_periodic_ning_gao.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 31, 2008
 *
 *  Function        :   State-space based throughput analysis for Ning and
 *                      Gao's buffer sizing problem
 *
 *  History         :
 *      31-03-08    :   Initial version.
 *
 * $Id: static_periodic_ning_gao.h,v 1.1 2008-09-18 07:35:13 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_THROUGHPUT_STATIC_PERIODIC_NING_GAO_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_THROUGHPUT_STATIC_PERIODIC_NING_GAO_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "sdf/analysis/buffersizing/storage_distribution.h"
#include <memory>


namespace SDF {
/**
 * Throughput / storage-space trade-off exploration for Ning and Gao's buffer
 * sizing problem. Analyze the trade-offs between storage distributions and
 * throughput.
 */
class SDFstateSpaceThroughputAnalysisNingGao {
public:
    // Constructor
    SDFstateSpaceThroughputAnalysisNingGao() = default;

    // Destructor
    ~SDFstateSpaceThroughputAnalysisNingGao() = default;

    // Delete copy constructor and copy assignment operator
    SDFstateSpaceThroughputAnalysisNingGao(const SDFstateSpaceThroughputAnalysisNingGao&) = delete;
    SDFstateSpaceThroughputAnalysisNingGao& operator=(const SDFstateSpaceThroughputAnalysisNingGao&) = delete;

    // Delete move constructor and move assignment operator
    SDFstateSpaceThroughputAnalysisNingGao(SDFstateSpaceThroughputAnalysisNingGao&&) = delete;
    SDFstateSpaceThroughputAnalysisNingGao& operator=(SDFstateSpaceThroughputAnalysisNingGao&&) = delete;

    // Analyze throughput
    static TDtime analyze(TimedSDFgraph& gr,
                   std::unique_ptr<StorageDistribution>& d,
                   SDFdiscreteTime period,
                   std::vector<SDFdiscreteTime> &startTime);

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

            // Copy constructor
            State(const State&) = default;
            // Copy assignment operator
            State& operator=(const State&) = default;
            // Move constructor
            State(State&&) = default;
            // Move assignment operator
            State& operator=(State&&) = default;

            // Destructor
            ~State() = default;

            // Initialize the state
            void init(const uint nrActors, const uint nrChannels) {
                actClk.resize(nrActors);
                ch.resize(nrChannels);
                sp.resize(nrActors);
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
            std::vector<TBufSize> sp;
            uint32_t glbClk{};
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(TimedSDFgraph *gr) : g(gr) { initOutputActor(); };

        // Destructor
        ~TransitionSystem() = default;

        // Delete copy constructor and copy assignment operator
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;

        // Delete move constructor and move assignment operator
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(const std::unique_ptr<TBufSize[]>&, SDFdiscreteTime &period, std::vector<SDFdiscreteTime> &startTime);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter& cycleIter);

        // State transitions
        bool releaseStorageSpaceSharedOutputBuffer(const SDFchannel& c);
        bool actorReadyToFire(const SDFactor& a);
        void startActorFiring(const TimedSDFactor& a);
        bool actorReadyToEnd(const SDFactor& a);
        void endActorFiring(const SDFactor& a);
        SDFdiscreteTime clockStep(std::vector<SDFdiscreteTime> &startTime,
                          std::vector<uint> &fireCnt,
                          SDFdiscreteTime &period,
                          TTime &globalTime);

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        TimedSDFgraph *g;
        SDFactor *outputActor{};
        TCnt outputActorRepCnt{};

        // Current state
        State currentState;

        // List of visited states that are stored
        States storedStates;
    };

    // SDF graph
    TimedSDFgraph *g{};

    // Transition system
    TransitionSystem *transitionSystem{};
};
} // namespace SDF
#endif
