/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   constraint.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Graph binding constraints.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *      19-11-15    :   Store channel name instead of pointer (Reinier)
 *
 * $Id: constraint.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "constraint.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * ChannelBindingConstraint()
 * Constructor.
 */
ChannelBindingConstraint::ChannelBindingConstraint(const CString &name) :
    channelName(name),
    bufferSizeConstraint(false),
    bufferLocation(BufferLocation::Value::Dst),
    bufferLocationConstraint(false),
    bandwidthConstraint(false),
    delayConstraint(false),
    bandwidth(0),
    delay(0) {}

/**
 * ~ChannelBindingConstraint()
 * Destructor.
 */
ChannelBindingConstraint::~ChannelBindingConstraint() = default;

/**
 * clone()
 * Create a cloned copy of this channel binding constraints object. The channel
 * relation and all associated scenarios are automatically updated to the
 * supplied channel.
 */
std::unique_ptr<ChannelBindingConstraint> ChannelBindingConstraint::clone(const CString &name) const {
    auto cb = std::make_unique<ChannelBindingConstraint>(name);

    // Buffer size
    if (hasBufferSizeConstraint()) {
        cb->setBufferSize(getBufferSize());
    }

    // Buffer size
    if (hasBufferLocationConstraint()) {
        cb->setBufferLocation(CString(getBufferLocation()));
    }

    // Delay
    if (hasDelayConstraint()) {
        cb->setDelay(getDelay());
    }

    // Bandwidth
    if (hasBandwidthConstraint()) {
        cb->setBandwidth(getBandwidth());
    }

    return cb;
}

/**
 * constructFromXML()
 * Construct channel binding constraints for scenario s from XML data.
 */
void ChannelBindingConstraint::constructFromXML(const CNodePtr channelConstraintsNode) {
    // Buffer size
    if (CHasChildNode(channelConstraintsNode, "bufferSize")) {
        CNode *bufferSizeNode = CGetChildNode(channelConstraintsNode, "bufferSize");
        BufferSize buf;

        buf[static_cast<int>(BufferLocation::Value::Mem)] = MAX_Size;
        buf[static_cast<int>(BufferLocation::Value::Src)] = MAX_Size;
        buf[static_cast<int>(BufferLocation::Value::Dst)] = MAX_Size;

        if (CHasAttribute(bufferSizeNode, BufferLocation::toString(BufferLocation::Value::Src))) {
            buf[static_cast<int>(BufferLocation::Value::Src)] = static_cast<Size>(CGetAttribute(
                    bufferSizeNode, BufferLocation::toString(BufferLocation::Value::Src)));
        }

        if (CHasAttribute(bufferSizeNode, BufferLocation::toString(BufferLocation::Value::Dst))) {
            buf[static_cast<int>(BufferLocation::Value::Dst)] = static_cast<Size>(CGetAttribute(
                    bufferSizeNode, BufferLocation::toString(BufferLocation::Value::Dst)));
        }

        if (CHasAttribute(bufferSizeNode, BufferLocation::toString(BufferLocation::Value::Mem))) {
            buf[static_cast<int>(BufferLocation::Value::Mem)] = static_cast<Size>(CGetAttribute(
                    bufferSizeNode, BufferLocation::toString(BufferLocation::Value::Mem)));
        }

        setBufferSize(buf);
    }

    // Buffer location
    if (CHasChildNode(channelConstraintsNode, "bufferLocation")) {
        CNode *bufferLocationNode = CGetChildNode(channelConstraintsNode, "bufferLocation");

        if (CHasAttribute(bufferLocationNode, "location")) {
            setBufferLocation(CGetAttribute(bufferLocationNode, "location"));
        }
    }

    // Bandwidth
    if (CHasChildNode(channelConstraintsNode, "bandwidth")) {
        CNode *bwNode = CGetChildNode(channelConstraintsNode, "bandwidth");

        if (CHasAttribute(bwNode, "min")) {
            setBandwidth(static_cast<CDouble>(CGetAttribute(bwNode, "min")));
        }
    }

    // Delay
    if (CHasChildNode(channelConstraintsNode, "delay")) {
        CNode *delayNode = CGetChildNode(channelConstraintsNode, "delay");

        if (CHasAttribute(delayNode, "min")) {
            setDelay(static_cast<Time>(CGetAttribute(delayNode, "min")));
        }
    }
}

