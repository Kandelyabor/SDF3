/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile processor.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: processor.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "processor.h"
#include "graph.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * ProcessorBinding()
 * Constructor.
 */
ProcessorBinding::ProcessorBinding(const GraphComponent &c, Processor *p) :
    GraphComponent(c), processor(p), wheelsizeUsedForOtherGraphs(0) {}

/**
 * ~ProcessorBinding()
 * Destructor.
 */
ProcessorBinding::~ProcessorBinding() = default;

/**
 * clone()
 * Create a cloned copy of this processor binding.
 */
std::unique_ptr<ProcessorBinding> ProcessorBinding::clone(const GraphComponent &c) {
    auto pb = std::make_unique<ProcessorBinding>(c, getProcessor());

    // Actor bindings
    pb->setActorBindings(getActorBindings());

    // Static-order schedules
    pb->setStaticOrderSchedules(getStaticOrderSchedules());

    // CompSOC scenario schedules
    pb->setCompsocScenarioSchedules(getCompsocScenarioSchedules());

    // TDMA wheelsize allocation
    pb->setWheelsizeAllocations(getWheelsizeAllocations());

    // TDMA wheelsize used for other graphs
    pb->setWheelsizeUsedForOtherGraphs(getWheelsizeUsedForOtherGraphs());

    return pb;
}

/**
 * constructFromXML()
 * This function creates the processor binding as specified by the
 * processorNode.
 */
void ProcessorBinding::constructFromXML(const Scenario& s, const CNodePtr processorNode) {
    // Application graph
    ScenarioGraph *sg = s.getScenarioGraph();

    // Iterate over the list of actors bound to this processor
    for (CNode *n = CGetChildNode(processorNode, "actor"); n != nullptr;
         n = CNextNode(n, "actor")) {
        // Name specified for the actor?
        if (!CHasAttribute(n, "name")) {
            throw CException("Mapped actor has no name identifier.");
        }

        // Find actor in scenario graph
        ActorRef a = sg->getActor(CGetAttribute(n, "name"));

        // Create binding of actor to processor
        addActorBinding(s, a);
    }

    // Arbitration
    if (CHasChildNode(processorNode, "arbitration")) {
        CNode *arbitrationNode = CGetChildNode(processorNode, "arbitration");

        // Wheel size
        allocateWheelsize(s, static_cast<Time>(CGetAttribute(arbitrationNode, "timeslice")));

        // Static order schedule
        if (CHasChildNode(arbitrationNode, "schedule")) {
            CNode *scheduleNode = CGetChildNode(arbitrationNode, "schedule");
            if (!CHasAttribute(scheduleNode, "name")) {
                // Create normal SDF3 schedule
                StaticOrderSchedule so;
                so.constructFromXML(sg, CGetChildNode(arbitrationNode, "schedule"));

                // Set schedule of the processor
                setStaticOrderSchedule(s, so);
            } else {
                // Create collection of CompSOC scenario schedules
                for (CNode *m = CGetChildNode(arbitrationNode, "schedule"); m != nullptr;
                     m = CNextNode(m, "schedule")) {
                    if (!CHasChildNode(m, "actor")) {
                        throw CException("Schedule '" + CGetAttribute(m, "name")
                                         + "' has no actor entries.");
                    }

                    CString scenarioName = CGetAttribute(m, "name");

                    // Collect schedule entries in a list
                    std::list<CString> schedule;
                    for (CNode *p = CGetChildNode(m, "actor"); p != nullptr;
                         p = CNextNode(p, "actor")) {
                        schedule.push_back(CGetAttribute(p, "name"));
                    }
                    addCompsocScenarioSchedule(scenarioName, schedule);
                }
            }
        }
    }
}

/**
 * constructFromXML()
 * This function sets the resource usage of the processor.
 */
void ProcessorBinding::constructResourceUsageFromXML(const CNodePtr processorNode) {
    // Arbitration
    if (CHasChildNode(processorNode, "arbitration")) {
        CNode *arbitrationNode = CGetChildNode(processorNode, "arbitration");

        // Wheel size
        setWheelsizeUsedForOtherGraphs(
                static_cast<Time>(CGetAttribute(arbitrationNode, "timeslice")));
    }
}

/**
 * convertToXML()
 * This function converts the processor binding to an XML object.
 */
void ProcessorBinding::convertToXML(const Scenario& s, const CNodePtr processorNode) {
    std::map<const Scenario *, ActorRefs>::iterator scActorIter;

    // Name
    CAddAttribute(processorNode, "name", getName());

    // Actor binding exists for this scenario?
    scActorIter = actorBindings.find(&s);
    if (scActorIter != actorBindings.end()) {
        // List of all actors bound to this processor
        for (const auto& a : scActorIter->second) {
            CNode *n = CAddNode(processorNode, "actor");
            CAddAttribute(n, "name", a->getName());
        }
    }

    // Arbitration
    CNode *arbitrationNode = CAddNode(processorNode, "arbitration");

    // Wheel size
    CAddAttribute(arbitrationNode, "timeslice", CString(getAllocatedWheelsize(s)));

    // Static order schedule
    if (hasStaticOrderSchedule(s)) {
        getStaticOrderSchedule(s).convertToXML(CAddNode(arbitrationNode, "schedule"));
    } else if (getCompsocScenarioSchedules().begin() != getCompsocScenarioSchedules().end()) {
        // Iterate over the scenarios
        for (const auto &schedules : getCompsocScenarioSchedules()) {

            // Add a schedule node with the scenario name
            CNode *schedule = CAddNode(arbitrationNode, "schedule");
            CAddAttribute(schedule, "name", schedules.first);

            // Add a node for each actor in the schedule
            std::list<CString> sos = schedules.second;
            for (auto k = sos.begin(); k != sos.end(); k++) {
                CNode *entry = CAddNode(schedule, "state");
                CAddAttribute(entry, "actor", *k);
                if (k == sos.begin()) {
                    CAddAttribute(entry, "startOfPeriodicRegime", "true");
                }
            }
        }
    }
}

