/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   interconnect_graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Interconnect graph
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: interconnect_graph.cc,v 1.1 2008-03-20 16:16:19 sander Exp $
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

#include "interconnect_graph.h"
namespace SDF {
/**
 * InterconnectGraph ()
 * Constructor.
 */
InterconnectGraph::InterconnectGraph(uint slotTableSize,
                                     TTime slotTablePeriod,
                                     uint packetHeaderSize,
                                     uint flitSize,
                                     TTime reconfigurationTimeNI) :
    slotTableSize(slotTableSize),
    slotTablePeriod(slotTablePeriod),
    packetHeaderSize(packetHeaderSize),
    flitSize(flitSize),
    reconfigurationTimeNI(reconfigurationTimeNI) {}

/**
 * InterconnectGraph ()
 * Constructor.
 */
InterconnectGraph::InterconnectGraph(const CNodePtr archNode, TTime slotTablePeriod) :
    slotTablePeriod(slotTablePeriod) {
    // Network node in architecture
    CNode *networkNode = CGetChildNode(archNode, "network");
    if (networkNode == nullptr) {
        throw CException("[ERROR] Missing 'network' in 'architectureGraph'.");
    }

    // Network properties
    slotTableSize = static_cast<uint>(CGetAttribute(networkNode, "slotTableSize"));
    packetHeaderSize = static_cast<uint>(CGetAttribute(networkNode, "packetHeaderSize"));
    flitSize = static_cast<uint>(CGetAttribute(networkNode, "flitSize"));
    reconfigurationTimeNI = static_cast<TTime>(CGetAttribute(networkNode, "reconfigurationTimeNI"));

    // Create node in the architecture graph for each tile
    for (CNode *tileNode = CGetChildNode(archNode, "tile"); tileNode != nullptr;
         tileNode = CNextNode(tileNode, "tile")) {
        CString name = CGetAttribute(tileNode, "name");
        Node *n = createNode(name);
        n->setType("tile");
    }

    // Create node in the architecture graph for each router
    for (CNode *routerNode = CGetChildNode(networkNode, "router"); routerNode != nullptr;
         routerNode = CNextNode(routerNode, "router")) {
        CString name = CGetAttribute(routerNode, "name");
        Node *n = createNode(name);
        n->setType("router");
    }

    // Create link between the nodes in the architecture graph
    for (CNode *linkNode = CGetChildNode(networkNode, "link"); linkNode != nullptr;
         linkNode = CNextNode(linkNode, "link")) {
        CString name = CGetAttribute(linkNode, "name");
        CString src = CGetAttribute(linkNode, "src");
        CString dst = CGetAttribute(linkNode, "dst");

        Node *srcNode = getNode(src);
        Node *dstNode = getNode(dst);

        createLink(name, srcNode, dstNode);
    }
}

/**
 * ~InterconnectGraph ()
 * Destructor.
 */
InterconnectGraph::~InterconnectGraph() = default;

/**
 * createNode ()
 * Create a new node and add it to the graph.
 */
Node *InterconnectGraph::createNode(const CString &name) {
    // Node with the same name does exist?
    if (getNode(name) != nullptr) {
        throw CException("[ERROR] Duplicate node name.");
    }

    // Create a new node and add it to the graph
    auto n = std::make_unique<Node>(name, nrNodes());
    Node* nodeRef = n.get();
    nodes.push_back(std::move(n));
    return nodeRef;
}

/**
 * createLink ()
 * Create a new link and add it to the graph.
 */
Link *InterconnectGraph::createLink(const CString &name, Node *src, Node *dst) {
    // Link with the same name does exist?
    if (getLink(name) != nullptr) {
        throw CException("[ERROR] Duplicate link name.");
    }

    // Create a new link and add it to the graph
    auto l = std::make_unique<Link>(name, nrLinks(), src, dst, getSlotTableSize(), getSlotTablePeriod());
    Link* linkRef = l.get();
    links.push_back(std::move(l));
    src->addOutgoingLink(linkRef);
    dst->addIncomingLink(linkRef);

    return linkRef;
}

/**
 * getNode ()
 * The function returns a pointer to the node with the supplied id, or nullptr if
 * no such node exists.
 */
Node *InterconnectGraph::getNode(const CId id) {
    for (const auto& n : nodes) {
        if (n->getId() == id) {
            return n.get();
        }
    }

    return nullptr;
}

/**
 * getNode ()
 * The function returns a pointer to the node with the supplied name, or nullptr if
 * no such node exists.
 */
Node *InterconnectGraph::getNode(const CString &name) {
    for (const auto& n : nodes) {
        if (n->getName() == name) {
            return n.get();
        }
    }

    return nullptr;
}

/**
 * getLink ()
 * The function returns a pointer to the link with the supplied id, or nullptr if
 * no such link exists.
 */
Link *InterconnectGraph::getLink(const CId id) {
    for (const auto& l : links) {
        if (l->getId() == id) {
            return l.get();
        }
    }

    return nullptr;
}

/**
 * getLink ()
 * The function returns a pointer to the link with the supplied name, or nullptr if
 * no such link exists.
 */
Link *InterconnectGraph::getLink(const CString &name) {
    for (const auto& l : links) {
        if (l->getName() == name) {
            return l.get();
        }
    }

    return nullptr;
}

/**
 * setUsageInterconnectGraph ()
 * Reserve resource in the interconnect graph.
 */
void InterconnectGraph::setUsage(const CNodePtr usageNode) {
    SlotReservations usedSlots(getSlotTableSize());

    // Usage information given?
    if (usageNode == nullptr) {
        return;
    }

    // Network node in architecture
    CNode *networkNode = CGetChildNode(usageNode, "network");
    if (networkNode == nullptr) {
        return;
    }

    // Set usage of each link in the network
    for (CNode *linkNode = CGetChildNode(networkNode, "link"); linkNode != nullptr;
         linkNode = CNextNode(linkNode, "link")) {
        // Find link in architecture graph
        CString name = CGetAttribute(linkNode, "name");
        Link *l = getLink(name);
        if (l == nullptr) {
            throw CException("[ERROR] Cannot set usage of non existing link.");
        }

        // Mark all slots as not used
        for (uint i = 0; i < getSlotTableSize(); i++) {
            usedSlots[i] = false;
}

        // Split string of occupied slots in a list and mark the individual
        // slots which are occupied as used
        CString occupiedSlots = CGetAttribute(linkNode, "occupiedSlots");
        CStrings slots;
        slots.clear();
        stringtok(slots, occupiedSlots, ",");
        for (auto &s: slots) {
            uint sl = static_cast<uint>(s);

            if (sl >= getSlotTableSize()) {
                throw CException("[ERROR] Cannot mark slot outside "
                                 "slot table as used.");
            }

            usedSlots[sl] = true;
        }

        // Mark all occupied slots
        l->setUsedSlots(usedSlots, 0, TTIME_MAX);
    }
}

/**
 * createInterconnectGraphNode ()
 * The function returns a description of the interconnect graph in XML format.
 */
CNode *InterconnectGraph::createInterconnectGraphNode() {

    // Create new node
    CNode* networkNode = CNewNode("network");

    // Add properties to the network node
    CAddAttribute(networkNode, "slotTableSize", CString(getSlotTableSize()));
    CAddAttribute(networkNode, "packetHeaderSize", CString(getPacketHeaderSize()));
    CAddAttribute(networkNode, "flitSize", CString(getFlitSize()));
    CAddAttribute(networkNode, "reconfigurationTimeNI", CString(getReconfigurationTimeNI()));

    // Create node for every tile and router
    for (const auto& n: nodes) {
        // Create node
        CNode* nodeNode = CAddNode(networkNode, n->getType());
        CAddAttribute(nodeNode, "name", n->getName());
    }

    // Create node for every link
    for (const auto& l: links) {
        // Create node
        CNode* linkNode = CAddNode(networkNode, "link");
        CAddAttribute(linkNode, "name", l->getName());
        CAddAttribute(linkNode, "src", l->getSrcNode()->getName());
        CAddAttribute(linkNode, "dst", l->getDstNode()->getName());
    }

    return networkNode;
}

} // namespace SDF