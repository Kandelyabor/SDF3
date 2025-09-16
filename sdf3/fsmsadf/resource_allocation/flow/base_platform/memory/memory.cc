/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 22, 2009
 *
 *  Function        :   Memory dimensioning
 *
 *  History         :
 *      22-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: memory.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "memory.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

using namespace SDF;

/**
 * initialize()
 * The function initializes the memory dimensioning algorithm.
 */
void MemoryDimAlgoBase::init(BindingMethod bm) {
    // Has application graph?
    if (applicationGraph == nullptr) {
        throw CException("No application graph given.");
    }

    // Application graph has no isolated scenarios?
    if (!applicationGraph->hasIsolatedScenarios()) {
        throw CException("Graph has no isolated scenarios.");
    }

    // Has platform graph?
    if (platformGraph == nullptr) {
        throw CException("No platform graph given.");
    }

    // Has list of platform bindings?
    if (platformBindings == nullptr) {
        throw CException("No list of platform bindings given.");
    }

    // No platform bindings given?
    if (platformBindings->empty()) {
        // Create an empty initial platform binding
        platformBindings->push_back(std::make_unique<PlatformBinding>(
                GraphComponent(nullptr, 0, "initial"), platformGraph, applicationGraph, bm));
    }

    // No storage distributions selected
    selectedStorageDistribution.clear();
}

/**
 * getPlatformBinding()
 * The function returns a pointer to a platform binding object with the
 * specified name. An exception is thrown when no such object exists.
 */
PlatformBinding* MemoryDimAlgoBase::getPlatformBinding(const CString &name) const {
    for (const auto& pb : *platformBindings) {
        if (pb->getName() == name) {
            return pb.get();
        }
    }

    throw CException("No platform binding exists with name '" + name + "'");
}

/**
 * getInitialPlatformBinding()
 * The function returns a pointer to the initial platform binding object. An
 * exception is thrown when no such object exists.
 */
PlatformBinding* MemoryDimAlgoBase::getInitialPlatformBinding() const {
    for (const auto& pb : *platformBindings) {
        if (pb->isInitialBinding()) {
            return pb.get();
        }
    }

    throw CException("No initial platform binding exists");
}

/**
 * computeStorageDist ()
 * Compute trade-off space between storage space allocated to channels and
 * maximal throughput of the application graph.
 */
bool MemoryDimAlgoBase::computeStorageDist() {
    SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;

    // Application graph has no isolated scenarios?
    if (!applicationGraph->hasIsolatedScenarios()) {
        throw CException("Graph has no isolated scenarios.");
    }

    // Iterate over the scenarios
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Create an SDF graph for this scenario
        std::unique_ptr<TimedSDFgraph> g = sg->extractSDFgraph(*s);

        // Assume absence of auto-concurrency (i.e. add self-edge to actors)
        for (const auto& a : g->getActors()) {
            g->createChannel(a.get(), 1, a.get(), 1, 1);
        }

        // Compute throughput/storage-space trade-offs
        StorageDistributionSet* dSet = bufferAnalysisAlgo.analyze(*g);

        // Convert storage distributions to FSM-SADF type
        StorageDistributions ds;
        while (dSet != nullptr) {
            SDF::StorageDistribution* distributions = dSet->distributions.get();

            while (distributions != nullptr) {
                StorageDistribution d;

                // Set throughput of the distribution
                d.setThroughput(distributions->thr);

                // Set buffer size of the channels
                for (const auto& c : sg->getChannels()) {
                    d[c.get()] = distributions->sp[c->getId()];
                }

                // Add distribution to the set of distributions
                ds.insert(d);

                // Next distribution
                distributions = distributions->next.get();
            }

            // Next set
            dSet = dSet->next.get();
        }

        // Storage distribution with throughput 0 not present?
        if (ds.empty() || ds.begin()->getThroughput() != 0) {
            StorageDistribution dz;

            // Set throughput of the distribution
            dz.setThroughput(0);

            // Set buffer size of the channels
            for (const auto& c : sg->getChannels()) {
                dz[c.get()] = 0;
            }

            // Add distribution to the set of distributions
            ds.insert(dz);
        }

        // Assign storage distributions to the scenario graph
        sg->setStorageDistributionsOfScenario(*s, ds);

        // Initialize the selected storage distribution of this scenario to the
        // first storage distribution
        selectedStorageDistribution[s.get()] = sg->getStorageDistributionsOfScenario(*s).begin();

    }

    return true;
}

/**
 * selectStorageDist ()
 * Select storage distribution from the trade-off space.
 */
bool MemoryDimAlgoBase::selectStorageDist() {
    bool selectedNewDistribution = false;

    // Iterate over the scenarios
    for (auto i = selectedStorageDistribution.begin(); i != selectedStorageDistribution.end();
         i++) {
        ScenarioRef s = i->first;
        ScenarioGraph *sg = s->getScenarioGraph();
        auto di = i->second;

        // Select next storage distribution
        di++;

        // Larger storage distribution available?
        if (di != sg->getStorageDistributionsOfScenario(*s).end()) {
            i->second++;

            // Selected a new storage distribution for at least one graph
            selectedNewDistribution = true;
        }

        logMsg(CString("Selected distribution of size ") + CString(i->second->getSize())
               + " with throughput " + CString(i->second->getThroughput()) + " for scenario graph '"
               + sg->getName() + "'");
    }

    // Selected no new storage distribution for any scenario graph?
    if (!selectedNewDistribution) {
        logMsg("No storage distribution left.");
        return false;
    }

    return true;
}

