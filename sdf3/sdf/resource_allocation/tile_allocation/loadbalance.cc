/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   loadbalance.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 11, 2006
 *
 *  Function        :   Load-balance algorithm
 *
 *  History         :
 *      11-04-06    :   Initial version.
 *
 * $Id: loadbalance.cc,v 1.4.2.2 2010-04-25 02:08:52 mgeilen Exp $
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

#include "loadbalance.h"
#include "sdf/base/algo/cycle.h"
#include "sdf/resource_allocation/scheduling/list_scheduler.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace SDF {

// Scheduling strategy (default: list scheduling)
// #define _CREATE_STATIC_ORDER_SCHEDULES_WITH_PRIORITIES

/**
 * LoadBalanceBinding ()
 * Constructor.
 */
LoadBalanceBinding::LoadBalanceBinding(SDFflowType flowType) : // NOLINT(*member-init)
    Binding(flowType), maxCycleMean(nullptr), tileLoad(nullptr) {
    setConstantsTileCostFunction(
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
}

/**
 * ~LoadBalanceBinding ()
 * Destructor.
 */
LoadBalanceBinding::~LoadBalanceBinding() = default;

/**
 * setAppGraph ()
 * Set the application graph. This function is overloaded from the base class.
 */
void LoadBalanceBinding::setAppGraph(TimedSDFgraph* g) {
    // Class function in base class
    Binding::setAppGraph(g);

    // Compute repetition vector
    repVec = computeRepetitionVector(*(this->getAppGraph()));

    // Cleanup existing cycle mean estimate
    maxCycleMean.reset();

    // Initialize vector containing all actor to tile bindings
    actorTileBinding.resize(this->getAppGraph()->nrActors());
    for (uint i = 0; i < this->getAppGraph()->nrActors(); i++) {
        actorTileBinding[i] = nullptr;
    }
}

/**
 * getConstantsTileCostFunction ()
 * The function returns all constants used in the tile cost function.
 */
void LoadBalanceBinding::getConstantsTileCostFunction(double &a,
                                                      double &b,
                                                      double &c,
                                                      double &d,
                                                      double &e,
                                                      double &f,
                                                      double &g,
                                                      double &k,
                                                      double &l,
                                                      double &m,
                                                      double &n,
                                                      double &o,
                                                      double &p,
                                                      double &q) const {
    a = cnst_a;
    b = cnst_b;
    c = cnst_c;
    d = cnst_d;
    e = cnst_e;
    f = cnst_f;
    g = cnst_g;
    k = cnst_k;
    l = cnst_l;
    m = cnst_m;
    n = cnst_n;
    o = cnst_o;
    p = cnst_p;
    q = cnst_q;
}

/**
 * setConstantsTileCostFunction ()
 * The function sets all constants used in the tile cost function.
 */
void LoadBalanceBinding::setConstantsTileCostFunction(double a,
                                                      double b,
                                                      double c,
                                                      double d,
                                                      double e,
                                                      double f,
                                                      double g,
                                                      double k,
                                                      double l,
                                                      double m,
                                                      double n,
                                                      double o,
                                                      double p,
                                                      double q) {
    cnst_a = a;
    cnst_b = b;
    cnst_c = c;
    cnst_d = d;
    cnst_e = e;
    cnst_f = f;
    cnst_g = g;
    cnst_k = k;
    cnst_l = l;
    cnst_m = m;
    cnst_n = n;
    cnst_o = o;
    cnst_p = p;
    cnst_q = q;
}

/**
 * estimateMaxCycleMean ()
 * The function estimates the maximum cycle mean of all actors in the
 * application graph.
 */
void LoadBalanceBinding::estimateMaxCycleMean() {
    SDFgraphCycles cycles;

    // Cleanup existing estimates
    maxCycleMean.reset();

    // Initialize
    maxCycleMean = std::make_unique<double[]>(this->getAppGraph()->nrActors());
    for (uint i = 0; i < this->getAppGraph()->nrActors(); i++) {
        maxCycleMean[i] = 0;
    }

    // Actor weight
    auto actorWeight = std::make_unique<double[]>(this->getAppGraph()->nrActors());
    for (const auto& iter : this->getAppGraph()->getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        double execTime = 0;

        // Average execution time
        for (const auto& p : a->getProcessors()) {
            execTime += p->execTime;
        }
        execTime = execTime / a->nrProcessors();

        actorWeight[a->getId()] = execTime * repVec[a->getId()];
    }

    // Channel weight
    auto channelWeight = std::make_unique<std::unique_ptr<double[]>[]>(this->getAppGraph()->nrActors());
    for (uint i = 0; i < this->getAppGraph()->nrActors(); i++) {
        channelWeight[i] = std::make_unique<double[]>(this->getAppGraph()->nrActors());

        for (uint j = 0; j < this->getAppGraph()->nrActors(); j++) {
            channelWeight[i][j] = 0;
        }
    }
    for (const auto& c : this->getAppGraph()->getChannels()) {
        CId srcActorId = c->getSrcActor()->getId();
        CId dstActorId = c->getDstActor()->getId();
        double weight = NAN;

        // Channel weight
        weight = static_cast<double>(c->getInitialTokens()) / static_cast<double>(c->getDstPort()->getRate());

        // Weight of this channel larger than of any other channel between
        // same source and destination seen so far
        channelWeight[srcActorId][dstActorId] = (std::max)(channelWeight[srcActorId][dstActorId], weight);
    }

    // Find all simple cycles in the graph
    cycles = findSimpleCycles(*(this->getAppGraph()));

    // Compute the cycle mean for each cycle
    for (const auto &iter : cycles) {
        SDFactor *srcActor = nullptr;
        SDFactor *dstActor = nullptr;
        SDFactorRefsCIter actorIter;
        double cycleMean = NAN;
        double weightChannels = 0;
        double weightActors = 0;
        SDFactorRefs cycle = iter;


        actorIter = cycle.begin();
        srcActor = *actorIter++;
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

        // Compute cycle mean
        cycleMean = weightActors / weightChannels;

        // Update maximum cycle mean for all actors on the cycle
        actorIter = cycle.begin();
        while (actorIter != cycle.end()) {
            srcActor = *actorIter;
            // Cycle mean of this cycle larger than current maximum
            maxCycleMean[srcActor->getId()] = (std::max)(cycleMean, maxCycleMean[srcActor->getId()]);

            // Next channel in the cycle
            actorIter++;
        }
    }

    channelWeight.reset();
    actorWeight.reset();
}

/**
 * sortActorsOnCriticality ()
 * The function computes for each actor the maximum cycle mean of all simple
 * cycles it is part of. The cycle mean is defined as the sum of the execution
 * times of the actors (in one period) divided by the normalized initial tokens
 * on the edges of the cycle. The function orders the actors according to their
 * maximum cycle mean (high to low).
 */
SDFactorRefs LoadBalanceBinding::sortActorsOnCriticality() {
    SDFactorRefs sortedActors;

    // Cycle mean not estimated?
    if (maxCycleMean == nullptr) {
        estimateMaxCycleMean();
    }

    // Sort actors based on maximum cycle mean (high to low)
    sortedActors = this->getAppGraph()->getActorRefs();
    sortOnCost(sortedActors, maxCycleMean.get());
    sortedActors.reverse();

    return sortedActors;
}

/**
 * sortTilesOnLoad ()
 * The function sorts the list of tiles from a low to a high load to which actor
 * a can be mapped.
 *
 * Cost of a tile is determined by:
 *  - the processor load measured in execution time of the mapped actors per
 *    iteration of the graph,
 *  - the memory requirements of the mapped actors,
 *  - the memory requirements of the mapped communication channels,
 *  - the bandwidth requirements of the mapped communication channels,
 *  - the number of mapped communication channels,
 *  - the number of additional connections required when mapping the actor to
 *    the tile.
 *
 * When computing the cost of a tile, while considering the mapping of an
 * actor to the tile, the actor is considered to be mapped to the tile. It
 * is only checked wether the processor type inside the tile is supported by
 * the actor.
 *
 * The cost of a tile t is defined as:
 *
 *    cost(t) = a*ProcLoad^k + b*(MemAct+MemCh)^l + c*Bw^m
                + d*NrConn^n + e*NrNewConn^o + g*latencyConn^q
 *
 * The constants a, b, c, d, e are used to scale the various properties
 * determining the cost wrt each other. The constants k, l, m, n, o are used to
 * increase cost for heavily loaded tiles.
 */
void LoadBalanceBinding::sortTilesOnLoad(const TimedSDFactor& a,
                                         TileRefs &tiles,
                                         double const_a,
                                         double const_b,
                                         double const_c,
                                         double const_d,
                                         double const_e,
                                         double const_f,
                                         double const_g,
                                         double const_k,
                                         double const_l,
                                         double const_m,
                                         double const_n,
                                         double const_o,
                                         double const_p,
                                         double const_q) {
    double procLoad = NAN;
    double memLoad = NAN;
    double inBwLoad = NAN;
    double outBwLoad = NAN;
    double bwLoad = NAN;
    double connLoad = NAN;
    double commLoad = NAN;
    double newConnLoad = NAN;
    double maxConnBinding = 0;

    // Initialize
    auto connBinding = std::make_unique<double[]>(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    auto costOfTile = std::make_unique<double[]>(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));

    // Estimate the maximum, average processing load to scale procLoad
    double maxProcLoad = 0;
    for (const auto& iter : this->getAppGraph()->getActors()) {
        auto* b = dynamic_cast<TimedSDFactor*>(iter.get());
        SDFtime maxExecTime = 0;

        for (const auto& iterP : b->getProcessors()) {
            maxExecTime = (std::max)(iterP->execTime, maxExecTime);
        }

        maxProcLoad += repVec[b->getId()] * maxExecTime;
    }
    maxProcLoad = maxProcLoad / static_cast<double>(this->getArchGraph()->nrTiles());

    // Compute resource usage and cost for each tile
    for (const auto& t : tiles) {
        Processor *p = t->getProcessor();
        Memory *m = t->getMemory();
        NetworkInterface *ni = t->getNetworkInterface();
        bool actorOnTile = false;

        // Can actor a be mapped to tile t?
        if (p != nullptr && a.getProcessor(p->getType()) != nullptr) {
            actorOnTile = true;
        }

        // Processing load of actors mapped to tile t
        procLoad = 0;
        for (const auto& iterA : this->getAppGraph()->getActors()) {
            auto* b = dynamic_cast<TimedSDFactor*>(iterA.get());
            TileConstRef tB = actorTileBinding[b->getId()];

            // Actor b mapped to tile t?
            if (tB != nullptr && tB->getId() == t->getId()) {
                // Add total execution time for executing actor b on tile t
                // to processing load of tile t
                procLoad += actorLoadOnTile(*b, *t);
            }

            // Actor b is same as actor a which is considered for mapping
            // and actor a can be mapped to tile t?
            if (a.getId() == b->getId() && actorOnTile) {
                // Add total execution time for executing actor a on tile t
                // to processing load of tile t
                procLoad += actorLoadOnTile(a, *t);
            }
        }

        // Add penalty for time wheel occupation
        // procLoad = procLoad * (p->availableTimewheelSize()
        //                                / (double) p->getTimewheelSize());
        // if (p->availableTimewheelSize() > 0)
        //    procLoad = procLoad * (p->getTimewheelSize()
        //                            / (double) p->availableTimewheelSize());

        // Scale procLoad
        procLoad = procLoad / maxProcLoad;

        // Memory
        memLoad = static_cast<double>(m->getSize() - m->availableMemorySize());
        if (actorOnTile) {
            // Is the memory reserved for actors not large enough?
            if (m->occupiedMemorySizeByActors() < a.getProcessor(p->getType())->stateSize) {
                // Increase memory needed for actor state
                memLoad = memLoad + static_cast<double>(a.getProcessor(p->getType())->stateSize)
                          - static_cast<double>(m->occupiedMemorySizeByActors());
            }

            // Add memory needed for newly mapped channels
            memLoad += static_cast<double>(memLoadChannelsOnTile(a, *t));
        }
        memLoad = memLoad / static_cast<double>(m->getSize());

        // Incoming bandwidth
        inBwLoad = ni->getInBandwidth() - ni->availableInBandwidth();
        if (actorOnTile) {
            inBwLoad += bwChannelsMappedToInConnection(a, *t);
        }
        inBwLoad = inBwLoad / ni->getInBandwidth();

        // Outgoing bandwidth
        outBwLoad = ni->getOutBandwidth() - ni->availableOutBandwidth();
        if (actorOnTile) {
            outBwLoad += bwChannelsMappedToOutConnection(a, *t);
        }
        outBwLoad = outBwLoad / ni->getOutBandwidth();

        // Bandwidth
        if (inBwLoad > outBwLoad) {
            bwLoad = inBwLoad;
        } else {
            bwLoad = outBwLoad;
        }

        // Connections
        connLoad = static_cast<double>(ni->getNrConnections() - ni->availableNrConnections());
        if (actorOnTile) {
            connLoad += nrChannelsMappedToConnection(a, *t);
        }
        connLoad = connLoad / static_cast<double>(ni->getNrConnections());

        // New connections
        newConnLoad = 0;
        if (actorOnTile) {
            newConnLoad = nrChannelsMappedToConnection(a, *t);
        }
        newConnLoad = newConnLoad / static_cast<double>(a.nrPorts());

        // Communication load average
        commLoad = (inBwLoad + outBwLoad + connLoad) / 3.0; // NOLINT(*magic-numbers)

        // Connection binding
        connBinding[t->getId()] = computeLoadOfChannelToConnectionBinding(a, *t);
        maxConnBinding = (std::max)(connBinding[t->getId()], maxConnBinding);

        // Cost of tile t
        costOfTile[t->getId()] = const_a * pow(procLoad, const_k);
        costOfTile[t->getId()] += const_b * pow(memLoad, const_l);
        costOfTile[t->getId()] += const_c * pow(bwLoad, const_m);
        costOfTile[t->getId()] += const_d * pow(connLoad, const_n);
        costOfTile[t->getId()] += const_e * pow(newConnLoad, const_o);
        costOfTile[t->getId()] += const_f * pow(commLoad, const_p);
    }

    // Add cost of connections (latency) to the cost of every tile
    for (const auto& t : tiles) {
        // Connection binding (normalize wrt to maximal load)
        connBinding[t->getId()] = connBinding[t->getId()] / maxConnBinding;

        // Cost of tile t
        costOfTile[t->getId()] += const_g * pow(connBinding[t->getId()], const_q);
    }

    // Sort tiles on cost
    sortOnCost(tiles, costOfTile.get());

}

/**
 * sortTilesOnCommunicationOverhead ()
 * The function takes as input a list of tiles ordered based on the processor
 * load and an actor when can be mapped to all tiles in the list. The functions
 * considers reordering the tiles based on the expected communication overhead
 * (connection delay + timewheel synchronization). When the communication
 * overhead of mapping actor a to tile t1 exceeds the gain of mapping actor a
 * onto a different tile t2, the tiles t1 and t2 are reordered.
 */
void LoadBalanceBinding::sortTilesOnCommunicationOverhead(const TimedSDFactor& a, TileRefs &tiles) {
    // Factor which determines the weight with which the communication overhead
    // is corrected when the overhead can be resolved by remapping an already
    // mapped actor to a different tile (e.g. a valued of 0.5 means count only
    // overhead for 50%).
    constexpr double remapCommFactor = 0.5;

    // Find all tiles which have a processor supported by the actor
    for (auto iter = tiles.begin(); iter != tiles.end();) {
        Tile *t = *iter;
        Processor *p = t->getProcessor();

        // Processor type of tile t not supported by actor a?
        if (p == nullptr || a.getProcessor(p->getType()) == nullptr) {
            tiles.erase(iter);
        } else {
            iter++;
        }
    }

    // Initialize cost of tiles
    auto costOfTile = std::make_unique<double[]>(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    for (const auto& t : tiles) {
        Processor *p = t->getProcessor();

        // Tile contains a processor
        if (p != nullptr) {
            costOfTile[t->getId()] =
                    tileLoad[t->getId()] + a.getExecutionTime(p->getType()) * repVec[a.getId()];
        }
    }

    // Initialize list of visited actors
    auto visitedActor = std::make_unique<bool[]>(this->getAppGraph()->nrActors());
    for (uint i = 0; i < this->getAppGraph()->nrActors(); i++) {
        visitedActor[i] = false;
    }

    // Consider all actors connected with a channel to actor a
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();
        auto *b = dynamic_cast<TimedSDFactor *>(c->oppositePort(p.get())->getActor());

        // Seen other channel between actors a and b before?
        if (visitedActor[b->getId()]) {
            continue;
        }

        // Iterate over all tiles to which actor a can be mapped
        for (auto iterT = tiles.begin(); iterT != tiles.end(); iterT++) {
            Tile *tileA = *iterT;
            const Tile *tileB = actorTileBinding[b->getId()];

            // Tile A contains no processor? then no mapping possible
            if (tileA->getProcessor() == nullptr) {
                continue;
            }

            // Is actor b mapped, but not to tile t?
            if (tileB != nullptr && tileB != tileA) {
                double commOverhead = 0;
                Connection *co = nullptr;

                // Find connection in architecture graph to be used
                if (p->getType() == SDFport::SDFportType::In) {
                    co = PlatformGraph::getConnection(tileB, tileA);
                } else {
                    co = PlatformGraph::getConnection(tileA, tileB);
                }

                // Compute communication overhead
                //  overhead = latency connection + max over all channels of
                // (token size * #tokens per firing * #firings per period)
                // divided by the bandwidth of the connection.

                // Find maximum over all channels which connect actor a and b
                commOverhead = 0;
                for (auto iterB = b->portsBegin(); iterB != b->portsEnd(); iterB++) {
                    SDFport* pB = iterB->get();
                    auto *ch = dynamic_cast<TimedSDFchannel *>(pB->getChannel());
                    double commOverheadCh = NAN;

                    // Does the channel connected to port pB connect to actor a?
                    if (ch->oppositePort(pB)->getActor()->getId() == a.getId()) {
                        commOverheadCh = ch->getTokenSize() * pB->getRate();
                        commOverheadCh = commOverheadCh * repVec[b->getId()];
                        commOverheadCh = commOverheadCh / ch->getMinBandwidth();
                        commOverheadCh = ceil(commOverheadCh);

                        commOverhead = (std::max)(commOverheadCh, commOverhead);
                    }
                }

                // Add delay of the connection to the overhead
                commOverhead += static_cast<double>(co->getLatency());

                // Can actor b be mapped to tile tileA?
                if (b->getProcessor(tileA->getProcessor()->getType()) != nullptr) {
                    // Let's reduce the impact of the communication as it can be
                    // resolved
                    commOverhead = remapCommFactor * commOverhead;
                }

                costOfTile[tileA->getId()] += commOverhead;
            }
        }

        // Mark actor b (connected to actor a) as visited
        visitedActor[b->getId()] = true;
    }

    // Sort tiles on cost
    sortOnCost(tiles, costOfTile.get(), 1, static_cast<int>(this->getArchGraph()->nrTiles()));
}

/**
 * computeLoadOfChannelToConnectionBinding ()
 * The function computes the cost of binding actor a to tile t in terms of the
 * latency of the created connections. It computes the sum of the latency of
 * all channels that are bound to a connection by binding actor a to tile t.
 */
double LoadBalanceBinding::computeLoadOfChannelToConnectionBinding(const SDFactor& a, const Tile& t) {
    auto *archGraph = dynamic_cast<PlatformGraph *>(t.getParent());
    double connBindingCost = 0;
    Connection *c = nullptr;

    for (const auto& p : a.getPorts()) {
        // Opposite actor connected to channel c
        SDFactor *b = p->getChannel()->oppositePort(p.get())->getActor();

        // Actor b mapped to a tile not equal to tile t?
        if (actorTileBinding[b->getId()] != nullptr
            && actorTileBinding[b->getId()]->getId() != t.getId()) {

            if (p->getType() == SDFport::SDFportType::In) {
                c = PlatformGraph::getConnection(actorTileBinding[b->getId()], &t);
            } else {
                c = PlatformGraph::getConnection(&t, actorTileBinding[b->getId()]);
            }

            connBindingCost += static_cast<double>(c->getLatency());
        }
    }

    return connBindingCost;
}

/**
 * memLoadChannelsOnTile ()
 * The function returns the memory size needed to map all channels connected to
 * actor a when the actor is mapped to tile t. The size of a channel is only
 * considered when the other actor which is connected to the channel is already
 * mapped to a tile.
 */
CSize LoadBalanceBinding::memLoadChannelsOnTile(const SDFactor& a, const Tile& t) {
    CSize mem = 0;

    // Iterate over all channels connected to actor a
    for (const auto& p : a.getPorts()) {
        auto *c = dynamic_cast<TimedSDFchannel *>(p->getChannel());
        SDFactor *b = c->oppositePort(p.get())->getActor();
        const Tile *tB = actorTileBinding[b->getId()];

        // Actor b already mapped to a tile?
        if (tB != nullptr) {
            // Actor b mapped to same tile as actor a?
            if (t.getId() == tB->getId()) {
                mem += static_cast<CSize>(c->getBufferSize().mem * c->getTokenSize());
            } else {
                // Actor a source of the channel?
                if (p->getType() == SDFport::SDFportType::Out) {
                    mem += static_cast<CSize>(c->getBufferSize().src * c->getTokenSize());
                } else {
                    mem += static_cast<CSize>(c->getBufferSize().dst * c->getTokenSize());
                }
            }
        }
    }

    return mem;
}

/**
 * bwChannelsMappedToInConnection ()
 * The function returns the bandwidth of channels connected to the actor a which
 * will be mapped to an ingoing connection if the actor is mapped to tile t.
 * Channels of which the other actor is not mapped are ignored.
 */
double LoadBalanceBinding::bwChannelsMappedToInConnection(const SDFactor& a, const Tile& t) {
    double bw = 0;

    // Iterate over all channels connected to actor a
    for (const auto& p : a.getPorts()) {
        auto *c = dynamic_cast<TimedSDFchannel *>(p->getChannel());
        SDFactor *b = nullptr;

        // Port p is an input port?
        if (p->getType() == SDFport::SDFportType::In) {
            // Opposite actor connected to channel c
            b = c->oppositePort(p.get())->getActor();

            // Actor b mapped to a tile not equal to tile t?
            if (actorTileBinding[b->getId()] != nullptr
                && actorTileBinding[b->getId()]->getId() != t.getId()) {
                bw += c->getMinBandwidth();
            }
        }
    }

    return bw;
}

/**
 * bwChannelsMappedToOutConnection ()
 * The function returns the bandwidth of channels connected to the actor a which
 * will be mapped to an outgoing connection if the actor is mapped to tile t.
 * Channels of which the other actor is not mapped are ignored.
 */
double LoadBalanceBinding::bwChannelsMappedToOutConnection(const SDFactor& a, const Tile& t) {
    double bw = 0;

    // Iterate over all channels connected to actor a
    for (const auto& p : a.getPorts()) {
        auto *c = dynamic_cast<TimedSDFchannel *>(p->getChannel());
        SDFactor *b = nullptr;

        // Port p is an output port?
        if (p->getType() == SDFport::SDFportType::Out) {

            // Opposite actor connected to channel c
            b = c->oppositePort(p.get())->getActor();

            // Actor b mapped to a tile not equal to tile t?
            if (actorTileBinding[b->getId()] != nullptr
                && actorTileBinding[b->getId()]->getId() != t.getId()) {
                bw += c->getMinBandwidth();
            }
        }
    }

    return bw;
}

/**
 * nrChannelsMappedToConnection ()
 * The function returns the number of channels connected to the actor a which
 * will be mapped to a connection if the actor is mapped to tile t. Channels of
 * which the other actor is not mapped are ignored.
 */
int LoadBalanceBinding::nrChannelsMappedToConnection(const SDFactor& a, const Tile& t) {
    int nrConnections = 0;

    // Iterate over all channels connected to actor a
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();
        SDFactor *b = nullptr;

        // Opposite actor connected to channel c
        b = c->oppositePort(p.get())->getActor();

        // Actor b mapped to a tile not equal to tile t?
        if (actorTileBinding[b->getId()] != nullptr
            && actorTileBinding[b->getId()]->getId() != t.getId()) {
            nrConnections++;
        }
    }

    return nrConnections;
}

