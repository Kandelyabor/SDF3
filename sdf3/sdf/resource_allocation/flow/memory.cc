/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *
 * $Id: memory.cc,v 1.5 2008-03-06 13:59:05 sander Exp $
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

#include "flow.h"
#include <algorithm>
#include <memory>

namespace SDF {
// Search the complete storage-space throughput trade-off space in the
// computeStorageDistributions function. This makes it possible to quickly
// select a distribution in the next step of the flow (i.e. faster iterations).
// The alternative is to search the space only when a new Pareto point is
// needed.
// #define _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE

/**
 * modelNonLocalMemoryAccesses ()
 * Model non-local memory accesses in the application graph.
 */
void SDF3Flow::modelNonLocalMemoryAccesses() {
    std::cerr << "[WARNING] model non-local memory access not implemented." << std::endl;

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowComputeStorageDist);
}

/**
 * computeStorageDistributions ()
 * Compute trade-off space between storage space allocated to channels and
 * maximal throughput of the application graph.
 */
void SDF3Flow::computeStorageDistributions() {
    // Output current state of the flow
    logInfo("Compute storage-space / throughput trade-offs.");

    // Assume absence of auto-concurrency (i.e. add self-edge to every actor)
    std::unique_ptr<TimedSDFgraph> g = getAppGraph()->clone();
    for (const auto& a : g->getActors()) {
        g->createChannel(a.get(), 1, a.get(), 1, 1);
    }

    // Set processor with lowest execution time as the default processor
    for (const auto& iter : g->getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        TimedSDFactor::Processor *defaultProc = nullptr;

        for (const auto& p : a->getProcessors()) {
            if (defaultProc == nullptr || defaultProc->execTime > p->execTime) {
                defaultProc = p.get();
            }
        }

        a->setDefaultProcessor(defaultProc->type);
    }

#ifdef _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE
    // Compute trade-off space
    minStorageDistributions = bufferAnalysisAlgo.analyze(g);

    // Update storage-space size to ignore added self-edges. These channels have
    // space for 2 tokens.
    distributionSet = minStorageDistributions;
    while (distributionSet != nullptr) {
        // Only update size when it is larger than zero
        if (distributionSet->sz > 0) {
            // Update size of the set
            distributionSet->sz = distributionSet->sz - 2 * g->nrActors();

            // Update all distributions in the set
            distr = distributionSet->distributions;
            while (distr != nullptr) {
                // Update size of the distribution
                distr->sz = distributionSet->sz;

                // Next distribution
                distr = distr->next;
            }
        }

        // Next set
        distributionSet = distributionSet->next;
    }

    // No storage distribution selected so far
    selectedStorageDistributionSet = nullptr;
    selectedStorageDistribution = nullptr;

#else // _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE

    // Initialize the exploration algorithm
    bufferAnalysisAlgo.initSearch(*g);

    // Bit of a hack to make the analysis algorithm own the SDFG to prevent it being destructed prematurely.
    bufferAnalysisAlgo.ownGraph(std::move(g));

    // No storage distribution found and selected so far
    minStorageDistributions = nullptr;
    selectedStorageDistributionSet = nullptr;
    selectedStorageDistribution = nullptr;

    // Search the storage-space / throughput trade-off space for a Pareto point
    // with a throughput equal or larger than the constraint
    do {
        // Find next pareto point in the space
        selectedStorageDistributionSet = bufferAnalysisAlgo.findNextStorageDistributionSet();
        // No new pareto point discovered?
        if (selectedStorageDistributionSet == nullptr) {
            logError("No storage distribution meets throughput constraint.");
            setNextStateOfFlow(FlowState::FlowFailed);
            return;
        }
        // Update storage-space size to ignore added self-edges. These channels
        // have space for 2 tokens. Note that only  distributions with a size
        // larger than zero must be updated.
        StorageDistributionSet* distributionSet = selectedStorageDistributionSet;
        if (distributionSet->sz > 0) {
            // Update size of the set
            distributionSet->sz = distributionSet->sz - 2 * bufferAnalysisAlgo.getGraph()->nrActors();

            // Update all distributions in the set
            StorageDistribution* distr = distributionSet->distributions.get();
            while (distr != nullptr) {
                // Update size of the distribution
                distr->sz = distributionSet->sz;

                // Next distribution
                distr = distr->next.get();
            }
        }

        // Is this the first point ever to be found?
        if (minStorageDistributions == nullptr) {
            minStorageDistributions = selectedStorageDistributionSet;
        }
    } while (selectedStorageDistributionSet->thr
             < getAppGraph()->getThroughputConstraint());

#endif

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowSelectStorageDist);
}

/**
 * selectStorageDistribution ()
 * Select storage distribution from the trade-off space.
 */
