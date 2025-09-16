/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF actor
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: actor.cc,v 1.1.2.7 2010-04-22 07:07:10 mgeilen Exp $
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

#include "actor.h"
#include "graph.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {
/**
 * Actor ()
 * Constructor.
 */
Actor::Actor(const GraphComponent& c) : GraphComponent(c), detectorActor(false) {
    // Do not set the default repetitions, they must be specified
    // setRepetitionsOfScenario(getDefaultScenario(), 1);
}

/**
 * ~Actor ()
 * Destructor.
 */
Actor::~Actor() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated actor object.
 */
std::unique_ptr<Actor> Actor::create(const GraphComponent &c) { return std::make_unique<Actor>(c); }
/**
 * createCopy ()
 * The function returns a pointer to a newly allocated actor object.
 * All properties of the actor are copied.
 */
std::unique_ptr<Actor> Actor::createCopy(const GraphComponent &c) const {
    std::unique_ptr<Actor> a = create(c);

    // Properties
    a->setName(getName());
    a->setType(getType());
    a->setProcessorTypes(getProcessorTypes());
    a->setDefaultProcessorType(getDefaultProcessorType());
    a->setRepetitions(getRepetitions());
    a->setDetectorActor(isDetectorActor());

    return a;
}
/**
 * clone ()
 * The function returns a pointer to a newly allocated actor object.
 * All properties and ports of the actor are cloned.
 */
std::unique_ptr<Actor> Actor::clone(const GraphComponent &c) const {
    std::unique_ptr<Actor> a = createCopy(c);

    // Ports
    for (const auto& p : ports) {
        std::unique_ptr<Port> pA = p->clone(GraphComponent(a.get(), p->getId()));
        a->addPort(std::move(pA));
    }

    return a;
}
/**
 * constructFromXML ()
 * The function initializes all basic actor properties based on the XML data.
 */
void Actor::constructFromXML(const CNodePtr actorNode) {
    // Name
    if (!CHasAttribute(actorNode, "name")) {
        throw CException("Invalid graph, missing actor name.");
    }
    setName(CGetAttribute(actorNode, "name"));

    // Type
    if (!CHasAttribute(actorNode, "type")) {
        throw CException("Invalid graph, missing actor type.");
    }
    setType(CGetAttribute(actorNode, "type"));

    // Repetitions
    if (CHasAttribute(actorNode, "repetitions")) {
        setRepetitionsOfScenario(*getDefaultScenario(),
                                 int(CGetAttribute(actorNode, "repetitions") // NOLINT(*casting)
                                ));
    }

    // Ports
    for (CNodePtr portNode = CGetChildNode(actorNode, "port"); portNode != nullptr;
         portNode = CNextNode(portNode, "port")) {
        // Construct port
        GraphComponent component = GraphComponent(this, static_cast<CId>(getPorts().size()));
        std::unique_ptr<Port> p = std::make_unique<Port>(component);
        p->constructFromXML(portNode);

        // Add port to actor
        addPort(std::move(p));
    }
}
/**
 * constructPropertiesFromXML ()
 * The function initializes all actor properties based on the XML data.
 */
