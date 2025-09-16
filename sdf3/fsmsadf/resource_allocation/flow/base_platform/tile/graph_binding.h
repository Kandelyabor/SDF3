/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph_binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 25, 2009
 *
 *  Function        :   Graph binding
 *
 *  History         :
 *      25-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: graph_binding.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_GRAPH_BINDING_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_GRAPH_BINDING_H_INCLUDED

#include "fsmsadf/resource_allocation/platform_binding/graph.h"
#include <memory>

namespace FSMSADF {

// Forward class definitions
class ActorBinding;
class ChannelBinding;
class GraphBindingBase;

// List of actor bindings
using ActorBindings = std::list<ActorBinding>;

// List of channel bindings
using ChannelBindings = std::list<ChannelBinding>;

// List of graph bindings
class GraphBindings: public std::list<std::unique_ptr<GraphBindingBase>> {
public:

    GraphBindings() = default;
    ~GraphBindings() = default;
    GraphBindings(const GraphBindings &) = delete;
    GraphBindings& operator=(const GraphBindings &) = delete;
    GraphBindings(GraphBindings&&) = delete;
    GraphBindings& operator=(GraphBindings&&) = delete;


    // Remove a graph binding
    [[nodiscard]] std::unique_ptr<GraphBindingBase> removeGraphBinding(GraphBindingBase *g) {
        auto it = std::find_if(this->begin(), this->end(),
                               [g](const std::unique_ptr<GraphBindingBase>& gb) { return gb.get() == g; });
        if (it != this->end()) {
            std::unique_ptr<GraphBindingBase> removed = std::move(*it);
            this->erase(it);
            return removed;
        }
        return nullptr;
    }
};

/**
 * Actor binding
 */
class ActorBinding {
public:
    // Constructor
    explicit ActorBinding(Processor *p = nullptr,
                          Memory *m = nullptr,
                          Actor::Memory *am = nullptr) :
        processor(p),
        tile_memory(m),
        actor_memory(am){

        };

    Processor *processor;
    // We bind Actor::Memory to a Memory
    Memory *tile_memory;
    const Actor::Memory *actor_memory;
};

/**
 * Channel binding
 */
class ChannelBinding {
public:
    // Constructor
    explicit ChannelBinding(Memory *m = nullptr) :
        memoryMem(m){

        };

    // Constructor
    ChannelBinding(Memory *mSrc,
                   NetworkInterface *nSrc,
                   Memory *mDst,
                   NetworkInterface *nDst,
                   Connection *c) : memoryMem(nullptr), memorySrc(mSrc), memoryDst(mDst), networkInterfaceIn(nDst), networkInterfaceOut(nSrc), connection(c) {








    };

    // Constructor
    ChannelBinding(Memory *mSrc,
                   NetworkInterface *nSrc,
                   Memory *mDst,
                   NetworkInterface *nDst,
                   Memory *mMem,
                   Connection *cToMem,
                   Connection *cFromMem) : memoryMem(mMem), memorySrc(mSrc), memoryDst(mDst), networkInterfaceIn(nDst), networkInterfaceOut(nSrc), connToMem(cToMem), connFromMem(cFromMem) {








    };


    Memory *memoryMem;
    Memory *memorySrc{nullptr};
    Memory *memoryDst{nullptr};
    NetworkInterface *networkInterfaceIn{nullptr};
    NetworkInterface *networkInterfaceOut{nullptr};
    Connection *connection{nullptr};
    Connection *connToMem{nullptr};
    Connection *connFromMem{nullptr};
};

/**
 * class GraphBinding
 * This class annotates the platform binding with information used by the
 * tile binding algorithm.
 */
class GraphBindingBase {
public:
    // Rule of Five: explicitly default or delete special member functions
    GraphBindingBase(const GraphBindingBase&) = delete;
    GraphBindingBase& operator=(const GraphBindingBase&) = delete;
    GraphBindingBase(GraphBindingBase&&) = delete;
    GraphBindingBase& operator=(GraphBindingBase&&) = delete;

    class ParetoQuantities {
    public:
        // Pareto dominance
        [[nodiscard]] bool dominates(const ParetoQuantities &q) const;

        // Euclidean distance
        [[nodiscard]] double distance(const ParetoQuantities &q) const;

