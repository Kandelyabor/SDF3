/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer_ning_gao.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 31, 2008
 *
 *  Function        :   State-space based buffer size analysis for Ning and
 *                      Gao's buffer sizing problem
 *
 *  History         :
 *      31-03-08    :   Initial version.
 *
 * $Id: buffer_ning_gao.cc,v 1.3.2.2 2010-04-24 23:52:42 mgeilen Exp $
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

#include "buffer_ning_gao.h"
#include "base/exception/exception.h"
#include "sdf/analysis/throughput/selftimed_throughput.h"
#include "sdf/base/algo/repetition_vector.h"
#include "types.h"
#include <algorithm>
#include <cfloat> // IWYU pragma: keep
#include <cmath>
#include <cstdint>
#include <memory>

namespace SDF {
/******************************************************************************
 * Bounds on the search space
 *****************************************************************************/

/**
 * initBoundsSearchSpace ()
 * Compute bounds on the trade-off space that must be explored.
 */
void SDFstateSpaceBufferAnalysisNingGao::initBoundsSearchSpace(TimedSDFgraph *g) {
    initMinimalChannelSzStep(g);
    initMinimalChannelSz(g);
    initLbDistributionSz(g);
    initMaxThroughput(g);
}

/**
 * initMinimalChannelSzStep ()
 * Compute lower bound on the step size of channels
 */
void SDFstateSpaceBufferAnalysisNingGao::initMinimalChannelSzStep(TimedSDFgraph *g) {
    minSzStep = std::make_unique<TBufSize[]>(g->nrActors());

    for (uint a = 0; a < g->nrActors(); a++) {
        minSzStep[a] = 1;
    }
}

/**
 * initMinimalChannelSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBufferAnalysisNingGao::initMinimalChannelSz(TimedSDFgraph *g) {
    minSz = std::make_unique<TBufSize[]>(g->nrActors());

    // Initialize storage space of all actors to one
    for (uint a = 0; a < g->nrActors(); a++) {
        minSz[a] = 1;
    }

    // Increase storage space of actors that have more initial tokens than 1
    for (const auto &ch : g->getChannels()) {
        minSz[ch->getSrcActor()->getId()] =
                (std::max)(ch->getInitialTokens(), minSz[ch->getSrcActor()->getId()]);
    }
}

/**
 * initLbDistributionSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBufferAnalysisNingGao::initLbDistributionSz(TimedSDFgraph *g) {
    lbDistributionSz = 0;

    for (uint a = 0; a < g->nrActors(); a++) {
        lbDistributionSz += minSz[a];
    }
}

/**
 * initMaxThroughput ()
 * Compute the maximal throughput that can be achieved by the graph.
 */
void SDFstateSpaceBufferAnalysisNingGao::initMaxThroughput(TimedSDFgraph *g) {
    SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;

    maxThroughput = SDF::SDFstateSpaceThroughputAnalysis::analyze(*g);
}

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::State::print(std::ostream &out) {
    out << "### State ###" << std::endl;

    for (uint i = 0; i < actClk.size(); i++) {
        out << "actClk[" << i << "] =";

        for (auto iter = actClk[i].begin(); iter != actClk[i].end(); iter++) {
            out << " " << (*iter) << ", ";
        }

        out << std::endl;
    }

    for (uint i = 0; i < ch.size(); i++) {
        out << "ch[" << i << "] = " << ch[i] << std::endl;
    }

    for (uint i = 0; i < sp.size(); i++) {
        out << "sp[" << i << "] = " << ch[i] << std::endl;
    }

    out << "glbClk = " << glbClk << std::endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::State::clear() {
    for (auto &i : actClk) {
        i.clear();
    }

    for (uint32_t &i : ch) {
        i = 0;
    }

    for (uint32_t &i : sp) {
        i = 0;
    }

    glbClk = 0;
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::State::operator==(const State &s) {
    if (glbClk != s.glbClk) {
        return false;
    }

    for (uint i = 0; i < ch.size(); i++) {
        if (ch[i] != s.ch[i]) {
            return false;
        }
    }

    for (uint i = 0; i < sp.size(); i++) {
        if (sp[i] != s.sp[i]) {
            return false;
        }
    }

    for (uint i = 0; i < actClk.size(); i++) {
        if (actClk[i] != s.actClk[i]) {
            return false;
        }
    }

    return true;
}

/******************************************************************************
 * Transition system
 *****************************************************************************/

/**
 * initOutputActor ()
 * The function selects an actor to be used as output actor in the
 * state transition system.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::initOutputActor() {
    RepetitionVector repVec;
    int min = INT_MAX;
    const SDFactor* a = nullptr;

    // Compute repetition vector
    repVec = computeRepetitionVector(*g);

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto &iter : g->getActors()) {
        if (repVec[iter->getId()] < min) {
            a = iter.get();
            min = repVec[a->getId()];
        }
    }

    // Set output actor and its repetition vector count
    outputActor = a;
    outputActorRepCnt = repVec[outputActor->getId()];
}

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::storeState(State &s, StatesIter &pos) {
    // Find state in the list of stored states
    for (auto iter = storedStates.begin(); iter != storedStates.end(); iter++) {
        State &x = *iter;

        // State s at position iter in the list?
        if (x == s) {
            pos = iter;
            return false;
        }
    }

    // State not found, store it at the end of the list
    storedStates.push_back(s);

    // Added state to the end of the list
    pos = storedStates.end();

    return true;
}

/**
 * computeThroughput ()
 * The function calculates the throughput of the states on the cycle. Its
 * value is equal to the average number of firings of an actor per time unit.
 */
TDtime SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::computeThroughput(
        const StatesIter &cycleIter) {
    int nr_fire = 0;
    TDtime time = 0;

    // Check all state from stack till cycle complete
    for (StatesIter iter = cycleIter; iter != storedStates.end(); iter++) {
        State &s = *iter;

        // Number of states in cycle is equal to number of iterations
        // in the period
        nr_fire++;

        // Time between previous state
        time += s.glbClk;
    }

    return static_cast<TDtime>(nr_fire) / (time);
}

/******************************************************************************
 * Dependencies
 *****************************************************************************/

/**
 * dfsVisitDependencies ()
 * The function performs a DFS on the abstract dependency graph from a node a to
 * find all cycles of which a is part. Channels on a cycle are when needed
 * marked to have a storage dependency.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::dfsVisitDependencies(
        uint a,
        int *color,
        int *pi,
        const TDependencyGraph &abstractDepGraph,
        const TDependencies &dep) {
    uint c = 0;
    uint d = 0;

    // Mark node a as visited
    color[a] = 1;

    // Visit all nodes reachable from a
    for (uint b = 0; b < g->nrActors(); b++) {
        // Node b reachable from a?
        if (abstractDepGraph[a][b]) {
            // Visited node b before?
            if (color[b] == 1) {
                // Found a cycle in the graph containing node b
                c = a;
                d = b;
                do {
                    // All channels from d to c in the SDFG have
                    // storage dependency
                    for (const auto &ch : g->getChannels()) {
                        CId srcId = ch->getSrcActor()->getId();
                        CId dstId = ch->getDstActor()->getId();

                        if (dstId == d && srcId == c) {
                            dep[ch->getId()] = true;
                        }
                    }

                    // Next
                    d = c;
                    c = pi[d];
                } while (d != b);
            } else {
                // Visit node b
                pi[b] = static_cast<int>(a);
                dfsVisitDependencies(b, color, pi, abstractDepGraph, dep);
            }
        }
    }

    // Discovered all cycles which actor a is part of. Remove its edges to avoid
    // re-discovering same cycles again.
    for (uint i = 0; i < g->nrActors(); i++) {
        abstractDepGraph[i][a] = false;
        abstractDepGraph[a][i] = false;
    }

    // Mark node a as not visited
    color[a] = 0;
}

/**
 * findStorageDependencies ()
 * The function find all cycles in the abstract dependency graph. Any channel
 * modeling storage space which is part of a cycle is marked to have a storage
 * dependency.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem ::findStorageDependencies(
        const TDependencyGraph &abstractDepGraph, const TDependencies &dep) {

    // Initialize DFS data structures
    auto color = std::make_unique<int[]>(g->nrActors());
    auto pi = std::make_unique<int[]>(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        color[i] = 0;
    }

    // Initialize storage dependencies
    for (uint c = 0; c < g->nrChannels(); c++) {
        dep[c] = false;
    }

    // DFS from every node in the graph to find all cycles
    for (int i = 0; i < g->nrActors(); i++) {
        pi[i] = i;
        dfsVisitDependencies(i, color.get(), pi.get(), abstractDepGraph, dep);
    }
}

/******************************************************************************
 * SDF
 *****************************************************************************/

// NOLINTBEGIN(*-macro-usage)

#define CH(c) currentState.ch[c]
#define SP(a) currentState.sp[a]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CH_SPACE(a, n) (SP(a) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);
#define CONSUME_SP(a, n) SP(a) = SP(a) - (n);
#define PRODUCE_SP(a, n) SP(a) = SP(a) + (n);

#define CH_TOKENS_PREV(c, n) (previousState.ch[c] >= (n))
#define CH_SPACE_PREV(a, n) (previousState.sp[a] >= (n))

// NOLINTEND(*-macro-usage)

/**
 * releaseStorageSpaceSharedOutputBuffer ()
 * The function checks whether space must be produced on the shared output
 * buffer. This is the case when the channel 'c' is the last channel to consume
 * the token from the shared output buffer connected to the source of the
 * channel. This condition is met when all outgoing channels of the source actor
 * of the channel 'c' contain less tokens than the channel 'c'.
 */
bool SDFstateSpaceBufferAnalysisNingGao::TransitionSystem ::releaseStorageSpaceSharedOutputBuffer(
        const SDFchannel &c) {
    SDFactor *a = c.getSrcActor();

    // Check all output ports for tokens

    const auto &ports = a->getPorts();
    // check that for all output ports to channels other than c, the port rate
    // is smaller than the number of tokens on the channel
    return std::all_of(ports.begin(), ports.end(), [this, &c](const auto &p) {
        return
                // Actor is source of the channel?
                (p->getType() != SDFport::SDFportType::Out)
                || (p->getChannel()->getId() == c.getId())
                || (CH(p->getChannel()->getId()) < CH(c.getId()));
    });
}

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */

// for (const auto& p : a->getPorts()) {
//     const SDFchannel& c = *(p->getChannel());

//     // Actor is destination of the channel?
//     if (p->getType() == SDFport::SDFportType::In) {
//         if (!CH_TOKENS(c->getId(), p->getRate())) {
//             return false;
//         }
//     } else {
//         if (!CH_SPACE(c->getSrcActor()->getId(), p->getRate())) {
//             return false;
//         }
//     }
// }

bool SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::actorReadyToFire(const SDFactor &a) {
    // Check all input ports for tokens
    const auto &ports = a.getPorts();
    return std::all_of(ports.begin(), ports.end(), [this](const auto &p) {
        const SDFchannel &c = *(p->getChannel());
        if (p->getType() == SDFport::SDFportType::In) {
            return CH_TOKENS(c.getId(), p->getRate());
        }
        return CH_SPACE(c.getSrcActor()->getId(), p->getRate());
    });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::startActorFiring(
        const TimedSDFactor &a) {
    bool consumedOutputSpace = false;

    // Consume tokens from inputs and space for output tokens
    for (const auto &p : a.getPorts()) {
        const SDFchannel &c = *(p->getChannel());

        // Actor is destination of the channel?
        if (p->getType() == SDFport::SDFportType::In) {
            // Space must be produced in case this is the last channel
            // to consume the token from the shared output buffer connected
            // to the source of the channel. This condition is met when all
            // outgoing channels of the source actor of the channel 'c' contain
            // less tokens than the channel 'c' contains before the upcoming
            // consumption.
            if (releaseStorageSpaceSharedOutputBuffer(c)) {
                PRODUCE_SP(c.getSrcActor()->getId(), p->getRate());
            }

            // Consume the tokens from the channel
            CONSUME(c.getId(), p->getRate());
        } else {
            // No space consumed for the output yet?
            if (!consumedOutputSpace) {
                CONSUME_SP(c.getSrcActor()->getId(), p->getRate());
                consumedOutputSpace = true;
            }
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a.getId()].push_back(a.getDiscreteExecutionTime());
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::actorReadyToEnd(const SDFactor &a) {
    if (currentState.actClk[a.getId()].empty()) {
        return false;
    }

    // First actor firing in sorted list has execution time left?
    if (currentState.actClk[a.getId()].front() != 0) {
        return false;
    }

    return true;
}

/**
 * endActorFiring ()
 * Produce tokens on all output channels and remove the actor firing from the
 * list of active firings.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::endActorFiring(const SDFactor &a) {
    for (const auto &p : a.getPorts()) {
        const SDFchannel &c = *(p->getChannel());

        // Actor is source of the channel?
        if (p->getType() == SDFport::SDFportType::Out) {
            PRODUCE(c.getId(), p->getRate());
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a.getId()].pop_front();
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFdiscreteTime SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::clockStep() {
    SDFdiscreteTime step = UINT_MAX;

    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++) {
        if (!currentState.actClk[a].empty()) {
            SDFdiscreteTime actClk = currentState.actClk[a].front();

            step = (std::min)(step, actClk);
        }
    }

    // Still actors ready to end their firing?
    if (step == 0) {
        return 0;
    }

    // Check for progress (i.e. no deadlock)
    if (step == UINT_MAX) {
        return UINT_MAX;
    }

    // Lower remaining execution time actors
    for (uint a = 0; a < g->nrActors(); a++) {
        for (auto &actFiringTime : currentState.actClk[a]) {
            // Lower remaining execution time of the actor firing
            actFiringTime -= step;
        }
    }

    // Advance the global clock
    currentState.glbClk += step;

    return step;
}

/**
 * findCausalDependencies ()
 * The function tracks all causal dependencies in the actor firing of actor a.
 * Any causal dependency that is found is added to the abstract dependency
 * graph.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem ::findCausalDependencies(
        const SDFactor &a, const TDependencyGraph &abstractDepGraph) {
    // Check all input ports for tokens and output ports for space
    for (const auto &p : a.getPorts()) {
        const SDFchannel &c = *(p->getChannel());
        SDFactor *srcActor = c.getSrcActor();
        SDFactor *dstActor = c.getDstActor();

        // Actor is destination of the channel?
        if (p->getType() == SDFport::SDFportType::In) {
            // Not enough tokens in the previous state?
            if (!CH_TOKENS_PREV(c.getId(), p->getRate())) {
                abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
            }
        } else {
            // Not enough space in the previous state?
            if (!CH_SPACE_PREV(c.getSrcActor()->getId(), p->getRate())) {
                abstractDepGraph[srcActor->getId()][dstActor->getId()] = true;
            }
        }
    }
}

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find all blocked channels. This
 * is done using the abstract dependency graph.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::analyzePeriodicPhase(
        const std::unique_ptr<TBufSize[]> & /* sp */, const TDependencies &dep) {
    State periodicState(g->nrActors(), g->nrChannels());
    bool startedActorFiring = false;
    int repCnt = 0;

    // Current state is a periodic state
    periodicState = currentState;

    // Abstract dependency graph
    auto abstractDepGraph = std::make_unique<TDependencies[]>(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        abstractDepGraph[i] = std::make_unique<bool[]>(g->nrActors());
        for (uint j = 0; j < g->nrActors(); j++) {
            abstractDepGraph[i][j] = false;
        }
    }

    // Start new iteration of the periodic phase
    currentState.glbClk = 0;

    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (const auto &a : g->getActors()) {
        while (actorReadyToEnd(*a)) {
            if (a->getId() == outputActor->getId()) {
                repCnt++;
                if (repCnt == outputActorRepCnt) {
                    currentState.glbClk = 0;
                    repCnt = 0;
                }
            }

            endActorFiring(*a);
        }
    }

    // Fire the actors
    while (true) {
        // Start actor firings - repeat till no start enables another start
        do {
            startedActorFiring = false;

            for (const auto &iter : g->getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

                // Ready to fire actor a?
                while (actorReadyToFire(*a)) {
                    // Track causal dependencies on firing of actor a
                    findCausalDependencies(*a, abstractDepGraph);

                    // Fire actor a
                    startActorFiring(*a);
                    startedActorFiring = true;
                }
            }
        } while (startedActorFiring);

        // Clock step
        clockStep();

        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }
        for (uint i = 0; i < g->nrActors(); i++) {
            previousState.sp[i] = currentState.sp[i];
        }

        // Finish actor firings
        for (const auto &a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Found periodic state
                        if (currentState == periodicState) {
                            // Cycles in the dependency graph indicate storage
                            // dependencies
                            findStorageDependencies(abstractDepGraph, dep);

                            // Done
                            return;
                        }
                        currentState.glbClk = 0;
                        repCnt = 0;
                    }
                }

                // End the actor firing
                endActorFiring(*a);
            }
        }
    }
}

/**
 * analyzeDeadlock ()
 * Analyze the deadlock in the schedule.
 */
void SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::analyzeDeadlock(
        const std::unique_ptr<TBufSize[]> & /*sp*/, const TDependencies &dep) {
    // Abstract dependency graph
    auto abstractDepGraph = std::make_unique<TDependencies[]>(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        abstractDepGraph[i] = std::make_unique<bool[]>(g->nrActors());
        for (uint j = 0; j < g->nrActors(); j++) {
            abstractDepGraph[i][j] = false;
        }
    }

    // Check number of tokens on every channel in the graph
    for (const auto &c : g->getChannels()) {
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();

        // Insufficient tokens to fire destination actor
        if (!CH_TOKENS(c->getId(), c->getDstPort()->getRate())) {
            abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
        }

        // Insufficient space to fire source actor
        if (!CH_SPACE(c->getSrcActor()->getId(), c->getSrcPort()->getRate())) {
            abstractDepGraph[srcActor->getId()][dstActor->getId()] = true;
        }
    }

    // Cycles in the dependency graph indicate storage dependencies
    findStorageDependencies(abstractDepGraph, dep);
}

/**
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime SDFstateSpaceBufferAnalysisNingGao::TransitionSystem::execSDFgraph(
        const std::unique_ptr<TBufSize[]> &sp,
        const TDependencies &dep,
        std::vector<SDFdiscreteTime> &startTime) {
    StatesIter recurrentState;
    bool startedActorFiring = false;
    std::vector<uint> iterCnt;
    SDFdiscreteTime minStartTime = 0;
    TTime globalTime = 0;
    uint maxIterCnt = 0;
    TTime clkStep = 0;
    int repCnt = 0;
    TDtime thr = NAN;

    // startTime gives the last time at which an actor firing occurred
    startTime.resize(g->nrActors());

    // iterCnt gives the number of firings of each actor that have occurred
    iterCnt.resize(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        iterCnt[i] = 0;
    }

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();
    previousState.init(g->nrActors(), g->nrChannels());
    previousState.clear();

    // Infinite space for output buffer of actors
    for (uint i = 0; i < g->nrActors(); i++) {
        SP(i) = TBUFSIZE_MAX;
    }

    // Initial tokens and space
    for (const auto &c : g->getChannels()) {
        // Not enough space for initial tokens?
        if (sp[c->getSrcActor()->getId()] < c->getInitialTokens()) {
            dep[c->getId()] = true;
            return 0;
        }

        CH(c->getId()) = c->getInitialTokens();

        // Decrease storage space of the actor when current storage space more
        // then room offered by assigned storage space minus the number of
        // tokens in channel c
        SP(c->getSrcActor()->getId()) =
                (std::min)(SP(c->getSrcActor()->getId()),
                           sp[c->getSrcActor()->getId()] - c->getInitialTokens());
    }

    // Fire the actors
    while (true) {
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }
        for (uint i = 0; i < g->nrActors(); i++) {
            previousState.sp[i] = currentState.sp[i];
        }

        // Finish actor firings
        for (const auto &a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
                            // Find storage dependencies in periodic phase
                            analyzePeriodicPhase(sp, dep);

                            // Compute throughput
                            thr = computeThroughput(recurrentState);

                            // Find maximum number of iterations
                            maxIterCnt = 0;
                            for (uint i = 0; i < g->nrActors(); i++) {
                                maxIterCnt = (std::max)(maxIterCnt, iterCnt[i]);
                            }

                            // Shift actor firings that haven't fired as often
                            // as the maximum number of times over one full
                            // period and determine the earliest start time of
                            // all actors
                            for (uint i = 0; i < g->nrActors(); i++) {
                                startTime[i] =
                                        startTime[i]
                                        + (maxIterCnt - iterCnt[i]) * static_cast<int>(1.0 / thr);
                            }

                            // Find earliest start time
                            minStartTime = SDFDISCRETETIME_MAX;
                            for (uint i = 0; i < g->nrActors(); i++) {
                                minStartTime = (std::min)(minStartTime, startTime[i]);
                            }

                            // Shift all start times such that first actor
                            // firing occurs at time zero
                            for (uint i = 0; i < g->nrActors(); i++) {
                                startTime[i] = startTime[i] - minStartTime;
                            }

                            return thr;
                        }
                        currentState.glbClk = 0;
                        repCnt = 0;
                    }
                }

                // End the actor firing
                endActorFiring(*a);
            }
        }

        // Start actor firings - repeat till no start enables another start
        do {
            startedActorFiring = false;

            for (const auto &iter : g->getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

                // Ready to fire actor a?
                while (actorReadyToFire(*a)) {
                    // Fire actor a
                    startActorFiring(*a);
                    startTime[a->getId()] = static_cast<SDFdiscreteTime>(globalTime);
                    iterCnt[a->getId()]++;
                    startedActorFiring = true;
                }
            }
        } while (startedActorFiring);

        // Clock step
        clkStep = clockStep();
        globalTime += clkStep;

        // Deadlocked?
        if (clkStep == UINT_MAX) {
            // Find cause of deadlock
            analyzeDeadlock(sp, dep);
            return 0;
        }
    }

    return 0;
}

/*******************************************************************************
 * Storage distributions
 ******************************************************************************/

/**
 * newStorageDistribution ()
 * Allocate memory for a new storage distribution.
 */
std::unique_ptr<StorageDistribution> SDFstateSpaceBufferAnalysisNingGao::newStorageDistribution() {
    auto d = std::make_unique<StorageDistribution>();
    d->sp = std::make_unique<TBufSize[]>(g->nrActors());
    d->dep = std::make_unique<bool[]>(g->nrChannels());

    return d;
}

/**
 * execStorageDistribution ()
 * Compute throughput and storage dependencies of the given storage
 * distribution.
 */
void SDFstateSpaceBufferAnalysisNingGao::execStorageDistribution(
        StorageDistribution& d, std::vector<SDFdiscreteTime> &startTime) {
    // Initialize blocking channels
    for (uint c = 0; c < g->nrChannels(); c++) {
        d.dep[c] = false;
    }

    // Execute the SDF graph to find its output interval
    d.thr = transitionSystem->execSDFgraph(d.sp, d.dep, startTime);

    // std::cerr << d.sz << " " << d.thr << std::endl;
    // for (uint a = 0; a < g->nrActors(); a++)
    //     std::cerr << d.sp[a] << " " << d.dep[a] << std::endl;
    // std::cerr << std::endl;
}

/**
 * minimizeMinStorageDistributions ()
 * The function removes all storage distributions within the supplied
 * set of storage distributions which are non-minimal. All storage distributions
 * within the set should have the same size.
 */
void SDFstateSpaceBufferAnalysisNingGao::minimizeStorageDistributionsSet(
        StorageDistributionSet& ds) {
    StorageDistribution* d = nullptr;

    // Maximal throughput with current distribution size equal to previous
    // throughput with previous (smaller) distribution size
    if (ds.prev != nullptr && ds.prev->thr == ds.thr) {
        // No minimal storage distributions exist in this list
        // Iterate over the list of storage distributions
        d = ds.distributions.get();
        while (d != nullptr) {
            // Temporary reference to next element in list
            StorageDistribution* t = d->next.get();

            // Next
            d = t;
        }

        // No distributions left
        ds.distributions = nullptr;
    } else {
        // Iterate over the list of storage distributions
        d = ds.distributions.get();
        while (d != nullptr) {
            // Throughput of distribution smaller than maximum throughput with
            // same distribution size?
            if (d->thr < ds.thr) {
                // Temporary reference to next element in list
                StorageDistribution* t = d->next.get();

                // Remove d from linked list
                if (d->next != nullptr) {
                    d->next->prev = d->prev;
                }
                if (d->prev != nullptr) {
                    d->prev->next = std::move(d->next);
                } else {
                    ds.distributions = std::move(d->next);
                }
                // Next
                d = t;
            } else {
                // Next
                d = d->next.get();
            }
        }
    }
}

/**
 * addStorageDistributionToChecklist ()
 * The function add the storage distribution 'd' to the list of
 * storage distributions which must still be checked. This list is
 * ordered wrt to the size of the storage distribution. A storage
 * distribution is only added if it is not already contained in the
 * list. When the distribution is added to the list, the function returns
 * 'true', else the function returns 'false'.
 */
bool SDFstateSpaceBufferAnalysisNingGao::addStorageDistributionToChecklist(
        std::unique_ptr<StorageDistribution> d) {
    StorageDistributionSet* ds = nullptr;
    std::unique_ptr<StorageDistributionSet> dsNew = nullptr;
    StorageDistribution* di = nullptr;
    bool equalDistr = false;

    // First distribution ever added?
    if (minStorageDistributions == nullptr) {
        // Create new set of storage distributions
        dsNew = std::make_unique<StorageDistributionSet>();
        dsNew->sz = d->sz;
        dsNew->thr = 0;
        dsNew->distributions = std::move(d);
        dsNew->next = nullptr;
        dsNew->prev = nullptr;

        // Set is first in list to be checked
        minStorageDistributions = std::move(dsNew);

        // Done
        return true;
    }

    // Find set of distributions with the same size
    ds = minStorageDistributions.get();
    while (ds->next != nullptr && ds->next->sz <= d->sz) {
        ds = ds->next.get();
    }

    ASSERT(ds != nullptr, "checkDistributions list cannot be empty");

    // Set of storage distribution with same size as d exists?
    if (ds->sz == d->sz) {

        // Check that distribution d does not exist in the set
        di = ds->distributions.get();
        while (di != nullptr) {
            equalDistr = true;

            // All actors have same storage space allocation?
            for (uint a = 0; equalDistr && a < g->nrActors(); a++) {
                if (di->sp[a] != d->sp[a]) {
                    equalDistr = false;
                }
            }

            // Found equal distribution
            if (equalDistr) {
                return false;
            }

            // Next
            di = di->next.get();
        }

        // Distribution 'd' not yet in the set, so let's add it
        ds->distributions->prev = d.get();
        d->next = std::move(ds->distributions);
        ds->distributions = std::move(d);
    } else if (ds->next == nullptr) {
        // No set of distribution in the list with same or larger size?

        // Create new set of storage distributions
        dsNew = std::make_unique<StorageDistributionSet>();
        dsNew->sz = d->sz;
        dsNew->thr = 0;
        dsNew->distributions = std::move(d);
        dsNew->next = nullptr;
        dsNew->prev = ds;

        // Add set to the list of sets
        ds->next = std::move(dsNew);
    } else {
        // In this case 'ds->next' has a size larger than 'd' and 'ds' has
        // a size smaller than 'd'. Insert a new set of storage
        // distributions between 'ds' and 'ds->next' containing storage
        // distribution 'd'.

        // Create new set of storage distributions
        dsNew = std::make_unique<StorageDistributionSet>();
        dsNew->sz = d->sz;
        dsNew->thr = 0;
        dsNew->distributions = std::move(d);
        auto* dsNext = ds->next.get();
        dsNew->next = std::move(ds->next);
        dsNew->prev = ds;

        // Add set to the list of sets
        dsNext->prev = dsNew.get();
        ds->next = std::move(dsNew);
    }

    // Done
    return true;
}

/**
 * exploreStorageDistribution ()
 * The function computes the throughput of the storage distribution
 * 'd' and adds new storage distributions to the list of distributions
 * which must be checked based on the storage dependencies found in d.
 * The function also updates the maximal throughput of the set of
 * storage distributions when needed.
 */
void SDFstateSpaceBufferAnalysisNingGao::exploreStorageDistribution(
        StorageDistributionSet& ds, StorageDistribution& d) {
    std::unique_ptr<StorageDistribution> dNew = nullptr;
    std::vector<SDFdiscreteTime> startTime;

    // Compute throughput and storage dependencies
    execStorageDistribution(d, startTime);

    // Throughput of d larger than current maximum of the set
    ds.thr = (std::max)(d.thr, ds.thr);

    // Create new storage distributions for every actor which
    // has a storage dependency in d
    for (uint c = 0; c < g->nrChannels(); c++) {
        // Channel c has storage dependency?
        if (d.dep[c]) {
            SDFchannel *ch = g->getChannel(c);

            // Create new storage distribution with storage space of a enlarged
            dNew = newStorageDistribution();
            dNew->sz = d.sz + minSzStep[ch->getSrcActor()->getId()];
            dNew->thr = 0;
            for (uint i = 0; i < g->nrActors(); i++) {
                dNew->sp[i] = d.sp[i];
                if (i == ch->getSrcActor()->getId()) {
                    dNew->sp[i] += minSzStep[ch->getSrcActor()->getId()];
                }
            }
            dNew->next = nullptr;
            dNew->prev = nullptr;

            // Add storage distribution to set of distributions to be checked
            addStorageDistributionToChecklist(std::move(dNew));
        }
    }
}

/**
 * exploreStorageDistributionSet ()
 * Explore all distributions within the set and remove all non-minimal
 * distributions from it.
 */
void SDFstateSpaceBufferAnalysisNingGao::exploreStorageDistributionSet(
        StorageDistributionSet& ds) {
    StorageDistribution* d = nullptr;

    // Explore all storage distributions contained in the set
    d = ds.distributions.get();
    while (d != nullptr) {
        // Explore distribution d
        exploreStorageDistribution(ds, *d);

        // Next
        d = d->next.get();
    }

    // Remove all non-minimal storage distributions from the set
    minimizeStorageDistributionsSet(ds);
}

/**
 * findMinimalStorageDistributions ()
 * Explore the throughput/storage-size trade-off space till either
 * all minimal storage distributions are found or the throughput bound is
 * reached.
 */
void SDFstateSpaceBufferAnalysisNingGao::findMinimalStorageDistributions(const double thrBound) {
    std::unique_ptr<StorageDistribution> d = nullptr;
    StorageDistribution* t = nullptr;
    StorageDistributionSet* ds = nullptr;
    StorageDistributionSet* dt = nullptr;

    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint a = 0; a < g->nrActors(); a++) {
        d->sp[a] = minSz[a];
    }
    d->next = nullptr;
    d->prev = nullptr;

