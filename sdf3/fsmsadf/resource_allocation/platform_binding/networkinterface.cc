/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile network interface.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: networkinterface.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "networkinterface.h"
#include "graph.h"
#include <algorithm>

namespace FSMSADF {

/**
 * NetworkInterfaceBinding()
 * Constructor.
 */
NetworkInterfaceBinding::NetworkInterfaceBinding(const GraphComponent& c, NetworkInterface *n) :
    GraphComponent(c),
    networkInterface(n),
    nrInConnectionsUsedForOtherGraphs(0),
    nrOutConnectionsUsedForOtherGraphs(0),
    inBandwidthUsedForOtherGraphs(0),
    outBandwidthUsedForOtherGraphs(0) {}

/**
 * ~NetworkInterfaceBinding()
 * Destructor.
 */
NetworkInterfaceBinding::~NetworkInterfaceBinding() = default;

/**
 * clone()
 * Create a cloned copy of this network interface binding.
 */
std::unique_ptr<NetworkInterfaceBinding> NetworkInterfaceBinding::clone(const GraphComponent& c) {
    auto nb = std::make_unique<NetworkInterfaceBinding>(c, getNetworkInterface());

    // Channels
    nb->setInChannelBindings(getInChannelBindings());
    nb->setOutChannelBindings(getOutChannelBindings());

    // Initial resource occupancy
    nb->setNrInConnectionsUsedForOtherGraphs(getNrInConnectionsUsedForOtherGraphs());
    nb->setNrOutConnectionsUsedForOtherGraphs(getNrOutConnectionsUsedForOtherGraphs());
    nb->setInBandwidthUsedForOtherGraphs(getInBandwidthUsedForOtherGraphs());
    nb->setOutBandwidthUsedForOtherGraphs(getOutBandwidthUsedForOtherGraphs());

    return nb;
}

/**
 * constructFromXML()
 * This function creates the network interface binding as specified by the
 * connectionNode.
 */
void NetworkInterfaceBinding::constructFromXML(const Scenario& s, const CNodePtr networkInterfaceNode) {
    // Application graph
    ScenarioGraph *sg = s.getScenarioGraph();

    // Iterate over the list of channels bound to this network interface
    for (CNode *n = CGetChildNode(networkInterfaceNode, "channel"); n != nullptr;
         n = CNextNode(n, "channel")) {
        // Name specified for the channel?
        if (!CHasAttribute(n, "name")) {
            throw CException("Mapped channel has no name identifier.");
        }

        // Find channel in scenario graph
        Channel* c = sg->getChannel(CGetAttribute(n, "name"));

        // Is this channel bound to a network interface input?
        if (CHasAttribute(n, "in") && CGetAttribute(n, "in") == "true") {
            // Create binding of channel to network interface
            addInChannelBinding(s, c);
        }

        // Is this channel bound to a network interface output?
        if (CHasAttribute(n, "out") && CGetAttribute(n, "out") == "true") {
            // Create binding of channel to network interface
            addOutChannelBinding(s, c);
        }
    }
}

/**
 * constructFromXML()
 * This function sets the resource usage of the network interface.
 */
void NetworkInterfaceBinding::constructResourceUsageFromXML(const CNodePtr networkInterfaceNode) {
    // Input connections
    if (!CHasAttribute(networkInterfaceNode, "nrInConnections")) {
        throw CException("Network interface usage has no nrInConnections.");
    }
    setNrInConnectionsUsedForOtherGraphs(
            static_cast<Size>(CGetAttribute(networkInterfaceNode, "nrInConnections")));

    // Input bandwidth
    if (!CHasAttribute(networkInterfaceNode, "inBandwidth")) {
        throw CException("Network interface usage has no inBandwidth.");
    }
    setInBandwidthUsedForOtherGraphs(
            static_cast<Bandwidth>(CGetAttribute(networkInterfaceNode, "inBandwidth")));

    // Output connections
    if (!CHasAttribute(networkInterfaceNode, "nrOutConnections")) {
        throw CException("Network interface usage has no nrOutConnections.");
    }
    setNrOutConnectionsUsedForOtherGraphs(
            static_cast<Size>(CGetAttribute(networkInterfaceNode, "nrOutConnections")));

    // Output bandwidth
    if (!CHasAttribute(networkInterfaceNode, "outBandwidth")) {
        throw CException("Network interface usage has no outBandwidth.");
    }
    setOutBandwidthUsedForOtherGraphs(
            static_cast<Bandwidth>(CGetAttribute(networkInterfaceNode, "outBandwidth")));
}

/**
 * convertToXML()
 * This function converts the network interface binding to an XML object.
 */
void NetworkInterfaceBinding::convertToXML(const Scenario& s, const CNodePtr networkInterfaceNode) {
    // Name
    CAddAttribute(networkInterfaceNode, "name", getName());

    // Input channel binding exists for this scenario?
    auto scInChannelIter = inChannelBindings.find(&s);
    if (scInChannelIter != inChannelBindings.end()) {
        // List of all channels bound to this network interface
        for (auto *c : scInChannelIter->second) {
            CNode *n = CAddNode(networkInterfaceNode, "channel");
            CAddAttribute(n, "name", c->getName());
            CAddAttribute(n, "in", "true");
        }
    }

    // Output channel binding exists for this scenario?
    auto scOutChannelIter = outChannelBindings.find(&s);
    if (scOutChannelIter != outChannelBindings.end()) {
        // List of all channels bound to this network interface
        for (auto *c : scOutChannelIter->second) {
            CNode *n = CAddNode(networkInterfaceNode, "channel");
            CAddAttribute(n, "name", c->getName());
            CAddAttribute(n, "out", "true");
        }
    }
}

/**
 * convertToXML()
 * This function converts the resource usage of this network interface binding
 * to an XML object.
 */
void NetworkInterfaceBinding::convertResourceUsageToXML(const CNodePtr networkInterfaceNode) {
    // Name
    CAddAttribute(networkInterfaceNode, "name", getName());

    // Input connections
    CAddAttribute(
            networkInterfaceNode,
            "nrInConnections",
            CString(getNrInConnectionsUsedForGraph() + getNrInConnectionsUsedForOtherGraphs()));

    // Input bandwidth
    CAddAttribute(networkInterfaceNode,
                  "inBandwidth",
                  CString(getInBandwidthUsedForGraph() + getInBandwidthUsedForOtherGraphs()));

    // Output connections
    CAddAttribute(
            networkInterfaceNode,
            "nrOutConnections",
            CString(getNrOutConnectionsUsedForGraph() + getNrOutConnectionsUsedForOtherGraphs()));

    // Output bandwidth
    CAddAttribute(networkInterfaceNode,
                  "outBandwidth",
                  CString(getOutBandwidthUsedForGraph() + getOutBandwidthUsedForOtherGraphs()));
}

/**
 * getGraphBindingConstraints()
 * Get the graph binding constraints associated with this binding.
 */
GraphBindingConstraints *NetworkInterfaceBinding::getGraphBindingConstraints() const {
    return getTileBinding()->getPlatformBinding()->getGraphBindingConstraints();
}

/**
 * addInChannelBinding()
 * The function binds channel c in scenario s to this interface. The function
 * returns true on success. Otherwise it returns false.
 */
bool NetworkInterfaceBinding::addInChannelBinding(const Scenario& s, Channel *c) {
    // Binding already exists?
    if (hasInChannelBinding(s, c)) {
        return true;
    }

    // Sufficient resources available?
    if (hasResourcesInChannelBinding(s, c)) {
        inChannelBindings[&s].push_back(c);
        return true;
    }

    return false;
}

/**
 * removeInChannelBinding()
 * The function removes the binding of channel c in scenario s to this
 * network interface.
 */
void NetworkInterfaceBinding::removeInChannelBinding(const Scenario& s, const Channel *c) {
    for (auto i = inChannelBindings[&s].begin(); i != inChannelBindings[&s].end(); i++) {
        if (*i == c) {
            inChannelBindings[&s].erase(i);
            break;
        }
    }
}

/**
 * hasInChannelBinding()
 * The function returns true when a binding of channel c in scenario s to this
 * network interface exists. Otherwise it returns false.
 */
bool NetworkInterfaceBinding::hasInChannelBinding(const Scenario& s, const Channel *c) const {
    auto scIter = inChannelBindings.find(&s);

    if (scIter == inChannelBindings.end()) {
        return false;
    }

    return std::any_of(scIter->second.begin(), scIter->second.end(), [c](const Channel *cc) {
        return cc == c;
    });
}

/**
 * getAvailableNrInConnections()
 * The function returns the number of available input connections on this
 * network interface in scenario s.
 */
Size NetworkInterfaceBinding::getAvailableNrInConnections(const Scenario& s) const {
    return (networkInterface->getMaxNrInConnections() - nrInConnectionsUsedForOtherGraphs
            - getAllocatedNrInConnections(s));
}

/**
 * getAllocatedNrInConnections()
 * The function returns the number of input connections allocated to the
 * application graph on this network interface in scenario s.
 */
Size NetworkInterfaceBinding::getAllocatedNrInConnections(const Scenario& s) const {
    auto scIter = inChannelBindings.find(&s);

    if (scIter != inChannelBindings.end()) {
        return static_cast<Size>(scIter->second.size());
    }

    return 0;
}

/**
 * getAvailableBandwidthInConnections()
 * The function returns the amount of input bandwidth which is available on
 * this network interface in scenario s.
 */
Bandwidth NetworkInterfaceBinding::getAvailableBandwidthInConnections(const Scenario& s) const {
    return (networkInterface->getInBandwidth() - inBandwidthUsedForOtherGraphs
            - getAllocatedBandwidthInConnections(s));
}

/**
 * getAllocatedBandwidthInConnections()
 * The function returns the amount of input bandwidth which is allocated to the
 * application graph on this network interface in scenario s.
 */
Bandwidth NetworkInterfaceBinding::getAllocatedBandwidthInConnections(const Scenario& s) const {
    ChannelBindingConstraint *cb = nullptr;
    auto scIter = inChannelBindings.find(&s);
    Bandwidth b = 0;

    if (scIter == inChannelBindings.end()) {
        return 0;
    }

    for (auto *c : scIter->second) {
        cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)->getConstraintsOfChannel(
                c->getName());
        b += cb->getBandwidth();
    }

