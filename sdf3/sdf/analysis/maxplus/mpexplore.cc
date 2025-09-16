/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 13, 2008
 *
 *  Function        :   Max-Plus based state-space exploration algorithms.
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

#include "mpexplore.h"
#include "base/basic_types.h"
#include "extendedmatrix.h"
#include "mpstorage.h"
#include "sdf/base/algo/repetition_vector.h"
#include "sdf/base/hsdf/check.h"
#include "sdf/base/timed/actor.h"
#include "sdf/base/timed/graph.h"
#include "sdf/base/untimed/channel.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>

namespace SDF::MaxPlusAnalysis {

using namespace GraphDecoration;
using namespace MaxPlus;

namespace {

std::ostream *out = nullptr; // NOLINT(*-global-variables)

} // namespace

/**
 * explore()
 * perform state space exploration till recurrent state to determine throughput
 */
MPThroughput Exploration::explore() {
    // set x to initial state (zero vector)
    std::unique_ptr<State> x = State::zeroState(*G);
    StateRef y = nullptr;

    // store all visited states (schedules) in S
    auto S = std::make_unique<StoredStates>();

    // first make one iteration to determine the static schedule;

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(G->repetitionVector());

    // create table to hold the schedule
    // size is equal to sum of the repetition vector
    int sch_length = static_cast<int>(E.sum());
    this->staticSchedule = std::make_unique<ActorRef[]>(sch_length);
    int sch_idx = 0;

    ActorList::const_iterator i = G->actors.begin();

    // until a complete iteration has been scheduled...
    while ((!E.isEmpty()) && sch_idx < sch_length) {
        Actor *a = nullptr;
        while (true) // find an enabled actor
        {
            a = i->get();
            // break if actor is enabled
            if (E.getValue(*a) > 0 && x->actorEnabled(*a)) {
                break;
            }
            // try next actor, wrap around if at end of actor list
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }
        // account for firing of a
        E.decreaseValue(*a);
        // fill in the schedule table
        this->staticSchedule[sch_idx++] = a;
        // let G fire a on state x
        Graph::fire(*x, *a);
    }
    // an iteration is complete, normalize the schedule
    Exploration::normalize(*x);

    // from now on continue with fixed schedule in staticSchedule[]

    // until a normalized schedule is reached that we saw before
    this->maxNumberOfStatesReached = false;
    while (!S->includes(x, &y)
           && ((!this->limitMaxNumberOfStates) || (S->size() < this->maxNumberOfStates))) {
        // store the state x in the set S of visited schedules
        S->store(x->copy());
        // count the number of iterations explored
        (x->count)++;
        // fire a complete iteration according to the static schedule
        for (int k = 0; k < sch_length; k++) {
            Graph::fire(*x, *(this->staticSchedule[k]));
        }
        // normalizer the result
        Exploration::normalize(*x);
    }

    // we have reached a recurrent state and we are done or we reached the maximum number of states.
    if ((this->limitMaxNumberOfStates) && (S->size() >= this->maxNumberOfStates)) {
        this->maxNumberOfStatesReached = true;
        return 0.0;
    }

    // return the throughput: number of iterations between x and y divided by
    // their time difference
    return static_cast<CDouble>(x->count - y->count)
           / static_cast<CDouble>((x->timestamp - y->timestamp));
}

/**
 * explore_backward()
 * Perform state space exploration backward in time till recurrent state to
 * determine throughput
 */
MPThroughput Exploration::explore_backward() {
    // set x to initial state (zero vector)
    std::unique_ptr<State> x = State::zeroState(*G);
    StateRef y = nullptr;

    // store all visited states (schedules) in S
    auto S = std::make_unique<StoredStates>();

    // first make one iteration to determine the static schedule

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(G->repetitionVector());

    // create table to hold the schedule
    // size is equal to sum of the repetition vector
    int sch_length = static_cast<int>(E.sum());
    staticSchedule = std::make_unique<ActorRef[]>(sch_length);
    int sch_idx = 0;

    ActorList::const_iterator i = G->actors.begin();

    // until a complete iteration has been scheduled...
    while ((!E.isEmpty()) && sch_idx < sch_length) {
        Actor *a = nullptr;
        while (true) // find an enabled actor
        {
            a = i->get();
            // break if actor is 'reverse-enabled'
            if (E.getValue(*a) > 0 && x->actorReverseEnabled(*a)) {
                break;
            }
            // try next actor, warp around if at end of actor list
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }
        // account for firing of a
        E.decreaseValue(*a);
        // fill in the scheduling table
        staticSchedule[sch_idx++] = a;
        // let G fire backward on state x
        Graph::fire_reverse(*x, *a);
    }

    // an iteration is complete, normalize the schedule
    Exploration::normalize(*x);

    // from now on continue with fixed schedule in staticSchedule[]

    // until a normalized schedule is reached that we saw before
    while (!S->includes(x, &y)) {
        // store the state x in the set S of visited
        S->store(x->copy());
        // count the number of iterations explored
        (x->count)++;
        // fire a complete iteration according to the static schedule
        for (int k = 0; k < sch_length; k++) {
            Graph::fire_reverse(*x, *(staticSchedule[k]));
        }

        // normalize the result
        Exploration::normalize(*x);
    }

    // we have reached a recurrent state and we are done.

    // return the throughput: number of iterations between x and y divided by
    // their time difference
    return static_cast<CDouble>(x->count - y->count)
           / static_cast<CDouble>((y->timestamp - x->timestamp));
}

/**
 * convertToTracingInfo()
 */
std::unique_ptr<std::array<std::unique_ptr<Matrix>, 2>> Exploration::convertToTracingInfo() {
    unsigned int actorFiringsCounter = 0;
    auto actorsFiringOrder = std::make_unique<std::list<CString>>();
    // do a symbolic exploration of a single iteration

    // make symbolic zero-vector
    std::unique_ptr<SymbolicState> x = SymbolicState::zeroState(*G);

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(G->repetitionVector());

    // calculate total number of firings
    unsigned int totalNumberOfFirings = 0;
    for (const auto &a : G->actors) {
        totalNumberOfFirings = totalNumberOfFirings + E.getValue(*a);
    }

    auto MH = std::make_unique<Matrix>(2 * totalNumberOfFirings, x->numberOfTokens());
    auto i = G->actors.begin();
    while (!E.isEmpty()) {
        Actor *a = nullptr;
        while (true) // find an enabled actor
        {
            a = i->get();
            if (E.getValue(*a) > 0 && x->actorEnabled(*a)) {
                break;
            }
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }

        actorsFiringOrder->push_back(g->getConstActor(static_cast<CId>(a->index))->getName());
        E.decreaseValue(*a);
        G->fireSymbolicCaptureTracingInfo(*x, *a, MH.get(), 2 * actorFiringsCounter);
        actorFiringsCounter++;
    }

    this->actorsFiringOrder = std::move(actorsFiringOrder);
    // Construct a MaxPlus matrix

    unsigned int N = x->numberOfTokens();
    auto MG = std::make_unique<Matrix>(N);

    // Add fill matrix rows symbolic tokens
    std::unique_ptr<SymbolicToken> t = x->firstToken();
    for (unsigned int i = 0; i < N; i++) {
        for (unsigned int j = 0; j < N; j++) {
            // Take the j'th entry out of symbolic token t
            MG->put(i, j, ((*t)[j])->productionTime);
        }
        t = x->nextToken();
    }
    std::unique_ptr<std::array<std::unique_ptr<Matrix>, 2>> MGH =
            std::make_unique<std::array<std::unique_ptr<Matrix>, 2>>();

    (*MGH)[0] = std::move(MG);
    (*MGH)[1] = std::move(MH);

    return MGH;
}

CStrings Exploration::getMatrixActorLabelling() const {
    CStrings result;

    for (const auto &a : this->g->getActors()) {
        result.push_back(a->getName());
    }

    return result;
}

CStrings Exploration::getMatrixStateLabelling() const {
    CStrings result;

    int k = 0;
    for (auto i = this->g->channelsBegin(); i != this->g->channelsEnd(); i++, k++) {
        const std::unique_ptr<SDFchannel> &c = *i;
        bool multipleTokens = c->getInitialTokens() > 1;
        for (uint j = 0; j < c->getInitialTokens(); j++) {
            if (multipleTokens) {
                result.push_back(c->getName() + "_" + CString(j + 1));
            } else {
                result.push_back(c->getName());
            }
        }
    }

    return result;
}

CStrings Exploration::getMatrixInputLabelling() const {
    CStrings result;
    auto rv = computeRepetitionVector(*g);

    for (const auto &i : this->g->getInputs()) {
        auto *inputChannel = i->getPort()->getInputChannel();
        auto actorId = inputChannel->getDstActor()->getId();
        auto dstRate = inputChannel->getDstPort()->getRate();

        bool multipleInputs = rv[actorId] * dstRate > 1;

        for (uint j = 0; j < rv[actorId] * dstRate; j++) {
            if (multipleInputs) {
                result.push_back(i->getName() + "_" + CString(j + 1));
            } else {
                result.push_back(i->getName());
            }
        }
    }

    return result;
}

CStrings Exploration::getMatrixOutputLabelling() const {
    CStrings result;
    auto rv = computeRepetitionVector(*g);

    for (const auto &o : this->g->getOutputs()) {
        auto *outputChannel = o->getPort()->getOutputChannel();
        auto actorId = outputChannel->getSrcActor()->getId();
        auto srcRate = outputChannel->getSrcPort()->getRate();

        bool multipleInputs = rv[actorId] * srcRate > 1;

        for (uint j = 0; j < rv[actorId] * srcRate; j++) {
            if (multipleInputs) {
                result.push_back(o->getName() + "_" + CString(j + 1));
            } else {
                result.push_back(o->getName());
            }
        }
    }

    return result;
}

std::list<std::unique_ptr<TokenLabel>> Exploration::getTokenDecoration() const {
    std::list<std::unique_ptr<TokenLabel>> tokenLabels;

    uint idx = 0;
    for (const auto &c : this->g->getChannels()) {
        uint nTokens = c->getInitialTokens();
        bool multipleTokens = nTokens > 1;
        for (uint k = 0; k < nTokens; k++) {
            auto t = std::make_unique<TokenLabel>();
            if (multipleTokens) {
                t->name = c->getName() + "_" + CString(k + 1);
            } else {
                t->name = c->getName();
            }
            t->index = idx + k;
            t->channelFirst = k == 0;
            t->channelLast = k == (nTokens - 1);
            t->channel = c.get();
            tokenLabels.push_back(std::move(t));
        }
        idx += nTokens;
    }

    return tokenLabels;
}

/**
 * convertToMaxPlusMatrix()
 */
std::unique_ptr<Matrix> Exploration::convertToMaxPlusMatrix() const {

    // do a symbolic exploration of a single iteration

    // make symbolic zero-vector
    std::unique_ptr<SymbolicState> x = SymbolicState::zeroState(*G);

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(OG->repetitionVector());

    ActorList::const_iterator i = G->actors.begin();

    while (!E.isEmpty()) {
        Actor *a = nullptr;

        while (true) // find an enabled actor
        {
            a = i->get();

            if (E.getValue(*a) > 0 && x->actorEnabled(*a)) {
                break;
            }
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }
        E.decreaseValue(*a);
        G->fireSymbolic(*x, *a);
    }

    // Construct a MaxPlus matrix

    unsigned int M = x->numberOfTokens();
    auto N = static_cast<unsigned int>(x->getSymbolicTokenSize());

    auto MG = std::make_unique<Matrix>(M, N);

    // Add fill matrix rows symbolic tokens
    std::unique_ptr<SymbolicToken> t = x->firstToken();
    for (unsigned int i = 0; i < M; i++) {
        for (unsigned int j = 0; j < N; j++) {
            // take the j'th entry out of symbolic token t
            MG->put(i, j, ((*t)[j])->productionTime);
        }
        t = x->nextToken();
    }

    return MG;
}

/**
 * convertToMaxPlusMatrixForWeakFSMSADF()
 * compute the max-plus matrix for the scenario graph G
 * throws a CDeadlockException when the repetition vector of the
 * scenario cannot be completed, i.e., if state is reached where none
 * of the actors that still need to fire in the repetition vector is
 * enabled.
 */
std::unique_ptr<Matrix> Exploration::convertToMaxPlusMatrixForWeakFSMSADF() {

    // do a symbolic exploration of a single iteration

    // make symbolic zero-vector
    std::unique_ptr<SymbolicState> x = makeSymbolicState();
    unsigned int NC = x->numberOfTokens();

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(G->repetitionVector());

    // for profiling purpose, should be removed

    ActorList::const_iterator i = G->actors.begin();
    try {
        while (!E.isEmpty()) {
            Actor *a = nullptr;
            auto initial_actor = i;
            while (true) // find an enabled actor
            {
                a = i->get();
                if (E.getValue(*a) > 0 && x->actorEnabled(*a)) {
                    break;
                }
                if (++i == G->actors.end()) {
                    i = G->actors.begin();
                }
                if (i == initial_actor) {
                    throw CDeadlockException("Graph is in deadlock in symbolic execution.");
                }
            }
            E.decreaseValue(*a);
            G->fireSymbolic(*x, *a);
        }
    } catch (CDeadlockException &e) {
        throw;
    }

    // for performance test
    // throw CException("performance test exception.");

    // Construct a MaxPlus matrix

    unsigned int NR = x->numberOfTokens();
    std::unique_ptr<Matrix> MG = makeMatrix(NR, NC);

    // Add fill matrix rows symbolic tokens
    std::unique_ptr<SymbolicToken> t = x->firstToken();
    for (unsigned int i = 0; i < NR; i++) {
        for (unsigned int j = 0; j < NC; j++) {
            // take the j'th entry out of symbolic token t
            auto *EMG = dynamic_cast<ExtendedMatrix *>(MG.get());
            if (EMG != nullptr) {
                auto *tEMR = dynamic_cast<TokenEMR *>(((*t)[j]).get());
                EMG->put(i, j, tEMR->productionTime, tEMR->bufferSet);
            } else {
                MG->put(i, j, ((*t)[j])->productionTime);
            }
        }
        t = x->nextToken();
    }

    return MG;
}

/**
 * convertToTracingInfoWeakConsistency()
 */
std::unique_ptr<std::array<std::unique_ptr<MaxPlus::Matrix>, 2>>
Exploration::convertToTracingInfoWeakConsistency() {

    // do a symbolic exploration of a single iteration

    // make symbolic zero-vector
    std::unique_ptr<SymbolicState> x = SymbolicState::zeroState(*G);
    unsigned int NC = x->numberOfTokens();

    unsigned int actorFiringsCounter = 0;
    actorsFiringOrder = std::make_unique<std::list<CString>>();

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(G->repetitionVector());

    // calculate total number of firings
    unsigned int totalNumberOfFirings = 0;
    for (const auto &a : G->actors) {
        totalNumberOfFirings = totalNumberOfFirings + E.getValue(*a);
    }

    auto MH = std::make_unique<Matrix>(2 * totalNumberOfFirings, NC);
    auto i = G->actors.begin();
    while (!E.isEmpty()) {
        Actor *a = nullptr;
        while (true) // find an enabled actor
        {
            a = i->get();
            if (E.getValue(*a) > 0 && x->actorEnabled(*a)) {
                break;
            }
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }

        actorsFiringOrder->push_back(g->getConstActor(static_cast<CId>(a->index))->getName());
        E.decreaseValue(*a);
        G->fireSymbolicCaptureTracingInfo(*x, *a, MH.get(), 2 * actorFiringsCounter);
        actorFiringsCounter++;
    }

    // Construct a MaxPlus matrix

    unsigned int NR = x->numberOfTokens();
    auto MG = std::make_unique<Matrix>(NR, NC);

    // Add fill matrix rows symbolic tokens
    std::unique_ptr<SymbolicToken> t = x->firstToken();
    for (unsigned int i = 0; i < NR; i++) {
        for (unsigned int j = 0; j < NC; j++) {
            // take the j'th entry out of symbolic token t
            MG->put(i, j, ((*t)[j])->productionTime);
        }
        t = x->nextToken();
    }

    std::unique_ptr<std::array<std::unique_ptr<Matrix>, 2>> MGH = nullptr;
    MGH = std::make_unique<std::array<std::unique_ptr<Matrix>, 2>>();

    (*MGH)[0] = std::move(MG);
    (*MGH)[1] = std::move(MH);

    return MGH;
}

/**
 * computeStateAndOutputMatrices()
 */
std::pair<std::unique_ptr<MaxPlus::Matrix>, std::unique_ptr<MaxPlus::Matrix>>
Exploration::computeStateAndOutputMatrices() const {

    // do a symbolic exploration of a single iteration

    // make symbolic zero-vector
    auto x = std::unique_ptr<SymbolicState>(SymbolicState::zeroState(*G));
    unsigned int NC = x->numberOfTokens();

    // every actor needs to fire a number of times according to the repetition
    // vector
    ActorCount E(G->repetitionVector());
    ActorRefList &outputActors = G->outputActors;
    ActorRefList::const_iterator outputActorsIter;

    // calculate total number of firings
    unsigned int totalNumberOfOutputs = 0;
    Actor *a = nullptr;
    for (const auto &a : G->actors) {
        outputActorsIter = find(outputActors.begin(), outputActors.end(), a.get());
        if (outputActorsIter != outputActors.end()) {
            totalNumberOfOutputs = totalNumberOfOutputs + E.getValue(*a);
        }
    }

    auto MO = std::make_unique<Matrix>(0, NC, NC * totalNumberOfOutputs);
    auto i = G->actors.begin();
    while (!E.isEmpty()) {
        Actor *a = nullptr;
        while (true) // find an enabled actor
        {
            a = i->get();
            if (E.getValue(*a) > 0 && x->actorEnabled(*a)) {
                break;
            }
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }

        E.decreaseValue(*a);
        G->fireSymbolicCaptureOutputMatrix(*x, *a, *MO);
    }

    // Construct a MaxPlus matrix

    unsigned int NR = x->numberOfTokens();
    auto MG = std::make_unique<Matrix>(NR, NC);

    // Add fill matrix rows symbolic tokens
    std::unique_ptr<SymbolicToken> t = x->firstToken();
    for (unsigned int i = 0; i < NR; i++) {
        for (unsigned int j = 0; j < NC; j++) {
            // take the j'th entry out of symbolic token t
            MG->put(i, j, ((*t)[j])->productionTime);
        }
        t = x->nextToken();
    }

    std::pair<std::unique_ptr<MaxPlus::Matrix>, std::unique_ptr<MaxPlus::Matrix>> matrices =
            std::make_pair(std::move(MG), std::move(MO));
    return matrices;
}

bool Exploration::isLinearStateSpaceDeadlocking() const {
    uint nActors = this->g->nrActors();
    auto allPairsMatrix = std::make_unique<Matrix>(nActors, nActors);

    // go through channels, set values in matrix to
    for (const auto &c : this->g->getChannels()) {
        // Only the channels without tokens
        if (c->getInitialTokens() == 0) {
            auto *a1 = dynamic_cast<TimedSDFactor *>(c->getSrcActor());
            auto *a2 = dynamic_cast<TimedSDFactor *>(c->getDstActor());

            // channel producer actor index (row) -> consumer actor index (column) in matrix must
            // get execution time of (producing actor)
            allPairsMatrix->put(a1->getId(), a2->getId(), MPTime(a1->getExecutionTime()));
        }
    }

    // perform all pairs longest path, non-positive cycle only, no implied self edges
    return allPairsMatrix->allPairLongestPathMatrix(MPTime(0), false, *allPairsMatrix);
}

std::tuple<std::unique_ptr<Matrix>,
           std::unique_ptr<Matrix>,
           std::unique_ptr<Matrix>,
           std::unique_ptr<Matrix>,
           std::unique_ptr<Matrix>,
           std::unique_ptr<Matrix>>
Exploration::computeLinearStateSpaceMatrices() const {

    // Get list of labelled tokens
    auto tokenLabels = this->getTokenDecoration();

    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(*g)) {
        throw CException("Graph is not an HSDF graph.");
    }