    // Add distribution to set of distributions which must be checked
    addStorageDistributionToChecklist(std::move(d));

    // Check sets of storage distributions till no distributions left to check,
    // or throughput bound exceeded, or maximal throughput reached
    ds = minStorageDistributions.get();
    while (ds != nullptr) {
        // Explore all distributions with size 'ds->sz'
        exploreStorageDistributionSet(*ds);

        // Reached maximum throughput or exceed thrBound
        if (ds->thr >= thrBound || ds->thr == maxThroughput) {
            // thrBound equal to zero is a special case
            if (thrBound == 0) {
                // Only stop when throughput above zero
                if (ds->thr > 0) {
                    break;
                }
            } else {
                break;
            }
        }

        // Temporary pointer to set
        dt = ds;

        // Next
        ds = ds->next.get();

        // No distributions left with the given distribution size?
        if (dt->distributions == nullptr) {
            // Remove distr from linked list
            if (dt->next != nullptr) {
                dt->next->prev = dt->prev;
            }
            if (dt->prev != nullptr) {
                dt->prev->next = std::move(dt->next);
            } else {
                minStorageDistributions = std::move(dt->next);
            }
        }
    }

    // Unexplored distributions left?
    if (ds != nullptr && ds->next != nullptr) {
        // Pointer to first set which must be removed
        ds = ds->next.get();

        // Mark previous set as last set in list of minimal storage distr
        ds->prev->next = nullptr;
    }

