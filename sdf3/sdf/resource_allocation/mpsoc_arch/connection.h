/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connection.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Connection between tiles in an architecture graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: connection.h,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_CONNECTION_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_CONNECTION_H_INCLUDED

#include "component.h"
#include "binding.h"
namespace SDF
{
    // Forward class definition
    class Tile;

    /**
     * Connection
     * Container for connection.
     */
    class Connection : public ArchComponent
    {
        public:

            // Constructor
            explicit Connection(ArchComponent &c);

            // Destructor
            ~Connection() override;

            // Rule of Five: explicitly default or delete copy/move operations
            Connection(const Connection&) = delete;
            Connection& operator=(const Connection&) = delete;
            Connection(Connection&&) = delete;
            Connection& operator=(Connection&&) = delete;

            // Latency
            [[nodiscard]] TTime getLatency() const
            {
                return latency;
            };
            void setLatency(const TTime t)
            {
                latency = t;
            };

            // Tiles
            [[nodiscard]] Tile *getSrcTile() const
            {
                return srcTile;
            };
            void setSrcTile(Tile *t)
            {
                srcTile = t;
            };
            [[nodiscard]] Tile *getDstTile() const
            {
                return dstTile;
            };
            void setDstTile(Tile *t)
            {
                dstTile = t;
            };

            // Channel binding
            bool bindChannel(const TimedSDFchannel *c);
            bool unbindChannel(const TimedSDFchannel *c);
            CompBindings *getChannelBindings()
            {
                return channelBindings;
            };

        private:
            // Latency
            TTime latency;

            // Tiles
            Tile *srcTile;
            Tile *dstTile;

            // Channel binding
            CompBindings *channelBindings;
    };

    using Connections = std::vector<std::unique_ptr<Connection>>;
    using ConnectionRefs = std::vector<Connection *>;
    using ConnectionsIter = Connections::iterator;
    using ConnectionsCIter = Connections::const_iterator;
    using ConnectionRefsIter = ConnectionRefs::iterator;
    using ConnectionRefsCIter = ConnectionRefs::const_iterator;
}//namespace SDF
#endif
