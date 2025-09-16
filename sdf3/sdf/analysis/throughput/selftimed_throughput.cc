/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed_throughput.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   October 10, 2007
 *
 *  Function        :   State-space based throughput analysis
 *
 *  History         :
 *      10-10-07    :   Initial version.
 *
 * $Id: selftimed_throughput.cc,v 1.1.2.1 2010-04-22 07:18:40 mgeilen Exp $
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

#include "selftimed_throughput.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/algo/repetition_vector.h"
#include "sdf/base/timed/graph.h"
#include <algorithm>
#include <cmath>
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
void SDFstateSpaceThroughputAnalysis::TransitionSystem::State::print(std::ostream &out) {
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
void SDFstateSpaceThroughputAnalysis::TransitionSystem::State::clear() {
    for (auto & i : actClk) {
        i.clear();
    }

    for (uint32_t & i : ch) {
        i = 0;
    }

    glbClk = 0;
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceThroughputAnalysis::TransitionSystem::State::operator==(const State &s) {
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
void SDFstateSpaceThroughputAnalysis::TransitionSystem::initOutputActor() {
    RepetitionVector repVec;
    int min = INT_MAX;
    const SDFactor* a = nullptr;

    // Compute repetition vector
    repVec = computeRepetitionVector(*g);

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto& iter: g->getActors()) {
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
bool SDFstateSpaceThroughputAnalysis::TransitionSystem::storeState(State &s, StatesIter &pos) {
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
SDFstateSpaceThroughputAnalysis::TransitionSystem::computeThroughput(const StatesIter& cycleIter) {
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
bool SDFstateSpaceThroughputAnalysis::TransitionSystem::actorReadyToFire(const SDFactor& a) {
    // Check all input ports for tokens
    const auto& ports = a.getPorts();
    return std::all_of(ports.begin(), ports.end(), [this](const auto& p){
        const SDFchannel& c = *(p->getChannel());
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
void SDFstateSpaceThroughputAnalysis::TransitionSystem::startActorFiring(const TimedSDFactor& a) {
    // Consume tokens from inputs and space for output tokens
    for (const auto& p: a.getPorts()) {
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
bool SDFstateSpaceThroughputAnalysis::TransitionSystem::actorReadyToEnd(const SDFactor& a) {
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
void SDFstateSpaceThroughputAnalysis::TransitionSystem::endActorFiring(const SDFactor& a) {
    for (const auto& p: a.getPorts()) {
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
SDFdiscreteTime SDFstateSpaceThroughputAnalysis::TransitionSystem::clockStep() {
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
        for (auto &actFiringTime: currentState.actClk[a]) {
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
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime SDFstateSpaceThroughputAnalysis::TransitionSystem::execSDFgraph() {
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
    for (const auto& c: g->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    while (true) {
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++) {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings
        for (const auto& a: g->getActors()) {
            while (actorReadyToEnd(*a)) {
                if (outputActor == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
#ifdef _PRINT_STATESPACE
                        currentState.print(std::cout);
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
        for (const auto& iter: g->getActors()) {
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
TDtime SDFstateSpaceThroughputAnalysis::analyze(const TimedSDFgraph& g) {
    SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
    RepetitionVector repVec;
    TDtime thr = NAN;

    // Compute repetition vector
    repVec = computeRepetitionVector(g);

    // Check that the graph gr is a strongly connected graph
    if (!isStronglyConnectedGraph(g)) {
        // Split graph into its strongly connected components
        // and find minimal throughput of all components
        SDFgraphComponents comp = stronglyConnectedComponents(g);
        thr = TDTIME_MAX;

        for (auto co: comp) {
            RepetitionVector repVecGC;
            std::unique_ptr<TimedSDFgraph> gc = nullptr;
            TDtime thrGc = NAN;
            CId idFirstActor = 0;
            CId id = 0;

            // Construct graph from component
            gc = std::unique_ptr<TimedSDFgraph>(dynamic_cast<TimedSDFgraph*>(componentToSDFgraph(co).release()));

            // Id of first actor in component
            idFirstActor = (*gc->actorsBegin())->getId();

            // Relabel actors
            id = 0;
            for (const auto& u: gc->getActors()) {
                u->setId(id);
                id++;
            }

            // Relabel channels
            id = 0;
            for (const auto& c: gc->getChannels()) {
                c->setId(id);
                id++;
            }

            // Graph contains at least one channel
            if (gc->nrChannels() > 0) {
                // Compute throughput component
                thrGc = SDF::SDFstateSpaceThroughputAnalysis::analyze(*gc);

                // Compute repetition vector of component
                repVecGC = computeRepetitionVector(*gc);

                // Scale throughput wrt repetition vector component vs graph
                thrGc = (thrGc * repVecGC[0]) / static_cast<TDtime>(repVec[idFirstActor]);

                thr = (std::min)(thrGc, thr);
            }
        }

        return thr;
    }

    // Create a transition system
    TransitionSystem transitionSystem(g);

    // Find the maximal throughput
    thr = transitionSystem.execSDFgraph();

    return thr;
}
} // namespace SDF