/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
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
 * $Id: actor.h,v 1.1.2.7 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_ACTOR_H_INCLUDED
#define FSMSADF_BASE_ACTOR_H_INCLUDED

#include <memory>
#include <utility>

#include "fsmsadf/base/memorytype.h"
#include "port.h"
#include "type.h"

namespace FSMSADF {

// Forward class definition
class Actor;

// List of actor pointers
using Actors = std::list<std::unique_ptr<Actor>>;
using ActorRef = Actor*;
using ActorRefs = std::list<ActorRef>;

/**
 * Actor
 * Actor in FSM-based SADF graph
 */
class Actor : public GraphComponent {
public:
    Actor(const Actor& other) = delete;
    Actor& operator=(const Actor& other) = delete;
    Actor(Actor&& other) noexcept = delete;
    Actor& operator=(Actor&& other) noexcept = delete;
    class Memory {
    public:
        Memory(const MemoryType &type, CString name, const Size size) :
            type(type), name(std::move(name)), size(size){};
        // type of memory
        [[nodiscard]] MemoryType getType() const { return type; };
        // Get the name
        [[nodiscard]] CString getName() const { return name; };
        // Max size
        [[nodiscard]] Size getSize() const { return size; };
        void setSize(Size s) { size = s; };

    private:
        // type of memory
        MemoryType type;
        // Name of memory
        CString name;
        // Max req. size
        Size size;
    };
    using Memories = std::list<std::shared_ptr<Actor::Memory>>;
    using MemoryRefs = std::list<Actor::Memory*>;

    class ProcessorType {
    public:
        // Constructor
        explicit ProcessorType(CString t) : type(std::move(t)){};

        ~ProcessorType() = default;

        ProcessorType(const ProcessorType&) = default;
        ProcessorType& operator=(const ProcessorType&) = delete;
        ProcessorType(ProcessorType&&) noexcept = default;
        ProcessorType& operator=(ProcessorType&&) noexcept = delete;

        // Type
        CString type;

        // Execution time
        std::map<const Scenario *, Time> executionTime;

        // Memory spaces required by the actor.
        std::map<const Scenario *, Memories> scenarioMemories;

        // All other unrecognized implementation data
        std::map<const Scenario *, CNodePtr> implementation;
    };

    class ProcessorTypes : public std::list<ProcessorType> {
    public:
        ProcessorType &operator[](const CString &type) {
            for (auto &i : *this) {
                if (i.type == type) {
                    return i;
                }
            }
            emplace_back(ProcessorType(type));
            return back();
        };
    };

    // Constructor
    explicit Actor(const GraphComponent& c);

    // Destructor
    ~Actor() override;

    // Construct
    static std::unique_ptr<Actor> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<Actor> createCopy(const GraphComponent &c) const;
    [[nodiscard]] std::unique_ptr<Actor> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr actorNode);
    void constructPropertiesFromXML(const Scenario& s, CNodePtr actorPropertiesNode);

    // Convert
    void convertToXML(CNodePtr actorNode);
    void convertPropertiesToXML(const Scenario& s, CNodePtr actorPropertiesNode);

    // Isolate scenario
    void isolateScenario(const Scenario& s);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Ports
    [[nodiscard]] const Ports &getPorts() const { return ports; };
    [[nodiscard]] const Ports &getConstPorts() const { return ports; };
    bool hasPort(CId id);
    bool hasPort(const CString &name);
    Port* getPort(CId id);
    Port* getPort(const CString &name);
    void addPort(std::unique_ptr<Port> p);
    Ports::iterator removePort(const CString &name);
    Port* createPort(GraphComponent &c);
    Port* createPort(Port::PortType type);

    // All ports connected to a channel?
    [[nodiscard]] bool isConnected() const;

    // Execution time
    [[nodiscard]] Time getExecutionTimeOfScenario(const Scenario& s, const CString &processorType) const;
    void setExecutionTimeOfScenario(const Scenario& s, const CString &processorType, Time t);

    [[nodiscard]] CNodePtr getImplementation(const Scenario& s, const CString &processorType) const;
    void createImplementation(const Scenario& s, const CString &processorType, CNodePtr procNode);
    void setImplementation(const Scenario& s, const CString &processorType, CNodePtr ImplementationNode);

    // Get memories
    [[nodiscard]] Memories getMemoriesOfScenario(const Scenario& s, const CString &processorType) const;
    void setMemoriesOfScenario(const Scenario& s, const CString &processorType, Memories m) {
        processorTypes[processorType].scenarioMemories[&s] = std::move(m);
    }
    void clearMemoriesOfScenario(const Scenario& s, const CString &processorType) {
        processorTypes[processorType].scenarioMemories[&s].clear();
    }

    void addMemoryForScenario(const Scenario& s, const CString &processorType, std::shared_ptr<Memory> m);

    // Processor type
    [[nodiscard]] const ProcessorTypes &getProcessorTypes() const { return processorTypes; };
    void setProcessorTypes(const ProcessorTypes &p);
    [[nodiscard]] const ProcessorType &getProcessorType(const CString &type) const;

    // get/set repetitions for weakly consistent scenarios
    [[nodiscard]] const std::map<const Scenario *, uint> &getRepetitions() const { return repetitions; };
    void setRepetitions(const std::map<const Scenario *, uint> &r);
    [[nodiscard]] uint getRepetitionsOfScenario(const Scenario& s) const;
    [[nodiscard]] bool hasRepetitionsOfScenario(const Scenario& s) const;
    void setRepetitionsOfScenario(const Scenario& s, uint r);

    // Default processor type
    [[nodiscard]] const CString &getDefaultProcessorType() const { return defaultProcessorType; };
    void setDefaultProcessorType(const CString &t) { defaultProcessorType = t; };

    // Graph
    [[nodiscard]] ScenarioGraph *getScenarioGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, Actor &a) { return a.print(out); };

    // Detector
    void setDetectorActor(const bool d) { detectorActor = d; }
    [[nodiscard]] bool isDetectorActor() const { return detectorActor; }

private:
    // Default scenario
    [[nodiscard]] Scenario* getDefaultScenario() const;

    // Type
    CString type;

    // Ports
    Ports ports;

    // Processor types
    ProcessorTypes processorTypes;

    // Default processor type
    CString defaultProcessorType;

    // Repetitions
    std::map<const Scenario *, uint> repetitions;

    // Detector or not
    bool detectorActor;
};

} // End namespace FSMSADF

#endif
