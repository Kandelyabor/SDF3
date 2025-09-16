/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: tile.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "tile.h"
#include "graph.h"

namespace FSMSADF {

/**
 * TileBinding ()
 * Constructor.
 */
TileBinding::TileBinding(const GraphComponent &c, Tile *t, BindingMethod bm, bool create) :
    GraphComponent(c), tile(t), bindingMethod(bm), memoryBinding(nullptr) {
    // Create processor, memory and network interface binding object for all
    // corresponding elements in the tile?
    if (create) {
        // Create binding object for all processors
        for (const auto& p : t->getProcessors()) {
            createProcessorBinding(p.get());
        }

        /// @todo fix the ref assignment
        createMemoryBinding(MemoryRefs(t->getMemories()), bm);

        // Create binding object for all network interfaces
        for (const auto& ni : t->getNetworkInterfaces()) {
            createNetworkInterfaceBinding(ni.get());
        }
    }
}

/**
 * ~TileBinding ()
 * Destructor.
 */
TileBinding::~TileBinding() = default;

/**
 * clone()
 * Create a cloned copy of this tile binding.
 */
std::unique_ptr<TileBinding> TileBinding::clone(const GraphComponent &c) {
    auto tb = std::make_unique<TileBinding>(c, getTile(), getBindingMethod(), false);

    // Processor bindings
    for (const auto& pb : processorBindings) {
        std::unique_ptr<ProcessorBinding> pbNew = pb->clone(GraphComponent(tb.get(), pb->getId(), pb->getName()));
        tb->getProcessorBindings().push_back(std::move(pbNew));
    }

    // Memory bindings
    tb->memoryBinding = memoryBinding->clone(
            GraphComponent(tb.get(), memoryBinding->getId(), memoryBinding->getName()));

    // Network interface bindings
    for (const auto& nb : networkInterfaceBindings) {
        std::unique_ptr<NetworkInterfaceBinding> nbNew = nb->clone(GraphComponent(tb.get(), nb->getId(), nb->getName()));
        tb->getNetworkInterfaceBindings().push_back(std::move(nbNew));
    }

    return tb;
}

/**
 * constructFromXML()
 * This function creates the tile binding as specified by the
 * tileNode.
 */
void TileBinding::constructFromXML(const Scenario& s, const CNodePtr tileNode) {
    // Iterate over all child nodes in the tile
    for (CNode *n = CGetChildNode(tileNode); n != nullptr; n = CNextNode(n)) {
        // Processor?
        if (CIsNode(n, "processor")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Processor has no name identifier.");
            }

            // Locate processor
            Processor *p = getTile()->getProcessor(CGetAttribute(n, "name"));

            // Set resource usage
            getProcessorBinding(*p)->constructFromXML(s, n);
        }

        // Memory?
        /// @todo fix this.
        if (CIsNode(n, "memory")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Memory has no name identifier.");
            }

            // Set binding method
            getMemoryBinding()->setBindingMethod(getBindingMethod());

            // Set resource usage
            getMemoryBinding()->constructFromXML(s, n);
        }

        // Network interface?
        if (CIsNode(n, "networkInterface")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Network interface has no name identifier.");
            }

            // Locate network interface
            NetworkInterface *ni = getTile()->getNetworkInterface(CGetAttribute(n, "name"));

            // Set resource usage
            getNetworkInterfaceBinding(ni)->constructFromXML(s, n);
        }
    }
}

/**
 * constructResourceUsageFromXML()
 * This function sets the resource usage of the tile.
 */
void TileBinding::constructResourceUsageFromXML(const CNodePtr tileNode) {
    // Iterate over all child nodes in the tile
    for (CNode *n = CGetChildNode(tileNode); n != nullptr; n = CNextNode(n)) {
        // Processor?
        if (CIsNode(n, "processor")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Processor has no name identifier.");
            }

            // Locate processor
            Processor *p = getTile()->getProcessor(CGetAttribute(n, "name"));

            // Set resource usage
            getProcessorBinding(*p)->constructResourceUsageFromXML(n);
        }

        // Memory?
        if (CIsNode(n, "memory")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Memory has no name identifier.");
            }
            /// @todo FIX THIS!
            // Locate memory
            Memory *m = getTile()->getMemory(CGetAttribute(n, "name"));

            // Set resource usage
            getMemoryBinding()->constructResourceUsageFromXML(m, n);
        }

        // Network interface?
        if (CIsNode(n, "networkInterface")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Network interface has no name identifier.");
            }

            // Locate network interface
            NetworkInterface *ni = getTile()->getNetworkInterface(CGetAttribute(n, "name"));

            // Set resource usage
            getNetworkInterfaceBinding(ni)->constructResourceUsageFromXML(n);
        }
    }
}