/**
 * initTileLoad ()
 * Initially all tiles are not loaded.
 */
void LoadBalanceBinding::initTileLoad() {
    // Cleanup
    tileLoad.reset();

    // Initialize
    tileLoad = std::make_unique<double[]>(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    for (uint i = 0; i < this->getArchGraph()->nrTiles(); i++) {
        tileLoad[i] = 0;
    }
}

/**
 * actorLoadOnTile ()
 * The function returns the load of an actor on a tile. The load is defined as
 * the execution time of the actor per firing times the number of firings
 * required per period.
 */
double LoadBalanceBinding::actorLoadOnTile(const TimedSDFactor& a, const Tile& t) {
    Processor *p = t.getProcessor();

    // Tile contains no processor?
    if (p == nullptr) {
        return 0;
    }

    return a.getProcessor(p->getType())->execTime * repVec[a.getId()];
}

/**
 * increaseLoadTile ()
 * Add the load of an actor to a tile. The load is defined as the execution
 * time of the actor per firing times the number of firings required per period.
 */
void LoadBalanceBinding::increaseLoadTile(const TimedSDFactor& a, const Tile& t) {
    tileLoad[t.getId()] += actorLoadOnTile(a, t);
}

/**
 * decreaseLoadTile ()
 * Remove the load of an actor from a tile. The load is defined as the execution
 * time of the actor per firing times the number of firings required per period.
 */
void LoadBalanceBinding::decreaseLoadTile(const TimedSDFactor& a, const Tile& t) {
    tileLoad[t.getId()] -= actorLoadOnTile(a, t);

    ASSERT(tileLoad[t.getId()] >= 0, "tileLoad cannot be negative.");
}

/**
 * isActorBound ()
 * The function returns true if an actor is bound to a tile. Otherwise it
 * returns false.
 */
bool LoadBalanceBinding::isActorBound(const SDFactor& a) const {
    return actorTileBinding[a.getId()] != nullptr;
}

/**
 * isChannelBound ()
 * The function returns true if a channel is both the source and destination
 * actor are bound to the a tile. Otherwise it returns false.
 */
bool LoadBalanceBinding::isChannelBound(const SDFchannel& c) const {
    const Tile *src = actorTileBinding[c.getSrcActor()->getId()];
    const Tile *dst = actorTileBinding[c.getDstActor()->getId()];

    return src != nullptr && dst != nullptr;
}

/**
 * isChannelBoundToTile ()
 * The function returns true if a channel is both the source and destination
 * actor are bound to the same tile. Otherwise it returns false.
 */
bool LoadBalanceBinding::isChannelBoundToTile(const SDFchannel& c) const {
    const Tile *src = actorTileBinding[c.getSrcActor()->getId()];
    const Tile *dst = actorTileBinding[c.getDstActor()->getId()];

    return src != nullptr && dst != nullptr && src == dst;
}

/**
 * isChannelBoundToConnection ()
 * The function returns true if a channel is both the source and destination
 * actor are bound to the different tiles. Otherwise it returns false.
 */
bool LoadBalanceBinding::isChannelBoundToConnection(const SDFchannel& c) const {
    const Tile *src = actorTileBinding[c.getSrcActor()->getId()];
    const Tile *dst = actorTileBinding[c.getDstActor()->getId()];

    return src != nullptr && dst != nullptr && src != dst;
}

/**
 * releaseResources ()
 * The function releases all resources allocated by a application.
 */
void LoadBalanceBinding::releaseResources() {
    for (const auto& iter : this->getAppGraph()->getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        const Tile *t = actorTileBinding[a->getId()];

        // Actor bound to tile?
        if (t != nullptr) {
            // Release resources reserved by actor (including channels)
            releaseResources(*a, *t);
        }
    }
}

/**
 * releaseConnectionResources ()
 * The function releases all connection resources allocated by a channel.
 */
void LoadBalanceBinding::releaseConnectionResources(const TimedSDFchannel& c) {

    // Source and destination tiles
    const Tile *srcTile = actorTileBinding[c.getSrcActor()->getId()];
    const Tile *dstTile = actorTileBinding[c.getDstActor()->getId()];

    // Release memory allocations
    Memory *srcMem = srcTile->getMemory();
    srcMem->releaseMemory(&c);

    Memory *dstMem = dstTile->getMemory();
    dstMem->releaseMemory(&c);

    // Release network interface allocations
    NetworkInterface *srcNI = srcTile->getNetworkInterface();
    srcNI->releaseConnection(&c);

    NetworkInterface *dstNI = dstTile->getNetworkInterface();
    dstNI->releaseConnection(&c);

    // Remove binding of channel to connection
    ConnectionRefs connections;
    connections = PlatformGraph::getConnections(srcTile, dstTile);
    for (const auto& co : connections) {
        co->unbindChannel(&c);
    }

#ifdef VERBOSE
    std::cerr << "Removed binding of channel '" << c.getName() << "' to connection";
    std::cerr << " from '" << srcTile->getName() << "' to '" << dstTile->getName();
    std::cerr << "'." << std::endl;
#endif
}

/**
 * releaseResources ()
 * The function releases all resources allocated by an actor on a tile t. This
 * includes possible connections to other actors.
 */
void LoadBalanceBinding::releaseResources(const TimedSDFactor& a,const Tile& t) {
    Processor *p = nullptr;
    Memory *m = nullptr;

    // Components inside the tile
    p = t.getProcessor();
    m = t.getMemory();

    // Release memory allocation
    m->releaseMemory(&a);

    // Release reservations for channels
    for (const auto& po : a.getPorts()) {
        auto *ch = dynamic_cast<TimedSDFchannel *>(po->getChannel());

        if (isChannelBoundToTile(*ch)) {
            // Release memory allocation
            m->releaseMemory(ch);

#ifdef VERBOSE
            std::cerr << "Removed binding of channel '" << ch->getName() << "' to ";
            std::cerr << "tile '" << t.getName() << "'." << std::endl;
#endif

        } else if (isChannelBoundToConnection(*ch)) {
            // Release connection allocations
            releaseConnectionResources(*ch);
        }
    }

    // Release binding of actor to processor
    p->unbindActor(&a);

    // Release time slice allocation on processor
    p->releaseTimeSlice();

    // Update binding of actor to tile
    actorTileBinding[a.getId()] = nullptr;

    // Update load on tile
    decreaseLoadTile(a, t);

#ifdef VERBOSE
    std::cerr << "Removed binding of actor '" << a.getName() << "' to tile '";
    std::cerr << t.getName() << "'." << std::endl;
#endif
}

/**
 * allocateConnectionResources ()
 * The function allocates all connection resources needed by a channel.
 */
bool LoadBalanceBinding::allocateConnectionResources(const TimedSDFchannel& c) {
    NetworkInterface *srcNI = nullptr;
    NetworkInterface *dstNI = nullptr;
    Memory *srcMem = nullptr;
    Memory *dstMem = nullptr;
    ConnectionRefs connections;

    // Source and destination tiles
    const Tile *srcTile = actorTileBinding[c.getSrcActor()->getId()];
    const Tile *dstTile = actorTileBinding[c.getDstActor()->getId()];

    // Bind channel to connection
    bool boundChannel = false;
    connections = PlatformGraph::getConnections(srcTile, dstTile);
    for (auto iter = connections.begin(); iter != connections.end() && !boundChannel; iter++) {
        Connection *co = *iter;

        if (co->bindChannel(&c)) {
            boundChannel = true;
        }
    }
    if (!boundChannel) {
        return false;
    }

    // Allocate memory
    srcMem = srcTile->getMemory();
    if (!srcMem->reserveMemory(&c, static_cast<CSize>(c.getBufferSize().src) * static_cast<CSize>(c.getTokenSize()))) {
#ifdef VERBOSE
        std::cerr << "Not enough memory in source tile." << std::endl;
#endif
        return false;
    }

    dstMem = dstTile->getMemory();
    if (!dstMem->reserveMemory(&c, static_cast<CSize>(c.getBufferSize().dst) * static_cast<CSize>(c.getTokenSize()))) {
#ifdef VERBOSE
        std::cerr << "Not enough memory in destination tile." << std::endl;
#endif
        return false;
    }

    // Allocate network interface bandwidth and connection
    srcNI = srcTile->getNetworkInterface();
    if (!srcNI->reserveConnection(&c, 1, 0, c.getMinBandwidth())) {
#ifdef VERBOSE
        std::cerr << "Not enough bandwidth in source tile." << std::endl;
#endif
        return false;
    }

    dstNI = dstTile->getNetworkInterface();
    if (!dstNI->reserveConnection(&c, 1, c.getMinBandwidth(), 0)) {
#ifdef VERBOSE
        std::cerr << "Not enough bandwidth in destination tile." << std::endl;
#endif
        return false;
    }

#ifdef VERBOSE
    std::cerr << "Bound channel '" << c.getName() << "' to connection";
    std::cerr << " from '" << srcTile->getName() << "' to '" << dstTile->getName();
    std::cerr << "'." << std::endl;
#endif

    return true;
}

/**
 * allocateResources ()
 * The function attempts to allocate resources for actor a on the tile t.
 * The allocated time slice is equal to the actors execution time.
 */
bool LoadBalanceBinding::allocateResources(const TimedSDFactor& a, const Tile& t) {
    // Components inside the tile
    Processor *p = t.getProcessor();
    Memory *m = t.getMemory();

    // No processor available?
    if (p == nullptr) {
        return false;
    }

    // No time slice available?
    if (p->availableTimewheelSize() == 0) {
        return false;
    }

    // Bind actor to tile
    if (!p->bindActor(&a)) {
        return false;
    }

    // Actor is now mapped to tile; update binding of actor to tile
    actorTileBinding[a.getId()] = &t;

    // Update load on tile
    increaseLoadTile(a, t);

    // Allocate memory for the actor
    if (!m->reserveMemory(&a, a.getStateSize())) {
#ifdef VERBOSE
        std::cerr << "Failed allocating memory for actor '";
        std::cerr << a.getName() << "' on tile '";
        std::cerr << t.getName() << "'." << std::endl;
#endif

        // Release allocations
        releaseResources(a, t);

        return false;
    }

#ifdef VERBOSE
    std::cerr << "Bound actor '" << a.getName() << "' to tile '";
    std::cerr << t.getName() << "'." << std::endl;
#endif

    // Allocate network interface resources (if needed)
    for (const auto& po : a.getPorts()) {
        auto *c = dynamic_cast<TimedSDFchannel *>(po->getChannel());

        // Is connected actor b already mapped?
        if (isChannelBoundToTile(*c)) {
            // Allocate memory for channel in tile
            if (!m->reserveMemory(c, static_cast<CSize>(c->getBufferSize().mem) * static_cast<CSize>(c->getTokenSize()))) {
#ifdef VERBOSE
                std::cerr << "Failed allocating connection for channel '";
                std::cerr << c->getName() << "' in tile '";
                std::cerr << t.getName() << "'." << std::endl;
#endif

                // Release allocations
                releaseResources(a, t);

                return false;
            }

#ifdef VERBOSE
            std::cerr << "Bound channel '" << c->getName() << "' to tile '";
            std::cerr << t.getName() << "'." << std::endl;
#endif
        } else if (isChannelBoundToConnection(*c)) {
            if (!allocateConnectionResources(*c)) {
#ifdef VERBOSE
                std::cerr << "Failed allocating connection for channel '";
                std::cerr << c->getName() << "' from between tiles.";
                std::cerr << std::endl;
#endif

                // Release allocations
                releaseResources(a, t);

                return false;
            }
        }
    }

    // Done
    return true;
}

/**
 * moveActorBinding ()
 * The function moves an actor from one processor to another processor. This
 * operation invalidates the schedules of the involved processors.
 */
bool LoadBalanceBinding::moveActorBinding(const TimedSDFactor& a, bool allowExistingTile) {

    // Remove old binding of actor to tile
    const Tile *oldTile = actorTileBinding[a.getId()];
    releaseResources(a, *oldTile);

    // Sort tiles based on load
    TileRefs tiles = this->getArchGraph()->getTileRefs();
    sortTilesOnLoad(a,
                    tiles,
                    cnst_a,
                    cnst_b,
                    cnst_c,
                    cnst_d,
                    cnst_e,
                    cnst_f,
                    cnst_g,
                    cnst_k,
                    cnst_l,
                    cnst_m,
                    cnst_n,
                    cnst_o,
                    cnst_p,
                    cnst_q);

    // Try resource allocation on tiles
    for (const auto& t : tiles) {

        // Mapping of actor to old tile not allowed?
        if (!allowExistingTile && oldTile->getId() == t->getId()) {
            continue;
        }

        // Successfully allocated resources for the actor on the tile?
        if (allocateResources(a, *t)) {
#ifdef VERBOSE
            std::cerr << "Moved actor '" << a.getName() << "' from tile '";
            std::cerr << oldTile->getName() << "' to tile '";
            std::cerr << t->getName() << "'." << std::endl;
#endif

            // Invalidate schedule of involved tiles (if needed)
            if (oldTile->getId() != t->getId()) {
                oldTile->getProcessor()->getSchedule().clear();
                t->getProcessor()->getSchedule().clear();
            }

            return true;
        }
    }

    // No resources allocated for the actor, keep original binding
    allocateResources(a, *oldTile);

#ifdef VERBOSE
    std::cerr << "Failed allocating resources for actor '";
    std::cerr << a.getName();
    std::cerr << "'. Original binding preserved." << std::endl;
#endif

    return false;
}

/**
 * changeBandwidthAllocation ()
 * The function modifies the bandwidth allocation of a channel. On success, it
 * return true. On failure, it returns false and the channel keeps the original
 * bandwidth assignment.
 */
bool LoadBalanceBinding::changeBandwidthAllocation(const TimedSDFchannel& c, double bw) {
    // Bandwidth below required minimum?
    if (bw < c.getMinBandwidth()) {
        return false;
    }

    // Source and destination network interface
    const Tile *srcTile = actorTileBinding[c.getSrcActor()->getId()];
    const Tile *dstTile = actorTileBinding[c.getDstActor()->getId()];
    NetworkInterface *srcNI = srcTile->getNetworkInterface();
    NetworkInterface *dstNI = dstTile->getNetworkInterface();

    // Current bandwidth allocated to channel binding
    ComponentBinding *binding = srcNI->getBindings()->find(&c);
    double curBw = binding->getValue(NetworkInterface::outBw);

    // Enough bandwidth available on source and destination NI
    if (srcNI->availableOutBandwidth() + curBw < bw || dstNI->availableInBandwidth() + curBw < bw) {
        return false;
    }

    // Remove existing binding
    srcNI->releaseConnection(&c);
    dstNI->releaseConnection(&c);

    // Allocate network interface bandwidth and connection
    srcNI->reserveConnection(&c, 1, 0, bw);
    dstNI->reserveConnection(&c, 1, bw, 0);

#ifdef VERBOSE
    std::cerr << "Changed bandwidth of channel '" << c.getName() << "' from ";
    std::cerr << curBw << " to " << bw << "." << std::endl;
#endif

    return true;
}

/**
 * changeSlotAllocation ()
 * The function changes the size of the TDMA slot allocation of the tile to
 * the given size. On failure, the original allocation is maintained and false
 * is returned. On success, the function returns true.
 */
bool LoadBalanceBinding::changeSlotAllocation(const Tile& t, CSize sz) {
    Processor *p = t.getProcessor();

    // Tile contains no processor?
    if (p == nullptr) {
        return false;
    }

    // Not enough free space available for new allocation?
    if (p->availableTimewheelSize() + p->getReservedTimeSlice() < sz) {
        return false;
    }

    // Reserve TDMA time slice
    p->releaseTimeSlice();
    p->reserveTimeSlice(sz);

    return true;
}

/**
 * bindActorsToTiles ()
 * Bind each actor to a tile. Actors are handled in order of their criticality
 * and tiles are tried in order of their load. The objective is to spread the
 * load evenly over all tiles.
 */
bool LoadBalanceBinding::bindActorsToTiles() {
    SDFactorRefsIter actorIter;
    SDFactorRefs actors;

    // Sort actors based on criticality
    actors = sortActorsOnCriticality();

    // Map actors one-by-one
    actorIter = actors.begin();
    while (actorIter != actors.end()) {
        auto* a = dynamic_cast<TimedSDFactor*>(*actorIter);
        bool mappedActor = false;

        // Sort tiles based on load
        TileRefs tiles = this->getArchGraph()->getTileRefs();
        sortTilesOnLoad(*a,
                        tiles,
                        cnst_a,
                        cnst_b,
                        cnst_c,
                        cnst_d,
                        cnst_e,
                        cnst_f,
                        cnst_g,
                        cnst_k,
                        cnst_l,
                        cnst_m,
                        cnst_n,
                        cnst_o,
                        cnst_p,
                        cnst_q);

        // Try resource allocation on tiles
        for (const auto& t : tiles) {
            // Successfully allocated resources for the actor on the tile?
            if (allocateResources(*a, *t)) {
                mappedActor = true;
                break;
            }
        }

        // No resources allocated for the actor?
        if (!mappedActor) {
#ifdef VERBOSE
            std::cerr << "Failed allocating resources for actor '";
            std::cerr << a->getName();
            std::cerr << "'." << std::endl;
#endif

            return false;
        }

        // Next actor
        actorIter++;
    }

    return true;
}

/**
 * constructStaticOrderSchedules ()
 * The function generates a static-order schedule for each processor in the
 * architecture graph. The schedules order the firing of the actors in the
 * application graph. They respects the buffer requirements and mapping of all
 * actors.
 */
void LoadBalanceBinding::constructStaticOrderSchedules() {

#ifdef _CREATE_STATIC_ORDER_SCHEDULES_WITH_PRIORITIES
    SDFstateSpacePriorityListScheduler listScheduler;
#else
    SDFstateSpaceListScheduler listScheduler;
#endif

    // Assume that 50% of available tdma wheel is allocated for graph
    constexpr double FiftyPercent = .5;
    reserveTimeSlices(FiftyPercent);

    // Create a binding-aware SDFG
    BindingAwareSDFG bindingAwareSDFG(this->getAppGraph(), this->getArchGraph(), getFlowType());

    // Construct static-order schedules
    SDFstateSpaceListScheduler::schedule(&bindingAwareSDFG);

    // Assign schedules to processors in the tiles
    for (uint i = 0; i < bindingAwareSDFG.nrTilesInPlatformGraph(); i++) {

        Tile *t = this->getArchGraph()->getTile(i);
        Processor *p = t->getProcessor();

        p->setSchedule(bindingAwareSDFG.getScheduleOnTile(i));
    }

    // Release the allocated time slices
    releaseTimeSlices();

    // Actors used in the schedule of the processors come from
    // 'mappedAppGraph'. These must be mapped to actors from 'appGraph'.
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // Processor inside the tile?
        if (p != nullptr) {
            p->getSchedule().changeActorAssociations(this->getAppGraph());

            logMsg("Schedule on '" + t->getName() + "' has " + CString(p->getSchedule().size())
                   + " states.");
        }
    }
}