    uint nActors = this->g->nrActors();
    uint nInputs = this->g->nrInputs();
    uint nOutputs = this->g->nrOutputs();
    uint nTokens = tokenLabels.size();

    // Construct the all pairs matrix (is a max-plus matrix)
    auto allPairsMatrix = std::make_unique<Matrix>(nActors, nActors);

    // go through channels, set values in matrix to
    for (const auto &c : this->g->getChannels()) {
        // Only the channels without tokens
        if (c->getInitialTokens() == 0) {
            auto *a1 = dynamic_cast<TimedSDFactor *>(c->getSrcActor());
            auto *a2 = dynamic_cast<TimedSDFactor *>(c->getDstActor());

            // channel producer actor index (row) -> consumer actor index (column) in matrix must
            // get execution time of (producing actor)
            allPairsMatrix->put(a1->getId(), a2->getId(), MPTime(a1->getExecutionTime()));
        }
    }

    // perform all pairs longest path, non-positive cycle only, no implied self edges
    auto positiveCycle =
            allPairsMatrix->allPairLongestPathMatrix(MPTime(0), false, *allPairsMatrix);

    if (positiveCycle) {
        throw CException("Graph contains a positive cycle.");
    }

    // Create A B C D H J matrices using the longest paths
    auto A = std::make_unique<Matrix>(nTokens, nTokens);
    auto B = std::make_unique<Matrix>(nTokens, nInputs);
    auto C = std::make_unique<Matrix>(nOutputs, nTokens);
    auto D = std::make_unique<Matrix>(nOutputs, nInputs);
    auto H = std::make_unique<Matrix>(nActors, nTokens);
    auto J = std::make_unique<Matrix>(nActors, nInputs);

