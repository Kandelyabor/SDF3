/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   packet.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Packet
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: packet.h,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_PACKET_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_PACKET_H_INCLUDED

#include "sdf/basic_types.h"
#include "base/basic_types.h"
namespace SDF
{
    /**
     * Packet
     * Container for packet size, start time, and end time
     */
    class Packet
    {
        public:
            // Constructor
            Packet() = default;

            // Destructor
            ~Packet() = default;

            // Prevent copying and moving
            Packet(const Packet&) = delete;
            Packet& operator=(const Packet&) = delete;
            Packet(Packet&&) = delete;
            Packet& operator=(Packet&&) = delete;


            // Properties
            CSize nrSlots{};
            TTime startTime{};
            TTime endTime{};
            bool loop{};
    };

    using Packets = std::list<std::unique_ptr<Packet>>;
    using PacketsIter = Packets::iterator;
    using PacketsCIter = Packets::const_iterator;

    struct PacketLess
    {
        bool operator()(const std::unique_ptr<Packet>& x, const std::unique_ptr<Packet>& y)
        {
            return x->nrSlots < y->nrSlots;
        }
    };

}//namespace SDF
#endif