    return b;
}

/**
 * hasResourcesInChannelBinding()
 * The function checks whether there are sufficient resources available
 * to bind channel c to this network interface.
 */
bool NetworkInterfaceBinding::hasResourcesInChannelBinding(const Scenario& s, const Channel *c) const {
    ChannelBindingConstraint *cb = nullptr;
    cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)->getConstraintsOfChannel(
            c->getName());

    return getAvailableBandwidthInConnections(s) > cb->getBandwidth()
           && getAvailableNrInConnections(s) > 1;
}

/**
 * addOutChannelBinding()
 * The function binds channel c in scenario s to this interface. The function
 * returns true on success. Otherwise it returns false.
 */
bool NetworkInterfaceBinding::addOutChannelBinding(const Scenario& s, Channel *c) {
    // Binding already exists?
    if (hasOutChannelBinding(s, c)) {
        return true;
    }

    // Sufficient resources available?
    if (hasResourcesOutChannelBinding(s, c)) {
        outChannelBindings[&s].push_back(c);
        return true;
    }

    return false;
}

/**
 * removeOutChannelBinding()
 * The function removes the binding of channel c in scenario s to this
 * network interface.
 */
void NetworkInterfaceBinding::removeOutChannelBinding(const Scenario& s, const Channel *c) {
    for (auto i = outChannelBindings[&s].begin(); i != outChannelBindings[&s].end(); i++) {
        if (*i == c) {
            outChannelBindings[&s].erase(i);
            break;
        }
    }
}

