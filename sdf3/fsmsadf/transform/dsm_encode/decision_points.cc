/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   decision_points.cc
 *
 *  Author          :   Morteza Damavandpeyma (m.damavandpeyma@tue.nl)
 *
 *  Date            :   March 25, 2011
 *
 *  Function        :   Find all decision points in a (SDF) graph according
 *                      to the given static periodic schedule
 *
 *  History         :
 *      25-03-11    :   Initial version.
 *      02-11-15    :   Adaptation and refactoring for use in fsm-sadf (Reinier)
 *
 * $Id: decision_points.h,v 1.1 2011-03-25 07:35:55 morteza Exp $
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

#include "decision_points.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {
/**
 * appearedUpToNow ()
 * return the number of times that actor 'a' was executed in the schedule 'sc' up to pointer 'ptr'
 */
int appearedUpToNow(StaticOrderSchedule sc, const CString &a, int ptr) {
    int cnt = 0;
    for (int i = 0; i < ptr; i++) {
        if (sc[i].actor->getName() == a) {
            cnt++;
        }
    }
    return cnt;
}

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void DecisionPoints::TransitionSystem::State::print(std::ostream &out) {
    out << "### State ###" << std::endl;

    for (uint i = 0; i < actClk.size(); i++) {
        out << "actClk[" << i << "] =";

        for (std::list<ScenarioTime>::const_iterator iter = actClk[i].begin();
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
    out << "#############" << std::endl;
}

void DecisionPoints::TransitionSystem::printDecisionPoints(std::ostream &out) {
    out << "### Decision points ###" << std::endl;

    for (const auto &actors : readyActorList) {
        for (const auto &actor : actors) {
            out << " " << actor << " ";
        }
        out << std::endl;
    }

    out << "#############" << std::endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void DecisionPoints::TransitionSystem::State::clear() {
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
bool DecisionPoints::TransitionSystem::State::operator==(const State &s) const {

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
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the function returns false.
 */
bool DecisionPoints::TransitionSystem::storeState(State &s) {
    // Find state in the list of stored states
    for (const auto &x : storedStates) {

        // State s at position iter in the list?
        if (x == s) {
            return false;
        }
    }

    // State not found, store it at the end of the list
    storedStates.push_back(s);

    return true;
}

/******************************************************************************
 * Scenario
 *****************************************************************************/

// NOLINTBEGIN(*macro-usage)

 #define CH(c) currentState.ch[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);

// NOLINTEND(*macro-usage)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool DecisionPoints::TransitionSystem::actorReadyToFire(Scenario *s, Actor *a) {
    // Check all input ports for tokens
    return std::all_of(a->getPorts().begin(), a->getPorts().end(), [this, s](const std::unique_ptr<Port>& p) {
        Channel *c = p->getChannel();

        // Actor is destination of the channel?
        if (p->getType() == Port::PortType::In) {
            return CH_TOKENS(c->getId(), p->getRateOfScenario(*s));
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
void DecisionPoints::TransitionSystem::startActorFiring(Scenario *s, Actor *a) {
    // Consume tokens from inputs and space for output tokens
    for (const auto& p : a->getPorts()) {
        Channel *c = p->getChannel();

        // Actor is destination of the channel?
        if (p->getType() == Port::PortType::In) {
            CONSUME(c->getId(), p->getRateOfScenario(*s));
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a->getId()].push_back(
            a->getExecutionTimeOfScenario(*s, a->getDefaultProcessorType()));
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool DecisionPoints::TransitionSystem::actorReadyToEnd(Actor *a) {
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
void DecisionPoints::TransitionSystem::endActorFiring(Scenario *s, Actor *a) {
    for (const auto& p : a->getPorts()) {
        Channel *c = p->getChannel();

        // Actor is source of the channel?
        if (p->getType() == Port::PortType::Out) {
            PRODUCE(c->getId(), p->getRateOfScenario(*s));
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a->getId()].pop_front();
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
ScenarioTime DecisionPoints::TransitionSystem::clockStep(ScenarioTime step) {
    // Find maximal time progress
    for (uint a = 0; a < sg->getActors().size(); a++) {
        ScenarioTime actClk = 0;

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
    for (uint a = 0; a < sg->getActors().size(); a++) {
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
 * appearedUpToNow ()
 * return number of times actor was executed in the schedule up to now
 */
int DecisionPoints::TransitionSystem::appearedUpToNow(const CString &actorName) {
    int cnt = 0;

    for (uint i = 0; i < schedulePointer; i++) {
        if (staticPeriodicSchedule[i].actor->getName() == actorName) {
            cnt++;
        }
    }
    return cnt;
}

/**
 * actorIsInSched ()
 * return number of times actor was executed in the schedule up to now
 */
bool DecisionPoints::TransitionSystem::actorIsInSched(const CString &actorName) {
    bool isInSched = false;

    for (uint i = 0; i < staticPeriodicSchedule.size(); i++) {
        if (staticPeriodicSchedule[i].actor->getName() == actorName) {
            isInSched = true;
        }
    }
    return isInSched;
}

/**
 * actorsDone ()
 *
 */
bool DecisionPoints::TransitionSystem::actorsDone() {
    // add actors of h which do not exit in g to the output graph
    return std::all_of(sg->getActors().begin(), sg->getActors().end(), [this](const std::unique_ptr<Actor>& a) {
        return !(appearedUpToNow(a->getName()) < repVec[a->getId()]);
    });
}

/**
 * execScenarioGraph()
 * Execute the scenario graph according to the given static periodic schedule
 */
void DecisionPoints::TransitionSystem::execScenarioGraph(Scenario *s) {
    int schedProgress = 2;
    int unSchedProgress = 2;

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(static_cast<uint>(sg->getActors().size()),
                      static_cast<uint>(sg->getChannels().size()));
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : sg->getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    storeState(currentState);

    while (schedProgress != 0) {
        while (unSchedProgress != 0) {
            if (fireUnScheduledActors(s)) {
                unSchedProgress--;
            } else {
                unSchedProgress = 2;
            }
        }
        unSchedProgress = 2;

        if (fireScheduledActors(s)) {
            schedProgress--;
        } else {
            schedProgress = 2;
        }
    }
}

/**
 * fireScheduledActors()
 * Try to fire scheduled actors
 */
bool DecisionPoints::TransitionSystem::fireScheduledActors(Scenario *s) {
    static ScenarioTime clkStep;

    if (finishActorFirings(scheduled, s) || (schedulePointer >= staticPeriodicSchedule.size())) {
        return true;
    }

    std::vector<CString> tempVector;
    Actor *nextActor = nullptr;

    // Start actor firings
    for (const auto& a : scheduled) {
        // Ready to fire actor a?
        if (actorReadyToFire(s, a)) {
            // Each actor (a) should execute at most 'repVec[a]' times in a periodic schedule
            if (repVec[a->getId()] > appearedUpToNow(a->getName())) {
                // Actor to execute (according to schedule) will be placed at the beginning of the
                // vector
                if (a->getName() == staticPeriodicSchedule[schedulePointer].actor->getName()) {
                    tempVector.insert(tempVector.begin(), a->getName());
                    nextActor = a;
                }
                // Others are placed at the end
                else {
                    tempVector.push_back(a->getName());
                }
            }
        }
    }
    // Fire actor a
    if ((nextActor != nullptr) && !ProcessorIsBusy()) {
        startActorFiring(s, nextActor);
        activeProcessor();
        schedulePointer++;
    }
    readyActorList.push_back(tempVector);

    // Clock step
    clkStep = clockStep();

    // Deadlocked?
    return (clkStep == UINT_MAX) || (finishActorFirings(scheduled, s));
}

/**
 * fireUnScheduledActors()
 * Try to fire unscheduled actors
 */
bool DecisionPoints::TransitionSystem::fireUnScheduledActors(Scenario *s) {
    ScenarioTime clkStep = 0;
    bool noProgress = true;

    // Finish actor firings
    for (auto& [a, n]: unScheduled) {
        if (actorReadyToEnd(a)) {
            endActorFiring(s, a);
            releaseProcessor();
            // End the actor firing and Add state to hash of visited states
            if (!storeState(currentState)) {
                return true;
            }
        }
    }

    // Start actor firings
    for (auto& [a, n]: unScheduled) {
        // Ready to fire actor a?
        if (actorReadyToFire(s, a)) {
            // Fire actor a
            if ((n < repVec[a->getId()]) && !ProcessorIsBusy()) {
                n++;
                startActorFiring(s, a);
                noProgress = false;
                activeProcessor();
            }
        }
    }
    // Clock step
    clkStep = clockStep();

    // Deadlocked or no progress?
    return (clkStep == UINT_MAX) || noProgress;
}

/**
 * finishActorFiring()
 * Finish an actor firing.
 */
bool DecisionPoints::TransitionSystem::finishActorFirings(const ActorRefs& actors, Scenario *s) {
    // Finish actor firings
    for (const auto& a: actors) { // NOLINT(*use-anyofallof)
        if (actorReadyToEnd(a)) {
            endActorFiring(s, a);
            releaseProcessor();
            // End the actor firing and Add state to hash of visited states
            if ((!storeState(currentState)) && actorsDone()) {
                return true;
            }
        }
    }
    return false;
}

/**
 * splitActors()
 * Store the scheduled and unscheduled actors as two separate lists.
 */
void DecisionPoints::TransitionSystem::splitActors() {
    for (const auto& a: sg->getActors()) {
        if (staticPeriodicSchedule.hasActor(a->getName())) {
            scheduled.push_back(a.get());
        } else {
            unScheduled.emplace_back(a.get(), 0);
        }
    }
}

/**
 * find ()
 * Find decision points in the scenario graph g when static periodic schedule sos is
 * selected to order the execution of the graph.
 * output will place in the
 */
std::vector<std::vector<CString>>
DecisionPoints::find(Scenario *s, ScenarioGraph *sg, const StaticOrderSchedule& sos) {
    // Create a transition system
    TransitionSystem transitionSystem(s, sg, sos);

    transitionSystem.execScenarioGraph(s);

    return transitionSystem.getDecisionPoints();
}

} // End namespace FSMSADF
