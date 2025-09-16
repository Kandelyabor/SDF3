/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.cc
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Memory dimensioning
 *
 *  History         :
 *      16-11-11    :   Initial version.
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
#include "fsmsadf/analysis/throughput/maxplusautomaton.h"
#include "fsmsadf/resource_allocation/flow/mamps_platform/binding_aware_graph/binding_aware_graph.h"
#include "fsmsadf/resource_allocation/scheduling/earliest_deadline_first/edf.h"
#include "graph_binding.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * TileBindingAlgo()
 * Constructor.
 */
TileBindingAlgoMampsPlatform::TileBindingAlgoMampsPlatform() = default;

/**
 * ~TileBindingAlgo()
 * Destructor.
 */
TileBindingAlgoMampsPlatform::~TileBindingAlgoMampsPlatform() = default;

/**
 * analyzeThroughput ()
 * The function returns the throughput of the application graph
 * mapped onto the platform graph.
 */
Throughput TileBindingAlgoMampsPlatform::analyzeThroughput(const PlatformBinding& pb) {
    // Create binding-aware graph
    std::unique_ptr<BindingAwareGraphBase> g = std::make_unique<BindingAwareGraphMampsPlatform>(&pb);
    g->constructBindingAwareGraph(true);

    // Compute throughput of mapped graph
    // (two methods available: state-space or max-plus automaton)
    Throughput thr = throughputAnalysisGraphUsingMaxPlusAutomaton(*g);
    // thr = throughputAnalysisGraphUsingStateSpace(g);

    return thr;
}

/**
 * This function is used by #TileBindingAlgoBase to create GraphBinding object.
 * So if you want another GraphBinding object, override this function.
 *
 * @param pb a #PlatformBinging
 * @return a new #GraphBindingBase algorithm
 */
std::unique_ptr<GraphBindingBase> TileBindingAlgoMampsPlatform::createGraphBinding(const PlatformBinding& pb) {
    return std::make_unique<GraphBindingMampsPlatform>(&pb);
}
/**
 * constructTileSchedules()
 * Construct static-order and TDMA schedules for the application on all platform
 * tiles.
 */
