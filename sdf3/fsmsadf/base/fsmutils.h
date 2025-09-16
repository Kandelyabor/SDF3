/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   fsm_util.h
*
*  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
*
*  Date            :   July 30, 2017
*
*  Function        :   definitions required for building up an FSM
*                      from a regular expression.
*
*  History         :
*      06-05-10    :   Initial version.
*
* $Id: thrutils.h,v 1.1.2.4 2010-08-06 07:17:49 mgeilen Exp $
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


#ifndef SDF3_FSMSADF_FSM_UTILS_H_INCLUDED
#define SDF3_FSMSADF_FSM_UTILS_H_INCLUDED

#include "maxplus/base/fsm/fsm.h" // IWYU pragma: keep
#include "fsm.h"
#include <memory>

/*
A datatype that defines the interface of an FSM.
The interface is composed of exactly one starting state and non-zero number of ending states.
*/
class EdgeLabeledScenarioFSMInterface
{
public:
    FSMSADF::ELSStateRef startingState;
    std::vector <FSMSADF::ELSStateRef> endingStates;
};


/*
EdgeLabeledScenarioFSM builder class to hide the implementation details of Regex to FSM conversion.
*/
class EdgeLabeledScenarioFSMBuilder
{
public:
    //constructor
    EdgeLabeledScenarioFSMBuilder() : fsm(new ::FSMSADF::EdgeLabeledScenarioFSM())
    { }

    // deleted copy/move constructors and assignment operators
    EdgeLabeledScenarioFSMBuilder(const EdgeLabeledScenarioFSMBuilder&) = delete;
    EdgeLabeledScenarioFSMBuilder& operator=(const EdgeLabeledScenarioFSMBuilder&) = delete;
    EdgeLabeledScenarioFSMBuilder(EdgeLabeledScenarioFSMBuilder&&) = delete;
    EdgeLabeledScenarioFSMBuilder& operator=(EdgeLabeledScenarioFSMBuilder&&) = delete;

    //destructor
    ~EdgeLabeledScenarioFSMBuilder() = default;

    /*Builds an FSM for a single scenario. It creates two states and an edge
    in between, labeled by the scenario name, and adds it to the existing fsm.*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> buildFromScenario(const CString& name);

    /*Builds an FSM for a sequential composition in a regex (e.g. a.b). Connects all
    ending states of the src interface to the starting state of the snk interface by
    epsilon transitions.*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> seqComposition(const EdgeLabeledScenarioFSMInterface* src, const EdgeLabeledScenarioFSMInterface* snk);

    /*Builds an FSM for a choice in a regex (e.g. a+b). Connects the starting state
    of the src interface to the starting state of the snk interface by epsilon transitions.*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> parComposition(const EdgeLabeledScenarioFSMInterface* src, const EdgeLabeledScenarioFSMInterface* snk);

    /*Builds an FSM for an indefinite repetition in a regex (e.g. a@).
    Connects all ending states to the starting state by epsilon transitions.*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> omegaComposition(const EdgeLabeledScenarioFSMInterface* i);

    /*Builds an FSM for a Kleene iteration in a regex (e.g. a*).
    Connects all ending states to the starting state by epsilon transitions. Also connects
    the starting state to all ending states by epsilon transitions*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> starComposition(const EdgeLabeledScenarioFSMInterface* i);


    /*Builds an FSM for a sequential composition in a deterministic regex (e.g. a.b). Connects the
    ending state of the src interface to all reachable states from the outgoing edges of starting
    state of the snk interface. Then it removes the starting state of the snk interface an the
    edges connected to this state.*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> epsilonFreeSeqComposition(const EdgeLabeledScenarioFSMInterface* src, const EdgeLabeledScenarioFSMInterface* snk);

    /*Builds an FSM for an indefinite repetition in a regex (e.g. a@) without the use of epsilon transitions.*/
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> epsilonFreeOmegaComposition(const EdgeLabeledScenarioFSMInterface* i);

	::FSMSADF::EdgeLabeledScenarioFSM* getFSM()
    {
        return fsm.get();
    }

    std::unique_ptr<::FSMSADF::EdgeLabeledScenarioFSM> takeFSM()
    {
        return std::move(fsm);
    }

    void setLatestInterface(const EdgeLabeledScenarioFSMInterface* i)
    {
        latestInterface = i;
    }

private:

	std::unique_ptr<::FSMSADF::EdgeLabeledScenarioFSM> fsm;
    uint numberOfStates{0};
    const EdgeLabeledScenarioFSMInterface* latestInterface{nullptr};
};

#endif
