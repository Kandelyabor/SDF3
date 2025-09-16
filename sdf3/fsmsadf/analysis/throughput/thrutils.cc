/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   thr_util.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 6, 2010
 *
 *  Function        :   definitions shared for different throughput
 *                      algorithms.
 *
 *  History         :
 *      06-05-10    :   Initial version.
 *
 * $Id: thrutils.cc,v 1.1.2.5 2010-08-18 09:01:01 mgeilen Exp $
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

#include "thrutils.h"
#include "fsmsadf/base/graph.h"
#include "sdf/analysis/maxplus/mpexplore.h"
#include <array>
#include <memory>
#include <utility>

namespace FSMSADF {

using namespace SDF;
using namespace SDF::MaxPlusAnalysis;
using namespace MaxPlus;

// Get the maxplus matrices for each of the scenarios.
// The initial tokens are ordered
std::unique_ptr<ScenarioMatrices> GraphConversion::computeMaxPlusMatricesFromScenarios(const Graph& g) {
    auto sm = std::make_unique<ScenarioMatrices>();
    const Scenarios& scenarios = g.getConstScenarios();
    CStrings persistentTokens = g.getPersistentTokens();

    for (const auto& i : scenarios) {
        PersistentTokenIndexMap mp = i->getScenarioGraph()->getPersistentTokenIndexMap();
        std::list<uint> tokenIndices;
        CStrings::const_iterator ti;
        for (ti = persistentTokens.begin(); ti != persistentTokens.end(); ti++) {
            tokenIndices.push_back(mp[*ti]);
        }

        auto sg = std::unique_ptr<TimedSDFgraph>(FSMSADF::Graph::extractSDFgraph(*i));
        auto G = std::make_unique<GraphDecoration::Graph>(*sg, false, nullptr);
        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);
        mpe.g = sg.get();
        auto matrix = std::unique_ptr<MaxPlus::Matrix>(mpe.convertToMaxPlusMatrix());
        // create the sub matrix of only the persistent tokens.
        std::unique_ptr<Matrix> sMatrix = matrix->getSubMatrixPtr(tokenIndices);
        (*sm)[i->getName()] = std::move(sMatrix);
    }
    return sm;
}

// Get the rewards for each of the scenarios.
std::unique_ptr<ScenarioRewards> GraphConversion::getRewardsFromScenarios(const Graph& g) {
    auto sr = std::make_unique<ScenarioRewards>();
    const Scenarios& scenarios = g.getConstScenarios();

    for (const auto& sc : scenarios) {
        (*sr)[sc->getName()] = sc->getReward();
    }
    return sr;
}

// Get the maxplus matrices, tracing matrices and the order in which actors are fired for each of
// the scenarios.
std::unique_ptr<ScenarioTracinginfo> GraphConversion::computeTracinginfoFromScenarios(const Graph& g) {
    auto sti = std::make_unique<ScenarioTracinginfo>();
    const Scenarios& scenarios = g.getConstScenarios();
    CStrings persistentTokens = g.getPersistentTokens();

    for (const auto& i : scenarios) {
        PersistentTokenIndexMap mp = i->getScenarioGraph()->getPersistentTokenIndexMap();
        std::list<uint> tokenIndices;
        CStrings::const_iterator ti;
        for (ti = persistentTokens.begin(); ti != persistentTokens.end(); ti++) {
            tokenIndices.push_back(mp[*ti]);
        }
        auto sg = std::unique_ptr<TimedSDFgraph>(FSMSADF::Graph::extractSDFgraph(*i));
        auto G = std::make_unique<GraphDecoration::Graph>(*sg, false, nullptr);
        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);
        mpe.g = sg.get();
        std::unique_ptr<std::array<std::unique_ptr<MaxPlus::Matrix>, 2>> matrix = mpe.convertToTracingInfo();
        std::unique_ptr<MaxPlus::Matrix> gMatrix = std::move((*matrix)[0]);
        std::unique_ptr<MaxPlus::Matrix> hMatrix = std::move((*matrix)[1]);
        auto tracinginfo = std::make_unique<Tracinginfo>();
        // create the sub matrix of only the persistent tokens.
        auto sMatrix = std::make_unique<MaxPlus::Matrix>(gMatrix->getSubMatrix(tokenIndices));
        auto tMatrix = std::make_unique<MaxPlus::Matrix>(hMatrix->getSubMatrixNonSquare(tokenIndices));
        tracinginfo->create(std::move(sMatrix), std::move(tMatrix), std::move(mpe.actorsFiringOrder));
        (*sti)[i->getName()] = std::move(tracinginfo);
    }
    return sti;
}

