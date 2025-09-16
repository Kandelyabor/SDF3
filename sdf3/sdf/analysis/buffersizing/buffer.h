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
 * $Id: buffer.h,v 1.1 2008-03-06 10:49:42 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_BUFFER_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_BUFFER_H_INCLUDED

#include "sdf/base/timed/graph.h"
#include "storage_distribution.h"
#include "types.h"
#include <cfloat> // IWYU pragma: keep
#include <cstdint>
#include <memory>

namespace SDF {
/**
 * Throughput / storage-space trade-off exploration
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */

class SDFstateSpaceBufferAnalysis {
public:
    // Constructor
    SDFstateSpaceBufferAnalysis() = default;

    // Destructor
    ~SDFstateSpaceBufferAnalysis() = default;

    // Delete copy constructor and copy assignment operator
    SDFstateSpaceBufferAnalysis(const SDFstateSpaceBufferAnalysis&) = delete;
    SDFstateSpaceBufferAnalysis& operator=(const SDFstateSpaceBufferAnalysis&) = delete;

    // Delete move constructor and move assignment operator
    SDFstateSpaceBufferAnalysis(SDFstateSpaceBufferAnalysis&&) = delete;
    SDFstateSpaceBufferAnalysis& operator=(SDFstateSpaceBufferAnalysis&&) = delete;

    // Analyze throughput/storage-space trade-off space
    StorageDistributionSet* analyze(const TimedSDFgraph& gr, double thrBound = DBL_MAX);

    // Analyze throughput/storage-space trade-off space step-by-step
    void initSearch(const TimedSDFgraph& gr);
    void ownGraph(std::unique_ptr<TimedSDFgraph> gr);
    const TimedSDFgraph *getGraph() { return this->g; };
    StorageDistributionSet* findNextStorageDistributionSet();

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

            // Copy constructor
            State(const State&) = default;

            // Copy assignment operator
            State& operator=(const State&) = default;

            // Move constructor
            State(State&&) = default;

            // Move assignment operator
            State& operator=(State&&) = default;

            // Initialize the state
            void init(const uint nrActors, const uint nrChannels) {
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
            std::vector<std::list<SDFdiscreteTime>> actClk;
            std::vector<TBufSize> ch;
            std::vector<TBufSize> sp;
            uint32_t glbClk{};
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(const TimedSDFgraph *gr) : g(gr) { initOutputActor(); };

        // Destructor
        ~TransitionSystem() = default;

        // Delete copy constructor and copy assignment operator
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;

        // Delete move constructor and move assignment operator
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(const std::unique_ptr<TBufSize[]> &sp, const std::unique_ptr<bool[]> & dep);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a, int *color, int *pi, const TDependencyGraph& abstractDepGraph, const std::unique_ptr<bool[]> &dep);
        void findStorageDependencies(const TDependencyGraph& abstractDepGraph, const std::unique_ptr<bool[]> &dep);
        void findCausalDependencies(const SDFactor& a, const TDependencyGraph& abstractDepGraph);
        void analyzePeriodicPhase(const std::unique_ptr<TBufSize[]> &sp, const std::unique_ptr<bool[]>& dep);
        void analyzeDeadlock(const std::unique_ptr<TBufSize[]> &sp, const std::unique_ptr<bool[]>& dep);

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
        const TimedSDFgraph *g;
        SDFactor *outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };

    // Bounds on the search space
    void initBoundsSearchSpace(const TimedSDFgraph& g);
    void initMinimalChannelSzStep(const TimedSDFgraph& g);
    void initMinimalChannelSz(const TimedSDFgraph& g);
    void initLbDistributionSz(const TimedSDFgraph& g);
    void initMaxThroughput(const TimedSDFgraph& g);

    // Storage distributions
    std::unique_ptr<StorageDistribution> newStorageDistribution();
    void execStorageDistribution(StorageDistribution& d);
    static void minimizeStorageDistributionsSet(StorageDistributionSet& ds);
    bool addStorageDistributionToChecklist(std::unique_ptr<StorageDistribution> d);
    void exploreStorageDistribution(StorageDistributionSet& ds,
                                                             StorageDistribution& d);
    void exploreStorageDistributionSet(StorageDistributionSet& ds);
    void findMinimalStorageDistributions(double thrBound);

    // SDF graph
    const TimedSDFgraph *g{};
    // Handle to hold on to the graph if needed.
    std::unique_ptr<TimedSDFgraph> ownedGraph;

    // Transition system
    std::unique_ptr<TransitionSystem> transitionSystem;

    // Storage distributions
    std::unique_ptr<StorageDistributionSet> minStorageDistributions;
    StorageDistributionSet* lastExploredStorageDistributionSet{};

    // Bounds on the search space
    std::unique_ptr<TBufSize[]> minSz;
    std::unique_ptr<TBufSize[]> minSzStep;
    TBufSize lbDistributionSz{};
    TDtime maxThroughput{};

    // Check if throughput th is equal to maximal throughput
    static inline bool checkAgainstMaxThroughput(double th, double maxTh, double margin);

    // static parameters
    // accuracy of throughput comparisons
    // initialized to 1e-10 in the .cc file (because VS wants it)
    static const double ThroughputMargin;
};

} // namespace SDF
#endif
