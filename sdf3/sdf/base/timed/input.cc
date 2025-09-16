/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input.cc
 *
 *  Author          :   Joep van Wanrooij (j.c.p.van.wanrooij@tue.nl)
 *
 *  Date            :   May 1, 2025
 *
 *  Function        :   Timed SDF input
 *
 *  History         :
 *      01-05-2025  :   Initial version.
 *
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

#include "input.h"
#include "base/exception/exception.h"

namespace SDF {

/**
 * TimedSDFinput ()
 * Constructor.
 */
TimedSDFinput::TimedSDFinput(SDFcomponent &c) : SDFinput(c) {}

/**
 * ~TimedSDFinput ()
 * Destructor.
 */
TimedSDFinput::~TimedSDFinput() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF input object.
 */
std::unique_ptr<SDFinput> TimedSDFinput::create(SDFcomponent &c) const {
    return std::make_unique<TimedSDFinput>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF input object.
 * The properties of this input are also assigned to the new input. The
 * ports are also cloned.
 */
std::unique_ptr<SDFinput> TimedSDFinput::clone(SDFcomponent &c) const {
    std::unique_ptr<TimedSDFinput> i = std::unique_ptr<TimedSDFinput>(dynamic_cast<TimedSDFinput*>(createCopy(c).release()));

    // Ports
    auto *p = this->getPort();
    i->setPort(p->clone(c));

    return std::move(i);
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF input object.
 * The properties of this input are also assigned to the new input.
 */
std::unique_ptr<SDFinput> TimedSDFinput::createCopy(SDFcomponent &c) const {
    auto i = std::make_unique<TimedSDFinput>(c);

    // Properties
    i->setName(getName());

    // Sequence
    for (auto const &es : this->getEventSequences()) {
        i->addEventSequence(std::make_unique<EventSequence>(*es));
    }

    return i;
}

/**
 * setProperties ()
 * Set the properties of a timed input.
 */
void TimedSDFinput::setProperties(const CNodePtr propertiesNode) {

    // EventSequence definitions
    for (CNode *eventSequenceNode = CGetChildNode(propertiesNode, "eventSequence"); eventSequenceNode != nullptr;
        eventSequenceNode = CNextNode(eventSequenceNode, "eventSequence")) {

        if (!CHasAttribute(eventSequenceNode, "name")) {
            throw CException("eventSequence must have a name");
        }

        auto es = std::make_unique<EventSequence>(CGetAttribute(eventSequenceNode, "name"));

        for (CNode *eventNode = CGetChildNode(eventSequenceNode, "event"); eventNode != nullptr;
        eventNode = CNextNode(eventNode, "event")) {

            if (!CHasAttribute(eventNode, "timeStamp")) {
                throw CException("Event must have a timestamp");
            }

            CString timeStamp = CGetAttribute(eventNode, "timeStamp");

            if (!timeStamp.isDouble()) {
                throw CException("Time stamp is not in correct format.");
            }
            if (!timeStamp.isNNInteger()) {
                this->isDiscreteTime = false;
            }

            es->push_back(static_cast<SDFtime>(timeStamp));
        }

        this->addEventSequence(std::move(es));
    }
}

/**
 * removeTimeStamp ()
 * Remove a specific time stamp from the input sequence of this input
 */
void TimedSDFinput::removeEventSequence(const CString &name) {
    sequences.remove_if([&](const std::unique_ptr<EventSequence>& es) {
        return es->getName() == name;
    });
}

/**
 * print ()
 * Print the sequence
 */
std::ostream &EventSequence::print(std::ostream &out) {
    out << "Sequence(" << getName() << ")" << std::endl;
    out << "[";
    for (size_t i=0; i < this->size(); ++i) {
        out << this[i];
        if (i != this->size() - 1) {
            out << ", ";
        }
    }
    out << "]" << std::endl;

    return out;
}

/**
 * print ()
 * Print the input and its sequences
 */
std::ostream &TimedSDFinput::print(std::ostream &out) {
    out << "Input (" << getName() << ")" << std::endl;
    out << "id:         " << getId() << std::endl;

    out << "input sequences: " << std::endl;
    for (const auto &es : this->getEventSequences()) {
        es->print(out);
        out << std::endl;
    }

    out << std::endl;

    auto* p = this->getPort();
    p->print(out);

    return out;
}

} // namespace SDF
