/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input.h
 *
 *  Author          :   Joep van Wanrooij (j.c.p.van.wanrooij@tue.nl)
 *
 *  Date            :   January 30, 2025
 *
 *  Function        :   SDF input
 *
 *  History         :
 *      30-01-25    :   Initial version.
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
#include "graph.h"


namespace SDF {

/**
 * SDFinput ()
 * Constructor.
 */
SDFinput::SDFinput(SDFcomponent &c) : SDFcomponent(c), port(nullptr) {}

/**
 * ~SDFinput ()
 * Destructor.
 */
SDFinput::~SDFinput() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF input object.
 */
std::unique_ptr<SDFinput> SDFinput::create(SDFcomponent &c) const { return std::make_unique<SDFinput>(c); }

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF input object.
 * All properties and port of the input are cloned.
 */
std::unique_ptr<SDFinput> SDFinput::clone(SDFcomponent &c) const {
    auto i = std::make_unique<SDFinput>(c);

    // Properties
    i->setName(getName());
    i->setPort(port->clone(c));

    return i;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF input object.
 * All properties of the input are copied.
 */
std::unique_ptr<SDFinput> SDFinput::createCopy(SDFcomponent &c) const {
    auto i = std::make_unique<SDFinput>(c);

    // Properties
    i->setName(getName());

    return i;
}

/**
 * construct ()
 * The function initializes all input properties based on the XML data.
 */
void SDFinput::construct(const CNodePtr inputNode) {
    SDFinput *i = this;

    // Name
    if (!CHasAttribute(inputNode, "name")) {
        throw CException("Invalid SDF graph, missing input name.");
    }
    i->setName(CGetAttribute(inputNode, "name"));
}

/**
 * isConnected ()
 * The function returns true if port of the input is connected, else
 * it returns false.
 */
bool SDFinput::isConnected() const {
    return port->isConnected();
}

/**
 * getPort ()
 * The function returns a reference to the port.
 */
SDFport *SDFinput::getPort(const CId id) {
    if (port->getId() == id) {
        return port.get();
    }

    throw CException("Input '" + getName() + "' has no port with id '" + CString(id) + "'.");
}

/**
 * getPort ()
 * The function returns a reference to a port with the given name.
 */
SDFport *SDFinput::getPort(const CString &name) {
    if (port->getName() == name) {
        return port.get();
    }

    throw CException("Input '" + getName() + "' has no port '" + name + "'.");
}

/**
 * setPort ()
 * set the port of an input, port can only be set once.
 */
void SDFinput::setPort(std::unique_ptr<SDFport> p) {
    if (p == nullptr) {
        throw CException("Input port '" + p->getName() + "' cannot be nullptr.");
    }

    if (port != nullptr) {
        throw CException("Input port '" + port->getName() + "' already set.");
    }

    if (p->getType() != SDFport::SDFportType::Out) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "input '"
                         + getName() + "'; conflicting port type.");
    }

    port = std::move(p);
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &SDFinput::print(std::ostream &out) {
    out << "Input (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << std::endl;

    port->print(out);

    return out;
}

SDFgraph *SDFinput::getGraph() const {
    return dynamic_cast<SDFgraph *>(getParent());
}


} // namespace SDF
