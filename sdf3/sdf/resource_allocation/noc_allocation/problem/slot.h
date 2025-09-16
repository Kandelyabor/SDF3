/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   slot.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Slot tables
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: slot.h,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_SLOT_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_SLOT_H_INCLUDED

#include <utility>

#include "sdf/basic_types.h"
#include "base/base.h" // IWYU pragma: keep
namespace SDF
{
    /**
     * SlotReservations
     * A sequence of booleans which make wether a slot in the slot table is used or
     * not-used.
     */
    using SlotReservations = std::vector<bool>;

    // Forward class definition
    class NoCSchedulingEntity;
    using NoCSchedulingEntities = std::vector<std::shared_ptr<NoCSchedulingEntity>>;
    using NoCSchedulingEntitiesIter = NoCSchedulingEntities::iterator;
    using NoCSchedulingEntitiesCIter = NoCSchedulingEntities::const_iterator;

    /**
     * SlotTable
     * The slot table...
     */
    class SlotTable
    {
        public:
            // Constructor
            explicit SlotTable(uint sz);

            // Destructor
            ~SlotTable();

            // copying and moving
            SlotTable(const SlotTable&) = default;
            SlotTable& operator=(const SlotTable&) = default;
            SlotTable(SlotTable&&) = default;
            SlotTable& operator=(SlotTable&&) = default;

            // Properties
            [[nodiscard]] TTime getStartTime() const
            {
                return startTime;
            };
            void setStartTime(const TTime t)
            {
                startTime = t;
            };
            [[nodiscard]] TTime getEndTime() const
            {
                return endTime;
            };
            void setEndTime(const TTime t)
            {
                endTime = t;
            };
            [[nodiscard]] uint getNrSlots() const
            {
                return nrSlots;
            };
            [[nodiscard]] uint getNrFreeSlots() const
            {
                return nrFreeSlots;
            };
            [[nodiscard]] bool isSlotFree( uint i) const;

            // Slots
            NoCSchedulingEntitiesIter begin()
            {
                return tableEntities.begin();
            };
            [[nodiscard]] NoCSchedulingEntitiesCIter begin() const
            {
                return tableEntities.begin();
            };
            NoCSchedulingEntitiesIter end()
            {
                return tableEntities.end();
            };
            [[nodiscard]] NoCSchedulingEntitiesCIter end() const
            {
                return tableEntities.end();
            };
            NoCSchedulingEntity *operator[](const uint i)
            {
                return tableEntities[i].get();
            };

            // Slot (de)allocation
            void setUsedSlots(SlotReservations &s);
            void reserveSlots(const std::shared_ptr<NoCSchedulingEntity>& e, SlotReservations &s);
            void releaseSlots(NoCSchedulingEntity *e);

            // Slot reservations
            SlotReservations getSlotReservations(NoCSchedulingEntity *e) const;
            [[nodiscard]] SlotReservations getUsedSlotsOfSchedule() const;

        private:
            // Properties
            TTime startTime;
            TTime endTime;
            uint nrFreeSlots;
            uint nrSlots;
            SlotReservations tableReservations;
            NoCSchedulingEntities tableEntities;
    };

    using SlotTables = std::list<SlotTable>;
    using SlotTablesIter = SlotTables::iterator;
    using SlotTablesCIter = SlotTables::const_iterator;

    /**
     * SlotTableSeq
     * A sequence of slot tables
     */
    class SlotTableSeq
    {
        public:
            // Constructor
            SlotTableSeq(uint slotTableSize, TTime slotTablePeriod);

            // Destructor
            ~SlotTableSeq();

            // Prevent copying and moving
            SlotTableSeq(const SlotTableSeq&) = delete;
            SlotTableSeq& operator=(const SlotTableSeq&) = delete;
            SlotTableSeq(SlotTableSeq&&) = delete;
            SlotTableSeq& operator=(SlotTableSeq&&) = delete;

            // Properties
            [[nodiscard]] uint getSlotTableSize() const
            {
                return slotTableSize;
            };
            [[nodiscard]] TTime getSlotTablePeriod() const
            {
                return slotTablePeriod;
            };
            uint nrFreeSlots( TTime startTime,  TTime duration);

            // Slot tables
            SlotTablesIter seqBegin()
            {
                return seq.begin();
            };
            [[nodiscard]] SlotTablesCIter seqBegin() const
            {
                return seq.begin();
            };
            SlotTablesIter seqEnd()
            {
                return seq.end();
            };
            [[nodiscard]] SlotTablesCIter seqEnd() const
            {
                return seq.end();
            };

            // Modify slot table sequence
            void insertSlotTable(SlotTablesIter pos, SlotTable &s)
            {
                seq.insert(std::move(pos), s);
            };

            // Slot (de)allocation
            void setUsedSlots(SlotReservations &s, TTime startTime, TTime endTime);
            void reserveSlots(const std::shared_ptr<NoCSchedulingEntity>& e, SlotReservations &s,
                               ulong linkSeqNr);
            void releaseSlots(NoCSchedulingEntity *e,  ulong linkSeqNr);

        private:
            // Properties
            uint slotTableSize;
            TTime slotTablePeriod;

            // Slot tables
            SlotTables seq;
    };
}//namespace SDF
#endif
