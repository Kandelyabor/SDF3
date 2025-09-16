/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF graph
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: graph.h,v 1.1.2.11 2010-04-22 07:07:10 mgeilen Exp $
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
#ifndef FSMSADF_BASE_GRAPH_H_INCLUDED
#define FSMSADF_BASE_GRAPH_H_INCLUDED

#include "component.h"
#include "channel.h"
#include "fsm.h"
#include "sdf/sdf.h" // IWYU pragma: keep
#include <memory>
#include <utility>

namespace FSMSADF {

// List of scenario graph pointers
class ScenarioGraphs: public std::list<std::unique_ptr<ScenarioGraph>>{
    public:
    ScenarioGraphs() = default;
    ~ScenarioGraphs() = default;
    ScenarioGraphs(const ScenarioGraphs &) = delete;
    ScenarioGraphs& operator=(const ScenarioGraphs &) = delete;
    ScenarioGraphs(ScenarioGraphs&&) = delete;
    ScenarioGraphs& operator=(ScenarioGraphs&&) = delete;
    

    void removeScenarioGraph(ScenarioGraph *g) {
        auto it = std::find_if(this->begin(), this->end(),
                               [g](const std::unique_ptr<ScenarioGraph>& sg) { return sg.get() == g; });
        if (it != this->end()) {
            this->erase(it);
        }
    }
};

// maps a persistent token by its name to its channel and index on that channel
using PersistentTokenMap = std::map<CString, std::pair<Channel *, uint>>;

// maps a persistent token by its name to a unique index in the graph
// (and in the graph's matrix)
using PersistentTokenIndexMap = std::map<CString, uint>;

/**
 * ScenarioGraph
 * Container for FSM-based SADF scenario graph.
 */
class ScenarioGraph : public GraphComponent {
public:
    // Constructor
    explicit ScenarioGraph(const GraphComponent& c);

    // Copy constructor
    ScenarioGraph(const ScenarioGraph& other) = delete;

    // Copy assignment operator
    ScenarioGraph& operator=(const ScenarioGraph& other) = delete;

    // Move constructor
    ScenarioGraph(ScenarioGraph&& other) noexcept = delete;

    // Move assignment operator
    ScenarioGraph& operator=(ScenarioGraph&& other) noexcept = delete;

    // Destructor
    ~ScenarioGraph() override;

    // Construct
    [[nodiscard]] static std::unique_ptr<ScenarioGraph> create(const GraphComponent& c);
    [[nodiscard]] std::unique_ptr<ScenarioGraph> createCopy(const GraphComponent& c) const;
    [[nodiscard]] std::unique_ptr<ScenarioGraph> clone(const GraphComponent& c) const;

    // Construct
    void constructFromXML(CNodePtr scenarioGraphNode);
    void constructPropertiesFromXML(const Scenario& s, CNodePtr scenarioNode);

    // Convert
    void convertToXML(CNodePtr scenarioGraphNode);
    void convertPropertiesToXML(Scenario& s, CNodePtr scenarioNode);

    // Extract SDF graph
    [[nodiscard]] std::unique_ptr<SDF::TimedSDFgraph> extractSDFgraph(const Scenario& s) const;

    // get partial repetition vector from spec of weakly consistent fsmsadf
    [[nodiscard]] std::unique_ptr<RepetitionVector> getPartialRepetitionVector(const Scenario& s) const;

    // set partial repetition vector for scenario
    void setPartialRepetitionVector(Scenario *s, const RepetitionVector &v);

    // Isolate scenario
    void isolateScenario(const Scenario& s);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Actors
    Actors &getActors() { return actors; };
    ActorRef getActor(CId id);
    ActorRef getActor(const CString &name);
    bool hasActor(const CString &name);
    void addActor(std::unique_ptr<Actor> a);
    void removeActor(const CString &name);
    ActorRef createActor();
    ActorRef createActor(const GraphComponent& c);
    ChannelRef reduceActor(Scenario *s, Actor *a);

