/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer_ning_gao.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 31, 2008
 *
 *  Function        :   State-space based buffer size analysis for Ning and
 *                      Gao's buffer sizing problem
 *
 *  History         :
 *      31-03-08    :   Initial version.
 *
 * $Id: buffer_ning_gao.h,v 1.2 2008-09-18 07:38:21 sander Exp $
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

#ifndef SDF_ANALYSIS_STATE_SPACE_BUFFER_NING_GAO_H_INCLUDED
#define SDF_ANALYSIS_STATE_SPACE_BUFFER_NING_GAO_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "storage_distribution.h"
#include "types.h"
#include <memory>

namespace SDF {
/**
 * Throughput / storage-space trade-off exploration for Ning and Gao's buffer
 * sizing problem. Analyze the trade-offs between storage distributions and
 * throughput.
 */

class SDFstateSpaceBufferAnalysisNingGao {
public:
    // Constructor
    SDFstateSpaceBufferAnalysisNingGao() = default;

    // Destructor
    ~SDFstateSpaceBufferAnalysisNingGao() = default;

    // Delete copy and move constructors and assignment operators
    SDFstateSpaceBufferAnalysisNingGao(const SDFstateSpaceBufferAnalysisNingGao&) = delete;
    SDFstateSpaceBufferAnalysisNingGao& operator=(const SDFstateSpaceBufferAnalysisNingGao&) = delete;
    SDFstateSpaceBufferAnalysisNingGao(SDFstateSpaceBufferAnalysisNingGao&&) = delete;
    SDFstateSpaceBufferAnalysisNingGao& operator=(SDFstateSpaceBufferAnalysisNingGao&&) = delete;

    // Analyze throughput/storage-space trade-off space
    std::unique_ptr<StorageDistribution> analyze(TimedSDFgraph *gr, std::vector<SDFdiscreteTime> &startTime);

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

            // Delete copy and move constructors and assignment operators
            State(const State&) = default;
            State& operator=(const State&) = default;
            State(State&&) = delete;
            State& operator=(State&&) = delete;

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

        // Delete copy and move constructors and assignment operators
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(const std::unique_ptr<TBufSize[]> &sp,
                            const std::unique_ptr<bool[]> &dep,
                            std::vector<SDFdiscreteTime> &startTime);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a,
                                  int *color,
                                  int *pi,
                                  const TDependencyGraph& abstractDepGraph,
                                  const std::unique_ptr<bool[]> &dep);
        void findStorageDependencies(const TDependencyGraph& abstractDepGraph, const std::unique_ptr<bool[]> &dep);
        void findCausalDependencies(
        const SDFactor& a, const TDependencyGraph& abstractDepGraph);
        void analyzePeriodicPhase(const std::unique_ptr<TBufSize[]> &sp,
                                  const std::unique_ptr<bool[]> &dep);
        void analyzeDeadlock(const std::unique_ptr<TBufSize[]> &sp,
                             const std::unique_ptr<bool[]> &dep);

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter &cycleIter);

        // State transitions
        bool releaseStorageSpaceSharedOutputBuffer(const SDFchannel& c);
        bool actorReadyToFire(const SDFactor& a);
        void startActorFiring(const TimedSDFactor& a);
        bool actorReadyToEnd(const SDFactor& a);
        void endActorFiring(const SDFactor& a);
        SDFdiscreteTime clockStep();

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        TimedSDFgraph *g;
        const SDFactor *outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };

    // Bounds on the search space
    void initBoundsSearchSpace(TimedSDFgraph *g);
    void initMinimalChannelSzStep(TimedSDFgraph *g);
    void initMinimalChannelSz(TimedSDFgraph *g);
    void initLbDistributionSz(TimedSDFgraph *g);
    void initMaxThroughput(TimedSDFgraph *g);

    // Storage distributions
    std::unique_ptr<StorageDistribution> newStorageDistribution();
    void execStorageDistribution(StorageDistribution& d,
                                 std::vector<SDFdiscreteTime> &startTime);
    static void minimizeStorageDistributionsSet(StorageDistributionSet& ds);
    bool addStorageDistributionToChecklist(std::unique_ptr<StorageDistribution> d);
    void exploreStorageDistribution(StorageDistributionSet& ds,
                                    StorageDistribution& d);
    void exploreStorageDistributionSet(StorageDistributionSet& ds);
    void findMinimalStorageDistributions(double thrBound);

    // SDF graph
    TimedSDFgraph *g{};

    // Transition system
    std::unique_ptr<TransitionSystem> transitionSystem = nullptr;

    // Storage distributions
    std::unique_ptr<StorageDistributionSet> minStorageDistributions;
    StorageDistributionSet* lastExploredStorageDistributionSet{};

    // Bounds on the search space
    std::unique_ptr<TBufSize[]> minSz;
    std::unique_ptr<TBufSize[]> minSzStep;
    TBufSize lbDistributionSz{};
    TDtime maxThroughput{};
};

} // namespace SDF
#endif