void Actor::constructPropertiesFromXML(const Scenario& s, const CNodePtr actorPropertiesNode) { //NOLINT(*cognitive-complexity))
    CString processorType;

    // Repetitions
    if (CHasAttribute(actorPropertiesNode, "repetitions")) {
        this->setRepetitionsOfScenario(s,
            uint(CGetAttribute(actorPropertiesNode, "repetitions") // NOLINT(*-casting)
        ));
    }

    // Processor definitions
    for (CNode *procNode = CGetChildNode(actorPropertiesNode, "processor"); procNode != nullptr;
         procNode = CNextNode(procNode, "processor")) {
        if (!CHasAttribute(procNode, "type")) {
            throw CException("Processor must have a type");
        }

        processorType = CGetAttribute(procNode, "type");

        // Execution time
        if (CHasChildNode(procNode, "executionTime")) {
            CNode *execTimeNode = CGetChildNode(procNode, "executionTime");

            if (!CHasAttribute(execTimeNode, "time")) {
                throw CException("Execution time not specified");
            }

            CString s_execTime = CGetAttribute(execTimeNode, "time");
            if (!s_execTime.isNNInteger()) {
                throw CException("Execution time must be non-negative integer.");
            }

            setExecutionTimeOfScenario(s, processorType, static_cast<Time>(s_execTime));
        }

        // Memory
        CNode *memoryNode = nullptr;
        for (memoryNode = CGetChildNode(procNode, "memory"); memoryNode != nullptr;
             memoryNode = CNextNode(memoryNode, "memory")) {
            // Our new memory elements
            if (CHasAttribute(memoryNode, "type")) {
                CString name = CGetAttribute(memoryNode, "name");
                CString size = CGetAttribute(memoryNode, "size");
                CString type = CGetAttribute(memoryNode, "type");

                auto m = std::make_unique<Memory>(MemoryType(type), name, static_cast<uint>(size));
                addMemoryForScenario(s, processorType, std::move(m));
            }
        }

        // Default processor?
        if (CHasAttribute(procNode, "default") && CGetAttribute(procNode, "default") == "true") {
            setDefaultProcessorType(processorType);
        }

        // Implementation
        if (CHasChildNode(procNode, "implementation")) {
            CNode *implementationNode = CGetChildNode(procNode, "implementation");
            setImplementation(s, processorType, implementationNode);
        }
    }

    // Ports
    for (CNode *portNode = CGetChildNode(actorPropertiesNode, "port"); portNode != nullptr;
         portNode = CNextNode(portNode, "port")) {
        if (!CHasAttribute(portNode, "name")) {
            throw CException("No port name specified.");
        }

        if (!CHasAttribute(portNode, "rate")) {
            throw CException("No port rate specified.");
        }

        Port* p = getPort(CGetAttribute(portNode, "name"));
        p->setRateOfScenario(s, Rate(CGetAttribute(portNode, "rate"))); // NOLINT(*-casting)
    }
}
/**
 * convertToXML ()
 * The function converts all port properties to XML data.
 */
void Actor::convertToXML(const CNodePtr actorNode) {
    // Name
    CAddAttribute(actorNode, "name", getName());

    // Type
    CAddAttribute(actorNode, "type", getType());

    // Ports
    for (Ports::const_iterator iter = ports.begin(); iter != ports.end(); iter++) {
        CNode *portNode = CAddNode(actorNode, "port");
        (*iter)->convertToXML(portNode);
    }
}
/**
 * convertPropertiesToXML ()
 * The function converts all actor properties based to XML data.
 */
void Actor::convertPropertiesToXML(const Scenario& s, const CNodePtr actorPropertiesNode) {
    // Name
    CAddAttribute(actorPropertiesNode, "actor", getName());

    // Repetitions
    if (&s != this->getDefaultScenario()) {
        if (this->hasRepetitionsOfScenario(s)) {
            CAddAttribute(actorPropertiesNode, "repetitions", CString(getRepetitionsOfScenario(s)));
        }
    }

    // Processor definitions
    for (const auto &p : processorTypes) {
        // No properties specified for this scenario?
        if (p.executionTime.find(&s) == p.executionTime.end()
            && p.scenarioMemories.find(&s) == p.scenarioMemories.end()) {
            continue;
        }

        // Processor node
        CNode *procNode = CAddNode(actorPropertiesNode, "processor");

        // Type
        CAddAttribute(procNode, "type", p.type);

        // Default processor?
        if (p.type == getDefaultProcessorType()) {
            CAddAttribute(procNode, "default", "true");
        }

        // Execution Time
        if (p.executionTime.find(&s) != p.executionTime.end()) {
            CNode *execTimeNode = CAddNode(procNode, "executionTime");
            CAddAttribute(execTimeNode, "time", CString(getExecutionTimeOfScenario(s, p.type)));
        }

        // Implementation
        createImplementation(s, p.type, procNode);

        // Memory
        if (p.scenarioMemories.find(&s) != p.scenarioMemories.end()) {
            // Get list of memories
            const Memories *mems = &((p.scenarioMemories.find(&s))->second);
            for (const auto &m : *mems) {
                CNode *memoryNode = CAddNode(procNode, "memory");
                CAddAttribute(memoryNode, "name", m->getName());
                CAddAttribute(memoryNode, "size", CString(m->getSize()));
                CAddAttribute(memoryNode, "type", m->getType().asString());
            }
        }
    }

    // Ports are not part of default scenario
    {
        if (&s != this->getDefaultScenario()) {
            // Ports
            for (const auto& p : ports) {
                if (p->getRate().find(&s) != p->getRate().end()) {
                    CNode *portNode = CAddNode(actorPropertiesNode, "port");
                    CAddAttribute(portNode, "name", p->getName());
                    CAddAttribute(portNode, "rate", CString(p->getRateOfScenario(s)));
                }
            }
        }
    }
}
/**
 * isolateScenario()
 * The function removes all scenario except scenario s from the actor.
 */