/**
 * convertToXML()
 * Convert channel bindings constraints of scenario s to XML data.
 */
void ChannelBindingConstraint::convertToXML(const CNodePtr channelConstraintsNode) const {
    // Channel
    CAddAttribute(channelConstraintsNode, "name", getChannelName());

    // Buffer size
    if (hasBufferSizeConstraint()) {
        BufferSize buf = getBufferSize();
        CNode *bufferSizeNode = CAddNode(channelConstraintsNode, "bufferSize");

        /// @todo ugly looking, would be nice todo this nicer.
        CAddAttribute(bufferSizeNode,
                      BufferLocation::toString(BufferLocation::Value::Mem),
                      CString(buf[static_cast<int>(BufferLocation::Value::Mem)]));
        CAddAttribute(bufferSizeNode,
                      BufferLocation::toString(BufferLocation::Value::Src),
                      CString(buf[static_cast<int>(BufferLocation::Value::Src)]));
        CAddAttribute(bufferSizeNode,
                      BufferLocation::toString(BufferLocation::Value::Dst),
                      CString(buf[static_cast<int>(BufferLocation::Value::Dst)]));
    }

    // Buffer location
    if (hasBufferLocationConstraint()) {
        CNode *bufferLocationNode = CAddNode(channelConstraintsNode, "bufferLocation");
        CAddAttribute(bufferLocationNode, "location", getBufferLocation().asString());
    }

    // Bandwidth
    if (hasBandwidthConstraint()) {
        CNode *bwNode = CAddNode(channelConstraintsNode, "bandwidth");
        CAddAttribute(bwNode, "min", CString(getBandwidth()));
    }

    // Delay
    if (hasDelayConstraint()) {
        CNode *delayNode = CAddNode(channelConstraintsNode, "delay");
        CAddAttribute(delayNode, "min", CString(getDelay()));
    }
}

/**
 * getBufferSize ()
 * The function returns the buffer size of this channel. When no buffer size is
 * specified, the an exception is thrown.
 */
BufferSize ChannelBindingConstraint::getBufferSize() const {
    if (hasBufferSizeConstraint()) {
        return bufferSize;
    }

    throw CException("Channel '" + getChannelName() + "' has no buffer size.");
}

/**
 * setBufferSize ()
 * Set the buffer size of this channel.
 */
void ChannelBindingConstraint::setBufferSize(const BufferSize &sz) {
    bufferSize = sz;
    bufferSizeConstraint = true;
}

/**
 * hasBufferSizeConstraint ()
 * The function returns true when a buffer size has been specified. Otherwise
 * the function returns false.
 */
bool ChannelBindingConstraint::hasBufferSizeConstraint() const { return bufferSizeConstraint; }

/**
 * getBufferLocation ()
 * The function returns the buffer l of this channel. When no buffer Location is
 * specified, the an exception is thrown.
 */
BufferLocation ChannelBindingConstraint::getBufferLocation() const {
    if (hasBufferLocationConstraint()) {
        return bufferLocation;
    }

    throw CException("Channel '" + getChannelName() + "' has no buffer location.");
}

/**
 * setBufferLocation ()
 * Set the buffer location of this channel.
 */
void ChannelBindingConstraint::setBufferLocation(const CString &loc) {
    bufferLocation = loc;
    bufferLocationConstraint = true;
}

/**
 * hasBufferLocationConstraint ()
 * The function returns true when a buffer Location has been specified. Otherwise
 * the function returns false.
 */
bool ChannelBindingConstraint::hasBufferLocationConstraint() const {
    return bufferLocationConstraint;
}

/**
 * getDelay ()
 * The function returns the delay of this channel. When no delay has been
 * specified, an exception is thrown.
 */
Time ChannelBindingConstraint::getDelay() const {
    if (hasDelayConstraint()) {
        return delay;
    }

    throw CException("Channel '" + getChannelName() + "' has no delay.");
}

