/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed_minimal.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2006
 *
 *  Function        :   Compute latency of SDFG while using self-timed
 *                      execution both constraining the execution of the
 *                      source actor to achieve minimal latency with maximal
 *                      throughput.
 *
 *  History         :
 *      08-11-06    :   Initial version.
 *      12-02-08    :   Complete re-write in which code is restructured into
 *                      a C++ class.
 *
 * $Id: selftimed_minimal.cc,v 1.1.2.2 2010-04-24 23:52:42 mgeilen Exp $
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

#include "selftimed_minimal.h"
#include "base/exception/exception.h"
#include "minimal.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/algo/repetition_vector.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>

namespace SDF {

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::State ::print(
        std::ostream &out) {
    out << "### State ###" << std::endl;

    for (uint i = 0; i < actClk.size(); i++) {
        out << "actClk[" << i << "] =";

        for (std::list<SDFdiscreteTime>::const_iterator iter = actClk[i].begin(); iter != actClk[i].end();
             iter++) {
            out << " " << (*iter) << ", ";
        }

        out << std::endl;
    }

    for (uint i = 0; i < ch.size(); i++) {
        out << "ch[" << i << "] = " << ch[i] << std::endl;
    }

    out << "glbClk = " << glbClk << std::endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::State ::clear() {
    for (auto &i : actClk) {
        i.clear();
    }

    for (uint32_t &i : ch) {
        i = 0;
    }

    glbClk = 0;
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::State ::operator==(
        const State &s) {
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
void SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::initOutputActor(
        SDFactor *outActor) {
    RepetitionVector repVec;

    // Compute repetition vector
    repVec = computeRepetitionVector(*g);

    // Set output actor and its repetition vector count
    outputActor = outActor->getId();
    outputActorRepCnt = repVec[outputActor];
}

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::storeState(State &s,
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
TDtime SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::computeThroughput(
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
 * SDF
 *****************************************************************************/

// NOLINTBEGIN(*-macro-usage)

#define CH(c) currentState.ch[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);

#define CH_TOKENS_PREV(c, n) (previousState.ch[c] >= (n))

// NOLINTEND(*-macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::actorReadyToFire(
        const SDFactor& a) {
    // Check all input ports for tokens using std::all_of
    const auto& ports = a.getPorts();
    return std::all_of(
        ports.begin(), ports.end(),
        [&](const auto& p) {
            SDFchannel *c = p->getChannel();
            // Only check input ports
            if (p->getType() == SDFport::SDFportType::In) {
                return CH_TOKENS(c->getId(), p->getRate());
            }
            return true;
        }
    );
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::startActorFiring(
        const TimedSDFactor& a) {
    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a.getPorts()) {
        const SDFchannel *c = p->getChannel();

        // Actor is destination of the channel?
        if (p->getType() == SDFport::SDFportType::In) {
            CONSUME(c->getId(), p->getRate());
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
bool SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::actorReadyToEnd(const SDFactor& a) {
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
c * list of active firings.
 */
void SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::endActorFiring(const SDFactor& a) {
    for (const auto& p : a.getPorts()) {
        const SDFchannel& c = *(p->getChannel());

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
SDFdiscreteTime
SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem ::clockStep(const uint maxStep) {
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

    // Step does not exceed maximal size?
    step = (std::min)(step, maxStep);

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
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * A list of timestamps at which the destination actor fires is returned.
 */
std::shared_ptr<SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::TimingConstraintFiring>
SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::execSDFgraph(const SDFactor& dstActor) {
    std::shared_ptr<TimingConstraintFiring> timeDstFire = nullptr;
    std::shared_ptr<TimingConstraintFiring> timeConstraint = nullptr;
    std::shared_ptr<TimingConstraintFiring> lastTimeConstraint = nullptr;
    SDFdiscreteTime clkStep = 0;
    SDFdiscreteTime globalTime = 0;
    StatesIter recurrentState;
    int repCnt = 0;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();
    previousState.init(g->nrActors(), g->nrChannels());
    previousState.clear();

    // Initial tokens and space
    for (const auto& c : g->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    // Fire the actors
    while (true) {
        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
                            // Close cycle on firing times dst actor.
                            // Last timing constraint added is related to
                            // first timing constraint in the list which
                            // was added after the recurrent state
                            timeConstraint = timeDstFire;
                            while (timeConstraint != nullptr) {
                                // timeConstraint was stored in same state
                                // as the recurrent state?
                                if (timeConstraint->lastStoredState == currentState) {
                                    lastTimeConstraint->next = timeConstraint;
                                    return timeDstFire;
                                }

                                // Try next time constraint
                                timeConstraint = timeConstraint->next;
                            }

                            throw CException("[ERROR] No recurrent state"
                                             " found.");
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
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);

                // Firing the destination actor?
                if (dstActor.getId() == a->getId()) {
                    // Allocate memory for new timing constraint
                    timeConstraint = std::make_shared<TimingConstraintFiring>();

                    // Global time indicates time between subsequent firings of
                    // dst actor
                    timeConstraint->timeTillNextFiring = globalTime;
                    globalTime = 0;

                    // Remember last state stored (needed to close cycle)
                    if (!storedStates.empty()) {
                        timeConstraint->lastStoredState = storedStates.back();
                    } else {
                        timeConstraint->lastStoredState.glbClk = UINT_MAX;
                    }

                    // Default values
                    timeConstraint->nrFiringsEnabled = 0;
                    timeConstraint->visit = false;
                    timeConstraint->next = nullptr;

                    // Add timing constraint to the end of the list
                    if (lastTimeConstraint == nullptr) {
                        timeConstraint->id = 0;
                        timeDstFire = timeConstraint;
                    } else {
                        timeConstraint->id = lastTimeConstraint->id + 1;
                        lastTimeConstraint->next = timeConstraint;
                    }
                    lastTimeConstraint = timeConstraint;
                }
            }
        }

        // Clock step
        clkStep = clockStep();

        // Advance the global time (this is the total time since the beginning
        // of everything)
        globalTime += clkStep;

        // Deadlocked?
        if (clkStep == UINT_MAX) {
            return timeDstFire;
        }
    }

    return nullptr;
}

/**
 * execConstrainedSDFgraph ()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The inverse of throughput is returned. The graph is executed in a self-timed
 * manner. Except that the firings of the source actor may only occur after the
 * time at which the destination actor fires minus the latency of the graph.
 */
TDtime SDFstateSpaceSelfTimedMinimalLatencyAnalysis::TransitionSystem::execConstrainedSDFgraph(
        const SDFactor& srcActor,
        std::shared_ptr<TimingConstraintFiring>& timeSrcFire,
        double latency,
        const SDFactor& dstActor,
        std::vector<SDFdiscreteTime> &srcTimeFire,
        std::vector<SDFdiscreteTime> &dstTimeFire,
        uint distance) {
    uint64_t globalTime = 0;
    uint64_t previousStep = -(static_cast<uint64_t>(latency));
    bool foundRecurrentState = false;
    uint nrAllowedSrcFirings = 0;
    StatesIter recurrentState;
    SDFdiscreteTime clkStep = 0;
    int repCnt = 0;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();
    previousState.init(g->nrActors(), g->nrChannels());
    previousState.clear();

    // Initial tokens and space
    for (const auto& c : g->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    // Enable all firings of source which can start from the beginning of time
    while (globalTime >= previousStep + timeSrcFire->timeTillNextFiring) {
        // Increase number of allowed firings
        nrAllowedSrcFirings += timeSrcFire->nrFiringsEnabled;

        // Is this an actual firings of the dummy source actor?
        if (timeSrcFire->nrFiringsEnabled > 0) {
            srcTimeFire.push_back(static_cast<SDFdiscreteTime>(globalTime));
        }

        // Store global time at which current step occurs
        previousStep += timeSrcFire->timeTillNextFiring;

        // Next
        timeSrcFire = timeSrcFire->next;
    }

    // Fire the actors
    while (true) {
        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Recurrent state not yet seen
                        if (!foundRecurrentState) {
                            // Add state to hash of visited states
                            if (!storeState(currentState, recurrentState)) {
                                foundRecurrentState = true;
                            }
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
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(*a)) {
                // Current actor is the source actor
                if (a->getId() == srcActor.getId()) {
                    // Is the source allowed to fire?
                    if (nrAllowedSrcFirings > 0) {
                        // Fire actor a
                        startActorFiring(*a);

                        // Decrease the number of allowed firings
                        nrAllowedSrcFirings--;
                    } else {
                        // No more firings allowed of the source actor
                        break;
                    }
                } else {
                    // Fire actor a
                    startActorFiring(*a);
                }

                // Current actor is the destination actor?
                if (dstActor.getId() == a->getId()) {
                    dstTimeFire.push_back(static_cast<SDFdiscreteTime>(globalTime));

                    // Explored full state-space and found all destination
                    // firing, related to the source firings in the space?
                    if (dstTimeFire.size() >= srcTimeFire.size() + distance
                        && foundRecurrentState) {
                        return computeThroughput(recurrentState);
                    }
                }
            }
        }

        // Make a clock step. This clock step should never exceed the time when
        // the next source actor firing should occur.
        clkStep = clockStep(static_cast<SDFdiscreteTime>(previousStep) + timeSrcFire->timeTillNextFiring
                            - static_cast<SDFdiscreteTime>(globalTime));

        // Advance the global time (this is the total time since the beginning
        // of everything)
        globalTime += clkStep;

        // Enable all firings of source which can start from this point in time
        while (globalTime >= previousStep + timeSrcFire->timeTillNextFiring) {
            // Increase number of allowed firings
            nrAllowedSrcFirings += timeSrcFire->nrFiringsEnabled;

            // Is this an actual firings of the dummy source actor?
            if (timeSrcFire->nrFiringsEnabled > 0 && !foundRecurrentState) {
                srcTimeFire.push_back(static_cast<SDFdiscreteTime>(globalTime));
            }

            // Store global time at which current step occurs
            previousStep += timeSrcFire->timeTillNextFiring;

            // Next
            timeSrcFire = timeSrcFire->next;
        }

        // Deadlocked?
        if (clkStep == UINT_MAX) {
            return 0;
        }
    }

    return 0;
}

/**
 * analyze ()
 * Compute the throughput of an SDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
void SDFstateSpaceSelfTimedMinimalLatencyAnalysis::analyze(const TimedSDFgraph& g,
                                                           SDFactor& srcActor,
                                                           SDFactor& dstActor,
                                                           TDtime &latency,
                                                           TDtime &throughput) {
    SDFstateSpaceMinimalLatencyAnalysis stateSpaceMinimalLatencyAnalysis;
    std::shared_ptr<TransitionSystem::TimingConstraintFiring> timeDstFire = nullptr;
    std::shared_ptr<TransitionSystem::TimingConstraintFiring> dstFirings = nullptr;
    std::shared_ptr<TransitionSystem::TimingConstraintFiring> timeSrcFire = nullptr;
    std::shared_ptr<TransitionSystem::TimingConstraintFiring> srcFirings = nullptr;
    std::shared_ptr<TransitionSystem::TimingConstraintFiring> lastSrcFirings = nullptr;
    std::shared_ptr<TransitionSystem::TimingConstraintFiring> x = nullptr;
    std::vector<SDFdiscreteTime> srcTimeFire;
    std::vector<SDFdiscreteTime> dstTimeFire;
    std::shared_ptr<TransitionSystem> transitionSystem = nullptr;
    TimedSDFchannel *ch = nullptr;
    double actorLatency = NAN;
    double minLatency = NAN;
    TimedSDFactor::Processor proc;
    TimedSDFactor *dstDummyActor = nullptr;
    uint nrFirings = 0;
    uint distance = 0;
    RepetitionVector repVec;
    std::shared_ptr<TimedSDFgraph> gr = nullptr;

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g)) {
        throw CException("[ERROR] Graph is not strongly connected.");
    }

    // Compute repetition vector of the SDFG
    repVec = computeRepetitionVector(g);

    // Create a copy of the graph (the graph is modified in the function!)
    SDFcomponent comp(g.getParent(), g.getId(), g.getName());
    gr = TimedSDFgraph::dynamicCast(g.clone(comp));

    // Add destination dummy actor
    dstDummyActor = gr->createActor();
    proc.type = "default";
    proc.execTime = 0;
    proc.stateSize = 0;
    dstDummyActor->addProcessor(&proc);
    dstDummyActor->setDefaultProcessor("default");

    // Connect dummy destination actor to real destination actor
    gr->createChannel(
            gr->getActor(dstActor.getName()), 1, dstDummyActor, repVec[dstActor.getId()], 0);

    // Add self-loop to source actor in graph gr with no tokens
    ch = gr->createChannel(
            gr->getActor(srcActor.getName()), 1, gr->getActor(srcActor.getName()), 1, 0);

    // Create a transition system
    transitionSystem = std::make_shared<TransitionSystem>(*gr, dstDummyActor);

    // Find all moments in time at which dst actor fire
    timeDstFire = transitionSystem->execSDFgraph(*dstDummyActor);

    // Distance is equal to number of firings of the destination actor
    distance = 0;
    dstFirings = timeDstFire;
    while (dstFirings != nullptr) {
        // Next
        distance++;
        dstFirings = dstFirings->next;
    }

    // Remove self-loop on source actor
    ch->getSrcActor()->removePort(ch->getSrcPort()->getName());
    ch->getDstActor()->removePort(ch->getDstPort()->getName());
    gr->removeChannel(ch->getName());

    // Create a transition system
    transitionSystem = std::make_shared<TransitionSystem>(*gr, dstDummyActor);

    // Find all moments in time at which dst actor fire
    timeDstFire = transitionSystem->execSDFgraph(*dstDummyActor);

    // Compute timing constraints for the source actor
    nrFirings = 0;
    dstFirings = timeDstFire;
    lastSrcFirings = nullptr;
    timeSrcFire = nullptr;
    while (dstFirings != nullptr) {
        if (dstFirings->visit) {
            // Find matching firing in source
            // Length of cycle in source and destination has same size!
            uint lengthCycle = 0;
            x = dstFirings;
            do {
                x->visit = false;
                lengthCycle++;
                x = x->next;
            } while (x->visit);

            x = timeSrcFire;
            while (x->id != lastSrcFirings->id - lengthCycle + 1) {
                x = x->next;
            }
            lastSrcFirings->next = x;
            break;
        }

        if (nrFirings >= distance) {
            dstFirings->visit = true;
        }

        // Create new timing constraint for source actor firings
        srcFirings = std::make_shared<TransitionSystem::TimingConstraintFiring>();
        srcFirings->timeTillNextFiring = dstFirings->timeTillNextFiring;
        srcFirings->visit = false;

        if (nrFirings < distance) {
            srcFirings->nrFiringsEnabled = 0;
        } else {
            srcFirings->nrFiringsEnabled = repVec[srcActor.getId()];
        }

        // Add timing constraint to list of constraints
        if (lastSrcFirings == nullptr) {
            srcFirings->id = 0;
            timeSrcFire = srcFirings;
        } else {
            srcFirings->id = lastSrcFirings->id + 1;
            lastSrcFirings->next = srcFirings;
        }
        lastSrcFirings = srcFirings;

        // Next
        nrFirings++;
        dstFirings = dstFirings->next;
    }

    // Compute minimal latency of the graph
    SDFstateSpaceMinimalLatencyAnalysis::analyze(g, srcActor, dstActor, minLatency, throughput);

    // Create a transition system
    transitionSystem = std::make_shared<TransitionSystem>(*gr, dstDummyActor);

    // Execute graph with given timing constraints to compute throughput
    throughput = transitionSystem->execConstrainedSDFgraph(
            srcActor, timeSrcFire, minLatency, *dstDummyActor, srcTimeFire, dstTimeFire, distance);

    // Compute latency
    latency = 0;
    for (uint i = 0; i < srcTimeFire.size() && i + distance < dstTimeFire.size(); i++) {
        actorLatency = dstTimeFire[i + distance] - srcTimeFire[i];

        latency = (std::max)(actorLatency, latency);
    }
}
} // namespace SDF