    // Construct A and H matrices
    for (const auto &ti : tokenLabels) {
        // A matrix
        for (const auto &tj : tokenLabels) {
            // check if tokens are on the same channel
            if (ti->channel == tj->channel && ti->index + 1 == tj->index) {
                A->put(tj->index, ti->index, MPTime(0));

                // tokens are on different channels
            } else if (ti->channelLast && tj->channelFirst) {
                auto *a1 = dynamic_cast<TimedSDFactor *>(ti->channel->getDstActor());
                auto *a2 = dynamic_cast<TimedSDFactor *>(tj->channel->getSrcActor());

                // tokens are connected through same actor
                if (a1 == a2) {
                    A->put(tj->index, ti->index, MPTime(a2->getExecutionTime()));
                } else {
                    MPTime aPM = allPairsMatrix->get(a1->getId(), a2->getId());
                    auto srcTime = MPTime(a2->getExecutionTime());
                    A->put(tj->index, ti->index, MP_PLUS(aPM, srcTime));
                }
            }
        }

        // H matrix
        for (const auto &aj : this->g->getActors()) {
            if (ti->channelLast) {
                if (ti->channel->getDstActor() == aj.get()) {
                    H->put(aj->getId(), ti->index, MPTime(0));
                } else {
                    SDFactor *aSource = ti->channel->getDstActor();
                    MPTime aPM = allPairsMatrix->get(aSource->getId(), aj->getId());
                    H->put(aj->getId(), ti->index, aPM);
                }
            }
        }
    }

    for (const auto &ii : this->g->getInputChannels()) {
        ;
        SDFactor *ai = ii->getDstActor();
        CId iId = ii->getSrcInput()->getId();

        // B matrix
        for (const auto &tj : tokenLabels) {

            if (tj->channelFirst) {
                auto *aj = dynamic_cast<TimedSDFactor *>(tj->channel->getSrcActor());

                // Input and token are connected through same actor
                if (ai == aj) {
                    B->put(tj->index, iId, MPTime(aj->getExecutionTime()));
                } else {
                    MPTime aPM = allPairsMatrix->get(ai->getId(), aj->getId());
                    auto srcTime = MPTime(aj->getExecutionTime());
                    B->put(tj->index, iId, MP_PLUS(aPM, srcTime));
                }
            }
        }

        // J matrix
        for (const auto &aj : this->g->getActors()) {
            if (ai == aj.get()) {
                J->put(aj->getId(), iId, MPTime(0));
            } else {
                MPTime aPM = allPairsMatrix->get(ai->getId(), aj->getId());
                J->put(aj->getId(), iId, aPM);
            }
        }
    }

