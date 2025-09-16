/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connection.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Connection between two network interface.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: connection.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_CONNECTION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_CONNECTION_H_INCLUDED

#include "fsmsadf/base/type.h"

namespace FSMSADF {

// Forward class definition
class Connection;
class NetworkInterface;
class PlatformGraph;

// List of connection pointers
using Connections = std::list<std::unique_ptr<Connection>>;
using ConnectionRefs = std::list<Connection*>;

/**
 * Connection
 * Container for connection.
 */
class Connection : public GraphComponent {
public:
    // Constructor
    explicit Connection(const GraphComponent &c);

    // Destructor
    ~Connection() override;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<Connection> create(const GraphComponent &c);
    std::unique_ptr<Connection> createCopy(const GraphComponent &c);
    std::unique_ptr<Connection> clone(const GraphComponent &c);

    // Construct
    void constructFromXML(CNodePtr connectionNode);

    // Convert
    void convertToXML(CNodePtr connectionNode);

    // Latency
    [[nodiscard]] Time getLatency() const { return latency; };
    void setLatency(const Time t) { latency = t; };

    // Network interfaces
    NetworkInterface *getSrcNetworkInterface() { return srcNetworkInterface; };
    NetworkInterface *getDstNetworkInterface() { return dstNetworkInterface; };
    void connectSrc(NetworkInterface *ni);
    void connectDst(NetworkInterface *ni);

    // Platform graph
    [[nodiscard]] PlatformGraph *getGraph() const;

private:
    // Latency
    Time latency;

    // Tiles
    NetworkInterface *srcNetworkInterface;
    NetworkInterface *dstNetworkInterface;
};

} // End namespace FSMSADF

#endif