/**
 * setDelay ()
 * Set the delay of this channel.
 */
void ChannelBindingConstraint::setDelay(const Time t) {
    delay = t;
    delayConstraint = true;
}

/**
 * hasDelayConstraint ()
 * The function returns true when a delay has been specified. Otherwise the
 * function returns false.
 */
bool ChannelBindingConstraint::hasDelayConstraint() const { return delayConstraint; }

/**
 * getBandwidth ()
 * The function returns the bandwidth of this channel. When no bandwidth has
 * been specified, an exception is thrown.
 */
Bandwidth ChannelBindingConstraint::getBandwidth() const {
    if (hasBandwidthConstraint()) {
        return bandwidth;
    }

    throw CException("Channel '" + getChannelName() + "' has no bandwidth.");
}

/**
 * setBandwidth ()
 * Set the bandwidth of this channel.
 */
void ChannelBindingConstraint::setBandwidth(const Bandwidth b) {
    bandwidth = b;
    bandwidthConstraint = true;
}

/**
 * hasBandwidthConstraint ()
 * The function returns true when a bandwidth has been specified. Otherwise
 * the function returns false.
 */
bool ChannelBindingConstraint::hasBandwidthConstraint() const { return bandwidthConstraint; }

/**
 * ClusterBindingConstraints()
 * Constructor.
 */
ClusterBindingConstraints::ClusterBindingConstraints(const CString &cName) :
    constraintName(cName) {}

/**
 * ~ClusterBindingConstraints()
 * Destructor.
 */
ClusterBindingConstraints::~ClusterBindingConstraints() = default;

/**
 * clone()
 * Create a cloned copy of this cluster binding constraints object. The cluster
 * relation and all associated scenarios are automatically updated to the
 * supplied cluster.
 */
std::unique_ptr<ClusterBindingConstraints> ClusterBindingConstraints::clone(const CString &cName) {
    auto ab = std::make_unique<ClusterBindingConstraints>(cName);

    ab->setActorNames(getActorNames());

    return ab;
}

/**
 * constructFromXML()
 * Construct actor cluster constraints for scenario s from XML data.
 */
void ClusterBindingConstraints::constructFromXML(const CNodePtr clusterConstraintsNode) {
    // Actors
    for (CNode *n = CGetChildNode(clusterConstraintsNode); n != nullptr; n = CNextNode(n)) {
        if (CIsNode(n, "actor")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Actor has no name identifier.");
            }

            addActorName(CGetAttribute(n, "name"));
        }
    }
}

/**
 * convertToXML()
 * Convert actor bindings constraints of scenario s to XML data.
 */
void ClusterBindingConstraints::convertToXML(const CNodePtr clusterConstraintsNode) {
    // ClusterConstraint
    CAddAttribute(clusterConstraintsNode, "name", getClusterBindingConstraintName());

    // Actors
    if (!getActorNames()->empty()) {
        std::list<CString> *actorNames = getActorNames();
        for (const auto &n : *actorNames) {
            CNode *actorNode = CAddNode(clusterConstraintsNode, "actor");
            CAddAttribute(actorNode, "name", n);
        }
    }
}

bool ClusterBindingConstraints::hasActor(const CString &actorName) {
    std::list<CString> *actorNames = getActorNames();
    return std::any_of(actorNames->begin(), actorNames->end(), [actorName](const CString &n) {
        return n == actorName;
    });
}

/**
 * ClusterExclusionConstraints()
 * Constructor.
 */
ClusterExclusionConstraints::ClusterExclusionConstraints() = default;

/**
 * ~ClusterExclusionConstraints()
 * Destructor.
 */
ClusterExclusionConstraints::~ClusterExclusionConstraints() = default;

/**
 * clone()
 * Create a cloned copy of this cluster exclusion constraints object. The cluster
 * relation and all associated scenarios are automatically updated to the
 * supplied cluster.
 */
std::unique_ptr<ClusterExclusionConstraints> ClusterExclusionConstraints::clone() {
    auto ab = std::make_unique<ClusterExclusionConstraints>();

    ab->setClusterExclusionNames(getClusterExclusionNames());

    return ab;
}