/**
 * reconstructStaticOrderSchedules ()
 * The function generates a static-order schedule for each processor in the
 * architecture graph. The schedules order the firing of the actors in the
 * application graph. They respects the buffer requirements and mapping of all
 * actors.
 */
void LoadBalanceBinding::reconstructStaticOrderSchedules() {

#ifdef _CREATE_STATIC_ORDER_SCHEDULES_WITH_PRIORITIES
    SDFstateSpacePriorityListScheduler listScheduler;
#else
    SDFstateSpaceListScheduler listScheduler;
#endif

    // Create a binding-aware SDFG
    BindingAwareSDFG bindingAwareSDFG(this->getAppGraph(), this->getArchGraph(), getFlowType());

    // Construct static-order schedules
    SDFstateSpaceListScheduler::schedule(&bindingAwareSDFG);

    // Assign schedules to processors in the tiles
    for (uint i = 0; i < bindingAwareSDFG.nrTilesInPlatformGraph(); i++) {
        Tile *t = this->getArchGraph()->getTile(i);
        Processor *p = t->getProcessor();

        p->setSchedule(bindingAwareSDFG.getScheduleOnTile(i));
    }

    // Actors used in the schedule of the processors come from
    // 'mappedAppGraph'. These must be mapped to actors from 'appGraph'.
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // Processor inside the tile?
        if (p != nullptr) {
            p->getSchedule().changeActorAssociations(this->getAppGraph());

            logMsg("Schedule on '" + t->getName() + "' has " + CString(p->getSchedule().size())
                   + " states.");
        }
    }
}