/**
 * convertToXML()
 * This function converts the tile binding to an XML object.
 */
void TileBinding::convertToXML(const Scenario& s, const CNodePtr tileNode) {
    // Name
    CAddAttribute(tileNode, "name", getName());

    // Iterate over all processor bindings
    for (const auto& pb : processorBindings) {
        pb->convertToXML(s, CAddNode(tileNode, "processor"));
    }

    // Iterate over all memory bindings
    // This function will add subnodes memory. so pass it the tileNode.
    // Memories are only known in the binding, not the tile.
    memoryBinding->convertToXML(s, tileNode);

    // Iterate over all network interface bindings
    for (const auto& nib : networkInterfaceBindings) {
        nib->convertToXML(s, CAddNode(tileNode, "networkInterface"));
    }
}

/**
 * convertToXML()
 * This function converts the resource usage of this tile binding
 * to an XML object.
 */
void TileBinding::convertResourceUsageToXML(const CNodePtr tileNode) {
    // Name
    CAddAttribute(tileNode, "name", getName());

    // Iterate over all processor bindings
    for (const auto& pb : processorBindings) {
        pb->convertResourceUsageToXML(CAddNode(tileNode, "processor"));
    }

    // Iterate over all memory bindings
    // this function it will add memory subnodes, so pass it tileNode
    // This is because there can be multiple memories known by the binding
    // and not the tile.
    /// @todo make the tile know about memories?
    memoryBinding->convertResourceUsageToXML(tileNode);

    // Iterate over all network interface bindings
    for (const auto& nib : networkInterfaceBindings) {
        nib->convertResourceUsageToXML(CAddNode(tileNode, "networkInterface"));
    }
}

/**
 * getProcessorBinding()
 * The function returns a pointer to the processor binding associated
 * with processor p.
 */
ProcessorBinding* TileBinding::getProcessorBinding(const Processor& p) {
    for (const auto& pb : processorBindings) {
        if (pb->getProcessor() == &p) {
            return pb.get();
        }
    }

    throw CException("Processor '" + p.getName() + "' has no binding.");
}

/**
 * getMemoryBinding()
 * The function returns a pointer to the memory binding
 */
MemoryBinding *TileBinding::getMemoryBinding() { return memoryBinding.get(); }

/**
 * getNetworkInterfaceBinding()
 * The function returns a pointer to the network interface binding associated
 * with network interface n.
 */
NetworkInterfaceBinding *TileBinding::getNetworkInterfaceBinding(const NetworkInterface *n) {
    for (const auto& nib : networkInterfaceBindings) {
        if (nib->getNetworkInterface() == n) {
            return nib.get();
        }
    }

    throw CException("Network interface '" + n->getName() + "' has no binding.");
}

/**
 * createProcessorBinding()
 * The function creates a processor binding object for processor p
 * and adds it to the list of processor bindings associated with this
 * tile binding.
 */
ProcessorBinding* TileBinding::createProcessorBinding(Processor *p) {
    auto pb = std::make_unique<ProcessorBinding>(GraphComponent(this, p->getId(), p->getName()), p);
    auto* pbr = pb.get();
    processorBindings.push_back(std::move(pb));

    return pbr;
}

/**
 * createMemoryBinding()
 * The function creates a memory binding object for memory m
 * and adds it to the list of memory bindings associated with this
 * tile binding.
 */
MemoryBinding *TileBinding::createMemoryBinding(const MemoryRefs& m, BindingMethod bm) {
    /// @todo what id,name?
    auto mb = std::make_unique<MemoryBinding>(GraphComponent(this, 0, "MemBinding"), m, bm);
    memoryBinding = std::move(mb);

    auto* mbr = mb.get();
    return mbr;
}

/**
 * createNetworkInterfaceBinding()
 * The function creates a network interface binding object for network interface
 * n and adds it to the list of network interface bindings associated with this
 * tile binding.
 */
NetworkInterfaceBinding *TileBinding::createNetworkInterfaceBinding(NetworkInterface *n) {
    auto nb = std::make_unique<NetworkInterfaceBinding>(GraphComponent(this, n->getId(), n->getName()), n);
    auto* nbr = nb.get();
    networkInterfaceBindings.push_back(std::move(nb));

    return nbr;
}

/**
 * @brief Get the Platform Binding object
 *
 * @return Platform*
 */
PlatformBinding *TileBinding::getPlatformBinding() const {
    return dynamic_cast<PlatformBinding *>(getParent());
}


} // End namespace FSMSADF
