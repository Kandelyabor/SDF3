/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   maxplusautomaton.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      Max-Plus automaton
 *
 *  History         :
 *      26-04-10    :   Initial version.
 *
 * $Id: maxplusautomaton.cc,v 1.1.2.15 2010-08-04 14:47:36 mgeilen Exp $
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

#include "maxplusautomaton.h"
#include "fsmsadf/base/graph.h"
#include "fsmsadf/base/scenario.h"
#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include "maxplus/base/analysis/mcm/mcmyto.h"
#include "regex_thr.h"
#include "regexparser_thr.h"
#include <memory>

using namespace MaxPlus;
using namespace ::Graphs;

namespace FSMSADF {

using namespace FSMSADF::MPExploreMaxPlusAutomaton;

// #define _PRINT_REGEX
// #define _No_REGEX_COMPRESSION
// #define _RTM //detailed runtime measurements

/**
 * throughputAnalysisGraphUsingMaxPlusAutomaton()
 * Compute the throughput of an FSM-based SADF graph through a
 * max-plus automaton.
 */
Throughput throughputAnalysisGraphUsingMaxPlusAutomaton(const Graph& g) {
    Throughput thr = 0.0;

    // Create a copy of the graph
    auto gt = std::unique_ptr<Graph>(g.clone(GraphComponent(g.getParent(), g.getId(), g.getName())));

    // Compute throughput
    thr = ThroughputCalculator::calculateThroughput(*gt);

    return thr;
}

/**
 * throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent()
 * Compute the throughput of an FSM-SADF graph through a max-plus automaton.
 */
Throughput throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent(const Graph& g) {
    Throughput thr = 0.0;

    // Create a copy of the graph
    auto gt = std::unique_ptr<Graph>(g.clone(GraphComponent(g.getParent(), g.getId(), g.getName())));

    // check consistency
    if (!gt->isWeaklyConsistent()) {
        throw ::CException("FSM-SADF graph is not consistent. \n" + gt->reportWeaklyConsistent());
    }

    // Compute throughput
    thr = ThroughputCalculator::calculateThroughputWeaklyConsistent(*gt);

    return thr;
}

/**
 * throughputAnalysisGraphUsingMaxPlusAutomatonRegex()
 * Compute the throughput of an FSM-based SADF graph through a
 * max-plus automaton.
 */
Throughput throughputAnalysisGraphUsingMaxPlusAutomatonRegex(const Graph& g) {
    Throughput thr = 0.0;

    // check if there is a regex in g
    if (g.getRegEx().empty()) {
        throw ::CException("Graph does not contain a regular expression.");
    }

    // create a copy of the graph
    auto gt = std::unique_ptr<Graph>(g.clone(GraphComponent(g.getParent(), g.getId(), g.getName())));

    // check for invalid scenario names.
    gt->checkScenarioNames();

    // check consistency
    gt->checkWeaklyConsistency();

    // Compute throughput
    thr = ThroughputCalculator::calculateThroughputRegex(*gt);

    return thr;
}
/**
 * criticalScenarioSequenceWeaklyConsistent()
 * Compute a critical scenario sequence of a weakly consistent FSM-SADF graph through a max-plus
 * automaton.
 */
std::unique_ptr<ScenarioConstRefList> criticalScenarioSequenceWeaklyConsistent(const Graph& g) {
    MPExploreMaxPlusAutomaton::ThroughputCalculator tc;

    // Create a copy of the graph
    auto gt = std::unique_ptr<Graph>(g.clone(GraphComponent(g.getParent(), g.getId(), g.getName())));

    // check consistency
    if (!gt->isWeaklyConsistent()) {
        throw ::CException("FSM-SADF graph is not consistent");
    }

    // Compute throughput
    std::unique_ptr<ScenarioConstRefList> sl = FSMSADF::MPExploreMaxPlusAutomaton::ThroughputCalculator::criticalScenarioSequenceWeaklyConsistent(*gt);

    // Copy the scenarios to point to the original graph instead of the clone gt, which will
    // be freed soon

    auto new_sl = std::make_unique<ScenarioConstRefList>();
    for (const auto& s : *sl) {
        new_sl->push_back(g.getScenarioRef(s->getName()));
    }

    return new_sl;
}

namespace MPExploreMaxPlusAutomaton {

/**
 * Compute the throughput of an FSM-SADF graph through a max-plus automaton.
 */
Throughput ThroughputCalculator::calculateThroughput(const Graph& g) {

    std::unique_ptr<ScenarioMatrices> scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenarios(g);
    std::unique_ptr<ScenarioRewards> scenarioRewards = GraphConversion::getRewardsFromScenarios(g);

    // create a minimal edge labelled scenario automaton
    std::unique_ptr<EdgeLabeledScenarioFSM> elsFSM =
            FSMConverter::minimalEdgeLabeledFSM(*(g.getFSM()));

    auto fsm = std::make_unique<MaxPlusAutomatonWithRewards>();

    // add state for every initial token and location of the fsm

    // get the number of initial tokens
    unsigned int nrTokens = (*scenarioMatrices->begin()).second->getSize();
    if (nrTokens == 0) {
        throw ::CException("Graph has no initial tokens. Cannot compute throughput");
}

    // create the FSM states for every pair of a state of the FSMSADF's FSM
    // and an initial token
    const ELSSetOfStates &Q = elsFSM->getStates();
    for (const auto &[key, q] : Q) {
        for (unsigned int k = 0; k < nrTokens; k++) {
            // create a state for (q, k)
            auto *qq = dynamic_cast<ELSState *>(&(*q));
            CId qId = qq->getLabel();
            fsm->addState({qId, static_cast<int>(k)});
        }
    }

    // add the edges. For every edge (q1, q2) in the original fsm,
    // let s be the scenario of q2 and let M_s be the matrix of s
    // For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
    // to (q2,m) labelled with d.

    // for every state of the fsm...
    for (const auto &[key, q] : Q) {
        auto *q1 = dynamic_cast<ELSState *>(&(*q));
        CId q1Id = q1->getLabel();

        // for every outgoing edge of the state
        const ELSSetOfEdgeRefs &t = q1->getOutgoingEdges();
        for (const auto *e : t) {
            const auto *tr = dynamic_cast<ELSEdgeRef>(e);
            CId q2Id = (dynamic_cast<ELSStateRef>(tr->getDestination()))->getLabel();
            CString sc = tr->getLabel();
            const auto* Ms = dynamic_cast<const Matrix*>((*scenarioMatrices)[sc].get());
            CDouble rew = (*scenarioRewards)[sc];

            // for every entry in the matrix
            for (unsigned int k = 0; k < nrTokens; k++) {
                for (unsigned int m = 0; m < nrTokens; m++) {
                    MPDelay d = Ms->get(k, m);
                    if (!MP_IS_MINUS_INFINITY(d)) {
                        MPARStateRef src = fsm->getStateLabeled({q1Id, static_cast<int>(k)});
                        MPARStateRef dst = fsm->getStateLabeled({q2Id, static_cast<int>(m)});
                        fsm->addEdge(*src, {d, sc.c_str(), rew}, *dst);
                    }
                }
            }
        }
    }

    // calculate MCR of the FSM
    double mcm = calculateMCR(fsm.get());

    return 1.0 / mcm;
}

/**
 * Compute the throughput of the weakly consistent FSM-SADF graph
 * as well as a corresponding critical scenario sequence.
 */
std::unique_ptr<ScenarioConstRefList>
ThroughputCalculator::throughputAndCriticalScenarioSequenceWeaklyConsistent(const Graph& g,
                                                                            Throughput &th) {

    std::unique_ptr<ScenarioMatrices> scenarioMatrices = nullptr;
    try {
        scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenariosForWeakFSMSADF(g);
    } catch (CScenarioDeadlockException &e) {
        // one of the scenarios deadlocks and has no matrix.
        // therefore the throughput is zero
        th = 0.0;
        auto l = std::make_unique<ScenarioConstRefList>();
        l->push_back(e.scenario);
        return l;
    }

    // create a minimal edge labelled scenario automaton
    std::unique_ptr<EdgeLabeledScenarioFSM> elsFSM =
            FSMConverter::minimalEdgeLabeledFSM(*(g.getFSM()));

    auto fsm = std::make_unique<MaxPlusAutomatonWithRewards>();

    // add state for every initial token and location of the fsm

    // create the FSM states for every pair of a state of the FSMSADF's FSM
    // and an initial token
    const ELSSetOfStates &Q = elsFSM->getStates();
    for (const auto &[key, q] : Q) {
        // get the number of initial tokens
        auto *s = dynamic_cast<ELSState *>(&(*q));
        ELSEdgeRef e = dynamic_cast<ELSEdgeRef>(*s->getOutgoingEdges().begin());
        unsigned int nrTokens = (*scenarioMatrices)[e->getLabel()]->getCols();
        for (unsigned int k = 0; k < nrTokens; k++) {
            // create a state for (q, k)
            auto *qq = dynamic_cast<ELSState *>(&(*q));
            CId qId = qq->getLabel();
            fsm->addState({qId, static_cast<int>(k)});
        }
    }

    // add the edges. For every edge (q1, q2) in the original fsm,
    // let s be the scenario of q2 and let M_s be the matrix of s
    // For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
    // to (q2,m) labelled with d.

    // for every state of the fsm...
    for (const auto &[key, qq] : Q) {
        auto *q1 = dynamic_cast<ELSState *>(&(*qq));

        CId q1Id = q1->getLabel();

        // for every outgoing edge of the state
        const auto &t = static_cast<const ELSSetOfEdgeRefs &>(q1->getOutgoingEdges());
        for (const auto *e : t) {
            const auto *tr = dynamic_cast<const ELSEdgeRef >(&(*e));
            CId q2Id = (dynamic_cast<ELSStateRef>(tr->getDestination()))->getLabel();
            CString sc = tr->getLabel();
            auto* Ms = (*scenarioMatrices)[sc].get();
            CDouble rew = g.getScenarioRef(sc)->getReward();

            // for every entry in the matrix
            for (unsigned int r = 0; r < Ms->getRows(); r++) {
                for (unsigned int c = 0; c < Ms->getCols(); c++) {
                    MPDelay d = Ms->get(r, c);
                    if (!MP_IS_MINUS_INFINITY(d)) {
                        MPARStateRef src =fsm->getStateLabeled({q1Id, static_cast<int>(c)});
                        MPARStateRef dst = fsm->getStateLabeled({q2Id, static_cast<int>(r)});
                        fsm->addEdge(*src, {d, sc.c_str(), rew}, *dst);
                    }
                }
            }
        }
    }

    // Calculate MCR of the FSM
    MPARCycle critCycle;
    double mcr = calculateMCRAndCycle(fsm.get(), &critCycle);
    th = 1.0 / mcr;

    auto scenarios = std::make_unique<ScenarioConstRefList>();
    for (const auto *i : critCycle) {
        const auto *e = dynamic_cast<const MPAREdge *>(i);
        scenarios->push_back(g.getScenarioRef(e->getLabel().mode));
    }

    return scenarios;
}

/**
 * Compute the throughput of a weakly consistent FSM-based SADF graph through a
 * max-plus automaton.
 */
Throughput ThroughputCalculator::calculateThroughputWeaklyConsistent(const Graph& g) {

    Throughput th = 0.0;
    std::unique_ptr<ScenarioConstRefList> sl = throughputAndCriticalScenarioSequenceWeaklyConsistent(g, th);

    return th;
}

/**
 * Compute the throughput of a weakly consistent FSM-based SADF graph through a
 * max-plus automaton, where the FSM is represented by a regular expression.
 */
Throughput ThroughputCalculator::calculateThroughputRegex(const Graph& g) {
#ifdef _RTM
    CTimer timer;
#endif //_RTM

    Throughput th = 0.0;

    // get scenario matrices and their rewards from the graph
    std::unique_ptr<ScenarioMatrices> scenarioMatrices = nullptr;

#ifdef _RTM
    startTimer(&timer);
#endif //_RTM

    scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenariosForWeakFSMSADF(g);
    std::unique_ptr<ScenarioRewards> scenarioRewards = GraphConversion::getRewardsFromScenarios(g);

#ifdef _RTM
    // Measure execution time
    stopTimer(&timer);
    std::cout << "scenario matrices computation time : ";
    printTimer(std::cout, &timer);
    std::cout << std::endl;
#endif //_RTM

    // A parser to be constructed each time a regular expression needs to be parsed.
    RegExParserThr parser(g.getRegEx(), *scenarioMatrices, *scenarioRewards);

    // parse the regular expression
    auto regEx = std::unique_ptr<RegEx>(parser.parse());

#ifdef _PRINT_REGEX
    std::cout << "the entered regular expression interpreted in the following way:" << std::endl;
    regEx->print();
    std::cout << std::endl;
#endif // _PRINT_REGEX

    // cast to a regular expression type designed for throughput analysis
    std::unique_ptr<RegExThr> regExThr = std::unique_ptr<RegExThr>(dynamic_cast<RegExThr*>(regEx.release()));

    // construct the compact regex
    auto compactRegex = std::unique_ptr<RegExThr>(regExThr->computeCompactRegExThr());

#ifdef _PRINT_REGEX
    // prints the compact regular expression
    std::cout << "the compact regular expression is:" << std::endl;
    compactRegex->print();
    std::cout << std::endl;
#endif // _PRINT_REGEX

#ifdef _No_REGEX_COMPRESSION
    // use the uncompressed regex
    compactRegex = regExThr;
#endif // _No_REGEX_COMPRESSION

    // collect the required information from the compact regex to do throughput analysis
    ThroughputAnalysisInformation t;
    compactRegex->collectInformation(t);
    ScenarioMatrices &compactScenarioMatrices = t.getScenarioMatrices();
    ScenarioRewards &compactScenarioRewards = t.getScenarioRewards();

#ifdef _PRINT_REGEX
    // print compact scenarios
    std::map<CString, CString> subExpressions = t.getsubExpressions();
    for (std::map<CString, CString>::iterator subExpIter = subExpressions.begin();
         subExpIter != subExpressions.end();
         subExpIter++) {
        std::cout << subExpIter->first << "    " << subExpIter->second << std::endl;
    }
#endif // _PRINT_REGEX

    // create an interface to build an FSM by composing smaller FSMs
    EdgeLabeledScenarioFSMBuilder fsmBuilder;

    std::unique_ptr<EdgeLabeledScenarioFSM> elsFSM = nullptr;

    // check if the compact regex is deterministic.
    bool deterministic = compactRegex->checkDeterministic();
    if (deterministic) {

#ifdef _RTM
        startTimer(&timer);

#endif //_RTM
       // convert the compact regex directly to epsilon free DFA (in time linear in the length of
       // the expression)
        compactRegex->convertToDFA(fsmBuilder);

#ifdef _RTM
        // Measure execution time
        stopTimer(&timer);
        std::cout << "Regex to DFA conversion: ";
        printTimer(std::cout, &timer);
        std::cout << std::endl;
#endif //_RTM

        // get the fsm from fsmbuilder
        elsFSM = fsmBuilder.takeFSM();
    } else {

#ifdef _RTM
        startTimer(&timer);

#endif //_RTM
       // convert the compact regex to NFA-e (in time linear in the length of the expression)
        compactRegex->convertToFSM(fsmBuilder);

#ifdef _RTM
        // Measure execution time
        stopTimer(&timer);
        std::cout << "Regex to NFA-e conversion: ";
        printTimer(std::cout, &timer);
        std::cout << std::endl;
#endif //_RTM

        // get the fsm from fsmbuilder
        std::unique_ptr<EdgeLabeledScenarioFSM> elsFSMe = fsmBuilder.takeFSM();

#ifdef _RTM
        startTimer(&timer);

#endif //_RTM

        // convert to epsilon-free FSM (in time quadratic in the length of the expression)
        std::unique_ptr<EdgeLabeledScenarioFSM> elsFSMRef = FSMConverter::convertNFAeToNFA(*elsFSMe);

#ifdef _RTM
        // Measure execution time
        stopTimer(&timer);
        std::cout << "NFA-e to NFA conversion time: ";
        printTimer(std::cout, &timer);
        std::cout << std::endl;
#endif //_RTM

#ifdef _RTM
        startTimer(&timer);
#endif //_RTM

        // remove possible dangling states from the FSM  (in time quadratic in the length of the
        // expression)
        elsFSMRef->removeDanglingStates();

#ifdef _RTM
        // Measure execution time
        stopTimer(&timer);
        std::cout << "pruning time: ";
        printTimer(std::cout, &timer);
        std::cout << std::endl;
#endif //_RTM

        elsFSM = std::move(elsFSMRef);
    }

    auto fsm = std::make_unique<MaxPlusAutomatonWithRewards>();

    // add state for every initial token and location of the fsm

    // create the FSM states for every pair of a state of the FSMSADF's FSM
    // and an initial token
    const ELSSetOfStates &Q = elsFSM->getStates();
    for (const auto &[key, ss] : Q) {
        auto *s = dynamic_cast<ELSState *>(&(*ss));
        const auto &t = dynamic_cast<const ELSSetOfEdgeRefs &>(s->getOutgoingEdges());

        ELSEdgeRef e = dynamic_cast<ELSEdgeRef>(*(t.begin()));
        unsigned int nrTokens = compactScenarioMatrices[e->getLabel()]->getCols();
        for (unsigned int k = 0; k < nrTokens; k++) {
            // create a state for (q, k)
            CId qId = s->getLabel();
            fsm->addState({qId, static_cast<int>(k)});
        }
    }

    // add the edges. For every edge (q1, q2) in the original fsm,
    // let s be the scenario of q2 and let M_s be the matrix of s
    // For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
    // to (q2,m) labelled with d.

    // for every state of the fsm...
    for (const auto &[key, qq] : Q) {
        auto *q1 = dynamic_cast<ELSState *>(&(*qq));
        CId q1Id = q1->getLabel();

        // for every outgoing edge of the state
        const auto &t = dynamic_cast<const ELSSetOfEdgeRefs &>(q1->getOutgoingEdges());

        for (const auto *e : t) {
            const auto *tr = dynamic_cast<ELSEdgeRef>(e);
            CId q2Id = (dynamic_cast<ELSStateRef>(tr->getDestination()))->getLabel();
            CString sc = tr->getLabel();
            auto* Ms = compactScenarioMatrices[sc].get();
            // CDouble rew = g->getScenario(*sc)->getReward();
            CDouble rew = compactScenarioRewards[sc];

            // for every entry in the matrix
            for (unsigned int r = 0; r < Ms->getRows(); r++) {
                for (unsigned int c = 0; c < Ms->getCols(); c++) {
                    MPDelay d = Ms->get(r, c);
                    if (!MP_IS_MINUS_INFINITY(d)) {
                        MPARStateRef src = fsm->getStateLabeled({q1Id, static_cast<int>(c)});
                        MPARStateRef dst = fsm->getStateLabeled({q2Id, static_cast<int>(r)});
                        fsm->addEdge(*src, {d, sc.c_str(), rew}, *dst);
                    }
                }
            }
        }
    }

    // calculate MCR of the FSM
    MPARCycle critCycle;
    double mcr = ThroughputCalculator::calculateMCRAndCycle(fsm.get(), &critCycle);
    // double mcr = fsm->calculateMCRAndCycle(&critCycle);
    th = 1.0 / mcr;

    return th;
}

/**
 * Compute a critical scenario sequence of a weakly consistent FSM-based SADF graph through a
 * max-plus automaton.
 */
std::unique_ptr<ScenarioConstRefList> ThroughputCalculator::criticalScenarioSequenceWeaklyConsistent(const Graph& g) {
    Throughput th = 0.0;
    return FSMSADF::MPExploreMaxPlusAutomaton::ThroughputCalculator::throughputAndCriticalScenarioSequenceWeaklyConsistent(g, th);
}

/**
 * Compute the MCM of the maxplus automaton.
 */
MPDelay ThroughputCalculator::calculateMCM(MaxPlusAutomaton *mpa) {

    // Create a new MCM graph
    auto mcmGraph = std::make_unique<MCMgraph>();

    // Generate ids by counting
    CId id = 0;

    // associate states and nodes in the mcm graph
    std::map<const MPAState *, MCMnode *> stateMap;

    // Create the nodes of the MCM graph
    const MPASetOfStates &states = mpa->getStates();
    for (const auto& [key, ss]: states) {

        auto* s = dynamic_cast<MPAState*>(&(*ss));

        // Create an MCM node for this state
        MCMnode *n = mcmGraph->addNode(id, true);

        // Store node of this state
        stateMap[s] = n;
        id++;
    }

    // Add edges to the MCM graph
    // for all states of the fsm...
    CId edgeId = 0;
    for (const auto& [key, ss]: states) {
        auto* s = dynamic_cast<MPAState*>(&(*ss));
        MCMnode *srcNode = stateMap[s];
        auto edges = static_cast<const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs &>(s->getOutgoingEdges());

        // ...iterate over the outgoing edges
        for (const auto *j : edges) {
            const auto *edg = dynamic_cast<MPAEdgeRef>(j);
            const auto* ds = dynamic_cast<MPAStateRef>(edg->getDestination());
            // Create an edge in the MCM graph for this state transition
            MCMnode *src = srcNode;
            MCMnode *dst = stateMap[ds];
            auto w = static_cast<CDouble>(edg->getLabel().delay);
            CDouble d = 1;

            // Add the edge to the MCM graph and the src and dst node
            mcmGraph->addEdge(edgeId++, *src, *dst, w, d, true);
        }
    }

    std::unique_ptr<MCMgraph> pruned = mcmGraph->pruneEdges();

    double mcm = pruned->calculateMaximumCycleMeanKarp();
    return static_cast<MPDelay>(mcm);
}

/**
 * Compute the MCR of the max-plus automaton with rewards.
 */
CDouble ThroughputCalculator::calculateMCR(MaxPlusAutomatonWithRewards *mpa) {
    return calculateMCRAndCycle(mpa, nullptr);
}

/**
 * Compute the MCR of the max-plus automaton with rewards and return a critical cycle.
 */
CDouble ThroughputCalculator::calculateMCRAndCycle(MaxPlusAutomatonWithRewards *mpa,
                                                   MPARCycle *cycle) {

    // Create a new MCM graph
    auto mcmGraph = std::make_unique<MCMgraph>();

    // Generate ids by counting
    CId id = 0;

    // associate states and ids in the mcm graph
    std::map<MPARStateRef, MCMnode *> stateMap;
    std::map<const MCMedge *, MPAREdgeRef> revEdgeMap;

    // Create the nodes of the MCM graph
    MPARSetOfStates::const_iterator i;
    const MPARSetOfStates &states = mpa->getStates();
    for (const auto &[key, ss] : states) {
        auto *s = dynamic_cast<MPARState *>(&(*ss));

        // Create an MCM node for this state
        MCMnode *n = mcmGraph->addNode(id, true);

        // Store id of this state
        stateMap[s] = n;
        id++;
    }

    // Add edges to the MCM graph
    // for all states of the fsm...
    CId edgeId = 0;
    for (const auto &[key, ss] : states) {
        auto *s = dynamic_cast<MPARState *>(&(*ss));
        MCMnode *srcNode = stateMap[s];
        const auto &x = (s->getOutgoingEdges());
        const auto &edges = dynamic_cast<const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs &>(s->getOutgoingEdges());

        // ...iterate over the outgoing edges
        for (const auto *ee : edges) {
            const auto *edg = dynamic_cast<MPAREdgeRef>(ee);
            const auto *ds = dynamic_cast<MPARStateRef>(edg->getDestination());
            // Create an edge in the MCM graph for this state transition
            MCMnode *src = srcNode;
            MCMnode *dst = stateMap[ds];
            auto w = static_cast<CDouble>(edg->getLabel().delay);
            CDouble d = edg->getLabel().reward;

            // Add the edge to the MCM graph and the src and dst node
            MCMedge *e = mcmGraph->addEdge(edgeId++, *src, *dst, w, d, true);
            revEdgeMap[e] = edg;
        }
    }

    // prune the graph
    std::unique_ptr<MCMgraph> pruned = mcmGraph->pruneEdges();

    CDouble mcr = 0.0;

    // do we need the critical cycle?
    if (cycle != nullptr) {
        // compute MRC and cycle
        std::vector<const MCMedge *> mcmCritCycle;
        mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(*pruned, &mcmCritCycle);
        // translate mcmgraph cycle back to MPA cycle
        for (auto & i : mcmCritCycle) {
            (*cycle).push_back(revEdgeMap[mcmGraph->getEdge(i->id)]);
        }
    } else {
        mcr = maxCycleRatioYoungTarjanOrlin(*pruned);
    }

    // return the result
    return mcr;
}

/**
 * Compute the MCR of the maxplus automaton with rewards and return a critical cycle.
 */
CDouble MaxPlusAutomatonWithRewardsAndCriticalBuffers::calculateMCRAndCycle(MPARCycleB *cycle) {
    // Create a new MCM graph
    auto mcmGraph = std::make_unique<MCMgraph>();

    // Generate ids by counting
    CId id = 0;

    // associate states and ids in the mcm graph
    std::map<MPARStateBRef, MCMnode *> stateMap;
    std::map<MCMedge *, MPAREdgeBRef> revEdgeMap;

    // Create the nodes of the MCM graph
    const MPARSetOfStatesB &states = this->getStates();
    for (const auto& [key, ss] : states) {
        auto* s = dynamic_cast<MPARStateB *>(&(*ss));

        // Create an MCM node for this state
        MCMnode *n = mcmGraph->addNode(id, true);

        // Store id of this state
        stateMap[s] = n;
        id++;
    }

    // Add edges to the MCM graph
    // for all states of the fsm...
    CId edgeId = 0;
    for (const auto& [key, ss]: states) {
        auto* s = dynamic_cast<MPARStateB *>(&(*ss));
        MCMnode *srcNode = stateMap[s];
        const auto &edges = dynamic_cast<const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs &>(s->getOutgoingEdges());

        // ...iterate over the outgoing edges
        for (const auto* ee: edges) {
            const auto *edg = dynamic_cast<MPAREdgeBRef>(ee);
            const auto *ds = dynamic_cast<MPARStateBRef>(edg->getDestination());
            // Create an edge in the MCM graph for this state transition
            MCMnode *src = srcNode;
            MCMnode *dst = stateMap[ds];
            auto w = static_cast<CDouble>(edg->getLabel().delay);
            CDouble d = edg->getLabel().reward;

            // Add the edge to the MCM graph and the src and dst node
            MCMedge *e = mcmGraph->addEdge(edgeId++, *src, *dst, w, d, true);
            revEdgeMap[e] = edg;
        }
    }

    // prune the graph
    std::unique_ptr<MCMgraph> pruned = mcmGraph->pruneEdges();

    CDouble mcr = 0.0;

    // do we need the critical cycle?
    if (cycle != nullptr) {
        // compute MRC and cycle
        std::vector<const MCMedge *> mcmCritCycle;
        mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(*pruned, &mcmCritCycle);
        // translate mcmgraph cycle back to MPA cycle
        for (auto & i : mcmCritCycle) {
            (*cycle).push_back(revEdgeMap[mcmGraph->getEdge(i->id)]);
        }
    } else {
        mcr = maxCycleRatioYoungTarjanOrlin(*pruned);
    }

    // return the result
    return mcr;
}

} // namespace MPExploreMaxPlusAutomaton

} // End namespace FSMSADF