/**
 * optimizeActorToTileBindings ()
 * The function moves actors between tiles to achieve an optimal load balance.
 * It considers both computation and communication cost when moving actors.
 */
void LoadBalanceBinding::optimizeActorToTileBindings() {
    SDFactorRefs actors = this->getAppGraph()->getActorRefs();
    uint nrActors = this->getAppGraph()->nrActors();
    bool updateCost = true;

    // Allocate memory for storing actorCost
    auto actorCost = std::make_unique<double[]>(static_cast<size_t>(nrActors) * 2);

    while (updateCost) {
        // Compute cost of actors considering the tile bindings
        for (const auto& a : actors) {
            const Tile *t = actorTileBinding[a->getId()];

            // Actor mapped to a tile?
            if (t != nullptr) {
                // Dimension 1: tile load
                actorCost[a->getId()] = 1.0 / tileLoad[t->getId()];

                // Dimension 2: actor criticality
                actorCost[nrActors + a->getId()] = maxCycleMean[a->getId()];
            }
        }

        // Sort actors on cost
        sortOnCost(actors, actorCost.get(), 2, static_cast<int>(nrActors));

        // Cost are up-to-date
        updateCost = false;

        // Traverse actors in order and try to move them to a new tile
        for (auto iter = actors.begin(); !updateCost && iter != actors.end();) {
            auto* a = dynamic_cast<TimedSDFactor*>(*iter);

            // Try to move actor to new tile
            if (moveActorBinding(*a, true)) {
                // Remove all actors already considered from list and update
                // cost of remaining actors
                actors.erase(actors.begin(), ++iter);
                updateCost = true;
            } else {
                iter++;
            }
        }
    }
}