    for (const auto &oci : this->g->getOutputChannels()) {
        SDFoutput *oi = oci->getDstOutput();
        auto *ai = dynamic_cast<TimedSDFactor *>(oci->getSrcActor());

        // C matrix
        Vector cVector = H->getRowVector(ai->getId());
        cVector.add(MPTime(ai->getExecutionTime()), cVector);
        C->pasteRowVector(oi->getId(), 0, &cVector);

        // D matrix
        Vector dVector = J->getRowVector(ai->getId());
        dVector.add(MPTime(ai->getExecutionTime()), dVector);
        D->pasteRowVector(oi->getId(), 0, &dVector);
    }

    auto matrices = std::make_tuple(
            std::move(A), std::move(B), std::move(C), std::move(D), std::move(H), std::move(J));

    return matrices;
}

std::tuple<VectorSequence, VectorSequence, VectorSequence>
Exploration::selfTimedStateSpaceSchedule(uint iterations) const {

    auto [A, B, C, D, H, J] = this->computeLinearStateSpaceMatrices();

    uint xSize = A->getCols();
    uint uSize = B->getCols();
    uint ySize = C->getRows();
    uint sSize = H->getRows();

    VectorSequence xSeq;
    VectorSequence uSeq;
    VectorSequence sSeq;
    VectorSequence ySeq;

    // Change in future, initial state vector defined by user (default 0)
    xSeq.push_back(std::make_unique<Vector>(xSize, MPTime(0)));

    // Reserve space for the input vector sequence
    uSeq.reserve(iterations);

    // Create iteration of vectors
    for (uint i = 0; i < iterations; i++) {
        auto v = std::make_unique<Vector>(uSize);
        uSeq.push_back(std::move(v));
    }

    // fill input matrix with input sequences
    for (const auto &i : this->g->getInputs()) {
        auto *ti = dynamic_cast<TimedSDFinput *>(i.get());

        // find corresponding event sequence otherwise front of ess
        const auto& ess = ti->getEventSequences();
        if (ess.empty()) {
            throw CException("No input sequence for input: " + ti->getName());
        }

        auto es = std::make_unique<EventSequence>(*(ess.front()));

        // select input sequence (eventSequenceName) from commandline,
        //       now default first sequence is chosen
        // for (const auto& esOfti: ti->getEventSequences()) {
        //     if (esOfti && esOfti->getName() == eventSequenceName) {
        //         es = esOfti;
        //         break;
        //     }
        // }

        if (es->size() < iterations) {
            throw CException("Not enough timestamps in input sequence: " + ti->getName());
        }

        // resize the input sequence of input i to the required iterations size
        es->resize(iterations);

        // Put the input sequence of input ti
        for (uint i = 0; i < iterations; i++) {
            uSeq[i]->put(ti->getId(), MPTime((*es)[i]));
        }
    }

    // perform iterations of the graph and store them in the vector sequences.
    for (uint i = 0; i < iterations; i++) {
        // compute x vector
        auto x = std::make_unique<Vector>(B->mp_multiply(*uSeq[i]));
        x->maximum(A->mp_multiply(*xSeq[i]), *x);

        // compute y vector
        auto y = std::make_unique<Vector>(D->mp_multiply(*uSeq[i]));
        y->maximum(C->mp_multiply(*xSeq[i]), *y);

        // compute s vector
        auto s = std::make_unique<Vector>(J->mp_multiply(*uSeq[i]));
        s->maximum(H->mp_multiply(*xSeq[i]), *s);

        // push vectors in corresponding vector sequences
        xSeq.push_back(std::move(x));
        ySeq.push_back(std::move(y));
        sSeq.push_back(std::move(s));
    }

    auto vectorSequences = std::make_tuple(uSeq, sSeq, ySeq);

    return vectorSequences;
}

/**
 * convertToHSDFByMP()
 * Convert SDF graph to canonical HSDFG according to DAC09 submission procedure.
 */
