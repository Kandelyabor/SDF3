/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   constraint.h
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
 * $Id: constraint.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONSTRAINT_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONSTRAINT_H_INCLUDED

#include "fsmsadf/base/graph.h"
#include "fsmsadf/base/type.h"

namespace FSMSADF {

/**
 * ChannelBindingConstraint
 * Container for channel binding constraints.
 */
class ChannelBindingConstraint {
public:
    // Constructor
    explicit ChannelBindingConstraint(const CString &name);

    // Destructor
    ~ChannelBindingConstraint();

    ChannelBindingConstraint(const ChannelBindingConstraint&) = delete;
    ChannelBindingConstraint& operator=(const ChannelBindingConstraint&) = delete;
    ChannelBindingConstraint(ChannelBindingConstraint&&) = delete;
    ChannelBindingConstraint& operator=(ChannelBindingConstraint&&) = delete;

    // Construct
    [[nodiscard]] std::unique_ptr<ChannelBindingConstraint> clone(const CString &name) const;

    // Construct
    void constructFromXML(CNodePtr channelConstraintsNode);

    // Convert
    void convertToXML(CNodePtr channelConstraintsNode) const;

    // Channel
    [[nodiscard]] CString getChannelName() const { return channelName; };
    void setChannelName(const CString &name) { channelName = name; };

    // Buffer size
    [[nodiscard]] BufferSize getBufferSize() const;
    void setBufferSize(const BufferSize& sz);
    [[nodiscard]] bool hasBufferSizeConstraint() const;

    // Buffer location
    [[nodiscard]] BufferLocation getBufferLocation() const;
    void setBufferLocation(const CString &loc);
    [[nodiscard]] bool hasBufferLocationConstraint() const;

    // Delay
    [[nodiscard]] Time getDelay() const;
    void setDelay(Time t);
    [[nodiscard]] bool hasDelayConstraint() const;

    // Bandwidth
    [[nodiscard]] Bandwidth getBandwidth() const;
    void setBandwidth(Bandwidth b);
    [[nodiscard]] bool hasBandwidthConstraint() const;

private:
    // Channel name
    CString channelName;

    // Buffer size
    BufferSize bufferSize;
    bool bufferSizeConstraint;

    // Buffer location
    BufferLocation bufferLocation;
    bool bufferLocationConstraint;

    // Bandwidth
    Bandwidth bandwidth;
    bool bandwidthConstraint;

    // Delay
    Time delay;
    bool delayConstraint;
};
using ChannelBindingConstraints = std::map<const CString, std::unique_ptr<ChannelBindingConstraint>>;

/**
 * ClusterBindingConstraints
 * Container for actor clustering constraints.
 */
class ClusterBindingConstraints {
public:
    // Constructor
    explicit ClusterBindingConstraints(const CString &cName);

    // Destructor
    ~ClusterBindingConstraints();

    ClusterBindingConstraints(const ClusterBindingConstraints&) = delete;
    ClusterBindingConstraints& operator=(const ClusterBindingConstraints&) = delete;
    ClusterBindingConstraints(ClusterBindingConstraints&&) = delete;
    ClusterBindingConstraints& operator=(ClusterBindingConstraints&&) = delete;

    // Construct
    std::unique_ptr<ClusterBindingConstraints> clone(const CString &cName);

    // Construct
    void constructFromXML(CNodePtr clusterConstraintsNode);

    // Convert
    void convertToXML(CNodePtr clusterConstraintsNode);

    // Constraint
    [[nodiscard]] CString getClusterBindingConstraintName() const { return constraintName; };

    // Actor
    std::list<CString> *getActorNames() { return &actorNames; };
    void setActorNames(const std::list<CString> *aNames) { actorNames = *aNames; };
    void addActorName(const CString &actorName) { actorNames.push_back(actorName); };
    bool hasActor(const CString &actorName);

private:
    // Name
    const CString constraintName;

    // Actor names
    std::list<CString> actorNames;
};

/**
 * ClusterExclusionConstraints
 * Container for actor cluster exclusion constraints.
 */
class ClusterExclusionConstraints {
public:
    // Constructor
    ClusterExclusionConstraints();

    // Destructor
    ~ClusterExclusionConstraints();

    ClusterExclusionConstraints(const ClusterExclusionConstraints&) = delete;
    ClusterExclusionConstraints& operator=(const ClusterExclusionConstraints&) = delete;
    ClusterExclusionConstraints(ClusterExclusionConstraints&&) = delete;
    ClusterExclusionConstraints& operator=(ClusterExclusionConstraints&&) = delete;

    // Construct
    std::unique_ptr<ClusterExclusionConstraints> clone();

    // Construct
    void constructFromXML(CNodePtr clusterExclusionsNode);

