/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_order_schedule.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 3, 2008
 *
 *  Function        :   Static-order schedule for SDF graphs
 *
 *  History         :
 *      03-03-08    :   Initial version.
 *
 * $Id: static_order_schedule.h,v 1.2 2008-03-06 13:59:06 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_ORDER_SCHEDULE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_ORDER_SCHEDULE_H_INCLUDED

#include "sdf/base/timed/graph.h" // IWYU pragma: keep
namespace SDF {

/**
 * Static-order schedule entry
 * One position in a static-order schedule.
 **/
class StaticOrderScheduleEntry {
public:
    // Constructor
    explicit StaticOrderScheduleEntry(const SDFactor *actor = nullptr) : actor(actor){};

    // Destructor
    ~StaticOrderScheduleEntry() = default;

    // Prevent copying and moving
    StaticOrderScheduleEntry(const StaticOrderScheduleEntry&) = default;
    StaticOrderScheduleEntry& operator=(const StaticOrderScheduleEntry&) = default;
    StaticOrderScheduleEntry(StaticOrderScheduleEntry&&) = default;
    StaticOrderScheduleEntry& operator=(StaticOrderScheduleEntry&&) = default;


    // Actor scheduled with this entry
    const SDFactor* actor;
};

/**
 * Static-order schedule entry iterator
 * Iterator that provides a reference to an entry in a static-order schedule.
 **/
using StaticOrderScheduleEntryIter = std::vector<StaticOrderScheduleEntry>::iterator;
using StaticOrderScheduleEntryCIter = std::vector<StaticOrderScheduleEntry>::const_iterator;

/**
 * Static-order schedule
 * A static-order schedule provides a sequence of actor firings. The schedule
 * may contain a cycle which makes it effectively an infinite schedule.
 **/
class StaticOrderSchedule : public std::vector<StaticOrderScheduleEntry> {
public:
    // Constructor
    StaticOrderSchedule() = default;

    // Destructor
    ~StaticOrderSchedule() = default;

    // Prevent copying and moving
    StaticOrderSchedule(const StaticOrderSchedule&) = default;
    StaticOrderSchedule& operator=(const StaticOrderSchedule&) = default;
    StaticOrderSchedule(StaticOrderSchedule&&) = default;
    StaticOrderSchedule& operator=(StaticOrderSchedule&&) = default;


    // Append actor to the schedule
    void appendActor(const SDFactor *a) { push_back(StaticOrderScheduleEntry(a)); };

    // Append schedule entry to the schedule
    void appendScheduleEntry(StaticOrderScheduleEntry &s) { push_back(s); };

    // Insert actor before the element at position into the schedule and return
    // iterator to newly inserted element
    StaticOrderScheduleEntryIter insertActor(const StaticOrderScheduleEntryIter &position,
                                             SDFactor *a) {
        // Update start of periodic schedule when new element is in transient
        if (begin() + startPeriodicSchedule > position) {
            startPeriodicSchedule++;
        }

        // Insert actor
        return insert(position, StaticOrderScheduleEntry(a));
    };

    // Access schedule entry
    [[nodiscard]] StaticOrderScheduleEntryIter getScheduleEntry(const uint i) {
        if (i >= size()) {
            return end();
        }
        return (begin() + i);
    };

    // Access schedule entry
    [[nodiscard]] StaticOrderScheduleEntryCIter getConstScheduleEntry(const uint i) const {
        if (i >= size()) {
            return end();
        }
        return (begin() + i);
    };

    // Loop-back in schedule
    [[nodiscard]] uint getStartPeriodicSchedule() const { return startPeriodicSchedule; };
    void setStartPeriodicSchedule(const uint i) { startPeriodicSchedule = i; };

    // Next entry in the schedule
    [[nodiscard]] uint next(uint current) const {
        if (isLast(current)) {
            return getStartPeriodicSchedule();
        }

        return ++current;
    };

    // Previous entry in the schedule
    static uint previous(uint current) {
        if (current == 0) {
            return UINT_MAX;
        }

        return --current;
    };

    // Last entry before the loop-back
    [[nodiscard]] bool isLast(uint current) const {
        return current + 1 == static_cast<uint>(size());
    }

    // Compute a minimized version of the schedule
    void minimize();

    // Change the associated actors in the schedule
    void changeActorAssociations(SDFgraph *newGraph);

    // Convert the schedule to an XML representation
    CNode *convertToXML();

private:
    uint startPeriodicSchedule{};
};

} // namespace SDF
#endif
