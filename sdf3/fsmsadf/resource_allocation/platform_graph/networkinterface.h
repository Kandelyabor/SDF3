/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.h
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
 * $Id: networkinterface.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_NETWORKINTERFACE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_NETWORKINTERFACE_H_INCLUDED

#include "fsmsadf/base/type.h"
#include "connection.h"

namespace FSMSADF {

// Forward class definition
class NetworkInterface;
class Tile;

// List of network interface pointers
using NetworkInterfaces = std::list<std::unique_ptr<NetworkInterface>>;

/**
 * NetworkInterface
 * Container for tile network interface.
 */
class NetworkInterface : public GraphComponent {
public:
    // Constructor
    explicit NetworkInterface(const GraphComponent &c);

    // Destructor
    ~NetworkInterface() override;

    NetworkInterface(const NetworkInterface&) = delete;
    NetworkInterface& operator=(const NetworkInterface&) = delete;
    NetworkInterface(NetworkInterface&&) = delete;
    NetworkInterface& operator=(NetworkInterface&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<NetworkInterface> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<NetworkInterface> createCopy(const GraphComponent &c) const;
    [[nodiscard]] std::unique_ptr<NetworkInterface> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr networkInterfaceNode);

    // Convert
    void convertToXML(CNodePtr networkInterfaceNode);

    // Connections
    ConnectionRefs &getInConnections() { return inConnections; };
    ConnectionRefs &getOutConnections() { return outConnections; };
    void addConnectionAsInput(Connection* c) { inConnections.push_back(c); };
    void addConnectionAsOutput(Connection* c) { outConnections.push_back(c); };

    // Maximal number of connections which can be used
    [[nodiscard]] Size getMaxNrInConnections() const { return maxNrInConnections; };
    void setMaxNrInConnections(const Size n) { maxNrInConnections = n; };
    [[nodiscard]] Size getMaxNrOutConnections() const { return maxNrOutConnections; };
    void setMaxNrOutConnections(const Size n) { maxNrOutConnections = n; };

    // Bandwidth
    [[nodiscard]] Bandwidth getInBandwidth() const { return inBandwidth; };
    void setInBandwidth(const Bandwidth b) { inBandwidth = b; };
    [[nodiscard]] Bandwidth getOutBandwidth() const { return outBandwidth; };
    void setOutBandwidth(const Bandwidth b) { outBandwidth = b; };

    // Tile
    [[nodiscard]] Tile *getTile() const;

private:
    // Connections
    ConnectionRefs inConnections;
    ConnectionRefs outConnections;

    // Maximal number of connections which can be used
    Size maxNrInConnections;
    Size maxNrOutConnections;

    // Bandwidth
    Bandwidth inBandwidth;
    Bandwidth outBandwidth;
};

} // End namespace FSMSADF

#endif
