/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   statespace.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 12, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      state-space traversal.
 *
 *  History         :
 *      12-08-09    :   Initial version.
 *
 * $Id: statespace.cc,v 1.2.2.23 2010-08-18 09:01:00 mgeilen Exp $
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

#include "statespace.h"

#include "fsmsadf/analysis/throughput/statespace.h"
#include "maxplus/base/analysis/mcm/mcmyto.h"
#include "thrutils.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

// #define _DEBUG_

using namespace ::Graphs;
using namespace FSMSADF::MPExploreStateSpace;

namespace FSMSADF {

/**
 * throughputAnalysisGraphUsingStateSpace()
 * Compute the throughput of an FSM-based SADF graph through a state-space
 * traversal.
 */
Throughput throughputAnalysisGraphUsingStateSpace(const Graph& g) {
    MPExploreStateSpace::StateSpaceExploration sse;
    //        Graph *gt;
    Throughput thr = 0;

    // MG removed, because it changes throughput
    //        gt = GraphConversion::ensureStronglyBounded(g);

    // check consistency
    if (!g.isWeaklyConsistent()) {
        throw ::CException("FSM-SADF graph is not consistent");
    }

    // Compute throughput
    thr = FSMSADF::MPExploreStateSpace::StateSpaceExploration::exploreThroughputMax(g);

    return thr;
}

/**
 * traceGenerationUsingStateSpace()
 * Generate traces of actor firings for the specified scenario sequence.
 */
void traceGenerationUsingStateSpace(const Graph& g,
                                    CStrings filenames,
                                    const std::list<CString>& scenarioSequence,
                                    TraceFormat f) {
    MPExploreStateSpace::StateSpaceExploration sse;
    auto firings = std::unique_ptr<TraceOfFirings>(StateSpaceExploration::generateTraces(g, scenarioSequence));

    if (f == Trace4CPS) {
        firings->Trace4CPSTrace(filenames);
    } else {
        firings->xmlTrace(filenames.front());
    }
}

/**
 * traceGenerationUsingStateSpace()
 * Generate traces of actor firings for the specified scenario sequence when the graph is a weakly
 * consistent fsmsadf
 */
void traceGenerationWeaklyConsistent(const Graph& g,
                                     CStrings filenames,
                                     const std::list<CString>& scenarioSequence,
                                     TraceFormat f)

{
    MPExploreStateSpace::StateSpaceExploration sse;
    auto firings = std::unique_ptr<TraceOfFirings>(StateSpaceExploration::generateTracesWeakConsistency(g, scenarioSequence));

    if (f == Trace4CPS) {
        firings->Trace4CPSTrace(filenames);
    } else {
        firings->xmlTrace(filenames.front());
    }
}

/**
 * traceOfVectorsGenerationUsingStateSpace()
 * Generate traces of state vectors for the specified scenario sequence when the graph is a weakly
 * consistent fsmsadf
 */
void traceOfVectorsGenerationWeaklyConsistent(const Graph& g,
                                              CStrings filenames,
                                              const std::list<CString>& scenarioSequence,
                                              TraceFormat f)

{
    MPExploreStateSpace::StateSpaceExploration sse;
    auto vectors = std::unique_ptr<TraceOfVectors>(
        StateSpaceExploration::generateTraceOfVectorsWeakConsistency(g, scenarioSequence));

    if (f == Trace4CPS) {
        vectors->Trace4CPSTrace(filenames);
    } else {
        vectors->xmlTrace(filenames.front());
    }
}

/**
 * scheduleCheckUsingStateSpace()
 * Construct a binding aware graph and check the given schedules
 * against the state-space generated traces.
 */
void scheduleCheckUsingStateSpace(Graph& g,
                                  std::list<CString> &stringSequence,
                                  CNode *architectureNode,
                                  CNode *mappingNode) {

    // Check if all graphs are available
    if (architectureNode == nullptr) {
        throw ::CException("No architectureGraph found.");
    }

    if (mappingNode == nullptr) {
        throw ::CException("No mapping found.");
    }

    // Construct the platform graph
    auto platformGraph = std::make_unique<PlatformGraph>(GraphComponent(nullptr, 0));
    platformGraph->constructFromXML(architectureNode);

    // Create a new platform binding
    PlatformBinding platformBinding(
            GraphComponent(nullptr, 0), platformGraph.get(), &g, BindingMethod(BindingMethod::Method::MaxOfScenarios));

    // Load the mapping
    platformBinding.constructFromXML(mappingNode);

    // Generate the traces
    MPExploreStateSpace::StateSpaceExploration sse;
    auto firings = std::unique_ptr<TraceOfFirings>(StateSpaceExploration::generateTraces(g, stringSequence));

    // Convert the sequence of scenarios from string to scenario pointers
    auto scenarioSequence = std::unique_ptr<ScenarioRefList>(
            convertScenarioSequence(&platformBinding, stringSequence));

    // Get the trace of scheduled actors
    auto trace = std::unique_ptr<Trace>(firings->scheduleTrace());

    // Check the schedule against the trace for each processor
    checkSchedule(*scenarioSequence, &platformBinding, *trace);
}

/**
 * convertScenarioSequence()
 * Convert a sequence of scenarios from strings to scenario pointers
 */
std::unique_ptr<std::list<ScenarioRef>> convertScenarioSequence(PlatformBinding *pb,
                                               std::list<CString> &stringSequence) {
    auto scenarioSequence = std::make_unique<std::list<ScenarioRef>>();

    for (const auto &scenarioName : stringSequence) {
        scenarioSequence->push_back(&(*(pb->getApplicationGraph()->getScenarioRef(scenarioName))));
    }
    return scenarioSequence;
}

/**
 * checkSchedule()
 * Check the schedules given in a binding-aware graph against the given trace.
 */
void checkSchedule(ScenarioRefList& scenarioSequence, // NOLINT(*cognitive-complexity)
                   PlatformBinding *platformBinding,
                   const Trace& trace) {
    TileBindings& tbs = platformBinding->getTileBindings();

    // Iterate over scenario sequence in trace
    for (const auto& traceIter : trace) {
        std::pair<CString, std::list<CString>> *scenarioTrace = traceIter.get();
        ScenarioRef scenario = scenarioSequence.front();

        if (scenarioTrace->first != scenario->getName()) {
            throw ::CException("Scenario trace does not match sequence: expected '"
                             + scenario->getName() + "', got '" + scenarioTrace->first + "'.");
        }
        scenarioSequence.pop_front();

        // Iterate over tile bindings
        for (const auto& tb : tbs) {
            // Iterate over processor bindings
            for (const auto& pb : tb->getProcessorBindings()) {
                // Get sos of this processor, this scenario
                std::map<ScenarioConstRef, StaticOrderSchedule> scheduleMap =
                        pb->getStaticOrderSchedules();
                StaticOrderSchedule sos = scheduleMap[scenario];

                // Create set and list with all actor names that appear in schedule
                std::set<CString> actorSet;
                std::list<CString> actorList;
                for (auto & so : sos) {
                    actorSet.insert(so.actor->getName());
                    actorList.push_back(so.actor->getName());
                }

                // Actual schedule check for each entry
                for (const auto &entry : scenarioTrace->second) {
                    if (actorSet.find(entry) != actorSet.end()) {
                        if (entry != actorList.front()) {
                            throw ::CException("Error: scheduling incorrect. Actor trace in '"
                                             + scenarioTrace->first + "' on tile '"
                                             + tb->getTile()->getName() + "' "
                                             + "does not match static-order schedule. "
                                             + "Expected actor '" + actorList.front() + "', found '"
                                             + entry + "'.");
                            break;
                        }
                        actorList.pop_front();
                    }
                }
                // All actors in schedule fired?
                if (!actorList.empty()) {
                    throw ::CException("Error: scheduling incorrect. Actor trace in '"
                                     + scenarioTrace->first + "' on tile '"
                                     + tb->getTile()->getName() + "' "
                                     + "does not execute entire static-order schedule. " + "Actor '"
                                     + actorList.front() + "' and onwards are not fired.");
                }
            }
        }
    }
}

namespace MPExploreStateSpace {

/**
 * DelaysAndRewards::add
 * add a new pair of delay d and reward r
 */
void DelaysAndRewards::add(MPDelay d, CDouble r) {
    // check if the delay already exists
    if (this->count(d) == 0) {
        (*this)[d] = r;
    } else {
        // keep the minimum reward (the worst case)
        (*this)[d] = (std::min)(r, (*this)[d]);
    }
}

/**
 * DelaysAndRewards::addAndPrune
 * add a new pair of delay d and reward r and prune the dominated combinations (smaller d and larger
 * r)
 */
void DelaysAndRewards::addAndPrune(MPDelay d, CDouble r) {
    // remove all combinations dominated by new one
    for (auto i = this->begin(); i != this->end(); /* increment in loop */) {
        MPDelay dp = (*i).first;
        CDouble rp = (*i).second;

        if (dp <= d && rp >= r) {
            this->erase(i++);
        } else {
            // if the new combination is dominated, then exit
            if (dp >= d && rp <= r) {
                return;
            }
            ++i;
        }
    }
    this->add(d, r);
}

/**
 * State()
 * Constructor.
 */
State::State(State* prevState, const ::MaxPlus::FSM::Abstract::State* newFsmState, const Matrix* m, double r) :
    fsmState(newFsmState), mat(m) {

    // reward
    this->reward = MPTime(r);

    // vector is initially taken from the previous state, to be multiplied by matrix mat
    if (prevState != nullptr) {
        this->mpvector = std::make_unique<MaxPlus::Vector>(*prevState->mpvector);
    }

    // Previous state
    this->previousState = prevState;
}

/**
 * ~State()
 * Destructor
 */
State::~State() = default;

/**
 * hashValue()
 * Compute hash value of the state.
 */
constexpr size_t HashFactor = 31;
size_t State::hashValue() const {
    size_t h = 0;

    // Vector
    for (uint i = 0; i < this->mpvector->getSize(); i++) {
        int dum = 0;
        // attempt to make hash more effective for small execution times <<1.
        // was: h = 31*h + (size_t)this->mpvector->get(i);
        h = HashFactor * h
            + static_cast<size_t>(
                    ldexp(frexp(static_cast<CDouble>(this->mpvector->get(i)), &dum), HashFactor));
    }

    // FSM state
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4311 4302)
// suppress the pointer truncation warning for VS x64 build, we know what we are doing :-)
#endif
h = 31 * h + reinterpret_cast<uintptr_t>(fsmState); // NOLINT(*reinterpret-cast,*magic-numbers)
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    return h;
}

/**
 * equals()
 * Compare the states for equality on all their token timestamps and
 * equality of the FSM state.
 */
bool State::equals(const State *s) const {
    // FSM state
    if (this->fsmState != s->fsmState) {
        return false;
    }

    // Vector
    for (uint i = 0; i < this->mpvector->getSize(); i++) {
        if (fabs(static_cast<CDouble>(this->mpvector->get(i) - s->mpvector->get(i)))
            > static_cast<CDouble>(MP_EPSILON)) {
            return false;
        }
    }

    return true;
}

/**
 * compare()
 * The function implements a comparison to generate a total order
 * return -1 if receiver smaller than s,
 * 0 if equal, otherwise 1
 * performs a lexicographical order on :
 * 1) fsm state
 * 2) vector size and
 * 3) token time stamps.
 */
int State::compare(const std::unique_ptr<State>& s) const {

    // Check if the pointers are equal
    if (this == s.get()) {
        return 0;
    }

    // FSM state
    if (this->fsmState != s->fsmState) {
        if (this->fsmState < s->fsmState) {
            return -1;
        }
        return 1;
    }

    // Vector size
    if (this->mpvector->getSize() != s->mpvector->getSize()) {
        if (this->mpvector->getSize() < s->mpvector->getSize()) {
            return -1;
        }
        return 1;
    }

    // Vector
    for (uint i = 0; i < this->mpvector->getSize(); i++) {
        MPDelay diff;
        diff = this->mpvector->get(i) - s->mpvector->get(i);
        if (-diff > MP_EPSILON) {
            return -1;
        }
        if (diff > MP_EPSILON) {
            return 1;
        }
    }

    return 0;
}

/**
 * executeIteration()
 * Execute one iteration of the scenario graph associated with this state.
 */
void State::executeIteration() {
#ifdef _DEBUG_
    std::cerr << "execute iteration for state: " << std::endl;
    print(std::cerr);
    std::cerr << std::endl;
#endif

    this->mpvector = std::make_unique<Vector>(this->mat->mp_multiply(*this->mpvector));
}

/**
 * normalize()
 * Normalize the production time of the tokens in all FIFOs.
 */
void State::normalize() { this->delay = this->mpvector->normalize(); }

/**
 * addReachableState()
 * Add the state to the set of reachable states.
 */
bool State::addReachableState(State* state, MPDelay d, double r) {
    // if there is some reachable state (q',v') with d' such that
    // q'=q, d+v <= d'+v' and  r >= r' then forget
    ReachableStates::const_iterator i;
    for (i = this->reachableStates.begin(); i != this->reachableStates.end(); i++) {
        // check q'== q
        if ((*i).first->fsmState == state->fsmState) {
            // get vectors
            Vector* vp = (*i).first->mpvector.get();
            Vector* v = state->mpvector.get();

            // check d+v <= d'+v' and  r >= r'
            const DelaysAndRewards &edges = (*i).second;
            DelaysAndRewards::const_iterator j;
            for (j = edges.begin(); j != edges.end(); j++) {
                CDouble rp = (*j).second;
                if (r >= rp) {
                    // check d+v <= d'+v'
                    MPDelay dp = (*j).first;
                    bool dominates = true;
                    for (unsigned int k = 0; dominates && k < this->mpvector->getSize(); k++) {
                        dominates = dominates && (d + (v->get(k)) <= dp + (vp->get(k)));
                    }
                    // if it dominates, the state does not need to be added
                    if (dominates) {
                        return false;
                    }
                }
            }
        }
    }

    // if state is already reachable update edges, while pruning dominated edges...
    if (this->reachableStates.find(state) != this->reachableStates.end()) {
        DelaysAndRewards &edges = this->reachableStates[state];
        edges.addAndPrune(d, r);
        return false;
    }
    // add a new state
    DelaysAndRewards newDr;
    this->reachableStates[state] = newDr;
    this->reachableStates[state].add(d, r);
    return true;

#ifdef _DEBUG_
    std::cerr << "reached state: " << std::endl;
    state->print(std::cerr);
    std::cerr << "from state: " << std::endl;
    print(std::cerr);
    std::cerr << "with delay: " << state->delay << std::endl;
#endif
}

/**
 * print()
 * Print the state.
 */
void State::print(std::ostream &out) const {
    // Vector
    for (uint i = 0; i < this->mpvector->getSize(); i++) {
        out << "[" << MPString(this->mpvector->get(i)) << "]: ";
        out << std::endl;
    }

    // FSM state
    out << "fsm state: " << (dynamic_cast<const ELSState*>(fsmState))->stateLabel << std::endl;
}

/**
 * exploreThroughput()
 * Compute the throughput of the FSM-based SADF graph using a state-space
 * traversal.
 */
Throughput StateSpaceExploration::exploreThroughput(const Graph& g) {

    class _local {
    public:
        static void
        createNewReachableStates(State* curState, const std::unique_ptr<ScenarioMatrices>& sm, StateQueue& sq) {
            // Create new reachable state from this state
            const auto &edges = curState->fsmState->getOutgoingEdges();
            for (const auto *i : edges) {
                const auto *e = dynamic_cast<ELSEdgeRef>(i);
                // Create a new state based on this transition
                auto newState = std::make_unique<State>(curState,
                                           e->getDestination(),
                                           ((*sm)[e->getLabel()]).get(),
                                           static_cast<CDouble>(curState->reward));
                // Add new state to the state queue
                sq.push_front(std::move(newState));
            }
        }
    };
    _local local;

    auto visitedStates = std::make_unique<States>();
    auto stateQueue = std::make_unique<StateQueue>();
    Throughput thr = 0;

    std::unique_ptr<ScenarioMatrices> scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenarios(g);

    // create a minimal edge labelled scenario automaton
    std::unique_ptr<::FSMSADF::EdgeLabeledScenarioFSM> elsFSM =
            FSMConverter::minimalEdgeLabeledFSM(*(g.getFSM()));

    // Create an initial state based on the initial FSM state
    std::unique_ptr<State> initialStateSP = std::make_unique<State>(nullptr, elsFSM->getInitialState(), nullptr, 0.0);
    initialStateSP->mpvector =
            std::make_unique<Vector>(static_cast<unsigned int>(scenarioMatrices->vectorSize()), MPTime(0.0));

    // Add the initial state to the set of visited states
    auto* initialState = initialStateSP.get();
    visitedStates->insert(std::move(initialStateSP));

#ifdef _DEBUG_
    std::cerr << "insert state: " << std::endl;
    initialState->print(std::cerr);
    std::cerr << std::endl;
#endif

    // create new states to explore frm the initial state.
    _local::createNewReachableStates(initialState, scenarioMatrices, *stateQueue);

    std::unique_ptr<State> curState = nullptr;

    // Explore state-space
    while (!stateQueue->empty()) {

        // Take the first state from the queue
        curState = std::move(stateQueue->front());
        stateQueue->erase(stateQueue->begin());

        // Execute one iteration of the scenario graph
        curState->executeIteration();

        // Normalize state
        curState->normalize();

#ifdef _DEBUG_
        std::cerr << std::endl;
        std::cerr << "reached state: " << std::endl;
        curState->print(std::cerr);
        std::cerr << std::endl;
#endif

        // Reached this state before?
        if (visitedStates->includes(curState)) {
            // Get recurrent state from the set of visited states
            State* s = visitedStates->find(curState)->get();

#ifdef _DEBUG_
            std::cerr << "recurrent state:" << std::endl;
            s->print(std::cerr);
            std::cerr << std::endl;
#endif

            // Previous state can go to recurrent state
            curState->previousState->addReachableState(
                    s, curState->delay, static_cast<CDouble>(curState->reward));

        } else {
            // Add state to the set of reachable states of the previous
            // state
            bool added = curState->previousState->addReachableState(
                    curState.get(), curState->delay, static_cast<CDouble>(curState->reward));

            // if the state was not redundant and has really been added
            if (added) {
                // Add state to the set of visited states
                visitedStates->insert(std::move(curState));

#ifdef _DEBUG_
                std::cerr << std::endl;
                std::cerr << "insert state: " << std::endl;
                curState->print(std::cerr);
                std::cerr << std::endl;
#endif

                // Create new reachable state from this state
                _local::createNewReachableStates(curState.get(), scenarioMatrices, *stateQueue);

            }
        }
    }

#ifdef _DEBUG_
    std::ofstream dot("statespace.dot");
    printStateSpace(visitedStates, dot);
#endif

    // Compute throughput
    thr = computeThroughput(*visitedStates, initialState);

    // Cleanup
    visitedStates->clear();

    return thr;
}

/**
 * exploreThroughputMax()
 * Compute the throughput of the FSM-based SADF graph using a state-space
 * traversal while combine multiple scenarios leading the same net state of
 * the fsm.
 */
Throughput StateSpaceExploration::exploreThroughputMax(const Graph& g) {

    class _local {
    public:
        static void createNewReachableStates(State* curState, StateQueue& sq) {
            // Create new reachable state from this state
            const auto &edges = curState->fsmState->getOutgoingEdges();
            for (const auto *i : edges) {
                const auto *e = dynamic_cast<MLSEdgeRef>(i);
                // Create a new state based on this transition
                auto newState = std::make_unique<State>(
                        curState, e->getDestination(), e->getLabel().mat.get(), e->getLabel().rew);
                // Add new state to the state queue
                sq.push_front(std::move(newState));
            }
        }
    };
    _local local;

    std::unique_ptr<State> curState = nullptr;
    auto visitedStates = std::make_unique<States>();
    auto stateQueue = std::make_unique<StateQueue>();
    Throughput thr = 0;

    // Create maxplus matrices from the individual scenario graphs
    std::unique_ptr<ScenarioMatrices> scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenariosForWeakFSMSADF(g);

    std::unique_ptr<ScenarioRewards> scenarioRewards = GraphConversion::getRewardsFromScenarios(g);

    // create a minimal edge labelled scenario automaton
    std::unique_ptr<EdgeLabeledScenarioFSM> elsFSM =
            FSMConverter::minimalEdgeLabeledFSM(*(g.getFSM()));
    std::unique_ptr<MatrixLabeledScenarioFSM> mlsFSM =
            FSMConverter::convertToMatrixLabeledScenarioFSM(
                    *elsFSM, *scenarioMatrices, *scenarioRewards);

    // Create an initial state based on the initial FSM state
    std::unique_ptr<State> initialStateSP = std::make_unique<State>(nullptr, mlsFSM->getInitialState(), nullptr, 0.0);
    initialStateSP->mpvector =
            std::make_unique<Vector>(static_cast<unsigned int>(scenarioMatrices->vectorSize()), MPTime(0.0));

    auto* initialState = initialStateSP.get();
    // Add the initial state to the set of visited states
    visitedStates->insert(std::move(initialStateSP));

#ifdef _DEBUG_
    std::cerr << "insert state: " << std::endl;
    initialState->print(std::cerr);
    std::cerr << std::endl;
#endif

    // create new states to explore from the initial state.
    _local::createNewReachableStates(initialState, *stateQueue);

    // Explore state-space
    while (!stateQueue->empty()) {
        // Take the first state from the queue
        curState = std::move(stateQueue->front());
        stateQueue->erase(stateQueue->begin());

        // Execute one iteration of the scenario graph
        curState->executeIteration();

        // Normalize state
        curState->normalize();

        // Reached this state before?
        if (visitedStates->includes(curState)) {
            // Get recurrent state from the set of visited states
            State* s = visitedStates->find(curState)->get();

            // Previous state can go to recurrent state
            curState->previousState->addReachableState(
                    s, curState->delay, static_cast<CDouble>(curState->reward));

        } else {
            // std::cout << "adding state: " << std::endl;
            // curState->print(std::cout);
            // std::cout << std::endl;
            //  Add state to the set of reachable states of the previous
            //  state
            bool added = curState->previousState->addReachableState(
                    curState.get(), curState->delay, static_cast<CDouble>(curState->reward));

            // if the state was not redundant and has really been added
            if (added) {
                auto* curStatePtr = curState.get();
                // Add state to the set of visited states
                visitedStates->insert(std::move(curState));

                // Create new reachable state from this state
                _local::createNewReachableStates(curStatePtr, *stateQueue);
            }
        }
    }

    // Compute throughput
    // std::cout << "number of states: " << visitedStates->size() << std::endl;
    thr = computeThroughput(*visitedStates, initialState);

    // Cleanup
    visitedStates->clear();

    return thr;
}

/**
 * generateTraces()
 * Generate traces of actor firings for the specified scenario sequence.
 */
std::unique_ptr<TraceOfFirings> StateSpaceExploration::generateTraces(const Graph& g,
                                                      const std::list<CString> &scenarioSequence) {
    std::unique_ptr<ScenarioTracinginfo> scenarioTracingInfo = GraphConversion::computeTracinginfoFromScenarios(g);
    auto startingTimeStamps =
            std::make_unique<Vector>(static_cast<unsigned int>(g.getPersistentTokens().size()), MPTime(0));
    unsigned int iterationCounter = 0;
    auto firings = std::make_unique<TraceOfFirings>();
    CString resource = "Default Processor";

    // iterate through the scenario sequence
    for (const auto &i : scenarioSequence) {
        iterationCounter++;
        auto j = scenarioTracingInfo->find(i);
        Tracinginfo* ti = (*j).second.get();
        MaxPlus::Matrix *SM = ti->getScenarioMatrix();
        MaxPlus::Matrix *TM = ti->getTracingMatrix();
        const std::list<CString>* firingsOrder = ti->getActorsFiringOrder();
        MaxPlus::Vector startEndTimesOfActors = TM->mp_multiply(*startingTimeStamps);
        MaxPlus::Vector newTimeStamps = SM->mp_multiply(*startingTimeStamps);

        startingTimeStamps = std::make_unique<MaxPlus::Vector>(newTimeStamps);

        // create the firings
        unsigned int numberOfActors = startEndTimesOfActors.getSize() / 2;
        auto p = firingsOrder->begin();
        for (unsigned int k = 0; k < numberOfActors; k++) {
            auto firing = std::make_unique<TraceFiring>();
            firing->create(*p,
                           i,
                           resource,
                           iterationCounter,
                           startEndTimesOfActors.get(2 * k),
                           startEndTimesOfActors.get((2 * k) + 1));
            firings->push_back(std::move(firing));
            p++;
        }
    }

    return firings;
}

/**
 * generateTracesWeakConsistency()
 * Generate traces of actor firings for the specified scenario sequence when the graph is weakly
 * consistent
 */
std::unique_ptr<TraceOfFirings>
StateSpaceExploration::generateTracesWeakConsistency(const Graph& g,
                                                     const std::list<CString> &scenarioSequence) {

    // check consistency
    if (!g.isWeaklyConsistent()) {
        throw ::CException("FSM-SADF graph is not consistent. \n" + g.reportWeaklyConsistent());
    }

    std::unique_ptr<ScenarioTracinginfo> scenarioTracinginfo =  GraphConversion::computeTracinginfoFromScenariosWeakConsistency(g);
    uint initialTimeStampSize = static_cast<uint>(g.getFSM()
                                                          ->getInitialState()
                                                          ->getScenario()
                                                          ->getScenarioGraph()
                                                          ->getPersistentTokenIndexMap()
                                                          .size());

    auto startingTimeStamps = std::make_unique<Vector>(initialTimeStampSize, MPTime(0.0));
    uint iterationCounter = 0;
    auto firings = std::make_unique<TraceOfFirings>();
    CString resource = "Default Processor";

    // iterate through the scenario sequence
    for (const auto &i : scenarioSequence) {
        auto j = scenarioTracinginfo->find(i);
        Tracinginfo* ti = (*j).second.get();
        MaxPlus::Matrix *SM = ti->getScenarioMatrix();
        MaxPlus::Matrix *TM = ti->getTracingMatrix();
        FiringOrder* firingsOrder = ti->getActorsFiringOrder();
        MaxPlus::Vector startEndTimesOfActors = TM->mp_multiply(*startingTimeStamps);
        MaxPlus::Vector newTimeStamps = SM->mp_multiply(*startingTimeStamps);

        startingTimeStamps = std::make_unique<MaxPlus::Vector>(newTimeStamps);

        // create the firings
        unsigned int numberOfActors = startEndTimesOfActors.getSize() / 2;
        auto p = firingsOrder->begin();
        for (unsigned int k = 0; k < numberOfActors; k++) {
            auto firing = std::make_unique<TraceFiring>();
            firing->create(*p,
                           i,
                           resource,
                           iterationCounter,
                           startEndTimesOfActors.get(2 * k),
                           startEndTimesOfActors.get((2 * k) + 1));
            firings->push_back(std::move(firing));
            p++;
        }
        iterationCounter++;
    }

    return firings;
}

/**
 * generateTraceOfVectorsWeakConsistency()
 * Generate traces of state vectors for the specified scenario sequence when the graph is weakly
 * consistent
 */
std::unique_ptr<TraceOfVectors> StateSpaceExploration::generateTraceOfVectorsWeakConsistency(
        const Graph& g, const std::list<CString> &scenarioSequence) {
    std::unique_ptr<ScenarioMatrices> SMs = GraphConversion::computeMaxPlusMatricesFromScenarios(g);

    const CStrings &initialPersistentTokenss = g.getFSM()
                                     ->getInitialState()
                                     ->getScenario()
                                     ->getScenarioGraph()
                                     ->getPersistentTokens();
    uint initialTimeStampSize = static_cast<uint>(initialPersistentTokenss.size());
    auto startingTimeStamps = std::make_unique<Vector>(initialTimeStampSize, MPTime(0.0));

    uint iterationCounter = 0;

    auto vectors = std::make_unique<TraceOfVectors>();

    auto tv = std::make_unique<TraceVector>(initialPersistentTokenss, *startingTimeStamps);
    vectors->push_back(std::move(tv));

    // iterate through the scenario sequence
    for (auto i = scenarioSequence.begin(); i != scenarioSequence.end(); i++, iterationCounter++) {
        const Scenario* sc = g.getScenarioRef(*i);
        auto *SM = (SMs->at(*i)).get();
        MaxPlus::Vector newTimeStamps = SM->mp_multiply(*startingTimeStamps);

        const CStrings &pts = FSMSADF::Graph::getPersistentTokensOfScenario(*sc);
        tv = std::make_unique<TraceVector>(pts, newTimeStamps);
        vectors->push_back(std::move(tv));

        startingTimeStamps = std::make_unique<Vector>(newTimeStamps);
    }

    return vectors;
}

/**
 * computeThroughput()
 * Compute the MCM of the state space. The
 * inverse of the MCM is the throughput of the graph.
 */
Throughput StateSpaceExploration::computeThroughput(const States& visitedStates, State* initialState) {
#ifdef _COMPUTE_MCM_USING_DFS
    std::map<State *, State *> parents;
    States color;
#endif
    double mcm = 0.0;

#ifdef _DEBUG_
    std::cerr << std::endl;
    std::cerr << "Throughput computation:" << std::endl;
#endif

#ifdef _COMPUTE_MCM_USING_DFS
    // Compute mcm using DFS
    parents[initialState] = nullptr;
    mcm = dfsStateSpace(visitedStates, initialState, parents, color);
#else
    // Compute mcr
    mcm = computeMCR(visitedStates, initialState);
#endif

    // MCM is zero? Then there is no throughput (throughout is infinite)
    if (mcm == 0.0) {
        throw ::CException(
                "Cannot compute throughout. MCM of the statespace is 0 (throughput is infinite).");
    }

    return static_cast<Throughput>(1.0 / mcm);
}

/**
 * computeMCR()
 * The function returns the MCR of the statespace annotated with rewards
 */
double StateSpaceExploration::computeMCR(const States& visitedStates, State* /*initialState*/) {
    std::map<State*, MCMnode *> stateNodes;
    CId id = 0;

    // Create a new MCM graph
    auto mcmGraph = std::make_unique<MCMgraph>();

    // Create the nodes of the MCR graph
    for (const auto& s : visitedStates) {
        // Create an MCM node for this state
        MCMnode *n = mcmGraph->addNode(id, true);

        // Store node of this state
        stateNodes[s.get()] = n;
        id++;
    }

    // Counter for edge Ids
    CId eId = 0;

    // Add edges to the MCM graph
    for (const auto& s : visitedStates) {
        // Iterate over the reachable states
        for (ReachableStates::const_iterator j = s->reachableStates.begin();
             j != s->reachableStates.end();
             j++) {
            State* rs = (*j).first;
            const DelaysAndRewards &dr = (*j).second;
            // Iterate over the delay and reward combinations
            for (auto [kDel, kRew] : dr) {
                // Add the edge to the MCM graph and the src and dst node
                MPDelay del = kDel;
                double rew = kRew; // find reward!
                mcmGraph->addEdge(
                        eId++, *(stateNodes[s.get()]), *(stateNodes[rs]), static_cast<CDouble>(del), rew);
            }
        }
    }

    // calculate the MCR
    return maxCycleRatioYoungTarjanOrlin(*mcmGraph);
}

/**
 * printStateSpace()
 * The function outputs the state-space in DOT format.
 */
void StateSpaceExploration::printStateSpace(States *states, std::ostream &out) {
    std::map<State*, uint> stateId;

    out << "digraph statespace {" << std::endl;
    out << "    size=\"7,10\";" << std::endl;

    for (const auto& s : *states) {
        // No id assigned to state s?
        if (stateId.find(s.get()) == stateId.end()) {
            stateId[s.get()] = static_cast<uint>(stateId.size());
        }

        // Get id of state s
        uint sId = stateId[s.get()];

        // Output state s as a new node
        out << "    " << sId << " [shape=box label=\"";

        // vector
        for (uint j = 0; j < s->mpvector->getSize(); j++) {
            out << "[" << static_cast<CDouble>(s->mpvector->get(j)) << "]";
            out << "\\n";
        }

        // FSM state
        out << "fsm state: " << (dynamic_cast<const ELSState *>(s->fsmState))->stateLabel << "\\n";

        // Delay
        out << "delay: " << static_cast<CDouble>(s->delay) << "\\n";

        // Close node
        out << "\"];" << std::endl;

        // Output edges to all reachable states
        for (ReachableStates::const_iterator j = s->reachableStates.begin();
             j != s->reachableStates.end();
             j++) {
            State* t = (*j).first;
            const DelaysAndRewards &dr = (*j).second;

            // No id assigned to state t?
            if (stateId.find(t) == stateId.end()) {
                stateId[t] = static_cast<uint>(stateId.size());
            }

            // Get id of state t
            uint tId = stateId[t];

            // iterate over delay and reward combinations
            for (auto [kDel, kRew]: dr) {
                MPDelay d = kDel;
                CDouble r = kRew;
                // Output edge from state s to state t
                out << "    " << sId << " -> " << tId << " [label=";
                out << static_cast<CDouble>(d) << ", " << std::endl;
                out << r << "];" << std::endl;
            }
        }
    }

    out << "}" << std::endl;
}

} // End namespace MPExploreStateSpace

} // End namespace FSMSADF
