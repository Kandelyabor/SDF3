/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   fsm_util.cc
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

#include"fsmutils.h"

using namespace FSMSADF;

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::buildFromScenario(const CString& name)
{
    auto i = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    ELSStateRef start = fsm->addState(numberOfStates++);
    ELSStateRef end = fsm->addState(numberOfStates++);

    i->startingState = start;
    i->endingStates.push_back(end);

    fsm->addEdge(*start, name, *end);
    fsm->setInitialState(*start);
    return i;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::seqComposition(const EdgeLabeledScenarioFSMInterface* src, const EdgeLabeledScenarioFSMInterface* snk)
{
    auto i = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    for (const auto *s: src->endingStates)
    {
        fsm->addEdge(*s, "epsilon", *(snk->startingState));
    }

    i->startingState = src->startingState;
    i->endingStates = snk->endingStates;

    fsm->setInitialState(*(src->startingState));
    return i;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::parComposition(const EdgeLabeledScenarioFSMInterface* src, const EdgeLabeledScenarioFSMInterface* snk)
{
    auto i = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    fsm->addEdge(*(src->startingState), "epsilon", *(snk->startingState));

    i->startingState = src->startingState;
    i->endingStates = src->endingStates;

    for (const auto *a: snk->endingStates)
    {
        i->endingStates.push_back(a);
    }

    fsm->setInitialState(*(src->startingState));
    return i;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::omegaComposition(const EdgeLabeledScenarioFSMInterface* i)
{
    auto in = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    in->startingState = i->startingState;
    in->endingStates = i->endingStates;

    for (const auto *s: i->endingStates)
    {
        fsm->addEdge(*s, "epsilon", *(i->startingState));
    }

    fsm->setInitialState(*(i->startingState));
    return in;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::starComposition(const EdgeLabeledScenarioFSMInterface* i)
{
    auto in = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    in->startingState = i->startingState;
    in->endingStates = i->endingStates;

    for (const auto *s: i->endingStates)
    {
        fsm->addEdge(*s, "epsilon", *(i->startingState));
        fsm->addEdge(*(i->startingState), "epsilon", *s);
    }

    fsm->setInitialState(*(i->startingState));
    return in;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::epsilonFreeSeqComposition(const EdgeLabeledScenarioFSMInterface* src, const EdgeLabeledScenarioFSMInterface* snk)
{
    auto i = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    //get the starting state of the snk interface
    ELSStateRef startingStateOfSnk = snk->startingState;

    //get the outgoing edges of starting state of the snk interface
    auto outgoingEdgesOfStartingStateOfSnk = dynamic_cast<const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs&>(startingStateOfSnk->getOutgoingEdges());

    //get the complete set of states and edges of the fsm under construction
    const ELSSetOfStates& fsmStates = fsm->getStates();
    const ELSSetOfEdges& fsmEdges = fsm->getEdges();

    //get the ending state of the src interface. Note that for deterministic
    //regex, src has only one ending state.
    ELSStateRef endingStateOfSrc = src->endingStates.front();

    //temporary container for edges that should be removed.
    std::vector<ELSEdgeRef> edgesToBeRemoved;

    //create and add the edges from the ending state of src to all reachable states from
    // the starting state of the snk
    for (const auto *edgesIter: outgoingEdgesOfStartingStateOfSnk)
    {
        const auto *e = dynamic_cast<ELSEdgeRef>(edgesIter);
        edgesToBeRemoved.push_back(e);
        fsm->addEdge(*endingStateOfSrc, e->getLabel(), *dynamic_cast<ELSStateRef>(e->getDestination()));
    }

    //remove the starting state of the snk and all its outgoing edges from the fsm
    for (const auto *e: edgesToBeRemoved)
    {
        // startingStateOfSnk->removeOutgoingEdge(*e);
        fsm->removeEdge(*e);
    }
    fsm->removeState(*startingStateOfSnk);

    i->startingState = src->startingState;
    i->endingStates = snk->endingStates;

    fsm->setInitialState(*(src->startingState));
    return i;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface> EdgeLabeledScenarioFSMBuilder::epsilonFreeOmegaComposition(const EdgeLabeledScenarioFSMInterface* i)
{
    auto in = std::make_unique<EdgeLabeledScenarioFSMInterface>();

    in->startingState = i->startingState;
    in->endingStates = i->endingStates;

    //get the starting state of the snk interface
    ELSStateRef startingStateOfSnk = i->startingState;

    //get the outgoing edges of starting state of the snk interface
    auto outgoingEdgesOfStartingStateOfSnk = dynamic_cast<const ::MaxPlus::FSM::Abstract::SetOfEdgeRefs&>(startingStateOfSnk->getOutgoingEdges());

    //get the complete set of states and edges of the fsm under construction
    const ELSSetOfStates& fsmStates = fsm->getStates();
    const ELSSetOfEdges& fsmEdges = fsm->getEdges();

    //get the ending state of the src interface. Note that for deterministic
    //regex, src has only one ending state.
    ELSStateRef endingStateOfSrc = i->endingStates.front();

    //temporary container for edges that should be removed.
    std::vector<ELSEdgeRef> edgesToBeRemoved;

    //create and add the edges from the ending state of src to all reachable states from
    // the starting state of the snk
    for (const auto *edgesIter: outgoingEdgesOfStartingStateOfSnk)
    {
        const auto* e = dynamic_cast<ELSEdgeRef>(edgesIter);
        edgesToBeRemoved.push_back(e);
        fsm->addEdge(*endingStateOfSrc, e->getLabel(), *dynamic_cast<ELSStateRef>(e->getDestination()));
    }

    //remove the starting state of the snk and all its outgoing edges from the fsm
    for (const auto *e: edgesToBeRemoved)
    {
        // startingStateOfSnk->removeOutgoingEdge(*e);
        fsm->removeEdge(*e);
    }

    fsm->removeState(*startingStateOfSnk);

    fsm->setInitialState(*(i->endingStates.front()));

    return in;
}