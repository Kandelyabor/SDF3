/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   priority_list_scheduler.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 6, 2008
 *
 *  Function        :   Construct a list schedule for each processor in the
 *                      platform graph.
 *
 *  History         :
 *      06-03-08    :   Initial version.
 *
 * $Id: priority_list_scheduler.h,v 1.1 2008-03-06 10:49:45 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_SCHEDULING_PRIORITY_LIST_SCHEDULER_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_SCHEDULING_PRIORITY_LIST_SCHEDULER_H_INCLUDED

#include "sdf/analysis/analysis.h" // IWYU pragma: keep
namespace SDF {

/**
 * Priority list scheduler
 * Constructs a set of static-order schedules for the processors in the
 * architecture platform. Each schedule orders the firings of the actors
 * running on the processor. A priority-base list scheduler is used to construct
 * the static-order schedules. The actor priorities depend on the number of
 * occurrences of an actor in the abstract dependency graph.
 */
class SDFstateSpacePriorityListScheduler {
public:
    // Constructor
    SDFstateSpacePriorityListScheduler() = default;

    // Destructor
    ~SDFstateSpacePriorityListScheduler() = default;

    // Prevent copying and moving
    SDFstateSpacePriorityListScheduler(const SDFstateSpacePriorityListScheduler&) = delete;
    SDFstateSpacePriorityListScheduler& operator=(const SDFstateSpacePriorityListScheduler&) = delete;
    SDFstateSpacePriorityListScheduler(SDFstateSpacePriorityListScheduler&&) = delete;
    SDFstateSpacePriorityListScheduler& operator=(SDFstateSpacePriorityListScheduler&&) = delete;


    // Schedule the graph
    static void schedule(BindingAwareSDFG *bg);

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

            // Prevent copying and moving
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
        explicit TransitionSystem(BindingAwareSDFG *bg) : bindingAwareSDFG(bg) {
            initOutputActor();
            initStaticOrderSchedules();
            initActorPriorities();
        };

        // Destructor
        ~TransitionSystem() = default;

        // Prevent copying and moving
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        TDtime execSDFgraph();

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

        // Initialize static-order schedules
        void initStaticOrderSchedules();

        // Actor priorities
        void findAllDependencies(
                bool **nodes, int nrNodes, int actor, int b, int *color, int *pi, int *cnt);
        void initActorPriorities();

        // Binding-aware SDFG
        BindingAwareSDFG *bindingAwareSDFG;

        // SDF graph and output actor
        SDFactor *outputActor{};
        TCnt outputActorRepCnt{};

        // Current and previous state
        State currentState;
        State previousState;

        // List of visited states that are stored
        States storedStates;

        // State information for schedulers
        std::vector<SDFactorRefs> actorReadyList;
        std::vector<bool> procIdle;

        // Actor priorities
        SDFactorRefs actorsOnPriority;
    };
};

} // namespace SDF
#endif
