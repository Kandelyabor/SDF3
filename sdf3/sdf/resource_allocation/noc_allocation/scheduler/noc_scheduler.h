/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   noc_scheduler.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Communication scheduling
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: noc_scheduler.h,v 1.1 2008-03-20 16:16:21 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_NOC_SCHEDULER_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_NOC_SCHEDULER_H_INCLUDED

#include "sdf/resource_allocation/noc_allocation/problem/problem.h"
#include "sdf/resource_allocation/noc_allocation/problem/message.h"
#include "sdf/resource_allocation/noc_allocation/problem/packet.h"

namespace SDF
{
    class NoCScheduler
    {
        public:
            // Constructor
            NoCScheduler()= default;

            // Destructor
            virtual ~NoCScheduler() = default;

            // Prevent copying and moving
            NoCScheduler(const NoCScheduler&) = delete;
            NoCScheduler& operator=(const NoCScheduler&) = delete;
            NoCScheduler(NoCScheduler&&) = delete;
            NoCScheduler& operator=(NoCScheduler&&) = delete;

            // Schedule function
            bool schedule(SetOfNoCScheduleProblems &problems);

            // Load scheduling entities for the scheduling problems from XML
            void assignSchedulingEntities(SetOfNoCScheduleProblems &problems,
                                          CNode *networkMappingNode);

        protected:
            // Schedule the current scheduling problem
            virtual bool solve() = 0;

            // Load scheduling entities for the current scheduling problems from XML
            void assignSchedulingEntities(CNode *messagesNode);

            // Mark preferred slots in the interconnect graph
            void markPreferredSlotsOnLinks(SetOfNoCScheduleProblems &problems);

            // Scheduling problem that is being solved
            void setSchedulingProblem(NoCScheduleProblem *p)
            {
                curScheduleProblem = p;
            };
            [[nodiscard]] NoCScheduleProblem *getSchedulingProblem() const
            {
                ASSERT(curScheduleProblem != nullptr, "No scheduling problem set.");
                return curScheduleProblem;
            };

            // Interconnect graph of the current scheduling problem
            [[nodiscard]] InterconnectGraph *getInterconnectGraph() const
            {
                return getSchedulingProblem()->getInterconnectGraph();
            };

            // Messages of the current scheduling problem
            MessagesIter messagesBegin()
            {
                return getSchedulingProblem()->messagesBegin();
            };
            MessagesIter messagesEnd()
            {
                return getSchedulingProblem()->messagesEnd();
            };
            [[nodiscard]] MessagesCIter messagesBegin() const
            {
                return getSchedulingProblem()->messagesBegin();
            };
            [[nodiscard]] MessagesCIter messagesEnd() const
            {
                return getSchedulingProblem()->messagesEnd();
            };

            // Scheduling entities of the current scheduling problem
            NoCSchedulingEntitiesIter schedulingEntitiesBegin()
            {
                return getSchedulingProblem()->schedulingEntitiesBegin();
            };
            NoCSchedulingEntitiesIter schedulingEntitiesEnd()
            {
                return getSchedulingProblem()->schedulingEntitiesEnd();
            };
            [[nodiscard]] uint nrSchedulingEntities() const
            {
                return getSchedulingProblem()->nrSchedulingEntities();
            };

            // Routing
            void findAllRoutes(const Node *src, const Node *dst,
                                CSize maxDetour, bool exact, Routes &routes);
            void findRoutes(const Node *src, const Node *dst,  CSize minLength,
                             CSize maxLength, Route &route, Routes &routes);
            CSize getLengthShortestPathBetweenNodes(const Node *src, const Node *dst);

            // Slots
            uint minFreeSlotsOnLink(Link *l,  TTime startTime,
                                     TTime duration);
            bool findSlotsOnRoute(NoCSchedulingEntity *e, SlotReservations &s);
            uint nrSlotsRequired( TTime duration,  CSize size,
                                  uint nrPacketsPerSlotTable);
            void findFreePackets(NoCSchedulingEntity *e, SlotReservations &slotsRoute,
                                 Packets &packets);
            SlotReservations findFreeSlotsOnRoute(const Route *r,  TTime startTime,
                                                   TTime duration);
            bool findSlotsOnRoute(NoCSchedulingEntity *e,
                                  SlotReservations &slotsForPackets, SlotReservations &s);
            SlotReservations findFreeSlotsOnLink(const Link *l,  TTime startTime,
                                                  TTime duration);
            SlotReservations findFreeSlotsOnFirstLinkRoute(const Route *r,
                     TTime startTime,  TTime duration);
            void raisePreferenceLevelSlotsOnRoute(NoCSchedulingEntity *e);
            void lowerPreferenceLevelSlotsOnRoute(NoCSchedulingEntity *e);

            // Stream
            void ripupStream(MessagesIter iterMsg);
            void findSlotsAllocatedForStream(NoCSchedulingEntity *e,
                                             SlotReservations &slotsAllocated);
            void findFreeSlotsForStream(Route *r,
                                        SlotReservations &slotsRoute);

            // Conflicts
            double severityConflict(NoCSchedulingEntity *e, Message *m, Route *r);
            void ripupScheduleEntity(MessagesIter iterMsg);

            // Cost
            void sortMessagesOnCost();
            void sortRoutesOnCost(Routes &routes,  TTime startTime,
                                   TTime duration);

            // Scheduling entities
            bool findScheduleEntityForMessage(Message *m,  CSize maxDetour);
            static TTime earliestStartTime(const Message *m);
            static TTime maximalDuration(const Message *m, TTime startTime,
                                  CSize lengthRoute);
            TTime minimalDuration(const Message *m, TTime startTime,
                                  SlotReservations slotReservations) const;
            static NoCSchedulingEntity *getFirstScheduledMessageInStream(Message *m);

            // Resource management
            void reserveResources(const std::shared_ptr<NoCSchedulingEntity>& e);
            void releaseResources(NoCSchedulingEntity *e);

            // Routes (hash)
            void createRoutesHash( CSize maxDetour);
            void destroyRoutesHash();

            // Print
            std::ostream &print(std::ostream &out) const;

        private:
            // Scheduling problem that is being solved
            NoCScheduleProblem *curScheduleProblem{};
    };
}//namespace SDF
#endif