void Actor::isolateScenario(const Scenario& s) {
    // Iterate over all ports
    for (const auto& p : ports) {
        p->isolateScenario(s);
    }

    // Iterate over processor types
    for (auto &procType : processorTypes) {
        // Execution time
        Time t = getExecutionTimeOfScenario(s, procType.type);
        procType.executionTime.clear();
        setExecutionTimeOfScenario(s, procType.type, t);

        // Memories
        Memories mems = getMemoriesOfScenario(s, procType.type);
        procType.scenarioMemories.clear();
        setMemoriesOfScenario(s, procType.type, mems);
    }
}
/**
 * createPort ()
 * Create a new port on the actor.
 */
Port* Actor::createPort(GraphComponent &c) {
    auto p = std::make_unique<Port>(c);
    auto* pr = p.get();
    addPort(std::move(p));

    return pr;
}
/**
 * createPort ()
 * Create a new port on the actor.
 */
Port* Actor::createPort(const Port::PortType type) {
    GraphComponent c = GraphComponent(this, static_cast<CId>(getPorts().size()));
    Port* p = createPort(c);

    p->setType(type);

    return p;
}
/**
 * isConnected ()
 * The function returns true if all ports are connected to a channel, else
 * it returns false.
 */
bool Actor::isConnected() const {
    return std::all_of(ports.begin(), ports.end(),
                       [](const std::unique_ptr<Port>& p) { return p->isConnected(); });
}

/**
 * hasPort ()
 * The function returns a true if the port with given id exists, and false if not.
 */
bool Actor::hasPort(const CId id) {
    return std::any_of(ports.begin(), ports.end(),
                       [id](const std::unique_ptr<Port>& p) { return p->getId() == id; });
}

/**
 * hasPort ()
 * The function returns a true if the port with given name exists, and false if not.
 */
bool Actor::hasPort(const CString &name) {
    return std::any_of(ports.begin(), ports.end(),
                       [&name](const std::unique_ptr<Port>& p) { return p->getName() == name; });
}

/**
 * getPort ()
 * The function returns a reference to a port with the given id.
 */
Port* Actor::getPort(const CId id) {
    for (const auto& p : ports) {
        if (p->getId() == id) {
            return p.get();
        }
    }

    throw CException("Actor '" + getName() + "' has no port with id '" + CString(id) + "'.");
}
/**
 * getPort ()
 * The function returns a reference to a port with the given name.
 */
Port* Actor::getPort(const CString &name) {
    for (const auto& p : ports) {
        if (p->getName() == name) {
            return p.get();
        }
    }

    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}
/**
 * addPort ()
 * Add a port to an actor.
 */
void Actor::addPort(std::unique_ptr<Port> p) {
    ASSERT(!this->hasPort(p->getName()),
           "Actor '" + this->getName() + "' already has a port named '" + p->getName());
    ports.push_back(std::move(p));
}
/**
 * removePort ()
 * Remove a port from an actor and destroy its memory space.
 */
Ports::iterator Actor::removePort(const CString &name) {
    for (auto iter = ports.begin(); iter != ports.end(); iter++) {
        const Port* p = iter->get();

        if (p->getName() == name) {
            iter = ports.erase(iter);
            return iter;
        }
    }

    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}
/**
 * getExecutionTimeOfScenario ()
 * The function returns the execution time of the actor in scenario s on the
 * specified processor type.
 */