/**
 * hasOutChannelBinding()
 * The function returns true when a binding of channel c in scenario s to this
 * network interface exists. Otherwise it returns false.
 */
bool NetworkInterfaceBinding::hasOutChannelBinding(const Scenario& s, const Channel *c) const {
    auto scIter = outChannelBindings.find(&s);

    if (scIter == outChannelBindings.end()) {
        return false;
    }

    return std::any_of(scIter->second.begin(), scIter->second.end(), [c](const Channel *cc) {
        return cc == c;
    });
}

/**
 * getAvailableNrOutConnections()
 * The function returns the number of available output connections on this
 * network interface in scenario s.
 */
Size NetworkInterfaceBinding::getAvailableNrOutConnections(const Scenario& s) const {
    return (networkInterface->getMaxNrOutConnections() - nrOutConnectionsUsedForOtherGraphs
            - getAllocatedNrOutConnections(s));
}

/**
 * getAllocatedNrOutConnections()
 * The function returns the number of output connections allocated to the
 * application graph on this network interface in scenario s.
 */
Size NetworkInterfaceBinding::getAllocatedNrOutConnections(const Scenario& s) const {
    auto scIter = outChannelBindings.find(&s);

    if (scIter != outChannelBindings.end()) {
        return static_cast<Size>(scIter->second.size());
    }

    return 0;
}

