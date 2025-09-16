/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   constraint.cc
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
 * $Id: constraint.cc,v 1.3 2008-03-06 10:49:44 sander Exp $
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
#include "sdf/base/algo/cycle.h"
#include "sdf/transform/model/buffersize.h"
#include <algorithm>
#include <memory>

namespace SDF {
/**
 * estimateLatencyConstraints ()
 * Estimate the minimal latency between the production and consumption of
 * a token on a channel.
 */
void SDF3Flow::estimateLatencyConstraints() {
    double throughputDist = selectedStorageDistribution->thr;
    RepetitionVector repVec;
    SDFgraphCycles cycles;

    // Output current state of the flow
    logInfo("Estimate latency constraint.");

    // Initialize latency of all channels to infinity
    for (const auto &iter : getAppGraph()->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

        c->setMinLatency(UINT_MAX);
    }

    // Model storage constraints in the graph
    std::unique_ptr<TimedSDFgraph> storageAppGraph = modelBufferSizeInSDFgraph(*getAppGraph());

    // Compute repetition vector
    repVec = computeRepetitionVector(*storageAppGraph);

    // Actor weight (minimum execution time)
    auto actorWeight = std::make_unique<double[]>(appGraph->nrActors());
    for (const auto &iter : appGraph->getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        double execTime = UINT_MAX;

        // Minimum execution time
        for (const auto &p : a->getProcessors()) {
            execTime = (std::min)(execTime, p->execTime);
        }

        actorWeight[a->getId()] = execTime * repVec[a->getId()];
    }

    // Channel weight
    auto channelWeight = std::make_unique<std::unique_ptr<double[]>[]>(appGraph->nrActors());
    for (uint i = 0; i < appGraph->nrActors(); i++) {
        channelWeight[i] = std::make_unique<double[]>(appGraph->nrActors());

        for (uint j = 0; j < appGraph->nrActors(); j++) {
            channelWeight[i][j] = 0;
        }
    }
    for (const auto &c : storageAppGraph->getChannels()) {
        CId srcActorId = c->getSrcActor()->getId();
        CId dstActorId = c->getDstActor()->getId();

        // Channel weight
        auto weight = static_cast<double>(c->getInitialTokens() // NOLINT(bugprone-integer-division)
                                          / c->getDstPort()->getRate());

        // Weight of this channel larger than of any other channel between
        // same source and destination seen so far
        channelWeight[srcActorId][dstActorId] =
                (std::max)(channelWeight[srcActorId][dstActorId], weight);
    }

    // Find all simple cycles in the graph
    cycles = findSimpleCycles(*storageAppGraph);

    // Compute the cycle mean for each cycle and the latency for all channels on
    // the cycle
    for (auto iter = cycles.begin(); iter != cycles.end(); iter++) {
        SDFactorRefsCIter actorIter;
        double weightChannels = 0;
        double weightActors = 0;
        SDFactorRefs cycle = *iter;

        actorIter = cycle.begin();
        SDFactor *srcActor = *actorIter++;
        SDFactor *dstActor = nullptr;
        while (actorIter != cycle.end()) {
            dstActor = *actorIter;
            // Add weight of source actor and channel from source to destination
            weightActors += actorWeight[srcActor->getId()];
            weightChannels += channelWeight[srcActor->getId()][dstActor->getId()];

            // Next channel in the cycle
            srcActor = dstActor;
            actorIter++;
        }

        // srcActor points to last actor in the cycle. Its weight must be added,
        // as well as the weight of the channel to the first actor in the cycle.
        dstActor = cycle.front();
        weightActors += actorWeight[srcActor->getId()];
        weightChannels += channelWeight[srcActor->getId()][dstActor->getId()];

        // Cycle mean
        double cycleMean = weightActors / weightChannels;

        // Latency for every channel on the cycle
        int latency = static_cast<int>(ceil((((1.0 / cycleMean) - throughputDist) / throughputDist)
                                            * (weightActors / static_cast<double>(cycle.size()))));

        // Latency should always be a non-negative value
        constexpr int MaxLatency = 100000;
        latency = (std::max)(latency, 0);
        latency = (std::min)(latency, MaxLatency);

        // Update latency for all channels on the cycle
        actorIter = cycle.begin();
        srcActor = *actorIter;
        while (actorIter != cycle.end()) {
            // Destination actor
            actorIter++;
            if (actorIter == cycle.end()) {
                dstActor = cycle.front();
            } else {
                dstActor = *actorIter;
            }

            // Find all channels from the source to the destination actor
            for (const auto &iter : getAppGraph()->getChannels()) {
                auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

                if (c->getSrcActor()->getId() == srcActor->getId()
                    && c->getDstActor()->getId() == dstActor->getId()) {
                    if (c->getMinLatency() > static_cast<uint>(latency)) {
                        c->setMinLatency(latency);
                    }
                }
            }

            // Destination becomes next source actor
            srcActor = dstActor;
        }
    }

    if (logHasLevel()) {
        // Output all latencies as messages to terminal
        for (const auto &iter : getAppGraph()->getChannels()) {
            auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

            logMsg("Latency channel '" + c->getName() + "': " + CString(c->getMinLatency())
                   + " time-units");
        }
    }

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowEstimateBandwidthConstraint);
}

/**
 * estimateBandwidthConstraints ()
 * Estimate the required minimal bandwidth when binding a channel to a
 * connection.
 */
void SDF3Flow::estimateBandwidthConstraints() {
    RepetitionVector repVec = computeRepetitionVector(*getAppGraph());

    // Output current state of the flow
    logInfo("Compute bandwidth constraint.");

    for (const auto &iter : getAppGraph()->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

        double bandwidth = c->getSrcPort()->getRate() * repVec[c->getSrcActor()->getId()]
                           * c->getTokenSize() * selectedStorageDistribution->thr;

        c->setMinBandwidth(bandwidth);

        logMsg("Bandwidth channel '" + c->getName() + "': " + CString(c->getMinBandwidth())
               + " bits/time-units");
    }

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowBindSDFGtoTile);
}
} // namespace SDF