// Get the maxplus matrices, tracing matrices and the order in which actors are fired for each of
// the scenarios when the graph is a weakly consistent fsmsadf
std::unique_ptr<ScenarioTracinginfo> GraphConversion::computeTracinginfoFromScenariosWeakConsistency(const Graph& g) {

    auto sti = std::make_unique<ScenarioTracinginfo>();

    const Scenarios& scenarios = g.getConstScenarios();

    for (const auto& sc : scenarios) {
        CStrings persistentTokens;
        CStrings persistentTokensFinal;
        PersistentTokenIndexMap mp = sc->getScenarioGraph()->getPersistentTokenIndexMap();
        PersistentTokenIndexMap mpf = sc->getScenarioGraph()->getPersistentTokenIndexMapFinal();

        // collect names of the initial and final persistent tokens
        PersistentTokenIndexMap::const_iterator ptmi;
        for (ptmi = mp.begin(); ptmi != mp.end(); ptmi++) {
            persistentTokens.push_back((*ptmi).first);
        }
        for (ptmi = mpf.begin(); ptmi != mpf.end(); ptmi++) {
            persistentTokensFinal.push_back((*ptmi).first);
        }

        // sort them to get a canonical order
        persistentTokens.sort();
        persistentTokensFinal.sort();

        std::list<uint> tokenIndicesInitial;
        CStrings::const_iterator ti;
        for (ti = persistentTokens.begin(); ti != persistentTokens.end(); ti++) {
            tokenIndicesInitial.push_back(mp[*ti]);
        }
        std::list<uint> tokenIndicesFinal;
        for (ti = persistentTokensFinal.begin(); ti != persistentTokensFinal.end(); ti++) {
            tokenIndicesFinal.push_back(mpf[*ti]);
        }

        auto sg = std::unique_ptr<TimedSDFgraph>(FSMSADF::Graph::extractSDFgraph(*sc));
        std::unique_ptr<RepetitionVector> partialRepVec = FSMSADF::Graph::getPartialRepetitionVector(*sc);
        auto G = std::make_unique<GraphDecoration::Graph>(*sg, true, partialRepVec.get());
        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);
        mpe.g = sg.get();

        std::unique_ptr<std::array<std::unique_ptr<MaxPlus::Matrix>, 2>> matrix = mpe.convertToTracingInfoWeakConsistency();
        std::unique_ptr<MaxPlus::Matrix> gMatrix = std::move((*matrix)[0]);
        std::unique_ptr<MaxPlus::Matrix> hMatrix = std::move((*matrix)[1]);
        auto tracinginfo = std::make_unique<Tracinginfo>();
        // create the sub matrix of only the persistent tokens.
        auto sMatrix =
                std::make_unique<MaxPlus::Matrix>(gMatrix->getSubMatrix(tokenIndicesFinal, tokenIndicesInitial));
        auto tMatrix = std::make_unique<MaxPlus::Matrix>(hMatrix->getSubMatrixNonSquare(tokenIndicesInitial));
        tracinginfo->create(std::move(sMatrix), std::move(tMatrix), std::move(mpe.actorsFiringOrder));
        (*sti)[sc->getName()] = std::move(tracinginfo);
    }

    return sti;
}
// Get the maxplus matrices for each of the scenarios.
std::unique_ptr<ScenarioMatrices> GraphConversion::computeMaxPlusMatricesFromScenariosForWeakFSMSADF(const Graph& g) {
    auto sm = std::make_unique<ScenarioMatrices>();
    const Scenarios& scenarios = g.getConstScenarios();

    for (const auto& sc : scenarios) {
        CStrings persistentTokens;
        CStrings persistentTokensFinal;
        PersistentTokenIndexMap mp = sc->getScenarioGraph()->getPersistentTokenIndexMap();
        PersistentTokenIndexMap mpf = sc->getScenarioGraph()->getPersistentTokenIndexMapFinal();

        // collect names of the initial anf final persistent tokens
        PersistentTokenIndexMap::const_iterator ptm_i;
        for (ptm_i = mp.begin(); ptm_i != mp.end(); ptm_i++) {
            persistentTokens.push_back((*ptm_i).first);
        }
        for (ptm_i = mpf.begin(); ptm_i != mpf.end(); ptm_i++) {
            persistentTokensFinal.push_back((*ptm_i).first);
        }

        // sort them to get a canonical order
        persistentTokens.sort();
        persistentTokensFinal.sort();

        std::list<uint> tokenIndicesInitial;
        CStrings::const_iterator ti;
        for (ti = persistentTokens.begin(); ti != persistentTokens.end(); ti++) {
            tokenIndicesInitial.push_back(mp[*ti]);
        }
        std::list<uint> tokenIndicesFinal;
        for (ti = persistentTokensFinal.begin(); ti != persistentTokensFinal.end(); ti++) {
            tokenIndicesFinal.push_back(mpf[*ti]);
        }

        auto sg = std::unique_ptr<TimedSDFgraph>(FSMSADF::Graph::extractSDFgraph(*sc));
        std::unique_ptr<RepetitionVector> partialRepVec = FSMSADF::Graph::getPartialRepetitionVector(*sc);
        auto G = std::make_unique<GraphDecoration::Graph>(*sg, true, partialRepVec.get());
        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);
        mpe.g = sg.get();
        std::unique_ptr<Matrix> matrix = nullptr;
        try {
            matrix = mpe.convertToMaxPlusMatrixForWeakFSMSADF();
        } catch (CDeadlockException &) {
            throw CScenarioDeadlockException("Cannot compute matrix for scenario " + sc->getName()
                                                     + ", because it deadlocks.",
                                             sc.get());
        }
        // create the sub matrix of only the persistent tokens.
        auto sMatrix = matrix->getSubMatrixPtr(tokenIndicesFinal, tokenIndicesInitial);
        (*sm)[sc->getName()] = std::move(sMatrix);
    }
    return sm;
}

