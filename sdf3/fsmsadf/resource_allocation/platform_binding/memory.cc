/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      13-05-11    :   Add support for multiple memories and memory types.
 *      12-05-09    :   Initial version.
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
#include "fsmsadf/resource_allocation/platform_graph/memory.h"
#include "graph.h"
#include <algorithm>

namespace FSMSADF {
/**
 * MemoryBinding()
 * Constructor.
 */
MemoryBinding::MemoryBinding(const GraphComponent &c, MemoryRefs m, BindingMethod bm) :
    GraphComponent(c), memories(std::move(m)), bindingMethod(bm) {
    // Create an initial used size for all memory
    for (const auto& mem : this->memories) {
        setMemorySizeUsedForOtherGraphs(mem, static_cast<Size>(0));
    }
}
/**
 * ~MemoryBinding()
 * Destructor.
 */
MemoryBinding::~MemoryBinding() = default;
/**
 * clone()
 * Create a cloned copy of this memory binding.
 */
std::unique_ptr<MemoryBinding> MemoryBinding::clone(const GraphComponent &c) {
    auto mb = std::make_unique<MemoryBinding>(c, getMemories(), getBindingMethod());

    // Actors
    mb->setActorBindings(getActorBindings());

    // Channels
    mb->setChannelBindings(getChannelBindings());

    // Memory size used for other graphs
    for (const auto& m : memories) {
        mb->setMemorySizeUsedForOtherGraphs(m, getMemorySizeUsedForOtherGraphs(m));
    }
    return mb;
}

/**
 * constructFromXML()
 * This function creates the memory binding as specified by the
 * connectionNode.
 */
/// @todo fixme
void MemoryBinding::constructFromXML(const Scenario& s, const CNodePtr memoryNode) {
    // Application graph
    ScenarioGraph *sg = s.getScenarioGraph();

    // Iterate over the list of actors bound to this memory
    for (CNode *n = CGetChildNode(memoryNode, "actor"); n != nullptr; n = CNextNode(n, "actor")) {
        // Name specified for the actor?
        if (!CHasAttribute(n, "name")) {
            throw CException("Mapped actor has no name identifier.");
        }

        // Processor type specified for the actor?
        if (!CHasAttribute(n, "processorType")) {
            throw CException("Mapped actor has no processorType identifier.");
        }

        // Find actor in scenario graph
        ActorRef a = sg->getActor(CGetAttribute(n, "name"));

        // Find processor type in actor
        const Actor::ProcessorType &p = a->getProcessorType(CGetAttribute(n, "processorType"));

        // Create binding of actor to memory
        addActorBinding(s, a, p);
    }

    // Iterate over the list of channels bound to this memory
    for (CNode *n = CGetChildNode(memoryNode, "channel"); n != nullptr;
         n = CNextNode(n, "channel")) {
        // Name specified for the channel?
        if (!CHasAttribute(n, "name")) {
            throw CException("Mapped channel has no name identifier.");
        }

        // Buffer type specified for the actor?
        if (!CHasAttribute(n, "bufferType")) {
            throw CException("Mapped actor has no bufferType identifier.");
        }

        // Find channel in scenario graph
        ChannelRef c = sg->getChannel(CGetAttribute(n, "name"));

        // Buffer location
        BufferSize b;
        BufferLocation bufferLocation = BufferLocation(CGetAttribute(n, "bufferType"));

        // Create binding of channel to memory
        if (!addChannelBinding(s, c, bufferLocation)) {
            throw CException("Could not create binding for channel '" + c->getName() + "'.");
        }
    }
}

/**
 * constructResourceUsageFromXML()
 * This function sets the resource usage of the memory.
 */
/// @todo fix me
void MemoryBinding::constructResourceUsageFromXML(Memory *m, const CNodePtr tileNode) {
    // Used memory
    if (!CHasAttribute(tileNode, "size")) {
        throw CException("Memory resource usage has no size.");
    }

    setMemorySizeUsedForOtherGraphs(m, static_cast<Size>(CGetAttribute(tileNode, "size")));
}
/**
 * convertToXML()
 * This function converts the memory binding to an XML object.
 */
void MemoryBinding::convertToXML(const Scenario& s, const CNodePtr tileNode) {
    for (const auto& m : memories) {
        CNode *memoryNode = CAddNode(tileNode, "memory");

        // Name
        CAddAttribute(memoryNode, "name", m->getName());

        // Actor binding exists for this scenario?
        auto scActorIter = actorBindings.find(&s);
        if (scActorIter != actorBindings.end()) {
            // List of all actors bound to this memory
            for (const auto &ab : scActorIter->second) {
                /* Bound to this memory*/
                if (ab.tile_memory == m) {
                    CNode *n = CAddNode(memoryNode, "actor");
                    CAddAttribute(n, "name", ab.actor->getName());
                    CAddAttribute(n, "processorType", ab.processorType.type);
                }
            }
        }

        // Channel binding exists for this scenario?
        auto scChannelIter = channelBindings.find(&s);
        if (scChannelIter != channelBindings.end()) {
            // List of all channels bound to this memory
            for (auto mb : scChannelIter->second) {
                /* Bound to this memory*/
                if (mb.memory == m) {
                    BufferSize b;
                    CNode *n = CAddNode(memoryNode, "channel");
                    CAddAttribute(n, "name", mb.channel->getName());
                    CAddAttribute(n, "bufferType", mb.bufferLocation.asString());
                }
            }
        }
    }
}
/**
 * convertResourceUsageToXML()
 * This function converts the resource usage of this memory binding to an
 * XML object.
 */
void MemoryBinding::convertResourceUsageToXML(const CNodePtr memoryNode) {
    for (const auto& m : memories) {
        CNode *mem = CAddNode(memoryNode, "memory");
        // Name
        CAddAttribute(mem, "name", m->getName());
        CAddAttribute(mem,
                      "size",
                      CString(getMemorySizeUsedForGraph(m) + getMemorySizeUsedForOtherGraphs(m)));
    }
}
/**
 * getGraphBindingConstraints()
 * Get the graph binding constraints associated with this binding.
 */
GraphBindingConstraints *MemoryBinding::getGraphBindingConstraints() const {
    return getTileBinding()->getPlatformBinding()->getGraphBindingConstraints();
}
/**
 * findActorBinding()
 * The function returns the actor binding for scenario with name
 * scenarioName or returns nullptr if not found.
 */
const MemoryBinding::ActorBindings *MemoryBinding::findActorBinding(const CString &scenarioName) {
    for (auto &[s, actorBindings] : actorBindings) {
        if (s->getName() == scenarioName) {
            return &(actorBindings);
        }
    }
    return nullptr;
}
/**
 * addActorBinding()
 * The function binds actor a in scenario s to this memory. The function
 * returns true on success. Otherwise it returns false.
 */
bool MemoryBinding::addActorBinding(const Scenario& s, Actor *a, const Actor::ProcessorType &t) {
    //    Actor::Memory *mem;
    // Binding already exists?
    if (hasActorBinding(s, a)) {
        return true;
    }
    /// @todo we need a mapping strategy. stupid greedy for now.
    Actor::Memories mem = a->getMemoriesOfScenario(s, t.type);
    for (const auto& aMem : mem) {
        bool mapped = false;

        for (auto j = memories.begin(); !mapped && j != memories.end(); j++) {
            Memory *memory = *j;
            if (aMem->getType() == memory->getType()) {
                if (getAvailableMemorySize(s, memory) >= aMem->getSize()) {
                    actorBindings[&s].emplace_back(a, t, aMem.get(), memory);
                    mapped = true;
                }
            }
        }
        if (!mapped) {
            // Remove the mappings we made so far.
            removeActorBinding(s, a);
            // throw CException("Failed to create a mapping");
            return false;
        }
    }
    return true;
}
/**
 * removeActorBinding()
 * The function removes the binding of actor a in scenario s to this
 * memory.
 */
void MemoryBinding::removeActorBinding(const Scenario& s, const Actor *a) {
    for (auto i = actorBindings[&s].begin(); i != actorBindings[&s].end(); i++) {
        if (i->actor == a) {
            actorBindings[&s].erase(i);
            break;
        }
    }
}
/**
 * hasActorBinding()
 * The function returns true when a binding of actor a in scenario s to this
 * memory exists. Otherwise it returns false.
 */
bool MemoryBinding::hasActorBinding(const Scenario& s, const Actor *a) const {
    auto scIter = actorBindings.find(&s);
    if (scIter == actorBindings.end()) {
        return false;
    }

    return std::any_of(scIter->second.begin(), scIter->second.end(), [a](const ActorBinding &ab) {
        return ab.actor == a;
    });
}
/**
 * addChannelBinding()
 * The function binds channel c in scenario s to this memory. The function
 * returns true on success. Otherwise it returns false.
 */
bool MemoryBinding::addChannelBinding(const Scenario& s, Channel *c, BufferLocation l) {
    ChannelBindingConstraint *cbc =
            getGraphBindingConstraints()->getConstraintsOfScenario(s)->getConstraintsOfChannel(
                    c->getName());

    // Sufficient resources available?
    for (auto *mem : memories) {
        MemoryType type(MemoryType::Value::Shared);
        // Special case: if buffer is mapped to shared memory, the CommOut on the dst tile should be
        // used
        if (l == BufferLocation::Value::Dst && cbc->hasBufferLocationConstraint()
            && cbc->getBufferLocation() == BufferLocation::Value::Mem) { // NOLINT(*branch-clone)
            type = MemoryType::Value::CommOut;
        } else if (l == BufferLocation::Value::Dst) {
            type = MemoryType::Value::CommIn;
        } else if (l == BufferLocation::Value::Src) {
            type = MemoryType::Value::CommOut;
        } else {
            // The channel is placed into the memory (both sides on same tile?)
            type = MemoryType::Value::Data;
        }

        // Binding of same buffer location already exists?
        if (hasChannelBinding(s, c, mem, l)) {
            return true;
        }

        if (mem->getType() == type || mem->getType() == MemoryType::Value::Shared) {
            if (getAvailableMemorySize(s, mem)
                > cbc->getBufferSize()[l.asInt()] * c->getTokenSizeOfScenario(s)) {
                ChannelBinding cb = ChannelBinding(c, l, mem);
                channelBindings[&s].push_back(cb);
                return true;
            }
        }
    }

    return false;
}
/**
 * findChannelBinding()
 * The function returns the channelBinding for scenario with name
 * scenarioName or returns nullptr if not found.
 */
const MemoryBinding::ChannelBindings *
MemoryBinding::findChannelBinding(const CString &scenarioName) {
    for (auto& [s, channelBinding] : channelBindings) {
        if (s->getName() == scenarioName) {
            return &(channelBinding);
        }
    }
    return nullptr;
}
/**
 * removeChannelBinding()
 * The function removes the binding of channel c in scenario s to this
 * memory.
 */
void MemoryBinding::removeChannelBinding(const Scenario& s, const Channel *c) {
    for (auto i = channelBindings[&s].begin(); i != channelBindings[&s].end(); i++) {
        if (i->channel == c) {
            channelBindings[&s].erase(i);
            break;
        }
    }
}
/**
 * hasChannelBinding()
 * The function returns true when a binding of channel c in scenario s to this
 * memory exists. Otherwise it returns false.
 */
bool MemoryBinding::hasChannelBinding(const Scenario& s,
                                      const Channel *c,
                                      Memory *mem,
                                      BufferLocation l) const {
    auto scIter = channelBindings.find(&s);
    if (scIter == channelBindings.end()) {
        return false;
    }

    return std::any_of(
            scIter->second.begin(), scIter->second.end(), [c, mem, l](const ChannelBinding &cb) {
                return cb.channel == c && cb.memory == mem && cb.bufferLocation == l;
            });
}
/**
 * getAvailableMemorySize()
 * The function returns the amount of memory which is available in this memory
 * in scenario s.
 */
Size MemoryBinding::getAvailableMemorySize(const Scenario& s, Memory *m) const {
    if (bindingMethod == BindingMethod::Method::MaxOfScenarios) {
        return (m->getSize() - getAllocatedActorMemorySize(s, m)
                - getAllocatedChannelMemorySize(s, m) - getMemorySizeUsedForOtherGraphs(m));
    }
    auto available =
            static_cast<int32_t>(m->getSize() - getMemorySizeUsedForGraph(m) - getMemorySizeUsedForOtherGraphs(m));
    if (available < 0) {
        return 0;
    }
        return static_cast<Size>(available);
}
/**
 * getAllocatedActorMemorySize()
 * The function returns the amount of memory which has been allocated to the
 * actors of this application in scenario s.
 */
Size MemoryBinding::getAllocatedActorMemorySize(const Scenario& s, Memory *m) const {
    Size szActors = 0;

    /// @todo Actors require max memory size off all bound actors to this memory within scenario s
    // Actors require max state size of all bound actors within scenario s
    auto actorScIter = actorBindings.find(&s);
    if (actorScIter != actorBindings.end()) {
        MemoryBinding::ActorBindings b = actorScIter->second;
        ActorBindings::const_iterator i = b.begin();
        while (i != b.end()) {
            const MemoryBinding::ActorBinding *c = &(*i);
            if (m == (c->tile_memory)) {
                if ((bindingMethod == BindingMethod::Method::SumOfScenarios)
                    && (m->getType() == MemoryType::Value::Instr)) {
                    szActors += c->actor_memory->getSize();
                } else {
                    szActors = MAX(szActors, c->actor_memory->getSize());
                }
            }
            i++;
        }
    }

    return szActors;
}
/**
 * getAllocatedChannelMemorySize()
 * The function returns the amount of memory which has been allocated to the
 * channels of this application in scenario s.
 */
Size MemoryBinding::getAllocatedChannelMemorySize(const Scenario& s, Memory *m) const {
    Size szChannels = 0;

    // Channels require sum of requirements per channel
    auto channelScIter = channelBindings.find(&s);
    if (channelScIter != channelBindings.end()) {
        for (const auto& i: channelScIter->second) {
            Channel *c = i.channel;
            BufferLocation l = i.bufferLocation;
            ChannelBindingConstraint *cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)->getConstraintsOfChannel(
                    c->getName());
            if (i.memory == m) {
                szChannels += cb->getBufferSize()[l.asInt()] * c->getTokenSizeOfScenario(s);
            }
        }
    }

    return szChannels;
}
/**
 * getMemorySizeUsedForGraph()
 * The function returns the amount of memory which has been allocated
 * to the application in its scenarios.
 */
Size MemoryBinding::getMemorySizeUsedForGraph(Memory *mem) const {
    // Application graph
    Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();
    Size maxActorSize = 0;
    Size maxChannelSize = 0;

    for (const auto& i: g->getScenarios()) {
        Size actorSize = getAllocatedActorMemorySize(*i, mem);
        Size channelSize = getAllocatedChannelMemorySize(*i, mem);

        if (bindingMethod == BindingMethod::Method::MaxOfScenarios) {
            maxActorSize = MAX(maxActorSize, actorSize);
            maxChannelSize = MAX(maxChannelSize, channelSize);
        } else {
            if (mem->getType() == MemoryType::Value::Instr) {
                maxActorSize += actorSize;
            } else {
                maxActorSize = MAX(maxActorSize, actorSize);
            }
            maxChannelSize += channelSize;
        }
    }

    return maxActorSize + maxChannelSize;
}

// Tile binding
TileBinding *MemoryBinding::getTileBinding() const {
    return dynamic_cast<TileBinding *>(getParent());
};


} // namespace FSMSADF
