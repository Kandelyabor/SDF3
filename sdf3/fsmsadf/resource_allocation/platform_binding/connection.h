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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONNECTION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONNECTION_H_INCLUDED

#include "fsmsadf/base/graph.h" // IWYU pragma: keep
#include "fsmsadf/resource_allocation/platform_graph/graph.h" // IWYU pragma: keep


namespace FSMSADF {

// Forward class definition
class ConnectionBinding;
class PlatformBinding;

// List of connection pointers
using ConnectionBindings = std::list<std::unique_ptr<ConnectionBinding>>;

/**
 * ConnectionBinding
 * Container for connection binding.
 */
class ConnectionBinding : public GraphComponent {
public:
    // Constructor
    ConnectionBinding(const GraphComponent& c, Connection *co);

    // Destructor
    ~ConnectionBinding() override;

    ConnectionBinding(const ConnectionBinding&) = delete;
    ConnectionBinding& operator=(const ConnectionBinding&) = delete;
    ConnectionBinding(ConnectionBinding&&) = delete;
    ConnectionBinding& operator=(ConnectionBinding&&) = delete;

    // Construct
    std::unique_ptr<ConnectionBinding> clone(const GraphComponent& c);

    // Construct
    void constructFromXML(const Scenario& s, CNodePtr connectionNode);
    void constructResourceUsageFromXML(CNodePtr connectionNode);

    // Convert
    void convertToXML(const Scenario& s, CNodePtr connectionNode);
    void convertResourceUsageToXML(CNodePtr connectionNode);

    // Connection
    [[nodiscard]] Connection *getConnection() const { return connection; };

    // Channel bindings
    std::map<const Scenario *, ChannelRefs> &getChannelBindings() { return channelBindings; };
    void setChannelBindings(std::map<const Scenario *, ChannelRefs> &ch) { channelBindings = ch; };
    bool addChannelBinding(const Scenario& s, Channel *c);
    void removeChannelBinding(const Scenario& s, const Channel *c);
    bool hasChannelBinding(const Scenario& s, const Channel *c) const;
    static bool hasResourcesChannelBinding(const Scenario& s, const Channel *c);

    // Platform binding
    [[nodiscard]] PlatformBinding *getPlatformBinding() const;

private:
    // Connection
    Connection *connection;

    // Channel bindings
    std::map<const Scenario *, ChannelRefs> channelBindings;
};

} // End namespace FSMSADF

#endif