bool TileBindingAlgoMampsPlatform::constructTileSchedules(const uint maxNrAppBindings) {
    EarliestDeadLineFirstScheduling edfScheduling;
    GraphBindings applicationBindings;
    ProcessorRefs processors;
    CId mappingId = 0;

    // Create a list with all processors in the platform
    for (const auto& t : this->getPlatformGraph()->getTiles()) {
        for (const auto& p : t->getProcessors()) {
            processors.push_back(p.get());
        }
    }

    // Create an application binding for each platform binding (except initial
    // binding)
    for (const auto& pb : *this->getPlatformBindings()) {
        if (!pb->isInitialBinding()) {
            // Create an application binding given the initial platform binding
            applicationBindings.push_back(createGraphBinding(*pb));
        }
    }

    // Create an empty set of new application bindings
    auto  newAppBindings = std::make_unique<GraphBindings>();

    // For each application binding b in applicationBindings
    for (const auto& b : applicationBindings) {
        std::set<Processor *> usedProcessors;

        // Create schedule for binding b
        logInfo("Creating schedule for binding '" + b->getPlatformBinding()->getName() + "'");

        // Create an empty set of new platform bindings bbNew
        auto bbNew = std::make_unique<GraphBindings>();

        // Iterate over all scenarios
        for (auto& [s, actorBindings] : b->getActorBindings()) {
            // Iterate over all actor bindings
            for (auto &[a, ab] : actorBindings) {
                Processor *p = ab.processor;

                // Compute minimum TDMA time slice allocation on p given actor
                // binding and scheduling type (i.e. preemptive, non-preemptive)
                if (p->hasSupportForPreemption()) {
                    b->setMinTDMAslice(
                            *s, *p, (std::max)(p->getContextSwitchOverhead() + 1, b->getMinTDMAslice(*s, *p)));
                } else {
                    b->setMinTDMAslice(
                            *s,
                            *p,
                            (std::max)(b->getMinTDMAslice(*s, *p),
                                (std::min)(p->getWheelsize(),
                                    p->getContextSwitchOverhead()
                                            + a->getExecutionTimeOfScenario(*s, p->getType()))));
                }

                // Compute maximum TDMA time slice allocation on p given
                // existing resource usage
                b->setMaxTDMAslice(*s, *p, b->getMaxTDMAslice(*s, *p));

                // Add processor to the set of used processors
                usedProcessors.insert(p);
            }
        }

        // Unified mapping requires that all scenarios have the same TDMA slice
        // allocation
        for (const auto& p : processors) {
            Time maxMinTDMAslice = 0;
            Time minMaxTDMAslice = TIME_MAX;

            // Find maximum minTDMAslice and minimum maxTDMAslice
            for (const auto& s : this->getApplicationGraph()->getScenarios()) {
                maxMinTDMAslice = (std::max)(maxMinTDMAslice, b->getMinTDMAslice(*s, *p));
                minMaxTDMAslice = (std::min)(minMaxTDMAslice, b->getMaxTDMAslice(*s, *p));
            }

            // Set maximum minTDMAslice and minimum maxTDMAslice
            for (const auto& s : this->getApplicationGraph()->getScenarios()) {
                // Is the processor used in at least one scenario?
                if (usedProcessors.find(p) != usedProcessors.end()) {
                    b->setMinTDMAslice(*s, *p, maxMinTDMAslice);
                    b->setMaxTDMAslice(*s, *p, minMaxTDMAslice);
                } else {
                    // No actor is mapped to the processor in any scenario. So,
                    // there is no need to allocate a TDMA slice on it.
                    b->setMinTDMAslice(*s, *p, 0);
                    b->setMaxTDMAslice(*s, *p, 0);
                }
            }
        }

        // Add b to the set of application bindings ab
        GraphBindings ab;
        ab.push_back(b->clone());

        // While application bindings left in ab
        while (!ab.empty()) {
            bool flag = true;

            // Take first application binding bs from set ab
            std::unique_ptr<GraphBindingBase> bs = std::move(ab.front());
            ab.pop_front();

            // Assign TDMA time slices for each scenario associated with the
            // application graph
            for (auto j = this->getApplicationGraph()->getScenarios().begin();
                 j != this->getApplicationGraph()->getScenarios().end() && flag;
                 j++) {
                Scenario* s = j->get();

                // For each processor p in the application binding bs associated
                // with as
                for (auto k = processors.begin(); k != processors.end() && flag; k++) {
                    Processor *p = *k;
                    Time slice = 0;

                    // Compute slice
                    slice = bs->getMinTDMAslice(*s, *p)
                            + (bs->getMaxTDMAslice(*s, *p) - bs->getMinTDMAslice(*s, *p)) / 2;

                    // Allocate time slice
                    if (!bs->allocateTDMAslice(*s, *p, slice)) {
                        /* The slice allocation was not successful */
                        int maxSlice = bs->getMaxTDMAslice(*s, *p);

                        slice = (std::min)(maxSlice, slice);

                        flag = bs->allocateTDMAslice(*s, *p, slice);
                    }

                    if (flag) {
                        // Create schedule for binding b
                        logMsg("Set time slice in scenario '" + s->getName() + "' on processor '"
                               + p->getTile()->getName() + "." + p->getName() + "' to "
                               + CString(bs->getAllocatedTDMAslice(*s, *p)) + " time-units");
                    } else {
                        // Create schedule for binding b
                        logMsg("Set time slice in scenario '" + s->getName() + "' on processor '"
                               + p->getTile()->getName() + "." + p->getName() + "' Failed");
                    }
                }
            }

            if (flag) {
                // Create EDF schedule for the platform binding
                auto bg = std::make_unique<BindingAwareGraphMampsPlatform>(bs->getPlatformBinding());
                FSMSADF::EarliestDeadLineFirstScheduling::schedule(*(bs->getPlatformBinding()), *bg);

                // Throughput of bs meets throughput constraint?
                if (isThroughputConstraintSatisfied(*(bs->getPlatformBinding()))) {
                    bool validNewBinding = true;

                    // Compute cost of bs
                    bs->computeParetoQuantities(this->getRepetitionVectors(), true);

                    // Add bs to bbNew
                    bs->getPlatformBinding()->setName(CString(mappingId++));
                    bbNew->push_back(bs->clone());

                    // Clone bs
                    std::unique_ptr<GraphBindingBase> bsNew = bs->clone();

                    // Decrease maximum TDMA time slice allocation for all
                    // processors
                    for (auto& [p, slices] : bsNew->getMaxTDMAslices()) {
                        for (auto& [s, n] : slices) {
                            bsNew->setMaxTDMAslice(*s, *p, bs->getAllocatedTDMAslice(*s, *p));

                            if (bsNew->getMaxTDMAslice(*s, *p) != 0
                                && bsNew->getMaxTDMAslice(*s, *p) - bsNew->getMinTDMAslice(*s, *p)
                                           <= 1) {
                                validNewBinding = false;
                            }
                        }
                    }

                    // Is this new binding possible within available time wheels?
                    if (validNewBinding) {
                        ab.push_back(std::move(bsNew));
                    }
                } else {
                    bool validNewBinding = true;

                    // Clone bs
                    std::unique_ptr<GraphBindingBase> bsNew = bs->clone();

                    // Increase minimum TDMA time slice allocation for all
                    // processor
                    for (auto& [p, slices] : bsNew->getMinTDMAslices()) {
                        // Increase minimum time slice of all scenarios
                        for (auto& [s, n] : slices) {
                            bsNew->setMinTDMAslice(*s, *p, bs->getAllocatedTDMAslice(*s, *p));

                            if (bsNew->getMaxTDMAslice(*s, *p) != 0
                                && bsNew->getMaxTDMAslice(*s, *p) - bsNew->getMinTDMAslice(*s, *p)
                                           <= 1) {
                                validNewBinding = false;
                            }
                        }
                    }

                    // Is this new binding possible within available time wheels?
                    if (validNewBinding) {
                        ab.push_back(std::move(bsNew));
                    }
                }
            }
        }

        // Perform pareto minimization on bbNew
        paretoMinimizationOfApplicationBindings(bbNew);

        // Add bbNew to newAppBindings
        for (const auto& b: *bbNew) {
            newAppBindings->push_back(b->clone());
}
    }

    // Select new platform bindings from newAppBindings using heuristic
    reduceApplicationBindings(newAppBindings, maxNrAppBindings);

    // Replace the existing platform bindings (except the initial binding)
    replacePlatformBindings(*newAppBindings);

    // Found no valid platform binding?
    if (newAppBindings->empty()) {
        return false;
}

    // Output the pareto dimensions of all mappings
    for (const auto& gb: *newAppBindings) {
        std::cerr << "Pareto quantities of mapping '";
        std::cerr << gb->getPlatformBinding()->getName() << "'" << std::endl;
        gb->getParetoQuantities().print(std::cerr);
    }

    return true;
}

} // End namespace FSMSADF