Time Actor::getExecutionTimeOfScenario(const Scenario& s, const CString &processorType) const {
    std::map<const Scenario *, Time>::const_iterator scenarioIter;

    // Find the properties of the specified processor
    const ProcessorType &p = getProcessorType(processorType);

    // Find the properties of the specified scenario
    scenarioIter = p.executionTime.find(&s);

    // Scenario has no specific properties, use default scenario
    if (scenarioIter == p.executionTime.end()) {
        scenarioIter = p.executionTime.find(getDefaultScenario());
    }

    // Default scenario does not exist?
    if (scenarioIter == p.executionTime.end()) {
        throw CException("Actor '" + getName() + "' has no execution time.");
    }

    return scenarioIter->second;
}
/**
 * setExecutionTimeOfScenario ()
 * The function sets the execution time of the actor in scenario s on the
 * specified processor type.
 */
void Actor::setExecutionTimeOfScenario(const Scenario& s, const CString &processorType, const Time t) {
    processorTypes[processorType].executionTime[&s] = t;
}

/**
 * getImplementation ()
 * The function returns the implementation data
 */
CNodePtr Actor::getImplementation(const Scenario& s, const CString &processorType) const {
    std::map<const Scenario *, CNodePtr>::iterator scenarioIter;

    // Find the properties of the specified processor
    ProcessorType p = getProcessorType(processorType);

    // Find the properties of the specified scenario
    scenarioIter = p.implementation.find(&s);

    // Scenario has no specific properties, return
    if (scenarioIter == p.implementation.end()) {
        return nullptr;
    }

    return scenarioIter->second;
}

/**
 * createImplementation ()
 * The function creates the standard implementation data
 */
void Actor::createImplementation(const Scenario&  /*s*/,
                                 const CString & /*processorType*/,
                                 const CNodePtr procNode) {
    CNode *implementationNode = CAddNode(procNode, "implementation");
    CNode *functionNode = CAddNode(implementationNode, "function");
    CAddAttribute(functionNode, "symbol", this->getName() + "_func");
    CNode *argumentMappingNode = CAddNode(functionNode, "argumentMapping");

    uint outNr = 0;
    uint inNr = 0;
    for (const auto& p : getPorts()) {
        CNode *argumentNode = CAddNode(argumentMappingNode, "argument");
        if (p->getType() == Port::PortType::Out) {
            CAddAttribute(argumentNode, "number", CString(outNr));
            outNr++;
        } else {
            CAddAttribute(argumentNode, "number", CString(inNr));
            inNr++;
        }
        CAddAttribute(argumentNode, "port", p->getName());
    }
}

/**
 * setImplementation ()
 * The function stores the implementation data
 */
void Actor::setImplementation(const Scenario& s,
                              const CString &processorType,
                              const CNodePtr implementationNode) {
    processorTypes[processorType].implementation[&s] = CCopyNode(implementationNode);
}

/**
 * setRepetitions ()
 * The function sets the repetition counts of all scenarios.
 */
void Actor::setRepetitions(const std::map<const Scenario *, uint> &r) {
    // Clear existing map
    repetitions.clear();

    // Iterate over the new map
    for (auto [sc, rep] : r) {
        // Find corresponding scenario in this graph
        ScenarioRef s = this->getScenarioGraph()->getGraph()->getScenarioRef(sc->getName());
        setRepetitionsOfScenario(*s, rep);
    }
}

/**
 * getRepetitionsOfScenario()
 * The function returns the repetitions count of the actor in the given scenario. When no
 * specific count is specified for the scenario, the default scenario is used.
 */
uint Actor::getRepetitionsOfScenario(const Scenario& s) const {
    if (repetitions.find(&s) != repetitions.end()) {
        return repetitions.find(&s)->second;
    }

    if (repetitions.find(getDefaultScenario()) == repetitions.end()) {
        throw CException("Actor '" + getName() + "' has no repetition count.");
    }

    return repetitions.find(getDefaultScenario())->second;
}

/**
 * hasRepetitionsOfScenario()
 * The function checks if the repetitions count of the actor is set in the given scenario
 * or in the default scenario is used.
 */
bool Actor::hasRepetitionsOfScenario(const Scenario& s) const {
    if (repetitions.find(&s) != repetitions.end()) {
        return true;
    }

    if (repetitions.find(getDefaultScenario()) == repetitions.end()) {
        return false;
    }

    return true;
}