std::unique_ptr<TimedSDFgraph> Exploration::convertToHSDFByMP() const {
    // calculate the MaxPlus
    std::unique_ptr<Matrix> matrix = this->convertToMaxPlusMatrix();

    // construct the equivalent graph
    std::unique_ptr<TimedSDFgraph> h = nullptr;

    // Construct a new (H)SDF graph
    SDFcomponent component = SDFcomponent(nullptr, 0);
    h = TimedSDFgraph::dynamicCast(g->createCopy(component));

    unsigned int N = matrix->getSize();

    std::vector<TimedSDFactorRef> newActors(static_cast<size_t>((N + 2) * N));
    // make cross actors
    for (unsigned int i = 0; i < N; i++) {
        for (unsigned int j = 0; j < N; j++) {
            // only add an actor if the value is not -infty
            if (!matrix->get(i, j).isMinusInfinity()) {
                // create a new actor
                component = SDFcomponent(h.get(), h->nrActors());
                std::unique_ptr<TimedSDFactor> a = std::make_unique<TimedSDFactor>(*h);
                a->setName(CString("CrossActor_") + CString(i) + CString("_") + CString(j));
                auto p = std::make_unique<TimedSDFactor::Processor>();
                p->execTime = static_cast<CDouble>(matrix->get(i, j));
                p->type = "default";
                p->stateSize = 1;
                // ta = std::dynamic_pointer_cast<TimedSDFactor>(a);
                a->addProcessor(p.get());
                a->setDefaultProcessor("default");
                auto *ar = a.get();
                h->addActor(std::move(a));
                newActors[(i * N) + j] = ar;
                // std::cout << ta->getName();
            } else {
                // -infty, so no actor
                newActors[(i * N) + j] = nullptr;
            }
        }
    }
    // make sync actors for each row/column
    for (unsigned int i = 0; i < N; i++) {
        TimedSDFactorRef a = nullptr;

        // if it has more than 1 outgoing edge, i.e. if more than one of
        // its (k,i)>-inf
        // count the number of entries in column i
        unsigned int edges = 0;
        for (unsigned int k = 0; k < N; k++) {
            if (!matrix->get(k, i).isMinusInfinity()) {
                edges++;
                // pick up one that is not -inf to be used in the else branch for edges  = 1
                a = newActors[(k * N) + i];
            }
        }
        if (edges > 1) {
            // create a new synch actor
            component = SDFcomponent(h.get(), h->nrActors());
            auto ta = std::make_unique<TimedSDFactor>(*h);
            ta->setName(CString("SyncActorL_") + CString(i));
            auto p = std::make_unique<TimedSDFactor::Processor>();
            p->execTime = 0;
            p->type = "default";
            p->stateSize = 1;
            ta->addProcessor(p.get());
            ta->setDefaultProcessor("default");
            auto *tar = ta.get();
            h->addActor(std::move(ta));
            a = tar;
        }
        // newActors[N * N + i] contains null if the entire column is -infty,
        // the single cross actor if only one value is not -infty, or the outgoing sync actor
        newActors[(N * N) + i] = a;

        // if it has more than 1 incoming edge, i.e. if more than one of
        // its (i,k)>-inf
        a = nullptr;
        edges = 0;
        for (unsigned int k = 0; k < N; k++) {
            if (!matrix->get(i, k).isMinusInfinity()) {
                edges++;
                // pick up one that is not -inf to be used in the else branch for edges  = 1
                a = newActors[(i * N) + k];
            }
        }
        if (edges > 1) {
            component = SDFcomponent(h.get(), h->nrActors());
            auto ta = std::make_unique<TimedSDFactor>(*h);
            ta->setName(CString("SyncActorR_") + CString(i));
            auto p = std::make_unique<TimedSDFactor::Processor>();
            p->execTime = 0;
            p->type = "default";
            p->stateSize = 1;
            ta->addProcessor(p.get());
            ta->setDefaultProcessor("default");
            auto *tar = ta.get();
            h->addActor(std::move(ta));
            a = tar;
        }
        // newActors[(N+1) * N + i] contains null if the entire row is -infty,
        // the single cross actor if only one value is not -infty, or the outgoing sync actor
        newActors[((N + 1) * N) + i] = a;
    }

    // add cross edges for evey row/column
    for (unsigned int i = 0; i < N; i++) {
        for (unsigned int j = 0; j < N; j++) {
            // only if there is a cross actor (i,j)
            if (newActors[(i * N) + j] != nullptr) {
                // from j to i
                SDFactorRef srcActor = newActors[(N * N) + j];
                SDFactorRef midActor = newActors[(i * N) + j];
                SDFactorRef dstActor = newActors[((N + 1) * N) + i];

                // if column sync exists
                if (newActors[(N * N) + j] != newActors[(i * N) + j]) {
                    // Create port on source node
                    component = SDFcomponent(srcActor, srcActor->nrPorts());
                    auto srcP = std::make_unique<SDFport>(component);
                    srcP->setName(CString("toCross_") + CString(i));
                    srcP->setRate(1);
                    srcP->setType(SDFport::SDFportType::Out);
                    auto *srcPr = srcP.get();
                    srcActor->addPort(std::move(srcP));

                    // Create column sync port on mid node
                    component = SDFcomponent(midActor, midActor->nrPorts());
                    auto midLP = std::make_unique<SDFport>(component);
                    midLP->setName(CString("fromSync_") + CString(j));
                    midLP->setRate(1);
                    midLP->setType(SDFport::SDFportType::In);
                    auto *midLPr = midLP.get();
                    midActor->addPort(std::move(midLP));

                    // Create channel between left sync and cross
                    component = SDFcomponent(h.get(), h->nrChannels());
                    std::unique_ptr<SDFchannel> c = std::make_unique<TimedSDFchannel>(component);
                    c->setName(CString("Sync_") + CString(j) + CString("_toCross_") + CString(i));
                    c->connectSrc(srcPr);
                    c->connectDst(midLPr);
                    c->setInitialTokens(0);
                    h->addChannel(std::move(c));
                }

                // if row sync exists
                if (newActors[((N + 1) * N) + i] != newActors[(i * N) + j]) {
                    // Create port on dest node
                    component = SDFcomponent(dstActor, dstActor->nrPorts());
                    auto dstP = std::make_unique<SDFport>(component);
                    dstP->setName(CString("fromCross_") + CString(j));
                    dstP->setRate(1);
                    dstP->setType(SDFport::SDFportType::In);
                    auto *dstPr = dstP.get();
                    dstActor->addPort(std::move(dstP));

                    // Create right port on mid node
                    component = SDFcomponent(midActor, midActor->nrPorts());
                    auto midRP = std::make_unique<SDFport>(component);
                    midRP->setName(CString("toSync_") + CString(i));
                    midRP->setRate(1);
                    midRP->setType(SDFport::SDFportType::Out);
                    auto *midRPr = midRP.get();
                    midActor->addPort(std::move(midRP));

                    // Create channel between cross and right sync
                    component = SDFcomponent(h.get(), h->nrChannels());
                    std::unique_ptr<SDFchannel> c = std::make_unique<TimedSDFchannel>(component);
                    c->setName(CString("Cross_") + CString(j) + CString("_toSync_") + CString(i));
                    c->connectSrc(midRPr);
                    c->connectDst(dstPr);
                    c->setInitialTokens(0);
                    h->addChannel(std::move(c));
                }
            }
        }
    }

    // make channels with initial tokens from actor in ((N + 1), i) to actor in (N, i)
    for (unsigned int i = 0; i < N; i++) {
        // from row sync i to column  sync i
        SDFactorRef srcActor = newActors[((N + 1) * N) + i];
        SDFactorRef dstActor = newActors[(N * N) + i];

        // only if bot row and column are not entirely -inf
        if (srcActor != nullptr && dstActor != nullptr) {

            // Create port on source node
            component = SDFcomponent(srcActor, srcActor->nrPorts());
            auto srcP = std::make_unique<SDFport>(component);
            srcP->setName(CString("toInitialTokenEdge_") + CString(i));
            srcP->setRate(1);
            srcP->setType(SDFport::SDFportType::Out);
            auto *srcPr = srcP.get();
            srcActor->addPort(std::move(srcP));

            // Create port on dest node
            component = SDFcomponent(dstActor, dstActor->nrPorts());
            auto dstP = std::make_unique<SDFport>(component);
            dstP->setName(CString("fromInitialEdge_") + CString(i));
            dstP->setRate(1);
            dstP->setType(SDFport::SDFportType::In);
            auto *dstPr = dstP.get();
            dstActor->addPort(std::move(dstP));

            // Create channel between left sync and cross
            component = SDFcomponent(h.get(), h->nrChannels());
            std::unique_ptr<SDFchannel> c = std::make_unique<TimedSDFchannel>(component);
            c->setName(CString("InitialEdge_") + CString(i));
            c->connectSrc(srcPr);
            c->connectDst(dstPr);
            c->setInitialTokens(1);
            h->addChannel(std::move(c));
        }
    }

    return h;
}

/**
 * replaceString()
 * Utility function for substring replacement.
 */
CString &Exploration::replaceString(CString &s, const CString &fnd, const CString &rep) {
    CString::size_type p = 0;
    while ((p = s.find(fnd, 0)) != CString::npos) {
        s = s.std::string::replace(p, fnd.size(), rep);
    }
    return s;
}

/**
 * exploreDelayForSchedule()
 * Compute the minimal delay of a delay-period model for give period, starting
 * schedule and steady-state schedule according to the algorithm presented in
 * the paper "Synchronous Dataflow Scenarios".
 */
MPTime Exploration::exploreDelayForSchedule(Vector *gamma0, Vector *gamma_s, MPTime period) {
    std::unique_ptr<State> gamma = State::fromVector(*G, gamma0);
    std::unique_ptr<State> schedule = State::fromVector(*G, gamma_s);
    MPTime tau = MP_MINUS_INFINITY;
    std::unique_ptr<State> m = nullptr;

    // delta accumulates the total elapsed time minus the number of iterations
    // times period such that the actual schedule equals the normalized schedule
    // + k.period + delta
    MPTime delta = gamma->normalize() - period;

    // do a regular exploration and compute delay tau along the way.
    StateRef y = nullptr;
    auto S = std::make_unique<StoredStates>();
    S->store(gamma->copy());

    // make one iteration to determine the schedule;
    ActorCount E(G->repetitionVector());
    unsigned int sch_length = E.sum();
    staticSchedule = std::make_unique<ActorRef[]>(sch_length);
    unsigned int sch_idx = 0;

    ActorList::const_iterator i = G->actors.begin();
    while (!E.isEmpty()) {
        Actor *a = nullptr;
        // find an arbitrary enabled actor
        while (true) {
            a = i->get();
            if (E.getValue(*a) > 0 && gamma->actorEnabled(*a)) {
                break;
            }
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }
        E.decreaseValue(*a);
        staticSchedule[sch_idx++] = a;
        Graph::fire(*gamma, *a);
    }

    // determine the difference between the reference schedule and the actual
    // schedule
    m = gamma->minus(*schedule);
    tau = MAX(tau, m->norm() + delta);

    // update delta by adding the normalization difference and subtracting
    // period
    delta += Exploration::normalize(*gamma);
    delta -= period;

    // after the first iteration, continue with fixed schedule
    // until a recurrent state is found
    while (!S->includes(gamma, &y)) {
        S->store(gamma->copy());
        (gamma->count)++;
        for (uint k = 0; k < sch_length; k++) {
            Graph::fire(*gamma, *staticSchedule[k]);
        }

        // update tau and delta
        std::unique_ptr<State> m = gamma->minus(*schedule);
        tau = MAX(tau, m->norm() + delta);
        delta += Exploration::normalize(*gamma);
        delta -= period;
    }

    // tau equals the delay we seek.
    return tau;
}

/**
 * exploreEigen()
 * Compute the eigenvector and eigenvalue of the MaxPlus matrix associated with
 * the SDFG.
 */
