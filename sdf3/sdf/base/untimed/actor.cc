/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input.h
 *
 *  Author          :   Joep van Wanrooij (j.c.p.van.wanrooij@tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF input
 *
 *  History         :
 *      30-01-25    :   Initial version.
 *
 * $Id: input.cc,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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
#include "component.h"
#include "graph.h"
#include <algorithm>
#include <memory>
namespace SDF {

/**
 * SDFactor ()
 * Constructor.
 */
SDFactor::SDFactor(SDFcomponent &c) : SDFcomponent(c) {}


/**
 * create ()
 * The function returns a pointer to a newly allocated SDF actor object.
 */
std::unique_ptr<SDFactor> SDFactor::create(SDFcomponent &c) const { return std::make_unique<SDFactor>(c); }

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * All properties and ports of the actor are cloned.
 */
std::unique_ptr<SDFactor> SDFactor::clone(SDFcomponent &c) const {
    auto a = std::make_unique<SDFactor>(c);

    // Properties
    a->setName(getName());
    a->setType(getType());

    // Ports
    for (const auto& p : ports) {
        SDFcomponent component = SDFcomponent(a.get(), a->nrPorts());
        std::unique_ptr<SDFport> pA = p->clone(component);
        a->addPort(std::move(pA));
    }

    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * All properties of the actor are copied.
 */
std::unique_ptr<SDFactor> SDFactor::createCopy(SDFcomponent &c) const {
    auto a = std::make_unique<SDFactor>(c);

    // Properties
    a->setName(getName());
    a->setType(getType());

    return a;
}

/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void SDFactor::construct(const CNodePtr actorNode) {
    SDFactor* a = this;

    // Name
    if (!CHasAttribute(actorNode, "name")) {
        throw CException("Invalid SDF graph, missing actor name.");
    }
    a->setName(CGetAttribute(actorNode, "name"));

    // Type
    if (!CHasAttribute(actorNode, "type")) {
        throw CException("Invalid SDF graph, missing actor type.");
    }
    a->setType(CGetAttribute(actorNode, "type"));
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
SDFportRef SDFactor::createPort(SDFcomponent &c) {
    auto p = std::make_unique<SDFport>(c);
    auto* pr = p.get();
    addPort(std::move(p));

    return pr;
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
SDFportRef SDFactor::createPort(const SDFport::SDFportType type, const SDFrate rate) {
    SDFcomponent c = SDFcomponent(this, nrPorts());
    SDFportRef p = createPort(c);

    p->setType(type);
    p->setRate(rate);

    return p;
}

/**
 * isConnected ()
 * The function returns true if all ports are connected to a channel, else
 * it returns false.
 */
bool SDFactor::isConnected() const {
    return std::all_of(portsBegin(), portsEnd(), [](const auto& p) { return p->isConnected(); });
}

/**
 * getPort ()
 * The function returns a reference to a port with the given id.
 */
SDFport *SDFactor::getPort(const CId id) {
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
SDFport *SDFactor::getPort(const CString &name) {
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
void SDFactor::addPort(std::unique_ptr<SDFport> p) { ports.push_back(std::move(p)); }

SDFgraph * SDFactor::getGraph() const {
    return dynamic_cast<SDFgraph *>(getParent());
}


/**
 * removePort ()
 * Remove a port from an actor and destroy its memory space.
 */
void SDFactor::removePort(const CString &name) {
    for (auto iter = ports.begin(); iter != ports.end(); iter++) {
        const std::unique_ptr<SDFport>& p = *iter;

        if (p->getName() == name) {
            ports.erase(iter);
            return;
        }
    }

    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &SDFactor::print(std::ostream &out) {
    out << "Actor (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;
    out << std::endl;

    for (const auto& p : ports) {
        p->print(out);
    }

    return out;
}
} // namespace SDF
