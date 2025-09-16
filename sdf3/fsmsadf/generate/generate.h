/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2009
 *
 *  Function        :   Generate FSM-based SADF graphs
 *
 *  History         :
 *      14-07-09    :   Initial version.
 *
 * $Id: generate.h,v 1.1.2.2 2010-04-22 07:07:11 mgeilen Exp $
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

#ifndef FSMSADF_GENERATE_GENERATE_H_INCLUDED
#define FSMSADF_GENERATE_GENERATE_H_INCLUDED

#include "fsmsadf/base/graph.h"
#include <memory>

namespace FSMSADF {

/**
 * RandomGraph
 * Generate a random FSM-based SADF graph.
 */
class RandomGraph {
public:
    // Constructor
    RandomGraph();

    // Destructor
    ~RandomGraph() = default;

    RandomGraph(const RandomGraph&) = delete;
    RandomGraph& operator=(const RandomGraph&) = delete;
    RandomGraph(RandomGraph&&) = delete;
    RandomGraph& operator=(RandomGraph&&) = delete;

    // Generate random graph
    std::unique_ptr<Graph > generateGraph();

    // Initialize generator from XML
    void initializeFromXML(CNode *settingsNode);

    // Structure of the graph
    bool stronglyConnected;
    uint nrScenarioGraphs;
    uint nrActors;

    // Degree of the actors
    double avgDegree;
    double varDegree;
    double minDegree;
    double maxDegree;

    // Actor iteration counts
    double avgIterCnt;
    double varIterCnt;
    double minIterCnt;
    double maxIterCnt;

    // Initial tokens
    double initialTokenChannelProp;
    double initialTokenProp;

    // Scenarios
    uint nrScenarios;

    // Processor types
    uint nrProcTypes;
    double mapChance;
    bool unifiedActorProperties;

    // Execution time
    bool execTime;
    double avgExecTime;
    double varExecTime;
    double minExecTime;
    double maxExecTime;

    // State size
    /// @todo rename this to actor memory (or so)
    bool stateSize;
    double avgStateSize;
    double varStateSize;
    double minStateSize;
    double maxStateSize;

    // Token size
    bool tokenSize;
    double avgTokenSize;
    double varTokenSize;
    double minTokenSize;
    double maxTokenSize;

    // Throughput constraint
    bool throughputConstraint;
    uint autoConcurrencyDegree;
    double throughputScaleFactor;

private:
    // Graph creation
    std::unique_ptr<Graph > createGraph();
    std::unique_ptr<ScenarioGraph > createScenarioGraph(Graph *g);
    ChannelRef createChannel(ScenarioGraph *sg, PortRefs &inPorts, PortRefs &outPorts);
    ActorRef createActor(ScenarioGraph *sg, PortRefs &inPorts, PortRefs &outPorts);
    static Port* createPort(Actor& a,  Port::PortType type);
    void assignRatesAndInitialTokens(Graph *g);
    ChannelRefs selectInitialTokenChannels(ScenarioGraph *sg);
    static void getChannelsBetweenActors(const Actor& a, const Actor& b, ChannelRefs &channels);
    void assignInitialTokensToChannels(ChannelRefs &initialTokenChannels, RepetitionVector &repVec);
    static void assignRatesToPorts(ScenarioGraph *sg, Scenario *s, RepetitionVector &repVec);
    void findCycles(const Actor* p,
                    const Actor* a,
                    std::map<const Actor *, uint> &color,
                    std::map<const Actor *, const Actor *> &parent,
                    ChannelRefs &channels);
    void dfsStronglyConnected(const Actor& a, std::vector<bool> &visit, bool reverseChannels);
    bool isStronglyConnected(ScenarioGraph *sg);

    // Scenario creation
    std::unique_ptr<Scenario > createScenario(Graph *g);

    // FSM creation
    FSM* createFSM(Graph *g);

    // Graph properties
    void assignProperties(Graph *g);
    void assignActorProperties(Actor& a, ScenarioRefs &scenarios, CStrings &processorTypes);
    void assignChannelProperties(Channel *c, ScenarioRefs &scenarios);
    void assignThroughputConstraint(Graph *g);

    // Random graph components
    Ports::iterator randomPort(Ports &p);
    PortRefs::iterator randomPort(PortRefs &p);
    ChannelRef randomChannel(const ChannelRefs &c);
    ScenarioGraph* randomScenarioGraph(ScenarioGraphs &s);
    FSMstate* randomState(FSMstates &s);
    CStrings randomProcessorTypes();

    // Random number generator
    MTRand mtRand;
};

} // End namespace FSMSADF

#endif
