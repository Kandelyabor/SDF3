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
 *     08-09-11    :   Initial version.
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
#include <memory>

namespace FSMSADF {

/**
 * estimateStorageDist ()
 * Set the storage space constraints of the channels based on the
 * selected storage distribution.
 */
bool MemoryDimAlgoCompSoCPlatform::estimateStorageDist() {
    StorageDistribution maxSz;
    StorageDistribution minSz;
    std::map<CString, Rate> maxSrcRate;
    std::map<CString, Rate> maxDstRate;
    std::map<CString, uint> maxInitialToken;

    // Remove the existing platform bindings(except the initial binding)
    PlatformBindings* platformBindings = this->getPlatformBindings();
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
    auto *pbPtr = pb.get();
    platformBindings->push_back(std::move(pb));

    // Iterate over the scenarios (compute max buffer size)
    for (auto& [s, sd]: this->getSelectedStorageDistribution()) {
        ScenarioGraph *sg = s->getScenarioGraph();
        const StorageDistribution& d = *(sd);

        // Iterate over all channels
        for (const auto& c: sg->getChannels()) {
            maxSz[c.get()] = (std::max)(maxSz[c.get()], d[c.get()]);
        }
    }

    // Iterate over the scenario graphs (compute min buffer size)
    for (const auto& s: this->getApplicationGraph()->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();
        StorageDistributions ds = sg->getStorageDistributionsOfScenario(*s);

        // Find first storage distribution with non-zero throughput
        for (const auto& d: ds) {

            // Throughput larger than zero?
            if (d.getThroughput() > 0) {
                // Iterate over all channels
                for (const auto& c: sg->getChannels()) {
                    minSz[c.get()] = (std::max)(minSz[c.get()], d[c.get()]);
                }
                break;
            }
        }
    }

    // Iterate over the scenario graphs (compute maximum rate and initial token)
    for (const auto& s: this->getApplicationGraph()->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Iterate over all channels
        for (const auto& c: sg->getChannels()) {
            maxSrcRate[c->getName()] =
                    (std::max)(maxSrcRate[c->getName()], c->getSrcPort()->getRateOfScenario(*s));
            maxDstRate[c->getName()] =
                    (std::max)(maxDstRate[c->getName()], c->getDstPort()->getRateOfScenario(*s));
            maxInitialToken[c->getName()] =
                    (std::max)(maxInitialToken[c->getName()], c->getInitialTokens());
        }
    }

    // Iterate over the scenario graphs
    for (const auto& s: this->getApplicationGraph()->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();
        GraphBindingConstraints *gb = pbPtr->getGraphBindingConstraints();

        RepetitionVector repVec = s->computeRepetitionVector();

        // Iterate over all channels
        for (const auto& c: sg->getChannels()) {
            BufferSize sz;

            ChannelBindingConstraint *cbc =
                    gb->getConstraintsOfScenario(*s)->getConstraintsOfChannel(c->getName());

            // Select minimal deadlock free buffersize for channels mapped
            // to memory (connected actors are fired sequentially anyhow

            if (cbc->hasBufferLocationConstraint()
                && (cbc->getBufferLocation() == BufferLocation::Value::Mem)) {
                // The src buffer in compsoc is always the max rate.
                // In a shared-memory connection on CompSoC,
                // the Dst buffer is also the max rate and the
                // and the Mem buffer is placed in a shared memory.
                sz[static_cast<int>(BufferLocation::Value::Mem)] = minSz[c.get()];
                sz[static_cast<int>(BufferLocation::Value::Dst)] = maxDstRate[c->getName()];
                sz[static_cast<int>(BufferLocation::Value::Src)] = maxSrcRate[c->getName()];
            } else {
                // The src buffer in compsoc is always the max rate.
                // In a tile-to-tile connection on CompSoC,
                // the Dst and Mem buffers are merged and placed at the consumer.
                // Therefore, Dst is equal to Mem.
                sz[static_cast<int>(BufferLocation::Value::Mem)] = minSz[c.get()];
                sz[static_cast<int>(BufferLocation::Value::Dst)] = sz[static_cast<int>(BufferLocation::Value::Mem)];
                sz[static_cast<int>(BufferLocation::Value::Src)] = maxSrcRate[c->getName()];
            }

            // Set the buffer size of this channel in scenario s
            cbc->setBufferSize(sz);

            logMsg("Storage space of channel '" + c->getName() + "' in scenario graph '"
                   + sg->getName()
                   + "' (mem/src/dst): " + CString(sz[static_cast<int>(BufferLocation::Value::Mem)]) + " / "
                   + CString(sz[static_cast<int>(BufferLocation::Value::Src)]) + " / "
                   + CString(sz[static_cast<int>(BufferLocation::Value::Dst)]) + " tokens");
        }
    }

    return true;
}

} // namespace FSMSADF