/**
 * reserveTimeSlices ()
 * The function reserves a TDMA time slice of fraction of the available wheel
 * size on each tile which contains actors of the application graph.
 */
void LoadBalanceBinding::reserveTimeSlices(double fraction) {
    CSize slice = 0;

    ASSERT((fraction >= 0.0) && (fraction <= 1.0), "Fraction must be within [0,1]"); //NOLINT(*simplify-boolean-expr)

    // Reserve slice for each tile
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // No processor and/or no actor mapped to tile?
        if (p == nullptr || tileLoad[t->getId()] == 0) {
            continue;
        }

        // Slice is fraction of available time wheel
        slice = static_cast<CSize>(fraction * static_cast<double>(p->availableTimewheelSize()));

        // At least 1 slot is needed
        slice = (std::max)(slice, static_cast<CSize>(1));

        // Reserve time slice
        p->reserveTimeSlice(slice);

#ifdef VERBOSE
        double percentageTile = static_cast<double>(p->getReservedTimeSlice()) / static_cast<double>(p->getTimewheelSize());
        std::cerr << "Reserved " << 100.0 * percentageTile << "% of time wheel tile '";
        std::cerr << t->getName() << "'" << std::endl;
#endif
    }
}

/**
 * releaseTimeSlices ()
 * The function releases the complete TDMA time slice it has allocated on
 * each tile.
 */
