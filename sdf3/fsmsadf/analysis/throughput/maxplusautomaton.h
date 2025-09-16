/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   maxplusautomaton.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      max plus automaton.
 *
 *  History         :
 *      26-04-10    :   Initial version.
 *
 * $Id: maxplusautomaton.h,v 1.1.2.4 2010-05-07 06:14:52 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_MAXPLUSAUTOMATON_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_MAXPLUSAUTOMATON_H_INCLUDED

#include "fsmsadf/base/graph.h"
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/graph/mpautomaton.h"
#include "thrutils.h" // IWYU pragma: keep
#include <memory>
#include <unordered_set>


namespace FSMSADF
{

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomaton()
     * Compute the throughput of an FSM-SADF graph through a max-plus automaton.
     */
    Throughput throughputAnalysisGraphUsingMaxPlusAutomaton(const Graph& g);

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent()
     * Compute the throughput of a weakly consistent FSM-SADF graph through a max-plus automaton.
     */
    Throughput throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent(const Graph& g);

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomatonRegex()
     * Compute the throughput of a weakly consistent regex-based SADF graph through a max-plus automaton.
     */

    Throughput throughputAnalysisGraphUsingMaxPlusAutomatonRegex(const Graph& g);

    /**
    * criticalScenarioSequenceWeaklyConsistent()
    * Compute a critical scenario sequence of a weakly consistent FSM-based SADF graph through a max-plus automaton.
    */

    std::unique_ptr<ScenarioConstRefList> criticalScenarioSequenceWeaklyConsistent(const Graph& g);

    /**
     * namespace MPExploreStateSpace
     * This namespace contains the implementation of the throughput analysis function.
     */
    namespace MPExploreMaxPlusAutomaton
    {

        struct MPAEdgeLabelCriticalBuffers : MaxPlus::MPAREdgeLabel
        {
            std::unordered_set <int> bufferSet;
        };

        // inline MPAEdgeLabelCriticalBuffers make_rewbuffersetedgelabel(MPDelay d, CString* sc, CDouble r, unordered_set<int> bs)
        // {
        //     MPAEdgeLabelCriticalBuffers el;
        //     el.delay = d;
        //     el.scenario = *sc;
        //     el.reward = r;
        //     el.bufferSet = bs;
        //     return el;
        // }


        // types of states, edges, sets and cycle of an MPAR
        using MPARStateB = ::MaxPlus::FSM::Labeled::State<MaxPlus::MPAStateLabel, MPAEdgeLabelCriticalBuffers>;
        using MPARStateBRef = ::MaxPlus::FSM::Labeled::StateRef<MaxPlus::MPAStateLabel, MPAEdgeLabelCriticalBuffers>;
        using MPAREdgeB = ::MaxPlus::FSM::Labeled::Edge<MaxPlus::MPAStateLabel, MPAEdgeLabelCriticalBuffers>;
        using MPAREdgeBRef = ::MaxPlus::FSM::Labeled::EdgeRef<MaxPlus::MPAStateLabel, MPAEdgeLabelCriticalBuffers>;
        using MPARSetOfStatesB = ::MaxPlus::FSM::Labeled::SetOfStates<MaxPlus::MPAStateLabel, MPAEdgeLabelCriticalBuffers>;
        using MPARSetOfEdgesB = ::MaxPlus::FSM::Abstract::SetOfEdges;
        using MPARCycleB = std::list<MPAREdgeBRef>;

        class MaxPlusAutomatonWithRewardsAndCriticalBuffers : public
            ::MaxPlus::FSM::Labeled::FiniteStateMachine<MaxPlus::MPAStateLabel, MPAEdgeLabelCriticalBuffers >
        {
        public:
             MaxPlusAutomatonWithRewardsAndCriticalBuffers() = default;
            // destructor
             ~MaxPlusAutomatonWithRewardsAndCriticalBuffers() override = default;

            // Delete copy/move constructors and assignment operators
            MaxPlusAutomatonWithRewardsAndCriticalBuffers(const MaxPlusAutomatonWithRewardsAndCriticalBuffers &) = delete;
            MaxPlusAutomatonWithRewardsAndCriticalBuffers &operator=(const MaxPlusAutomatonWithRewardsAndCriticalBuffers &) = delete;
            MaxPlusAutomatonWithRewardsAndCriticalBuffers(MaxPlusAutomatonWithRewardsAndCriticalBuffers &&) = delete;
            MaxPlusAutomatonWithRewardsAndCriticalBuffers &operator=(MaxPlusAutomatonWithRewardsAndCriticalBuffers &&) = delete;

             // compute the maximum cycle ratio of delay over progress
            //CDouble calculateMCR(void);
            // compute the maximum cycle ratio of delay over progress and also return a critical cycle
            CDouble calculateMCRAndCycle(MPARCycleB *cycle);
        };

        /**
         *  A class to encapsulate the analysis algorithms on FSMSADFs using MPAs
         */
        class ThroughputCalculator
        {
            public:
                // Strongly consistent FSM-SADF.
                static Throughput calculateThroughput(const Graph& g);
                static MaxPlus::MPDelay calculateMCM(MaxPlus::MaxPlusAutomaton *mpa);

                // Weakly consistent FSM-SADF.
                static Throughput calculateThroughputWeaklyConsistent(const Graph& g);
                static Throughput calculateThroughputRegex(const Graph& g);
                static std::unique_ptr<ScenarioConstRefList> criticalScenarioSequenceWeaklyConsistent(const Graph& g);
                static std::unique_ptr<ScenarioConstRefList> throughputAndCriticalScenarioSequenceWeaklyConsistent(const Graph& g, Throughput &th);

                static CDouble calculateMCR(MaxPlus::MaxPlusAutomatonWithRewards *mpa);
                static CDouble calculateMCRAndCycle(MaxPlus::MaxPlusAutomatonWithRewards *mpa, MaxPlus::MPARCycle *cycle);
        };

    } // End namespace MPExploreMaxPlusAutomaton

} // End namespace FSMSADF

#endif

