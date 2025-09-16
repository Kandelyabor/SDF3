/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpdependencies.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 13, 2008
 *
 *  Function        :   Max-Plus based buffer sizing. Analyse and manage storage
 *                      critical dependencies.
 *
 *  History         :
 *      13-05-08    :   Initial version.
 *
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
#include "mpdependencies.h"
#include "mpexplore.h"
#include "mpstorage.h"
#include <memory>

namespace SDF::MaxPlusAnalysis {

using namespace GraphDecoration;
using namespace MaxPlus;
namespace DependencyGraphs {

namespace {


// NOLINTBEGIN(*-global-variables)
std::unique_ptr<ChannelRef[]> criticalChannels;
unsigned int numberOfCriticalChannels = 0;
// NOLINTEND(*-global-variables)

} // namespace

/**
 * Dependency()
 */
Dependency::Dependency(Channel *from, Channel *to, Node *toNd) :
    fromChannel(from), toChannel(to), toNode(toNd) {}

/**
 * Node()
 */
Node::Node(Channel *ch, const unsigned int numberOfChannels) :
    channel(ch),
    dependencies(std::make_unique<std::unique_ptr<Dependency>[]>(numberOfChannels)),
    numberOfDependencies(0),
    dependenciesVisited(-1),
    partOfCycle(false) {}

/**
 * addDependency()
 */
void Node::addDependency(Channel *from, Channel *to, Node *toNode) {
    // check if it exists
    for (unsigned int i = 0; i < this->numberOfDependencies; i++) {
        if (this->dependencies[i]->toChannel == to) {
            return;
        }
    }

    this->dependencies[this->numberOfDependencies++] = std::make_unique<Dependency>(from, to, toNode);
}

/**
 * Graph()
 */
DependencyGraphs::Graph::Graph(const GraphDecoration::Graph *SG) : G(SG) {

    criticalChannels = std::make_unique<ChannelRef[]>(SG->channels.size());
    auto noc = static_cast<unsigned int>(G->channels.size());
    this->nodes = std::make_unique<std::unique_ptr<Node>[]>(noc);
    ChannelList::const_iterator i;
    for (i = G->channels.begin(); i != G->channels.end(); i++) {
        Channel* c = i->get();
        this->nodes[c->index] = std::make_unique<Node>(c, noc);
    }
}

/**
 * addDependency()
 */
void DependencyGraphs::Graph::addDependency(Channel *from, Channel *to) const {
    this->nodes[from->index]->addDependency(from, to, this->nodes[to->index].get());
}

/**
 * fire()
 */
void DependencyGraphs::Graph::fire(State *x, Actor *a) const {
    // this is too slow? Make a quick note of criticality on the channel
    // and build the graph later?
    // Maybe use a two-dimensional static array of #channels x #channels
    // with booleans instead of the dynamic data structures?

    numberOfCriticalChannels = 0;
    MPTime time;
    auto ftime = MPTime(0);
    for (auto i = a->inputPorts.begin(); i != a->inputPorts.end(); i++) {
        const Port* p = i->get();
        time = x->consume(*p);
        if (i != a->inputPorts.begin()) {
            if (time > ftime) {
                numberOfCriticalChannels = 1;
                ftime = time;
                criticalChannels[0] = p->channel;
            } else {
                if (time == ftime) {
                    criticalChannels[numberOfCriticalChannels++] = p->channel;
                }
            }
        } else {
            ftime = time;
            criticalChannels[numberOfCriticalChannels++] = p->channel;
        }
    }
    time = ftime + a->executionTime;
    for (const auto& p : a->outputPorts) {
        x->produce(*p, time);
        for (unsigned int k = 0; k < numberOfCriticalChannels; k++) {
            this->addDependency(criticalChannels[k], p->channel);
        }
    }
}

/**
 * markCycle()
 */
void Node::markCycle() {
    Node *n = this;
    do {
        n->partOfCycle = true;
        n = n->dependencies[n->dependenciesVisited]->toNode;
    } while (n != this);
}

/**
 * findCycle()
 */
void Node::findCycle() {
    if (this->dependenciesVisited >= 0) {
        if ((static_cast<unsigned int>(this->dependenciesVisited)) < this->numberOfDependencies) {
            this->markCycle();
        }
        return;
    }

    for (this->dependenciesVisited = 0;
         this->dependenciesVisited < (static_cast<int>(this->numberOfDependencies));
         this->dependenciesVisited++) {
        this->dependencies[dependenciesVisited]->toNode->findCycle();
    }
}

/**
 * findCriticalChannels()
 */
std::unique_ptr<ChannelRefList> DependencyGraphs::Graph::findCriticalChannels() const {
    for (unsigned int i = 0; i < this->G->channels.size(); i++) {
        Node *n = this->nodes[i].get();
        if (n->channel->isStorageChannel) {
            n->findCycle();
        }
    }
    auto cl = std::make_unique<ChannelRefList>();
    for (unsigned int i = 0; i < this->G->channels.size(); i++) {
        Node *n = this->nodes[i].get();
        if (n->channel->isStorageChannel && n->partOfCycle) {
            cl->push_back(n->channel);
        }
    }

    return cl;
}
} // namespace DependencyGraphs
} // namespace SDF::MaxPlusAnalysis
