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
 * $Id: tile.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_TILE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_TILE_H_INCLUDED

#include "memory.h"
#include "networkinterface.h"
#include "processor.h"


namespace FSMSADF {

// Forward class definition
class Tile;

// List of tile pointers
using Tiles = std::list<std::unique_ptr<Tile>>;

/**
 * Tile
 * Container for tile.
 */
class Tile : public GraphComponent {
public:
    // Constructor
    explicit Tile(const GraphComponent &c);

    // Destructor
    ~Tile() override;

    Tile(const Tile &) = delete;
    Tile &operator=(const Tile &) = delete;
    Tile(Tile &&) = delete;
    Tile &operator=(Tile &&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<Tile> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<Tile> createCopy(const GraphComponent &c) const;
    [[nodiscard]] std::unique_ptr<Tile> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr tileNode);

    // Convert
    void convertToXML(CNodePtr tileNode);

    // Processors
    Processors &getProcessors() { return processors; };
    [[nodiscard]] Processor *getProcessor(const CString &name) const;
    Processor *createProcessor(const CString &name);
    void addProcessor(std::unique_ptr<Processor> p);
    void removeProcessor(Processor *p);

    // Memories
    Memories &getMemories() { return memories; };
    [[nodiscard]] Memory *getMemory(const CString &name) const;
    Memory *createMemory(const CString &name);
    void addMemory(std::unique_ptr<Memory> m);
    void removeMemory(Memory *m);
    bool hasSharedMemories();
    std::unique_ptr<MemoryRefs> getSharedMemories();

    // Network interfaces
    NetworkInterfaces &getNetworkInterfaces() { return networkInterfaces; };
    [[nodiscard]] NetworkInterface *getNetworkInterface(const CString &name) const;
    NetworkInterface *createNetworkInterface(const CString &name);
    void addNetworkInterface(std::unique_ptr<NetworkInterface> n);
    void removeNetworkInterface(NetworkInterface *n);

private:
    // Processors
    Processors processors;

    // Memories
    Memories memories;

    // Network interfaces
    NetworkInterfaces networkInterfaces;
};

} // End namespace FSMSADF

#endif
