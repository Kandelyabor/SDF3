/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF finite state machine
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: fsm.h,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_FSM_H_INCLUDED
#define FSMSADF_BASE_FSM_H_INCLUDED

#include "component.h"
#include "maxplus/base/fsm/fsm.h"
#include "scenario.h"
#include <memory>

namespace FSMSADF {

// Forward class definition
class FSM;
class FSMstate;
class FSMtransition;

// List of FSM state pointers
using FSMstates = std::list<std::unique_ptr<FSMstate>>;
using FSMstatesIter = FSMstates::iterator;
using FSMstatesCIter = FSMstates::const_iterator;
using FSMstateRef = FSMstate *;
using FSMstateRefs = std::list<FSMstateRef>;

// List of FSM transition pointers
using FSMtransitions = std::list<std::unique_ptr<FSMtransition>>;
using FSMtransitionRef = FSMtransition *;
using FSMtransitionRefs = std::list<FSMtransitionRef>;
using FSMtransitionsIter = FSMtransitions::iterator;
using FSMtransitionsCIter = FSMtransitions::const_iterator;

/**
 * FSMstate
 * State in an FSM
 */
class FSMstate : public GraphComponent {
public:
    // Constructor
    explicit FSMstate(const GraphComponent &c);

    // Copy constructor
    FSMstate(const FSMstate &other) = delete;

    // Copy assignment operator
    FSMstate &operator=(const FSMstate &other) = delete;

    // Move constructor
    FSMstate(FSMstate &&other) noexcept = delete;

    // Move assignment operator
    FSMstate &operator=(FSMstate &&other) noexcept = delete;

    // Destructor
    ~FSMstate() override;

    // Construct
    [[nodiscard]] static std::unique_ptr<FSMstate> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<FSMstate> createCopy(const GraphComponent &c) const;
    [[nodiscard]] FSMstateRef clone(const GraphComponent &c) const;

    // Scenario
    [[nodiscard]] ScenarioRef getScenario() const { return scenario; };
    void setScenario(ScenarioRef s) { scenario = s; };

    // Transitions
    void connectTransition(FSMtransition *t) { transitions.push_back(t); };

    FSMtransitionRefs::const_iterator removeTransition(FSMtransition *t);

    void clearTransitions() { transitions.clear(); };

    [[nodiscard]] const FSMtransitionRefs &getTransitions() const { return transitions; };

    bool hasTransitionTo(FSMstate *s) const;

    // FSM
    [[nodiscard]] FSM *getFSM() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, FSMstate &s) { return s.print(out); };

    // accessing
    std::unique_ptr<FSMstateRefs> getNextStatesWithScenario(const CString &sc);

private:
    // Scenario
    ScenarioRef scenario;

    // Transitions
    FSMtransitionRefs transitions;
};

/**
 * FSMtransition
 * State transition in an FSM
 */
class FSMtransition : public GraphComponent {
public:
    // Constructor
    explicit FSMtransition(const GraphComponent &c);

    // Destructor
    ~FSMtransition() override;

    // Prevent copying and moving
    FSMtransition(const FSMtransition &) = delete;
    FSMtransition &operator=(const FSMtransition &) = delete;
    FSMtransition(FSMtransition &&) = delete;
    FSMtransition &operator=(FSMtransition &&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<FSMtransition> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<FSMtransition> createCopy(const GraphComponent &c) const;
    [[nodiscard]] FSMtransitionRef clone(const GraphComponent &c) const;

    // States
    [[nodiscard]] FSMstate *getSrcState() const { return srcState; };
    [[nodiscard]] FSMstate *getDstState() const { return dstState; };
    void connect(FSMstate *src, FSMstate *dst);

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, FSMtransition &t) { return t.print(out); };

private:
    // States
    FSMstate *srcState;
    FSMstate *dstState;
};

/**
 * FSM
 * FSM in FSM-based SADF graph
 */
class FSM : public GraphComponent {
public:
    // Constructor
    explicit FSM(const GraphComponent &c);

    // Destructor
    ~FSM() override;

    // Prevent copying and moving
    FSM(const FSM&) = delete;
    FSM& operator=(const FSM&) = delete;
    FSM(FSM&&) = delete;
    FSM& operator=(FSM&&) = delete;


    // Construct
    [[nodiscard]] static std::unique_ptr<FSM> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<FSM> createCopy(const GraphComponent &c) const;
    [[nodiscard]] std::unique_ptr<FSM> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr fsmNode);

    // Convert
    void convertToXML(CNodePtr fsmNode);

    // States
    [[nodiscard]] FSMstates &getStates() { return states; };
    [[nodiscard]] FSMstate *getState(const CString &name) const;
    [[nodiscard]] const FSMstates &getConstStates() const { return states; };
    [[nodiscard]] FSMstate *getInitialState() const { return initialState; };
    void setInitialState(FSMstate *s) { initialState = s; };
    void addState(std::unique_ptr<FSMstate> s) { states.push_back(std::move(s)); };

    // Transitions
    void addTransition(std::unique_ptr<FSMtransition> t) { transitions.push_back(std::move(t)); };
    FSMtransitions &getTransitions() { return transitions; };

    // Detector
    void setDetectorName(const CString &d) { detectorName = d; };

    [[nodiscard]] CString getDetectorName() const { return detectorName; };

    // Graph
    [[nodiscard]] Graph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, FSM &f) { return f.print(out); };

    // accessing
    std::unique_ptr<FSMstateRefs> getStatesWithScenario(const CString &sc);

    // testing
    [[nodiscard]] bool acceptsScenarioSequence(const CStrings &sSeq) const;

private:
    // Initial state
    // Someday we should perhaps support multiple initial states...
    FSMstate *initialState;

    // Detector
    CString detectorName;

    // States
    FSMstates states;

    // Transitions
    FSMtransitions transitions;
};

// Edge Labelled Scenario Automaton
using ELSState = ::MaxPlus::FSM::Labeled::State<CId, CString>;
using ELSStateRef = ::MaxPlus::FSM::Labeled::StateRef<CId, CString>;
using ELSEdge = ::MaxPlus::FSM::Labeled::Edge<CId, CString>;
using ELSEdgeRef = ::MaxPlus::FSM::Labeled::EdgeRef<CId, CString>;
using ELSSetOfStates = ::MaxPlus::FSM::Labeled::SetOfStates<CId, CString>;
using ELSSetOfEdges = ::MaxPlus::FSM::Abstract::SetOfEdges;
using ELSSetOfStateRefs = ::MaxPlus::FSM::Abstract::SetOfStateRefs;
using ELSSetOfEdgeRefs = ::MaxPlus::FSM::Abstract::SetOfEdgeRefs;

class EdgeLabeledScenarioFSM : public ::MaxPlus::FSM::Labeled::FiniteStateMachine<CId, CString> {
public:
    EdgeLabeledScenarioFSM() = default;
    ~EdgeLabeledScenarioFSM() override = default;

    EdgeLabeledScenarioFSM(const EdgeLabeledScenarioFSM &) = delete;
    EdgeLabeledScenarioFSM &operator=(const EdgeLabeledScenarioFSM &other) = delete;
    EdgeLabeledScenarioFSM(EdgeLabeledScenarioFSM &&) = delete;
    EdgeLabeledScenarioFSM &operator=(EdgeLabeledScenarioFSM &&) = delete;

    std::unique_ptr<::MaxPlus::FSM::Abstract::FiniteStateMachine> newInstance() override {
        return std::make_unique<EdgeLabeledScenarioFSM>();
    }

    virtual void removeDanglingStates();
};

} // End namespace FSMSADF

#endif
