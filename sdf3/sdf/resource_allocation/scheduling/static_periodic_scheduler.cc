/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_periodic_scheduler.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 21, 2008
 *
 *  Function        :   Construct static-periodic schedule with maximal
 *                      throughput
 *
 *  History         :
 *      21-08-08    :   Initial version.
 *
 * $Id: static_periodic_scheduler.cc,v 1.1.2.4 2010-04-25 02:08:52 mgeilen Exp $
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

#include "static_periodic_scheduler.h"
#include "base/exception/exception.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/algo/repetition_vector.h"
#include <algorithm>
#include <memory>

namespace SDF {

// #define _PRINT_STATESPACE

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::State::print(std::ostream &out) {
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
void SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::State::clear() {
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
bool SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::State::operator==(const State &s) {
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
void SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::initOutputActor() {
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
bool SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::storeState(State &s, StatesIter &pos) {
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
CFraction SDFstateSpaceStaticPeriodicScheduler::TransitionSystem ::computeThroughput(
        const StatesIter &cycleIter) {
    int nr_fire = 0;
    uint time = 0;

    // Check all state from stack till cycle complete
    for (StatesIter iter = cycleIter; iter != storedStates.end(); iter++) {
        State &s = *iter;

        // Number of states in cycle is equal to number of iterations
        // in the period
        nr_fire++;

        // Time between previous state
        time += s.glbClk;
    }

    return CFraction(nr_fire, static_cast<int>(time));
}

/******************************************************************************
 * SDF
 *****************************************************************************/

// NOLINTBEGIN (*macro-usage)

#define CH(c) currentState.ch[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);

// NOLINTEND (*macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::actorReadyToFire(const SDFactor& a) {
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
void SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::startActorFiring(const TimedSDFactor& a) {
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

#ifdef _PRINT_STATESPACE
    std::cout << "start: " << a.getName() << std::endl;
#endif
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::actorReadyToEnd(const SDFactor& a) {
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
void SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::endActorFiring(const SDFactor& a) {
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();

        // Actor is source of the channel?
        if (p->getType() == SDFport::SDFportType::Out) {
            PRODUCE(c->getId(), p->getRate());
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a.getId()].pop_front();

#ifdef _PRINT_STATESPACE
    std::cout << "end:   " << a.getName() << std::endl;
#endif
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFdiscreteTime SDFstateSpaceStaticPeriodicScheduler::TransitionSystem::clockStep(SDFdiscreteTime step) {
    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++) {
        SDFdiscreteTime actClk = 0;

        // Remaining execution time
        if (!currentState.actClk[a].empty()) {
            actClk = currentState.actClk[a].front();

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

#ifdef _PRINT_STATESPACE
    std::cout << "clk:   " << step << std::endl;
#endif

    return step;
}

/**
 * execSDFgraphComputeSchedule()
 * Execute the SDF graph and compute a static-periodic schedule. The function
 * returns the throughput of the SDF graph.
 */
CFraction SDFstateSpaceStaticPeriodicScheduler::TransitionSystem ::execSDFgraphComputeSchedule(
        std::vector<std::vector<int64_t>> &startTime,
        const int64_t period,
        const int64_t periodicity) {
    std::vector<std::vector<int64_t>> iterCnt;
    std::vector<int64_t> firingIdx;
    int64_t globalTime = 0;
    StatesIter recurrentState;
    RepetitionVector repVec;
    SDFdiscreteTime clkStep = 0;
    int repCnt = 0;

    // Compute repetition vector
    repVec = computeRepetitionVector(*g);

    // Initialize start time and iteration count vectors and the index of next
    // firing of every actor within the schedule
    firingIdx.resize(g->nrActors());
    startTime.resize(g->nrActors());
    iterCnt.resize(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        firingIdx[i] = 0;
        startTime[i].resize(static_cast<size_t>(repVec[i] * periodicity));
        iterCnt[i].resize(static_cast<size_t>(repVec[i] * periodicity));
        for (uint j = 0; j < repVec[i] * periodicity; j++) {
            startTime[i][j] = LONG_LONG_MIN;
            iterCnt[i][j] = 0;
        }
    }

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();
    // Initial tokens and space
    for (const auto& c : g->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    while (true) {
        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
#ifdef _PRINT_STATESPACE
                        printState(currentState, std::cout);
#endif
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
#ifdef _PRINT_STATESPACE
                            std::cout << "### end statespace exploration" << std::endl;
#endif
                            int64_t maxIterCnt = 0;
                            int64_t minStartTime = CSIZE_MAX;

                            // Find maximal iteration count
                            for (uint i = 0; i < g->nrActors(); i++) {
                                for (int64_t j : iterCnt[i]) {
                                    maxIterCnt = (std::max)(maxIterCnt, j);
                                }
                            }

                            // Shift start time till they are in the correct
                            // iteration and find lowest start time
                            for (uint i = 0; i < g->nrActors(); i++) {
                                for (uint j = 0; j < startTime[i].size(); j++) {
                                    startTime[i][j] =
                                            startTime[i][j] + (maxIterCnt - iterCnt[i][j]) * period;

                                    minStartTime = (std::min)(startTime[i][j], minStartTime);
                                }
                            }

                            // Shift all start times over the lowest start time
                            for (uint i = 0; i < g->nrActors(); i++) {
                                for (int64_t &j : startTime[i]) {
                                    j = j - minStartTime;
                                }
                            }

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
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Ready to fire actor a?
            while (startTime[a->getId()][static_cast<size_t>(firingIdx[a->getId()])] + period
                           <= globalTime
                   && actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);
                startTime[a->getId()][static_cast<size_t>(firingIdx[a->getId()])] = globalTime;
                iterCnt[a->getId()][static_cast<size_t>(firingIdx[a->getId()])]++;
                firingIdx[a->getId()] = (firingIdx[a->getId()] + 1) % static_cast<int64_t>(startTime[a->getId()].size());
            }
        }

        // Clock step
        clkStep = UINT_MAX;
        for (uint a = 0; a < g->nrActors(); a++) {
            SDFdiscreteTime actClk = 0;

            // Time till next point from which actor is allowed to fire
            if (startTime[a][static_cast<size_t>(firingIdx[a])] + period - globalTime > 0) {
                actClk = static_cast<SDFdiscreteTime>(startTime[a][static_cast<size_t>(firingIdx[a])]
                                              + period - globalTime);
                clkStep = (std::min)(clkStep, actClk);
            }
        }
        clkStep = clockStep(clkStep);
        globalTime += clkStep;

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return CFraction(0);
        }
    }

    return CFraction(0);
}

/**
 * execSDFgraphStaticPeriodic()
 * Execute the SDF graph following the supplied schedule. The function returns
 * the throughput of the graph under the given schedule or zero when the
 * schedule cannot be met.
 */
CFraction SDFstateSpaceStaticPeriodicScheduler::TransitionSystem ::execSDFgraphStaticPeriodic(
        const std::vector<std::vector<int64_t>> &startTime, const int64_t period) {
    std::vector<std::vector<int64_t>> iterCnt;
    std::vector<int64_t> firingIdx;
    int64_t globalTime = 0;
    StatesIter recurrentState;
    SDFdiscreteTime clkStep = 0;
    int repCnt = 0;

    // Initialize iteration count vectors and th index of next firing of
    // every actor within the schedule
    iterCnt.resize(g->nrActors());
    firingIdx.resize(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        firingIdx[i] = 0;
        iterCnt[i].resize(startTime[i].size());
        for (int64_t &j : iterCnt[i]) {
            j = 0;
        }
    }

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    while (true) {
        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
#ifdef _PRINT_STATESPACE
                        printState(currentState, std::cout);
#endif
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
#ifdef _PRINT_STATESPACE
                            std::cout << "### end statespace exploration" << std::endl;
#endif
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
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Should next firing of actor a start now?
            while (startTime[a->getId()][static_cast<size_t>(firingIdx[a->getId()])]
                           + period
                                     * iterCnt[a->getId()]
                                              [static_cast<size_t>(firingIdx[a->getId()])]
                   <= globalTime) {
                // Ready to fire actor a?
                if (actorReadyToFire(*a)) {
                    // Fire actor a
                    startActorFiring(*a);
                    iterCnt[a->getId()][static_cast<size_t>(firingIdx[a->getId()])]++;
                    firingIdx[a->getId()] =
                            (firingIdx[a->getId()] + 1) % static_cast<int64_t>(startTime[a->getId()].size());
                } else {
                    // Actor not ready to fire, violation of schedule
                    return CFraction(0);
                }
            }
        }

        // Clock step
        clkStep = UINT_MAX;
        for (uint a = 0; a < g->nrActors(); a++) {
            SDFdiscreteTime actClk = 0;

            // Time till next point from which actor is allowed to fire
            if (startTime[a][static_cast<size_t>(firingIdx[a])]
                        + period * iterCnt[a][static_cast<size_t>(firingIdx[a])] - globalTime
                > 0) {
                actClk = static_cast<SDFdiscreteTime>(
                        startTime[a][static_cast<size_t>(firingIdx[a])]
                        + (period * iterCnt[a][static_cast<size_t>(firingIdx[a])]) - globalTime);
                clkStep = (std::min)(clkStep, actClk);
            }
        }
        clkStep = clockStep(clkStep);
        globalTime += clkStep;

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return CFraction(0);
        }
    }

    return CFraction(0);
}

/**
 * execSDFgraphSelfTimed()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
CFraction SDFstateSpaceStaticPeriodicScheduler::TransitionSystem ::execSDFgraphSelfTimed() {
    StatesIter recurrentState;
    SDFdiscreteTime clkStep = 0;
    int repCnt = 0;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    while (true) {
        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
#ifdef _PRINT_STATESPACE
                        printState(currentState, std::cout);
#endif
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
#ifdef _PRINT_STATESPACE
                            std::cout << "### end statespace exploration" << std::endl;
#endif
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
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            // Ready to fire actor a?
            while (actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);
            }
        }

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return CFraction(0);
        }
    }

    return CFraction(0);
}

/**
 * schedule ()
 * Compute a static-periodic schedule with maximal throughput for the supplied
 * SDFG.
 */
void SDFstateSpaceStaticPeriodicScheduler::schedule(TimedSDFgraph *g) {
    std::vector<std::vector<int64_t>> startTime;
    int64_t periodicity = 0;
    int64_t period = 0;
    CFraction thr;
    CFraction thrSchedule;

    // The graph must be strongly connected when constructing the schedule
    if (!isStronglyConnectedGraph(*g)) {
        throw CException("[ERROR] The graph is not strongly connected.");
    }

    // Create a transition system
    TransitionSystem transitionSystem(g);

    // Compute the maximal throughput of the graph
    thr = transitionSystem.execSDFgraphSelfTimed();

    // Reduce throughput fraction to irreducible form
    thr = thr.lowestTerm();

    // Compute the periodicity and period of the schedule
    periodicity = thr.numerator();
    period = thr.denominator();

    // Compute the schedule
    transitionSystem.execSDFgraphComputeSchedule(startTime, period, periodicity);

    // Execute the schedule
    thrSchedule = transitionSystem.execSDFgraphStaticPeriodic(startTime, period);

    std::cout << "period: " << period << std::endl;
    for (uint i = 0; i < g->nrActors(); i++) {
        for (uint j = 0; j < startTime[i].size(); j++) {
            std::cout << "(" << g->getActor(i)->getName() << "," << j << "): " << startTime[i][j]
                      << std::endl;
        }
    }
    std::cout << "thr: " << thrSchedule << std::endl;

    if (thr != thrSchedule) {
        std::cout << "optimal: " << thr << std::endl;
    }
}

} // namespace SDF
