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
 * $Id: networkinterface.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_NETWORKINTERFACE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_NETWORKINTERFACE_H_INCLUDED

#include "fsmsadf/base/graph.h" // IWYU pragma: keep
#include "constraint.h"
#include "fsmsadf/resource_allocation/platform_graph/graph.h" // IWYU pragma: keep



namespace FSMSADF {

// Forward class definition
class NetworkInterfaceBinding;
class TileBinding;

// List of network interface binding pointers
using NetworkInterfaceBindings = std::list<std::unique_ptr<NetworkInterfaceBinding>>;

/**
 * NetworkInterfaceBinding
 * Container for tile network interface binding.
 */
class NetworkInterfaceBinding : public GraphComponent {
public:
    // Constructor
    NetworkInterfaceBinding(const GraphComponent& c, NetworkInterface *n);

    // Destructor
    ~NetworkInterfaceBinding() override;

    // Delete copy/move constructors and assignment operators
    NetworkInterfaceBinding(const NetworkInterfaceBinding&) = delete;
    NetworkInterfaceBinding& operator=(const NetworkInterfaceBinding&) = delete;
    NetworkInterfaceBinding(NetworkInterfaceBinding&&) = delete;
    NetworkInterfaceBinding& operator=(NetworkInterfaceBinding&&) = delete;

    // Construct
    std::unique_ptr<NetworkInterfaceBinding> clone(const GraphComponent& c);

    // Construct
    void constructFromXML(const Scenario& s, CNodePtr networkInterfaceNode);
    void constructResourceUsageFromXML(CNodePtr networkInterfaceNode);

    // Convert
    void convertToXML(const Scenario& s, CNodePtr networkInterfaceNode);
    void convertResourceUsageToXML(CNodePtr networkInterfaceNode);

    // Network interface
    [[nodiscard]] NetworkInterface *getNetworkInterface() const { return networkInterface; };

    // Graph binding constraints
    [[nodiscard]] GraphBindingConstraints *getGraphBindingConstraints() const;

    // Channel bindings (input)
    std::map<const Scenario *, ChannelRefs> &getInChannelBindings() { return inChannelBindings; };
    void setInChannelBindings(std::map<const Scenario *, ChannelRefs> &ch);
    bool addInChannelBinding(const Scenario& s, Channel *c);
    void removeInChannelBinding(const Scenario& s, const Channel *c);
    bool hasInChannelBinding(const Scenario& s, const Channel *c) const;

    // Resource occupancy (input)
    [[nodiscard]] Size getAvailableNrInConnections(const Scenario& s) const;
    [[nodiscard]] Size getAllocatedNrInConnections(const Scenario& s) const;
    [[nodiscard]] Bandwidth getAvailableBandwidthInConnections(const Scenario& s) const;
    [[nodiscard]] Bandwidth getAllocatedBandwidthInConnections(const Scenario& s) const;
    bool hasResourcesInChannelBinding(const Scenario& s, const Channel *c) const;

    // Initial resource occupancy (input)
    [[nodiscard]] Size getNrInConnectionsUsedForOtherGraphs() const { return nrInConnectionsUsedForOtherGraphs; };
    void setNrInConnectionsUsedForOtherGraphs(const Size n) {
        nrInConnectionsUsedForOtherGraphs = n;
    };
    [[nodiscard]] Bandwidth getInBandwidthUsedForOtherGraphs() const { return inBandwidthUsedForOtherGraphs; };
    void setInBandwidthUsedForOtherGraphs(const Bandwidth b) { inBandwidthUsedForOtherGraphs = b; };

    // Channel bindings (output)
    std::map<const Scenario *, ChannelRefs> &getOutChannelBindings() { return outChannelBindings; };
    void setOutChannelBindings(std::map<const Scenario *, ChannelRefs> &ch) { outChannelBindings = ch; };
    bool addOutChannelBinding(const Scenario& s, Channel *c);
    void removeOutChannelBinding(const Scenario& s, const Channel *c);
    bool hasOutChannelBinding(const Scenario& s, const Channel *c) const;

    // Resource occupancy (output)
    [[nodiscard]] Size getAvailableNrOutConnections(const Scenario& s) const;
    [[nodiscard]] Size getAllocatedNrOutConnections(const Scenario& s) const;
    [[nodiscard]] Bandwidth getAvailableBandwidthOutConnections(const Scenario& s) const;
    [[nodiscard]] Bandwidth getAllocatedBandwidthOutConnections(const Scenario& s) const;
    bool hasResourcesOutChannelBinding(const Scenario& s, const Channel *c) const;

    // Resource occupancy of application graph
    [[nodiscard]] Size getNrInConnectionsUsedForGraph() const;
    [[nodiscard]] Bandwidth getInBandwidthUsedForGraph() const;
    [[nodiscard]] Size getNrOutConnectionsUsedForGraph() const;
    [[nodiscard]] Bandwidth getOutBandwidthUsedForGraph() const;

    // Initial resource occupancy (output)
    [[nodiscard]] Size getNrOutConnectionsUsedForOtherGraphs() const {
        return nrOutConnectionsUsedForOtherGraphs;
    };
    void setNrOutConnectionsUsedForOtherGraphs(const Size n) {
        nrOutConnectionsUsedForOtherGraphs = n;
    };
    [[nodiscard]] Bandwidth getOutBandwidthUsedForOtherGraphs() const { return outBandwidthUsedForOtherGraphs; };
    void setOutBandwidthUsedForOtherGraphs(const Bandwidth b) {
        outBandwidthUsedForOtherGraphs = b;
    };

    // Tile binding
    [[nodiscard]] TileBinding *getTileBinding() const;

private:
    // Network interface
    NetworkInterface *networkInterface;

    // Channels
    std::map<const Scenario *, ChannelRefs> inChannelBindings;
    std::map<const Scenario *, ChannelRefs> outChannelBindings;

    // Initial resource occupancy
    Size nrInConnectionsUsedForOtherGraphs;
    Size nrOutConnectionsUsedForOtherGraphs;
    Bandwidth inBandwidthUsedForOtherGraphs;
    Bandwidth outBandwidthUsedForOtherGraphs;
};

} // End namespace FSMSADF

#endif