// ensure the FSM SADF is strongly bounded by introducing a dummy
// actor with a self loop, so that sub collections of initial tokens
// cannot develop independently on particular scenario sequences
std::unique_ptr<Graph> GraphConversion::ensureStronglyBounded(const Graph& g) {
    std::unique_ptr<Graph> gt = nullptr;
    // Create a copy of the graph
    gt = g.clone(GraphComponent(g.getParent(), g.getId(), g.getName()));

    // Add dummy actor with self-edge to each scenario graph
    for (const auto& sg : gt->getScenarioGraphs()) {
        // Use first channel in the graph without initial tokens
        Channel* cf = nullptr;
        for (const auto& c : sg->getChannels()) {
            if (c->getInitialTokens() == 0) {
                cf = c.get();
                break;
            }
        }

        // Create dummy actor
        ActorRef a = sg->createActor();
        a->setName("sync-actor");
        a->setExecutionTimeOfScenario(*(gt->getDefaultScenario()), "sync", 0);
        a->setDefaultProcessorType("sync");

        // Add self-edge to the dummy actor
        ChannelRef c = sg->createChannel(a, a);
        c->setName("sync-selfedge");
        c->getSrcPort()->setRateOfScenario(*(gt->getDefaultScenario()), 1);
        c->getDstPort()->setRateOfScenario(*(gt->getDefaultScenario()), 1);
        c->setInitialAndFinalTokens(1);
        c->setPersistentAndFinalTokenNames("sync-selfedge-persistent-token");

        // Attach dummy actor to the scenario graph
        ChannelRef ca = sg->createChannel(cf->getSrcActor(), a);
        ca->setName("sync-src");
        ca->getSrcPort()->setRate(cf->getSrcPort()->getRate());
        ca->getDstPort()->setRateOfScenario(*(gt->getDefaultScenario()), 1);
        ca->setInitialAndFinalTokens(0);

        ChannelRef cb = sg->createChannel(a, cf->getDstActor());
        cb->setName("sync-dst");
        cb->getSrcPort()->setRateOfScenario(*(gt->getDefaultScenario()), 1);
        cb->getDstPort()->setRate(cf->getDstPort()->getRate());
        cb->setInitialAndFinalTokens(cf->getInitialTokens());
        for (uint j = 0; j < cb->getInitialTokens(); j++) {
            cb->addPersistentTokenName("sync-dst-persistent-" + CString(j));
        }
    }
    return gt;
}

/**
 * Create a new state machine where edges are labelled instead of states
 */
