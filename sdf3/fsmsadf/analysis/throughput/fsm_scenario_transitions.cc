/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm_scenario_transitions.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      scenario graph and considering scenario transitions.
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: fsm_scenario_transitions.cc,v 1.1.2.2 2010-04-22 07:07:09 mgeilen Exp $
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

#include "fsm_scenario_transitions.h"
#include "fsmsadf/analysis/maxplus/mpexplore.h"

#include "maxplus/algebra/mpmatrix.h"
#include <memory>

using namespace FSMSADF::MPExplore;
using namespace ::Graphs;

namespace FSMSADF {

using namespace MaxPlus;

/**
 * computeMCMfsm()
 * The function returns the MCM of an FSM.
 */
 Throughput computeMCMfsm(FSM *fsm,
                         std::map<FSMtransition *, MPTime> &delay,
                         std::map<Scenario*, MPTime> &period) {

    // Create a new MCM graph
    auto mcmGraph = std::make_unique<MCMgraph>();

    // Create the nodes of the MCM graph
    for (const auto &s : fsm->getStates()) {
        // Create an MCM node for this state
        mcmGraph->addNode(s->getId(), true);
    }

    // Add edges to the MCM graph
    for (const auto &s : fsm->getStates()) {
        // Iterate over the transitions of this state
        for (auto *j : s->getTransitions()) {
            // Create an edge in the MCM graph for this transition
            mcmGraph->addEdge(
                    static_cast<CId>(mcmGraph->getEdges().size()),
                    *(mcmGraph->getNode(j->getSrcState()->getId())),
                    *(mcmGraph->getNode(j->getDstState()->getId())),
                    static_cast<CDouble>(delay[j] + period[j->getDstState()->getScenario()]),
                    1.0);
        }
    }

    CDouble result = mcmGraph->calculateMaximumCycleMeanKarp();

    return result;
}


/**
 * Throughput analysis considering scenario transitions
 * Compute the throughput of an FSM-based SADF graph considering the FSM the
 * schedules of each source and destination scenario in the FSM.
 *
 * Algorithm:
 * 1. Determine for each scenario s the eigenvector v_s and period T_s.
 * 2. The throughput is equal to the MCM of the FSM where the weight of an edge
 *    which represents a scenario transition from scenario s to scenario t is
 *    equal to: d_st = exploreDelayForSchedule(v_s, v_t, T_t).
 */
Throughput throughputAnalysisWithScenarioTransitions(const Graph &g) {
    std::map<ScenarioRef, std::unique_ptr<MaxPlus::Vector>> eigenvector;
    std::map<ScenarioRef, MPTime> period;
    std::map<FSMtransition *, MPTime> delay;
    std::map<ScenarioRef, std::unique_ptr<MPExplore::Exploration>> mpe;
    Throughput thr = 0.0;

    // The set of scenarios...
    const Scenarios &scenarios = g.getConstScenarios();

    // Create Max-Plus exploration object for each scenario
    for (const auto& s : scenarios) {
        mpe[s.get()] = std::make_unique<MPExplore::Exploration>();
        mpe[s.get()]->G = std::make_unique<SGraph>(s->getScenarioGraph(), s.get());
    }

    // Compute eigenvector and period for each scenario
    for (const auto &s : scenarios) {
        std::unique_ptr<Vector> v_s = nullptr;
        MPTime T_s;

        // Compute eigenvector and period of this scenario
        mpe[s.get()]->exploreEigen(&v_s, &T_s);

        // Store results
        eigenvector[s.get()] = std::unique_ptr<Vector>(std::move(v_s));
        period[s.get()] = T_s;
    }

    // Compute delay for each scenario transition
    for (const auto &i : g.getFSM()->getStates()) {
        ScenarioRef s = i->getScenario();

        // Iterate over all transitions which are leaving this state
        for (auto *j : (i)->getTransitions()) {
            ScenarioRef t = j->getDstState()->getScenario();

            // Convert eigen vector of scenario s to eigen vector of scenario t
            auto v_s = std::unique_ptr<Vector>(
                    mpe[t]->convertEigenvector(s->getScenarioGraph(), eigenvector[s].get()));

            // Compute delay of this scenario transition
            delay[j] = mpe[t]->exploreDelayForSchedule(*v_s, *eigenvector[t], period[t]);
        }
    }

    // Compute MCM of the FSM using delay and period computed with reference
    // schedule
    thr = computeMCMfsm(g.getFSM(), delay, period);
    thr = 1.0 / thr;

    return thr;
}

} // End namespace FSMSADF