        // Print quantities
        std::ostream &print(std::ostream &out) const;


        // Quantities
        std::map<Processor *, double> processingLoad;
        std::map<Memory *, double> memoryLoad;
        std::map<NetworkInterface *, double> communicationLoad;
    };


    // Constructor
    explicit GraphBindingBase(const PlatformBinding *b);

    // Destructor
    virtual ~GraphBindingBase();

    // Constructor
    [[nodiscard]] virtual std::unique_ptr<GraphBindingBase> clone() const;

    // Platform binding
    [[nodiscard]] PlatformBinding* getPlatformBinding() const { return platformBinding.get(); };

    // Actors bindings
    std::map<const Scenario *, std::map<const Actor *, ActorBinding>> &getActorBindings() {
        return actorBindings;
    };

    // Channel bindings
    std::map<const Scenario *, std::map<const Channel *, ChannelBinding>> &getChannelBindings() {
        return channelBindings;
    };

    // Bind actor to tile resources
    virtual bool bindActorToTile(const Scenario& s, Actor *a, ActorBinding &b);

    // Remove binding of actor to tile resources
    virtual void removeActorToTileBinding(const Scenario& s, Actor *a);

    // Bind channel to tile resources
    virtual bool bindChannelToTile(const Scenario& s, Channel *c, ChannelBinding &b);

    // Bind channel to connection resources
    virtual bool bindChannelToConnection(const Scenario& s, Channel *c, ChannelBinding &b);

    // Bind channel to connection resources
    virtual bool bindChannelToSharedMemory(const Scenario& s, Channel *c, ChannelBinding &b);

    // Remove binding of channel to platform resources
    virtual void removeChannelBinding(const Scenario& s, Channel *c);

    // Minimum TDMA time slices on processors
    std::map<const Processor *, std::map<const Scenario *, Time>> &getMinTDMAslices() { return minTDMAslices; };
    [[nodiscard]] Time getMinTDMAslice(const Scenario& s, const Processor& p) const;
    void setMinTDMAslice(const Scenario& s, const Processor& p, Time t);

    // Maximum TDMA time slices on processors
    std::map<const Processor *, std::map<const Scenario *, Time>> &getMaxTDMAslices() { return maxTDMAslices; };

    [[nodiscard]] Time getMaxTDMAslice(const Scenario& s, const Processor& p) const;
    void setMaxTDMAslice(const Scenario& s, const Processor& p, Time t);

    // Allocate TDMA time slice on processor
    bool allocateTDMAslice(const Scenario& s, const Processor& p, Time t);

    // Get allocated TDMA time slice on processor
    Time getAllocatedTDMAslice(const Scenario& s, const Processor& p);

    // Pareto quantities
    void computeParetoQuantities(const std::map<const Scenario *, RepetitionVector> &repetitionVectors,
                                  bool processingLoadBasedOnTDMAslices);
    [[nodiscard]] const ParetoQuantities &getParetoQuantities() const { return paretoQuantities; };
    bool paretoDominates(const GraphBindingBase *g) const {
        return paretoQuantities.dominates(g->getParetoQuantities());
    };

private:
    // Extract application binding from platform binding
    void extractApplicationBindingFromPlatformBinding();

    // Get platform binding object associated with platform object
    [[nodiscard]] ProcessorBinding* getProcessorBinding(const Processor& p) const;
    MemoryBinding *getMemoryBinding(Memory *m) const;
    NetworkInterfaceBinding *getNetworkInterfaceBinding(NetworkInterface *n) const;
    ConnectionBinding *getConnectionBinding(Connection *c) const;

protected:
    void copyInternalsTo(GraphBindingBase *gb) const;

private:
    // Platform binding
    std::unique_ptr<PlatformBinding> platformBinding;

    // Actors bindings
    std::map<const Scenario *, std::map<const Actor *, ActorBinding>> actorBindings;

    // Channel bindings
    std::map<const Scenario *, std::map<const Channel*, ChannelBinding>> channelBindings;

    // TDMA bounds
    std::map<const Processor *, std::map<const Scenario *, Time>> minTDMAslices;
    std::map<const Processor *, std::map<const Scenario *, Time>> maxTDMAslices;

    // Pareto quantities
    ParetoQuantities paretoQuantities;
};

} // End namespace FSMSADF

#endif