void Exploration::exploreEigen(std::unique_ptr<Vector> *eigenvector, MPTime *eigenvalue) {
    MPDelay lambda;
    std::unique_ptr<State> gamma = State::zeroState(*G);
    std::unique_ptr<State> gamma_prev = nullptr;

    // make one iteration to determine the schedule;
    ActorCount E(G->repetitionVector());
    unsigned int sch_length = E.sum();
    staticSchedule = std::make_unique<ActorRef[]>(sch_length);
    unsigned int sch_idx = 0;

    ActorList::const_iterator i = G->actors.begin();
    while (!E.isEmpty()) {
        Actor *a = nullptr;
        while (true) // find an enabled actor
        {
            a = i->get();
            if (E.getValue(*a) > 0 && gamma->actorEnabled(*a)) {
                break;
            }
            if (++i == G->actors.end()) {
                i = G->actors.begin();
            }
        }
        E.decreaseValue(*a);
        staticSchedule[sch_idx++] = a;
        Graph::fire(*gamma, *a);
    }
    lambda = Exploration::normalize(*gamma);
    gamma_prev = gamma->copy();

    // continue with fixed schedule
    bool equal = false;
    do {
        for (uint k = 0; k < sch_length; k++) {
            Graph::fire(*gamma, *(staticSchedule[k]));
        }
        lambda = Exploration::normalize(*gamma);
        gamma->smooth(*gamma_prev);
        equal = gamma->equals(*gamma_prev);
        gamma_prev = gamma->copy();
    } while (!equal);
    *eigenvalue = lambda;
    *eigenvector = gamma->asVector();
}

std::unique_ptr<SymbolicState> Exploration::makeSymbolicState() {
    std::unique_ptr<SymbolicState> x = SymbolicState::zeroState(*G);
    return x;
}

std::unique_ptr<Matrix> Exploration::makeMatrix(unsigned int nrows, unsigned int nCols) {
    return std::make_unique<Matrix>(nrows, nCols);
}

std::unique_ptr<Matrix> ExplorationEMR::makeMatrix(unsigned int nrows, unsigned int nCols) {
    return std::make_unique<ExtendedMatrix>(nrows, nCols);
}

std::unique_ptr<SymbolicState> ExplorationEMR::makeSymbolicState() {
    std::unique_ptr<SymbolicState> x = SymbolicStateEMR::zeroState(*G);
    return x;
}
/**
 * normalize()
 */
MPDelay Exploration::normalize(State &x) { return x.normalize(); }

/**
 * timeProgress()
 */
MPDelay Exploration::timeProgress(State *x) {
    // return first entry
    return x->firstToken()->productionTime;
}

/**
 * Port()
 */
Port::Port(unsigned int _rate, Channel *_channel) : rate(_rate), channel(_channel) {}

/**
 * InPort()
 */
InPort::InPort(unsigned int _rate, InputChannel *_inputChannel) :
    rate(_rate), inputChannel(_inputChannel) {}

/**
 * InPort()
 */
OutPort::OutPort(unsigned int _rate, OutputChannel *_outputChannel) :
    rate(_rate), outputChannel(_outputChannel) {}

/**
 * Actor()
 */
Actor::Actor() : index(0) { this->executionTime = MPDelay(0.0); }

/**
 * ~Actor()
 */
Actor::~Actor() = default;

/**
 * ActorCount()
 */
ActorCount::ActorCount(const GraphDecoration::Graph& G) :
    size(G.actors.size()), table(std::make_unique<unsigned int[]>(size)) {}

/**
 * reset()
 */
void ActorCount::reset(const GraphDecoration::Graph& G) {
    size = G.actors.size();
    table = std::make_unique<unsigned int[]>(size);
}


/**
 * ActorCount()
 */
ActorCount::ActorCount(size_t sz) : size(sz), table(std::make_unique<unsigned int[]>(sz)) {}

/**
 * ~ActorCount()
 */
ActorCount::~ActorCount() = default;

ActorCount::ActorCount(const ActorCount &other) :
    size(other.size), table(std::make_unique<unsigned int[]>(other.size)) {
    std::copy(other.table.get(), other.table.get() + other.size, table.get());
}

/**
 * isEmpty()
 */
bool ActorCount::isEmpty() const {
    for (unsigned int i = 0; i < this->size; i++) {
        if (table[i] > 0) {
            return false;
        }
    }
    return true;
}

/**
 * decreaseValue()
 */
void ActorCount::decreaseValue(const Actor &a) { table[a.index]--; }

/**
 * getValue()
 */
unsigned int ActorCount::getValue(const Actor &a) const { return table[a.index]; }

/**
 * setValue()
 */
void ActorCount::setValue(const Actor &a, unsigned int v) { table[a.index] = v; }

/**
 * setValue()
 */
void ActorCount::setValue(unsigned int i, unsigned int v) { table[i] = v; }

/**
 * copy()
 */
std::unique_ptr<ActorCount> ActorCount::copy() {
    auto ac = std::make_unique<ActorCount>(this->size);
    for (uint i = 0; i < this->size; i++) {
        ac->setValue(i, this->table[i]);
    }
    return ac;
}

/**
 * sum()
 */
unsigned int ActorCount::sum() const {
    unsigned int s = 0;
    for (uint i = 0; i < this->size; i++) {
        s += this->table[i];
    }
    return s;
}

/**
 * init()
 */
void GraphDecoration::Graph::init() {};

/**
 * Graph()
 */
GraphDecoration::Graph::Graph() :
    stateSize(0), inputSize(0), outputSize(0), historySize(0), newTokenIndex(0) {
    init();
}

/**
 * ~Graph()
 */
GraphDecoration::Graph::~Graph() = default;

GraphDecoration::Graph::Graph(const Graph &G) :
    actors(G.actors),
    channels(G.channels),
    inputChannels(G.inputChannels),
    outputChannels(G.outputChannels),
    stateSize(G.stateSize),
    inputSize(G.inputSize),
    outputSize(G.outputSize),
    historySize(G.historySize),
    newTokenIndex(G.newTokenIndex),
    repVec(G.repVec) {}

/**
 * Graph()
 */
