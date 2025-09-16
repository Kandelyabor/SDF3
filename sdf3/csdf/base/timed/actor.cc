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
 *  Function        :   Timed CSDF actor
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.2 2008-03-22 14:24:21 sander Exp $
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
#include "csdf/base/untimed/actor.h"
#include <memory>
namespace CSDF {
/**
 * TimedCSDFactor ()
 * Constructor.
 */
TimedCSDFactor::TimedCSDFactor(CSDFcomponent &c) : CSDFactor(c) {}

/**
 * ~TimedCSDFactor ()
 * Destructor.
 */
TimedCSDFactor::~TimedCSDFactor() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 */
std::unique_ptr<CSDFactor> TimedCSDFactor::create(CSDFcomponent &c) const {
    return std::make_unique<TimedCSDFactor>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * The properties of this actor are also assigned to the new actor. The
 * ports are also cloned.
 */
std::unique_ptr<CSDFactor> TimedCSDFactor::clone(CSDFcomponent &c) const {
    std::unique_ptr<CSDFactor> a = createCopy(c);

    // Ports
    for (auto iter = portsBegin(); iter != portsEnd(); iter++) {
        CSDFport& p = *(*iter);
        CSDFcomponent component = CSDFcomponent(a.get(), a->nrPorts());
        std::unique_ptr<CSDFport> pA = p.clone(component);
        a->addPort(std::move(pA));
    }

    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * The properties of this actor are also assigned to the new actor.
 */
std::unique_ptr<CSDFactor> TimedCSDFactor::createCopy(CSDFcomponent &c) const {
    auto a = std::make_unique<TimedCSDFactor>(c);

    // Properties
    a->setName(getName());
    a->setType(getType());
    a->setDefaultProcessor(getDefaultProcessor());

    // Processors
    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        a->addProcessor(*(*iter));
    }

    return a;
}

/**
 * getExecutionTime ()
 * The function returns the sequence of execution times of the actor on the
 * default processor.
 */
CSDFtimeSequence &TimedCSDFactor::getExecutionTime() const {
    if (defaultProcessor.empty()) {
        throw CException("No default processor set for actor.");
    }

    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        Processor& p = *(*iter);

        if (p.type == defaultProcessor) {
            return p.execTime;
        }
    }

    throw CException("Default processor on actor does not exist.");
}

/**
 * getDiscreteExecutionTime ()
 * The function returns the sequence of execution times of the actor on the
 * default processor.
 */
CSDFdiscreteTimeSequence TimedCSDFactor::getDiscreteExecutionTime() const {
    if (defaultProcessor.empty()) {
        throw CException("No default processor set for actor.");
    }

    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        const Processor& p = *(*iter);

        if (p.type == defaultProcessor) {
            CSDFdiscreteTimeSequence result;
            for (const auto &t : p.execTime) {
                result.push_back(static_cast<CSDFdiscreteTime>(t));
            }
            return result;
        }
    }

    throw CException("Default processor on actor does not exist.");
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the default
 * processor.
 */
CSize TimedCSDFactor::getStateSize() {
    if (defaultProcessor.empty()) {
        return CSIZE_MAX;
    }

    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        const Processor& p = *(*iter);

        if (p.type == defaultProcessor) {
            return p.stateSize;
        }
    }

    return CSIZE_MAX;
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the processor.
 */
CSize TimedCSDFactor::getStateSize(const CString &proc) {
    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        const Processor& p = *(*iter);

        if (p.type == proc) {
            return p.stateSize;
        }
    }

    return CSIZE_MAX;
}

/**
 * getProcessor ()
 * Get properties of actor for specified processor type
 */
const TimedCSDFactor::Processor *TimedCSDFactor::getProcessor(const CString &proc) {
    for (ProcessorsCIter iter = processorsBegin(); iter != processorsEnd(); iter++) {
        const Processor& p = *(*iter);

        if (p.type == proc) {
            return &p;
        }
    }

    return nullptr;
}

/**
 * addProcessor ()
 * Add a processor type to the list of processor types the actor can be mapped
 * to. A reference to the properties container for the processor is returned.
 */
TimedCSDFactor::Processor *TimedCSDFactor::addProcessor(const CString &proc) {
    
    // Processor already in the list?
    if (getProcessor(proc) != nullptr) {
        throw CException("Processor already present on actor.");
    }

    // Create a new processor
    auto p = std::make_unique<Processor>();
    p->type = proc;
    p->stateSize = CSIZE_MAX;

    auto* pr = p.get();

    // Add p to list of processors supported by the actor
    processors.push_back(std::move(p));

    return pr;
}

/**
 * addProcessor ()
 * Add a processor type to the list of processor types the actor can be mapped
 * to. A reference to the properties container for the processor is returned.
 */
void TimedCSDFactor::addProcessor(const Processor& p) {
    
    // Processor already in the list?
    if (getProcessor(p.type) != nullptr) {
        throw CException("Processor already present on actor.");
    }

    // Create a new processor
    auto pa = std::make_unique<Processor>();
    pa->type = p.type;
    pa->execTime = p.execTime;
    pa->stateSize = p.stateSize;

    // Add pa to list of processors supported by the actor
    processors.push_back(std::move(pa));
}

/**
 * removeProcessor ()
 * Remove a processor type from the list of processor types the actor can be
 * mapped to.
 */
void TimedCSDFactor::removeProcessor(const CString &proc) {
    for (auto iter = processorsBegin(); iter != processorsEnd();) {
        const Processor& p = *(*iter);

        if (p.type == proc) {
            processors.erase(iter);
            return;
        }
    }
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &TimedCSDFactor::print(std::ostream &out) const {
    out << "Actor (" << getName() << ")" << std::endl;
    out << "id:         " << getId() << std::endl;
    out << "type:       " << getType() << std::endl;

    for (auto iter = processorsBegin(); iter != processorsEnd(); iter++) {
        const Processor& p = *(*iter);

        out << "processor:  " << p.type;
        if (p.type == defaultProcessor) {
            out << " (default)";
        }
        out << std::endl;

        if (p.execTime.empty()) {
            out << "exec time:  (max)" << std::endl;
        } else {
            out << "exec time:  ";
            for (int i = 0; i < p.execTime.size(); i++) {
                if (i != 0) {
                    out << ", ";
                }
                out << p.execTime.at(i);
            }
            out << std::endl;
        }

        if (p.stateSize == CSIZE_MAX) {
            out << "state size: (max)" << std::endl;
        } else {
            out << "state size: " << p.stateSize << std::endl;
        }
    }

    out << std::endl;

    const CSDFports &ports = this->getConstPorts();
    for (const auto& p : ports) {
        p->print(out);
    }

    return out;
}
} // namespace CSDF
