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
 * $Id: tile.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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
#include <algorithm>

namespace FSMSADF {

/**
 * Tile ()
 * Constructor.
 */
Tile::Tile(const GraphComponent &c) : GraphComponent(c) {}

/**
 * ~Tile ()
 * Destructor.
 */
Tile::~Tile() = default;

/**
 * create()
 * The function creates a new tile object
 */
std::unique_ptr<Tile> Tile::create(const GraphComponent &c) { return std::make_unique<Tile>(c); }

/**
 * createCopy()
 * The function creates a copy of this tile.
 */
std::unique_ptr<Tile> Tile::createCopy(const GraphComponent &c) const {
    std::unique_ptr<Tile> t = create(c);

    // Processors
    for (const auto& p : processors) {
        t->addProcessor(p->createCopy(GraphComponent(t.get(), getId(), getName())));
    }

    // Memories
    for (const auto& m : memories) {
        t->addMemory(m->createCopy(GraphComponent(t.get(), getId(), getName())));
    }

    // Network interfaces
    for (const auto& ni : networkInterfaces) {
        t->addNetworkInterface(ni->createCopy(GraphComponent(t.get(), getId(), getName())));
    }

    return t;
}

/**
 * clone()
 * The function creates a clone of this tile.
 */
std::unique_ptr<Tile> Tile::clone(const GraphComponent &c) const {
    std::unique_ptr<Tile> t = create(c);

    // Processors
    for (const auto& p : processors) {
        t->addProcessor(p->clone(GraphComponent(t.get(), getId(), getName())));
    }

    // Memories
    for (const auto& m : memories) {
        t->addMemory(m->clone(GraphComponent(t.get(), getId(), getName())));
    }

    // Network interfaces
    for (const auto& ni : networkInterfaces) {
        t->addNetworkInterface(ni->clone(GraphComponent(t.get(), getId(), getName())));
    }

    return t;
}

/**
 * constructFromXML()
 * The function sets all elements of this tile using the supplied tile node.
 */
void Tile::constructFromXML(const CNodePtr tileNode) {
    // Name
    setName(CGetAttribute(tileNode, "name"));

    // Iterate over all child nodes of the tile
    for (CNode *n = CGetChildNode(tileNode); n != nullptr; n = CNextNode(n)) {
        // Processor
        if (CIsNode(n, "processor")) {
            Processor *p = createProcessor("(no name)");
            p->constructFromXML(n);
        }

        // Memory
        if (CIsNode(n, "memory")) {
            Memory *m = createMemory("(no name)");
            m->constructFromXML(n);
        }

        // Network interface
        if (CIsNode(n, "networkInterface")) {
            NetworkInterface *ni = createNetworkInterface("(no name)");
            ni->constructFromXML(n);
        }
    }
}

/**
 * convertToXML()
 * The function adds the elements of this tile to the supplied tile node.
 */
void Tile::convertToXML(const CNodePtr tileNode) {
    // Name
    CAddAttribute(tileNode, "name", getName());

    // Processors
    for (const auto& p : processors) {
        p->convertToXML(CAddNode(tileNode, "processor"));
    }

    // Memories
    for (const auto& m : memories) {
        m->convertToXML(CAddNode(tileNode, "memory"));
    }

    // Network interfaces
    for (const auto& ni : networkInterfaces) {
        ni->convertToXML(CAddNode(tileNode, "networkInterface"));
    }
}

/**
 * getProcessor()
 * The function returns a pointer to a processor with the supplied name. When
 * no such processor exists, an exception is thrown.
 */
Processor *Tile::getProcessor(const CString &name) const {
    for (const auto& p : processors) {
        if (name == p->getName()) {
            return p.get();
        }
    }

    throw CException("Tile does not contain a processor with name '" + name + "'.");
}

/**
 * createProcessor()
 * The function creates a new processor and adds it to this tile.
 */
Processor *Tile::createProcessor(const CString &name) {
    auto p = std::make_unique<Processor>(GraphComponent(this, Processor::uniqueId(), name));
    auto* pr = p.get();
    addProcessor(std::move(p));
    return pr;
}

/**
 * addProcessor()
 * The function adds processor p to this tile.
 */
void Tile::addProcessor(std::unique_ptr<Processor>p) { processors.push_back(std::move(p)); }

/**
 * removeProcessor()
 * The function removes the processor p from the tile.
 */
void Tile::removeProcessor(Processor *p) {
    for (auto i = processors.begin(); i != processors.end(); i++) {
        if (p->getName() == (*i)->getName()) {
            processors.erase(i);
            break;
        }
    }
}

/**
 * getMemory()
 * The function returns a pointer to a memory with the supplied
 * name. When no such memory exists, an exception is thrown.
 */
Memory *Tile::getMemory(const CString &name) const {
    for (const auto& m : memories) {
        if (name == m->getName()) {
            return m.get();
        }
    }

    throw CException("Tile does not contain a memory with name '" + name + "'.");
}

/**
 * createMemory()
 * The function creates a new memory and adds it to the tile.
 */
Memory *Tile::createMemory(const CString &name) {
    auto m = std::make_unique<Memory>(GraphComponent(this, Memory::uniqueId(), name));
    auto* mr = m.get();
    addMemory(std::move(m));
    return mr;
}

/**
 * addMemory()
 * The function adds memory m to the tile.
 */
void Tile::addMemory(std::unique_ptr<Memory> m) { memories.push_back(std::move(m)); }

/**
 * removeMemory()
 * The function removes memory m from the tile.
 */
void Tile::removeMemory(Memory *m) {
    for (auto i = memories.begin(); i != memories.end(); i++) {
        if (m->getName() == (*i)->getName()) {
            memories.erase(i);
            break;
        }
    }
}

/**
 * hasSharedMemories()
 * The function returns true if the tile has shared memories.
 */
bool Tile::hasSharedMemories() {
    return std::any_of(memories.begin(), memories.end(), [](const std::unique_ptr<Memory>& m) {
        return m->getType() == MemoryType::Value::Shared;
    });
}

/**
 * getSharedMemories()
 * The function returns a list of the tiles shared memories.
 */
std::unique_ptr<MemoryRefs> Tile::getSharedMemories() {
    auto sharedMemories = std::make_unique<MemoryRefs>();
    for (const auto& i : memories) {
        if (i->getType() == MemoryType::Value::Shared) {
            sharedMemories->push_back(i.get());
        }
    }
    return sharedMemories;
}

/**
 * getNetworkInterface()
 * The function returns a pointer to a network interface with the supplied
 * name. When no such network interface exists, an exception is thrown.
 */
NetworkInterface *Tile::getNetworkInterface(const CString &name) const {
    // Iterate over the network interfaces
    for (const auto& ni : networkInterfaces) {
        if (ni->getName() == name) {
            return ni.get();
        }
    }

    throw CException("Tile does not contain a network interface with name '" + name + "'.");
}

/**
 * createNetworkInterface()
 * The function creates a new network interface and adds it to this tile.
 */
NetworkInterface *Tile::createNetworkInterface(const CString &name) {
    // Create a new NI
    auto n = std::make_unique<NetworkInterface>(GraphComponent(this, static_cast<CId>(networkInterfaces.size()), name));

    auto* nr = n.get();

    // Add NI to tile
    addNetworkInterface(std::move(n));

    return nr;
}

/**
 * addNetworkInterface()
 * The function adds the network interface n to the tile.
 */
void Tile::addNetworkInterface(std::unique_ptr<NetworkInterface> n) { networkInterfaces.push_back(std::move(n)); }

/**
 * removeNetworkInterface()
 * The function removes the network interface n from the tile.
 */
void Tile::removeNetworkInterface(NetworkInterface *n) {
    // Iterate over the network interfaces
    for (auto i = networkInterfaces.begin(); i != networkInterfaces.end();
         i++) {
        if ((*i)->getName() == n->getName()) {
            networkInterfaces.erase(i);
            break;
        }
    }
}

} // End namespace FSMSADF
