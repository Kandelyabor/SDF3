/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   bounded_buffer.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Compute throughput/buffer trade-off for an SDFG which
 *                      is bound to an multi-processor architecture.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: bounded_buffer.cc,v 1.1.2.1 2010-04-22 07:18:38 mgeilen Exp $
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

#include "bounded_buffer.h"
#include "sdf/base/algo/repetition_vector.h"
#include <algorithm>
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
void SDFstateSpaceBindingAwareBufferAnalysis::initBoundsSearchSpace(TimedSDFgraph *g,
                                                                    bool *bufferChannels) {
    initMinimalChannelSzStep(g);
    initMinimalChannelSz(g, bufferChannels);
    initLbDistributionSz(g);
    initMaxThroughput(g);
}

/**
 * initMinimalChannelSzStep ()
 * Compute lower bound on the step size of channels
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initMinimalChannelSzStep(TimedSDFgraph *g) {
    this->minSzStep = std::make_unique<TBufSize[]>(g->nrChannels());

    for (const auto &ch : g->getChannels()) {
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();

        uint minStepSz = gcd(p, c);

        this->minSzStep[ch->getId()] = minStepSz;
    }
}

/**
 * initMinimalChannelSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initMinimalChannelSz(TimedSDFgraph *g,
                                                                   const bool *bufferChannels) {
    minSz = std::make_unique<TBufSize[]>(g->nrChannels());

    for (auto iter = g->channelsBegin(); iter != g->channelsEnd(); iter++) {
        const std::unique_ptr<SDFchannel> &ch = *iter;
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint lb = 0;

        // Channel models buffer space (ignore initial tokens)?
        if (bufferChannels[ch->getId()]) {
            lb = p + c - gcd(p, c) + t % gcd(p, c);

            minSz[ch->getId()] = lb;
        } else {
            lb = p + c - gcd(p, c) + t % gcd(p, c);
            lb = (lb > t ? lb : t);

            minSz[ch->getId()] = lb;
        }

        // Lower-bound of a self-edge is rate at which data is produced and
        // consumed
        if (ch->getSrcActor()->getId() == ch->getDstActor()->getId()) {
            minSz[ch->getId()] = p + c;
        }
    }
}

/**
 * initLbDistributionSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initLbDistributionSz(TimedSDFgraph *g) {
    lbDistributionSz = 0;

    for (uint c = 0; c < g->nrChannels(); c++) {
        lbDistributionSz += minSz[c];
    }
}

/**
 * initMaxThroughput ()
 * Compute the maximal throughput that can be achieved by the graph.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initMaxThroughput(TimedSDFgraph * /*g*/) {
    maxThroughput = TDTIME_MAX;
}

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * clear ()
 * The function sets the state to zero.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::State ::clear() {
    for (auto &i : actClk) {
        i.clear();
    }

    for (uint32_t &i : ch) {
        i = 0;
    }

    glbClk = 0;

    for (uint i = 0; i < schedulePos.size(); i++) {
        schedulePos[i] = 0;
        tdmaPos[i] = 0;
    }
}

/**
 * operator= ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::State ::operator==(const State &s) {
    if (glbClk != s.glbClk) {
        return false;
    }

    for (uint i = 0; i < ch.size(); i++) {
        if (ch[i] != s.ch[i]) {
            return false;
        }
    }

    for (uint i = 0; i < actClk.size(); i++) {
        if (actClk[i] != s.actClk[i]) {
            return false;
        }
    }

    for (uint i = 0; i < schedulePos.size(); i++) {
        if (schedulePos[i] != s.schedulePos[i] || tdmaPos[i] != s.tdmaPos[i]) {
            return false;
        }
    }

    return true;
}

/**
 * print ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::State ::print(std::ostream &out) {
    out << "### State ###" << std::endl;

    for (uint i = 0; i < actClk.size(); i++) {
        out << "actClk[" << i << "] =";

        for (std::list<SDFdiscreteTime>::const_iterator iter = actClk[i].begin();
             iter != actClk[i].end();
             iter++) {
            out << " " << (*iter) << ", ";
        }

        out << std::endl;
    }

    for (uint i = 0; i < ch.size(); i++) {
        out << "ch[" << i << "] = " << ch[i] << std::endl;
    }

    out << "glbClk = " << glbClk << std::endl;

    for (uint i = 0; i < schedulePos.size(); i++) {
        out << "tile[" << i << "] = (";
        out << schedulePos[i];
        out << ", " << tdmaPos[i] << ")" << std::endl;
    }
}

/******************************************************************************
 * Transition system
 *****************************************************************************/