/**
 * estimateStorageDist ()
 * Set the storage space constraints of the channels based on the
 * selected storage distribution.
 */
bool MemoryDimAlgoBase::estimateStorageDist() { // NOLINT(*cognitive-complexity)
    StorageDistribution maxSz;
    StorageDistribution minSz;
    std::map<CString, Rate> maxSrcRate;
    std::map<CString, Rate> maxDstRate;
    std::map<CString, uint> maxInitialToken;

    // Remove the existing platform bindings (except the initial binding)
    for (auto i = platformBindings->begin(); i != platformBindings->end();) {
        PlatformBinding* pb = i->get();

        if (pb->isInitialBinding()) {
            i++;
        } else {
            platformBindings->erase(i++);
        }
    }

    // Create a new platform binding based on the initial binding
    PlatformBinding* initPb = getInitialPlatformBinding();
    std::unique_ptr<PlatformBinding> pb = initPb->clone(*initPb);
    pb->setName("0");
    platformBindings->push_back(std::move(pb));

    // Iterate over the scenarios (compute max buffer size)
    for (auto& [s, i_s] : selectedStorageDistribution) {
        ScenarioGraph *sg = s->getScenarioGraph();
        StorageDistribution d = *i_s;

        // Iterate over all channels
        for (const auto& c : sg->getChannels()) {
            maxSz[c.get()] = (std::max)(maxSz[c.get()], d[c.get()]);
        }
    }

    // Iterate over the scenario graphs (compute min buffer size)
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();
        StorageDistributions ds = sg->getStorageDistributionsOfScenario(*s);

        // Find first storage distribution with non-zero throughput
        for (const auto &d : ds) {

            // Throughput larger than zero?
            if (d.getThroughput() > 0) {
                // Iterate over all channels
                for (const auto& c : sg->getChannels()) {
                    minSz[c.get()] = (std::max)(minSz[c.get()], d[c.get()]);
                }
                break;
            }
        }
    }

    // Iterate over the scenario graphs (compute maximum rate and initial token)
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Iterate over all channels
        for (const auto& c : sg->getChannels()) {
            maxSrcRate[c->getName()] =
                    (std::max)(maxSrcRate[c->getName()], c->getSrcPort()->getRateOfScenario(*s));
            maxDstRate[c->getName()] =
                    (std::max)(maxDstRate[c->getName()], c->getDstPort()->getRateOfScenario(*s));
            maxInitialToken[c->getName()] =
                    (std::max)(maxInitialToken[c->getName()], c->getInitialTokens());
        }
    }

    // Iterate over the scenario graphs
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();
        GraphBindingConstraints *gb = pb->getGraphBindingConstraints();

        // Iterate over all channels
        for (const auto& c : sg->getChannels()) {
            BufferSize sz;

            // Select minimal deadlock free buffersize for channels mapped
            // to memory (connected actors are fired sequentially anyhow)
            sz[static_cast<int>(BufferLocation::Value::Mem)] = minSz[c.get()];

            // Compute source and destination buffer sizes
            sz[static_cast<int>(BufferLocation::Value::Src)] =
                    maxSz[c.get()] / 2 + (maxSz[c.get()] % 2 > 0 ? 1 : 0);
            sz[static_cast<int>(BufferLocation::Value::Dst)] =
                    maxSz[c.get()] / 2 + (maxSz[c.get()] % 2 > 0 ? 1 : 0);

            // Correct buffer sizes for constraints
            sz[static_cast<int>(BufferLocation::Value::Src)] = (std::max)(sz[static_cast<int>(BufferLocation::Value::Src)], maxInitialToken[c->getName()]);
            sz[static_cast<int>(BufferLocation::Value::Src)] = (std::max)(sz[static_cast<int>(BufferLocation::Value::Src)], maxSrcRate[c->getName()]);
            sz[static_cast<int>(BufferLocation::Value::Dst)] = (std::max)(sz[static_cast<int>(BufferLocation::Value::Dst)], maxDstRate[c->getName()]);

            // Set the buffer size of this channel in scenario s
            gb->getConstraintsOfScenario(*s)
                    ->getConstraintsOfChannel(c->getName())
                    ->setBufferSize(sz);

            logMsg("Storage space of channel '" + c->getName() + "' in scenario graph '"
                   + sg->getName()
                   + "' (mem/src/dst): " + CString(sz[static_cast<int>(BufferLocation::Value::Mem)]) + " / "
                   + CString(sz[static_cast<int>(BufferLocation::Value::Src)]) + " / "
                   + CString(sz[static_cast<int>(BufferLocation::Value::Dst)]) + " tokens");
        }
    }

    return true;
}

} // End namespace FSMSADF
