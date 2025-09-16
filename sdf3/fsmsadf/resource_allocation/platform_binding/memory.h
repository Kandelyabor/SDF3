/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: memory.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_MEMORY_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_MEMORY_H_INCLUDED

#include <utility>

#include "fsmsadf/base/graph.h"  // IWYU pragma: keep
#include "constraint.h"
#include "fsmsadf/resource_allocation/platform_graph/graph.h"  // IWYU pragma: keep



namespace FSMSADF {

// Forward class definition
class MemoryBinding;
class TileBinding;

// List of memory pointers
using MemoryBindings = std::list<MemoryBinding *>;

/**
 * MemoryBinding
 * Container for tile memory binding.
 */
class MemoryBinding : public GraphComponent {
public:
    // Actor binding
    class ActorBinding {
    public:
        ActorBinding(Actor *a, Actor::ProcessorType t, const Actor::Memory *memory, Memory *tmem) :
            actor(a), processorType(std::move(t)), actor_memory(memory), tile_memory(tmem){};
        Actor *actor;
        Actor::ProcessorType processorType;
        // Want to create a link between an actor memory and a Tile memory
        const Actor::Memory *actor_memory;
        Memory *tile_memory;
    };
    using ActorBindings = std::list<ActorBinding>;

    // Channel binding
    class ChannelBinding {
    public:
        ChannelBinding(Channel *c, BufferLocation l, Memory *mem) :
            channel(c), bufferLocation(l), memory(mem){};
        Channel *channel;
        BufferLocation bufferLocation;
        Memory *memory;
    };
    using ChannelBindings = std::list<ChannelBinding>;

    // Constructor
    MemoryBinding(const GraphComponent& c, MemoryRefs m, BindingMethod bm);

    // Destructor
    ~MemoryBinding() override;

    MemoryBinding(const MemoryBinding&) = delete;
    MemoryBinding& operator=(const MemoryBinding&) = delete;
    MemoryBinding(MemoryBinding&&) = delete;
    MemoryBinding& operator=(MemoryBinding&&) = delete;

    // Construct
    std::unique_ptr<MemoryBinding> clone(const GraphComponent& c);

    // Construct
    void constructFromXML(const Scenario& s, CNodePtr memoryNode);
    void constructResourceUsageFromXML(Memory *m, CNodePtr tileNode);

    // Convert
    void convertToXML(const Scenario& s, CNodePtr tileNode);
    void convertResourceUsageToXML(CNodePtr memoryNode);

    // Memory
    [[nodiscard]] const MemoryRefs& getMemories() const { return memories; };

    // Graph binding constraints
    [[nodiscard]] GraphBindingConstraints *getGraphBindingConstraints() const;

    // Actor bindings
    std::map<const Scenario *, ActorBindings> &getActorBindings() { return actorBindings; };
    void setActorBindings(std::map<const Scenario *, ActorBindings> &a) { actorBindings = a; };
    const ActorBindings *findActorBinding(const CString& scenarioName);
    bool addActorBinding(const Scenario& s, Actor *a, const Actor::ProcessorType &t);

    void removeActorBinding(const Scenario& s, const Actor *a);
    bool hasActorBinding(const Scenario& s, const Actor *a) const;

    // Channel bindings
    std::map<const Scenario *, ChannelBindings> &getChannelBindings() { return channelBindings; };
    void setChannelBindings(std::map<const Scenario *, ChannelBindings> &ch) { channelBindings = ch; };
    const ChannelBindings *findChannelBinding(const CString& scenarioName);
    bool addChannelBinding(const Scenario& s, Channel *c, BufferLocation l);
    void removeChannelBinding(const Scenario& s, const Channel *c);
    bool hasChannelBinding(const Scenario& s, const Channel *c, Memory *mem, BufferLocation l) const;

    // Tile binding
    [[nodiscard]] TileBinding *getTileBinding() const;

    // Resource occupancy
    Size getAvailableMemorySize(const Scenario& s, Memory *m) const;
    Size getAllocatedActorMemorySize(const Scenario& s, Memory *m) const;
    Size getAllocatedChannelMemorySize(const Scenario& s, Memory *m) const;

    // Resource occupancy of application graph
    Size getMemorySizeUsedForGraph(Memory *mem) const;

    // Initial resource occupancy
    // Function uses find instead of [] so it can be const.
    Size getMemorySizeUsedForOtherGraphs(Memory *m) const {
        auto i = memorySizeUsedForOtherGraphs.find(m);
        if (i == memorySizeUsedForOtherGraphs.end()) {
            throw CException("MemorySizeUsedForOtherGraphs not found");
        }
        return i->second;
    };
    void setMemorySizeUsedForOtherGraphs(Memory *m, Size sz) {
        memorySizeUsedForOtherGraphs[m] = sz;
    };

    // Get method used for calculating size of binding
    BindingMethod &getBindingMethod() { return bindingMethod; }

    // Set method used for calculating size of binding
    void setBindingMethod(BindingMethod bm) { bindingMethod = bm; }

private:
    // List of memories on the tile
    MemoryRefs memories;

    // Actors
    std::map<const Scenario *, ActorBindings> actorBindings;

    // Channels
    std::map<const Scenario *, ChannelBindings> channelBindings;

    // Memory size used for other graphs
    std::map<Memory *, Size> memorySizeUsedForOtherGraphs;

    // Method used for calculating size of binding
    BindingMethod bindingMethod;
};

} // End namespace FSMSADF

#endif
