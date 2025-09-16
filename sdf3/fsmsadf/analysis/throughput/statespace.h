/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   statespace.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 12, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      state-space traversal.
 *
 *  History         :
 *      12-08-09    :   Initial version.
 *
 * $Id: statespace.h,v 1.1.2.9 2010-05-07 19:21:20 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_STATESPACE_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_STATESPACE_H_INCLUDED

#include "fsmsadf/resource_allocation/platform_binding/graph.h"
#include "fsmsadf/base/graph.h"
#include "trace.h"
#include <memory>


namespace FSMSADF
{

    /**
     * throughputAnalysisGraphUsingStateSpace()
     * Compute the throughput of an FSM-based SADF graph through a state-space
     * traversal.
     */
    Throughput throughputAnalysisGraphUsingStateSpace(const Graph& g);

    /**
    * Generate traces of actor firings for the specified scenario sequence.
    */
    void traceGenerationUsingStateSpace(const Graph& g, CStrings filenames, const std::list<CString>& scenarioSequence, TraceFormat = Trace4CPS);
    void traceGenerationWeaklyConsistent(const Graph& g, CStrings filenames, const std::list<CString>& scenarioSequence, TraceFormat = Trace4CPS);
    void traceOfVectorsGenerationWeaklyConsistent(const Graph& g, CStrings filenames, const std::list<CString>& scenarioSequence, TraceFormat = Trace4CPS);

    void scheduleCheckUsingStateSpace(Graph& g, std::list<CString>& stringSequence, CNode *architectureNode, CNode *mappingNode);

    std::unique_ptr<std::list <ScenarioRef>> convertScenarioSequence(PlatformBinding *pb, std::list<CString>& stringSequence);
    void checkSchedule(ScenarioRefList& scenarioSequence, PlatformBinding *platformBinding, const Trace& trace);

    /**
     * namespace MPExploreStateSpace
     * This namespace contains the implementation of the throughput analysis
     * function.
     */
    namespace MPExploreStateSpace
    {

        /**
        * class DelaysAndRewards
        * A collection of combinations of delay and rewards, used for edges of graphs/automata
        */
        class DelaysAndRewards : public std::map <MPDelay, CDouble> {
        public:
            // add a new pair of delay d and reward r
            void add(MPDelay d, CDouble r);
            // add a new pair of delay d and reward r and prune the dominated combinations (smaller d and larger r)
            void addAndPrune(MPDelay d, CDouble r);
        };

        // forward declaration for ReachableStates
        class State;

        class ReachableStates : public std::map<State*, DelaysAndRewards > {};

        /**
         * class State
         * State represents a MaxPlus vector and a state in the FSM of the graph.
         */
        class State
        {
            public:
                // Constructor
                State(State* prevState, const ::MaxPlus::FSM::Abstract::State *newFsmState, const Matrix* m, double r);

                // Destructor
                ~State();

                // Delete copy constructor and copy assignment operator
                State(const State&) = delete;
                State& operator=(const State&) = delete;

                // Delete move constructor and move assignment operator
                State(State&&) = delete;
                State& operator=(State&&) = delete;

                // Compute hash value of the state
                [[nodiscard]] size_t hashValue() const;

                // Compare the states for equality on all their token timestamps and
                // equality of the FSM state.
                bool equals(const State *s) const;

                // Compare the states
                [[nodiscard]] int compare(const std::unique_ptr<State>& s) const;

                // Execute one iteration of matrix associated with this
                // state
                void executeIteration();

                // Normalize the vector. This sets
                // the delay of the state
                void normalize();

                // Add the state to the set of reachable states
                // returns true if the state was really added, false if it was redundant.
                bool addReachableState(State* state, MPDelay d, double r);

                // Print the state
                void print(std::ostream &out) const;

                // MaxPlus vector
                std::unique_ptr<MaxPlus::Vector> mpvector;

                // FSM state
                const ::MaxPlus::FSM::Abstract::State *fsmState;

                // Scenario Matrix
                const Matrix* mat;

                // delay, reward !!not part of the state!!
                // temporary to be annotated to the edge from previousState
                MPDelay delay;
				MPDelay reward;

                // Reachable states
                // state is reachable with different combinations of delay and reward
                ReachableStates reachableStates;

                // Previous state
                State* previousState;
        };



		/**
		 * class StateComparator
		 * Compare two states
		 */
		class StateComparator
		{
		public:
			bool operator()(const std::unique_ptr<State>& s1, const std::unique_ptr<State>& s2) const
			{
				return s1->compare(s2) == -1;
			}
		};

        /**
         * class States
         * Set of states.
         */
		class States : public std::set<std::unique_ptr<State>, StateComparator>
        {
            public:
                // check if the state is included in the set of states, if so
                // return the state in y and return true, otherwise return false
                bool includes(const std::unique_ptr<State>& x, State **y)
                {
                    auto i = find(x);
                    if (i != end())
                    {
                        (*y) = (*i).get();
                        return true;
                    }
                    return false;
                }

                bool includes(const std::unique_ptr<State>& x)
                {
                    return find(x) != end();
                }
        };

        /**
         * class StateSpaceExploration
         * The state-space analysis algorithms.
         */
        class StateSpaceExploration
        {
            public:
                // Compute the throughput of the FSM-based SADF graph using a
                // state-space traversal
                static Throughput exploreThroughput(const Graph& g);
                // Compute the throughput of the FSM-based SADF graph using a
                // state-space traversal while combining multiple scenarios leading
                // the same net state of the fsm.
                static Throughput exploreThroughputMax(const Graph& g);
                // Generate traces of actor firings for the specified scenario sequence.
                static std::unique_ptr<TraceOfFirings> generateTraces(const Graph& g, const std::list<CString>& scenarioSequence);
                static std::unique_ptr<TraceOfFirings> generateTracesWeakConsistency(const Graph& g, const std::list<CString>& scenarioSequence);
                static std::unique_ptr<TraceOfVectors> generateTraceOfVectorsWeakConsistency(const Graph& g, const std::list<CString>& scenarioSequence);


            private:
                // Compute the throughput of the FSM-based SADF graph by computing the
                // MCM of the state-space
                static Throughput computeThroughput(const States& visitedStates,
                                             State* initialState);
                // Compute MCR
                static double computeMCR(const States& visitedStates, State* initialState);

                // Output the state-space in DOT format
                static void printStateSpace(States *states, std::ostream &out);
        };

        class StateQueue: public std::list<std::unique_ptr<State>>
        {
        };

    } // End namespace MPExploreStateSpace

} // End namespace FSMSADF

#endif

