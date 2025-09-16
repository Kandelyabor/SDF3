/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   precedence_graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 2, 2009
 *
 *  Function        :   Precedence graph
 *
 *  History         :
 *      02-06-09    :   Initial version.
 *
 * $Id: precedence_graph.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "precedence_graph.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * Node()
 * Constructor
 */
PrecedenceGraph::Node::Node(Actor *a, CId n, Time t) :
    actor(a), idx(n), actorExecutionTime(t), deadline(-1), seen(0) {}

/**
 * updateDeadline()
 * Update the deadline time of this node to t in case its current deadline
 * is less than t.
 */
void PrecedenceGraph::Node::updateDeadline(const Time t) {
    // Time t is smaller than current remaining time
    if (t + actorExecutionTime <= deadline) {
        return;
    }

    // Update the deadline
    deadline = t + actorExecutionTime;

    // Update the deadline of all nodes which are a predecessor of this node
    for (auto i = previous.begin(); i != previous.end(); i++) {
        Node *n = *i;
        n->updateDeadline(deadline);
    }
}

/**
 * isSchedulable()
 * A node can be scheduled when it has no previous constraints left.
 */
bool PrecedenceGraph::Node::isSchedulable() const { return previous.empty(); }

/**
 * PrecedenceGraph()
 * Constructor.
 */
PrecedenceGraph::PrecedenceGraph(ScenarioRef s) {
    ScenarioGraph *sg = nullptr;
    RepetitionVector v;

    // Get scenario graph sg which is associated with scenario s
    sg = s->getScenarioGraph();

    // Compute repetition vector of the scenario graph
    v = s->computeRepetitionVector();

    // Create nodes for each actor in the scenario graph
    for (auto i = sg->getActors().begin(); i != sg->getActors().end(); i++) {
        const std::unique_ptr<Actor>& a = *i;

        // Create as many nodes as repetition vector entries
        for (int j = 0; j < v[a->getId()]; j++) {
            auto n =
                    std::make_unique<Node>(a.get(), j, a->getExecutionTimeOfScenario(*s, a->getDefaultProcessorType()));

            // Add node n to the set of nodes
            nodes[n.get()] = std::move(n);
        }
    }

    // Create precedence constraints between the nodes
    for (auto it = sg->getChannels().begin(); it != sg->getChannels().end(); it++) {
        ChannelRef c = it->get();

        uint nA = c->getSrcPort()->getRateOfScenario(*s);
        uint nB = c->getDstPort()->getRateOfScenario(*s);
        uint qA = v[c->getSrcActor()->getId()];
        uint qB = v[c->getDstActor()->getId()];
        uint d = c->getInitialTokens();

        // Iterate over all source nodes related to the source actor of
        // the channel
        for (uint i = 1; i <= qA; i++) {
            // Source node
            Node *srcN = getNode(c->getSrcActor(), i - 1);

            // Create an edge for each
            for (uint k = 1; k <= nA; k++) {
                uint j = 1
                         + static_cast<uint>(
                                 floor(static_cast<double>((d + (i - 1) * nA + k - 1) % (nB * qB))
                                       / static_cast<double>(nB)));
                uint t = static_cast<uint>(floor(static_cast<double>(d + ((i - 1) * nA) + k - 1)
                                                 / static_cast<double>(nB * qB)));

                // Destination node
                Node *dstN = getNode(c->getDstActor(), j - 1);

                // No initial tokens on this edge?
                if (t == 0) {
                    dstN->previous.insert(srcN);
                    srcN->next.insert(dstN);
                }
            }
        }
    }

    // Validate that the graph is acyclic.
    // this avoid updateDeadline going into a deadlock.
    if (cycle_detect()) {
        throw CException("Graph is not acyclic");
    }

    // Compute deadlines of the nodes (starting from nodes on which no
    // precedence constraints exist)
    for (auto i = nodes.begin(); i != nodes.end(); i++) {
        Node *n = (*i).first;

        if (n->next.empty()) {
            n->updateDeadline(0);
        }
    }
}

namespace {


/**
 * updateThe node (recursive) using DFS.
 *
 * @ returns true is graph is cyclic
 */

bool visit(PrecedenceGraph::Node *n) {
    n->seen = 1;
    for (auto *c : n->previous) {
        if (c->seen == 1) {
            /* found a cycle */
            return true;
        }
        if (c->seen == 0) {
            if (visit(c)) {
                return true;
            }
        }
    }
    n->seen = 2;
    return false;
}

} // namespace

/**
 * This function does a modified depth first search (colored DFS).
 * All nodes are initially marked white (0).
 * When a node is encountered, it is marked grey (1),
 * and when its descendants are completely visited, it is marked black (2).
 * If a grey node is ever encountered, then there is a cycle.
 *
 * @return true is a cycle is detected.
 */
bool PrecedenceGraph::cycle_detect() {
    // set all nodes white
    for (const auto& i : nodes) {
        i.first->seen = 0;
    }
    //
    return std::any_of(nodes.begin(), nodes.end(), [](const std::pair<Node * const,std::unique_ptr<Node>>& n) {
        if (n.first->seen == 0) {
            if (visit(n.first)) {
                return true;
            }
        }
        return false;
    });
}

/**
 * ~PrecedenceGraph()
 * Destructor.
 */
PrecedenceGraph::~PrecedenceGraph() = default;

/**
 * getEarliestDeadline()
 * The function returns from the node with the earliest deadline (i.e. largest
 * value). When the set of nodes is empty, the function returns NULL.
 */
PrecedenceGraph::Node *PrecedenceGraph::getEarliestDeadline() const {
    Node *n = nullptr;

    // Iterate over all nodes
    for (const auto& m : nodes) {
        // Is this node schedulable?
        if (m.first->isSchedulable()) {
            // The deadline of node m is larger than the deadline of node n?
            if (n == nullptr || m.first->deadline > n->deadline) {
                n = m.first;
            }
        }
    }

    return n;
}

/**
 * removeNode()
 * The function removes the node n from the precedence graph. A node can only
 * be removed if all its predecessors or successors have been removed.
 */
void PrecedenceGraph::removeNode(Node *n) {
    // Node has no predecessors
    if (n->previous.empty()) {
        // Remove the node from all successor nodes (if any)
        for (auto *m : n->next) {
            m->previous.erase(n);
        }

        // Remove node n from the set of nodes
        nodes.erase(n);

    } else if (n->next.empty()) {
        // Remove the node from all predecessor nodes (if any)
        for (auto *m : n->previous) {
            m->next.erase(n);
        }

        // Remove node n from the set of nodes
        nodes.erase(n);
    } else {
        throw CException("Cannot remove node which has predecessors "
                         "and successors.");
    }
}

/**
 * getNode()
 * The function returns a pointer to the node with the supplied actor and id.
 * When no such pointer exists, an exception is thrown.
 */
PrecedenceGraph::Node *PrecedenceGraph::getNode(const Actor *a, const CId idx) const {
    for (const auto& n : nodes) {
        if (n.first->actor == a && n.first->idx == idx) {
            return n.first;
        }
    }

    throw CException("Precedence graph contains no node with idx " + CString(idx));
}

} // End namespace FSMSADF
