/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF actor
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.3 2008-03-22 14:24:21 sander Exp $
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

namespace CSDF {
/**
 * SDFactor ()
 * Constructor.
 */
CSDFactor::CSDFactor(CSDFcomponent &c) : CSDFcomponent(c) {}

/**
 * ~CSDFactor ()
 * Destructor.
 */
CSDFactor::~CSDFactor() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 */
std::unique_ptr<CSDFactor> CSDFactor::create(CSDFcomponent &c) const { return std::make_unique<CSDFactor>(c); }

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * All properties and ports of the actor are cloned.
 */
std::unique_ptr<CSDFactor> CSDFactor::clone(CSDFcomponent &c) const {
    auto a = std::make_unique<CSDFactor>(c);

    // Properties
    a->setName(getName());
    a->setType(getType());

    // Ports
    for (const auto& p : ports) {
        CSDFcomponent component = CSDFcomponent(a.get(), a->nrPorts());
        std::unique_ptr<CSDFport> pA = p->clone(component);
        a->addPort(std::move(pA));
    }

    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * All properties of the actor are copied.
 */
std::unique_ptr<CSDFactor> CSDFactor::createCopy(CSDFcomponent &c) const {
    auto a = std::make_unique<CSDFactor>(c);

    // Properties
    a->setName(getName());
    a->setType(getType());

    return a;
}

/**
 * getGraph()
 */
CSDFgraph *CSDFactor::getGraph() const {
    return dynamic_cast<CSDFgraph *>(getParent());
};


/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void CSDFactor::construct(const CNodePtr actorNode) {
    CSDFactor *a = this;

    // Name
    if (!CHasAttribute(actorNode, "name")) {
        throw CException("Invalid CSDF graph, missing actor name.");
    }
    a->setName(CGetAttribute(actorNode, "name"));

    // Type
    if (!CHasAttribute(actorNode, "type")) {
        throw CException("Invalid CSDF graph, missing actor type.");
    }
    a->setType(CGetAttribute(actorNode, "type"));
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
CSDFport *CSDFactor::createPort(CSDFcomponent &c) {
    auto p = std::make_unique<CSDFport>(c);
    auto* pr = p.get();
    addPort(std::move(p));

    return pr;
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
CSDFport *CSDFactor::createPort(const CString &type, const CSDFrate &rate) {
    CSDFcomponent c = CSDFcomponent(this, nrPorts());
    CSDFport *p = createPort(c);

    p->setType(type);
    p->setRate(rate);

    return p;
}

/**
 * isConnected ()
 * The function returns true if all ports are connected to a channel, else
 * it returns false.
 */
bool CSDFactor::isConnected() const {
    return std::all_of(ports.begin(), ports.end(), [](const auto& p){
        return p->isConnected();
    });
}

/**
 * getPort ()
 * The function returns a reference to a port with the given id.
 */
CSDFport *CSDFactor::getPort(const CId id) {
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
CSDFport *CSDFactor::getPort(const CString &name) {
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
void CSDFactor::addPort(std::unique_ptr<CSDFport> p) {
    ports.push_back(std::move(p));
}

/**
 * removePort ()
 * Remove a port from an actor and destroy its memory space.
 */
void CSDFactor::removePort(const CString &name) {
    for (auto iter = ports.begin(); iter != ports.end(); iter++) {
        const CSDFport& p = *(*iter);

        if (p.getName() == name) {
            ports.erase(iter);
            return;
        }
    }

    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}

/**
 * removePorts ()
 * Remove all ports from the actor and destroy their memory space.
 */
void CSDFactor::removePorts() {
    ports.clear();
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
std::ostream &CSDFactor::print(std::ostream &out) {
    out << "Actor (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;
    out << std::endl;

    for (const auto& p : ports) {
        p->print(out);
    }

    return out;
}
} // namespace CSDF
