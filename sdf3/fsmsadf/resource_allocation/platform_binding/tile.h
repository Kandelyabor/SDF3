/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: tile.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_TILE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_TILE_H_INCLUDED

#include "memory.h"
#include "networkinterface.h"
#include "processor.h"


namespace FSMSADF {

// Forward class definition
class TileBinding;
class PlatformBinding;

// List of tile  binding pointers
using TileBindings = std::list<std::unique_ptr<TileBinding>>;

/**
 * TileBinding
 * Container for tile bindings.
 */
class TileBinding : public GraphComponent {
public:
    // Constructor
    TileBinding(const GraphComponent& c, Tile *t, BindingMethod bm, bool create = true);

    // Destructor
    ~TileBinding() override;

    TileBinding(const TileBinding&) = delete;
    TileBinding& operator=(const TileBinding&) = delete;
    TileBinding(TileBinding&&) = delete;
    TileBinding& operator=(TileBinding&&) = delete;

    // Construct
    std::unique_ptr<TileBinding> clone(const GraphComponent& c);

    // Construct
    void constructFromXML(const Scenario& s, CNodePtr tileNode);
    void constructResourceUsageFromXML(CNodePtr tileNode);

    // Convert
    void convertToXML(const Scenario& s, CNodePtr tileNode);
    void convertResourceUsageToXML(CNodePtr tileNode);

    // Tile
    [[nodiscard]] Tile *getTile() const { return tile; };

    // Processor bindings
    ProcessorBindings &getProcessorBindings() { return processorBindings; };
    ProcessorBinding* getProcessorBinding(const Processor& p);

    // Memory bindings
    MemoryBinding *getMemoryBinding();

    // Network interface bindings
    NetworkInterfaceBindings &getNetworkInterfaceBindings() { return networkInterfaceBindings; };
    NetworkInterfaceBinding *getNetworkInterfaceBinding(const NetworkInterface *n);

    // Platform binding
    [[nodiscard]] PlatformBinding *getPlatformBinding() const;

    // Get method used for calculating size of binding
    BindingMethod &getBindingMethod() { return bindingMethod; }

    // Set method used for calculating size of binding
    void setBindingMethod(BindingMethod bm) { bindingMethod = bm; }

private:
    // Processor binding
    ProcessorBinding *createProcessorBinding(Processor *p);

    // Memory binding
    MemoryBinding *createMemoryBinding(const MemoryRefs& m, BindingMethod bm);

    // Network interface binding
    NetworkInterfaceBinding *createNetworkInterfaceBinding(NetworkInterface *n);

    // Tile
    Tile *tile;

    // Processor bindings
    ProcessorBindings processorBindings;

    // Memory bindings
    std::unique_ptr<MemoryBinding> memoryBinding;

    // Network interface bindings
    NetworkInterfaceBindings networkInterfaceBindings;

    // Method used for calculating size of binding
    BindingMethod bindingMethod;
};

} // End namespace FSMSADF

#endif
