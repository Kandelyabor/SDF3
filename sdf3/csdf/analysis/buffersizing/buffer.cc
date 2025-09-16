/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 5, 2006
 *
 *  Function        :   State-space based buffer size analysis
 *
 *  History         :
 *      05-04-06    :   Initial version.
 *      27-10-06    :   BFS based version of the trade-off space exploration
 *                      algorithm.
 *
 * $Id: buffer.cc,v 1.3.2.2 2010-04-25 01:21:16 mgeilen Exp $
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

#include "buffer.h"
#include "csdf/analysis/throughput/selftimed_throughput.h"
#include "sdf/basic_types.h"
#include <algorithm>
#include <memory>

namespace CSDF {

using namespace SDF;

/**
 * ThroughputMargin
 * accuracy of throughput comparisons
 */
const double CSDFstateSpaceBufferAnalysis::ThroughputMargin = 1e-10;

/******************************************************************************
 * Bounds on the search space
 *****************************************************************************/

/**
 * initBoundsSearchSpace ()
 * Compute bounds on the trade-off space that must be explored.
 */
void CSDFstateSpaceBufferAnalysis::initBoundsSearchSpace(const TimedCSDFgraph& g) {
    initMinimalChannelSzStep(g);
    initMinimalChannelSz(g);
    initLbDistributionSz(g);
    initMaxThroughput(g);
}

/**
 * initMinimalChannelSzStep ()
 * Compute lower bound on the step size of channels
 */
void CSDFstateSpaceBufferAnalysis::initMinimalChannelSzStep(const TimedCSDFgraph& g) {
    minSzStep = std::make_unique<TBufSize[]>(g.nrChannels());

    for (const auto& ch : g.getConstChannels()) {
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();

        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        uint minStepSz = srcPort->getRate().at(0);

        // Step size is equal to the gcd of all production and consumption
        // rates that are possible
        for (unsigned int i : srcPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }
        for (unsigned int i : dstPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }

        minSzStep[ch->getId()] = minStepSz;
    }
}

/**
 * initMinimalChannelSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void CSDFstateSpaceBufferAnalysis::initMinimalChannelSz(const TimedCSDFgraph& g) {
    minSz = std::make_unique<TBufSize[]>(g.nrChannels());

    for (const auto& ch : g.getConstChannels()) {
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();
        uint ratePeriod =
                static_cast<uint>(gcd(srcPort->getRate().size(), dstPort->getRate().size()));

        // Initialize lower bound to maximal size
        minSz[ch->getId()] = UINT_MAX;

        // Iterate over all production/consumption rate combinations to find
        // lower bound as given for SDFG case.
        for (uint i = 0; i < ratePeriod; i++) {
            uint p = srcPort->getRate().at(static_cast<int>(i % srcPort->getRate().size()));
            uint c = dstPort->getRate().at(static_cast<int>(i % dstPort->getRate().size()));
            uint t = ch->getInitialTokens();
            uint lb = 0;

            // Lower-bound of a self-edge is rate at which data is produced and
            // consumed and the number of initial tokens present
            if (ch->getSrcActor()->getId() == ch->getDstActor()->getId()) {
                lb = p + (c > t ? c : t);
            } else {
                if (gcd(p, c) != 0) {
                    lb = p + c - gcd(p, c) + t % gcd(p, c);
                } else {
                    lb = p + c - gcd(p, c);
                }
                lb = (lb > t ? lb : t);
            }

            minSz[ch->getId()] = (std::min)(lb, minSz[ch->getId()]);
        }
    }
}

/**
 * initLbDistributionSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void CSDFstateSpaceBufferAnalysis::initLbDistributionSz(const TimedCSDFgraph& g) {
    lbDistributionSz = 0;

    for (uint c = 0; c < g.nrChannels(); c++) {
        lbDistributionSz += minSz[c];
    }
}

/**
 * checkAgainstMaxThroughput ()
 * Check if throughput th is equal to maximal throughput maxTh up to a relative
 * margin of error equal to 'margin'.
 * Since both are floating point variable comparison with == may fail
 * It is assumed that th <= maxTh
 */
inline bool
CSDFstateSpaceBufferAnalysis::checkAgainstMaxThroughput(double th, double maxTh, double margin) {
    return (th / maxTh) > 1.0 - margin;
}

/**
 * initMaxThroughput ()
 * Compute the maximal throughput that can be achieved by the graph.
 */
void CSDFstateSpaceBufferAnalysis::initMaxThroughput(const TimedCSDFgraph& g) {
    maxThroughput = CSDF::CSDFstateSpaceThroughputAnalysis::analyze(g);
}

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void CSDFstateSpaceBufferAnalysis::TransitionSystem::State::print(std::ostream &out) {
    out << "### State ###" << std::endl;

    for (uint i = 0; i < actSeqPos.size(); i++) {
        out << "actSeqPos[" << i << "] =" << actSeqPos[i] << std::endl;
    }

    for (uint i = 0; i < actClk.size(); i++) {
        out << "actClk[" << i << "] =";

        for (std::list<CSDFdiscreteTime>::const_iterator iter = actClk[i].begin(); iter != actClk[i].end();
             iter++) {
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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::State::clear() {
    for (unsigned int &actSeqPo : actSeqPos) {
        actSeqPo = 0;
    }

    for (auto &i : actClk) {
        i.clear();
    }

    for (uint i = 0; i < ch.size(); i++) {
        ch[i] = 0;
        sp[i] = 0;
    }

    glbClk = 0;
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool CSDFstateSpaceBufferAnalysis::TransitionSystem::State::operator==(const State &s) {
    if (glbClk != s.glbClk) {
        return false;
    }

    for (uint i = 0; i < actSeqPos.size(); i++) {
        if (actSeqPos[i] != s.actSeqPos[i]) {
            return false;
        }
    }

    for (uint i = 0; i < ch.size(); i++) {
        if (ch[i] != s.ch[i] || sp[i] != s.sp[i]) {
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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::initOutputActor() {
    CSDFgraph::RepetitionVector repVec;
    int min = INT_MAX;
    CSDFactor *a = nullptr;

    // Compute repetition vector
    repVec = g->getRepetitionVector();

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto& iter : g->getConstActors()) {
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
bool CSDFstateSpaceBufferAnalysis::TransitionSystem::storeState(State &s, StatesIter &pos) {
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
TDtime
CSDFstateSpaceBufferAnalysis::TransitionSystem::computeThroughput(const StatesIter &cycleIter) {
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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::dfsVisitDependencies(
        uint a, int *color, int *pi, const TDependencyGraph& abstractDepGraph, const TDependencies &dep) {
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
                    // All channels from d to c in the CSDFG have
                    // storage dependency
                    for (const auto& ch : g->getConstChannels()) {
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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::findStorageDependencies(
        const TDependencyGraph& abstractDepGraph, const TDependencies &dep) {

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
 * CSDF
 *****************************************************************************/

// NOLINTBEGIN(*macro-usage)

#define CH(c) currentState.ch[c]
#define SP(c) currentState.sp[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CH_SPACE(c, n) (SP(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);
#define CONSUME_SP(c, n) SP(c) = SP(c) - (n);
#define PRODUCE_SP(c, n) SP(c) = SP(c) + (n);

#define CH_TOKENS_PREV(c, n) (previousState.ch[c] >= (n))
#define CH_SPACE_PREV(c, n) (previousState.sp[c] >= (n))

#define ACT_SEQ_POS(a) currentState.actSeqPos[a]

// NOLINTEND(*macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool CSDFstateSpaceBufferAnalysis::TransitionSystem::actorReadyToFire(const CSDFactor& a) {
    // Check all input ports for tokens
    const auto& ports = a.getConstPorts();
    return std::all_of(ports.begin(), ports.end(), [this, &a](const auto& p){
        const CSDFchannel& c = *(p->getChannel());

        // Actor is destination of the channel?
        if (p->getType() == CSDFport::In) {
            if (!CH_TOKENS(c.getId(), p->getRate().at(ACT_SEQ_POS(a.getId())))) {
                return false;
            }
        } else {
            if (!CH_SPACE(c.getId(), p->getRate().at(ACT_SEQ_POS(a.getId())))) {
                return false;
            }
        }
        return true;
    });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void CSDFstateSpaceBufferAnalysis::TransitionSystem::startActorFiring(const TimedCSDFactor& a) {
    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a.getConstPorts()) {
        CSDFchannel *c = p->getChannel();

        // Actor is destination of the channel?
        if (p->getType() == CSDFport::In) {
            CONSUME(c->getId(), p->getRate().at(ACT_SEQ_POS(a.getId())));
        } else {
            CONSUME_SP(c->getId(), p->getRate().at(ACT_SEQ_POS(a.getId())));
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a.getId()].push_back(a.getDiscreteExecutionTime()[ACT_SEQ_POS(a.getId())]);

    // Advance the sequence position of the actor
    ACT_SEQ_POS(a.getId()) = (ACT_SEQ_POS(a.getId()) + 1) % a.sequenceLength();
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool CSDFstateSpaceBufferAnalysis::TransitionSystem::actorReadyToEnd(const CSDFactor& a) {
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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::endActorFiring(const CSDFactor& a) {
    for (const auto& p : a.getConstPorts()) {
        CSDFchannel *c = p->getChannel();
        int seqPos = 0;

        // Position in the sequence to which this end corresponds
        seqPos = static_cast<int>(ACT_SEQ_POS(a.getId()) - currentState.actClk[a.getId()].size());

        // Actor is source of the channel?
        if (p->getType() == CSDFport::Out) {
            PRODUCE(c->getId(), p->getRate().at(seqPos));
        } else {
            PRODUCE_SP(c->getId(), p->getRate().at(seqPos));
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
CSDFdiscreteTime CSDFstateSpaceBufferAnalysis::TransitionSystem::clockStep() {
    CSDFdiscreteTime step = UINT_MAX;

    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++) {
        if (!currentState.actClk[a].empty()) {
            CSDFdiscreteTime actClk = currentState.actClk[a].front();

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
    currentState.glbClk += static_cast<uint32_t>(step);

    return step;
}

/**
 * findCausalDependencies ()
 * The function tracks all causal dependencies in the actor firing of actor a.
 * Any causal dependency that is found is added to the abstract dependency
 * graph.
 */
void CSDFstateSpaceBufferAnalysis::TransitionSystem::findCausalDependencies(
        const CSDFactor& a, const TDependencyGraph& abstractDepGraph) {
    // Check all input ports for tokens and output ports for space
    for (const auto& p : a.getConstPorts()) {
        CSDFchannel *c = p->getChannel();
        CSDFactor *srcActor = c->getSrcActor();
        CSDFactor *dstActor = c->getDstActor();

        // Actor is destination of the channel?
        if (p->getType() == CSDFport::In) {
            // Not enough tokens in the previous state?
            if (!CH_TOKENS_PREV(c->getId(), p->getRate().at(ACT_SEQ_POS(a.getId())))) {
                abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
            }
        } else {
            // Not enough space in the previous state?
            if (!CH_SPACE_PREV(c->getId(), p->getRate().at(ACT_SEQ_POS(a.getId())))) {
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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::analyzePeriodicPhase(const std::unique_ptr<TBufSize[]> & /*sp*/,
                             const TDependencies &dep) {
    State periodicState(g->nrActors(), g->nrChannels());
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
    for (const auto& a : g->getConstActors()) {
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
        // Start actor firings
        for (const auto& iter : g->getConstActors()) {
            const auto& a = *dynamic_cast<const TimedCSDFactor *>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(a)) {
                // Track causal dependencies on firing of actor a
                findCausalDependencies(a, abstractDepGraph);

                // Fire actor a
                startActorFiring(a);
            }
        }

        // Clock step
        clockStep();

        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
            previousState.sp[i] = currentState.sp[i];
        }

        // Finish actor firings
        for (const auto& a : g->getConstActors()) {

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
void CSDFstateSpaceBufferAnalysis::TransitionSystem::analyzeDeadlock(const std::unique_ptr<TBufSize[]> & /*sp*/,
                                                                     const TDependencies &dep) {
    // Abstract dependency graph
    auto abstractDepGraph = std::make_unique<TDependencies[]>(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        abstractDepGraph[i] = std::make_unique<bool[]>(g->nrActors());
        for (uint j = 0; j < g->nrActors(); j++) {
            abstractDepGraph[i][j] = false;
        }
    }

    // Check number of tokens on every channel in the graph
    for (const auto& c : g->getConstChannels()) {
        CSDFactor *srcActor = c->getSrcActor();
        CSDFactor *dstActor = c->getDstActor();

        // Insufficient tokens to fire destination actor
        if (!CH_TOKENS(c->getId(), c->getDstPort()->getRate().at(ACT_SEQ_POS(dstActor->getId())))) {
            abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
        }

        // Insufficient space to fire source actor
        if (!CH_SPACE(c->getId(), c->getSrcPort()->getRate().at(ACT_SEQ_POS(srcActor->getId())))) {
            abstractDepGraph[srcActor->getId()][dstActor->getId()] = true;
        }
    }

    // Cycles in the dependency graph indicate storage dependencies
    findStorageDependencies(abstractDepGraph, dep);
}

/**
 * execCSDFgraph()
 * Execute the CSDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime CSDFstateSpaceBufferAnalysis::TransitionSystem::execCSDFgraph(const std::unique_ptr<TBufSize[]> &sp,
                                                                     const TDependencies &dep) {
    StatesIter recurrentState;
    TTime clkStep = 0;
    int repCnt = 0;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();
    previousState.init(g->nrActors(), g->nrChannels());
    previousState.clear();

    // Initial tokens and space
    for (const auto& c : g->getConstChannels()) {

        // Not enough space for initial tokens?
        if (sp[c->getId()] < c->getInitialTokens()) {
            dep[c->getId()] = true;
            return 0;
        }

        CH(c->getId()) = c->getInitialTokens();
        SP(c->getId()) = sp[c->getId()] - c->getInitialTokens();
    }

    // Fire the actors
    while (true) {
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
            previousState.sp[i] = currentState.sp[i];
        }

        // Finish actor firings
        for (const auto& a : g->getConstActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
                            // Find storage dependencies in periodic phase
                            analyzePeriodicPhase(sp, dep);

                            return computeThroughput(recurrentState);
                        }
                        currentState.glbClk = 0;
                        repCnt = 0;
                    }
                }

                // End the actor firing
                endActorFiring(*a);
            }
        }

        // Start actor firings
        for (const auto& iter : g->getConstActors()) {
            const auto& a = *dynamic_cast<const TimedCSDFactor *>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(a)) {
                // Fire actor a
                startActorFiring(a);
            }
        }

        // Clock step
        clkStep = clockStep();

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
std::unique_ptr<StorageDistribution> CSDFstateSpaceBufferAnalysis::newStorageDistribution() {
    auto d = std::make_unique<StorageDistribution>();
    d->sp = std::make_unique<TBufSize[]>(g->nrChannels());
    d->dep = std::make_unique<bool[]>(g->nrChannels());

    return d;
}


/**
 * execStorageDistribution ()
 * Compute throughput and storage dependencies of the given storage
 * distribution.
 */
void CSDFstateSpaceBufferAnalysis::execStorageDistribution(StorageDistribution& d) {
    // Initialize blocking channels
    for (uint c = 0; c < g->nrChannels(); c++) {
        d.dep[c] = false;
    }

    // Execute the CSDF graph to find its output interval
    d.thr = transitionSystem->execCSDFgraph(d.sp, d.dep);

    // cerr << d.sz << " " << d.thr << std::endl;
    // for (uint c = 0; c < g->nrChannels(); c++)
    //     cerr << d.sp[c] << " " << d.dep[c] << std::endl;
    // cerr << std::endl;
}

/**
 * minimizeMinStorageDistributions ()
 * The function removes all storage distributions within the supplied
 * set of storage distributions which are non-minimal. All storage distributions
 * within the set should have the same size.
 */
void CSDFstateSpaceBufferAnalysis::minimizeStorageDistributionsSet(StorageDistributionSet& ds) {
    StorageDistribution* d = nullptr;
    StorageDistribution* t = nullptr;

    // Maximal throughput with current distribution size equal to previous
    // throughput with previous (smaller) distribution size
    if (ds.prev != nullptr
        && checkAgainstMaxThroughput(ds.prev->thr, ds.thr, ThroughputMargin)) {
        // No minimal storage distributions exist in this list
        ds.distributions = nullptr;
    } else {
        // Iterate over the list of storage distributions
        d = ds.distributions.get();
        while (d != nullptr) {
            // Throughput of distribution smaller than maximum throughput with
            // same distribution size?
            if (d->thr < ds.thr) {
                // Remove d from linked list
                if (d->next != nullptr) {
                    d->next->prev = d->prev;
                }
                auto* dNext = d->next.get();
                if (d->prev != nullptr) {
                    d->prev->next = std::move(d->next);
                }else {
                    ds.distributions = std::move(d->next);
                }

                // Next
                d = dNext;
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
bool CSDFstateSpaceBufferAnalysis::addStorageDistributionToChecklist(std::unique_ptr<StorageDistribution> d) {
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

            // All channels have same storage space allocation?
            for (uint c = 0; equalDistr && c < g->nrChannels(); c++) {
                if (di->sp[c] != d->sp[c]) {
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
        dsNew->next = std::move(ds->next);
        dsNew->prev = ds;

        // Add set to the list of sets
        ds->next->prev = dsNew.get();
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
void CSDFstateSpaceBufferAnalysis::exploreStorageDistribution(StorageDistributionSet& ds,
                                                              StorageDistribution& d) {
    std::unique_ptr<StorageDistribution> dNew = nullptr;

    // Compute throughput and storage dependencies
    execStorageDistribution(d);

    // Throughput of d larger than current maximum of the set
    ds.thr = (std::max)(d.thr, ds.thr);

    // Create new storage distributions for every channel which
    // has a storage dependency in d
    for (uint c = 0; c < g->nrChannels(); c++) {
        // Channel c has storage dependency?
        if (d.dep[c]) {
            // Do not enlarge the channel if its a self-edge
            if (g->getConstChannel(c)->getSrcActor()->getId()
                == g->getConstChannel(c)->getDstActor()->getId()) {
                continue;
            }

            // Create new storage distribution with channel c enlarged
            dNew = newStorageDistribution();
            dNew->sz = d.sz + minSzStep[c];
            dNew->thr = 0;
            for (uint i = 0; i < g->nrChannels(); i++) {
                dNew->sp[i] = d.sp[i];
                if (i == c) {
                    dNew->sp[i] += minSzStep[c];
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
void CSDFstateSpaceBufferAnalysis::exploreStorageDistributionSet(StorageDistributionSet& ds) {
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
void CSDFstateSpaceBufferAnalysis::findMinimalStorageDistributions(const double thrBound) {
    std::unique_ptr<StorageDistribution> d = nullptr;
    StorageDistribution* t = nullptr;
    StorageDistributionSet* ds = nullptr;
    StorageDistributionSet* dt = nullptr;

    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < g->nrChannels(); c++) {
        d->sp[c] = minSz[c];
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
        if (ds->thr >= thrBound
            || checkAgainstMaxThroughput(ds->thr, maxThroughput, ThroughputMargin)) {
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
        for (uint c = 0; c < g->nrChannels(); c++) {
            minStorageDistributions->distributions->sp[c] = 0;
        }
    }
}

/**
 * analyze ()
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */
std::unique_ptr<StorageDistributionSet> CSDFstateSpaceBufferAnalysis::analyze(const TimedCSDFgraph& gr,
                                                              const double thrBound) {
    // CSDF graph
    g = &gr;

    // Start with an empty set of storage distributions
    minStorageDistributions = nullptr;

    // Initialize bounds on the search space
    initBoundsSearchSpace(*g);

    // Create a transition system
    transitionSystem = std::make_unique<TransitionSystem>(*g);

    // Search the space
    findMinimalStorageDistributions(thrBound);

    return std::move(minStorageDistributions);
}

/**
 * initSearch ()
 * Initialize the storage distribution search algorithm.
 */
void CSDFstateSpaceBufferAnalysis::initSearch(const TimedCSDFgraph& gr) {
    std::unique_ptr<StorageDistribution> d = nullptr;

    // CSDF graph
    g = &gr;

    // Start with an empty set of storage distributions
    minStorageDistributions = nullptr;

    // Initialize bounds on the search space
    initBoundsSearchSpace(*g);

    // Create a transition system
    transitionSystem = std::make_unique<TransitionSystem>(*g);

    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < g->nrChannels(); c++) {
        d->sp[c] = minSz[c];
    }
    d->next = nullptr;
    d->prev = nullptr;

    // Add distribution to set of distributions which must be checked
    addStorageDistributionToChecklist(std::move(d));

    // No storage distribution set explored so far
    lastExploredStorageDistributionSet = nullptr;
}

/**
 * findNextStorageDistributionSet ()
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as a new pareto point has been
 * found in the space or when the full space has been explored. The function
 * returns the newly discovered storage distribution set or nullptr when no new set
 * was found.
 */
StorageDistributionSet* CSDFstateSpaceBufferAnalysis::findNextStorageDistributionSet() {
    std::unique_ptr<StorageDistribution> d = nullptr;
    StorageDistribution* t = nullptr;
    StorageDistributionSet* dp = nullptr;
    StorageDistributionSet* ds = nullptr;
    StorageDistributionSet* dt = nullptr;

    // Last explored distribution set
    dp = lastExploredStorageDistributionSet;

    // Last explored set reaches already maximal throughput?
    if (dp != nullptr && checkAgainstMaxThroughput(dp->thr, maxThroughput, ThroughputMargin)) {
        return nullptr;
    }

    // Next set to be explored
    if (dp == nullptr) {
        ds = minStorageDistributions.get();
    } else {
        ds = dp->next.get();
    }

    // Check sets of storage distributions till no distributions left to check,
    // or next pareto point is found
    while (ds != nullptr) {
        // Explore all distributions with size 'ds->sz'
        exploreStorageDistributionSet(*ds);

        // Distribution set ds has larger throughput than previous pareto point?
        if ((dp == nullptr && ds->thr > 0) || (dp != nullptr && ds->thr > dp->thr)) {
            break;
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

    // Unexplored distributions left, but maximal throughput reached?
    if (ds != nullptr && ds->next != nullptr
        && checkAgainstMaxThroughput(ds->thr, maxThroughput, ThroughputMargin)) {
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
        for (uint c = 0; c < g->nrChannels(); c++) {
            minStorageDistributions->distributions->sp[c] = 0;
        }
    }

    // Current set becomes last explored set
    lastExploredStorageDistributionSet = ds;

    return ds;
}

/**
 * approximate ()
 * The function uses an approximation technique to find a storage distribution
 * (set) that has a throughput equal or larger than 'thr' and that has an
 * over estimation of the total storage-space that is less than or equal to the
 * percentage given by 'maxPercentageOverEstimation'.
 */
StorageDistributionSet *CSDFstateSpaceBufferAnalysis::approximate(
        const TimedCSDFgraph& /*gr*/, const double  /*thr*/, const double  /*maxPercentageOverEstimation*/) {
    /*
        0. Initialize 'lastChStep' with all channels at 0.
           Set 'quantizationFactor' to 1.
        1. initialize the standard things (bounds, transition system, etc.)
        2. Create an initial storage-space distribution set with the initial storage
           distribution (i.e. the lower bounds)
        3. Search the first storage distribution set S that has not been searched yet
        4. Does set S meet the throughput constraint?
            No -> goto step 4-5
            Yes -> goto step 4-1

            4-1. Error within specified bound? Compute error using current
                 quantization factor. Note that actual quantization factor used to
                 find distributions in this distribution set may be smaller. But
                 never larger.
            Yes -> return storage distribution set S
            No -> goto step 4-2
            4-3. Create a new set of storage distributions S' in which the storage-
                 space of every channel has been reduced by the amount given in
                 'lastChStep'.
            4-2. The set S' replaces all storage distributions which have been
                 created (searched and not searched).
            4-3. Set the 'lastChStep' with all channels at 0.
                 Set 'quantizationFactor' to 1.
            4-4 Goto step 3

            4-5. Add storage distributions to the list of storage distributions that
                 must be searched. For all channels of which the storage space is
                 increased in at least one new storage distribution, update the
                 'lastChStep' entry.
            4-6. Double the quantization factor.
            4-7. Goto step 3
    */
    return nullptr;
}
} // namespace CSDF