    // Channels
    [[nodiscard]] const Channels &getChannels() const { return channels; };
    [[nodiscard]] ChannelRefs getChannelRefs() const {
        ChannelRefs result;
        for (const auto& c: this->channels) {
            result.push_back(c.get());
        }
        return result;
    };
    ChannelRef getChannel(CId id);
    ChannelRef getChannel(const CString &name);
    ChannelRef getChannelStartingWith(const CString &name);
    bool hasChannel(const CString &name);
    bool hasChannelStartingWith(const CString &name);
    void addChannel(std::unique_ptr<Channel> c);
    Channels::iterator removeChannel(const CString &name);
    ChannelRef createChannel(const GraphComponent& c);
    ChannelRef createChannel(Actor *src, Actor *dst);
    ChannelRef createChannel(Actor *src,
                           Actor *dst,
                           const std::map<const Scenario *, Rate> &srcRate,
                           const std::map<const Scenario *, Rate> &dstRate,
                           const CString& name);

    [[nodiscard]] CStrings getPersistentTokens() const;
    [[nodiscard]] CStrings getPersistentTokensFinal() const;
    [[nodiscard]] PersistentTokenMap getPersistentTokenMap() const;
    [[nodiscard]] PersistentTokenMap getPersistentTokenMapFinal() const;
    [[nodiscard]] PersistentTokenIndexMap getPersistentTokenIndexMap() const;
    [[nodiscard]] PersistentTokenIndexMap getPersistentTokenIndexMapFinal() const ;

    void checkDuplicatePersistentTokens() const;

    void reduceInitialTokens(const Scenario& s, Channel& c);

private:
    ChannelRef consumerGrouping(const Scenario& s, Channel& c);
    ChannelRef producerGrouping(const Scenario& s, Channel& c);

public:
    // Storage distribution sets
    [[nodiscard]] const std::map<const Scenario *, StorageDistributions> &
    getStorageDistributions() const {
        return storageDistributions;
    };
    void setStorageDistributions(const std::map<const Scenario *, StorageDistributions> &d) {
        storageDistributions = d;
    }
    StorageDistributions &getStorageDistributionsOfScenario(const Scenario& s);
    void setStorageDistributionsOfScenario(const Scenario& s, const StorageDistributions& d);
    [[nodiscard]] bool hasStorageDistributionsInScenario(const Scenario& s) const;
    void clearStorageDistributions() { storageDistributions.clear(); }

    // Graph
    [[nodiscard]] Graph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, ScenarioGraph &g) { return g.print(out); };

    // Detector
    static void setDetectorActor(Actor& a);

private:
    // Default scenario
    [[nodiscard]] ScenarioRef getDefaultScenario() const;

    // Graph type
    CString type;

    // Actors
    Actors actors;

    // Channels
    Channels channels;

    // persistent final tokens
    PersistentTokenMap persistentTokenMapFinal;

    // Storage distributions
    std::map<const Scenario *, StorageDistributions> storageDistributions;

public:
    // properties of the graph
    [[nodiscard]] bool isStronglyConsistent() const;
    [[nodiscard]] bool isWeaklyConsistent(const Scenario& s) const;
};

/**
 * Graph
 * Container for FSM-based SADF graph.
 */
class Graph : public GraphComponent {
public:
    // Constructor
    explicit Graph(const GraphComponent& c);
    explicit Graph(Graph *g);
    Graph();

    // Copy constructor
    Graph(const Graph& other) = delete;

    // Copy assignment operator
    Graph& operator=(const Graph& other) = delete;

    // Move constructor
    Graph(Graph&& other) noexcept = delete;

    // Move assignment operator
    Graph& operator=(Graph&& other) noexcept = delete;

    // create from a timed sdf graph
    explicit Graph(const SDF::TimedSDFgraph& g);

    // Destructor
    ~Graph() override;

    // Construct
    [[nodiscard]] static std::unique_ptr<Graph> create(const GraphComponent& c);
    [[nodiscard]] std::unique_ptr<Graph> createCopy(const GraphComponent& c) const;
    [[nodiscard]] std::unique_ptr<Graph> clone(const GraphComponent& c) const;