void LoadBalanceBinding::releaseTimeSlices() {
    // Release slice for each tile
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // Tile contains a processor?
        if (p != nullptr) {
            // Reserve time slice
            p->releaseTimeSlice();

#ifdef VERBOSE
            std::cerr << "Released complete slice of time wheel on tile '";
            std::cerr << t->getName() << "'" << std::endl;
#endif
        }
    }
}

/**
 * minimizeTimeSlices ()
 * The function minimizes the time slice allocation of all used tiles. The
 * objective is to find the minimal time slices with which the throughput
 * constraint is met. The function uses a binary search for this problem.
 */
bool LoadBalanceBinding::minimizeTimeSlices(double step, const double minStep) {
    std::vector<CSize> currentSlice(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    std::vector<double> tileUtilization;
    CSize slice = 0;
    CTimer timer;

    // Throughput constraint
    double thrConstraint = this->getAppGraph()->getThroughputConstraint();

    // Analyze throughput
    startTimer(&timer);
    double thrGraph = analyzeThroughput(tileUtilization);
    stopTimer(&timer);

#ifdef VERBOSE
    std::cerr << "Thr: " << thrGraph << std::endl;
    std::cerr << "Analysis time: ";
    printTimer(std::cerr, &timer);
    std::cerr << std::endl;
#endif

    // No step allowed?
    if (step < minStep) {
        // Constraint met?
        if (thrConstraint <= thrGraph) {
#ifdef VERBOSE
            std::cerr << "Minimum step size reached, constraint met." << std::endl;
#endif
            return true;
        }
#ifdef VERBOSE
        std::cerr << "Minimum step size reached, constraint not met." << std::endl;
#endif
        return false;
    }

    // No optimization needed (throughput graph less than 10% above throughput
    // constraint)?
    constexpr double TenPercentAbove = 1.1;
    if (thrConstraint <= thrGraph && thrConstraint * TenPercentAbove >= thrGraph) {
#ifdef VERBOSE
        std::cerr << "No minimization needed; throughput within 10% of constraint";
        std::cerr << std::endl;
#endif

        return true;
    }

    // Throughput constraint not met?
    if (thrConstraint > thrGraph) {
        bool increasedSlice = false;

        // Increase time slices and try again
        for (const auto& t : this->getArchGraph()->getTiles()) {
            Processor *p = t->getProcessor();

            // No processor and/or no actor mapped to tile?
            if (p == nullptr || tileLoad[t->getId()] == 0) {
                continue;
            }

            // Compute increase of slice
            slice = static_cast<CSize>(
                    ceil(step * static_cast<double>(p->availableTimewheelSize())));

            // Space available to increase slice?
            if (slice >= 1) {
                // Reserve the slice
                p->reserveTimeSlice(p->getReservedTimeSlice() + slice);
                increasedSlice = true;

#ifdef VERBOSE
                double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                                  / static_cast<double>(p->getTimewheelSize());
                std::cerr << "Increased time wheel reservation of tile '";
                std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
                std::cerr << std::endl;
#endif
            }
        }

        // No slices increased
        if (!increasedSlice) {
            return false;
        }

        // Recursively search for optimal time slices
        return minimizeTimeSlices(step, minStep);
    }
    bool decreasedSlice = false;

    // Constraint met, further reduction may be possible. However, do not
    // attempt if current throughput analysis took more than 5 seconds
    constexpr unsigned int FiveSeconds = 5;
    if (secondsElapsed(&timer) > FiveSeconds) {
#ifdef VERBOSE
        std::cerr << "Abort optimization - timeout" << std::endl;
#endif
        return true;
    }

    // Decrease time slices and try again
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // No processor and/or actor mapped to tile?
        if (p == nullptr || tileLoad[t->getId()] == 0) {
            continue;
        }

        // Keep size of current slice
        currentSlice[t->getId()] = p->getReservedTimeSlice();

        // Compute decrease of slice
        slice = static_cast<CSize>(step * static_cast<double>(p->availableTimewheelSize()));

        // Reduction must be less than current slice
        if (slice >= p->getReservedTimeSlice()) {
            slice = 1;
        }

        // Slice reduced by at least 1 slot?
        if (slice >= 1 && p->getReservedTimeSlice() > slice) {
            // Reserve the slice
            if (!p->reserveTimeSlice(p->getReservedTimeSlice() - slice)) {
                throw CException("[ERROR] Failed decreasing time slice.");
            }
            decreasedSlice = true;

#ifdef VERBOSE
            double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                                / static_cast<double>(p->getTimewheelSize());
            std::cerr << "Reduced time wheel reservation of tile '";
            std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
            std::cerr << std::endl;
#endif
        }
    }

    // No slices decreased
    if (!decreasedSlice) {
        return true;
    }

    // Recursively search for optimal time slices
    if (!minimizeTimeSlices(step / 2, minStep)) {
        // Decreased time slices cause failure on throughput constraint
        // With current slices, the constraint is met. Keep current slices.

        for (const auto& t : this->getArchGraph()->getTiles()) {
            Processor *p = t->getProcessor();

            // Tile contains a processor?
            if (p != nullptr) {
                p->reserveTimeSlice(currentSlice[t->getId()]);

#ifdef VERBOSE
                double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                                    / static_cast<double>(p->getTimewheelSize());
                std::cerr << "Reset time wheel reservation of tile '";
                std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
                std::cerr << std::endl;
#endif
            }
        }

        return true;
    }

    return true;
}

/**
 * optimizeTimeSlices ()
 * The function performs a binary search to minimize the time slice allocated on
 * each tile.
 */
