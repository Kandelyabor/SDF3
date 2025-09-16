/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 5, 2006
 *
 *  Function        :   State-space based buffer size analysis
 *
 *  History         :
 *      05-04-06    :   Initial version.
 *      27-10-06    :   BFS based version of the trade-off space exploration
 *                      algorithm.
 *
 * $Id: buffer.h,v 1.2 2008-04-09 15:02:00 sander Exp $
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

#ifndef CSDF_ANALYSIS_STATESPACE_BUFFER_H_INCLUDED
#define CSDF_ANALYSIS_STATESPACE_BUFFER_H_INCLUDED

#include "csdf/base/timed/graph.h"
#include "sdf/sdf.h" // IWYU pragma: keep
#include <memory>

namespace CSDF {

/**
 * Throughput / storage-space trade-off exploration
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */
class CSDFstateSpaceBufferAnalysis {
public:
    // Constructor
    CSDFstateSpaceBufferAnalysis() = default;

    // Destructor
    ~CSDFstateSpaceBufferAnalysis() = default;

    // Prevent copying and moving
    CSDFstateSpaceBufferAnalysis(const CSDFstateSpaceBufferAnalysis&) = delete;
    CSDFstateSpaceBufferAnalysis& operator=(const CSDFstateSpaceBufferAnalysis&) = delete;
    CSDFstateSpaceBufferAnalysis(CSDFstateSpaceBufferAnalysis&&) = delete;
    CSDFstateSpaceBufferAnalysis& operator=(CSDFstateSpaceBufferAnalysis&&) = delete;

    // Analyze throughput/storage-space trade-off space
    std::unique_ptr<SDF::StorageDistributionSet> analyze(const TimedCSDFgraph& gr, double thrBound = DBL_MAX);

    // Analyze throughput/storage-space trade-off space step-by-step
    void initSearch(const TimedCSDFgraph& gr);
    SDF::StorageDistributionSet* findNextStorageDistributionSet();

    // Approximation technique
    static SDF::StorageDistributionSet *
    approximate(const TimedCSDFgraph& gr, double thr, double maxPercentageOverEstimation);

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
                sp.resize(nrChannels);
            };

            // Compare states
            bool operator==(const State &s);

            // Clear state
            void clear();

            // Output state
            void print(std::ostream &out);

            // State information
            std::vector<std::list<CSDFdiscreteTime>> actClk;
            std::vector<uint> actSeqPos;
            std::vector<TBufSize> ch;
            std::vector<TBufSize> sp;
            uint32_t glbClk{};
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(const TimedCSDFgraph& gr) : g(&gr) { initOutputActor(); };

        // Destructor
        ~TransitionSystem() = default;

        // copying and moving
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execCSDFgraph(const std::unique_ptr<TBufSize[]> &sp,
                             const std::unique_ptr<bool[]> &dep);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a, int *color, int *pi, const TDependencyGraph& abstractDepGraph, const std::unique_ptr<bool[]> &dep);
        void findStorageDependencies(const TDependencyGraph& abstractDepGraph, const std::unique_ptr<bool[]> &dep);
        void findCausalDependencies(const CSDFactor& a, const TDependencyGraph& abstractDepGraph);
        void analyzePeriodicPhase(const std::unique_ptr<TBufSize[]> &sp,
                                  const std::unique_ptr<bool[]> &dep);
        void analyzeDeadlock(const std::unique_ptr<TBufSize[]> &sp,
                             const std::unique_ptr<bool[]> &dep);

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter &cycleIter);

        // State transitions
        bool actorReadyToFire(const CSDFactor& a);
        void startActorFiring(const TimedCSDFactor& a);
        bool actorReadyToEnd(const CSDFactor& a);
        void endActorFiring(const CSDFactor& a);
        CSDFdiscreteTime clockStep();

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        const TimedCSDFgraph *g;
        CSDFactor *outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };

    // Bounds on the search space
    void initBoundsSearchSpace(const TimedCSDFgraph& g);
    void initMinimalChannelSzStep(const TimedCSDFgraph& g);
    void initMinimalChannelSz(const TimedCSDFgraph& g);
    void initLbDistributionSz(const TimedCSDFgraph& g);
    void initMaxThroughput(const TimedCSDFgraph& g);

    // Storage distributions
    std::unique_ptr<SDF::StorageDistribution> newStorageDistribution();
    void execStorageDistribution(SDF::StorageDistribution& d);
    static void minimizeStorageDistributionsSet(SDF::StorageDistributionSet& ds);
    bool addStorageDistributionToChecklist(std::unique_ptr<SDF::StorageDistribution> d);
    void exploreStorageDistribution(SDF::StorageDistributionSet& ds,
                                    SDF::StorageDistribution& d);
    void exploreStorageDistributionSet(SDF::StorageDistributionSet& ds);
    void findMinimalStorageDistributions(double thrBound);

    // CSDF graph
    const TimedCSDFgraph *g{};

    // Transition system
    std::unique_ptr<TransitionSystem> transitionSystem;

    // Storage distributions
    std::unique_ptr<SDF::StorageDistributionSet> minStorageDistributions;
    SDF::StorageDistributionSet* lastExploredStorageDistributionSet{};

    // Bounds on the search space
    std::unique_ptr<TBufSize[]> minSz;
    std::unique_ptr<TBufSize[]> minSzStep;
    TBufSize lbDistributionSz{};
    TDtime maxThroughput{};

    // Approximation technique
    TBufSize *lastChStep{};
    uint quantizationFactor{};

    // Check if throughput th is equal to maximal throughput
    static inline bool checkAgainstMaxThroughput(double th, double maxTh, double margin);

    // static parameters
    // accuracy of throughput comparisons
    // initialized to 1e-10 in the .cc file (because VS wants it)
    static const double ThroughputMargin;
};
} // namespace CSDF
#endif
