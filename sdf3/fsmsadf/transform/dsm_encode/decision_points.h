/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   decision_points.h
 *
 *  Author          :   Morteza Damavandpeyma (m.damavandpeyma@tue.nl)
 *
 *  Date            :   March 25, 2011
 *
 *  Function        :   Find all decision points in a (SDF) graph according
 *                      to the given static periodic schedule
 *
 *  History         :
 *      25-03-11    :   Initial version.
 *      02-11-15    :   Adaptation and refactoring for use in fsm-sadf (Reinier)
 *
 * $Id: decision_points.h,v 1.1 2011-03-25 07:35:55 morteza Exp $
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

#ifndef FSMSADF_TRANSFORM_DECISION_POINTS_H_INCLUDED
#define FSMSADF_TRANSFORM_DECISION_POINTS_H_INCLUDED

#include <utility>

#include "fsmsadf/base/graph.h"
#include "fsmsadf/resource_allocation/scheduling/static_order/static_order_schedule.h"

namespace FSMSADF {
// Time
using ScenarioTime = uint;

#define SCENARIO_TIME_MAX UINT_MAX

/**
 * appearedUpToNow ()
 * return the number of times that actor 'a' was executed in the schedule 'sc' up to pointer 'ptr'
 */
int appearedUpToNow(StaticOrderSchedule sc, const CString& a, int ptr);

/**
 * Static-Periodic Scheduler
 * Compute a static-periodic schedule with maximal throughput for the supplied
 * scenario graph.
 */
class DecisionPoints {
public:
    // Constructor
    DecisionPoints() = default;

    // Destructor
    ~DecisionPoints() = default;
    
    DecisionPoints(const DecisionPoints&) = delete;
    DecisionPoints& operator=(const DecisionPoints&) = delete;
    DecisionPoints(DecisionPoints&&) = delete;
    DecisionPoints& operator=(DecisionPoints&&) = delete;

    // Schedule the graph
    static std::vector<std::vector<CString>> find(Scenario *s, ScenarioGraph *sg, const StaticOrderSchedule& sos);

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
            bool operator==(const State &s) const;

            // Clear state
            void clear();

            // Output state
            void print(std::ostream &out);

            // State information
            std::vector<std::list<ScenarioTime>> actClk;
            std::vector<TBufSize> ch;
            uint32_t glbClk{};
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        void splitActors();

        // Constructor
        TransitionSystem(Scenario *s, ScenarioGraph *scGraph, StaticOrderSchedule sos) :
            sg(scGraph), staticPeriodicSchedule(std::move(sos))  {
            releaseProcessor();
            repVec = s->computeRepetitionVectorOfScenarioGraph(sg);
            splitActors();
        };

        // Destructor
        ~TransitionSystem() = default;
        ;

        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;


        // Execute the scenario
        void execScenarioGraph(Scenario *s);

        // Fire scheduled actors
        bool fireScheduledActors(Scenario *s);

        // Fire unscheduled actors
        bool fireUnScheduledActors(Scenario *s);

        // print decision points in graph
        void printDecisionPoints(std::ostream &out);

        std::vector<std::vector<CString>> getDecisionPoints() { return readyActorList; }

    private:
        // Store state
        bool storeState(State &s);

        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // State transitions
        bool actorReadyToFire(Scenario *s, Actor *a);
        void startActorFiring(Scenario *s, Actor *a);
        bool actorReadyToEnd(Actor *a);
        void endActorFiring(Scenario *s, Actor *a);
        ScenarioTime clockStep(ScenarioTime step = UINT_MAX);

        // function to control execution of a single task during
        // the execution of graph
        [[nodiscard]] bool ProcessorIsBusy() const { return processorLock; }
        void activeProcessor() { processorLock = true; }
        void releaseProcessor() { processorLock = false; }

        int appearedUpToNow(const CString& actorName);
        bool actorIsInSched(const CString& actorName);
        bool actorsDone();

        bool finishActorFirings(const ActorRefs& actors, Scenario *s);

        // Scenario graph
        ScenarioGraph *sg;

        // Current state
        State currentState;

        // List of visited states that are stored
        States storedStates;

        // static periodic schedule
        StaticOrderSchedule staticPeriodicSchedule;

        // Position in schedule
        uint schedulePointer{};

        // ready to execute actor list ( more than one active means a decision point)
        std::vector<std::vector<CString>> readyActorList;

        bool processorLock{};

        // repetition vector of scenario graph
        RepetitionVector repVec;

        // Track which actors are scheduled and which are not
        ActorRefs scheduled;
        std::list<std::pair<ActorRef, int>> unScheduled;
    };
};

} // End namespace FSMSADF

#endif
