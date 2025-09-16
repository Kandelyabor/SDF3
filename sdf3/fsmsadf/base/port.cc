/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF port
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: port.cc,v 1.1.2.5 2010-04-22 07:07:10 mgeilen Exp $
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

#include "port.h"
#include "graph.h"
#include <memory>

namespace FSMSADF {

/**
 * Port ()
 * Constructor.
 */
Port::Port(const GraphComponent &c) : GraphComponent(c), type(PortType::Undef), channel(nullptr) {
    // Set the default rate
    setRateOfScenario(*getDefaultScenarioRef(), 1);
}

/**
 * ~Port ()
 * Destructor.
 */
Port::~Port() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated port object.
 */
std::unique_ptr<Port> Port::create(const GraphComponent &c) { return std::make_unique<Port>(c); }

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF port object.
 * The properties of the port are also copied.
 */
std::unique_ptr<Port> Port::createCopy(const GraphComponent &c) const {
    std::unique_ptr<Port> p = create(c);

    p->setName(getName());
    p->setType(getType());
    p->setRate(getRate());

    return p;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated port object.
 * The properties of the port are also cloned. Note: the connection to the
 * channel is lost. Cloning the channel restores this connection.
 */
std::unique_ptr<Port> Port::clone(const GraphComponent &c) const {
    std::unique_ptr<Port> p = createCopy(c);

    return p;
}

/**
 * constructFromXML ()
 * The function initializes all port properties based on the XML data.
 */
void Port::constructFromXML(const CNodePtr portNode) {
    // Name
    if (!CHasAttribute(portNode, "name")) {
        throw CException("Invalid graph, missing port name.");
    }
    setName(CGetAttribute(portNode, "name"));

    // Type
    if (!CHasAttribute(portNode, "type")) {
        throw CException("Invalid graph, missing port type.");
    }
    setType(CGetAttribute(portNode, "type"));

    // Rate
    if (!CHasAttribute(portNode, "rate")) {
        throw CException("Invalid graph, missing port rate.");
    }
    setRateOfScenario(*getDefaultScenarioRef(), std::stoi(CGetAttribute(portNode, "rate")));
}

/**
 * convertToXML ()
 * The function converts all port properties to XML data.
 */
void Port::convertToXML(const CNodePtr portNode) {
    // Name
    CAddAttribute(portNode, "name", getName());

    // Type
    CAddAttribute(portNode, "type", getTypeAsString());

    // Rate
    CAddAttribute(portNode, "rate", CString(getRateOfScenario(*getDefaultScenarioRef())));
}

/**
 * isolateScenario()
 * The function removes all scenario except scenario s from the actor. The rate
 * of this scenario is set as the rate of the default scenario on this graph.
 */
void Port::isolateScenario(const Scenario& s) {
    // Rate
    Rate r = getRateOfScenario(s);
    rate.clear();
    setRateOfScenario(*getDefaultScenarioRef(), r);
}

/**
 * getTypeAsString()
 * The function returns a string which specifies the port type.
 */
CString Port::getTypeAsString() const {
    if (type == PortType::In) {
        return "in";
    }
    if (type == PortType::Out) {
        return "out";
    }
    return "undef";
}

/**
 * setType()
 * The function sets the port type based on the string value.
 */
void Port::setType(const CString &t) {
    if (t == CString("in")) {
        type = PortType::In;
    } else if (t == CString("out")) {
        type = PortType::Out;
    } else {
        type = PortType::Undef;
    }
}

/**
 * setRate ()
 * The function sets the port rates of all scenarios.
 */
void Port::setRate(const std::map<const Scenario *, Rate> &r) {
    // Clear existing map
    rate.clear();

    // Iterate over the new map
    for (auto [sc, rt]: r) {
        // Find corresponding scenario in this graph
        ScenarioRef s = getActor().getScenarioGraph()->getGraph()->getScenarioRef(sc->getName());
        setRateOfScenario(*s, rt);
    }
}

Actor& Port::getActor() const {
    return *(dynamic_cast<Actor *>(getParent()));
}


/**
 * getRateOfScenario()
 * The function returns the rate of the port in the given scenario. When no
 * specific rate is specified for the scenario, the default scenario is used.
 */
Rate Port::getRateOfScenario(const Scenario& s) const {
    if (rate.find(&s) != rate.end()) {
        return rate.find(&s)->second;
    }

    if (rate.find(&(*getDefaultScenarioRef())) == rate.end()) {
        throw CException("Port '" + getName() + "' has no rate.");
    }

    return rate.find(&(*getDefaultScenarioRef()))->second;
}

/**
 * setRateOfScenario()
 * The function sets the rate of the port in the given scenario.
 */
void Port::setRateOfScenario(const Scenario& s, const Rate r) { rate[&s] = r; }

/**
 * connectToChannel ()
 * The function connects the port to a channel. A connection can only be made
 * if the port is not yet connected.
 */
void Port::connectToChannel(Channel *c) {
    if (isConnected() && c != nullptr) {
        throw CException("Port '" + getParent()->getName() + "." + getName()
                         + "' already connected.");
    }

    channel = c;
}

/**
 * isConnected ()
 * The function return true if the port is connected to a channel, else it
 * returns false.
 */
bool Port::isConnected() const {
    return getChannel() != nullptr;
}

/**
 * getDefaultScenario ()
 * The function returns a pointer to the default scenario.
 */
ScenarioRef Port::getDefaultScenarioRef() const {
    return getActor().getScenarioGraph()->getGraph()->getDefaultScenario();
}

/**
 * print ()
 * Print the port to the supplied output stream.
 */
std::ostream &Port::print(std::ostream &out) {
    out << "Port (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getTypeAsString() << std::endl;
    out << "connected: " << (isConnected() ? "true" : "false") << std::endl;

    for (auto [sc, r]: rate) {
        out << "rate: " << r << " (scenario: " << sc->getName();
        out << ")" << std::endl;
    }

    out << std::endl;

    return out;
}

} // End namespace FSMSADF