    // Lower bound on storage space (which is used in the beginning) is not a
    // minimal storage distribution if it deadlocks. The distribution <0,...,0>
    // is the actual minimal storage distribution for this throughput
    if (minStorageDistributions->thr == 0) {
        minStorageDistributions->sz = 0;
        minStorageDistributions->distributions->sz = 0;
        for (uint a = 0; a < g->nrActors(); a++) {
            minStorageDistributions->distributions->sp[a] = 0;
        }
    }
}

/**
 * analyze ()
 * The function returns a minimal storage distribution that gives maximal
 * throughput under the assumptions made by Ning and Gao. It also returns a
 * static periodic rate-optimal schedule for the graph under the computed
 * buffer constraints.
 */
std::unique_ptr<StorageDistribution>
SDFstateSpaceBufferAnalysisNingGao::analyze(TimedSDFgraph *gr, std::vector<SDFdiscreteTime> &startTime) {
    // SDF graph
    g = gr;

    // Start with an empty set of storage distributions
    minStorageDistributions = nullptr;

    // Initialize bounds on the search space
    initBoundsSearchSpace(g);

    // Create a transition system
    transitionSystem = std::make_unique<TransitionSystem>(g);

    // Search the space
    findMinimalStorageDistributions(DBL_MAX);

    // Find one minimal storage distribution that gives maximal throughput
    while (minStorageDistributions != nullptr && minStorageDistributions->next != nullptr) {
        minStorageDistributions = std::move(minStorageDistributions->next);
    }
    std::unique_ptr<StorageDistribution> d = std::move(minStorageDistributions->distributions);
    d->next = nullptr;
    minStorageDistributions = nullptr;

    // Compute start times for the selected storage distribution
    execStorageDistribution(*d, startTime);

    return d;
}

} // namespace SDF