/**
 * initOutputActor ()
 * The function selects an actor to be used as output actor in the
 * state transition system.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::initOutputActor() {
    RepetitionVector repVec;
    int min = INT_MAX;
    SDFactor* a = nullptr;

    // Compute repetition vector
    repVec = computeRepetitionVector(*bindingAwareSDFG);

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto &iter : bindingAwareSDFG->getActors()) {
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
 * checkBindingAwareSDFG ()
 * The function performs some sanity checks on the binding-aware SDFG
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::checkBindingAwareSDFG() {
    CId tileId = 0;

    // Check that all actor that are bound to a processors are bound to a
    // processor which has a schedule
    for (const auto &a : bindingAwareSDFG->getActors()) {
        tileId = bindingAwareSDFG->getBindingOfActorToTile(*a);

        if (tileId != ACTOR_NOT_BOUND) {
            if (bindingAwareSDFG->getScheduleOnTile(tileId).empty()) {
                throw CException("Actor mapped to processor without schedule.");
            }
        }
    }
}

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::storeState(State &s,
                                                                           StatesIter &pos) {
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
TDtime SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::computeThroughput(
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::dfsVisitDependencies(
        uint a,
        int *color,
        int *pi,
        const TDependencies *abstractDepGraph,
        const TDependencies &dep) {
    uint c = 0;
    uint d = 0;

    // Mark node a as visited
    color[a] = 1;

    // Visit all nodes reachable from a
    for (uint b = 0; b < bindingAwareSDFG->nrActors(); b++) {
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
                    for (const auto &ch : bindingAwareSDFG->getChannels()) {
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
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++) {
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::findStorageDependencies(
        const TDependencies *abstractDepGraph,
        const TDependencies &dep,
        const bool *bufferChannels) {
    std::unique_ptr<int[]> color;
    std::unique_ptr<int[]> pi;

    // Initialize DFS data structures
    color = std::make_unique<int[]>(bindingAwareSDFG->nrActors());
    pi = std::make_unique<int[]>(bindingAwareSDFG->nrActors());
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++) {
        color[i] = 0;
    }

    // Initialize storage dependencies
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++) {
        dep[c] = false;
    }

    // DFS from every node in the graph to find all cycles
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++) {
        pi[i] = static_cast<int>(i);
        dfsVisitDependencies(i, color.get(), pi.get(), abstractDepGraph, dep);
    }

    // Only channels that have a dependency and that are a buffer channel are
    // true storage dependencies
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++) {
        dep[c] = dep[c] && bufferChannels[c];
    }
}

/******************************************************************************
 * SDF
 *****************************************************************************/

// NOLINTBEGIN(cppcoreguidelines-macro-usage,readability-macro-usage)

#define CH(c) currentState.ch[c]
#define TDMA_POS(p) currentState.tdmaPos[p]
#define SOS_POS(p) currentState.schedulePos[p]

#define SOS(p) (bindingAwareSDFG->getScheduleOnTile(p))
#define SOS_ENTRY(p) (SOS(p).getConstScheduleEntry(SOS_POS(p)))
#define SOS_NEXT_POS(p) (SOS(p).next(SOS_POS(p)))

#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);

#define CH_TOKENS_PREV(c, n) (previousState.ch[c] >= (n))

