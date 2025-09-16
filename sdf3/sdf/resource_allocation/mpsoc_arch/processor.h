/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Processor.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: processor.h,v 1.2 2008-03-06 10:49:45 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_PROCESSOR_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_PROCESSOR_H_INCLUDED

#include "component.h"
#include "binding.h"
#include "sdf/resource_allocation/scheduling/static_order_schedule.h"


namespace SDF
{
    /**
     * Processor
     * Container for processor.
     */
    class Processor : public ArchComponent
    {
        public:

            // Constructor
            explicit Processor(ArchComponent &c);

            // Copy constructor
            Processor(const Processor& other) = delete;

            // Copy assignment operator
            Processor& operator=(const Processor& other) = delete;

            // Move constructor
            Processor(Processor&& other) noexcept = delete;

            // Move assignment operator
            Processor& operator=(Processor&& other) noexcept = delete;

            // Destructor
             ~Processor() override
            {
                delete actorBindings;
            };

            // Type
            [[nodiscard]] CString getType() const
            {
                return type;
            };
            void setType(const CString &t)
            {
                type = t;
            };

            // TDMA timewheel
            [[nodiscard]] CSize getTimewheelSize() const;
            void setTimewheelSize( CSize sz);
            [[nodiscard]] CSize getOccupiedTimeSlice() const;
            void setOccupiedTimeSlice( CSize sz);
            [[nodiscard]] CSize getReservedTimeSlice() const;
            bool reserveTimeSlice(CSize sz);
            bool releaseTimeSlice();
            [[nodiscard]] CSize availableTimewheelSize() const;

            // Schedule
            StaticOrderSchedule &getSchedule()
            {
                return schedule;
            };
            void setSchedule(StaticOrderSchedule &s)
            {
                schedule = s;
            };

            // Actor binding
            bool bindActor(const TimedSDFactor *a);
            bool unbindActor(const TimedSDFactor *a);
            CompBindings *getActorBindings()
            {
                return actorBindings;
            };

        private:
            // Type
            CString type;

            // TDMA timewheel
            CSize wheelSize;
            CSize occupiedTimeSlice;
            CSize reservedTimeSlice;

            // Schedule
            StaticOrderSchedule schedule;

            // Actor binding
            CompBindings *actorBindings;
    };

    using Processors = std::list<Processor *>;
    using ProcessorsIter = Processors::iterator;
    using ProcessorsCIter = Processors::const_iterator;
}//namespace SDF
#endif
