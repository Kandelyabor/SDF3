/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   transition_system.h
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


#ifndef SDF_OPTIMIZE_RETIMING_TRANSITIONSYSTEM_H_INCLUDED
#define SDF_OPTIMIZE_RETIMING_TRANSITIONSYSTEM_H_INCLUDED

#include "sdf/base/algo/repetition_vector.h"
#include "sdf/base/timed/graph.h"
#include "sdf/basic_types.h"

namespace SDF {

/**
 * Max Execution time  vector
 */
using MaxExTimeVector = std::vector<int>;

/***************************************************************************
 * Transition system
 **************************************************************************/
class TransitionSystem {
public:
    /***********************************************************************
     * Action
     **********************************************************************/
    class Action {
    public:
        // Constructor
        explicit Action(const CString &aType, const SDFactor *action = nullptr, const uint times = 1) :
            theActor(action),
            theType(aType),
            fireTimes(times){

            };

        // Destructor
        ~Action() = default;

        // Prevent copying and moving
        Action(const Action&) = default;
        Action& operator=(const Action&) = default;
        Action(Action&&) = default;
        Action& operator=(Action&&) = default;

        // Output action
        void print(std::ostream &out) const;
        void print() const;
        // Action information
        const SDFactor *theActor;
        CString theType;
        uint fireTimes;
    };

    // Sequence of actions in transition system
    using Actions = std::list<Action>;
    using ActionsIter = Actions::iterator;

    /***********************************************************************
     * State
     **********************************************************************/
    class State {
    public:
        // Constructor
        explicit State(const uint nrActors = 0, const uint nrChannels = 0) {
            init(nrActors, nrChannels);
        };

        // Destructor
        ~State() = default;

        // Prevent copying and moving
        State(const State&) = default;
        State& operator=(const State&) = default;
        State(State&&) = default;
        State& operator=(State&&) = default;

        // Initialize the state
        void init(const uint nrActors, const uint nrChannels) {
            actClk.resize(nrActors);
            ch.resize(nrChannels);
            numOfStarts.resize(nrActors);
            pToActions = ActionsIter();
            glbClk = 0;
        };

        // Compare states
        bool operator==(const State &s);

        // Clear state
        void clear();

        // Output state
        void print(std::ostream &out);
        void print();

        // State information
        std::vector<std::list<SDFdiscreteTime>> actClk;
        std::vector<TBufSize> ch;
        uint32_t glbClk{};
        ActionsIter pToActions;
        // count the start actions before this state
        std::vector<int> numOfStarts;
    };

    // Sequence of states in transition system
    using States = std::list<State>;
    using StatesIter = States::iterator;

    // Constructor
    explicit TransitionSystem(const TimedSDFgraph& gr) : g(gr)  {

        q = computeRepetitionVector(g);
        exVector = MaxExTimeVector(g.nrActors(), 0);
    };

    // Destructor
    ~TransitionSystem() {
        storedStates.clear();
        storedActions.clear();
        exVector.clear();
    };

    // Prevent copying and moving
    TransitionSystem(const TransitionSystem&) = delete;
    TransitionSystem& operator=(const TransitionSystem&) = delete;
    TransitionSystem(TransitionSystem&&) = delete;
    TransitionSystem& operator=(TransitionSystem&&) = delete;


    uint getCurrGlbClk() const;

    // Execute the SDFG without limitation on concurrency,
    // storing all states and actions
    bool execSDFgraphSelftimed();
    // Execute the SDFG with constraint actionTimesOfA<sizeOfAClk
    // storing all states and actions
    bool execSDFgraph();
    // Execute the SDFG with constraint actionTimesOfA<sizeOfAClk
    // exeType=1 to get the IP; exeType=2 to finish the whole execution
    // storing all states and actions
    bool execSDFgraph(uint exeType);

    // The following two methods give a whole execution
    // using FR-constrained STE
    bool execSDFgraphIP();
    bool execSDFgraphFR();
    // The following two methods give a whole execution
    // using original STE
    bool execSDFgraphIP_or();
    bool execSDFgraphFR_or();

    /* output the execution of the original STE
     * storing only some chosen states
     * */
    bool execSDFgraphOr();
    /* output the execution of the  STE
     * with constraint actionTimesOfA<sizeOfAClk
     * storing only some chosen states
     * */
    bool execSDFgraphQc();
    bool execSDFgraphQc(uint conLimit);
    void initEndActor();

    // Execute the SDFG for one iteration
    bool execSDFgraphOneIteration();

    /*countIPB()
     * the function compute the iteration period bound of the SDFG
     * used  after execSDFgraphOr()
     *
     * */

    double countIPB();

    MaxExTimeVector getExVector() { return exVector; };
    // find the number of start action/clk action in transient phase for each actor
    std::vector<int> countStartNumInTransPhaseNew();
    std::vector<int> countStartNumInTransPhase();
    uint countClkNumInTransPhase();

    // find the number of start action/clk action in periodic phase for each actor
    std::vector<int> countStartNumInPeriodicPhase();
    uint countClkNumInPeriodicPhase();

    // after execute graph, print execution space
    void printExSpace();

private:
    // Store state
    bool storeState(State &s, StatesIter &pos);
    // Store action
    void storeAction(Action &a);

    // State transitions
    bool actorReadyToFire(const SDFactor& a);
    void startActorFiring(const TimedSDFactor& a);
    bool actorReadyToEnd(const SDFactor& a);
    void endActorFiring(const SDFactor& a);
    SDFdiscreteTime clockStep();

    // SDF graph
    const TimedSDFgraph& g;

    // Current state
    State currentState;

    // List of visited states that are stored
    States storedStates;
    // List of visited actions that are stored
    Actions storedActions;

    // the first state in periodic phase
    StatesIter reCurrState;
    // repetition vector
    RepetitionVector q;
    // the longest zero delay path execution time for each actor
    MaxExTimeVector exVector;

    // indicate whether the self-timed execution has finished
    bool endSTE{};
    // Output actor
    const SDFactor *endActor{};
};

} // namespace SDF

#endif