/**
 * setRepetitionsOfScenario()
 * The function sets the repetitions count of the actor in the given scenario.
 */
void Actor::setRepetitionsOfScenario(const Scenario& s, const uint r) { repetitions[&s] = r; }

/**
 *
 * @param s
 * @param processorType
 * @param m
 */
void Actor::addMemoryForScenario(const Scenario& s, const CString &processorType, std::shared_ptr<Memory> m) {
    auto* mr = m.get();
    processorTypes[processorType].scenarioMemories[&s].push_back(std::move(m));
};
/**
 * setProcessorTypes ()
 * The function sets the processor type objects of all scenarios.
 */
void Actor::setProcessorTypes(const ProcessorTypes &p) {
    // Clear existing map
    processorTypes.clear();

    // Iterate over the processor types
    for (const auto &t : p) {

        // Iterate over the execution times
        for (auto [sc, et] : t.executionTime) {
            // Find corresponding scenario in this graph
            ScenarioRef s = getScenarioGraph()->getGraph()->getScenarioRef(sc->getName());
            processorTypes[t.type].executionTime[s] = et;
        }

        // Iterate over the memories
        for (const auto& [sc, mem] : t.scenarioMemories) {
            // Find corresponding scenario in this graph
            ScenarioRef s = getScenarioGraph()->getGraph()->getScenarioRef(sc->getName());
            processorTypes[t.type].scenarioMemories[s] = mem;
        }

        // Iterate over the implementation
        for (auto [sc, impl] : t.implementation) {
            // Find corresponding scenario in this graph
            ScenarioRef s = getScenarioGraph()->getGraph()->getScenarioRef(sc->getName());
            processorTypes[t.type].implementation[s] = impl;
        }
    }
}
/**
 * getProcessorType()
 * The function returns a pointer to the processor type object of the specified
 * type.
 */
const Actor::ProcessorType &Actor::getProcessorType(const CString &type) const {
    for (const auto &i : processorTypes) {
        if (i.type == type) {
            return i;
        }
    }

    throw CException("Actor '" + getName()
                     + "' does not support processor"
                       " type '"
                     + type + "'.");
}
/**
 * getDefaultScenario ()
 * The function returns a pointer to the default scenario.
 */
Scenario* Actor::getDefaultScenario() const {
    return getScenarioGraph()->getGraph()->getDefaultScenario();
}
/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &Actor::print(std::ostream &out) {
    out << "Actor (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;

    for (const auto& p: ports) {
        p->print(out);
    }

    out << "default processor type: " << defaultProcessorType << std::endl;

    for (const auto& i: processorTypes) {
        out << "processor type: " << i.type << std::endl;

        for (auto [sc,et]: i.executionTime) {
            out << "exec time: " << et << " (scenario: ";
            out << sc->getName() << ")" << std::endl;
        }

        for (const auto& [sc,mem]: i.scenarioMemories) {
            for (const auto& m: mem) {
                out << "Memory: " << m->getName() << "size: " << m->getSize()
                    << " (scenario: ";
                out << sc->getName() << ")" << std::endl;
            }
        }
    }

    out << std::endl;

    return out;
}
FSMSADF::Actor::Memories Actor::getMemoriesOfScenario(const Scenario& s,
                                                      const CString &processorType) const {
    std::map<const Scenario *, Memories>::const_iterator scenarioIter;

    // Find the properties of the specified processor
    const ProcessorType &p = getProcessorType(processorType);

    // Find the properties of the specified scenario
    scenarioIter = p.scenarioMemories.find(&s);

    // Scenario has no specific properties, use default scenario
    if (scenarioIter == p.scenarioMemories.end()) {
        scenarioIter = p.scenarioMemories.find(getDefaultScenario());
    }

    // Default scenario does not exist?
    if (scenarioIter == p.scenarioMemories.end()) {
        return {};
    }

    return scenarioIter->second;
}

// Graph
ScenarioGraph *Actor::getScenarioGraph() const {
    return dynamic_cast<ScenarioGraph *>(getParent());
}


} // End namespace FSMSADF