GraphDecoration::Graph::Graph(const TimedSDFgraph &g, bool weakConsistent, RepetitionVector *q) :
    stateSize(0), inputSize(0), outputSize(0), historySize(0), newTokenIndex(0) {
    init();
    auto channelList = std::make_unique<ChannelRef[]>(g.nrChannels());
    auto inputchannelList = std::make_unique<InputChannel *[]>(g.nrInputChannels());
    auto outputchannelList = std::make_unique<OutputChannel *[]>(g.nrOutputChannels());

    RepetitionVector E;
    if (!weakConsistent) {
        E = computeRepetitionVector(g);
    } else {
        E = *q;
    }

    // copy channels
    int stateIndex = 0;
    int historyIndex = 0;
    int k = 0;
    for (auto j = g.channelsBegin(); j != g.channelsEnd(); j++, k++) {
        auto *t = dynamic_cast<TimedSDFchannel *>((*j).get());
        auto c = std::make_unique<Channel>();
        c->isStorageChannel = t->isCapacityChannel;
        c->nrInitialTokens = t->getInitialTokens();
        c->index = k;
        SDFactorRef a = t->getSrcActor();
        int m = 0;
        for (auto i = g.actorsBegin(); i != g.actorsEnd(); i++, m++) {
            const std::unique_ptr<SDFactor> &b = *i;
            if (a == b.get()) {
                c->synchStorage = t->getInitialTokens() + E.at(m) * t->getSrcPort()->getRate();
                c->repetitionVectorEntry = E.at(m) * t->getSrcPort()->getRate();
                break;
            }
            assert(i != g.actorsEnd());
        }
        c->stateStartIndex = stateIndex;
        stateIndex += static_cast<int>(c->nrInitialTokens);
        c->historyStartIndex = historyIndex;
        if (c->nrInitialTokens > c->repetitionVectorEntry) {
            historyIndex += static_cast<int>(c->nrInitialTokens)
                            - static_cast<int>(c->repetitionVectorEntry);
        }

        channelList[k] = c.get();
        this->channels.push_back(std::move(c));
    }
    this->stateSize = stateIndex;
    this->historySize = historyIndex;

    // copy input channels
    int inputIndex = 0;
    int ik = 0;
    for (auto ij = g.inputchannelsBegin(); ij != g.inputchannelsEnd(); ij++, ik++) {
        auto *it = dynamic_cast<SDFinputchannel *>((*ij).get());
        auto ic = std::make_unique<InputChannel>();
        // ic->nrInitialTokens = it->getInitialTokens();
        ic->index = ik;
        SDFactorRef ia = it->getDstActor();
        SDFactorsCIter ii;
        int im = 0;
        for (ii = g.actorsBegin(); ii != g.actorsEnd(); ii++, im++) {
            const std::unique_ptr<SDFactor> &ib = *ii;
            if (ia == ib.get()) {
                ic->repetitionVectorEntry = E.at(im) * it->getDstPort()->getRate();
                break;
            }
            assert(ii != g.actorsEnd());
        }
        ic->inputStartIndex = inputIndex;
        inputIndex += static_cast<int>(ic->repetitionVectorEntry);

        inputchannelList[ik] = ic.get();
        this->inputChannels.push_back(std::move(ic));
    }
    this->inputSize = inputIndex;

    // Copy output channels
    int outputIndex = 0;
    int ok = 0;
    for (auto oj = g.outputchannelsBegin(); oj != g.outputchannelsEnd(); oj++, ok++) {
        auto *ot = dynamic_cast<SDFoutputchannel *>((*oj).get());
        auto oc = std::make_unique<OutputChannel>();
        // ic->nrInitialTokens = it->getInitialTokens();
        oc->index = ok;
        SDFactorRef oa = ot->getSrcActor();
        SDFactorsCIter oi;
        int om = 0;
        for (oi = g.actorsBegin(); oi != g.actorsEnd(); oi++, om++) {
            const std::unique_ptr<SDFactor> &ob = *oi;
            if (oa == ob.get()) {
                oc->repetitionVectorEntry = E.at(om) * ot->getSrcPort()->getRate();
                break;
            }
            assert(oi != g.actorsEnd());
        }
        oc->outputStartIndex = outputIndex;
        outputIndex += static_cast<int>(oc->repetitionVectorEntry);

        outputchannelList[ok] = oc.get();
        this->outputChannels.push_back(std::move(oc));
    }
    this->outputSize = outputIndex;

    // copy actors
    k = 0;
    for (auto i = g.actorsBegin(); i != g.actorsEnd(); i++, k++) {
        auto *t = dynamic_cast<TimedSDFactor *>((*i).get());
        auto a = std::make_unique<Actor>();
        a->executionTime = MPTime(t->getExecutionTime());

        SDFportsIter pi;
        for (pi = t->portsBegin(); pi != t->portsEnd(); pi++) {
            const SDFport* p = pi->get();

            int m = 0;
            SDFchannelsCIter ci;
            for (ci = g.channelsBegin(); ci != g.channelsEnd(); ci++, m++) {
                const std::unique_ptr<SDFchannel> &c = *ci;
                if (c.get() == p->getChannel()) {
                    break;
                }
            }

            if (ci != g.channelsEnd()) {
                std::unique_ptr<Port> np = std::make_unique<Port>(p->getRate(), channelList[m]);
                if (p->getType() == SDFport::SDFportType::In) {
                    a->inputPorts.push_back(std::move(np));
                } else {
                    a->outputPorts.push_back(std::move(np));
                }
            }

            int im = 0;
            SDFinputchannelsCIter ici;
            for (ici = g.inputchannelsBegin(); ici != g.inputchannelsEnd(); ici++, im++) {
                const std::unique_ptr<SDFinputchannel> &ic = *ici;
                if (ic.get() == p->getInputChannel()) {
                    break;
                }
            }

            if (ici != g.inputchannelsEnd()) {

                auto np = std::make_unique<InPort>(p->getRate(), inputchannelList[im]);

                if (p->getType() == SDFport::SDFportType::In) {
                    a->InPorts.push_back(std::move(np));
                }
            }

            int om = 0;
            SDFoutputchannelsCIter oci;
            for (oci = g.outputchannelsBegin(); oci != g.outputchannelsEnd(); oci++, om++) {
                const std::unique_ptr<SDFoutputchannel> &oc = *oci;
                if (oc.get() == p->getOutputChannel()) {
                    break;
                }
            }

            if (oci != g.outputchannelsEnd()) {

                auto np = std::make_unique<OutPort>(p->getRate(), outputchannelList[om]);

                if (p->getType() == SDFport::SDFportType::Out) {
                    a->OutPorts.push_back(std::move(np));
                }
            }

            assert(ci != g.channelsEnd() || ici != g.inputchannelsEnd()
                   || oci != g.outputchannelsEnd());
        }
        a->index = k;
        if (t->getType() == "output") {
            this->outputActors.push_back(a.get());
        }
        this->actors.push_back(std::move(a));
    }

    RepetitionVector rv;
    if (!weakConsistent) {
        rv = computeRepetitionVector(g);
    } else {
        rv = *q;
    }
    this->repVec.reset(*this);

    RepetitionVector::iterator v;
    int m = 0;
    for (m = 0, v = rv.begin(); v != rv.end(); v++, m++) {
        int c = *v;
        this->repVec.setValue(m, c);
    }
}

std::pair<std::unique_ptr<Graph>, std::unique_ptr<Graph>>
GraphDecoration::Graph::OpenGraphConversion(const TimedSDFgraph &g) {

    auto OG = std::make_unique<GraphDecoration::Graph>(g);
    auto G = std::make_unique<GraphDecoration::Graph>(g);

    // Convert input channels to channels

    G->stateSize += OG->inputSize;

    unsigned int m = 0;
    auto n = static_cast<unsigned int>(OG->channels.size());
    unsigned int repetition = 0;
    unsigned int stateIndex =
            OG->channels.at(n - 1)->stateStartIndex + OG->channels.at(n - 1)->nrInitialTokens;
    for (m = 0; m != OG->inputChannels.size(); m++, n++) {

        auto c = std::make_unique<Channel>();
        c->index = n;
        repetition = G->inputChannels.at(m)->repetitionVectorEntry;
        c->nrInitialTokens = repetition;
        c->repetitionVectorEntry = repetition;
        c->synchStorage = repetition;
        c->stateStartIndex = stateIndex;
        stateIndex += repetition;
        G->channels.push_back(std::move(c));
    }

    // Convert output channels to channel

    for (m = 0; m != OG->outputChannels.size(); m++, n++) {

        auto c = std::make_unique<Channel>();
        c->index = n;
        repetition = G->outputChannels.at(m)->repetitionVectorEntry;
        c->nrInitialTokens = 0;
        c->repetitionVectorEntry = repetition;
        c->synchStorage = repetition;
        // c->stateStartIndex = 0;
        // stateIndex += repetition;
        G->channels.push_back(std::move(c));
    }

    // Convert connected ports to input channels to the input ports

    unsigned int rate = 0;
    unsigned int index = 0;
    for (m = 0; m != OG->actors.size(); m++) {
        for (n = 0; n != OG->actors.at(m)->InPorts.size(); n++) {
            rate = OG->actors.at(m)->InPorts.at(n)->rate;
            index = OG->actors.at(m)->InPorts.at(n)->inputChannel->index;
            index += static_cast<unsigned int>(OG->channels.size());
            auto p = std::make_unique<Port>(rate, G->channels.at(index).get());
            G->actors.at(m)->inputPorts.push_back(std::move(p));
        }
    }

    // Convert connected ports to output channels to the output ports

    for (m = 0; m != OG->actors.size(); m++) {
        for (n = 0; n != OG->actors.at(m)->OutPorts.size(); n++) {
            rate = OG->actors.at(m)->OutPorts.at(n)->rate;
            index = OG->actors.at(m)->OutPorts.at(n)->outputChannel->index;
            index += static_cast<unsigned int>(OG->channels.size() + OG->inputChannels.size());
            auto p = std::make_unique<Port>(rate, G->channels.at(index).get());
            G->actors.at(m)->outputPorts.push_back(std::move(p));
        }
    }

    std::pair<std::unique_ptr<Graph>, std::unique_ptr<Graph>> Graphs = std::make_pair(std::move(OG), std::move(G));
    return Graphs;
}