/**
 * constructFromXML()
 * Construct cluster exclusion constraints for scenario s from XML data.
 */
void ClusterExclusionConstraints::constructFromXML(const CNodePtr clusterExclusionsNode) {
    // Actors
    for (CNode *n = CGetChildNode(clusterExclusionsNode); n != nullptr; n = CNextNode(n)) {
        if (CIsNode(n, "cluster")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Cluster has no name identifier.");
            }

            addClusterExclusionName(CGetAttribute(n, "name"));
        }
    }
}

/**
 * convertToXML()
 * Convert cluster exclusion constraints of scenario s to XML data.
 */
void ClusterExclusionConstraints::convertToXML(const CNodePtr clusterExclusionsNode) {
    // Clusters
    if (!getClusterExclusionNames()->empty()) {
        std::list<CString> *exclusionNames = getClusterExclusionNames();
        for (const auto &n : *exclusionNames) {
            CNode *clusterNode = CAddNode(clusterExclusionsNode, "cluster");
            CAddAttribute(clusterNode, "name", n);
        }
    }
}

bool ClusterExclusionConstraints::hasClusterExclusion(const CString &clusterName) {
    std::list<CString> *exclusionNames = getClusterExclusionNames();
    return std::any_of(exclusionNames->begin(),
                       exclusionNames->end(),
                       [clusterName](const CString &n) { return n == clusterName; });
}

/**
 * ScenarioBindingConstraints()
 * Constructor.
 */
ScenarioBindingConstraints::ScenarioBindingConstraints(const Scenario& s) : scenario(&s) {}

/**
 * ~GraphBindingConstraints()
 * Destructor.
 */
ScenarioBindingConstraints::~ScenarioBindingConstraints() = default;

/**
 * clone()
 * Create a cloned copy of this scenario binding object. The channel relations
 * are updated to the corresponding channels in the scenario graph of
 * scenario s.
 */
std::unique_ptr<ScenarioBindingConstraints> ScenarioBindingConstraints::clone(const Scenario& s) {
    auto bc = std::make_unique<ScenarioBindingConstraints>(s);

    // Channel binding constraints
    for (auto& [name, cbc] : channelBindingConstraints) {
        // Find corresponding channel in the scenario graph of scenario s

        // Set the channel binding constraints of channel
        bc->setConstraintsOfChannel(name, cbc->clone(name));
    }

    // Cluster binding constraints
    for (const auto& cbc : clusterBindingConstraints) {
        // Set the channel binding constraints of actor
        bc->addClusterBindingConstraint(cbc->clone(cbc->getClusterBindingConstraintName()));
    }

    // Cluster exclusion constraints
    for (const auto& cec : clusterExclusionConstraints) {
        // Set the channel binding constraints of cluster
        bc->addExclusionConstraint(cec->clone());
    }

    return bc;
}

/**
 * constructFromXML()
 * Construct scenario binding constraints from XML data.
 */
void ScenarioBindingConstraints::constructFromXML(const CNodePtr constraintsNode) {
    // Iterate over all child nodes in the constraint node
    for (CNode *n = CGetChildNode(constraintsNode); n != nullptr; n = CNextNode(n)) {
        // Channel?
        if (CIsNode(n, "channelConstraints")) {
            // Find channel name
            if (!CHasAttribute(n, "name")) {
                throw CException("channelConstraints has no name identifier.");
            }
            CString name = CGetAttribute(n, "name");

            // Construct binding constraints
            getConstraintsOfChannel(name)->constructFromXML(n);
        } else if (CIsNode(n, "cluster")) {
            // Find cluster name
            if (!CHasAttribute(n, "name")) {
                throw CException("cluster has no name identifier.");
            }
            CString name = CGetAttribute(n, "name");
            // Construct binding constraints
            getClusterBindingConstraints(name)->constructFromXML(n);
        } else if (CIsNode(n, "excludeClusters")) {
            // Construct binding constraints
            clusterExclusionConstraints.push_back(std::make_unique<ClusterExclusionConstraints>());
            clusterExclusionConstraints.back()->constructFromXML(n);
        }
    }
}

