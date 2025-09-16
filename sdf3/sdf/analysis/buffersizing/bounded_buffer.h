/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   bounded_buffer.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Compute throughput/buffer trade-off for an SDFG which
 *                      is bound to an multi-processor architecture.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: bounded_buffer.h,v 1.1 2008-03-06 10:49:42 sander Exp $
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

#ifndef SDF_ANALYSIS_STATE_SPACE_BOUNDED_BUFFER_H_INCLUDED
#define SDF_ANALYSIS_STATE_SPACE_BOUNDED_BUFFER_H_INCLUDED

#include "sdf/resource_allocation/binding_aware_sdfg/binding_aware_sdfg.h"
#include "storage_distribution.h"
#include "types.h"
#include <memory>

namespace SDF {
/**
 * Throughput / storage-space trade-off exploration
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency) under the given binding constraints of the application
 * to the architecture graph. The vector 'bufferChannels' indicates for each
 * channel wether initial tokens can be added to it. This is only true for
 * channels modeling buffer space. When the 'useBoundsOnBufferChannels' flag
 * is true, the number of initial tokens (i.e. storage space) is not enlarged
 * above the current number of initial tokens on the edge (i.e. current storage
 * space). This storage space allocation is then a constraint on the maximum
 * amount of storage space that can be allocated to the channel. The exploration
 * ends as soon as the throughput reaches the throughput bound (thrBound) or all
 * minimal storage distributions are found (default).
 */

class SDFstateSpaceBindingAwareBufferAnalysis {
public:
    // Constructor
    SDFstateSpaceBindingAwareBufferAnalysis() = default;

    // Destructor
    ~SDFstateSpaceBindingAwareBufferAnalysis() = default;

    // Delete copy constructor and copy assignment operator
    SDFstateSpaceBindingAwareBufferAnalysis(const SDFstateSpaceBindingAwareBufferAnalysis &) =
            delete;
    SDFstateSpaceBindingAwareBufferAnalysis &
    operator=(const SDFstateSpaceBindingAwareBufferAnalysis &) = delete;

    // Delete move constructor and move assignment operator
    SDFstateSpaceBindingAwareBufferAnalysis(SDFstateSpaceBindingAwareBufferAnalysis &&) = delete;
    SDFstateSpaceBindingAwareBufferAnalysis &
    operator=(SDFstateSpaceBindingAwareBufferAnalysis &&) = delete;

    // Analyze throughput/storage-space trade-off space
    std::unique_ptr<StorageDistributionSet> analyze(BindingAwareSDFG *bg,
                                    bool *bufferChannels,
                                    bool useBoundsOnBufferChannels,
                                    double thrBound);

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
            // Delete copy constructor and copy assignment operator
            State(const State &) = default;
            State &operator=(const State &) = default;
            // Delete move constructor and move assignment operator
            State(State &&) = delete;
            State &operator=(State &&) = delete;

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
            std::vector<uint32_t> schedulePos;
            std::vector<SDFdiscreteTime> tdmaPos;
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(BindingAwareSDFG *bg) : bindingAwareSDFG(bg) {
            checkBindingAwareSDFG();
            initOutputActor();
        };

        // Destructor
        ~TransitionSystem() = default;

        // Delete copy constructor and copy assignment operator
        TransitionSystem(const TransitionSystem &) = delete;
        TransitionSystem &operator=(const TransitionSystem &) = delete;

        // Delete move constructor and move assignment operator
        TransitionSystem(TransitionSystem &&) = delete;
        TransitionSystem &operator=(TransitionSystem &&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph(const std::unique_ptr<TBufSize[]> &sp,
                            const TDependencies& dep,
                            bool *bufferChannels);

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a,
                                  int *color,
                                  int *pi,
                                  const TDependencies *abstractDepGraph,
                                  const TDependencies& dep);
        void findStorageDependencies(const TDependencies *abstractDepGraph,
                                     const TDependencies& dep,
                                     const bool *bufferChannels);
        void findCausalDependencies(SDFactor *a, TDependencies *abstractDepGraph);
        void analyzePeriodicPhase(const TDependencies& dep, bool *bufferChannels);
        void analyzeDeadlock(const TDependencies& dep, bool *bufferChannels);

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter &cycleIter);

        // State transitions
        bool actorReadyToFire(SDFactor *a);
        void startActorFiring(TimedSDFactor *a);
        bool actorReadyToEnd(SDFactor *a);
        void endActorFiring(SDFactor *a);
        SDFdiscreteTime clockStep();

        // Output actor
        void initOutputActor();

        // Sanity checks on the binding-aware SDFG
        void checkBindingAwareSDFG();

        // Binding-aware SDFG
        BindingAwareSDFG *bindingAwareSDFG;

        // SDF graph and output actor
        SDFactorRef outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };

    // Bounds on the search space
    void initBoundsSearchSpace(TimedSDFgraph *g, bool *bufferChannels);
    void initMinimalChannelSzStep(TimedSDFgraph *g);
    void initMinimalChannelSz(TimedSDFgraph *g, const bool *bufferChannels);
    void initLbDistributionSz(TimedSDFgraph *g);
    void initMaxThroughput(TimedSDFgraph *g);

    // Storage distributions
    std::unique_ptr<StorageDistribution> newStorageDistribution();
    void execStorageDistribution(StorageDistribution& d, bool *bufferChannels);
    static void minimizeStorageDistributionsSet(StorageDistributionSet& ds);
    bool addStorageDistributionToChecklist(std::unique_ptr<StorageDistribution> d);
    void exploreStorageDistribution(StorageDistributionSet& ds,
                                    StorageDistribution& d,
                                    bool *bufferChannels,
                                    bool useBoundsOnBufferChannels);
    void exploreStorageDistributionSet(StorageDistributionSet& ds,
                                       bool *bufferChannels,
                                       bool useBoundsOnBufferChannels);
    void findMinimalStorageDistributions(double thrBound,
                                         bool *bufferChannels,
                                         bool useBoundsOnBufferChannels);

    // Binding-aware SDFG
    BindingAwareSDFG *bindingAwareSDFG{};

    // Transition system
    std::unique_ptr<TransitionSystem> transitionSystem;

    // Storage distributions
    std::unique_ptr<StorageDistributionSet> minStorageDistributions;

    // Bounds on the search space
    std::unique_ptr<TBufSize[]> minSz;
    std::unique_ptr<TBufSize[]> minSzStep;
    TBufSize lbDistributionSz{};
    TDtime maxThroughput{};
};

} // namespace SDF
#endif