    // Construct
    void constructFromXML(CNodePtr applicationGraphNode);

    // Convert
    void convertToXML(CNodePtr applicationGraphNode) const;

    // Extract SDF graph
    static std::unique_ptr<SDF::TimedSDFgraph> extractSDFgraph(const Scenario& s);

    // Get the partial repetition vector for a scenario of a weakly consistent FSMSADF
    static std::unique_ptr<RepetitionVector> getPartialRepetitionVector(const Scenario& s);

    // Isolate scenarios
    void isolateScenarios();
    [[nodiscard]] bool hasIsolatedScenarios() const;

    // Scenarios
    [[nodiscard]] ScenarioRef getScenarioRef(const CString &name);
    [[nodiscard]] const Scenario* getScenarioRef(const CString &name) const;
    [[nodiscard]] Scenarios &getScenarios() { return scenarios; };
    [[nodiscard]] const Scenarios &getConstScenarios() const { return scenarios; };
    void addScenario(std::unique_ptr<Scenario> s) { scenarios.insert(std::move(s)); };

    // Default scenario
    [[nodiscard]] Scenario* getDefaultScenario();
    [[nodiscard]] const Scenario* getDefaultScenario() const;

    // Scenario graphs
    [[nodiscard]] ScenarioGraph *getScenarioGraph(const CString &name) const;
    [[nodiscard]] ScenarioGraph *getScenarioGraph(const Scenario &name) const;
    [[nodiscard]] ScenarioGraphs &getScenarioGraphs() { return scenarioGraphs; };
    [[nodiscard]] const ScenarioGraphs &getConstScenarioGraphs() const { return scenarioGraphs; };
    void addScenarioGraph(std::unique_ptr<ScenarioGraph> g) { scenarioGraphs.push_back(std::move(g)); };
    void replaceScenarioGraph(ScenarioGraph *fromG, std::unique_ptr<ScenarioGraph> toG);

    // FSM
    [[nodiscard]] FSM *getFSM() const { return fsm.get(); };
    void setFSM(std::unique_ptr<FSM> f) { fsm = std::move(f); };

    // FSM
    [[nodiscard]] CString getRegEx() const { return regex; };

    void setRegEx(const CString& r) { regex = r; };

    // Get all persistent tokens
    [[nodiscard]] CStrings getPersistentTokens() const;

    // Get all persistent tokens of a scenario
    [[nodiscard]] static CStrings getPersistentTokensOfScenario(const Scenario &s);

    // get persistent tokens
    [[nodiscard]] CStrings getPersistentTokensFinal() const;

    // Get all persistent tokens of a scenario
    [[nodiscard]] static CStrings getPersistentTokensFinalOfScenario(const Scenario &s);

    // Throughput constraint
    [[nodiscard]] Throughput getThroughputConstraint() const { return throughputConstraint; };
    void setThroughputConstraint(const Throughput &t) { throughputConstraint = t; };

    // void setDefaultScenario(Scenario* s) { defaultScenario = s; }
    void setUnits(CNode *units) { unitsProperty = units; };

    [[nodiscard]] CNode *getUnits() const { return unitsProperty; };

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, Graph &g) { return g.print(out); };

    // properties of the graph
    [[nodiscard]] bool isStronglyConsistent() const;
    [[nodiscard]] bool isWeaklyConsistent() const;

    // regex-based
    void checkWeaklyConsistency();

    // check for prohibited scenario names for regex analysis
    void checkScenarioNames();

    [[nodiscard]] CString reportWeaklyConsistent() const;
    [[nodiscard]] bool hasUnitRewards() const;

private:
    // Scenarios
    Scenarios scenarios;
    Scenario defaultScenario;

    // Scenario graphs
    ScenarioGraphs scenarioGraphs;

    // FSM
    std::unique_ptr<FSM> fsm;

    // regex
    CString regex;

    // Throughput constraint
    Throughput throughputConstraint = 0.0;

    // Units
    CNode *unitsProperty;
};

} // End namespace FSMSADF

#endif