std::unique_ptr<EdgeLabeledScenarioFSM> FSMConverter::convertFSMToEdgeLabeled(const FSMSADF::FSM& fsm) {
    auto result = std::make_unique<EdgeLabeledScenarioFSM>();
    const FSMstates &states = fsm.getConstStates();

    // remember which state maps to which new state
    std::map<FSMstate*, ELSStateRef> stateMap;

    // create the states
    CId maxId = 0;
    FSMstates::const_iterator i;
    for (i = states.begin(); i != states.end(); i++) {
        FSMstate *q = (*i).get();
        ELSStateRef nq = result->addState(q->getId());
        stateMap[q] = nq;
        maxId = MAX(maxId, q->getId());
    }

    // create the edges
    for (i = states.begin(); i != states.end(); i++) {
        FSMstate *q = (*i).get();
        const FSMtransitionRefs &tr = q->getTransitions();
        for (auto j = tr.begin(); j != tr.end(); j++) {
            FSMtransition *t = *j;
            result->addEdge(*(stateMap[q]),
                            t->getDstState()->getScenario()->getName(),
                            *(stateMap[t->getDstState()]));
        }
    }

    // set the initial state.
    // note that our scenario fsm have the peculiar semantics that the first
    // scenario is encountered after the first transition an not in the initial state
    result->setInitialState(*(stateMap[fsm.getInitialState()]));
    return result;
}

// create a minimal edge labelled scenario automaton
std::unique_ptr<EdgeLabeledScenarioFSM> FSMConverter::minimalEdgeLabeledFSM(const FSM& fsm) {

    // step 1: convert to edge labeled automaton
    std::unique_ptr<EdgeLabeledScenarioFSM> elsFSMtemp = FSMConverter::convertFSMToEdgeLabeled(fsm);

    // optionally step2: determinize
    // Not clear whether that is a good idea in general.
    // Does not gain anything for the benchmark graphs, opportunistically remove :-)
    // EdgeLabeledScenarioFSM* detElsFSM = (EdgeLabeledScenarioFSM*)
    // elsFSMtemp->determinizeEdgeLabels();

    // step3: minimize, ignoring state labels
    auto melTemp = elsFSMtemp->minimizeEdgeLabels(true);
    auto elsFSM = std::unique_ptr<EdgeLabeledScenarioFSM>(dynamic_cast<EdgeLabeledScenarioFSM*>(melTemp.release()));

    return elsFSM;
}

// create an FSM with edges labelled with a matrix and a reward where
// the matrix and reward correspond to the scenario of
// the edges on the original graph between the states.
// If combineEdges is true (default), then create a
// matrix-labelled FSM with at most one edge per pair of states and reward value where
// the matrix represents the max of the matrices corresponding to scenario of
// all edges on the original graph between the states with that reward.

std::unique_ptr<MatrixLabeledScenarioFSM> FSMConverter::convertToMatrixLabeledScenarioFSM(
        const ::FSMSADF::EdgeLabeledScenarioFSM& elsFsm,
        ScenarioMatrices &sm,
        ScenarioRewards &sr,
        bool combineEdges) {

    std::unique_ptr<MatrixLabeledScenarioFSM> result = std::make_unique<MatrixLabeledScenarioFSM>();

    // Add states
    std::map<ELSStateRef, MLSStateRef> stateMap;
    ELSSetOfStates::const_iterator i;
    for (i = elsFsm.getStates().begin(); i != elsFsm.getStates().end(); i++) {
        auto* s = dynamic_cast<ELSState*>((*i).second.get());
        // create new state
        MLSStateRef ns = result->addState(s->stateLabel);
        stateMap[&(*s)] = ns;
    }

    // Add initialState
    result->setInitialState(*(stateMap[elsFsm.getInitialState()]));

    // Add edges to the new FSM
    std::map<std::pair<ELSStateRef, ELSStateRef>, std::map<CDouble, MLSEdge *>>
            stateEdges;
    for (auto i = elsFsm.getEdges().begin();
         i != elsFsm.getEdges().end();
         i++) {
        std::map<CDouble, MLSEdge *> *edges = nullptr;
        auto* e = dynamic_cast<ELSEdge*>((*i).second.get());

        //		note: combining edges only allowed for equal reward now!
        // find out if a new edge needs to be created
        bool newEdge = !combineEdges;

        // if there are no edges yet, create it
        if ((!combineEdges)
            || (stateEdges.find(make_pair(dynamic_cast<ELSStateRef>(e->getSource()),
                                          dynamic_cast<ELSStateRef>(e->getDestination())))
                == stateEdges.end())) {
            newEdge = true;
        } else {
            // check if an edge with the same reward already exists
            edges = &stateEdges[make_pair(dynamic_cast<ELSStateRef>(e->getSource()),
                                          dynamic_cast<ELSStateRef>(e->getDestination()))];
            if (edges->find(sr[e->getLabel()]) == edges->end()) {
                newEdge = true;
            }
        }
        if (newEdge) {
            // Add the edge to the MCM graph and the src and dst node
            MLSEdgeLabel lb;
            lb.mat = std::move((sm[e->getLabel()])->createCopyPtr());
            lb.rew = sr[e->getLabel()];
            MLSEdge *me = result->addEdge(*(stateMap[dynamic_cast<ELSStateRef>(e->getSource())]),
                                          std::move(lb),
                                          *(stateMap[dynamic_cast<ELSStateRef>(e->getDestination())]));
            std::map<CDouble, MLSEdge *> edgeMap;
            edgeMap[lb.rew] = me;
            stateEdges[make_pair(dynamic_cast<ELSStateRef>(e->getSource()),
                                 dynamic_cast<ELSStateRef>(e->getDestination()))] = edgeMap;
        } else {
            // else update the edge
            edges = &stateEdges[make_pair(dynamic_cast<ELSStateRef>(e->getSource()),
                                          dynamic_cast<ELSStateRef>(e->getDestination()))];
            MLSEdge *me = (*edges)[sr[e->getLabel()]];
            auto resMat = std::make_unique<MaxPlus::Matrix>(static_cast<unsigned int>(sm.vectorSize()));
            me->getLabel().mat->maximum(*(sm[e->getLabel()]), *resMat);
            MLSEdgeLabel l;
            const auto& mel = me->getLabel();
            l.rew = mel.rew; 
            l.mat = std::move(resMat);
            me->setLabel(std::move(l));
        }
    }
    return result;
}

