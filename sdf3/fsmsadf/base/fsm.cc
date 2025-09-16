/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm.cc
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
 * $Id: fsm.cc,v 1.1.2.3 2010-05-08 23:59:38 mgeilen Exp $
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

#include "fsm.h"
#include "graph.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * FSMstate()
 * Constructor.
 */
FSMstate::FSMstate(const GraphComponent &c) : GraphComponent(c), scenario(nullptr) {}

// Destructor
FSMstate::~FSMstate() = default;

/**
 * create ()
 * Create a new FSMstate object.
 */
std::unique_ptr<FSMstate> FSMstate::create(const GraphComponent &c) {
    return std::make_unique<FSMstate>(c);
}

/**
 * createCopy ()
 * Create a copy of this FSMstate component.
 */
std::unique_ptr<FSMstate> FSMstate::createCopy(const GraphComponent &c) const {
    std::unique_ptr<FSMstate> s = create(c);

    // Properties
    s->setName(getName());

    return s;
}

/**
 * clone ()
 * Create a cloned copy of this object.
 */
FSMstateRef FSMstate::clone(const GraphComponent &c) const {
    auto s = std::unique_ptr<FSMstate>(createCopy(c));

    // Get the FSM associated with the graph component c, which is the parent
    // of the new state s
    FSM *fsm = dynamic_cast<FSM *>(c.getParent());
    ASSERT(fsm != nullptr, "Parent of FSMstate should be an FSM.");

    // Insert the state into the FSM
    auto* sr = s.get();
    fsm->addState(std::move(s));

    // Scenario
    sr->setScenario((fsm->getGraph()->getScenarioRef(getScenario()->getName())));

    // Transitions
    for (auto *t : transitions) {
        // Clone transition (triggers cloning of destination state, when needed)
        auto *tc = t->clone(GraphComponent(sr, t->getId()));
    }

    return sr;
}

FSM* FSMstate::getFSM() const {
    return dynamic_cast<FSM *>(getParent());
};

/**
 * removeTransition ()
 * Removes a transition from the list of this state.
 */
FSMtransitionRefs::const_iterator FSMstate::removeTransition(FSMtransition *t) {
    for (auto i = transitions.begin(); i != transitions.end(); i++) {
        FSMtransition *it = *i;
        if (it == t) {
            return transitions.erase(i);
        }
    }
    return transitions.end();
}

/**
 * hasTransition ()
 * Returns true if such a transition exists in this state.
 */
bool FSMstate::hasTransitionTo(FSMstate *s) const {
    return std::any_of(transitions.begin(), transitions.end(), [s](FSMtransition *it) {
        return it->getDstState() == s;
    });
}

/**
 * getNextStatesWithScenario()
 * return the states that are reachable from this state and that are labelled with scenario sc
 */
std::unique_ptr<FSMstateRefs> FSMstate::getNextStatesWithScenario(const CString &sc) {
    auto res = std::make_unique<FSMstateRefs>();
    for (auto *t : this->transitions) {
        if (t->getDstState()->getScenario()->getName() == sc) {
            res->push_back(t->getDstState());
        }
    }
    return res;
}

/**
 * print()
 * Print the FSM state to the supplied output stream.
 */
std::ostream &FSMstate::print(std::ostream &out) {
    out << "state: " << getName() << std::endl;
    out << "scenario: " << scenario->getName() << std::endl;

    for (auto *i : transitions) {
        out << "destination: " << i->getDstState()->getName() << std::endl;
    }

    return out;
}

/**
 * FSMtransition ()
 * Constructor.
 */
FSMtransition::FSMtransition(const GraphComponent &c) :
    GraphComponent(c), srcState(nullptr), dstState(nullptr) {}

/**
 * ~FSMtransition ()
 * Destructor.
 */
FSMtransition::~FSMtransition() = default;

/**
 * create ()
 * Create a new FSMtransition object.
 */
std::unique_ptr<FSMtransition> FSMtransition::create(const GraphComponent &c) { return std::make_unique<FSMtransition>(c); }

/**
 * createCopy ()
 * Create a copy of this FSMtransition component.
 */
std::unique_ptr<FSMtransition> FSMtransition::createCopy(const GraphComponent &c) const {
    std::unique_ptr<FSMtransition> t = create(c);

    // Properties
    t->setName(getName());

    return t;
}

/**
 * clone ()
 * Create a cloned copy of this object.
 */
FSMtransitionRef FSMtransition::clone(const GraphComponent &c) const {
    std::unique_ptr<FSMtransition> t = createCopy(c);

    // Get the FSMstate associated with the graph component c, which is the
    // parent of the new transition t. This state is also the source of the
    // transition.
    auto *src = dynamic_cast<FSMstate *>(c.getParent());
    ASSERT(src != nullptr, "Parent of FSMtransition should be an FSMstate.");

    // Get the FSM associated with the source state.
    FSM *fsm = src->getFSM();

    // Locate the destination state in the FSM
    FSMstate *dst = nullptr;
    for (auto const& i : fsm->getStates()) {
        if (i->getName() == getDstState()->getName()) {
            dst = i.get();
            break;
        }
    }

    // No destination state exists?
    if (dst == nullptr) {
        dst = getDstState()->clone(GraphComponent(fsm, getDstState()->getId()));
    }

    // Connect the transition to the source and destination state
    t->connect(src, dst);
    auto* tr = t.get();
    fsm->addTransition(std::move(t));

    return tr;
}

