/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   output.h
 *
 *  Author          :   Joep van Wanrooij (j.c.p.van.wanrooij@tue.nl)
 *
 *  Date            :   January 30, 2025
 *
 *  Function        :   SDF output
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

#include "output.h"
#include "base/exception/exception.h"
#include "graph.h"

namespace SDF {

/**
 * SDFoutput ()
 * Constructor.
 */
SDFoutput::SDFoutput(SDFcomponent &c) : SDFcomponent(c), port(nullptr) {}

/**
 * ~SDFoutput ()
 * Destructor.
 */
SDFoutput::~SDFoutput() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF output object.
 */
std::unique_ptr<SDFoutput> SDFoutput::create(SDFcomponent &c) const { return std::make_unique<SDFoutput>(c); }

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF output object.
 * All properties and port of the output are cloned.
 */
std::unique_ptr<SDFoutput> SDFoutput::clone(SDFcomponent &c) const {
    auto i = std::make_unique<SDFoutput>(c);

    // Properties
    i->setName(getName());
    i->setPort(port->clone(c));

    return i;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF output object.
 * All properties of the output are copied.
 */
std::unique_ptr<SDFoutput> SDFoutput::createCopy(SDFcomponent &c) const {
    auto i = std::make_unique<SDFoutput>(c);

    // Properties
    i->setName(getName());

    return i;
}

/**
 * construct ()
 * The function initializes all output properties based on the XML data.
 */
void SDFoutput::construct(const CNodePtr outputNode) {
    SDFoutput *i = this;

    // Name
    if (!CHasAttribute(outputNode, "name")) {
        throw CException("Invalid SDF graph, missing output name.");
    }
    i->setName(CGetAttribute(outputNode, "name"));
}

/**
 * isConnected ()
 * The function returns true if port of the output is connected, else
 * it returns false.
 */
bool SDFoutput::isConnected() const {
    return port->isConnected();
}

/**
 * getPort ()
 * The function returns a reference to the port.
 */
SDFport *SDFoutput::getPort(const CId id) {
    if (port->getId() == id) {
        return port.get();
    }

    throw CException("Output '" + getName() + "' has no port with id '" + CString(id) + "'.");
}

/**
 * getPort ()
 * The function returns a reference to a port with the given name.
 */
SDFport *SDFoutput::getPort(const CString &name) {
    if (port->getName() == name) {
        return port.get();
    }

    throw CException("Output '" + getName() + "' has no port '" + name + "'.");
}

/**
 * setPort ()
 * set the port of an output, port can only be set once.
 */
void SDFoutput::setPort(std::unique_ptr<SDFport> p) {
    if (p == nullptr) {
        throw CException("Output port '" + p->getName() + "' cannot be nullptr.");
    }

    if (port != nullptr) {
        throw CException("Output port '" + port->getName() + "' already set.");
    }

    if (p->getType() != SDFport::SDFportType::In) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "output '"
                         + getName() + "'; conflicting port type.");
    }

    port = std::move(p);
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &SDFoutput::print(std::ostream &out) {
    out << "Output (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << std::endl;

    port->print(out);

    return out;
}

SDFgraph *SDFoutput::getGraph() const {
    return dynamic_cast<SDFgraph *>(getParent());
}


} // namespace SDF