    // Convert
    void convertToXML(CNodePtr clusterExclusionsNode);

    // Exclusions
    std::list<CString> *getClusterExclusionNames() { return &clusterNames; };
    void setClusterExclusionNames(const std::list<CString> *eNames) { clusterNames = *eNames; };
    void addClusterExclusionName(const CString &clusterName) {
        clusterNames.push_back(clusterName);
    };
    bool hasClusterExclusion(const CString &clusterName);

private:
    // Actor names
    std::list<CString> clusterNames;
};

/**
 * ScenarioBindingConstraints
 * Container for scenario binding constraints.
 */
class ScenarioBindingConstraints {
public:
    // Constructor
    explicit ScenarioBindingConstraints(const Scenario& s);

    // Destructor
    ~ScenarioBindingConstraints();

    ScenarioBindingConstraints(const ScenarioBindingConstraints&) = delete;
    ScenarioBindingConstraints& operator=(const ScenarioBindingConstraints&) = delete;
    ScenarioBindingConstraints(ScenarioBindingConstraints&&) = delete;
    ScenarioBindingConstraints& operator=(ScenarioBindingConstraints&&) = delete;

    // Construct
    std::unique_ptr<ScenarioBindingConstraints> clone(const Scenario& s);

    // Construct
    void constructFromXML(CNodePtr constraintsNode);

    // Convert
    void convertToXML(CNodePtr constraintsNode);

    // Scenario
    [[nodiscard]] const Scenario *getScenario() const { return scenario; };

    // Channel binding constraints
    ChannelBindingConstraint *getConstraintsOfChannel(const CString &name);
    void setConstraintsOfChannel(const CString &name, std::unique_ptr<ChannelBindingConstraint> bc);
    void removeConstraintsOfChannel(const CString &name);
    bool hasChannelBindingConstraint(const CString &name);
    ChannelBindingConstraints *getAllChannelBindingConstraints() {
        return &channelBindingConstraints;
    }

    // Cluster binding constraints
    ClusterBindingConstraints *getClusterBindingConstraintsOfActor(const CString &actorName);
    ClusterBindingConstraints *getClusterBindingConstraints(const CString &constraintName);
    std::list<std::unique_ptr<ClusterBindingConstraints>> *getAllClusterBindingConstraints() {
        return &clusterBindingConstraints;
    }
    bool hasClusterBindingConstraints(const CString &actorName);

private:
    void addClusterBindingConstraint(std::unique_ptr<ClusterBindingConstraints> bc);

public:
    // Cluster exclusion constraints
    ClusterExclusionConstraints *getClusterExclusionConstraints(const CString &clusterName);
    std::list<std::unique_ptr<ClusterExclusionConstraints>> *getAllClusterExclusionConstraints() {
        return &clusterExclusionConstraints;
    }
    void addExclusionConstraint(std::unique_ptr<ClusterExclusionConstraints> cec);
    bool hasClusterExclusionConstraints(const CString &clusterName);

private:
    // Scenario
    const Scenario *scenario;

    // Channel binding constraints
    ChannelBindingConstraints channelBindingConstraints;

    // Cluster binding constraints
    std::list<std::unique_ptr<ClusterBindingConstraints>> clusterBindingConstraints;

    // Cluster exclusion constraints
    std::list<std::unique_ptr<ClusterExclusionConstraints>> clusterExclusionConstraints;
};

/**
 * GraphBindingConstraints
 * Container for graph binding constraints.
 */
class GraphBindingConstraints {
public:
    // Constructor
    GraphBindingConstraints();

    // Destructor
    ~GraphBindingConstraints();

    GraphBindingConstraints(const GraphBindingConstraints&) = delete;
    GraphBindingConstraints& operator=(const GraphBindingConstraints&) = delete;
    GraphBindingConstraints(GraphBindingConstraints&&) = delete;
    GraphBindingConstraints& operator=(GraphBindingConstraints&&) = delete;

    // Construct
    std::unique_ptr<GraphBindingConstraints> clone(Graph *ag);

    // Construct
    void constructFromXML(const Scenario& s, CNodePtr constraintsNode);

    // Convert
    void convertToXML(Scenario *s, CNodePtr constraintsNode);

    // Scenario binding constraints
    ScenarioBindingConstraints *getConstraintsOfScenario(const Scenario& s);
    void setConstraintsOfScenario(const Scenario& s, std::unique_ptr<ScenarioBindingConstraints> bc);
    bool hasScenarioBindingConstraints(const Scenario& s);

private:
    // Scenario binding constraints
    std::map<const Scenario *, std::unique_ptr<ScenarioBindingConstraints>> scenarioBindingConstraints;
};

} // End namespace FSMSADF

#endif
