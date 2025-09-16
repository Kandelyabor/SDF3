/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph_binding.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 25, 2009
 *
 *  Function        :   Application graph binding
 *
 *  History         :
 *      25-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: graph_binding.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "graph_binding.h"

namespace FSMSADF {

/**
 * dominates()
 * The function returns true when this Pareto point dominates the Pareto point
 * q. Otherwise the function returns false.
 */
bool GraphBindingBase::ParetoQuantities::dominates(const ParetoQuantities &q) const {
    for (const auto& [p, l] : processingLoad) {
        auto j = q.processingLoad.find(p);
        ASSERT(j != q.processingLoad.end(), "Unequal dimensions.");

        if (l > j->second) {
            return false;
        }
    }

    for (const auto& [m, l] : memoryLoad) {
        std::map<Memory *, double>::const_iterator j;

        j = q.memoryLoad.find(m);
        ASSERT(j != q.memoryLoad.end(), "Unequal dimensions.");

        if (l > j->second) {
            return false;
        }
    }

    for (const auto& [ni, l] : communicationLoad) {
        std::map<NetworkInterface *, double>::const_iterator j;

        j = q.communicationLoad.find(ni);
        ASSERT(j != q.communicationLoad.end(), "Unequal dimensions.");

        if (l > j->second) {
            return false;
        }
    }

    return true;
}

/**
 * distance()
 * The function returns the euclidean distance between this Pareto point and q.
 */
double GraphBindingBase::ParetoQuantities::distance(const ParetoQuantities &q) const {
    double d = 0;

    for (const auto& [p, l] : processingLoad) {
        std::map<Processor *, double>::const_iterator j;

        j = q.processingLoad.find(p);
        ASSERT(j != q.processingLoad.end(), "Unequal dimensions.");

        d = pow(l - j->second, 2);
    }

    for (const auto& [m, l] : memoryLoad) {
        std::map<Memory *, double>::const_iterator j;

        j = q.memoryLoad.find(m);
        ASSERT(j != q.memoryLoad.end(), "Unequal dimensions.");

        d = pow(l - j->second, 2);
    }

    for (const auto& [ni, l] : communicationLoad) {
        std::map<NetworkInterface *, double>::const_iterator j;

        j = q.communicationLoad.find(ni);
        ASSERT(j != q.communicationLoad.end(), "Unequal dimensions.");

        d = pow(l - j->second, 2);
    }

    return d;
}

/**
 * print()
 * Print quantities to the output stream.
 */
std::ostream &GraphBindingBase::ParetoQuantities::print(std::ostream &out) const {
    out << "processing:" << std::endl;
    std::map<std::string, double> sortedProcessingLoad;
    for (const auto &[p, l] : processingLoad) {
        std::string name = p->getTile()->getName() + "." + p->getName();
        sortedProcessingLoad[name] = l;
    }
    for (const auto& [p, l] : sortedProcessingLoad) {
        out << "  " << p << " -> " << l << std::endl;
    }

    out << "memory:" << std::endl;
    std::map<std::string, double> sortedMemoryLoad;
    for (const auto &[m, l] : memoryLoad) {
        std::string name = m->getTile()->getName() + "." + m->getName();
        sortedMemoryLoad[name] = l;
    }
    for (const auto &[m, l] : sortedMemoryLoad) {
        out << "  " << m << " -> " << l << std::endl;
    }

    out << "communication:" << std::endl;
    std::map<std::string, double> sortedCommunicationLoad;
    for (const auto &[ni, l] : communicationLoad) {
        std::string name = ni->getTile()->getName() + "." + ni->getName();
        sortedCommunicationLoad[name] = l;
    }
    for (const auto &[ni, l] : sortedCommunicationLoad) {
        out << "  " << ni << " -> " << l << std::endl;
    }

    return out;
}

/**
 * GraphBinding()
 * Constructor. Note that the constructor creates a clone of the platform
 * binding. This clone is automatically destroyed when the GraphBinding
 * destructor is called. A pointer to the cloned platform binding can be
 * retrieved using the function getPlatformBinding.
 */
GraphBindingBase::GraphBindingBase(const PlatformBinding *b) : platformBinding(b->clone(*b)) {
    // Extract existing application binding from the platform binding
    extractApplicationBindingFromPlatformBinding();
}

/**
 * ~GraphBinding()
 * Destructor.
 */
GraphBindingBase::~GraphBindingBase() = default;

/**
 * copyInternals()
 * Copies the internal values from this to gb
 * @param gb the #GraphBindingBase that is going to get the copied values.
 */

void GraphBindingBase::copyInternalsTo(GraphBindingBase *gb) const {
    // Actors bindings
    gb->actorBindings = actorBindings;

    // Channel bindings
    gb->channelBindings = channelBindings;

    // TDMA bounds
    gb->minTDMAslices = minTDMAslices;
    gb->maxTDMAslices = maxTDMAslices;

    // Pareto quantities
    gb->paretoQuantities = paretoQuantities;
}
/**
 * clone()
 * Create a clone of this graph binding. The cloned copy will contain a cloned
 * platform binding object.
 */
std::unique_ptr<GraphBindingBase> GraphBindingBase::clone() const {
    auto gb = std::make_unique<GraphBindingBase>(platformBinding.get());

    this->copyInternalsTo(gb.get());
    return gb;
}

/**
 * extractApplicationBindingFromPlatformBinding()
 * The function extracts the application to platform binding from the platform
 * binding object.
 */
void GraphBindingBase::extractApplicationBindingFromPlatformBinding() {
    // Iterate over tiles in the platform
    for (const auto& tb : platformBinding->getTileBindings()) {
        // Iterate over the processor bindings in the tile binding
        for (const auto& pb : tb->getProcessorBindings()) {
            // Iterate over the actors bound to this processor
            for (auto& [sc, actors] : pb->getActorBindings()) {
                const Scenario *s = sc;

                // Iterate over the actor
                for (auto *a : actors) {
                    actorBindings[s][a].processor = pb->getProcessor();
                }
            }
        }

        // Iterate over the memory bindings in the tile binding
        //        for (MemoryBindings::iterator j = tb->getMemoryBindings().begin();
        //                j != tb->getMemoryBindings().end(); j++)
        {
            MemoryBinding *mb = tb->getMemoryBinding();

            // Iterate over the actors bound to this memory
            for (auto& [s, actBindings] : mb->getActorBindings()) {
                for (const auto &l : actBindings) {
                    Actor *a = l.actor;
                    actorBindings[s][a].actor_memory = l.actor_memory;
                    actorBindings[s][a].tile_memory = l.tile_memory;
                }
            }

            // Iterate over the channels bound to this memory
            for (auto& [s, memBindings] : mb->getChannelBindings()) {
                for (const auto &l : memBindings) {
                    Channel *c = l.channel;

                    if (l.bufferLocation == BufferLocation::Value::Mem) {
                        channelBindings[s][c].memoryMem = l.memory; // mb->getMemory();
                    } else if (l.bufferLocation == BufferLocation::Value::Src) {
                        channelBindings[s][c].memorySrc = l.memory; // mb->getMemory();
                    } else if (l.bufferLocation == BufferLocation::Value::Dst) {
                        channelBindings[s][c].memoryDst = l.memory; // mb->getMemory();
                    }
                }
            }
        }

        // Iterate over the network interface bindings in the tile
        for (const auto& nb : tb->getNetworkInterfaceBindings()) {
            // Iterate over the input channel bindings
            for (auto& [s, channels] : nb->getInChannelBindings()) {
                // Iterate over the channels
                for (auto *c : channels) {
                    channelBindings[s][c].networkInterfaceIn = nb->getNetworkInterface();
                }
            }

            // Iterate over the output channel bindings
            for (auto& [s, channels] : nb->getOutChannelBindings()) {
                // Iterate over the channels
                for (auto *c : channels) {
                    channelBindings[s][c].networkInterfaceOut = nb->getNetworkInterface();
                }
            }
        }
    }

    // Iterate over the connections in the platform
    for (const auto& cb : platformBinding->getConnectionBindings()) {
        // Iterate over the channel bindings
        for (auto& [s, channels] : cb->getChannelBindings()) {
            // Iterate over the channels
            for (auto *c : channels) {
                channelBindings[s][c].connection = cb->getConnection();
            }
        }
    }
}

/**
 * bindActorToTile()
 * The function binds actor a in scenario s to a processor and memory. The
 * function returns true when a binding has been made or false when the binding
 * fails.
 */
bool GraphBindingBase::bindActorToTile(const Scenario& s, Actor *a, ActorBinding &b) {
    ProcessorBinding* pb = getProcessorBinding(*(b.processor));
    MemoryBinding *mb = getMemoryBinding(b.tile_memory);

    // Processor and memory do not belong to the same tile?
    if (b.processor->getTile() != b.tile_memory->getTile()) {
        throw CException("Not all elements belong to the same tile.");
    }

    // Bind actor to resources
    if (!pb->addActorBinding(s, a)
        || !mb->addActorBinding(s, a, Actor::ProcessorType(b.processor->getType()))) {
        // Binding failed, remove partial binding
        pb->removeActorBinding(s, a);
        mb->removeActorBinding(s, a);

        return false;
    }

    // Update the binding state
    actorBindings[&s][a] = b;

    return true;
}

/**
 * removeActorToTileBinding()
 * The function removes the binding of actor a in scenario s.
 */
void GraphBindingBase::removeActorToTileBinding(const Scenario& s, Actor *a) {
    // Remove binding of actor to processor
    getProcessorBinding(*(actorBindings[&s][a].processor))->removeActorBinding(s, a);

    // Remove binding of actor to memory
    getMemoryBinding(actorBindings[&s][a].tile_memory)->removeActorBinding(s, a);

    // Update the binding state
    actorBindings[&s].erase(a);
}

/**
 * bindChannelToTile()
 * The function binds channel c in scenario s to memory m. The function returns
 * true when a binding has been made or false when the binding fails.
 */
bool GraphBindingBase::bindChannelToTile(const Scenario& s, Channel *c, ChannelBinding &b) {
    MemoryBinding *mb = getMemoryBinding(b.memoryMem);

    // Check if constraints exclude this binding
    ChannelBindingConstraint *cbc = platformBinding->getGraphBindingConstraints()
                                            ->getConstraintsOfScenario(s)
                                            ->getConstraintsOfChannel(c->getName());
    if (cbc->hasBufferLocationConstraint()
        && (cbc->getBufferLocation() == BufferLocation::Value::Mem)) {
        return false;
    }
    // For now, do not consider shared mem bindings if not explicit in constraint
    if (b.memoryMem->getType() == MemoryType::Value::Shared) {
        return false;
    }

    // Bind channel to memory
    if (!mb->addChannelBinding(s, c, BufferLocation(BufferLocation::Value::Mem))) {
        return false;
    }

    // Update the binding state
    channelBindings[&s][c] = b;

    return true;
}

/**
 * bindChannelToConnection()
 * The function binds channel c in scenario s to the connection binding cb. The
 * function allocates also resources in the source and destination tile (i.e.
 * on the memory and network interfaces). The function returns true when a
 * binding has been made or false when the binding fails.
 */
bool GraphBindingBase::bindChannelToConnection(const Scenario& s, Channel *c, ChannelBinding &b) {
    MemoryBinding *mbSrc = getMemoryBinding(b.memorySrc);
    NetworkInterfaceBinding *nbSrc = getNetworkInterfaceBinding(b.networkInterfaceOut);
    MemoryBinding *mbDst = getMemoryBinding(b.memoryDst);
    NetworkInterfaceBinding *nbDst = getNetworkInterfaceBinding(b.networkInterfaceIn);
    ConnectionBinding *cb = getConnectionBinding(b.connection);

    // Check if constraints exclude this binding
    ChannelBindingConstraint *cbc = platformBinding->getGraphBindingConstraints()
                                            ->getConstraintsOfScenario(s)
                                            ->getConstraintsOfChannel(c->getName());
    if (cbc->hasBufferLocationConstraint()
        && (cbc->getBufferLocation() == BufferLocation::Value::Mem)) {
        return false;
    }

    // Memory and network interface do not belong to the same tile?
    if (b.memorySrc->getTile() != nbSrc->getNetworkInterface()->getTile()
        || b.memoryDst->getTile() != nbDst->getNetworkInterface()->getTile()) {
        throw CException("Memory and network interface do not belong to the "
                         "same tile.");
    }

    // Bind channel to connection
    if (!cb->addChannelBinding(s, c)
        || !mbSrc->addChannelBinding(s, c, BufferLocation(BufferLocation::Value::Src))
        || !nbSrc->addOutChannelBinding(s, c)
        || !mbDst->addChannelBinding(s, c, BufferLocation(BufferLocation::Value::Dst))
        || !nbDst->addInChannelBinding(s, c)) {
        // Binding failed, remove partial binding
        cb->removeChannelBinding(s, c);
        mbSrc->removeChannelBinding(s, c);
        nbSrc->removeOutChannelBinding(s, c);
        mbDst->removeChannelBinding(s, c);
        nbDst->removeInChannelBinding(s, c);

        return false;
    }

    // Update binding state
    channelBindings[&s][c] = b;

    return true;
}

/**
 * bindChannelToSharedMemory()
 * The function binds channel c in scenario s to memory mbMem. The
 * function allocates also resources in the source and destination tile (i.e.
 * on the memory and network interfaces). The function returns true when a
 * binding has been made or false when the binding fails.
 */
bool GraphBindingBase::bindChannelToSharedMemory(const Scenario& s, Channel *c, ChannelBinding &b) {
    MemoryBinding *mbSrc = getMemoryBinding(b.memorySrc);
    NetworkInterfaceBinding *nbSrc = getNetworkInterfaceBinding(b.networkInterfaceOut);
    MemoryBinding *mbDst = getMemoryBinding(b.memoryDst);
    NetworkInterfaceBinding *nbDst = getNetworkInterfaceBinding(b.networkInterfaceIn);
    MemoryBinding *mbMem = getMemoryBinding(b.memoryMem);
    ConnectionBinding *cbToMem = getConnectionBinding(b.connToMem);
    ConnectionBinding *cbFromMem = getConnectionBinding(b.connFromMem);

    // Check if constraints enforce a shared memory binding, otherwise do not use it
    ChannelBindingConstraint *cbc = platformBinding->getGraphBindingConstraints()
                                            ->getConstraintsOfScenario(s)
                                            ->getConstraintsOfChannel(c->getName());
    if (!(cbc->hasBufferLocationConstraint()
          && cbc->getBufferLocation() == BufferLocation::Value::Mem)) {
        return false;
    }

    // Memory and network interface do not belong to the same tile?
    if (b.memorySrc->getTile() != nbSrc->getNetworkInterface()->getTile()
        || b.memoryDst->getTile() != nbDst->getNetworkInterface()->getTile()) {
        throw CException("Memory and network interface do not belong to the "
                         "same tile.");
    }

    // Bind channel to shared memory
    if (!cbToMem->addChannelBinding(s, c) || !cbFromMem->addChannelBinding(s, c)
        || !mbSrc->addChannelBinding(s, c, BufferLocation(BufferLocation::Value::Src))
        || !nbSrc->addOutChannelBinding(s, c)
        || !mbDst->addChannelBinding(s, c, BufferLocation(BufferLocation::Value::Dst))
        || !nbDst->addInChannelBinding(s, c)
        || !mbMem->addChannelBinding(s, c, BufferLocation(BufferLocation::Value::Mem))) {
        // Binding failed, remove partial binding
        cbToMem->removeChannelBinding(s, c);
        cbFromMem->removeChannelBinding(s, c);
        mbSrc->removeChannelBinding(s, c);
        nbSrc->removeOutChannelBinding(s, c);
        mbDst->removeChannelBinding(s, c);
        nbDst->removeInChannelBinding(s, c);
        mbMem->removeChannelBinding(s, c);
        return false;
    }

    // Update binding state
    channelBindings[&s][c] = b;
    return true;
}

/**
 * removeChannelBinding()
 * The function removes the binding of channel c in scenario s.
 */
void GraphBindingBase::removeChannelBinding(const Scenario& s, Channel *c) {
    // Remove binding of channel to connection (if exists)
    if (channelBindings[&s][c].connection != nullptr) {
        // Connection binding
        getConnectionBinding(channelBindings[&s][c].connection)->removeChannelBinding(s, c);

        // Remove binding of channel to network interface input
        getNetworkInterfaceBinding(channelBindings[&s][c].networkInterfaceIn)
                ->removeInChannelBinding(s, c);

        // Remove binding of channel to network interface outputs
        getNetworkInterfaceBinding(channelBindings[&s][c].networkInterfaceOut)
                ->removeInChannelBinding(s, c);
    }

    // Remove binding of channel to memories
    if (channelBindings[&s][c].memoryMem != nullptr) {
        getMemoryBinding(channelBindings[&s][c].memoryMem)->removeChannelBinding(s, c);
    }
    if (channelBindings[&s][c].memorySrc != nullptr) {
        getMemoryBinding(channelBindings[&s][c].memorySrc)->removeChannelBinding(s, c);
    }
    if (channelBindings[&s][c].memoryDst != nullptr) {
        getMemoryBinding(channelBindings[&s][c].memoryDst)->removeChannelBinding(s, c);
    }

    // Update binding state
    channelBindings[&s].erase(c);
}

/**
 * getProcessorBinding()
 * The function returns a pointer to the processor binding in the platform
 * binding which is associated with processor p.
 */
ProcessorBinding* GraphBindingBase::getProcessorBinding(const Processor& p) const {
    return platformBinding->getTileBinding(p.getTile())->getProcessorBinding(p);
}

/**
 * getMemoryBinding()
 * The function returns a pointer to the memory binding in the platform
 * binding which is associated with memory m.
 */
MemoryBinding *GraphBindingBase::getMemoryBinding(Memory *m) const {
    return platformBinding->getTileBinding(m->getTile())->getMemoryBinding();
}

/**
 * getNetworkInterfaceBinding()
 * The function returns a pointer to the network interface binding in the
 * platform binding which is associated with network interface n.
 */
NetworkInterfaceBinding *GraphBindingBase::getNetworkInterfaceBinding(NetworkInterface *n) const {
    return platformBinding->getTileBinding(n->getTile())->getNetworkInterfaceBinding(n);
}

/**
 * getConnectionBinding()
 * The function returns a pointer to the connection binding in the platform
 * binding which is associated with connection c.
 */
ConnectionBinding *GraphBindingBase::getConnectionBinding(Connection *c) const {
    return platformBinding->getConnectionBinding(c);
}

/**
 * getMinTDMAslice()
 * The function returns the minimum time slice for processor p in scenario s.
 * When no minimum has been set, the function returns zero.
 */
Time GraphBindingBase::getMinTDMAslice(const Scenario& s, const Processor& p) const {
    // Processor known?
    auto i = minTDMAslices.find(&p);
    if (i != minTDMAslices.end()) {
        auto j = i->second.find(&s);

        // Scenario known?
        if (j != i->second.end()) {
            return j->second;
        }
    }

    return 0;
}

/**
 * setMinTDMAslice()
 * Set the minimum TDMA slice on processor p in scenario s to t time units.
 */
void GraphBindingBase::setMinTDMAslice(const Scenario& s, const Processor& p, Time t) {
    minTDMAslices[&p][&s] = t;
}

/**
 * getMaxTDMAslice()
 * The function returns the maximum time slice for processor p in scenario s.
 * When no maximum has been set, the function returns the available
 * time wheel size on processor p.
 */
Time GraphBindingBase::getMaxTDMAslice(const Scenario& s, const Processor& p) const {
    // Processor known?
    auto i = maxTDMAslices.find(&p);
    if (i != maxTDMAslices.end()) {
        auto j = i->second.find(&s);

        // Scenario known?
        if (j != i->second.end()) {
            return j->second;
        }
    }

    return getProcessorBinding(p)->getAvailableWheelsize(s);
}

/**
 * setMaxTDMAslice()
 * Set the maximum TDMA slice on processor p in scenario s to t time units.
 */
void GraphBindingBase::setMaxTDMAslice(const Scenario& s, const Processor& p, Time t) {
    maxTDMAslices[&p][&s] = t;
}

/**
 * allocateTDMAslice()
 * Allocate TDMA time slice t on processor p in scenario s.
 */
bool GraphBindingBase::allocateTDMAslice(const Scenario& s, const Processor& p, Time t) {
    return getProcessorBinding(p)->allocateWheelsize(s, t);
}

/**
 * getAllocatedTDMAslice()
 * Get the allocated TDMA time slice on processor p in scenario s.
 */
Time GraphBindingBase::getAllocatedTDMAslice(const Scenario& s, const Processor& p) {
    return getProcessorBinding(p)->getAllocatedWheelsize(s);
}

/**
 * computeParetoQuantities()
 * This function computes the values of the pareto quantities of this
 * graph binding.
 */
void GraphBindingBase::computeParetoQuantities(
        const std::map<const Scenario *, RepetitionVector> &repetitionVectors,
        const bool processingLoadBasedOnTDMAslices) {
    double sumProcessingLoad = 0;

    // Clear existing Pareto quantities
    paretoQuantities.processingLoad.clear();
    paretoQuantities.memoryLoad.clear();
    paretoQuantities.communicationLoad.clear();

    // Iterate over all tiles in the platform binding
    for (const auto& tb: platformBinding->getTileBindings()) {
        // Iterate over the processor bindings
        for (const auto& pb: tb->getProcessorBindings()) {
            Processor *p = pb->getProcessor();

            // Initialize processing load at zero
            paretoQuantities.processingLoad[p] = 0;

            // Compute processing load using TDMA slice allocations?
            if (processingLoadBasedOnTDMAslices) {
                // Iterate over the TDMA slice allocations of this processor
                for (auto& [sc, sz]: pb->getWheelsizeAllocations()) {
                    // Add slice size of this scenario to the Pareto quantity
                    paretoQuantities.processingLoad[p] += sz;
                }

                // Normalize the Pareto quantity over the product of the wheel
                // size and number of scenarios in which it is used
                paretoQuantities.processingLoad[p] =
                        paretoQuantities.processingLoad[p]
                        / static_cast<double>(pb->getWheelsizeAllocations().size() * p->getWheelsize());
            } else {
                // Iterate over the actors bound to this processor
                for (auto& [s, actors]: pb->getActorBindings()) {
                    // Iterate over the actors
                    for (auto *a: actors) {
                        paretoQuantities.processingLoad[p] =
                                paretoQuantities.processingLoad[p]
                                + (a->getExecutionTimeOfScenario(*s, p->getType())
                                   * repetitionVectors.at(s).at(a->getId()));
                    }
                }
            }

            sumProcessingLoad = sumProcessingLoad + paretoQuantities.processingLoad[p];
        }

        // Iterate over the memory bindings
        //        for (MemoryBindings::iterator j = tb->getMemoryBindings().begin();
        //               j != tb->getMemoryBindings().end(); j++)
        {
            MemoryBinding *mb = tb->getMemoryBinding();
            for (auto *m: mb->getMemories()) {
                paretoQuantities.memoryLoad[m] =
                        mb->getMemorySizeUsedForGraph(m) / static_cast<double>(m->getSize());
            }
        }

        // Iterate over the network interface bindings
        for (const auto& nb: tb->getNetworkInterfaceBindings()) {
            NetworkInterface *n = nb->getNetworkInterface();

            paretoQuantities.communicationLoad[n] =
                    (nb->getInBandwidthUsedForGraph() + nb->getOutBandwidthUsedForGraph())
                    / (n->getInBandwidth() + n->getOutBandwidth());
        }
    }

    // Normalize the processing load
    for (auto& [p, l]: paretoQuantities.processingLoad) {
        paretoQuantities.processingLoad[p] = paretoQuantities.processingLoad[p] / sumProcessingLoad;
    }
}

} // End namespace FSMSADF
