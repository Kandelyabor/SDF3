/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   apg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 2, 2005
 *
 *  Function        :   Acyclic precedence graph
 *
 *  History         :
 *      02-08-05    :   Initial version.
 *
 * $Id: apg.cc,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#include "apg.h"
namespace SDF {

/**
 * newEdge ()
 * Connect a new edge to the node
 */
void APGnode::newEdge(APGedge *e) {
    // Is node source or destination of edge?
    if (e->getSrc()->getId() == getId()) {
        // Out edge
        outEdges.push_back(e);
        edges.push_back(e);
    } else {
        // In edge
        inEdges.push_back(e);
        edges.push_back(e);
    }
}

/**
 * delEdge ()
 * Disconnect an edge from the node
 */
void APGnode::delEdge(APGedge *e) {
    // Edges
    for (auto iter = edges.begin(); iter != edges.end(); iter++) {
        if ((*iter)->getId() == e->getId()) {
            edges.erase(iter);
            break;
        }
    }

    // In edges
    for (auto iter = inEdges.begin(); iter != inEdges.end(); iter++) {
        if ((*iter)->getId() == e->getId()) {
            inEdges.erase(iter);
            break;
        }
    }

    // Out edges
    for (auto iter = outEdges.begin(); iter != outEdges.end(); iter++) {
        if ((*iter)->getId() == e->getId()) {
            outEdges.erase(iter);
            break;
        }
    }
}

/**
 * delAllEdges ()
 * Disconnect all edges from the node
 */
void APGnode::delAllEdges() {
    edges.clear();
    inEdges.clear();
    outEdges.clear();
}

/**
 * opposite ()
 * Returns the node on the opposite side of e
 */
APGnode *APGnode::opposite(APGedge *e) {
    // Find edge
    for (auto iter = edges.begin(); iter != edges.end(); iter++) {
        if ((*iter)->getId() == e->getId()) {
            return (*iter)->opposite(this);
        }
    }

    // No edge found.
    throw CException("No edge found.");
}

/**
 * APGgraph ()
 * Constructor
 */
APGgraph::APGgraph(const CId id) : id(id) {}

/**
 * ~APGgraph ()
 * Destructor
 */
APGgraph::~APGgraph() = default;

/**
 * newNode ()
 * Add a new node to the graph
 */
APGnode *APGgraph::newNode(const CId id) {
    auto n = std::make_unique<APGnode>(id);

    auto *nr = n.get();
    nodes.push_back(std::move(n));

    return nr;
}

/**
 * delNode ()
 * Remove a node n, and thus all edges incident with n from the graph
 */
void APGgraph::delNode(APGnode *n) {
    // Remove all edges incident with n
    for (auto *iter : n->getEdges()) {
        n->delEdge(iter);
        delEdge(iter);
    }

    // Find node n
    for (auto iter = nodesBegin(); iter != nodesEnd(); iter++) {
        if ((*iter)->getId() == n->getId()) {
            nodes.erase(iter);
            return;
        }
    }
}

/**
 * delAllNodes ()
 * Remove all nodes from the graph
 */
void APGgraph::delAllNodes() { nodes.clear(); }

/**
 * getNode ()
 * Returns a pointer to the node with the given id.
 */
APGnode *APGgraph::getNode(CId id) {
    for (const auto &n : nodes) {
        if (n->getId() == id) {
            return n.get();
        }
    }

    throw CException("Node not found.");
}

/**
 * newEdge ()
 * Add a new edge to the graph.
 */
APGedge *APGgraph::newEdge(const CId id, APGnode *src, APGnode *dst) {
    auto e = std::make_unique<APGedge>(id, src, dst);

    auto *er = e.get();
    src->newEdge(er);
    dst->newEdge(er);

    edges.push_back(std::move(e));

    return er;
}

/**
 * delEdge ()
 * Deletes edge e
 */
void APGgraph::delEdge(APGedge *e) {
    e->getSrc()->delEdge(e);
    e->getDst()->delEdge(e);

    for (auto iter = edges.begin(); iter != edges.end(); iter++) {
        if ((*iter)->getId() == e->getId()) {
            edges.erase(iter);
            return;
        }
    }
}

/**
 * delAllEdges ()
 * Remove all edges from the graph
 */
void APGgraph::delAllEdges() { edges.clear(); }

/**
 * getEdge ()
 * Returns a pointer to the edge with the given id.
 */
APGedge *APGgraph::getEdge(CId id) {
    for (const auto& e : edges) {
        if (e->getId() == id) {
            return e.get();
        }
    }

    throw CException("Edge not found.");
}
} // namespace SDF
