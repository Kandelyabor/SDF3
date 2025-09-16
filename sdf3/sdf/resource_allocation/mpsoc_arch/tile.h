/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Tile in the architecture graph.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: tile.h,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_TILE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_TILE_H_INCLUDED

#include "connection.h"
#include "memory.h"
#include "networkinterface.h"
#include "processor.h"

namespace SDF
{
    /**
     * Tile
     * Container for tile.
     */
    class Tile : public ArchComponent
    {
        public:

            // Constructor
            explicit Tile(ArchComponent &c);

            // Destructor
             ~Tile() override;

            // Delete copy and move constructors and assignment operators
            Tile(const Tile&) = delete;
            Tile& operator=(const Tile&) = delete;
            Tile(Tile&&) = delete;
            Tile& operator=(Tile&&) = delete;

            // Processor
            [[nodiscard]] Processor *getProcessor() const
            {
                return processor.get();
            };
            Processor *createProcessor(const CString &name);
            void setProcessor(std::unique_ptr<Processor>& p)
            {
                processor = std::move(p);
            };

            // Memory
            [[nodiscard]] Memory *getMemory() const
            {
                return memory.get();
            };
            Memory *createMemory(const CString &name);
            void setMemory(std::unique_ptr<Memory>& m)
            {
                memory = std::move(m);
            };

            // Network interface
            [[nodiscard]] NetworkInterface *getNetworkInterface() const
            {
                return networkInterface.get();
            };
            NetworkInterface *createNetworkInterface(const CString &name);
            void setNetworkInterface(std::unique_ptr<NetworkInterface>& n)
            {
                networkInterface = std::move(n);
            };

            // Connections
            ConnectionRefsIter inConnectionsBegin()
            {
                return inConnections.begin();
            };
            ConnectionRefsIter inConnectionsEnd()
            {
                return inConnections.end();
            };
            void addInConnection(Connection* c);
            [[nodiscard]] ConnectionRefsCIter outConnectionsBegin() const
            {
                return outConnections.begin();
            };
            [[nodiscard]] ConnectionRefsCIter outConnectionsEnd() const
            {
                return outConnections.end();
            };
            void addOutConnection(Connection* c);

        private:
            // Processor
            std::unique_ptr<Processor> processor;

            // Memory
            std::unique_ptr<Memory> memory;

            // Network interface
            std::unique_ptr<NetworkInterface> networkInterface;

            // Connections
            ConnectionRefs inConnections;
            ConnectionRefs outConnections;
    };

    using TileCollection = std::vector<std::unique_ptr<Tile>>;
    using TileRef = Tile*;
    using TileConstRef = const Tile*;
    using TileRefs = std::vector<TileRef>;
    using TilesIter = TileCollection::iterator;
    using TilesCIter = TileCollection::const_iterator;
    using ActorTileBinding = std::vector<TileConstRef>;
}//namespace SDF
#endif