/**
 * getAvailableBandwidthOutConnections()
 * The function returns the amount of output bandwidth which is available on
 * this network interface in scenario s.
 */
Bandwidth NetworkInterfaceBinding::getAvailableBandwidthOutConnections(const Scenario& s) const {
    return (networkInterface->getOutBandwidth() - outBandwidthUsedForOtherGraphs
            - getAllocatedBandwidthOutConnections(s));
}

/**
 * getAllocatedBandwidthOutConnections()
 * The function returns the amount of output bandwidth which is allocated to the
 * application graph on this network interface in scenario s.
 */
Bandwidth NetworkInterfaceBinding::getAllocatedBandwidthOutConnections(const Scenario& s) const {
    ChannelBindingConstraint *cb = nullptr;
    auto scIter = outChannelBindings.find(&s);
    Bandwidth b = 0;

    if (scIter == outChannelBindings.end()) {
        return 0;
}

    for (auto *c: scIter->second) {
        cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)->getConstraintsOfChannel(
                c->getName());
        b += cb->getBandwidth();
    }

    return b;
}

/**
 * hasResourcesOutChannelBinding()
 * The function checks whether there are sufficient resources available
 * to bind channel c to this network interface.
 */
bool NetworkInterfaceBinding::hasResourcesOutChannelBinding(const Scenario& s, const Channel *c) const {
    ChannelBindingConstraint *cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)->getConstraintsOfChannel(
            c->getName());

    return getAvailableBandwidthOutConnections(s) > cb->getBandwidth()
        && getAvailableNrOutConnections(s) > 1;
}

/**
 * getNrInConnectionsUsedForGraph()
 * The function returns the maximal amount of connections which has been
 * allocated to the application in any of its scenarios.
 */
Size NetworkInterfaceBinding::getNrInConnectionsUsedForGraph() const {
    Size n = 0;
    Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

    for (const auto& s: g->getScenarios()) {
        n = MAX(n, getAllocatedNrInConnections(*s));
    }

    return n;
}

/**
 * getInBandwidthUsedForGraph()
 * The function returns the maximal amount of bandwidth which has been allocated
 * to the application in any of its scenarios.
 */
Bandwidth NetworkInterfaceBinding::getInBandwidthUsedForGraph() const {
    Bandwidth n = 0;
    Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

    for (const auto& s: g->getScenarios()) {
        n = MAX(n, getAllocatedBandwidthInConnections(*s));
    }

    return n;
}

/**
 * getNrOutConnectionsUsedForGraph()
 * The function returns the maximal amount of connections which has been
 * allocated to the application in any of its scenarios.
 */
Size NetworkInterfaceBinding::getNrOutConnectionsUsedForGraph() const {
    Size n = 0;
    Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

    for (const auto& s: g->getScenarios()) {
        n = MAX(n, getAllocatedNrOutConnections(*s));
    }

    return n;
}

/**
 * getOutBandwidthUsedForGraph()
 * The function returns the maximal amount of bandwidth which has been allocated
 * to the application in any of its scenarios.
 */
Bandwidth NetworkInterfaceBinding::getOutBandwidthUsedForGraph() const {
    Bandwidth n = 0;
    Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

    for (const auto& s: g->getScenarios()) {
        n = MAX(n, getAllocatedBandwidthOutConnections(*s));
    }

    return n;
}

void NetworkInterfaceBinding::setInChannelBindings(std::map<const Scenario *, ChannelRefs> &ch) {
    inChannelBindings = ch;
};

// Tile binding
TileBinding *NetworkInterfaceBinding::getTileBinding() const {
    return dynamic_cast<TileBinding *>(getParent());
};


} // namespace FSMSADF
