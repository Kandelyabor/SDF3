/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Platform graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: graph.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "graph.h"

namespace FSMSADF {

/**
 * PlatformGraph ()
 * Constructor.
 */
PlatformGraph::PlatformGraph(const GraphComponent &c) : GraphComponent(c) {}

/**
 * PlatformGraph ()
 * Constructor.
 */
PlatformGraph::PlatformGraph() : GraphComponent(nullptr, 0) {}

/**
 * ~PlatformGraph ()
 * Destructor.
 */
PlatformGraph::~PlatformGraph() = default;

/**
 * create()
 * The function creates a new platform graph object.
 */
std::unique_ptr<PlatformGraph> PlatformGraph::create(const GraphComponent &c) { return std::make_unique<PlatformGraph>(c); }

/**
 * createCopy()
 * The function creates a copy of the platform graph. Note that copying does
 * not relocate connections. The clone function should be used for this purpose.
 */
std::unique_ptr<PlatformGraph> PlatformGraph::createCopy(const GraphComponent &c) {
    std::unique_ptr<PlatformGraph> pg = create(c);

    // Name
    pg->setName(getName());

    // Tiles
    for (const auto& t : tiles) {
        pg->addTile(t->createCopy(GraphComponent(pg.get(), getId(), getName())));
    }

    // Connections
    for (const auto& cc : connections) {
        pg->addConnection(cc->createCopy(GraphComponent(pg.get(), getId(), getName())));
    }

    return pg;
}

/**
 * clone()
 * The function creates a clone of this platform graph.
 */
std::unique_ptr<PlatformGraph> PlatformGraph::clone(const GraphComponent &c) {
    std::unique_ptr<PlatformGraph> pg = create(c);

    // Name
    pg->setName(getName());

    // Tiles
    for (const auto& t : tiles) {
        pg->addTile(t->clone(GraphComponent(pg.get(), getId(), getName())));
    }

    // Connections
    for (const auto& cc : connections) {
        pg->addConnection(cc->clone(GraphComponent(pg.get(), getId(), getName())));
    }

    return pg;
}

void PlatformGraph::constructFromXML(const CNodePtr platformGraphNode) {
    // Name
    if (!CHasAttribute(platformGraphNode, "name")) {
        throw CException("Invalid graph, missing platform graph name.");
    }
    setName(CGetAttribute(platformGraphNode, "name"));

    // Iterate over all child nodes of the tile
    for (CNode *n = CGetChildNode(platformGraphNode); n != nullptr; n = CNextNode(n)) {
        // Tile
        if (CIsNode(n, "tile")) {
            Tile *t = createTile("(no name)");
            t->constructFromXML(n);
        }

        // Connection
        if (CIsNode(n, "connection")) {
            Connection *c = createConnection("(no name)");
            c->constructFromXML(n);
        }
    }
}

/**
 * convertToXML()
 * The function adds all elements of this graph to the supplied node.
 */
void PlatformGraph::convertToXML(const CNodePtr platformGraphNode) {
    // Name
    CAddAttribute(platformGraphNode, "name", getName());

    // Tiles
    for (const auto& t : tiles) {
        t->convertToXML(CAddNode(platformGraphNode, "tile"));
    }

    // Connections
    for (const auto& cc : connections) {
        cc->convertToXML(CAddNode(platformGraphNode, "connection"));
    }
}

/**
 * getTile()
 * The function returns a pointer to a tile with the supplied id. When
 * no such tile exists, an exception is thrown.
 */
Tile *PlatformGraph::getTile(const CId id) {
    for (const auto& t : tiles) {
        if (t->getId() == id) {
            return t.get();
        }
    }

    throw CException("Platform graph does not contain a tile with name '" + CString(id) + "'.");
}

/**
 * getTile()
 * The function returns a pointer to a tile with the supplied name. When
 * no such tile exists, an exception is thrown.
 */
Tile *PlatformGraph::getTile(const CString &name) {
    for (const auto& t : tiles) {
        if (t->getName() == name) {
            return t.get();
        }
    }

    throw CException("Platform graph does not contain a tile with name '" + name + "'.");
}

/**
 * createTile()
 * The function creates a new tile and adds it to the graph.
 */
Tile *PlatformGraph::createTile(const CString &name) {
    auto t = std::make_unique<Tile>(GraphComponent(this, static_cast<CId>(tiles.size()), name));
    auto* tr = t.get();
    addTile(std::move(t));
    return tr;
}

/**
 * addTile()
 * The function adds tile t to the graph.
 */
void PlatformGraph::addTile(std::unique_ptr<Tile> t) { tiles.push_back(std::move(t)); }

/**
 * removeTile()
 * The function removes tile t from the graph.
 */
void PlatformGraph::removeTile(Tile *t) {
    for (auto i = tiles.begin(); i != tiles.end(); i++) {
        if ((*i)->getName() == t->getName()) {
            tiles.erase(i);
            break;
        }
    }
}

/**
 * getConnection()
 * The function returns a pointer to a connection with the supplied id. When
 * no such connection exists, an exception is thrown.
 */
Connection *PlatformGraph::getConnection(const CId id) {
    for (const auto& cc : connections) {
        if (cc->getId() == id) {
            return cc.get();
        }
    }

    throw CException("Platform graph does not contain a connection with id '" + CString(id) + "'.");
}

/**
 * getConnection()
 * The function returns a pointer to a connection with the supplied name. When
 * no such connection exists, an exception is thrown.
 */
Connection *PlatformGraph::getConnection(const CString &name) {
    for (const auto& c : connections) {
        if (c->getName() == name) {
            return c.get();
        }
    }

    throw CException("Platform graph does not contain a connection with name '" + name + "'.");
}

/**
 * createConnection()
 * The function creates a new connection and adds it to the graph.
 */
Connection *PlatformGraph::createConnection(const CString &name) {
    auto c = std::make_unique<Connection>(GraphComponent(this, static_cast<CId>(connections.size()), name));
    auto* cr = c.get();
    addConnection(std::move(c));

    return cr;
}

/**
 * addConnection()
 * The function adds the connection c to the graph.
 */
void PlatformGraph::addConnection(std::unique_ptr<Connection> c) { connections.push_back(std::move(c)); }

/**
 * removeConnection()
 * The function removes connection c from the graph.
 */
void PlatformGraph::removeConnection(Connection *c) {
    for (auto i = connections.begin(); i != connections.end(); i++) {
        if ((*i)->getName() == c->getName()) {
            connections.erase(i);
            break;
        }
    }
}

} // End namespace FSMSADF
