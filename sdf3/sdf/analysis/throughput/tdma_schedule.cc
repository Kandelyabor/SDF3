/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tdma_schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Throughput calculation for SDFG mapped to an MPSoC
 *                      with TDMA arbitration on the processors and a
 *                      static-order schedule for actors running on the same
 *                      processor.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: tdma_schedule.cc,v 1.2.2.1 2010-04-22 07:18:41 mgeilen Exp $
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

#include "tdma_schedule.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/algo/repetition_vector.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace SDF {
// #define _PRINT_STATESPACE

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * clear ()
 * The function sets the state to zero.
 */
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::State ::clear() {
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
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::State ::operator==(
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::State ::print(
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::initOutputActor() {
    RepetitionVector repVec;
    int min = INT_MAX;
    SDFactor* a = nullptr;

    // Compute repetition vector
    repVec = computeRepetitionVector(*bindingAwareSDFG);

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto& iter : bindingAwareSDFG->getActors()) {
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::checkBindingAwareSDFG() {
    CId tileId = 0;

    // Check that all actor that are bound to a processors are bound to a
    // processor which has a schedule
    for (const auto& iter : bindingAwareSDFG->getActors()) {
        tileId = bindingAwareSDFG->getBindingOfActorToTile(*iter);

        if (tileId != ACTOR_NOT_BOUND) {
            if (bindingAwareSDFG->getConstScheduleOnTile(tileId).empty()) {
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
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::storeState(State &s,
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
TDtime SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::computeThroughput(
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

/**
 * computeTileUtilization ()
 * The function calculates the fraction of the time that a processor is busy
 * during the periodic part of the execution.
 */
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::computeTileUtilization(
        const StatesIter &recurrentState, std::vector<double> &tileUtilization) {
    RepetitionVector repVec = computeRepetitionVector(*bindingAwareSDFG);
    double nrItersInPeriod = 0;
    double lengthOfPeriod = 0;

    // Initialize tile utilizations
    tileUtilization.resize(bindingAwareSDFG->nrTilesInPlatformGraph());
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++) {
        tileUtilization[t] = 0;
    }

    // Check all state from stack till cycle complete
    for (StatesIter iter = recurrentState; iter != storedStates.end(); iter++) {
        State &s = *iter;

        // Number of states in cycle is equal to number of iterations
        // in the period
        nrItersInPeriod++;

        // Time between previous state
        lengthOfPeriod += s.glbClk;
    }

    // The activity of a processor is given by the sum of execution time of the
    // actors executed on the processor multiplied with the number of
    // invocations of an actor per iteration and the number of iterations per
    // period.
    for (const auto& iter : bindingAwareSDFG->getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        uint t = bindingAwareSDFG->getBindingOfActorToTile(*a);

        if (t != ACTOR_NOT_BOUND) {
            tileUtilization[t] = tileUtilization[t]
                                 + a->getExecutionTime() * repVec[a->getId()] * nrItersInPeriod;
        }
    }

    // Normalize length of execution on processors with length of the period
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++) {
        tileUtilization[t] = tileUtilization[t] / lengthOfPeriod;
    }
}

/******************************************************************************
 * SDF
 *****************************************************************************/

// NOLINTBEGIN(*macro-usage)

#define CH(c) currentState.ch[c]
#define TDMA_POS(p) currentState.tdmaPos[p]
#define SOS_POS(p) currentState.schedulePos[p]

#define SOS(p) (bindingAwareSDFG->getConstScheduleOnTile(p))
#define SOS_ENTRY(p) (SOS(p).getConstScheduleEntry(SOS_POS(p)))
#define SOS_NEXT_POS(p) (SOS(p).next(SOS_POS(p)))

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
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::actorReadyToFire(const SDFactor& a) {
    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(a) != ACTOR_NOT_BOUND) {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);

        // Actor not scheduled on processor?
        if (SOS_ENTRY(p)->actor->getId() != a.getId()) {
            return false;
        }
    }

    const auto& ports = a.getPorts();
    return std::all_of(ports.begin(), ports.end(), [this](const auto& p){
        const SDFchannel& c = *(p->getChannel());
        // Check all input ports for tokens
        return (!(p->getType() == SDFport::SDFportType::In)) ||
            (CH_TOKENS(c.getId(), p->getRate()));
    });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::startActorFiring(
        const TimedSDFactor& a) {
    SDFdiscreteTime execTime = 0;
    SDFdiscreteTime completionTime = 0;
    SDFdiscreteTime timeTileStartOfSlice = 0;
    SDFdiscreteTime waitingTime = 0;
    int remainingExecTime = 0;
    int nrOfFullRotationsInNonReservedPart = 0;

    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();

        // Actor is destination of the channel?
        if (p->getType() == SDFport::SDFportType::In) {
            CONSUME(c->getId(), p->getRate());
        }
    }

    // Execution time of the actor
    execTime = a.getDiscreteExecutionTime();

    // Compute time needed to complete actor firing
    if (bindingAwareSDFG->getBindingOfActorToTile(a) == ACTOR_NOT_BOUND) {
        completionTime = execTime;
    } else {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);

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
            // Start the execution time immediatly, wait the number of wheel
            // rotations after the current slice has been used to complete the
            // firing
            remainingExecTime = static_cast<int>(execTime)
                                - static_cast<int>(bindingAwareSDFG->getTDMAsizeOnTile(p))
                                + static_cast<int>(currentState.tdmaPos[p]);

            if (remainingExecTime < 0) {
                waitingTime = 0;
            } else {
                waitingTime = static_cast<SDFdiscreteTime>(
                        ceil(static_cast<float>(remainingExecTime)
                             / static_cast<float>(bindingAwareSDFG->getTDMAsliceOnTile(p)))
                        * (static_cast<float>(bindingAwareSDFG->getTDMAsizeOnTile(p))
                           - static_cast<float>(bindingAwareSDFG->getTDMAsliceOnTile(p))));
            }

            completionTime = execTime + waitingTime;
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a.getId()].push_back(completionTime);

#ifdef _PRINT_STATESPACE
    std::cout << "start: " << a.getName() << std::endl;
#endif
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::actorReadyToEnd(const SDFactor& a) {
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::endActorFiring(const SDFactor& a) {
    for (const auto& p : a.getPorts()) {
        SDFchannel *c = p->getChannel();

        // Actor is source of the channel?
        if (p->getType() == SDFport::SDFportType::Out) {
            PRODUCE(c->getId(), p->getRate());
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a.getId()].pop_front();

    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(a) != ACTOR_NOT_BOUND) {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);

        // Advance the schedule to the next state
        SOS_POS(p) = SOS_NEXT_POS(p);
    }

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
SDFdiscreteTime SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::clockStep() {
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

#ifdef _PRINT_STATESPACE
    std::cout << "clk:   " << step << std::endl;
#endif

    return step;
}

/**
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem ::execSDFgraph(
        std::vector<double> &tileUtilization) {
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
    for (const auto& c : bindingAwareSDFG->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    // Initial schedules
    for (uint p = 0; p < bindingAwareSDFG->nrTilesInPlatformGraph(); p++) {
        SOS_POS(p) = 0;
        TDMA_POS(p) = 0;
    }

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    while (true) {
        // Store partial state to check for progress
        for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings
        for (const auto& a : bindingAwareSDFG->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
#ifdef _PRINT_STATESPACE
                        currentState.print(std::cout);
#endif
                        // Add state to hash of visited states
                        if (!storeState(currentState, recurrentState)) {
                            computeTileUtilization(recurrentState, tileUtilization);

#ifdef _PRINT_STATESPACE
                            std::cout << "### end statespace exploration" << std::endl;
#endif

                            // Done
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
        for (const auto& iter : bindingAwareSDFG->getActors()) {
            auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

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
            std::cout << "Deadlock!" << std::endl;
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return 0;
        }
    }

    return 0;
}

/**
 * stateSpaceThroughputAnalysisBoundedSDFG ()
 * The function computes the throughput of a binding-aware SDFG bg. It assumes
 * that the platform uses a TDMA resource arbitration mechanism on the
 * processors. An Actor is scheduled on a processor if its input tokens are
 * available and the static-order schedule indicates that the actor is allowed
 * to fire. Actors which are not mapped to a processor, only wait for their
 * input tokens.
 */
double SDFstateSpaceBindingAwareThroughputAnalysis::analyze(const BindingAwareSDFG& bg,
                                                            std::vector<double> &tileUtilization) {
    double thr = NAN;

    // Check that the application graph is a strongly connected graph
    if (!isStronglyConnectedGraph(bg)) {
        throw CException("Graph is not strongly connected.");
    }

    // Create a transition system
    TransitionSystem transitionSystem(bg);

    // Find the maximal throughput
    thr = transitionSystem.execSDFgraph(tileUtilization);

    return thr;
}
} // namespace SDF