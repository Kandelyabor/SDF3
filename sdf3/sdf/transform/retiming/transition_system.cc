/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   transition_system.cc
 *
 *  Author          :   Xue-Yang Zhu
 *
 *  Date            :   2012
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

#include "transition_system.h"
#include "sdf/base/algo/repetition_vector.h"
#include <algorithm>
#include <memory>

namespace SDF {

// MG already defined (at least in VS)
// #define max(a,b) ((a)>(b) ? (a) : (b))
// #define min(a,b) ((a)<(b) ? (a) : (b))

// #define _PRINT_STATESPACE
// #define _STORE_DETAILSTATES
// #define _ZERO_EXT  //used for execution when there are actors with t(v)=0

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * print(out)
 * Print the state to the supplied stream.
 */
void TransitionSystem::State::print(std::ostream &out) {
    //   out << "### State ###" << std::endl;
    out << "(";
    for (uint32_t i : ch) {
        out << i << ",";
    }
    out << ") ";
    out << "(";
    for (auto &i : actClk) {
        out << "[";

        for (std::list<SDFdiscreteTime>::const_iterator iter = i.begin(); iter != i.end(); iter++) {
            out << " " << (*iter) << ", ";
        }

        out << "]";
    }
    out << ")";

    out << ",  glbClk = " << glbClk;
    //   out << ",  pToActions = " << pToActions ;
    out << std::endl;
}

void TransitionSystem::State::print() {
    //   out << "### State ###" << std::endl;
    std::cout << "(";
    for (uint32_t i : ch) {
        std::cout << i << ",";
    }
    std::cout << ") ";

    std::cout << "(";
    for (auto &i : actClk) {
        std::cout << "[";

        for (std::list<SDFdiscreteTime>::const_iterator iter = i.begin(); iter != i.end(); iter++) {
            std::cout << " " << (*iter) << ", ";
        }

        std::cout << "]";
    }
    std::cout << ")";

    std::cout << ",  glbClk = " << glbClk;

    std::cout << "----numOfStarts:(";
    for (int numOfStart : numOfStarts) {
        std::cout << numOfStart << ",";
    }
    std::cout << ") ";
    std::cout << std::endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void TransitionSystem::State::clear() {
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
 * The function compares to states and returns true if they are equal, except glbClk.
 */
bool TransitionSystem::State::operator==(const State &s) {
    /*   if (glbClk != s.glbClk)
           return false;
    */
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
 * Action
 *****************************************************************************/

// Output action
void TransitionSystem::Action::print(std::ostream &out) const {
    //  out << "### Action ###" << std::endl;
    if (theType == "clk") {
        out << "(" << theType << "," << fireTimes << "),";
    } else {
        out << fireTimes << "(" << theActor->getName() << "," << theType << "),";
    }
}

void TransitionSystem::Action::print() const {
    //  out << "### Action ###" << std::endl;
    if (theType == "clk") {
        std::cout << "(" << theType << "," << fireTimes << "),";
    } else {
        std::cout << fireTimes << "(" << theActor->getName() << "," << theType << "),";
    }
}

/******************************************************************************
 * Transition system
 *****************************************************************************/

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool TransitionSystem::storeState(State &s, StatesIter &pos) {
    std::ofstream tout;
    (static_cast<std::ostream &>(tout)).rdbuf(std::cout.rdbuf());
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

// Store action
void TransitionSystem::storeAction(Action &a) { storedActions.push_back(a); }

/******************************************************************************
 * SDF
 *****************************************************************************/

// NOLINTBEGIN(*macro-usage)

#define CH(c) currentState.ch[c]
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);

// NOLINTEND(*macro-usage)

uint TransitionSystem::getCurrGlbClk() const { return currentState.glbClk; }

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool TransitionSystem::actorReadyToFire(const SDFactor& a) {

    // Check all input ports for tokens
    const auto& ports = a.getPorts();
    return std::all_of(ports.begin(), ports.end(), [this](const auto& p){
        SDFchannel& c = *(p->getChannel());
        return (!(p->getType() == SDFport::SDFportType::In)) || (CH_TOKENS(c.getId(), p->getRate()));
    });
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void TransitionSystem::startActorFiring(const TimedSDFactor& a) {
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
    currentState.numOfStarts[a.getId()]++;

#ifdef _PRINT_STATESPACE
    std::cout << "start: " << a.getName() << std::endl;
#endif
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool TransitionSystem::actorReadyToEnd(const SDFactor& a) {
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
void TransitionSystem::endActorFiring(const SDFactor& a) {
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
SDFdiscreteTime TransitionSystem::clockStep() {
    SDFdiscreteTime step = UINT_MAX;

    // Find maximal time progress
    for (uint a = 0; a < g.nrActors(); a++) {
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
    for (uint a = 0; a < g.nrActors(); a++) {
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
/* The first implementation of original STE
 * store all states and actions
 *
 * */
bool TransitionSystem::execSDFgraphSelftimed() {
    SDFdiscreteTime clkStep = 0;
    Action currAction("");

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    storedActions.clear();
    auto actionIter = storedActions.begin();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }
    currentState.pToActions = actionIter;
    storeState(currentState, reCurrState);

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    int actionTimesOfA = 0;
    while (true) {

#ifdef _STORE_DETAILSTATES
        bool hasStartOrEnd = false;
#endif
        // Finish actor firings
        for (const auto& a : g.getActors()) {
            actionTimesOfA = 0;
            while (actorReadyToEnd(*a)) {

                // End the actor firing
                endActorFiring(*a);
                actionTimesOfA++;
            }
            if (actionTimesOfA > 0) {
#ifdef _STORE_DETAILSTATES
                hasStartOrEnd = true;
#endif
                currAction = Action("end", a.get(), actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }
#ifdef _STORE_DETAILSTATES
        if (hasStartOrEnd) {
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState))
                return true;
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
#endif
        // Start actor firings
#ifdef _STORE_DETAILSTATES
        hasStartOrEnd = false;
#endif
        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            actionTimesOfA = 0;

            // Ready to fire actor a?

            while (actorReadyToFire(*a)) {
                // Fire actor a

                startActorFiring(*a);
                actionTimesOfA++;
            }
            if (actionTimesOfA > 0) {
#ifdef _STORE_DETAILSTATES
                hasStartOrEnd = true;
#endif
                currAction = Action("start", a, actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }

#ifdef _STORE_DETAILSTATES

        if (hasStartOrEnd) {
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState))
                return true;
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
#endif

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
        if (clkStep != 0) {
            currAction = Action("clk", nullptr, clkStep);
            storeAction(currAction);
            actionIter++;
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState)) {
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
    }

    return true;
}

/**
 * execSDFgraph()  with the constrain actionTimesOfA<sizeOfAClk
 * when it starts the firing
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * store all states and actions
 */
bool TransitionSystem::execSDFgraph() {
    SDFdiscreteTime clkStep = 0;
    Action currAction("");

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    storedActions.clear();
    auto actionIter = storedActions.begin();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }
    currentState.pToActions = actionIter;
    storeState(currentState, reCurrState);

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    int actionTimesOfA = 0;
    while (true) {

        bool hasStartOrEnd = false;
        // Finish actor firings
        for (const auto& a : g.getActors()) {
            actionTimesOfA = 0;
            while (actorReadyToEnd(*a)) {

                // End the actor firing
                endActorFiring(*a);
                actionTimesOfA++;
            }
            if (actionTimesOfA > 0) {
                hasStartOrEnd = true;
                currAction = Action("end", a.get(), actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }
        // #ifdef _STORE_DETAILSTATES
        if (hasStartOrEnd) {
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState)) {
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
        // #endif
        //  Start actor firings
        hasStartOrEnd = false;
        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            actionTimesOfA = 0;
            int sizeOfAClk =
                    q[a->getId()] - static_cast<int>(currentState.actClk[a->getId()].size());
            // Ready to fire actor a?

            while (actionTimesOfA < sizeOfAClk && actorReadyToFire(*a)) {
                // Fire actor a

                startActorFiring(*a);
                actionTimesOfA++;
            }
            if (actionTimesOfA > 0) {
                hasStartOrEnd = true;
                currAction = Action("start", a, actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }
#ifdef _STORE_DETAILSTATES

        if (hasStartOrEnd) {
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState))
                return true;
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
#endif

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
        if (clkStep != 0) {
            currAction = Action("clk", nullptr, clkStep);
            storeAction(currAction);
            actionIter++;
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState)) {
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
    }

    return true;
}

/**
 * execSDFgraph()
 * exeType==1,execute the SDF graph till endActorFiring of the last end action
 *  of  the first iteration
 * exeType==2,continue to execute the SDF graph till a deadlock is found or
 * a recurrent state.
 * store all states and actions
 */
bool TransitionSystem::execSDFgraph(uint exeType) {
    SDFdiscreteTime clkStep = 0;
    Action currAction("");
    ActionsIter actionIter;

    RepetitionVector endTimes(q);
    uint firedSumOfQ = g.nrActors();

    if (endSTE) {
        return true;
    }

    if (exeType == 1) {
        // Clear the list of stored states and the list of stored actions
        storedStates.clear();
        storedActions.clear();
        actionIter = storedActions.begin();
        reCurrState = storedStates.begin();

        // Create initial state
        currentState.init(g.nrActors(), g.nrChannels());
        currentState.clear();

        // Initial tokens and space
        for (const auto& c : g.getChannels()) {
            CH(c->getId()) = c->getInitialTokens();
        }
        currentState.pToActions = actionIter;
        storeState(currentState, reCurrState);
    }

    actionIter = currentState.pToActions;

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    int actionTimesOfA = 0;
    while (true) {

        bool hasStartOrEnd = false;
        // Finish actor firings
        for (const auto& a : g.getActors()) {
            actionTimesOfA = 0;
            while (actorReadyToEnd(*a)) {

                // End the actor firing
                endActorFiring(*a);
                actionTimesOfA++;
                if (exeType == 1) {
                    endTimes[a->getId()]--;
                    if (endTimes[a->getId()] == 0) {
                        firedSumOfQ--;
                    }
                }
            }
            if (actionTimesOfA > 0) {
                hasStartOrEnd = true;
                currAction = Action("end", a.get(), actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }

        if (hasStartOrEnd) {
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState)) {
                endSTE = true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }

        if (exeType == 1) {
            if (firedSumOfQ == 0) {
                return true;
            }
        }

        // Start actor firings
        hasStartOrEnd = false;
        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            actionTimesOfA = 0;
            int sizeOfAClk =
                    q[a->getId()] - static_cast<int>(currentState.actClk[a->getId()].size());
            // Ready to fire actor a?

            while (actionTimesOfA < sizeOfAClk && actorReadyToFire(*a)) {
                // Fire actor a

                startActorFiring(*a);
                actionTimesOfA++;
            }
            if (actionTimesOfA > 0) {
                hasStartOrEnd = true;
                currAction = Action("start", a, actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }
#ifdef _STORE_DETAILSTATES

        if (hasStartOrEnd) {
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState)) {
                endSTE = true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
#endif

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
        if (clkStep != 0) {
            currAction = Action("clk", nullptr, clkStep);
            storeAction(currAction);
            actionIter++;
            currentState.pToActions = actionIter;
            if (!storeState(currentState, reCurrState)) {
                endSTE = true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout << "currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout << std::endl;
#endif
        }
    }

    return true;
}

/*bool TransitionSystem::execSDFgraphIP()
{
    SDFdiscreteTime clkStep;
    Action currAction("");
    ActionsIter actionIter;

    RepetitionVector endTimes(q);
    uint firedSumOfQ=g->nrActors();


    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    storedActions.clear();
    actionIter=storedActions.begin();
    reCurrState=storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (SDFchannelsIter iter = g.channelsBegin();
            iter != g.channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;

        CH(c->getId()) = c->getInitialTokens();
    }
    currentState.pToActions=actionIter;
    storeState(currentState,reCurrState);




#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    int actionTimesOfA=0;
    while (true)
    {

        bool hasStartOrEnd=false;
        // Finish actor firings
        for (SDFactorsIter iter = g.actorsBegin();
                iter != g.actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            actionTimesOfA=0;
            while (actorReadyToEnd(*a))
            {

                // End the actor firing
                endActorFiring(*a);
                actionTimesOfA++;

                 endTimes[a->getId()]--;
                 if (endTimes[a->getId()]==0) firedSumOfQ--;



            }
            if(actionTimesOfA>0){
                hasStartOrEnd=true;
                currAction=Action("end",a,actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }

        if(hasStartOrEnd){
            currentState.pToActions=actionIter;
            if(!storeState(currentState,reCurrState)){
                endSTE=true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout<<"currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout<<std::endl;
#endif
        }

       if (firedSumOfQ==0) return true;


        // Start actor firings
        hasStartOrEnd=false;
        for (SDFactorsIter iter = g.actorsBegin();
                iter != g.actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);
            actionTimesOfA=0;
            int sizeOfAClk= q[a->getId()]-(int)currentState.actClk[a->getId()].size();
            // Ready to fire actor a?

            while (actionTimesOfA<sizeOfAClk && actorReadyToFire(*a) )
            {
                // Fire actor a

                    startActorFiring(*a);
                    actionTimesOfA++;

            }
            if(actionTimesOfA>0){
                hasStartOrEnd=true;
                currAction=Action("start",a,actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }
#ifdef _STORE_DETAILSTATES

        if(hasStartOrEnd){
            currentState.pToActions=actionIter;
            if(!storeState(currentState,reCurrState)){
                endSTE=true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout<<"currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout<<std::endl;
#endif
        }
#endif

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX)
        {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
        else if(clkStep != 0){
            currAction=Action("clk",nullptr,clkStep);
            storeAction(currAction);
            actionIter++;
            currentState.pToActions=actionIter;
            if(!storeState(currentState,reCurrState)){
                endSTE=true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout<<"currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout<<std::endl;
#endif
        }


    }

    return true;
}
bool TransitionSystem::execSDFgraphFR()
{
    SDFdiscreteTime clkStep;
    Action currAction("");
    ActionsIter actionIter;



    if (endSTE)return true;

    actionIter=currentState.pToActions;

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors
    int actionTimesOfA=0;
    while (true)
    {

        bool hasStartOrEnd=false;
        // Finish actor firings
        for (SDFactorsIter iter = g.actorsBegin();
                iter != g.actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            actionTimesOfA=0;
            while (actorReadyToEnd(*a))
            {

                // End the actor firing
                endActorFiring(*a);
                actionTimesOfA++;


            }
            if(actionTimesOfA>0){
                hasStartOrEnd=true;
                currAction=Action("end",a,actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }

        if(hasStartOrEnd){
            currentState.pToActions=actionIter;
            if(!storeState(currentState,reCurrState)){
                endSTE=true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout<<"currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout<<std::endl;
#endif
        }

        // Start actor firings
        hasStartOrEnd=false;
        for (SDFactorsIter iter = g.actorsBegin();
                iter != g.actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);
            actionTimesOfA=0;
            int sizeOfAClk= q[a->getId()]-(int)currentState.actClk[a->getId()].size();
            // Ready to fire actor a?

            while (actionTimesOfA<sizeOfAClk && actorReadyToFire(*a) )
            {
                // Fire actor a

                    startActorFiring(*a);
                    actionTimesOfA++;

            }
            if(actionTimesOfA>0){
                hasStartOrEnd=true;
                currAction=Action("start",a,actionTimesOfA);
                storeAction(currAction);
                actionIter++;
            }
        }
#ifdef _STORE_DETAILSTATES

        if(hasStartOrEnd){
            currentState.pToActions=actionIter;
            if(!storeState(currentState,reCurrState)){
                endSTE=true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout<<"currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout<<std::endl;
#endif
        }
#endif

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX)
        {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
        else if(clkStep != 0){
            currAction=Action("clk",nullptr,clkStep);
            storeAction(currAction);
            actionIter++;
            currentState.pToActions=actionIter;
            if(!storeState(currentState,reCurrState)){
                endSTE=true;
                return true;
            }
#ifdef _PRINT_STATESPACE
            std::cout<<"currentState has result from the action:";
            (*currentState.pToActions).print();
            std::cout<<std::endl;
#endif
        }


    }

    return true;
}*/
/*
 * using IP-constrained STE, get a part of the execution ending with
 * the end of the actor that is the  last one finish its q(v)th firing
 */
bool TransitionSystem::execSDFgraphIP() {
    SDFdiscreteTime clkStep = 0;

    RepetitionVector endTimes(q);
    uint firedSumOfQ = g.nrActors();

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    storeState(currentState, reCurrState);

    ////    currentState.print();

    // Fire the actors

    while (true) {
        // Finish actor firings
        for (const auto& a : g.getActors()) {
            while (actorReadyToEnd(*a)) {
                // End the actor firing
                endActorFiring(*a);
                endTimes[a->getId()]--;
                if (endTimes[a->getId()] == 0) {
                    firedSumOfQ--;
                }
                if (firedSumOfQ == 0) {
                    endActor = a.get();

                    ////                     currentState.print();
                    if (!storeState(currentState, reCurrState)) {
                        endSTE = true;
                    }

                    return true;
                }
            }
        }

        // Start actor firings

        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            int sizeOfAClk =
                    q[a->getId()] - static_cast<int>(currentState.actClk[a->getId()].size());
            // Ready to fire actor a?
            int actionTimesOfA = 0;
            while (actionTimesOfA < sizeOfAClk && actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);
                actionTimesOfA++;
            }
        }

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }

    return true;
}

/*
 * using FR-constrained STE.
 *
 * following execSDFgraphIP() to finish the execution
 */
bool TransitionSystem::execSDFgraphFR() {
    SDFdiscreteTime clkStep = 0;

    if (endSTE) {
        return true;
    }

    // Fire the actors

    int firedNumOfEndActor = q[endActor->getId()];
    while (true) {

        // Finish actor firings
        for (const auto& a : g.getActors()) {
            while (actorReadyToEnd(*a)) {
                // End the actor firing
                endActorFiring(*a);
                if (a->getId() == endActor->getId()) {
                    firedNumOfEndActor--;
                }
                if (firedNumOfEndActor == 0) {
                    ////                    currentState.print();
                    if (!storeState(currentState, reCurrState)) {
                        endSTE = true;
                        return true;
                    }
                    firedNumOfEndActor = q[endActor->getId()];
                }
            }
        }

        // Start actor firings

        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            int actionTimesOfA = 0;
            int sizeOfAClk =
                    q[a->getId()] - static_cast<int>(currentState.actClk[a->getId()].size());
            // Ready to fire actor a?

            while (actionTimesOfA < sizeOfAClk && actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);
                actionTimesOfA++;
            }
        }

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------

/*
 * compute the IP using original STE. output a part of the execution
 * store only states after the each q(v)th end action of a chosen actor v.
 */
bool TransitionSystem::execSDFgraphIP_or() {
    SDFdiscreteTime clkStep = 0;

    RepetitionVector endTimes(q);
    uint firedSumOfQ = g.nrActors();

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    storeState(currentState, reCurrState);

    ////    currentState.print();

    // Fire the actors

    while (true) {
        // Finish actor firings
        for (const auto& a : g.getActors()) {
            while (actorReadyToEnd(*a)) {
                // End the actor firing
                endActorFiring(*a);
                endTimes[a->getId()]--;
                if (endTimes[a->getId()] == 0) {
                    firedSumOfQ--;
                }
                if (firedSumOfQ == 0) {
                    endActor = a.get();

                    ////                     currentState.print();
                    if (!storeState(currentState, reCurrState)) {
                        endSTE = true;
                    }

                    return true;
                }
            }
        }

        // Start actor firings

        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            // Ready to fire actor a?
            while (actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);
                //              actionTimesOfA++;
            }
        }

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }

    return true;
}

/*
 *  using original STE to get the remained execution after execSDFgraphIP_or()
 * store only states after the each q(v)th end action of a chosen actor v.
 */
bool TransitionSystem::execSDFgraphFR_or() {
    SDFdiscreteTime clkStep = 0;

    if (endSTE) {
        return true;
    }

    // Fire the actors

    int firedNumOfEndActor = q[endActor->getId()];
    while (true) {

        // Finish actor firings
        for (const auto& a : g.getActors()) {
            while (actorReadyToEnd(*a)) {
                // End the actor firing
                endActorFiring(*a);
                if (a->getId() == endActor->getId()) {
                    firedNumOfEndActor--;
                }
                if (firedNumOfEndActor == 0) {
                    ////                    currentState.print();
                    if (!storeState(currentState, reCurrState)) {
                        endSTE = true;
                        return true;
                    }
                    firedNumOfEndActor = q[endActor->getId()];
                }
            }
        }

        // Start actor firings

        for (const auto& iter : g.getActors()) {
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
            return false;
        }
    }
    return true;
}

/* output the execution of the original STE
 * storing only some chosen states
 * */
bool TransitionSystem::execSDFgraphOr() {
    SDFdiscreteTime clkStep = 0;

    //    RepetitionVector endTimes(q);
    //    uint firedSumOfQ=g->nrActors();

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    storeState(currentState, reCurrState);

    // choose the endActor
    initEndActor();
    int firedNumOfEndActor = q[endActor->getId()];

    // Fire  actors
    while (true) {

#ifdef _ZERO_EXT
        bool noEnabled = false;
        int tempCounter = 0;
        while (noEnabled == false) {
            tempCounter++;
#endif
            // Finish actor firings
            for (const auto& a : g.getActors()) {
                while (actorReadyToEnd(*a)) {
                    // End the actor firing
                    endActorFiring(*a);

                    //-----------------
                    if (a->getId() == endActor->getId()) {
                        firedNumOfEndActor--;

                        if (firedNumOfEndActor == 0) {
                            ////                   currentState.print();
                            if (!storeState(currentState, reCurrState)) {

                                return true;
                            }
                            firedNumOfEndActor = q[endActor->getId()];
                        }
                    }
                    //------------------
                }
            }

            // Start actor firings
            for (const auto& iter : g.getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                // Ready to fire actor a?
                while (actorReadyToFire(*a)) {
                    // Fire actor a
                    startActorFiring(*a);
                    //              actionTimesOfA++;
                }
            }

#ifdef _ZERO_EXT

            std::cout << tempCounter << ",,,";
            //------------
            // currentState.print();
            //------------
            noEnabled = true;
            for (const auto& iter : g.getActors()) {
                TimedSDFactor *a = (TimedSDFactor *)(*iter);
                if (actorReadyToEnd(*a))
                    noEnabled = false;
            }
        }
        std::cout << std::endl;
#endif
        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }

    return true;
}
/* output the execution of the  STE
 * with constraint actionTimesOfA<sizeOfAClk
 * storing only some chosen states
 * */
bool TransitionSystem::execSDFgraphQc() {

    SDFdiscreteTime clkStep = 0;

    //    RepetitionVector endTimes(q);
    //    uint firedSumOfQ=g->nrActors();

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    storeState(currentState, reCurrState);

    // choose the endActor
    initEndActor();
    int firedNumOfEndActor = q[endActor->getId()];

    // Fire  actors
    while (true) {
#ifdef _ZERO_EXT
        bool noEnabled = false;
        int tempCounter = 0;
        while (noEnabled == false) {
            tempCounter++;
#endif

            // Finish actor firings
            for (const auto& a : g.getActors()) {
                while (actorReadyToEnd(*a)) {
                    // End the actor firing
                    endActorFiring(*a);

                    //-----------------
                    if (a->getId() == endActor->getId()) {
                        firedNumOfEndActor--;

                        if (firedNumOfEndActor == 0) {

                            if (!storeState(currentState, reCurrState)) {

                                return true;
                            }
                            firedNumOfEndActor = q[endActor->getId()];
                        }
                    }
                    //------------------
                }
            }

            //------------
            // currentState.print();
            //------------

            // Start actor firings
            for (const auto& iter : g.getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

                int actionTimesOfA = 0;
                int sizeOfAClk =
                        q[a->getId()] - static_cast<int>(currentState.actClk[a->getId()].size());
                // Ready to fire actor a?
                while (actionTimesOfA < sizeOfAClk && actorReadyToFire(*a)) {
                    // Fire actor a
                    startActorFiring(*a);
                    actionTimesOfA++;
                }
            }

#ifdef _ZERO_EXT

            std::cout << tempCounter << ",,,";
            //------------
            // currentState.print();
            //------------
            noEnabled = true;
            for (const auto& iter : g.getActors()) {
                TimedSDFactor *a = (TimedSDFactor *)(*iter);
                if (actorReadyToEnd(*a))
                    noEnabled = false;
            }
        }
        std::cout << std::endl;
#endif
        // Clock step
        clkStep = clockStep();

        //------------
        // currentState.print();
        //------------

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }

    return true;
}

bool TransitionSystem::execSDFgraphQc(uint conLimit) {

    SDFdiscreteTime clkStep = 0;

    // Clear the list of stored states and the list of stored actions
    storedStates.clear();
    reCurrState = storedStates.begin();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

    storeState(currentState, reCurrState);

    // choose the endActor
    initEndActor();
    int firedNumOfEndActor = q[endActor->getId()];

    // Fire  actors
    while (true) {
#ifdef _ZERO_EXT
        bool noEnabled = false;
        int tempCounter = 0;
        while (noEnabled == false) {
            tempCounter++;
#endif

            // Finish actor firings
            for (const auto& a : g.getActors()) {
                while (actorReadyToEnd(*a)) {
                    // End the actor firing
                    endActorFiring(*a);

                    //-----------------
                    if (a->getId() == endActor->getId()) {
                        firedNumOfEndActor--;

                        if (firedNumOfEndActor == 0) {

                            if (!storeState(currentState, reCurrState)) {

                                return true;
                            }
                            firedNumOfEndActor = q[endActor->getId()];
                        }
                    }
                    //------------------
                }
            }

            //------------
            // currentState.print();
            //------------

            // Start actor firings
            for (const auto& iter : g.getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

                // int actionTimesOfA=0;
                // int sizeOfAClk= q[a->getId()]-(int)currentState.actClk[a->getId()].size();
                // Ready to fire actor a?
                while (currentState.actClk[a->getId()].size() < conLimit && actorReadyToFire(*a)) {
                    // Fire actor a
                    startActorFiring(*a);
                    //  actionTimesOfA++;
                }
            }

#ifdef _ZERO_EXT

            std::cout << tempCounter << ",,,";
            //------------
            // currentState.print();
            //------------
            noEnabled = true;
            for (const auto& iter : g.getActors()) {
                TimedSDFactor *a = (TimedSDFactor *)(*iter);
                if (actorReadyToEnd(*a))
                    noEnabled = false;
            }
        }
        std::cout << std::endl;
#endif
        // Clock step
        clkStep = clockStep();

        //------------
        // currentState.print();
        //------------

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }

    return true;
}

void TransitionSystem::initEndActor() {
    //---Select actor with lowest entry in repetition vector as output actor
    //    RepetitionVector repVec;
    int min = INT_MAX;
    const SDFactor* a = nullptr;

    // Select actor with lowest entry in repetition vector as output actor
    for (const auto& iter : g.getActors()) {
        if (q[iter->getId()] < min) {
            a = iter.get();
            min = q[a->getId()];
        }
    }
    //------------------------------------------------

    //--- Select actor with largest entry in repetition vector as output actor
    /* //    RepetitionVector repVec;
         int max = -1;
         SDFactor *a = nullptr;

         // Select actor with largest entry in repetition vector as output actor
         for (auto iter: g.getActors())
         {
             if (q[(*iter)->getId()] >max)
             {
                 a = *iter;
                 max = q[a->getId()];
             }
         }   */
    //-------------------------------------------------------------------

    //--- Select actor with largest entry in execution time vector as output actor
    /*//    RepetitionVector repVec;
        int max = -1;
        TimedSDFactor *a = nullptr;

        // Select actor with largest entry in execution time vector as output actor
        for (auto iter: g.getActors())
        {
            a = (TimedSDFactor*)*iter;;
            int et=a->getExecutionTime();
            if (et >max)
            {
                max = et;
            }
        }*/
    //-------------------------------------------------------------------

    // Set output actor and its repetition vector count
    endActor = a;
}

//--------------------------------------------------------------------------

/**
 * execSDFgraphOneIteration()
 * Execute(self-timed) the SDF graph one iteration.
 * storing NONE of states or actions
 */

bool TransitionSystem::execSDFgraphOneIteration() {

    SDFdiscreteTime clkStep = 0;
    //    Action currAction("");

    RepetitionVector startTimes(q);
    RepetitionVector endTimes(q);
    uint firedSumOfQ = g.nrActors();

    // Create initial state
    currentState.init(g.nrActors(), g.nrChannels());
    currentState.clear();

    // Initial tokens and space
    for (const auto& c : g.getChannels()) {
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    std::cout << "### start statespace exploration" << std::endl;
#endif

    // Fire the actors

    while (firedSumOfQ != 0) {

        // Finish actor firings
        for (const auto& a : g.getActors()) {
            //   actionTimesOfA=0;
            while (endTimes[a->getId()] > 0 && actorReadyToEnd(*a)) {

                // End the actor firing
                endActorFiring(*a);
                endTimes[a->getId()]--;
                if (endTimes[a->getId()] == 0) {
                    firedSumOfQ--;
                }
                exVector[a->getId()] = static_cast<int>(currentState.glbClk);
            }
        }

        // Start actor firings
        for (const auto& iter : g.getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            // Ready to fire actor a?
            while (startTimes[a->getId()] != 0 && actorReadyToFire(*a)) {
                // Fire actor a
                startActorFiring(*a);
                startTimes[a->getId()]--;
            }
        }

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX) {
#ifdef _PRINT_STATESPACE
            std::cout << "### end statespace exploration" << std::endl;
#endif
            return false;
        }
    }
    return true;
}

/*countStartNumInTransPhaseNew()
 *
 * the function find the number of start action in transient phase for each actor
 * used after execSDFgraphIP() and execSDFgraphFR()
 * or after execSDFgraphIP_or() and execSDFgraphFR_or()
 * or after execSDFgraphOr()
 * or after execSDFgraphQc()
 * */

std::vector<int> TransitionSystem::countStartNumInTransPhaseNew() {
    // std::vector<int> numOfStart(g.nrActors(),0);

    // if(reCurrState==storedStates.begin()) return numOfStart;

    State s = *reCurrState;

    return s.numOfStarts;
}

/*countIPB()
 *
 * the function compute the iteration period bound of the SDFG
 * used  after execSDFgraphOr()
 *
 * */

double TransitionSystem::countIPB() {

    uint endID = endActor->getId();
    State s = *reCurrState;
    State s1 = currentState;

    // number of start actions of endActor in the periodical phase
    double numOfIter = static_cast<double>(s1.numOfStarts[endID] - s.numOfStarts[endID]) / q[endID];

    // number of start actions of endActor in the periodical phase
    double numOfClk = static_cast<double>(s1.glbClk) - s.glbClk;
    //  std::cout<<"q[endID]"<<q[endID]<<"||
    //  numOfIter="<<numOfIter<<"||"<<"numOfClk="<<numOfClk<<std::endl;
    //  std::cout<<numOfIter<<","<<numOfClk<<",";
    return numOfClk / numOfIter;
}

/*countStartNumInTransPhase()
 *
 * the function find the number of start action in transient phase for each actor
 * used after execSDFgraph()
 * */

std::vector<int> TransitionSystem::countStartNumInTransPhase() {
    std::vector<int> numOfStart(g.nrActors(), 0);

    if (reCurrState == storedStates.begin()) {
        return numOfStart;
    }

    State s = *reCurrState;
    auto endActionOfTransPhase = s.pToActions;

    auto iter = storedActions.begin();
    while (iter != endActionOfTransPhase) {
        Action a = *iter;
        if (a.theType == "start") {
            const SDFactor *actor = a.theActor;
            numOfStart[actor->getId()] += static_cast<int>(a.fireTimes);
        }
        iter++;
    }
    return numOfStart;
}
/* countClkNumInTransPhase()
 * the function find the number of clk action in transient phase
 *  * used after execSDFgraph()
 * */

uint TransitionSystem::countClkNumInTransPhase() {
    uint numOfClk = 0;
    if (reCurrState == storedStates.begin()) {
        return numOfClk;
    }
    State s = *reCurrState;
    auto endActionOfTransPhase = s.pToActions;

    auto iter = storedActions.begin();
    while (iter != endActionOfTransPhase) {
        Action a = *iter;
        if (a.theType == "clk") {
            numOfClk += a.fireTimes;
        }

        iter++;
    }
    return numOfClk;
}
/* countStartNumInPeriodicPhase()
 *
 * the function find the number of start action in periodic phase for each actor
 *  * used after execSDFgraph()
 * */

std::vector<int> TransitionSystem::countStartNumInPeriodicPhase() {
    std::vector<int> numOfStart(g.nrActors(), 0);
    State s = *reCurrState;
    auto startActionOfPeriodicPhase = s.pToActions;

    if (reCurrState == storedStates.begin()) {
        startActionOfPeriodicPhase = storedActions.begin();
    } else {
        startActionOfPeriodicPhase++;
    }

    auto iter = startActionOfPeriodicPhase;
    while (iter != storedActions.end()) {
        Action a = *iter;
        if (a.theType == "start") {
            const SDFactor *actor = a.theActor;
            numOfStart[actor->getId()] += static_cast<int>(a.fireTimes);
        }

        iter++;
    }
    return numOfStart;
}
/* countClkNumInPeriodicPhase()
 * The function finds the number of clk action in periodic phase
 * * used after execSDFgraph()
 * */

uint TransitionSystem::countClkNumInPeriodicPhase() {
    uint numOfClk = 0;
    State s = *reCurrState;
    auto startActionOfPeriodicPhase = s.pToActions;

    if (reCurrState == storedStates.begin()) {
        startActionOfPeriodicPhase = storedActions.begin();
    } else {
        startActionOfPeriodicPhase++;
    }

    auto iter = startActionOfPeriodicPhase;
    while (iter != storedActions.end()) {
        Action a = *iter;
        if (a.theType == "clk") {
            numOfClk += a.fireTimes;
        }

        iter++;
    }
    return numOfClk;
}

/* printExSpace()
 * after execute graph, print execution space
 *  * used after execSDFgraph()
 *  OR used after execSDFgraphIP() and execSDFgraphFR()
 * */
void TransitionSystem::printExSpace() {
    std::cout << "### Print Execution Space ###" << std::endl;

    uint i = 0;
    uint reCurrStateNum = 0;
    auto preAIter = storedActions.begin();
    auto currAIter = storedActions.begin();
    auto sIter = storedStates.begin();
    State s = *sIter;
    std::cout << i << "--";
    s.print();
    sIter++;
    i++;
    while (sIter != storedStates.end()) {
        s = *sIter;

        if (sIter == reCurrState) {
            reCurrStateNum = i;
        }
        currAIter = s.pToActions;
        currAIter++;
        std::cout << i << "--";
        while (preAIter != currAIter) {
            Action a = *preAIter;
            a.print();
            preAIter++;
        }
        std::cout << " ==> ";
        s.print();

        sIter++;
        i++;
    }
    while (preAIter != storedActions.end()) {
        Action a = *preAIter;
        a.print();
        preAIter++;
    }
    std::cout << " ==> Loop";
    std::cout << std::endl;
    std::cout << "The first state of the periodic phase is: " << reCurrStateNum << "--";
    (*reCurrState).print();
    std::cout << "### End of Execution Space ###" << std::endl;

    std::vector<int> tempV = countStartNumInTransPhase();
    std::cout << std::endl;
    std::cout << "start action number in Transient Phase:(";
    for (int i : tempV) {
        std::cout << i << ",";
    }
    std::cout << ")" << std::endl;

    uint clkTrans = countClkNumInTransPhase();
    std::cout << "clk number in Transient Phase=" << clkTrans << std::endl;

    std::vector<int> tempV2 = countStartNumInPeriodicPhase();
    std::cout << "start action number in Periodic Phase:(";
    for (int i : tempV2) {
        std::cout << i << ",";
    }
    std::cout << ")" << std::endl;

    uint clkPeriodic = countClkNumInPeriodicPhase();

    std::cout << "clk number in Periodic Phase=" << clkPeriodic << std::endl;
}

} // namespace SDF
