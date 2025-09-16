/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_periodic_ning_gao.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 31, 2008
 *
 *  Function        :   State-space based throughput analysis for Ning and
 *                      Gao's buffer sizing problem
 *
 *  History         :
 *      31-03-08    :   Initial version.
 *
 * $Id: static_periodic_ning_gao.cc,v 1.1.2.2 2010-04-24 23:52:43 mgeilen Exp $
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

#include "static_periodic_ning_gao.h"
#include "sdf/base/algo/repetition_vector.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace SDF {
/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::State::print(std::ostream &out) {
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

    for (uint i = 0; i < sp.size(); i++) {
        out << "sp[" << i << "] = " << ch[i] << std::endl;
    }

    out << "glbClk = " << glbClk << std::endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::State::clear() {
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
bool SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::State ::operator==(const State &s) {
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
void SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::initOutputActor() {
    RepetitionVector repVec;
    int min = INT_MAX;
    SDFactor* a = nullptr;

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
bool SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::storeState(State &s,
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
TDtime SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem ::computeThroughput(
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
#define SP(a) currentState.sp[a]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CH_SPACE(a, n) (SP(a) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);
#define CONSUME_SP(a, n) SP(a) = SP(a) - (n);
#define PRODUCE_SP(a, n) SP(a) = SP(a) + (n);

// NOLINTEND(*macro-usage)

/**
 * releaseStorageSpaceSharedOutputBuffer ()
 * The function checks whether space must be produced on the shared output
 * buffer. This is the case when the channel 'c' is the last channel to consume
 * the token from the shared output buffer connected to the source of the
 * channel. This condition is met when all outgoing channels of the source actor
 * of the channel 'c' contain less tokens than the channel 'c'.
 */
bool SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem ::
        releaseStorageSpaceSharedOutputBuffer(const SDFchannel& c) {
    const SDFactor& a = *(c.getSrcActor());
    const auto& ports = a.getPorts();
    // Check all output ports for tokens
    return std::all_of(ports.begin(), ports.end(), [&c, this](const auto& p){
        return (!(p->getType() == SDFport::SDFportType::Out)) ||
            (!(p->getChannel()->getId() != c.getId())) ||
            (CH(p->getChannel()->getId()) < CH(c.getId()));
    });

}

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::actorReadyToFire(const SDFactor& a) {
    // Check all input ports for tokens
    const auto& ports = a.getPorts();
    return std::all_of(ports.begin(), ports.end(), [this](const auto& p){
        const SDFchannel& c = *(p->getChannel());
        if (p->getType() == SDFport::SDFportType::In) {
            // if it is an input port, check tokens
            return CH_TOKENS(c.getId(), p->getRate());
        }
        // if it is an output port, check space
        return CH_SPACE(c.getSrcActor()->getId(), p->getRate());
    });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::startActorFiring(const TimedSDFactor& a) {
    bool consumedOutputSpace = false;

    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a.getPorts()) {
        const SDFchannel& c = *(p->getChannel());

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
bool SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::actorReadyToEnd(const SDFactor& a) {
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
void SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::endActorFiring(const SDFactor& a) {
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
SDFdiscreteTime
SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::clockStep(std::vector<SDFdiscreteTime> &startTime,
                                                                    std::vector<uint> &fireCnt,
                                                                    SDFdiscreteTime &period,
                                                                    TTime &globalTime) {
    SDFdiscreteTime step = UINT_MAX;

    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++) {
        SDFdiscreteTime actClk = 0;

        if (!currentState.actClk[a].empty()) {
            actClk = currentState.actClk[a].front();

            step = (std::min)(step, actClk);
        }

        // Time till next firing of the actor
        actClk = startTime[a] + fireCnt[a] * period - static_cast<SDFdiscreteTime>(globalTime);
        step = (std::min)(step, actClk);
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
TDtime SDFstateSpaceThroughputAnalysisNingGao::TransitionSystem::execSDFgraph(
        const std::unique_ptr<TBufSize[]>& sp, SDFdiscreteTime &period, std::vector<SDFdiscreteTime> &startTime) {
    bool startedActorFiring = false;
    bool actorWaitingToFire = false;
    std::vector<uint> fireCnt(g->nrActors(), 0);
    StatesIter recurrentState;
    TTime globalTime = 0;
    TTime clkStep = 0;
    int repCnt = 0;

    // startTime gives the last time at which an actor firing occurred
    startTime.resize(g->nrActors());

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();

    // Infinite space for output buffer of actors
    for (uint i = 0; i < g->nrActors(); i++) {
        SP(i) = TBUFSIZE_MAX;
    }

    // Initial tokens and space
    for (const auto& c : g->getChannels()) {

        // Not enough space for initial tokens?
        if (sp[c->getSrcActor()->getId()] < c->getInitialTokens()) {
            return 0;
        }

        CH(c->getId()) = c->getInitialTokens();

        // Decrease storage space of the actor when current storage space more
        // then room offered by assigned storage space minus the number of
        // tokens in channel c
        SP(c->getSrcActor()->getId()) = (std::min)(SP(c->getSrcActor()->getId()), sp[c->getSrcActor()->getId()] - c->getInitialTokens());
    }

    // Fire the actors
    while (true) {
        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {

                            // Compute throughput
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

        // Start actor firings. Note that with Ning and Gao, a start can enable
        // other starts.
        do {
            startedActorFiring = false;
            actorWaitingToFire = false;

            for (const auto& iter : g->getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                CId aId = a->getId();

                // Should actor fire at this point in time?
                while (startTime[aId] + fireCnt[aId] * period == globalTime) {
                    // Actor not ready to fire?
                    if (!actorReadyToFire(*a)) {
                        // Actor not ready to fire, schedule not respected
                        actorWaitingToFire = true;
                    }

                    // Fire actor a
                    startActorFiring(*a);
                    startedActorFiring = true;
                    fireCnt[aId]++;
                }
            }
        } while (startedActorFiring);

        // An actor waiting to be fired at this point in time?
        if (actorWaitingToFire) {
            // Actor not ready to fire, schedule not respected
            return 0;
        }

        // Clock step
        clkStep = clockStep(startTime, fireCnt, period, globalTime);
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
 * The function returns the throughput of the graph under the given storage
 * constraints and using the supplied static-periodic schedule. The graph
 * execution follows the operational semantics of Ning and Gao.
 */
TDtime SDFstateSpaceThroughputAnalysisNingGao::analyze(TimedSDFgraph& g,
                                                       std::unique_ptr<StorageDistribution>& d,
                                                       SDFdiscreteTime period,
                                                       std::vector<SDFdiscreteTime> &startTime) {
    TDtime thr = NAN;

    // Create a transition system
    TransitionSystem transitionSystem(&g);

    // Find the maximal throughput
    thr = transitionSystem.execSDFgraph(d->sp, period, startTime);

    return thr;
}

} // namespace SDF