void SDF3Flow::selectStorageDistribution() {
    SDFthroughput thrConstraint = getAppGraph()->getThroughputConstraint();

    // Output current state of the flow
    logInfo("Select storage distribution.");

#ifdef _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE

    // Currently no storage distribution set selected?
    if (selectedStorageDistributionSet == nullptr) {
        // Find first set with throughput equal or larger
        // then constraint
        selectedStorageDistributionSet = minStorageDistributions;
        while (selectedStorageDistributionSet != nullptr
               && selectedStorageDistributionSet->thr < thrConstraint.value()) {
            selectedStorageDistributionSet = selectedStorageDistributionSet->next;
        }

        // No storage distribution set found which has sufficiently large
        // throughput?
        if (selectedStorageDistributionSet == nullptr) {
            logError("No storage distribution meets throughput constraint.");
            setNextStateOfFlow(FlowFailed);
            return;
        }
    }

    // Select next storage distribution within the set (advance to next set when
    // current set is exhausted)
    do {
        if (selectedStorageDistribution == nullptr) {
            selectedStorageDistribution = selectedStorageDistributionSet->distributions;
        } else {
            selectedStorageDistribution = selectedStorageDistribution->next;
        }

        // All storage distribution within current set (size) tried?
        if (selectedStorageDistribution == nullptr) {
            selectedStorageDistributionSet = selectedStorageDistributionSet->next;
        }

        // New set of larger storage distributions does not exist?
        if (selectedStorageDistributionSet == nullptr) {
            logError("No storage distribution left.");
            setNextStateOfFlow(FlowFailed);
            return;
        }
    } while (selectedStorageDistribution == nullptr);

#else // _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE
    StorageDistributionSet* distributionSet = nullptr;
    StorageDistribution* distr = nullptr;

    // Select next storage distribution within the set (advance to next set when
    // current set is exhausted)
    do {
        if (selectedStorageDistribution == nullptr) {
            selectedStorageDistribution = selectedStorageDistributionSet->distributions.get();
        } else {
            selectedStorageDistribution = selectedStorageDistribution->next.get();
        }

        // All storage distribution within current set (size) tried?
        if (selectedStorageDistribution == nullptr) {
            selectedStorageDistributionSet = bufferAnalysisAlgo.findNextStorageDistributionSet();

            // New set of larger storage distributions does not exist?
            if (selectedStorageDistributionSet == nullptr) {
                logError("No storage distribution left.");
                setNextStateOfFlow(FlowState::FlowFailed);
                return;
            }

            // Update storage-space size to ignore added self-edges. These
            // channels have space for 2 tokens. Note that only  distributions
            // with a size larger than zero must be updated.
            distributionSet = selectedStorageDistributionSet;
            if (distributionSet->sz > 0) {
                // Update size of the set
                distributionSet->sz = distributionSet->sz - 2 * getAppGraph()->nrActors();

                // Update all distributions in the set
                distr = distributionSet->distributions.get();
                while (distr != nullptr) {
                    // Update size of the distribution
                    distr->sz = distributionSet->sz;

                    // Next distribution
                    distr = distr->next.get();
                }
            }
        }
    } while (selectedStorageDistribution == nullptr);

#endif

    logMsg(CString("Selected distribution of size ") + CString(selectedStorageDistribution->sz)
           + " with throughput " + CString(selectedStorageDistribution->thr));

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowEstimateStorageDist);
}

/**
 * estimateStorageConstraints ()
 * Set the storage space constraints of the channels based on the
 * selected storage distribution.
 */
void SDF3Flow::estimateStorageConstraints() {
    TimedSDFchannel::BufferSize buf;

    // Output current state of the flow
    logInfo("Estimate storage distribution per connection.");

    for (const auto& iter: getAppGraph()->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

        // Storage size for the channel
        buf.sz = static_cast<int>(selectedStorageDistribution->sp[c->getId()]);

        // Compute source and destination buffer sizes
        if (getFlowType() == SDFflowType::SDFflowTypeMPFlow) {
            buf.src = buf.sz;
            buf.dst = static_cast<int>(c->getDstPort()->getRate());
        } else {
            buf.src = buf.sz / 2 + (buf.sz % 2 > 0 ? 1 : 0);
            buf.dst = buf.sz / 2 + (buf.sz % 2 > 0 ? 1 : 0);
        }

        // Correct buffer sizes for constraints
        buf.src = (std::max)(buf.src, static_cast<int>(c->getSrcPort()->getRate()));
        buf.src = (std::max)(buf.src, static_cast<int>(c->getInitialTokens()));
        buf.dst = (std::max)(buf.dst, static_cast<int>(c->getDstPort()->getRate()));

        // Select minimal deadlock free buffersize for channels mapped
        // to memory (connected actors are fired sequentially anyhow)
        StorageDistributionSet* minDeadlockFreeDistributions = minStorageDistributions;
        while (minDeadlockFreeDistributions != nullptr && minDeadlockFreeDistributions->thr == 0) {
            minDeadlockFreeDistributions = minDeadlockFreeDistributions->next.get();
        }
        buf.mem = static_cast<int>(minDeadlockFreeDistributions->distributions->sp[c->getId()]);

        c->setBufferSize(buf);

        logMsg("Storage space channel '" + c->getName() + "' (mem/src/dst): " + CString(buf.mem)
               + " / " + CString(buf.src) + " / " + CString(buf.dst) + " tokens");
    }

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowEstimateLatencyConstraint);
}

} // namespace SDF
