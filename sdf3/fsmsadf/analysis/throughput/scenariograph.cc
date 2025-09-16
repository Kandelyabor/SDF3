/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scenariograph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      scenario graph.
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: scenariograph.cc,v 1.2.2.2 2010-04-22 07:07:09 mgeilen Exp $
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

#include "scenariograph.h"
#include "fsmsadf/analysis/maxplus/mpexplore.h"
#include "maxplus/algebra/mpmatrix.h"
#include <memory>


using namespace FSMSADF::MPExplore;

namespace FSMSADF
{

    using namespace MaxPlus;

    /**
     * computeReferenceSchedule()
     * The function computes a reference schedule based on the set of eigenvectors.
     * The reference schedule contains a map of channel names to initial token time
     * stamps.
     */
    std::map<CString, MaxPlus::Vector> computeReferenceSchedule(
        std::map<ScenarioRef, std::unique_ptr<MaxPlus::Vector>>& eigenvector)
    {
        std::map<CString, MaxPlus::Vector> v_ref;

        // Iterate over all eigenvectors
        for (auto& [i_sc,i_vc]: eigenvector)
        {
            ScenarioRef s = i_sc;
            ScenarioGraph *sg = s->getScenarioGraph();
            uint idx = 0;

            // Iterate over all channels in the scenario graph
            for (const auto& c: sg->getChannels())
            {
                CString cName = c->getName();

                // Is a channel with the same name already part of the reference
                // schedule?
                auto k = v_ref.find(cName);
                if (k != v_ref.end())
                {
                    // Channel must have same number of initial tokens in all
                    // scenario graphs.
                    if (k->second.getSize() != c->getInitialTokens())
                    {
                        throw ::CException("Channel '" + cName + "' should have same "
                                         "number of initial tokens in all scenario "
                                         "graphs.");
                    }

                    // Take maximum time stamp as reference time stamp
                    for (uint l = 0; l < c->getInitialTokens(); l++)
                    {
                        MPTime a = v_ref[cName].get(l);
                        MPTime b = i_vc->get(idx++);
                        v_ref[cName].put(l, MP_MAX(a, b));
                    }
                }
                else
                {
                    // The channel has not been seen before, use the time stamps
                    // of this eigen vector as reference schedule.
                    for (uint l = 0; l < c->getInitialTokens(); l++)
                    {
                        v_ref[cName].put(l, i_vc->get(idx++));
                    }
                }
            }

            // All entries of the eigenvector should have been placed in the
            // reference schedule
            ASSERT(idx == i_vc->getSize(), "All initial tokens should be "
                   "used in the reference schedule.");
        }

        return v_ref;
    }

    /**
     * getReferenceScheduleForScenario()
     * The function returns the reference schedule for scenario s based on the
     * reference schedule in v_ref.
     */
    std::unique_ptr<MaxPlus::Vector> getReferenceScheduleForScenario(Scenario *s,
            std::map<CString, MaxPlus::Vector> &v_ref)
    {
        ScenarioGraph *sg = s->getScenarioGraph();
        auto v_s = std::make_unique<MaxPlus::Vector>();

        // Iterate over all channels
        for (const auto& c: sg->getChannels())
        {
            // Append entry to reference schedule v_s for each initial token on
            // the channel
            for (uint j = 0; j < c->getInitialTokens(); j++)
            {
                v_s->put(v_s->getSize(), v_ref[c->getName()].get(j));
            }
        }

        return v_s;
    }

    /**
     * Throughput analysis
     * Compute the throughput of an FSM-based SADF graph while ignoring the FSM.
     *
     * Algorithm:
     * 1. Determine for each scenario s the eigenvector v_s and period T_s.
     * 2. Compute a suitable 'reference schedule'. This can be done by for example,
     *    taking the max over all eigenvectors. An alternative could be to use the
     *    ALAP eigenvector. Currently, the first option is used.
     * 3. Compute the delay for each scenario s using the reference schedule and the
     *    period of scenario s.
     * 4. The throughput is equal to max_s (d_s + T_s).
     */
    Throughput throughputAnalysisGraph(Graph *g)
    {
        std::map<ScenarioRef, std::unique_ptr<MaxPlus::Vector>> eigenvector;
        std::map<ScenarioRef, MPTime> period;
        std::map<ScenarioRef, MPTime> delay;
        std::map<CString, MaxPlus::Vector> v_ref;
        std::map<ScenarioRef, std::unique_ptr<MPExplore::Exploration>> mpe;
        auto thr = MPTime(0);

        // The set of scenarios...
        Scenarios& scenarios = g->getScenarios();

        // check if the graph is strongly connected and if all rewards are equal to one.
        if (!g->isStronglyConsistent())
        {
            throw ::CException("Cannot compute throughput with this method, because the FSM-SADF graph is not strongly consistent");
        }
        // check if the graph is strongly connected and if all rewards are equal to one.
        if (!g->hasUnitRewards())
        {
            throw ::CException("Cannot compute throughput with this method, because the FSM-SADF graph does not have unit rewards for all scenarios");
        }

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

        // Compute throughput
        for (const auto& s: scenarios)
        {
            thr = MP_MAX(thr, delay[s.get()] + period[s.get()]);
        }

        return static_cast<Throughput>((1.0) / static_cast<CDouble>(thr));
    }

}  // End namespace FSMSADF