// create a matrix-labelled FSM where
// the matrix corresponds to the scenario of
// the edges on the original graph between the states.
std::unique_ptr<MatrixLabeledScenarioFSM> FSMConverter::convertToFullMatrixLabeledScenarioFSM(
        const EdgeLabeledScenarioFSM& elsFsm, ScenarioMatrices &sm, ScenarioRewards &sr) {
    return FSMConverter::convertToMatrixLabeledScenarioFSM(elsFsm, sm, sr, false);
}

using FSMConverterEdge = struct edge {
    ELSStateRef src;
    CString label;
    ELSStateRef dst;
};

// NOLINTNEXTLINE (*internal-linkage)
bool operator<(const FSMConverterEdge &e1, const FSMConverterEdge &e2) {
    if (e1.src < e2.src) {
        return true;
    }
    if (e1.src > e2.src) {
        return false;
    }
    if (e1.label < e2.label) {
        return true;
    }
    if (e1.label > e2.label) {
        return false;
    }
    if (e1.dst < e2.dst) {
        return true;
    }
    if (e1.dst > e2.dst) {
        return false;
    }
    return false;
};

/*converts non-deterministic finite automaton with epsilon
transitions to non-deterministic finite automaton without epsilon transitions. The conversion
returns an NFA with the same number of states as the given NFA-e.
Note that EdgeLabeledScenarioFSM class does not consider final states for FSMs,
therefore the conversion ignores finial states conversion. Throughout the algorithm, an NFA edge is
denoted by a tuple (q1,label,q2).*/
std::unique_ptr<EdgeLabeledScenarioFSM> FSMConverter::convertNFAeToNFA(const EdgeLabeledScenarioFSM &nfae) { // NOLINT(*cognitive-complexity)

    // create epsilon-free NFA nfa
    auto nfa = std::make_unique<EdgeLabeledScenarioFSM>();

    // create and add initial state
    ELSStateRef initialState = nfa->addState(nfae.getInitialState()->getLabel());
    nfa->setInitialState(*initialState);

    // get the initial state of the nfae
    ELSStateRef ini = nfae.getInitialState();

    /*create a temporary set for edges. Every edge in this set will be checked and
    if it is not an epsilon edge, it will be copied to the nfa.*/
    // ELSSetOfEdges tempSet;
    std::set<FSMConverterEdge> tempSet;

    /*Initially put all outgoing edges of the initial state in the temporary set of edges.*/
    const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs &E1 = ini->getOutgoingEdges();

    for (auto ei = E1.begin(); ei != E1.end(); ei++) {
        const auto *e1 = dynamic_cast<ELSEdgeRef>(*ei);

        const auto &q1 = dynamic_cast<ELSStateRef>(e1->getSource());
        const auto &q2 = dynamic_cast<ELSStateRef>(e1->getDestination());
        CString alpha = e1->getLabel();

        FSMConverterEdge newEdge = {q1, alpha, q2};
        tempSet.insert(newEdge);
    }

    // create an empty temporary set for edges. It contains only epsilon edges that are already
    // checked.
    std::set<FSMConverterEdge> tempSet2;

    /*while there are edges to be checked,*/
    while (!tempSet.empty()) {

        // pick e = (q1,alpha,q2) from tempSet
        auto ei = tempSet.begin();
        const FSMConverterEdge& e = *ei;
        tempSet.erase(ei);

        // get its label,source and destination state
        const ELSState &q1 = *(e.src);
        const ELSState &q2 = *(e.dst);
        CString alpha = e.label;

        // if e is not an epsilon edge
        if (alpha != "epsilon") {

            /*the source state already exists in the nfa. Check if the destination state does not
            exist, create and add it to the nfa. */
            ELSStateRef nq = nfa->checkStateLabeled(q2.getLabel());
            if (nq == nullptr) {
                nq = nfa->addState(q2.getLabel());
            }

            /*create and add sq, alpha, nq to the nfa */
            const ELSStateRef sq = nfa->getStateLabeled(q1.getLabel());
            nfa->addEdge(*sq, alpha, *nq);

            // find all destination states that can be reached by epsilon edges from the destination
            // state q2 (epsilon closures of q2)
            std::unique_ptr<::MaxPlus::FSM::Abstract::SetOfStateRefs> Q3 =
                    q2.nextStatesOfEdgeLabel("epsilon");

            /*for all states in Q3, check if there is no alpha labeled edge from
            q1 to q3 already in the given nfae, add it to the tempSet.*/
            for (auto si = Q3->begin(); si != Q3->end(); si++) {
                const auto *q3 = dynamic_cast<ELSStateRef>(*si);

                const ELSEdge *found = nfa->findEdge(q1.getLabel(), alpha, q3->getLabel());
                if (found == nullptr) {
                    // cannot make edges anymore without FSM...
                    FSMConverterEdge newEdge = {&q1, alpha, q3};
                    tempSet.insert(newEdge);
                }
            }

            /*for every non-epsilon outgoing edge of the destination state,
            check if it is not already in the partly constructed nfa, add it to the tempSet.*/
            const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs &E2 = q2.getOutgoingEdges();

            for (auto E2i = E2.begin(); E2i != E2.end(); E2i++) {
                const auto *e2 = dynamic_cast<ELSEdgeRef>(*E2i);
                if (e2->getLabel() != "epsilon") {
                    CString beta = e2->getLabel();
                    const auto &q3 = dynamic_cast<ELSStateRef>(e2->getDestination());
                    const ELSEdge *found = nfa->findEdge(q2.getLabel(), beta, q3->getLabel());
                    if (found == nullptr) {
                        FSMConverterEdge newEdge = {&q2, beta, q3};
                        tempSet.insert(newEdge);
                    }
                }
            }
        }

        // if e is an epsilon edge
        else {
            // add e to tempSet2 to avoid double check
            tempSet2.insert(e);

            /*for every outgoing edge e2 from the destination state q2,
            check if (q1,e2->label,q3) is not already in the partly constructed nfa
            and also not in tempSet2, add it to the tempSet.*/
            const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs &E2 = q2.getOutgoingEdges();
            for (auto E2i = E2.begin(); E2i != E2.end(); E2i++) {
                const auto *e2 = dynamic_cast<const ELSEdge *>(*E2i);

                CString beta = e2->getLabel();
                const auto &q3 = dynamic_cast<ELSStateRef>(e2->getDestination());
                const ELSEdge *found = nfa->findEdge(q1.getLabel(), beta, q3->getLabel());

                auto endIter = tempSet2.end();
                FSMConverterEdge e2e = {dynamic_cast<ELSStateRef>(e2->getSource()),
                                        e2->getLabel(),
                                        dynamic_cast<ELSStateRef>(e2->getDestination())};
                if ((found == nullptr) && tempSet2.find(e2e) == endIter) {
                    FSMConverterEdge newEdge = {&q1, beta, q3};
                    tempSet.insert(newEdge);
                }
            }
        }
    }

    return nfa;
}
} // namespace FSMSADF