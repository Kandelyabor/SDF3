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
 * $Id: selftimed_throughput.cc,v 1.1.2.3 2010-07-29 20:34:08 mgeilen Exp $
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

#include "csdf/base/algo/components.h"
#include <algorithm>


namespace CSDF {
// #define _PRINT_STATESPACE

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::State::print(std::ostream &out) {
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

    out << "glbClk = " << glbClk << std::endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::State::clear() {
    for (unsigned int &actSeqPo : actSeqPos) {
        actSeqPo = 0;
    }

    for (auto &i : actClk) {
        i.clear();
    }

    for (uint32_t &i : ch) {
        i = 0;
    }

    glbClk = 0;
}

/**
 * setInitialTokens ()
 * The function sets the initial tokens on the channels.
 */
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::State::setInitialTokens(
        const TimedCSDFgraph &g) {
    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        const CSDFchannel& c = *(*iter);
        this->ch[c.getId()] = c.getInitialTokens();
    }
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool CSDFstateSpaceThroughputAnalysis::TransitionSystem::State::operator==(const State &s) {
    if (glbClk != s.glbClk) {
        return false;
    }

    for (uint i = 0; i < actSeqPos.size(); i++) {
        if (actSeqPos[i] != s.actSeqPos[i]) {
            return false;
        }
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
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::initOutputActor() {
    CSDFgraph::RepetitionVector repVec;
    int min = INT_MAX;
    const CSDFactor *a = nullptr;

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
bool CSDFstateSpaceThroughputAnalysis::TransitionSystem::storeState(State &s, StatesIter &pos) {
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
CSDFstateSpaceThroughputAnalysis::TransitionSystem::computeThroughput(const StatesIter &cycleIter) {
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
 * CSDF
 *****************************************************************************/

// NOLINTBEGIN(*macro-usage)

#define CH(c) currentState.ch[c]
#define CH_S(s, c) s.ch[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define CONSUME_S(s, c, n) CH_S(s, c) = CH_S(s, c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);
#define PRODUCE_S(s, c, n) CH_S(s, c) = CH_S(s, c) + (n);

#define CH_TOKENS_PREV(c, n) (previousState.ch[c] >= (n))

#define ACT_SEQ_POS(a) currentState.actSeqPos[a]
#define ACT_SEQ_POS_S(s, a) s.actSeqPos[a]

// NOLINTEND(*macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool CSDFstateSpaceThroughputAnalysis::TransitionSystem::actorReadyToFire(const CSDFactor& a) {
    return actorReadyToFire(currentState, a);
}

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool CSDFstateSpaceThroughputAnalysis::TransitionSystem::actorReadyToFire(const State &s,
                                                                          const CSDFactor& a) {
    // Check all input ports for tokens
    const auto& ports = a.getConstPorts();
    return std::all_of(ports.begin(), ports.end(), [&s, &a](const auto& p){
        const CSDFchannel& c = *(p->getChannel());

        // Actor is destination of the channel?
        if (p->getType() == CSDFport::In) {
            if (s.ch[c.getId()] < p->getRate().at(s.actSeqPos[a.getId()])) {
                return false;
            }
        }
        return true;
    });
}

void CSDFstateSpaceThroughputAnalysis::TransitionSystem::fireActor(State& s, const TimedCSDFactor& a) {
    startActorFiring(s, a);
    endActorFiring(s, a);
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::startActorFiring(const TimedCSDFactor& a) {
    startActorFiring(currentState, a);

#ifdef _PRINT_STATESPACE
    std::cout << "start: " << a.getName() << std::endl;
#endif
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::startActorFiring(State& s,
                                                                          const TimedCSDFactor& a) {
    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a.getConstPorts()) {
        const CSDFchannel& c = *(p->getChannel());

        // Actor is destination of the channel?
        if (p->getType() == CSDFport::In) {
            CONSUME_S(s, c.getId(), p->getRate().at(ACT_SEQ_POS_S(s, a.getId())));
        }
    }

    // Add actor firing to the list of active firings of this actor
    s.actClk[a.getId()].push_back(a.getDiscreteExecutionTime()[ACT_SEQ_POS_S(s, a.getId())]);

    // Advance the sequence position of the actor
    ACT_SEQ_POS_S(s, a.getId()) = (ACT_SEQ_POS_S(s, a.getId()) + 1) % a.sequenceLength();
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool CSDFstateSpaceThroughputAnalysis::TransitionSystem::actorReadyToEnd(const CSDFactor& a) {
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
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::endActorFiring(const CSDFactor& a) {
    endActorFiring(currentState, a);

#ifdef _PRINT_STATESPACE
    std::cout << "end:   " << a.getName() << std::endl;
#endif
}

/**
 * endActorFiring ()
 * Produce tokens on all output channels and remove the actor firing from the
 * list of active firings.
 */
void CSDFstateSpaceThroughputAnalysis::TransitionSystem::endActorFiring(State& s, const CSDFactor& a) {
    int seqPos = 0;

    for (const auto& p : a.getConstPorts()) {
        CSDFchannel *c = p->getChannel();

        // Actor is source of the channel?
        if (p->getType() == CSDFport::Out) {
            // Position in the sequence to which this end corresponds
            seqPos = static_cast<int>(ACT_SEQ_POS_S(s, a.getId()) - s.actClk[a.getId()].size());

            // Produce the tokens
            PRODUCE_S(s, c->getId(), p->getRate().at(seqPos));
        }
    }

    // Remove the firing from the list of active actor firings
    s.actClk[a.getId()].pop_front();
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
CSDFdiscreteTime CSDFstateSpaceThroughputAnalysis::TransitionSystem::clockStep() {
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
        for (auto iter = currentState.actClk[a].begin(); iter != currentState.actClk[a].end();
             iter++) {
            CSDFdiscreteTime &actFiringTime = *iter;

            // Lower remaining execution time of the actor firing
            actFiringTime -= step;
        }
    }

    // Advance the global clock
    currentState.glbClk += static_cast<uint32_t>(step);

#ifdef _PRINT_STATESPACE
    std::cout << "clk:   " << step << std::endl;
#endif

    return step;
}

/**
 * execCSDFgraph()
 * Execute the CSDF graph till a deadlock is found or a recurrent state.
 * The throughput is returned.
 */
TDtime CSDFstateSpaceThroughputAnalysis::TransitionSystem::execCSDFgraph() {
    StatesIter recurrentState;
    CSDFdiscreteTime clkStep = 0;
    int repCnt = 0;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();
    previousState.init(g->nrActors(), g->nrChannels());
    previousState.clear();

    // Initial tokens and space
    currentState.setInitialTokens(*g);

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
        for (const auto& a : g->getConstActors()) {
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
 * Compute the throughput of an CSDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
TDtime CSDFstateSpaceThroughputAnalysis::analyze(const TimedCSDFgraph& g) {
    CSDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
    CSDFgraph::RepetitionVector repVec;
    TDtime thr = 0.0;

    // Compute repetition vector
    repVec = g.getRepetitionVector();

    // Check that the graph gr is a strongly connected graph
    if (!isStronglyConnectedGraph(g)) {
        // Split graph into its strongly connected components
        // and find minimal throughput of all components
        CSDFgraphComponents comp = stronglyConnectedComponents(g);
        thr = TDTIME_MAX;

        for (auto co : comp) {
            CSDFgraph::RepetitionVector repVecGC;
            TDtime thrGc = 0.0;
            CId idFirstActor = 0;
            CId id = 0;

            // Construct graph from component
            std::unique_ptr<TimedCSDFgraph> gc = TimedCSDFgraph::dynamicCast(componentToCSDFgraph(co));

            // Id of first actor in component
            idFirstActor = (*gc->actorsBegin())->getId();

            // Relabel actors
            id = 0;
            for (const auto& u : gc->getActors()) {
                u->setId(id);
                id++;
            }

            // Relabel channels
            id = 0;
            for (const auto& c : gc->getChannels()) {
                c->setId(id);
                id++;
            }

            // Graph contains at least one channel
            if (gc->nrChannels() > 0) {
                // Compute throughput component
                thrGc = CSDF::CSDFstateSpaceThroughputAnalysis::analyze(*gc);

                // Compute repetition vector of component
                repVecGC = gc->getRepetitionVector();

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
    thr = transitionSystem.execCSDFgraph();

    return thr;
}
} // namespace CSDF