/**
 * fire()
 */
inline void GraphDecoration::Graph::fire(State &x, const Actor &a) {
    MPTime time;
    MPTime ftime;
    const PortList *pl = &(a.inputPorts);
    for (auto i = pl->begin(); i != pl->end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        time = x.consume(*p);
        if (i != pl->begin()) {
            ftime = MAX(time, ftime);
        } else {
            ftime = time;
        }
    }
    time = ftime + a.executionTime;
    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        x.produce(*p, time);
    }
}

/**
 * fire()
 */
void GraphDecoration::Graph::fire(State &x, const Actor &a, double precision) {
    MPTime time;
    auto ftime = MPTime(0);

    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        time = x.consume(*p);
        if (i != a.inputPorts.begin()) {
            ftime = MAX(time, ftime);
        } else {
            ftime = time;
        }
    }
    time = ftime + a.executionTime;
    time = MPTime(
            precision
            * static_cast<double>(static_cast<int32_t>(static_cast<CDouble>(time) / precision)));
    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        x.produce(*p, time);
    }
}

std::unique_ptr<SymbolicToken> Graph::makeMinusInfinityToken(unsigned int s) {
    return SymbolicToken::minusInfinityToken(s);
}

std::unique_ptr<SymbolicToken> GraphEMR::makeMinusInfinityToken(unsigned int s) {
    return SymbolicTokenEMR::minusInfinityToken(s);
}

/**
 * fire_reverse()
 */
void GraphDecoration::Graph::fire_reverse(State &x, const Actor &a) {
    MPTime time;
    auto ftime = MPTime(0);

    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        time = x.consume_back(*p);
        if (i != a.outputPorts.begin()) {
            ftime = MIN(time, ftime);
        } else {
            ftime = time;
        }
    }
    time = ftime - a.executionTime;
    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        x.produce_front(*p, time);
    }
}

/**
 * fireSymbolic()
 */
void GraphDecoration::Graph::fireSymbolic(SymbolicState &x, const Actor &a) {
    std::shared_ptr<SymbolicToken> time = nullptr;
    std::shared_ptr<SymbolicToken> fTime = nullptr;
    std::shared_ptr<SymbolicToken> nfTime = nullptr;

    // initialize the actor start time to minus infinity
    fTime = makeMinusInfinityToken(this->stateSize + this->inputSize);

    // consume from all input ports and compute max with time stamps to determine actor start time
    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        time = x.consume(*p);

        // compute max
        nfTime = time->maxWith(*fTime);

        // remember the max
        fTime = nfTime;
    }

    // compute the symbolic completion time of the actor
    time = fTime->add(a.executionTime);
    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        if (p->rate != 0) {
            x.produce(*p, time->copy());
        }
    }
}

/**
 * fireSymbolicCaptureTracingInfo()
 */
void GraphDecoration::Graph::fireSymbolicCaptureTracingInfo(
        SymbolicState &x, const Actor &a, Matrix *MH, unsigned int actorFiringsCounter) const {
    std::shared_ptr<SymbolicToken> time = nullptr;
    std::shared_ptr<SymbolicToken> fTime = nullptr;
    std::shared_ptr<SymbolicToken> nfTime = nullptr;
    PortList::iterator i;

    // initialize the actor start time to minus infinity
    fTime = SymbolicToken::minusInfinityToken(this->stateSize);

    // consume from all input ports and compute max with time stamps to determine actor start time
    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        time = x.consume(*p);

        // compute max
        nfTime = time->maxWith(*fTime);

        // remember the max
        fTime = std::move(nfTime);
    }
    // Capture start time of firing as symbolic tokens
    for (unsigned int j = 0; j < MH->getCols(); j++) {
        MPTime xx = (*fTime)[j]->productionTime;
        MH->put(actorFiringsCounter, j, xx);
    }

    // compute the symbolic completion time of the actor
    time = fTime->add(a.executionTime);

    // Capture endtime of firing as symbolic tokens
    for (unsigned int j = 0; j < MH->getCols(); j++) {
        MPTime xx = (*time)[j]->productionTime;
        MH->put(actorFiringsCounter + 1, j, xx);
    }
    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;

        if (p->rate != 0) {
            x.produce(*p, time->copy());
        }
    }
}

/**
 * fireSymbolicCaptureOutputMatrix()
 */
void GraphDecoration::Graph::fireSymbolicCaptureOutputMatrix(SymbolicState &x,
                                                             const Actor &a,
                                                             Matrix &MO) const {
    std::shared_ptr<SymbolicToken> time = nullptr;
    std::shared_ptr<SymbolicToken> fTime = nullptr;
    std::shared_ptr<SymbolicToken> nfTime = nullptr;

    // initialize the actor start time to minus infinity
    fTime = SymbolicToken::minusInfinityToken(this->stateSize);

    // consume from all input ports and compute max with time stamps to determine actor start time
    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;
        time = x.consume(*p);

        // compute max
        nfTime = time->maxWith(*fTime);

        // remember the max
        fTime = std::move(nfTime);
    }

    // compute the symbolic completion time of the actor
    time = fTime->add(a.executionTime);

    // Capture endtime of firing as symbolic tokens if the actor is an output actor
    bool isOutputActor = std::find_if(outputActors.begin(),
                                      outputActors.end(),
                                      [&a](auto ap) { return ap == &a; })
                         != outputActors.end();
    if (isOutputActor) {
        MO.addRows(1);
        for (unsigned int j = 0; j < MO.getCols(); j++) {
            MPTime xx = (*time)[j]->productionTime;
            MO.put(MO.getRows() - 1, j, xx);
        }
    }

    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const std::unique_ptr<Port> &p = *i;

        if (p->rate != 0) {
            x.produce(*p, time->copy());
        }
    }
}

/**
 * repetitionVector()
 */
const ActorCount& GraphDecoration::Graph::repetitionVector() const {
    return this->repVec;
}

/**
 * mpStateSpaceThroughputAnalysis()
 */
MPTime mpStateSpaceThroughputAnalysis(const TimedSDFgraph &g, std::ostream &sout) {
    out = &sout;
    auto G = std::make_unique<GraphDecoration::Graph>(g);
    Exploration mpe;
    mpe.G = std::make_unique<GraphDecoration::Graph>(*G);
    mpe.OG = std::move(G);
    MPThroughput th = mpe.explore();
    return MPTime(th);
}

/**
 * mpStateSpaceThroughputAnalysis_backward()
 */
MPTime mpStateSpaceThroughputAnalysis_backward(const TimedSDFgraph &g, std::ostream &sout) {
    out = &sout;
    auto G = std::make_unique<GraphDecoration::Graph>(g);
    Exploration mpe;
    mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
    mpe.G = std::move(G);
    MPThroughput th = mpe.explore_backward();
    return MPTime(th);
}

/**
 * mpStateSpaceDelayAnalysis()
 */
MPTime mpStateSpaceDelayAnalysis(const TimedSDFgraph &g,
                                 Vector *gamma0,
                                 Vector *gamma_s,
                                 MPTime period,
                                 std::ostream &sout) {
    out = &sout;
    auto G = std::make_unique<GraphDecoration::Graph>(g);
    Exploration mpe;
    mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
    mpe.G = std::move(G);
    MPTime d = mpe.exploreDelayForSchedule(gamma0, gamma_s, period);
    return d;
}

/**
 * mpStateSpaceEigenAnalysis()
 */
void mpStateSpaceEigenAnalysis(const TimedSDFgraph &g,
                               std::unique_ptr<Vector> *eigenvector,
                               MPTime *eigenvalue,
                               std::ostream &sout) {
    out = &sout;
    auto G = std::make_unique<GraphDecoration::Graph>(g);
    Exploration mpe;
    mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
    mpe.G = std::move(G);
    mpe.exploreEigen(eigenvector, eigenvalue);
}

} // namespace SDF::MaxPlusAnalysis