/**
 * connect ()
 * Connect the FSM transition to the source state.
 */
void FSMtransition::connect(FSMstate *src, FSMstate *dst) {
    srcState = src;
    dstState = dst;
    src->connectTransition(this);
}

/**
 * print()
 * Print the FSM transition to the supplied output stream.
 */
std::ostream &FSMtransition::print(std::ostream &out) {
    out << "transition src: " << srcState->getName() << std::endl;
    out << "transition dst: " << dstState->getName() << std::endl;

    return out;
}

/**
 * FSM ()
 * Constructor
 */
FSM::FSM(const GraphComponent &c) : GraphComponent(c), initialState(nullptr) {}

/**
 * ~FSM ()
 * Destructor.
 */
FSM::~FSM() = default;

/**
 * create ()
 * Create a new FSM object.
 */
std::unique_ptr<FSM> FSM::create(const GraphComponent &c) { return std::make_unique<FSM>(c); }

/**
 * createCopy ()
 * Create a copy of this FSM component.
 */
std::unique_ptr<FSM> FSM::createCopy(const GraphComponent &c) const {
    std::unique_ptr<FSM> fsm = create(c);

    // Properties
    fsm->setName(getName());

    return fsm;
}

/**
 * clone ()
 * Create a cloned copy of this object.
 */
std::unique_ptr<FSM> FSM::clone(const GraphComponent &c) const {
    std::unique_ptr<FSM> fsm = createCopy(c);

    // Clone the initial state (this triggers cloning of all reachable states)
    FSMstateRef newInitialState = getInitialState()->clone(GraphComponent(fsm.get(), getInitialState()->getId()));
    fsm->setInitialState(newInitialState);

    // Clone the scenario detector
    fsm->setDetectorName(this->getDetectorName());

    return fsm;
}

Graph *FSM::getGraph() const {
    return dynamic_cast<Graph *>(getParent());
};

/**
 * getState()
 * The function returns a pointer to the state with the supplied name.
 */
FSMstate *FSM::getState(const CString &name) const {
    for (const auto& i : states) {
        if (i->getName() == name) {
            return i.get();
        }
    }

    throw CException("FSM does not contain a state with name '" + name + "'.");
}

/**
 * get all states labelled with a particular scenario
 */
std::unique_ptr<FSMstateRefs> FSM::getStatesWithScenario(const CString &sc) {
    auto res = std::make_unique<FSMstateRefs>();
    for (auto const& s : states) {
        if (s->getScenario()->getName() == sc) {
            res->push_back(s.get());
        }
    }
    return res;
}

/**
 * check if the FSM accepts the given sequence of scenarios
 */
bool FSM::acceptsScenarioSequence(const CStrings &sSeq) const {
    if (sSeq.empty()) {
        return true;
    }

    // check first scenario with initial states.
    if (sSeq.front() != this->getInitialState()->getScenario()->getName()) {
        return false;
    }

    auto reachableStates = std::make_unique<FSMstateRefs>();
    reachableStates->push_back(this->getInitialState());
    auto i = sSeq.begin();
    // initial state scenario has already been checked.
    i++;
    for (; i != sSeq.end(); i++) {
        auto nextReachableStates = std::make_unique<FSMstateRefs>();
        for (auto *s : *reachableStates) {
            std::unique_ptr<FSMstateRefs> nextStates = s->getNextStatesWithScenario(*i);
            for (auto *ns : *nextStates) {
                nextReachableStates->push_back(ns);
            }
        }
        reachableStates = std::move(nextReachableStates);
    }
    bool res = !reachableStates->empty();
    return res;
}

/**
 * constructFromXML ()
 * Construct the FSM from the XML node.
 */
