/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   link.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Link
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: link.h,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_LINK_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_LINK_H_INCLUDED

#include "slot.h"
namespace SDF
{
    // Forward class definition
    class Node;

    /**
     * Link
     * A physical connection between two nodes
     */
    class Link
    {
        public:
            // Constructor
            Link(CString name, CId id, Node *src, Node *dst, uint slotTableSize,
                 TTime slotTablePeriod);

            // Destructor
            ~Link();

            // Prevent copying and moving
            Link(const Link&) = delete;
            Link& operator=(const Link&) = delete;
            Link(Link&&) = delete;
            Link& operator=(Link&&) = delete;

            // Id
            [[nodiscard]] CId getId() const
            {
                return id;
            };

            // Name
            [[nodiscard]] CString getName() const
            {
                return name;
            };

            // Nodes
            [[nodiscard]] Node *getSrcNode() const
            {
                return srcNode;
            };
            [[nodiscard]] Node *getDstNode() const
            {
                return dstNode;
            };

            // Slot
            SlotTablesIter slotTableSeqBegin()
            {
                return slotTableSeq->seqBegin();
            };
            [[nodiscard]] SlotTablesCIter slotTableSeqBegin() const
            {
                return slotTableSeq->seqBegin();
            };
            SlotTablesIter slotTableSeqEnd()
            {
                return slotTableSeq->seqEnd();
            };
            [[nodiscard]] SlotTablesCIter slotTableSeqEnd() const
            {
                return slotTableSeq->seqEnd();
            };
            [[nodiscard]] SlotTableSeq *getSlotTableSeq() const
            {
                return slotTableSeq.get();
            };
            SlotReservations getUsedSlotsInSchedule();

            // Slot (de)allocation
            void setUsedSlots(SlotReservations &s, TTime startTime, TTime endTime);
            void reserveSlots(const std::shared_ptr<NoCSchedulingEntity>& e, SlotReservations &s, ulong seqNr)
            {
                slotTableSeq->reserveSlots(e, s, seqNr);
            };
            void releaseSlots(NoCSchedulingEntity *e, ulong seqNr)
            {
                slotTableSeq->releaseSlots(e, seqNr);
            };

            // Preferred slots (value indicates preference level)
            std::vector<uint> preferredSlots;

            // Print
            std::ostream &print(std::ostream &out) const;

        private:
            // Name
            CString name;

            // Id
            CId id;

            // Nodes
            Node *srcNode;
            Node *dstNode;

            // Slot
            std::unique_ptr<SlotTableSeq> slotTableSeq;
    };

    using Links = std::list<std::unique_ptr<Link>>;
    using LinkRefs = std::list<Link *>;
    using LinksIter = Links::iterator;
    using LinksCIter = Links::const_iterator;
    using LinkRefsIter = LinkRefs::iterator;
    using LinkRefsCIter = LinkRefs::const_iterator;
}//namespace SDF
#endif
