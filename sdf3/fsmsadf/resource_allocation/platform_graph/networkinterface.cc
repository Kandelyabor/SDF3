/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile network interface.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: networkinterface.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "networkinterface.h"
#include "tile.h"

namespace FSMSADF {

/**
 * NetworkInterface()
 * Constructor.
 */
NetworkInterface::NetworkInterface(const GraphComponent &c) :
    GraphComponent(c),
    maxNrInConnections(0),
    maxNrOutConnections(0),
    inBandwidth(0),
    outBandwidth(0) {}

/**
 * ~NetworkInterface()
 * Destructor.
 */
NetworkInterface::~NetworkInterface() = default;

/**
 * create()
 * The function returns a new NetworkInterface object.
 */
std::unique_ptr<NetworkInterface> NetworkInterface::create(const GraphComponent &c) {
    return std::make_unique<NetworkInterface>(c);
}

/**
 * createCopy()
 * The function returns a new NetworkInterface object and copies all properties.
 */
std::unique_ptr<NetworkInterface> NetworkInterface::createCopy(const GraphComponent &c) const {
    std::unique_ptr<NetworkInterface> n = create(c);

    // Properties
    n->setName(getName());
    n->setMaxNrInConnections(getMaxNrInConnections());
    n->setMaxNrOutConnections(getMaxNrOutConnections());
    n->setInBandwidth(getInBandwidth());
    n->setOutBandwidth(getOutBandwidth());

    return n;
}

/**
 * clone()
 * The function returns a new NetworkInterface object and copies all properties,
 * but when the parent of the graph component c and this object are different,
 * all pointers are relocated. However, the links to the connections are lost.
 * Those are restored when cloning the connections in the platform graph.
 */
std::unique_ptr<NetworkInterface> NetworkInterface::clone(const GraphComponent &c) const {
    std::unique_ptr<NetworkInterface> n = createCopy(c);

    return n;
}

/**
 * constructFromXML()
 * Load all properties of this NetworkInterface from the supplied XML node.
 */
void NetworkInterface::constructFromXML(const CNodePtr networkInterfaceNode) {
    // Name
    if (!CHasAttribute(networkInterfaceNode, "name")) {
        throw CException("Invalid graph, missing network interface name.");
    }
    setName(CGetAttribute(networkInterfaceNode, "name"));

    // Maximal number of input connections
    if (!CHasAttribute(networkInterfaceNode, "nrInConnections")) {
        throw CException("Invalid graph, missing nrInConnections.");
}
    setMaxNrInConnections(
            static_cast<Size>(CGetAttribute(networkInterfaceNode, "nrInConnections")));

    // Maximal number of output connections
    if (!CHasAttribute(networkInterfaceNode, "nrOutConnections")) {
        throw CException("Invalid graph, missing nrOutConnections.");
}
    setMaxNrOutConnections(
            static_cast<Size>(CGetAttribute(networkInterfaceNode, "nrOutConnections")));

    // Maximal number of output connections
    if (!CHasAttribute(networkInterfaceNode, "inBandwidth")) {
        throw CException("Invalid graph, missing inBandwidth.");
}
    setInBandwidth(static_cast<Bandwidth>(CGetAttribute(networkInterfaceNode, "inBandwidth")));

    // Maximal number of output connections
    if (!CHasAttribute(networkInterfaceNode, "outBandwidth")) {
        throw CException("Invalid graph, missing outBandwidth.");
}
    setOutBandwidth(static_cast<Bandwidth>(CGetAttribute(networkInterfaceNode, "outBandwidth")));
}

/**
 * convertToXML()
 * Add all properties of this NetworkInterface to the supplied XML node.
 */
void NetworkInterface::convertToXML(const CNodePtr networkInterfaceNode) {
    // Name
    CAddAttribute(networkInterfaceNode, "name", getName());

    // Maximal number of input connections
    CAddAttribute(networkInterfaceNode, "nrInConnections", CString(getMaxNrInConnections()));

    // Maximal number of output connections
    CAddAttribute(networkInterfaceNode, "nrOutConnections", CString(getMaxNrOutConnections()));

    // Input bandwidth
    CAddAttribute(networkInterfaceNode, "inBandwidth", CString(getInBandwidth()));

    // Output bandwidth
    CAddAttribute(networkInterfaceNode, "outBandwidth", CString(getOutBandwidth()));
}

// Tile
Tile *NetworkInterface::getTile() const {
    return dynamic_cast<Tile *>(getParent());
}


} // namespace FSMSADF