void FSM::constructFromXML(const CNodePtr fsmNode) {
    CString name;
    ScenarioRef s = nullptr;

    // States
    for (CNode *stateNode = CGetChildNode(fsmNode, "state"); stateNode != nullptr;
         stateNode = CNextNode(stateNode, "state")) {
        // Name
        if (!CHasAttribute(stateNode, "name")) {
            throw CException("FSM state has no name.");
        }
        name = CGetAttribute(stateNode, "name");

        // Scenario
        if (!CHasAttribute(stateNode, "scenario")) {
            throw CException("FSM state has no scenario.");
        }
        s = (getGraph()->getScenarioRef(CGetAttribute(stateNode, "scenario")));

        // Create a new state
        auto state =
                std::make_unique<FSMstate>(GraphComponent(this, static_cast<CId>(getStates().size()), name));
        state->setScenario(s);
        addState(std::move(state));
    }

    // Transitions
    for (CNode *stateNode = CGetChildNode(fsmNode, "state"); stateNode != nullptr;
         stateNode = CNextNode(stateNode, "state")) {
        // Name
        if (!CHasAttribute(stateNode, "name")) {
            throw CException("FSM state has no name.");
        }
        name = CGetAttribute(stateNode, "name");

        // Get the state
        auto *state = getState(name);

        for (CNode *transNode = CGetChildNode(stateNode, "transition"); transNode != nullptr;
             transNode = CNextNode(transNode, "transition")) {
            // Destination state
            if (!CHasAttribute(transNode, "destination")) {
                throw CException("FSM transition has no destination.");
            }

            // Create a new transition
            auto transition = std::make_unique<FSMtransition>(GraphComponent(this, 0));
            transition->connect(state, getState(CGetAttribute(transNode, "destination")));
            this->addTransition(std::move(transition));
        }
    }

    // Initial state
    if (!CHasAttribute(fsmNode, "initialstate")) {
        throw CException("FSM has no initial state.");
    }
    setInitialState(getState(CGetAttribute(fsmNode, "initialstate")));

    // Detector actor
    if (CHasAttribute(fsmNode, "detector")) {
        setDetectorName(CGetAttribute(fsmNode, "detector"));
    }
}

/**
 * convertToXML ()
 * Convert the FSM to an XML node.
 */
void FSM::convertToXML(const CNodePtr fsmNode) {
    // Initial state
    CAddAttribute(fsmNode, "initialstate", getInitialState()->getName());

    // Detector

    CAddAttribute(fsmNode, "detector", getDetectorName());

    // States
    for (const auto& s : states) {
        CNode *stateNode = CAddNode(fsmNode, "state");

        // Name
        CAddAttribute(stateNode, "name", s->getName());

        // Scenario
        CAddAttribute(stateNode, "scenario", s->getScenario()->getName());

        // Transitions
        for (auto *t : s->getTransitions()) {
            CNode *transitionNode = CAddNode(stateNode, "transition");
            CAddAttribute(transitionNode, "destination", t->getDstState()->getName());
        }
    }
}

/**
 * print ()
 * Print the FSM to the supplied output stream.
 */
std::ostream &FSM::print(std::ostream &out) {
    out << "FSM" << std::endl;
    out << "initial state: " << initialState->getScenario()->getName() << std::endl;

    // States
    for (const auto& i : states) {
        i->print(out);
    }

    out << std::endl;

    return out;
}

} // End namespace FSMSADF

/*removes states of the fsm with no outgoing edges.*/
void ::FSMSADF::EdgeLabeledScenarioFSM::removeDanglingStates() {

    // temporary sets of states and edges
    ELSSetOfEdgeRefs edgesToBeRemoved;
    ELSSetOfStateRefs statesToBeRemoved;

    {
        const ELSSetOfStates &elsStates = this->getStates();
        const ELSSetOfEdges &elsEdges = this->getEdges();

        /*go through all edges and find all edges that end in
        dangling states. Also store dangling states.*/
        for (const auto &it : elsEdges) {
            auto &e = *(it.second);
            const auto &s = dynamic_cast<ELSStateRef>(e.getDestination());
            const auto &oEdges = dynamic_cast<const ELSSetOfEdgeRefs &>(s->getOutgoingEdges());
            if (oEdges.empty()) {
                edgesToBeRemoved.insert(&e);
                statesToBeRemoved.insert(s);
            }
        }
    }

    while (!edgesToBeRemoved.empty()) {

        // remove edges ending in dangling states
        // remove edges ending in dangling states from the outgoing edges of their source states
        for (const auto &e : edgesToBeRemoved) {
            const auto *const cs = e->getSource();
            const auto& s = (this->getStates().at(cs->getId()));
            s->removeOutgoingEdge(e);
            this->removeEdge(dynamic_cast<const ELSEdge &>(*e));
        }

        // remove dangling states
        for (const auto &s : statesToBeRemoved) {
            this->removeState(*dynamic_cast<ELSStateRef>(s));
        }

        // empty the temporary sets
        edgesToBeRemoved.clear();
        statesToBeRemoved.clear();

        const ELSSetOfStates &elsStates = this->getStates();
        const ELSSetOfEdges &elsEdges = this->getEdges();

        /*go through all edges and find all edges that end in
        dangling states. Also store dangling states.*/
        for (const auto &it : elsEdges) {
            const auto &e = dynamic_cast<const ELSEdge &>(*it.second);
            const auto *s = dynamic_cast<ELSStateRef>(e.getDestination());
            const auto &oEdges = (s->getOutgoingEdges());
            if (oEdges.empty()) {
                edgesToBeRemoved.insert(&e);
                statesToBeRemoved.insert(s);
            }
        }
    }
}