/**
 * convertToXML()
 * Convert the scenario binding constraints to XML data.
 */
void ScenarioBindingConstraints::convertToXML(const CNodePtr constraintsNode) {
    // Iterate over all channel binding constraints
    for (auto& [c, cbc] : channelBindingConstraints) {
        cbc->convertToXML(CAddNode(constraintsNode, "channelConstraints"));
    }

    // Iterate over all cluster binding constraints
    for (const auto& cbc : clusterBindingConstraints) {
        cbc->convertToXML(CAddNode(constraintsNode, "cluster"));
    }

    // Iterate over all cluster exclusion constraints
    for (const auto& cec : clusterExclusionConstraints) {
        cec->convertToXML(CAddNode(constraintsNode, "excludeClusters"));
    }
}

/**
 * getConstraintsOfChannel()
 * The function returns the channel binding constraints of the channel.
 */
ChannelBindingConstraint *ScenarioBindingConstraints::getConstraintsOfChannel(const CString &name) {
    // Create empty set of binding constraints when no constraints exist
    if (!hasChannelBindingConstraint(name)) {
        setConstraintsOfChannel(name, std::make_unique<ChannelBindingConstraint>(name));
    }

    return (channelBindingConstraints.find(name)->second).get();
}

/**
 * setConstraintsOfChannel()
 * The function sets the channel binding constraints of the channel.
 */
void ScenarioBindingConstraints::setConstraintsOfChannel(const CString &name,
                                                         std::unique_ptr<ChannelBindingConstraint> bc) {
    channelBindingConstraints[name] = std::move(bc);
}

/**
 * hasChannelBindingConstraint()
 * The function returns true when the channel has channel binding constraints.
 * Otherwise the function returns false.
 */
bool ScenarioBindingConstraints::hasChannelBindingConstraint(const CString &name) {
    return channelBindingConstraints.find(name) != channelBindingConstraints.end();
}

/**
 * updateConstraintsOfChannel()
 * The function updates the name of the constraint with the given name.
 */
void ScenarioBindingConstraints::removeConstraintsOfChannel(const CString &name) {
    channelBindingConstraints.erase(name);
}

/**
 * getClusterBindingConstraints()
 * The function creates and returns the cluster binding constraints.
 */
ClusterBindingConstraints *
ScenarioBindingConstraints::getClusterBindingConstraints(const CString &constraintName) {
    for (const auto& cbc : clusterBindingConstraints) {
        if (cbc->getClusterBindingConstraintName() == constraintName) {
            return cbc.get();
        }
    }
    clusterBindingConstraints.push_back(std::make_unique<ClusterBindingConstraints>(constraintName));

    return clusterBindingConstraints.back().get();
}

/**
 * getClusterBindingConstraintsOfActor()
 * The function returns the actor binding constraints.
 */
ClusterBindingConstraints *
ScenarioBindingConstraints::getClusterBindingConstraintsOfActor(const CString &actorName) {
    for (const auto& cbc : clusterBindingConstraints) {
        std::list<CString> *names = cbc->getActorNames();
        for (const auto &n : *names) {
            if (n == actorName) {
                return cbc.get();
            }
        }
    }
    throw CException("No constraints exist for actor '" + actorName + "'.");
}

/**
 * addClusterBindingConstraint()
 * The function sets the actor binding constraints of the actor.
 */
void ScenarioBindingConstraints::addClusterBindingConstraint(std::unique_ptr<ClusterBindingConstraints> bc) {
    clusterBindingConstraints.push_back(std::move(bc));
}

/**
 * hasClusterBindingConstraints()
 * The function returns true when the cluster has clusterBinding constraints.
 * Otherwise the function returns false.
 */
bool ScenarioBindingConstraints::hasClusterBindingConstraints(const CString &actorName) {
    for (const auto& cbc : clusterBindingConstraints) {
        std::list<CString> *names = cbc->getActorNames();
        for (const auto &n : *names) {
            if (n == actorName) {
                return true;
            }
        }
    }
    return false;
}