// NOLINTEND(cppcoreguidelines-macro-usage,readability-macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::actorReadyToFire(SDFactor *a) {
    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(*a) != ACTOR_NOT_BOUND) {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(*a);

        // Actor not scheduled on processor?
        if (SOS_ENTRY(p)->actor->getId() != a->getId()) {
            return false;
        }
    }

    // Check all input ports for tokens
    return std::all_of(
            a->getPorts().begin(), a->getPorts().end(), [this](const std::unique_ptr<SDFport> &p) {
                return p->getType() != SDFport::SDFportType::In
                       || CH_TOKENS(p->getChannel()->getId(), p->getRate());
            });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::startActorFiring(
        TimedSDFactor *a) {
    SDFdiscreteTime execTime = 0;
    SDFdiscreteTime completionTime = 0;
    SDFdiscreteTime timeTileStartOfSlice = 0;
    SDFdiscreteTime waitingTime = 0;
    int remainingExecTime = 0;
    int nrOfFullRotationsInNonReservedPart = 0;

    // Consume tokens from inputs and space for output tokens
    for (const auto &p : a->getPorts()) {
        SDFchannel *c = p->getChannel();

        // Actor is destination of the channel?
        if (p->getType() == SDFport::SDFportType::In) {
            CONSUME(c->getId(), p->getRate());
        }
    }

    // Execution time of the actor
    execTime = a->getDiscreteExecutionTime();

    // Compute time needed to complete actor firing
    if (bindingAwareSDFG->getBindingOfActorToTile(*a) == ACTOR_NOT_BOUND) {
        completionTime = execTime;
    } else {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(*a);

        // Time wheel has not yet reached start of slice?
        if (currentState.tdmaPos[p]
            < bindingAwareSDFG->getTDMAsizeOnTile(p) - bindingAwareSDFG->getTDMAsliceOnTile(p)) {
            // Wait till start of slice and next complete the number
            // of required wheel rotations to get the full execution time
            // of the slice
            timeTileStartOfSlice = bindingAwareSDFG->getTDMAsizeOnTile(p)
                                   - bindingAwareSDFG->getTDMAsliceOnTile(p)
                                   - currentState.tdmaPos[p];
            nrOfFullRotationsInNonReservedPart =
                    static_cast<int>(ceil(static_cast<double>(execTime)
                                          / bindingAwareSDFG->getTDMAsliceOnTile(p)))
                    - 1;
            waitingTime = (bindingAwareSDFG->getTDMAsizeOnTile(p)
                           - bindingAwareSDFG->getTDMAsliceOnTile(p))
                          * nrOfFullRotationsInNonReservedPart;
            completionTime = timeTileStartOfSlice + execTime + waitingTime;
        } else {
            // Start the execution time immediately, wait the number of wheel
            // rotations after the current slice has been used to complete the
            // firing
            remainingExecTime = static_cast<int>(execTime)
                                - static_cast<int>(bindingAwareSDFG->getTDMAsizeOnTile(p))
                                + static_cast<int>(currentState.tdmaPos[p]);

            if (remainingExecTime < 0) {
                waitingTime = 0;
            } else {
                waitingTime = (remainingExecTime / bindingAwareSDFG->getTDMAsliceOnTile(p))
                              * (bindingAwareSDFG->getTDMAsizeOnTile(p)
                                 - bindingAwareSDFG->getTDMAsliceOnTile(p));
            }

            completionTime = execTime + waitingTime;
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a->getId()].push_back(completionTime);
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::actorReadyToEnd(SDFactor *a) {
    if (currentState.actClk[a->getId()].empty()) {
        return false;
    }

    // First actor firing in sorted list has execution time left?
    if (currentState.actClk[a->getId()].front() != 0) {
        return false;
    }

    return true;
}

/**
 * endActorFiring ()
 * Produce tokens on all output channels and remove the actor firing from the
 * list of active firings.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::endActorFiring(SDFactor *a) {
    for (const auto &p : a->getPorts()) {
        SDFchannel *c = p->getChannel();

        // Actor is source of the channel?
        if (p->getType() == SDFport::SDFportType::Out) {
            PRODUCE(c->getId(), p->getRate());
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a->getId()].pop_front();

    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(*a) != ACTOR_NOT_BOUND) {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(*a);

        // Advance the schedule to the next state
        SOS_POS(p) = SOS_NEXT_POS(p);
    }
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFdiscreteTime SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::clockStep() {
    SDFdiscreteTime step = UINT_MAX;

    // Find maximal time progress
    for (uint a = 0; a < bindingAwareSDFG->nrActors(); a++) {
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
    for (uint a = 0; a < bindingAwareSDFG->nrActors(); a++) {
        for (auto &actFiringTime : currentState.actClk[a]) {
            // Lower remaining execution time of the actor firing
            actFiringTime -= step;
        }
    }

    // Advance the time wheels
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++) {
        currentState.tdmaPos[t] =
                (currentState.tdmaPos[t] + step) % bindingAwareSDFG->getTDMAsizeOnTile(t);
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::findCausalDependencies(
        SDFactor *a, TDependencies *abstractDepGraph) {
    // Check all input ports for tokens and output ports for space
    for (const auto &p : a->getPorts()) {
        SDFchannel *c = p->getChannel();
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();

        // Actor is destination of the channel?
        if (p->getType() == SDFport::SDFportType::In) {
            // Not enough tokens in the previous state?
            if (!CH_TOKENS_PREV(c->getId(), p->getRate())) {
                abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
            }
        }
    }
}

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find all blocked channels. This
 * is done using the abstract dependency graph.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem ::analyzePeriodicPhase(
        const TDependencies &dep, bool *bufferChannels) {
    State periodicState;
    int repCnt = 0;

    // Current state is a periodic state
    periodicState = currentState;

    // Abstract dependency graph
    auto abstractDepGraph =
            std::make_unique<TDependencies[]>(bindingAwareSDFG->nrActors());
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++) {
        abstractDepGraph[i] = std::make_unique<bool[]>(bindingAwareSDFG->nrActors());
        for (uint j = 0; j < bindingAwareSDFG->nrActors(); j++) {
            abstractDepGraph[i][j] = false;
        }
    }

    // Start new iteration of the periodic phase
    currentState.glbClk = 0;

    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (const auto &a : bindingAwareSDFG->getActors()) {
        while (actorReadyToEnd(a.get())) {
            if (a->getId() == outputActor->getId()) {
                repCnt++;
                if (repCnt == outputActorRepCnt) {
                    currentState.glbClk = 0;
                    repCnt = 0;
                }
            }

            endActorFiring(a.get());
        }
    }

    // Fire the actors
    while (true) {
        // Start actor firings
        for (const auto &iter : bindingAwareSDFG->getActors()) {
            auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(a)) {
                // Track causal dependencies on firing of actor a
                findCausalDependencies(a, abstractDepGraph.get());

                // Fire actor a
                startActorFiring(a);
            }
        }

        // Clock step
        clockStep();

        // Store partial state to check for progress
        for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings
        for (const auto &a : bindingAwareSDFG->getActors()) {

            while (actorReadyToEnd(a.get())) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Found periodic state
                        if (currentState == periodicState) {
                            // Cycles in the dependency graph indicate storage
                            // dependencies
                            findStorageDependencies(abstractDepGraph.get(), dep, bufferChannels);

                            // Done
                            return;
                        }
                        currentState.glbClk = 0;
                        repCnt = 0;
                    }
                }

                // End the actor firing
                endActorFiring(a.get());
            }
        }
    }
}

/**
 * analyzeDeadlock ()
 * Analyze the deadlock in the schedule.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::analyzeDeadlock(
        const TDependencies &dep, bool *bufferChannels) {

    // Abstract dependency graph
    auto abstractDepGraph =
            std::make_unique<TDependencies[]>(bindingAwareSDFG->nrActors());
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++) {
        abstractDepGraph[i] = std::make_unique<bool[]>(bindingAwareSDFG->nrActors());
        for (uint j = 0; j < bindingAwareSDFG->nrActors(); j++) {
            abstractDepGraph[i][j] = false;
        }
    }

    // Check number of tokens on every channel in the graph
    for (const auto &c : bindingAwareSDFG->getChannels()) {
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();
        uint procDstActor = bindingAwareSDFG->getBindingOfActorToTile(*dstActor);

        // Destination actor allowed to fire (i.e. not bound to processor or
        // first actor in a schedule?)
        if (procDstActor == ACTOR_NOT_BOUND
            || SOS_ENTRY(procDstActor)->actor->getId() == dstActor->getId()) {
            // Insufficient tokens to fire destination actor
            if (!CH_TOKENS(c->getId(), c->getDstPort()->getRate())) {
                abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
            }
        }
    }

    // Cycles in the dependency graph indicate storage dependencies
    findStorageDependencies(abstractDepGraph.get(), dep, bufferChannels);
}

/**
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::execSDFgraph(
        const std::unique_ptr<TBufSize[]> &sp,
        const TDependencies &dep,
        bool *bufferChannels) {
    StatesIter recurrentState;
    SDFdiscreteTime clkStep = 0;
    int repCnt = 0;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(bindingAwareSDFG->nrActors(),
                      bindingAwareSDFG->nrChannels(),
                      bindingAwareSDFG->nrTilesInPlatformGraph());
    currentState.clear();
    previousState.init(bindingAwareSDFG->nrActors(),
                       bindingAwareSDFG->nrChannels(),
                       bindingAwareSDFG->nrTilesInPlatformGraph());
    previousState.clear();

    // Initial tokens
    for (const auto &iter : bindingAwareSDFG->getChannels()) {
        auto* ch = dynamic_cast<TimedSDFchannel*>(iter.get());

        // Channel models buffer space?
        if (bufferChannels[ch->getId()]) {
            // Produce as many tokens in the channel as 'sp' indicates
            PRODUCE(ch->getId(), sp[ch->getId()]);
        } else if (ch->getInitialTokens() != 0) {
            PRODUCE(ch->getId(), ch->getInitialTokens());
        }

        // Remove as many tokens from the channel as needed to store
        // the initial tokens of the channel it models
        if (ch->modelsStorageSpace()) {
            TimedSDFchannel *c = ch->getStorageSpaceChannel();

            // Related channel contains initial tokens?
            if (c->getInitialTokens() > 0) {
                // Channel ch models either memory buffer of source connection
                // buffer. Channels modelling destination buffer do not contain
                // initial tokens.
                if (c->getSrcActor()->getId() == ch->getDstActor()->getId()) {
                    // Sufficient tokens in channel ch to store all initial
                    // tokens in channel c?
                    if (CH_TOKENS(ch->getId(), c->getInitialTokens())) {
                        CONSUME(ch->getId(), c->getInitialTokens());
                    } else {
                        if (bufferChannels[ch->getId()]) {
                            dep[ch->getId()] = true;
                        }
                        return 0;
                    }
                }
            }
        }
    }

    // Initial schedules
    for (uint p = 0; p < bindingAwareSDFG->nrTilesInPlatformGraph(); p++) {
        SOS_POS(p) = 0;
        TDMA_POS(p) = 0;
    }

    // Fire the actors
    while (true) {
        // Store partial state to check for progress
        for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings
        for (const auto &a : bindingAwareSDFG->getActors()) {
            while (actorReadyToEnd(a.get())) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
                            // Find storage dependencies in periodic phase
                            analyzePeriodicPhase(dep, bufferChannels);

                            // Done
                            return computeThroughput(recurrentState);
                        }
                        currentState.glbClk = 0;
                        repCnt = 0;
                    }
                }

                // End the actor firing
                endActorFiring(a.get());
            }
        }

        // Start actor firings
        for (const auto &iter : bindingAwareSDFG->getActors()) {
            auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

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
            // Find storage dependencies in deadlock
            analyzeDeadlock(dep, bufferChannels);

            // Done
            return 0;
        }
    }

    return 0;
}

/*******************************************************************************
 * Distributions
 ******************************************************************************/

/**
 * newStorageDistribution ()
 * Allocate memory for a new storage distribution.
 */
std::unique_ptr<StorageDistribution>
SDFstateSpaceBindingAwareBufferAnalysis ::newStorageDistribution() {
    auto d = std::make_unique<StorageDistribution>();
    d->sp = std::make_unique<TBufSize[]>(bindingAwareSDFG->nrChannels());
    d->dep = std::make_unique<bool[]>(bindingAwareSDFG->nrChannels());

    return d;
}

/**
 * execStorageDistribution ()
 * Compute throughput and storage dependencies of the given storage
 * distribution.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::execStorageDistribution(
        StorageDistribution &d, bool *bufferChannels) {
    // Initialize blocking channels
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++) {
        d.dep[c] = false;
    }

    // Execute the SDF graph to find its output interval
    d.thr = transitionSystem->execSDFgraph(d.sp, d.dep, bufferChannels);

    // std::cerr << d.sz << " " << d.thr << std::endl;
    // for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
    //     std::cerr << d.sp[c] << " " << d.dep[c] << std::endl;
    // std::cerr << std::endl;
}

/**
 * minimizeMinStorageDistributions ()
 * The function removes all storage distributions within the supplied
 * set of storage distributions which are non-minimal. All storage distributions
 * within the set should have the same size.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::minimizeStorageDistributionsSet(
        StorageDistributionSet &ds) {
    StorageDistribution* d = nullptr;
    StorageDistribution* t = nullptr;

    // Maximal throughput with current distribution size equal to previous
    // throughput with previous (smaller) distribution size
    if (ds.prev != nullptr && ds.prev->thr == ds.thr) {
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
                } else {
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
bool SDFstateSpaceBindingAwareBufferAnalysis::addStorageDistributionToChecklist(
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

            // All channels have same storage space allocation?
            for (uint c = 0; equalDistr && c < bindingAwareSDFG->nrChannels(); c++) {
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
        if (ds->distributions != nullptr) {
            ds->distributions->prev = d.get();
        }
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
void SDFstateSpaceBindingAwareBufferAnalysis::exploreStorageDistribution(
       StorageDistributionSet& ds,
        StorageDistribution& d,
        bool *bufferChannels,
        bool useBoundsOnBufferChannels) {
    std::unique_ptr<StorageDistribution> dNew = nullptr;

    // Compute throughput and storage dependencies
    execStorageDistribution(d, bufferChannels);

    // Throughput of d larger than current maximum of the set
    ds.thr = (std::max)(d.thr, ds.thr);

    // Create new storage distributions for every channel which
    // has a storage dependency in d
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++) {
        // Channel c has storage dependency?
        if (d.dep[c]) {
            // Do not enlarge the channel if its a self-edge
            if (bindingAwareSDFG->getChannel(c)->getSrcActor()->getId()
                == bindingAwareSDFG->getChannel(c)->getDstActor()->getId()) {
                continue;
            }

            // Do not enlarge channel above its current bound
            if (useBoundsOnBufferChannels
                && d.sp[c] >= bindingAwareSDFG->getChannel(c)->getInitialTokens()) {
                continue;
            }

            // Create new storage distribution with channel c enlarged
            dNew = newStorageDistribution();
            dNew->sz = d.sz + minSzStep[c];
            dNew->thr = 0;
            for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++) {
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
void SDFstateSpaceBindingAwareBufferAnalysis::exploreStorageDistributionSet(
        StorageDistributionSet& ds,
        bool *bufferChannels,
        bool useBoundsOnBufferChannels) {
    StorageDistribution* d = nullptr;

    // Explore all storage distributions contained in the set
    d = ds.distributions.get();
    while (d != nullptr) {
        // Explore distribution d
        exploreStorageDistribution(ds, *d, bufferChannels, useBoundsOnBufferChannels);

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
void SDFstateSpaceBindingAwareBufferAnalysis::findMinimalStorageDistributions(
        const double thrBound, bool *bufferChannels, bool useBoundsOnBufferChannels) {
    std::unique_ptr<StorageDistribution> d = nullptr;
    StorageDistribution* t = nullptr;
    StorageDistributionSet* ds = nullptr;
    StorageDistributionSet* dt = nullptr;

    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++) {
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
        exploreStorageDistributionSet(*ds, bufferChannels, useBoundsOnBufferChannels);

        // Reached maximum throughput or exceed thrBound
        if (ds->thr >= thrBound || ds->thr == maxThroughput) {
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
        for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++) {
            minStorageDistributions->distributions->sp[c] = 0;
        }
    }
}

/**
 * Throughput / storage-space trade-off exploration
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency) under the given binding constraints of the application
 * to the architecture graph. The vector 'bufferChannels' indicates for each
 * channel wether initial tokens can be added to it. This is only true for
 * channels modeling buffer space. When the 'useBoundsOnBufferChannels' flag
 * is true, the number of initial tokens (i.e. storage space) is not enlarged
 * above the current number of initial tokens on the edge (i.e. current storage
 * space). This storage space allocation is then a constraint on the maximum
 * amount of storage space that can be allocated to the channel. The exploration
 * ends as soon as the throughput reaches the throughput bound (thrBound) or all
 * minimal storage distributions are found (default).
 */
std::unique_ptr<StorageDistributionSet>
SDFstateSpaceBindingAwareBufferAnalysis::analyze(BindingAwareSDFG *bg,
                                                 bool *bufferChannels,
                                                 bool useBoundsOnBufferChannels,
                                                 const double thrBound) {
    // Binding-aware graph
    bindingAwareSDFG = bg;

    // Start with an empty set of storage distributions
    minStorageDistributions = nullptr;

    // Initialize bounds on the search space
    initBoundsSearchSpace(bindingAwareSDFG, bufferChannels);

    // Create a transition system
    transitionSystem = std::make_unique<TransitionSystem>(bindingAwareSDFG);

    // Search the space
    findMinimalStorageDistributions(thrBound, bufferChannels, useBoundsOnBufferChannels);

    return std::move(minStorageDistributions);
}

} // namespace SDF
