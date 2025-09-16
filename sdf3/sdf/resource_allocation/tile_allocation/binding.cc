/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 11, 2006
 *
 *  Function        :   Base class for all binding algorithms
 *
 *  History         :
 *      11-04-06    :   Initial version.
 *
 * $Id: binding.cc,v 1.5 2008-03-06 10:49:45 sander Exp $
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

#include "binding.h"
#include "sdf/transform/model/autoconc.h"
#include "sdf/transform/model/buffersize.h"
#include <cmath>
#include <memory>

namespace SDF
{


    /**
     * analyzeThroughputApplication ()
     * The function returns the throughput of the application graph
     * considering the absence of auto-concurrency, the buffer constraints
     * and the mapping of actors to the fastest processor.
     */
    double Binding::analyzeThroughputApplication()
    {
        SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
        std::unique_ptr<TimedSDFgraph> g = nullptr;
        std::unique_ptr<TimedSDFgraph> gr = nullptr;
        double thrGraph = NAN;

        // Model absence of auto-concurrency in SDFG
        g = TimedSDFgraph::dynamicCast(modelAutoConcurrencyInSDFgraph(*appGraph, 1));

        // Set buffer size of each channel to max(mem, src+dst)
        for (auto iter = g->channelsBegin();
             iter != g->channelsEnd(); iter++)
        {
            auto* c = dynamic_cast<TimedSDFchannel*>((*iter).get());
            TimedSDFchannel::BufferSize sz = c->getBufferSize();

            if (sz.mem < sz.src + sz.dst)
            {
                sz.sz = sz.src + sz.dst;
                c->setBufferSize(sz);
            }
        }

        // Model buffer constraints in SDFG
        gr = modelBufferSizeInSDFgraph(*g);

        // Select fastest processor for each actor as the default processor
        for (auto *iter: gr->getActorRefs())
        {
            auto *a = dynamic_cast<TimedSDFactor *>(iter);
            TimedSDFactor::Processor *proc = nullptr;

            for (const auto& p: a->getProcessors())
            {
                if (proc == nullptr || proc->execTime > p->execTime) {
                    proc = p.get();
                }
            }

            if (proc != nullptr) {
                a->setDefaultProcessor(proc->type);
}
        }

        // Compute throughput of the SDFG
        thrGraph = SDFstateSpaceThroughputAnalysis::analyze(*gr);

        return thrGraph;
    }

    /**
     * analyzeThroughput ()
     * The function returns the throughput of the application graph
     * mapped onto the platform graph.
     */
    double Binding::analyzeThroughput(std::vector<double> &tileUtilization)
    {
        SDFstateSpaceBindingAwareThroughputAnalysis thrAnalysisAlgo;
        double thrGraph = NAN;

        // Create binding-aware SDFG
        BindingAwareSDFG bindingAwareSDFG(appGraph, archGraph, flowType);

        // Compute throughput of mapped SDF graph
        thrGraph = SDFstateSpaceBindingAwareThroughputAnalysis::analyze(bindingAwareSDFG, tileUtilization);

        return thrGraph;
    }

    /**
     * isThroughputConstraintSatisfied ()
     * Check wether or not the throughput constraint is satisfied.
     */
    bool Binding::isThroughputConstraintSatisfied()
    {
        double thrGraph = NAN;
        double thrConstraint = NAN;
        std::vector<double> tileUtilization;

        // Compute throughput of mapped SDF graph
        thrGraph = analyzeThroughput(tileUtilization);
        thrConstraint = appGraph->getThroughputConstraint();

#ifdef VERBOSE
        std::cerr << "Throughput: " << thrGraph << std::endl;
        std::cerr << "Constraint: " << thrConstraint << std::endl;
#endif

        // Is throughput constraint met?
        if (thrConstraint > thrGraph)
        {
#ifdef VERBOSE
            std::cerr << "Failed throughput constraint." << std::endl;
#endif

            return false;
        }

#ifdef VERBOSE
        std::cerr << "Throughput constraint satisfied." << std::endl;
#endif

        return true;
    }

    /**
     * minimizeStaticOrderSchedules ()
     * The function minimizes the length of all static-order schedules.
     */
    void Binding::minimizeStaticOrderSchedules(PlatformGraph *archGraph)
    {
        // Iterate over the tiles
        for (const auto& t: archGraph->getTiles())
        {
            Processor *p = t->getProcessor();

            // Processor inside the tile
            if (p != nullptr)
            {
                StaticOrderSchedule &s = p->getSchedule();

                // Minimize the schedule
                s.minimize();

                logMsg("Reduced schedule on '" + t->getName() + "' to "
                       + CString(s.size()) + " states.");
            }
        }
    }

}//namespace SDF
