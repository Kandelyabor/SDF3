/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed SDF actor
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.2 2008-05-15 09:18:17 sander Exp $
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
#include "base/exception/exception.h"
#include <limits> // IWYU pragma: keep
#include <memory>

namespace SDF {

/**
 * TimedSDFactor ()
 * Constructor.
 */
TimedSDFactor::TimedSDFactor(SDFcomponent &c) : SDFactor(c) {}

/**
 * ~TimedSDFactor ()
 * Destructor.
 */
TimedSDFactor::~TimedSDFactor() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF actor object.
 */
std::unique_ptr<SDFactor> TimedSDFactor::create(SDFcomponent &c) const {
    return std::make_unique<TimedSDFactor>(c);
}

/**
 * dynamicCast ()
 * Dynamic cast of untimed base type to timed derived type.
 */
std::unique_ptr<TimedSDFactor> TimedSDFactor::dynamicCast(std::unique_ptr<SDFactor> a) {
    return std::unique_ptr<TimedSDFactor>(dynamic_cast<TimedSDFactor*>(a.release()));
}


/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * The properties of this actor are also assigned to the new actor. The
 * ports are also cloned.
 */
std::unique_ptr<SDFactor> TimedSDFactor::clone(SDFcomponent &c) const {
    std::unique_ptr<TimedSDFactor> a = TimedSDFactor::dynamicCast(createCopy(c));

    // Ports
    for (const auto& p : this->getPorts()) {
        SDFcomponent component = SDFcomponent(a.get(), a->nrPorts());
        std::unique_ptr<SDFport> pA = p->clone(component);
        a->addPort(std::move(pA));
    }

    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * The properties of this actor are also assigned to the new actor.
 */
std::unique_ptr<SDFactor> TimedSDFactor::createCopy(SDFcomponent &c) const {
    auto a = std::make_unique<TimedSDFactor>(c);

    // Properties
    a->setName(getName());
    a->setType(getType());
    a->setDefaultProcessor(getDefaultProcessor());

    // Processors
    for (const auto& iter : this->getProcessors()) {
        a->addProcessor(iter.get());
    }

    return a;
}

/**
 * setProperties ()
 * Set the properties of a timed actor.
 */
void TimedSDFactor::setProperties(const CNodePtr propertiesNode) {
    TimedSDFactor::Processor *p = nullptr;

    // Processor definitions
    for (CNode *procNode = CGetChildNode(propertiesNode, "processor"); procNode != nullptr;
         procNode = CNextNode(procNode, "processor")) {
        if (!CHasAttribute(procNode, "type")) {
            throw CException("Processor must have a type");
        }

        // Add a processor to the actor
        p = addProcessor(CGetAttribute(procNode, "type"));

        // Execution time
        if (CHasChildNode(procNode, "executionTime")) {
            CNode *execTimeNode = CGetChildNode(procNode, "executionTime");

            if (!CHasAttribute(execTimeNode, "time")) {
                throw CException("Execution time not specified");
            }

            CString s_execTime = CGetAttribute(execTimeNode, "time");
            if (!s_execTime.isDouble()) {
                throw CException("Execution time must be non-negative.");
            }
            if (!s_execTime.isNNInteger()) {
                this->isDiscreteTime = false;
            }
            p->execTime = static_cast<SDFtime>(s_execTime);
        }

        // Memory
        if (CHasChildNode(procNode, "memory")) {
            CNode *memoryNode = nullptr;
            memoryNode = CGetChildNode(procNode, "memory");

            if (CHasChildNode(memoryNode, "stateSize")) {
                CNode *stateSizeNode = nullptr;
                stateSizeNode = CGetChildNode(memoryNode, "stateSize");

                if (!CHasAttribute(stateSizeNode, "max")) {
                    throw CException("No maximum state size given.");
                }

                int sz = static_cast<int>(CGetAttribute(stateSizeNode, "max"));
                p->stateSize = sz;
            }
        }

        // Default processor?
        if (CHasAttribute(procNode, "default")) {
            setDefaultProcessor(p->type);
        }
    }
}

/**
 * getExecutionTime ()
 * The function returns the execution time of the actor on the default
 * processor.
 */
SDFtime TimedSDFactor::getExecutionTime() const {
    if (defaultProcessor.empty()) {
        return 0;
    }

    for (const auto& p : this->getProcessors()) {
        if (p->type == defaultProcessor) {
            return p->execTime;
        }
    }

    return (SDFTIME_MAX);
}

/**
 * getDiscreteExecutionTime ()
 * The function returns the execution time of the actor on the default
 * processor.
 */
SDFdiscreteTime TimedSDFactor::getDiscreteExecutionTime() const {
    if (defaultProcessor.empty()) {
        return 0;
    }

    for (const auto& p : this->getProcessors()) {
        if (p->type == defaultProcessor) {
            return static_cast<SDFdiscreteTime>(p->execTime);
        }
    }

    return (SDFDISCRETETIME_MAX);
}


/**
 * getExecutionTime ()
 * The function returns the execution time of the actor on the processor.
 */
SDFtime TimedSDFactor::getExecutionTime(const CString &proc) const {
    for (const auto& p : this->getProcessors()) {
        if (p->type == proc) {
            return p->execTime;
        }
    }

    return SDFTIME_MAX;
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the default
 * processor.
 */
CSize TimedSDFactor::getStateSize() const {
    if (defaultProcessor.empty()) {
        return CSIZE_MAX;
    }

    for (const auto& p : this->getProcessors()) {
        if (p->type == defaultProcessor) {
            return p->stateSize;
        }
    }

    return CSIZE_MAX;
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the processor.
 */
CSize TimedSDFactor::getStateSize(const CString &proc) const {
    for (const auto& p : this->getProcessors()) {
        if (p->type == proc) {
            return p->stateSize;
        }
    }

    return CSIZE_MAX;
}

/**
 * getProcessor ()
 * Get properties of actor for specified processor type
 */
TimedSDFactor::Processor *TimedSDFactor::getProcessor(const CString &proc) const {
    for (const auto& p: this->getProcessors()) {
        if (p->type == proc) {
            return p.get();
}
    }

    return nullptr;
}

/**
 * addProcessor ()
 * Add a processor type to the list of processor types the actor can be mapped
 * to. A reference to the properties container for the processor is returned.
 */
TimedSDFactor::Processor *TimedSDFactor::addProcessor(const CString &proc) {
    std::unique_ptr<Processor> p = nullptr;

    // Processor already in the list?
    if (getProcessor(proc) != nullptr) {
        throw CException("Processor already present on actor.");
    }

    // Create a new processor
    p = std::make_unique<Processor>();
    p->type = proc;
    p->execTime = SDFTIME_MAX;
    p->stateSize = CSIZE_MAX;

    Processor* processorRef = p.get();

    // Add p to list of processors supported by the actor
    processors.push_back(std::move(p));

    return processorRef;
}

/**
 * addProcessor ()
 * Add a processor type to the list of processor types the actor can be mapped
 * to. A reference to the properties container for the processor is returned.
 */
void TimedSDFactor::addProcessor(const TimedSDFactor::Processor *p) {
    std::unique_ptr<Processor> pa = nullptr;

    // Processor already in the list?
    if (getProcessor(p->type) != nullptr) {
        throw CException("Processor already present on actor.");
}

    // Create a new processor
    pa = std::make_unique<Processor>();
    pa->type = p->type;
    pa->execTime = p->execTime;
    pa->stateSize = p->stateSize;

    // Add pa to list of processors supported by the actor
    processors.push_back(std::move(pa));
}

/**
 * removeProcessor ()
 * Remove a processor type from the list of processor types the actor can be
 * mapped to.
 */
void TimedSDFactor::removeProcessor(const CString &proc) {
    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        std::unique_ptr<Processor>& p = *iter;

        if (p->type == proc) {
            processors.erase(iter);
            return;
        }
    }
}

bool TimedSDFactor::isDiscreteTimeActor() const { return this->isDiscreteTime; }


/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &TimedSDFactor::print(std::ostream &out) {
    out << "Actor (" << getName() << ")" << std::endl;
    out << "id:         " << getId() << std::endl;
    out << "type:       " << getType() << std::endl;

    for (const auto& p: this->getProcessors()) {
        out << "processor:  " << p->type;
        if (p->type == defaultProcessor) {
            out << " (default)";
}
        out << std::endl;

        if (p->execTime == SDFTIME_MAX) {
            out << "exec time:  (max)" << std::endl;
        } else {
            out << "exec time:  " << p->execTime << std::endl;
}

        if (p->stateSize == CSIZE_MAX) {
            out << "state size: (max)" << std::endl;
        } else {
            out << "state size: " << p->stateSize << std::endl;
}
    }

    out << std::endl;

    for (const auto& p: this->getPorts()) {
        p->print(out);
    }

    return out;
}
} // namespace SDF