void LoadBalanceBinding::optimizeTimeSlices() {
    std::vector<CSize> minSlice(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    std::vector<CSize> maxSlice(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    double maxTileLoad = 0;

    // Find maximum tile load
    for (const auto& t : this->getArchGraph()->getTiles()) {
        maxTileLoad = (std::max)(tileLoad[t->getId()], maxTileLoad);
    }

    // Set bound on size of all slices
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // Tile contains a processor
        if (p != nullptr) {
            maxSlice[t->getId()] = p->getReservedTimeSlice();
            minSlice[t->getId()] = static_cast<CSize>(
                    floor(static_cast<double>(maxSlice[t->getId()]) * (tileLoad[t->getId()] / maxTileLoad)));
        } else {
            maxSlice[t->getId()] = 0;
            minSlice[t->getId()] = 0;
        }
    }

    // Perform the real optimization...
    if (!optimizeTimeSlices(minSlice, maxSlice)) {
        // Optimization failed, revert to original slices
        for (const auto& t : this->getArchGraph()->getTiles()) {
            Processor *p = t->getProcessor();

            // Actor mapped to tile t?
            if (p != nullptr && tileLoad[t->getId()] != 0) {
                // Reserve the slice
                p->reserveTimeSlice(maxSlice[t->getId()]);

#ifdef VERBOSE
                double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                                  / static_cast<double>(p->getTimewheelSize());
                std::cerr << "Reset time wheel reservation of tile '";
                std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
                std::cerr << std::endl;
#endif
            }
        }
    }
}

/**
 * optimizeTimeSlices ()
 * The function performs a binary search to minimize the time slice allocated on
 * each tile.
 */
bool LoadBalanceBinding::optimizeTimeSlices(std::vector<CSize> minSlice,
                                            std::vector<CSize> maxSlice) {
    std::vector<CSize> newSlice(static_cast<unsigned int>(this->getArchGraph()->nrTiles()));
    std::vector<double> tileUtilization;
    double thrGraph = NAN;
    double thrConstraint = NAN;
    bool resizedSlice = false;
    CTimer timer;

    // Resize slices
    for (const auto& t : this->getArchGraph()->getTiles()) {
        Processor *p = t->getProcessor();

        // Tile contains no processor?
        if (p == nullptr) {
            continue;
        }

        // Can slice be resized?
        if (minSlice[t->getId()] + 1 >= maxSlice[t->getId()]) {
            newSlice[t->getId()] = maxSlice[t->getId()];

            // Size different from current allocation?
            if (newSlice[t->getId()] != p->getReservedTimeSlice()) {
                p->reserveTimeSlice(newSlice[t->getId()]);

#ifdef VERBOSE
                double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                                  / static_cast<double>(p->getTimewheelSize());
                std::cerr << "Set time wheel reservation of tile '";
                std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
                std::cerr << std::endl;
#endif
            }
        } else {
            // Compute new size of slice
            newSlice[t->getId()] =
                    minSlice[t->getId()]
                    + static_cast<CSize>(ceil((static_cast<double>(maxSlice[t->getId()]) - static_cast<double>(minSlice[t->getId()])) / 2.0)); // NOLINT(*magic-numbers)

            // Reserve the slice
            p->reserveTimeSlice(newSlice[t->getId()]);
            resizedSlice = true;

#ifdef VERBOSE
            double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                              / static_cast<double>(p->getTimewheelSize());
            std::cerr << "Changed time wheel reservation of tile '";
            std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
            std::cerr << std::endl;
#endif
        }
    }

    // Throughput constraint
    thrConstraint = this->getAppGraph()->getThroughputConstraint();

    // Analyze throughput
    startTimer(&timer);
    thrGraph = analyzeThroughput(tileUtilization);
    stopTimer(&timer);

#ifdef VERBOSE
    std::cerr << "Thr: " << thrGraph;
    if (thrGraph >= thrConstraint) {
        std::cerr << " (constraint met)";
    }
    std::cerr << std::endl;
    std::cerr << "Analysis time: ";
    printTimer(std::cerr, &timer);
    std::cerr << std::endl;
#endif

    // No slice resized?
    if (!resizedSlice) {
#ifdef VERBOSE
        std::cerr << "No slice resized - stop optimization." << std::endl;
#endif

        // Done, constraint met?
        return thrGraph >= thrConstraint;
    }

    // Constraint met?
    if (thrGraph >= thrConstraint) {
        // Decrease size of slice (set max to current size and call function)
        // If call returns false, reset the size of the slice to current size
        if (!optimizeTimeSlices(minSlice, newSlice)) {
            for (const auto& t : this->getArchGraph()->getTiles()) {
                Processor *p = t->getProcessor();

                // Actor mapped to tile?
                if (p != nullptr && tileLoad[t->getId()] != 0) {
                    // Reserve the slice
                    p->reserveTimeSlice(newSlice[t->getId()]);

#ifdef VERBOSE
                    double percentageTile = static_cast<double>(p->getReservedTimeSlice())
                                      / static_cast<double>(p->getTimewheelSize());
                    std::cerr << "Reset time wheel reservation of tile '";
                    std::cerr << t->getName() << "' to " << 100.0 * percentageTile << "%";
                    std::cerr << std::endl;
#endif
                }
            }
        }

        return true;
    }
    // Increase size of slice (set min to current size and call function)
    return optimizeTimeSlices(newSlice, maxSlice);

    return false;
}

/**
 * updateStorageSpaceAllocation ()
 * The function changes the storage space allocation of every
 * channel marked as storage space in the mappedAppGraph into the
 * storage space indicated by the storage distribution d.
 */
void LoadBalanceBinding::updateStorageSpaceAllocation(const TimedSDFgraph& mappedAppGraph,
                                                      const std::unique_ptr<StorageDistribution>& d) {
    SDFactor *srcActor = nullptr;
    SDFactor *dstActor = nullptr;
    TimedSDFchannel* c = nullptr;
    TimedSDFchannel *cX = nullptr;
    TimedSDFchannel::BufferSize b;
    TimedSDFchannel::BufferSize bPrev;
    bool update = false;
    Memory *m = nullptr;
    const Tile *t = nullptr;

    if (d == nullptr) {
        logMsg("No new storage space distribution found.");
        logMsg("Storage space is already minimal.");
        return;
    }

    for (const auto& iter : mappedAppGraph.getChannels()) {
        c = dynamic_cast<TimedSDFchannel*>(iter.get());

        // Channel models storage space?
        if (c->modelsStorageSpace()) {
            cX = dynamic_cast<TimedSDFchannel *>(
                    this->getAppGraph()->getChannel(c->getStorageSpaceChannel()->getId()));
            srcActor = cX->getSrcActor();
            dstActor = cX->getDstActor();
            b = cX->getBufferSize();
            bPrev = cX->getBufferSize();

            // Channel cX mapped to tile?
            if (isChannelBoundToTile(*cX)) {
                // Buffer does not have to be resized?
                if (static_cast<ulong>(b.mem) == d->sp[c->getId()]) {
                    continue;
                }

                // Update buffers in channel
                b.sz = static_cast<int>(d->sp[c->getId()]);
                b.mem = static_cast<int>(d->sp[c->getId()]);
                cX->setBufferSize(b);

                // Update resource allocation
                t = actorTileBinding[srcActor->getId()];
                m = t->getMemory();
                m->releaseMemory(cX);
                if (!m->reserveMemory(cX, static_cast<CSize>(b.mem) * cX->getTokenSize())) {
                    throw CException("Failed resizing storage space "
                                     "of channel.");
                }

                // Performed update
                update = true;

                logMsg("Changed storage space '" + cX->getName() + "' on tile '" + t->getName()
                       + "' from " + CString(bPrev.mem) + " tokens to " + CString(b.mem)
                       + " tokens.");
            } else if (c->getDstActor()->getId() == srcActor->getId()) {
                // Channel c models source buffer

                // Buffer does not have to be resized?
                if (static_cast<ulong>(b.src) == d->sp[c->getId()]) {
                    continue;
                }

                // Update buffers in channel
                b.src = static_cast<int>(d->sp[c->getId()]);
                cX->setBufferSize(b);

                // Update resource allocation
                t = actorTileBinding[srcActor->getId()];
                m = t->getMemory();
                m->releaseMemory(cX);
                if (!m->reserveMemory(cX, static_cast<CSize>(b.src) * cX->getTokenSize())) {
                    throw CException("Failed resizing storage space "
                                     "of channel.");
                }

                // Performed update
                update = true;

                logMsg("Changed storage space '" + cX->getName() + "' on tile '" + t->getName()
                       + "' from " + CString(bPrev.src) + " tokens to " + CString(b.src)
                       + " tokens.");
            } else {
                // Channel c models destination buffer

                // Buffer does not have to be resized?
                if (static_cast<ulong>(b.dst) == d->sp[c->getId()]) {
                    continue;
                }

                // Update buffers in channel
                b.dst = static_cast<int>(d->sp[c->getId()]);
                cX->setBufferSize(b);

                // Update resource allocation
                t = actorTileBinding[dstActor->getId()];
                m = t->getMemory();
                m->releaseMemory(cX);
                if (!m->reserveMemory(cX, static_cast<CSize>(b.dst) * cX->getTokenSize())) {
                    throw CException("Failed resizing storage space "
                                     "of channel.");
                }

                // Performed update
                update = true;

                logMsg("Changed storage space '" + cX->getName() + "' on tile '" + t->getName()
                       + "' from " + CString(bPrev.dst) + " tokens to " + CString(b.dst)
                       + " tokens.");
            }
        }
    }

    if (!update) {
        logMsg("Storage space is already minimal.");
    }
}

/**
 * minimizeStorageSpace ()
 * The function computes the minimal buffer allocations needed to meet the
 * throughput constraint. Resource allocations are adjusted to this minimal
 * storage space.
 */
void LoadBalanceBinding::minimizeStorageSpace() {
    SDFstateSpaceBindingAwareBufferAnalysis bufferAnalysisAlgo;
    std::unique_ptr<StorageDistributionSet> minStorageDeps = nullptr;
    StorageDistributionSet* storageDep = nullptr;

    // Throughput constraint
    double thrConstraint = this->getAppGraph()->getThroughputConstraint();

    // Create a binding-aware SDFG
    BindingAwareSDFG bindingAwareSDFG(this->getAppGraph(), this->getArchGraph(), getFlowType());

    // Mark every channels as buffer channel or non-buffer channel
    auto bufferChannels = std::make_unique<bool[]>(bindingAwareSDFG.nrChannels());
    for (uint i = 0; i < bindingAwareSDFG.nrChannels(); i++) {
        auto *c = dynamic_cast<TimedSDFchannel *>(bindingAwareSDFG.getChannel(i));
        if (c->modelsStorageSpace()) {
            bufferChannels[i] = true;
        } else {
            bufferChannels[i] = false;
        }
    }

    // Compute buffer/throughput trade-off space
    minStorageDeps =
            bufferAnalysisAlgo.analyze(&bindingAwareSDFG, bufferChannels.get(), true, thrConstraint);

    // Update the storage space allocation with the smallest storage
    // distribution that satisfies the throughput constraint
    storageDep = minStorageDeps.get();
    while (storageDep != nullptr) {
        if (storageDep->thr >= thrConstraint) {
            updateStorageSpaceAllocation(bindingAwareSDFG, storageDep->distributions);
            break;
        }

        storageDep = storageDep->next.get();
    }

#ifdef VERBOSE
    // There should always be a storage dependency which satisfies the
    // throughput constraint (assuming that the initial allocation satisfies
    // the constraint)
    if (storageDep == nullptr) {
        std::cerr << "Storage space could not be optimized further." << std::endl;
    }
#endif
}

/**
 * bindingCheck ()
 * Run only binding step of the load balance binding algorithm.
 */
bool LoadBalanceBinding::bindingCheck() {
#ifdef VERBOSE
    std::cerr << "[INFO] Load balancing mapping algorithm (binding check)" << std::endl;
#endif

#ifdef VERBOSE
    std::cerr << "[INFO] Constants tile cost function:" << std::endl;
    std::cerr << "    a: " << cnst_a << std::endl;
    std::cerr << "    b: " << cnst_b << std::endl;
    std::cerr << "    c: " << cnst_c << std::endl;
    std::cerr << "    d: " << cnst_d << std::endl;
    std::cerr << "    e: " << cnst_e << std::endl;
    std::cerr << "    f: " << cnst_f << std::endl;
    std::cerr << "    g: " << cnst_g << std::endl;
    std::cerr << "    k: " << cnst_k << std::endl;
    std::cerr << "    l: " << cnst_l << std::endl;
    std::cerr << "    m: " << cnst_m << std::endl;
    std::cerr << "    n: " << cnst_n << std::endl;
    std::cerr << "    o: " << cnst_o << std::endl;
    std::cerr << "    p: " << cnst_p << std::endl;
    std::cerr << "    q: " << cnst_q << std::endl;
#endif

    // Initialize vector containing all actor to tile bindings
    actorTileBinding.resize(this->getAppGraph()->nrActors());
    for (uint i = 0; i < this->getAppGraph()->nrActors(); i++) {
        actorTileBinding[i] = nullptr;
    }

    // Initialize the load of the tiles
    initTileLoad();

#ifdef VERBOSE
    std::cerr << "[INFO] Binding" << std::endl;
#endif

    // Bind each actor to a tile
    if (!bindActorsToTiles()) {
#ifdef VERBOSE
        std::cerr << "Failed binding actor to tile." << std::endl;
#endif
        return false;
    }

    // Reserve complete time wheel on each tile
    reserveTimeSlices(1);

#ifdef VERBOSE
    std::cerr << "[INFO] Verifying throughput constraint" << std::endl;
#endif

    // Check throughput constraint
    return isThroughputConstraintSatisfied();
}

/**
 * bindSDFGtoTiles ()
 *
 */
bool LoadBalanceBinding::bindSDFGtoTiles() {
#ifdef VERBOSE
    std::cerr << "[INFO] Load balancing mapping algorithm" << std::endl;
#endif

#ifdef VERBOSE
    uint nrActorsHSDFG = 0;
    for (uint i = 0; i < this->getAppGraph()->nrActors(); i++) {
        nrActorsHSDFG += repVec[i];
    }

    std::cerr << "[INFO] Application graph analysis" << std::endl;
    std::cerr << "Application thr: " << analyzeThroughputApplication() << std::endl;
    std::cerr << "#actors:         " << this->getAppGraph()->nrActors() << std::endl;
    std::cerr << "#channels:       " << this->getAppGraph()->nrChannels() << std::endl;
    std::cerr << "#actors (HSDFG): " << nrActorsHSDFG << std::endl;
#endif

#ifdef VERBOSE
    std::cerr << "[INFO] Constants tile cost function:" << std::endl;
    std::cerr << "    a: " << cnst_a << std::endl;
    std::cerr << "    b: " << cnst_b << std::endl;
    std::cerr << "    c: " << cnst_c << std::endl;
    std::cerr << "    d: " << cnst_d << std::endl;
    std::cerr << "    e: " << cnst_e << std::endl;
    std::cerr << "    f: " << cnst_f << std::endl;
    std::cerr << "    g: " << cnst_g << std::endl;
    std::cerr << "    k: " << cnst_k << std::endl;
    std::cerr << "    l: " << cnst_l << std::endl;
    std::cerr << "    m: " << cnst_m << std::endl;
    std::cerr << "    n: " << cnst_n << std::endl;
    std::cerr << "    o: " << cnst_o << std::endl;
    std::cerr << "    p: " << cnst_p << std::endl;
    std::cerr << "    q: " << cnst_q << std::endl;
#endif

    // Initialize the load of the tiles
    initTileLoad();

#ifdef VERBOSE
    std::cerr << "[INFO] Binding" << std::endl;
#endif

    // Bind each actor to a tile
    if (!bindActorsToTiles()) {
#ifdef VERBOSE
        std::cerr << "Failed binding actor to tile." << std::endl;
#endif

        // Release allocated resources on failure
        for (const auto& iter : this->getAppGraph()->getActors()) {
            auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            const Tile *t = actorTileBinding[a->getId()];

            // Actor bound to tile?
            if (t != nullptr) {
                // Release resources reserved by actor (including channels)
                releaseResources(*a, *t);
            }
        }

        return false;
    }

#ifdef VERBOSE
    double max = 0;
    for (uint i = 0; i < this->getArchGraph()->nrTiles(); i++) {
        max = (max > tileLoad[i] ? max : tileLoad[i]);
    }

    std::cerr << "Tile load:" << std::endl;
    for (uint i = 0; i < this->getArchGraph()->nrTiles(); i++) {
        std::cerr << "    " << this->getArchGraph()->getTile(i)->getName() << ": ";
        std::cerr << (100 * tileLoad[i] / max) << "%" << std::endl; // NOLINT(*magic-numbers)
    }
#endif

#ifdef VERBOSE
    std::cerr << "[INFO] Optimize binding" << std::endl;
#endif

    // Optimize tile bindings
    optimizeActorToTileBindings();

#ifdef VERBOSE
    max = 0;
    for (uint i = 0; i < this->getArchGraph()->nrTiles(); i++) {
        max = (max > tileLoad[i] ? max : tileLoad[i]);
    }

    std::cerr << "Tile load:" << std::endl;
    for (uint i = 0; i < this->getArchGraph()->nrTiles(); i++) {
        std::cerr << "    " << this->getArchGraph()->getTile(i)->getName() << ": ";
        std::cerr << (100 * tileLoad[i] / max) << "%" << std::endl; // NOLINT(*magic-numbers)
    }
#endif

    return true;
}

/**
 * constructStaticOrderScheduleTiles ()
 *
 */
bool LoadBalanceBinding::constructStaticOrderScheduleTiles() {
    constructStaticOrderSchedules();

#ifdef VERBOSE
    std::cerr << "[INFO] Minimize static-order schedules" << std::endl;
#endif

    minimizeStaticOrderSchedules(this->getArchGraph());

    return true;
}

/**
 * allocateTDMAtimeSlices ()
 *
 */
bool LoadBalanceBinding::allocateTDMAtimeSlices() {
    std::vector<double> tileUtilization;
    bool success = false;

    constexpr double FractionHalf = 0.5;
    reserveTimeSlices(FractionHalf);

#ifdef VERBOSE
    std::cerr << "[INFO] Minimize time slices" << std::endl;
#endif

    success = minimizeTimeSlices(0.5, 0.01); // NOLINT(*magic-numbers)

    if (success) {
#ifdef VERBOSE
        std::cerr << "[INFO] Optimize time slices" << std::endl;
#endif

        optimizeTimeSlices();

#ifdef VERBOSE
        std::cerr << "[INFO] Tile utilization" << std::endl;

        // Analyze tile utilization
        analyzeThroughput(tileUtilization);

        for (const auto& t : this->getArchGraph()->getTiles()) {
            std::cerr << "   Tile load (" << t->getName() << "): ";
            std::cerr << 100.0 * tileUtilization[t->getId()] << "%" << std::endl;
        }
#endif
    }

#ifdef VERBOSE
    std::cerr << "[INFO] Verifying throughput constraint" << std::endl;
#endif

    // Check throughput constraint
    success = isThroughputConstraintSatisfied();

    // Release allocated resources on failure
    if (!success) {
        for (const auto& iter : this->getAppGraph()->getActors()) {
            auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            const Tile *t = actorTileBinding[a->getId()];

            // Actor bound to tile?
            if (t != nullptr) {
                // Release resources reserved by actor (including channels)
                releaseResources(*a, *t);
            }
        }
    }

    return success;
}

/**
 * optimizeStorageSpaceAllocations ()
 *
 */
bool LoadBalanceBinding::optimizeStorageSpaceAllocations() {
    minimizeStorageSpace();

    return true;
}

/**
 * bind ()
 * Load balance binding algorithm.
 */
bool LoadBalanceBinding::bind() {
    if (!bindSDFGtoTiles()) {
        return false;
    }

    if (!constructStaticOrderScheduleTiles()) {
        return false;
    }

    if (!allocateTDMAtimeSlices()) {
        return false;
    }

    if (!optimizeStorageSpaceAllocations()) {
        return false;
    }

    return true;
}
} // namespace SDF
