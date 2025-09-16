/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 14, 2006
 *
 *  Function        :   Compute latency of SDFG while using self-timed
 *                      execution.
 *
 *  History         :
 *      14-09-06    :   Initial version.
 *      12-02-08    :   Complete re-write in which code is restructured into
 *                      a C++ class.
 *
 * $Id: selftimed.cc,v 1.1.2.1 2010-04-22 07:18:39 mgeilen Exp $
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

#include "selftimed.h"
#include "base/exception/exception.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/algo/repetition_vector.h"
#include "sdf/basic_types.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>


namespace SDF {
/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::State::print(std::ostream &out) {
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
void SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::State::clear() {
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
bool SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::State::operator==(const State &s) {
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
void SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::initOutputActor() {
    RepetitionVector repVec;
    int min = INT_MAX;
    SDFactorRef a = nullptr;

    // Compute repetition vector
    repVec = computeRepetitionVector(*g);

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto& iter : g->getActors()) {
        if (repVec[iter->getId()] < min) {
            a = iter.get();
            min = repVec[a->getId()];
        }
    }

    // Set output actor and its repetition vector count
    outputActor = a->getId();
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
bool SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::storeState(State &s,
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
TDtime SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem ::computeThroughput(
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

// NOLINTBEGIN(*macro-usage)

#define CH(c) currentState.ch[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);

#define CH_TOKENS_PREV(c, n) (previousState.ch[c] >= (n))

// NOLINTEND(*macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::actorReadyToFire(const SDFactor& a) {
    // Check all input ports for tokens
    const auto& ports = a.getPorts();
    return std::all_of(ports.begin(), ports.end(), [this](const auto& p){
        const SDFchannel& c = *(p->getChannel());
        return (p->getType() != SDFport::SDFportType::In) || (CH_TOKENS(c.getId(), p->getRate()));
    });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::startActorFiring(const TimedSDFactor& a) {
    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();

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
bool SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::actorReadyToEnd(const SDFactor& a) {
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
void SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::endActorFiring(const SDFactor& a) {
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();

        // Actor is source of the channel?
        if (p->getType() == SDFport::SDFportType::Out) {
            PRODUCE(c->getId(), p->getRate());
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
SDFdiscreteTime SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::clockStep() {
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
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime SDFstateSpaceSelfTimedLatencyAnalysis::TransitionSystem::execSDFgraph(
        SDFactor *srcActor,
        SDFactor *dstActor,
        std::vector<SDFdiscreteTime> &timeSrcFire,
        std::vector<SDFdiscreteTime> &timeDstFire,
        uint distance) {
    bool foundRecurrentState = false;
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
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }

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

                // Firing the destination actor?
                if (dstActor->getId() == a->getId()) {
                    timeDstFire.push_back(globalTime);

                    // Explored full state-space and found all destination
                    // firings related to the source firings in te space?
                    if (timeDstFire.size() >= timeSrcFire.size() + distance
                        && foundRecurrentState) {
                        return computeThroughput(recurrentState);
                    }
                }
            }
        }

        // Start actor firings
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);

                // Only add source actor firing if the recurrent state is not
                // yet found
                if (!foundRecurrentState && srcActor->getId() == a->getId()) {
                    timeSrcFire.push_back(globalTime);
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
void SDFstateSpaceSelfTimedLatencyAnalysis::analyze(TimedSDFgraph *g,
                                                    SDFactor *srcActor,
                                                    SDFactor *dstActor,
                                                    TDtime &latency,
                                                    TDtime &throughput) {
    std::vector<SDFdiscreteTime> timeSrcFire;
    std::vector<SDFdiscreteTime> timeDstFire;
    RepetitionVector repVec;
    TimedSDFchannel *ch = nullptr;
    TDtime actorLatency = NAN;
    uint distance = 0;
    uint dist = 0;

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(*g)) {
        throw CException("[ERROR] Graph is not strongly connected.");
    }

    // Find distance between source and destination. Add self-loop to source
    // actor in graph g with no tokens
    ch = g->createChannel(srcActor, 1, srcActor, 1, 0);

    {
        // Create a transition system
        TransitionSystem transitionSystem(g);

        // Execute graph
        transitionSystem.execSDFgraph(srcActor, dstActor, timeSrcFire, timeDstFire, 0);
        distance = static_cast<uint>(timeDstFire.size());
        timeSrcFire.clear();
        timeDstFire.clear();

        // Compute repetition vector of the SDFG
        repVec = computeRepetitionVector(*g);

        // Distance (look at last firing of dst actor in iteration)
        dist = distance - (distance % repVec[dstActor->getId()]);
        dist = dist + repVec[dstActor->getId()] - 1;

        // Remove self-loop on source actor
        ch->getSrcActor()->removePort(ch->getSrcPort()->getName());
        ch->getDstActor()->removePort(ch->getDstPort()->getName());
        g->removeChannel(ch->getName());
    }
    // Create a transition system
    TransitionSystem transitionSystem(g);

    // Find all moments in time at which src and dst actor fire
    throughput = transitionSystem.execSDFgraph(srcActor, dstActor, timeSrcFire, timeDstFire, dist);

    // Compute latency
    latency = 0;
    for (uint i = 0; repVec[srcActor->getId()] * (i + 1) < static_cast<uint>(timeSrcFire.size())
                     && repVec[dstActor->getId()] * i + dist < timeDstFire.size();
         i++) {
        actorLatency = timeDstFire[(repVec[dstActor->getId()] * i) + dist]
                       - timeSrcFire[static_cast<size_t>(repVec[srcActor->getId()]) * i];

        latency = (std::max)(actorLatency, latency);
    }
}
} // namespace SDF