/**
 * getConstraintsOfCluster()
 * The function returns the cluster exclusion constraints.
 */
ClusterExclusionConstraints *
ScenarioBindingConstraints::getClusterExclusionConstraints(const CString &clusterName) {
    for (const auto& cec : clusterExclusionConstraints) {
        std::list<CString> *names = cec->getClusterExclusionNames();
        for (const auto &n : *names) {
            if (n == clusterName) {
                return cec.get();
            }
        }
    }
    throw CException("No constraints exist for cluster '" + clusterName + "'.");
}

/**
 * addExclusionConstraint()
 * The function sets the cluster exclusion constraints of the cluster.
 */
void ScenarioBindingConstraints::addExclusionConstraint(std::unique_ptr<ClusterExclusionConstraints> cec) {
    clusterExclusionConstraints.push_back(std::move(cec));
}

/**
 * hasClusterExclusionConstraints()
 * The function returns true when the cluster has clusterExclusion constraints.
 * Otherwise the function returns false.
 */
bool ScenarioBindingConstraints::hasClusterExclusionConstraints(const CString &clusterName) {
    for (const auto& cec: clusterExclusionConstraints) {
        std::list<CString> *names = cec->getClusterExclusionNames();
        for (const auto& n: *names) {
            if (n == clusterName) {
                return true;
            }
        }
    }
    return false;
}

/**
 * GraphBindingConstraints()
 * Constructor.
 */
GraphBindingConstraints::GraphBindingConstraints() = default;

/**
 * ~GraphBindingConstraints()
 * Destructor.
 */
GraphBindingConstraints::~GraphBindingConstraints() = default;

/**
 * clone()
 * Create a cloned copy of this graph binding object. The scenario relations
 * are updated to the corresponding scenario in the graph ag.
 */
std::unique_ptr<GraphBindingConstraints> GraphBindingConstraints::clone(Graph *ag) {
    auto g = std::make_unique<GraphBindingConstraints>();

    // Scenario binding constraints
    for (auto& [sc, bc]: scenarioBindingConstraints) {
        // Find corresponding scenario in application graph ag
        ScenarioRef s = ag->getScenarioRef(sc->getName());

        // Set the scenario binding constraints of scenario s
        g->setConstraintsOfScenario(*s, bc->clone(*s));
    }

    return g;
}

/**
 * constructFromXML()
 * Construct graph binding constraints for scenario s from XML data.
 */
void GraphBindingConstraints::constructFromXML(const Scenario& s, const CNodePtr constraintsNode) {
    getConstraintsOfScenario(s)->constructFromXML(constraintsNode);
}

/**
 * convertToXML()
 * Convert the graph binding constraints of scenario s to XML data.
 */
void GraphBindingConstraints::convertToXML(Scenario *s, const CNodePtr constraintsNode) {
    getConstraintsOfScenario(*s)->convertToXML(constraintsNode);
}

/**
 * getConstraintsOfScenario()
 * The function returns the scenario binding constraints of scenario s.
 */
ScenarioBindingConstraints *GraphBindingConstraints::getConstraintsOfScenario(const Scenario& s) {
    // Create empty set of binding constraints when no constraints exist
    if (!hasScenarioBindingConstraints(s)) {
        setConstraintsOfScenario(s, std::make_unique<ScenarioBindingConstraints>(s));
}

    return (scenarioBindingConstraints.find(&s)->second).get();
}

/**
 * setConstraintsOfScenario()
 * The function sets the scenario binding constraints of scenario s.
 */
void GraphBindingConstraints::setConstraintsOfScenario(const Scenario& s,
                                                       std::unique_ptr<ScenarioBindingConstraints> bc) {
    scenarioBindingConstraints[&s] = std::move(bc);
}

/**
 * hasScenarioBindingConstraints()
 * The function returns true when scenario s has scenario binding constraints.
 * Otherwise the function returns false.
 */
bool GraphBindingConstraints::hasScenarioBindingConstraints(const Scenario& s) {
    return scenarioBindingConstraints.find(&s) != scenarioBindingConstraints.end();
}

} // End namespace FSMSADF