/**
 * convertToXML()
 * This function converts the resource usage of this processor binding
 * to an XML object.
 */
void ProcessorBinding::convertResourceUsageToXML(const CNodePtr processorNode) {
    // Name
    CAddAttribute(processorNode, "name", getName());

    // Arbitration
    CNode *arbitrationNode = CAddNode(processorNode, "arbitration");

    // Wheel size
    CAddAttribute(arbitrationNode,
                  "timeslice",
                  CString(getWheelsizeUsedForGraph() + getWheelsizeUsedForOtherGraphs()));
}

/**
 * addActorBinding()
 * The function binds actor a in scenario s to this processor. The function
 * returns true on success. Otherwise it returns false.
 */
bool ProcessorBinding::addActorBinding(const Scenario& s, Actor *a) {
    // No binding exists yet?
    if (!hasActorBinding(s, a)) {
        actorBindings[&s].push_back(a);
    }

    return true;
}

/**
 * removeActorBinding()
 * The function removes the binding of actor a in scenario s to this
 * processor.
 */
void ProcessorBinding::removeActorBinding(const Scenario& s, const Actor *a) {
    for (auto i = actorBindings[&s].begin(); i != actorBindings[&s].end(); i++) {
        if (*i == a) {
            actorBindings[&s].erase(i);
            break;
        }
    }
}

/**
 * hasActorBinding()
 * The function returns true when a binding of actor a in scenario s to this
 * processor exists. Otherwise it returns false.
 */
bool ProcessorBinding::hasActorBinding(const Scenario& s, const Actor *a) const {
    auto scIter = actorBindings.find(&s);

    if (scIter == actorBindings.end()) {
        return false;
    }

    return std::any_of(
            scIter->second.begin(), scIter->second.end(), [a](const Actor *aa) { return aa == a; });
}

/**
 * hasActorBindingName()
 * Returns true if the given actor binding is present in the scenario
 */
bool ProcessorBinding::hasActorBindingName(const Scenario& s, const CString &name) const {
    auto scIter = actorBindings.find(&s);

    if (scIter == actorBindings.end()) {
        return false;
    }

    return std::any_of(scIter->second.begin(), scIter->second.end(), [name](const Actor *a) {
        return a->getName() == name;
    });
}

/**
 * getStaticOrderSchedule()
 * The function associates the static order schedule so with scenario s.
 */
StaticOrderSchedule &ProcessorBinding::getStaticOrderSchedule(const Scenario& s) {
    if (hasStaticOrderSchedule(s)) {
        return staticOrderSchedules.find(&s)->second;
    }

    throw CException("Scenario '" + s.getName() + "' has no schedule.");
}

/**
 * setStaticOrderSchedule()
 * The function associates the static order schedule so with scenario s.
 */
void ProcessorBinding::setStaticOrderSchedule(const Scenario& s, StaticOrderSchedule &so) {
    staticOrderSchedules[&s] = so;
}

/**
 * removeStaticOrderSchedule()
 * The function removes the static-order schedule of scenario s.
 */
void ProcessorBinding::removeStaticOrderSchedule(const Scenario& s) {
    if (hasStaticOrderSchedule(s)) {
        staticOrderSchedules.erase(&s);
}
}

/**
 * hasStaticOrderSchedule()
 * The function returns true when scenario s has a static order schedule.
 */
bool ProcessorBinding::hasStaticOrderSchedule(const Scenario& s) const {
    return staticOrderSchedules.find(&s) != staticOrderSchedules.end();
}

/**
 * hasResourcesWheelSize()
 * The function checks wether the size of time wheel which is allocated to the
 * application can be extended to time t.
 */
bool ProcessorBinding::hasResourcesWheelSize(const Scenario& s, Time t) const {
    return getAvailableWheelsize(s) + getAllocatedWheelsize(s) >= t;
}

/**
 * allocateWheelsize()
 * The function changes the TDMA time wheel allocation of the application graph
 * on this processor to time t.
 */
bool ProcessorBinding::allocateWheelsize(const Scenario& s, Time t) {
    // Sufficient resource available?
    if (hasResourcesWheelSize(s, t)) {
        wheelsize[&s] = t;
        return true;
    }

    return false;
}


/**
 * getAvailableWheelsize()
 * The function returns the wheelsize which is available on this processor in
 * scenario s.
 */
Time ProcessorBinding::getAvailableWheelsize(const Scenario& s) const {
    return (processor->getWheelsize() - getAllocatedWheelsize(s) - wheelsizeUsedForOtherGraphs);
}

/**
 * getAllocatedWheelsize()
 * The function returns the wheelsize allocated to the application graph in
 * scenario s.
 */
Time ProcessorBinding::getAllocatedWheelsize(const Scenario& s) const {
    auto i = wheelsize.find(&s);

    if (i != wheelsize.end()) {
        return i->second;
}

    return 0;
}

/**
 * getWheelsizeUsedForGraph()
 * The function returns the maximal wheel size which has been
 * allocated to the application in any of its scenarios.
 */
Time ProcessorBinding::getWheelsizeUsedForGraph() const {
    Time n = 0;
    Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

    for (const auto& s: g->getScenarios()) {
        n = MAX(n, getAllocatedWheelsize(*s));
    }

    return n;
}

TileBinding *ProcessorBinding::getTileBinding() const {
    return dynamic_cast<TileBinding *>(getParent());
}


} // End namespace FSMSADF
