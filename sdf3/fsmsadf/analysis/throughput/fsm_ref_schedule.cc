/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm_ref_schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      scenario graph with reference schedule.
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: fsm_ref_schedule.cc,v 1.1.2.2 2010-04-22 07:07:09 mgeilen Exp $
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

#include "maxplus/algebra/mpmatrix.h"
#include "fsm_ref_schedule.h"
#include "fsmsadf/analysis/maxplus/mpexplore.h"
#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include "scenariograph.h"
#include <memory>


using namespace FSMSADF::MPExplore;
using namespace ::Graphs;

namespace FSMSADF
{

    using namespace MaxPlus;

    namespace {
    Throughput computeMCMfsm(FSM *fsm, std::map<ScenarioRef, MPTime> &delay,
                             std::map<ScenarioRef, MPTime> &period)
    {
        // Create a new MCM graph
        auto mcmGraph = std::make_unique<MCMgraph>();

        // Create the nodes of the MCM graph
        for (const auto& s: fsm->getStates())
        {
            // Create an MCM node for this state
            mcmGraph->addNode(s->getId(), true);

        }

        // Add edges to the MCM graph
        for (const auto& s: fsm->getStates())
        {
            // Iterate over the transitions of this state
            for (const auto& j: s->getTransitions())
            {
                // Create an edge in the MCM graph for this transition
                mcmGraph->addEdge(static_cast<CId>(mcmGraph->getEdges().size()),
                                  *(mcmGraph->getNode(j->getSrcState()->getId())),
                                  *(mcmGraph->getNode(j->getDstState()->getId())),
                                  static_cast<CDouble>(delay[s->getScenario()] + period[s->getScenario()]),
                                  1.0);
            }
        }

        CDouble result = mcmGraph->calculateMaximumCycleMeanKarp();

        return result;
    }
    } // namespace

    /**
     * Throughput analysis using reference schedule
     * Compute the throughput of a strongly consistent FSM-based SADF graph considering the FSM and a
     * reference schedule.
     * Assumes that the reward of each scenario is equal to one.
     *
     * Algorithm:
     * 1. Determine for each scenario s the eigenvector v_s and period T_s.
     * 2. Compute a suitable 'reference schedule'. This can be done by for example,
     *    taking the max over all eigenvectors. An alternative could be to use the
     *    ALAP eigenvector. Currently, the first option is used.
     * 3. Compute the delay for each scenario s using the reference schedule and the
     *    period of scenario s.
     * 4. The throughput is equal to the MCM of the FSM where the weight of a
     *    node which corresponds to scenario s is equal to (d_s + T_s).
     */
    Throughput throughputAnalysisUsingRefSchedule(const Graph& g)
    {
        std::map<ScenarioRef, std::unique_ptr<MaxPlus::Vector>> eigenvector;
        std::map<ScenarioRef, MPTime> period;
        std::map<ScenarioRef, MPTime> delay;
        std::map<CString, MaxPlus::Vector> v_ref;
        std::map<Scenario *, std::unique_ptr<MPExplore::Exploration>> mpe;
        Throughput thr = 0.0;

        // check if the graph is strongly connected and if all rewards are equal to one.
        if (!g.isStronglyConsistent())
        {
            throw CException("Cannot compute throughput with this method, because the FSM-SADF graph is not strongly consistent");
        }
        // check if the graph is strongly connected and if all rewards are equal to one.
        if (!g.hasUnitRewards())
        {
            throw CException("Cannot compute throughput with this method, because the FSM-SADF graph does not have unit rewards for all scenarios");
        }


        // The set of scenarios...
        const Scenarios& scenarios = g.getConstScenarios();

        // Create Max-Plus exploration object for each scenario
        for (const auto& s: scenarios)
        {
            mpe[s.get()] = std::make_unique<MPExplore::Exploration>();
            mpe[s.get()]->G = std::make_unique<SGraph>(s->getScenarioGraph(), s.get());
        }

        // Compute eigenvector and period for each scenario
        for (const auto& s: scenarios)
        {
            std::unique_ptr<MaxPlus::Vector> v_s = nullptr;
            MPTime T_s;

            // Compute eigenvector and period of this scenario
            mpe[s.get()]->exploreEigen(&v_s, &T_s);

            // Store results
            eigenvector[s.get()] = std::move(v_s);
            period[s.get()] = T_s;
        }

        // Determine reference schedule
        v_ref = computeReferenceSchedule(eigenvector);

        // Compute delay for each scenario
        for (const auto& s: scenarios)
        {
            // Compute reference schedule for scenario s
            std::unique_ptr<MaxPlus::Vector> v_s = getReferenceScheduleForScenario(s.get(), v_ref);

            // Compute delay of this scenario
            delay[s.get()] = mpe[s.get()]->exploreDelayForSchedule(*v_s, *v_s, period[s.get()]);
        }

        // Compute MCM of the FSM using delay and period computed with reference
        // schedule
        thr = computeMCMfsm(g.getFSM(), delay, period);
        thr = 1.0 / thr;

        return thr;
    }

}  // End namespace FSMSADF

