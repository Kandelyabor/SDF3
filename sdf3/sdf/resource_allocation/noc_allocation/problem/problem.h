/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   problem.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   October 4, 2006
 *
 *  Function        :   Schedule problem
 *
 *  History         :
 *      04-10-06    :   Initial version.
 *
 * $Id: problem.h,v 1.1.2.1 2010-04-25 01:21:17 mgeilen Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_PROBLEM_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_PROBLEM_H_INCLUDED

#include "schedulingentity.h"  // IWYU pragma: keep
#include "sdf/resource_allocation/noc_allocation/problem/interconnect_graph.h"
#include "sdf/resource_allocation/noc_allocation/problem/message.h"
namespace SDF
{
    // Forward class definition
    class NoCScheduleProblem;

    /**
     * ScheduleSwitchConstraint
     * This data structure describes the time-relation between two scheduling
     * problems when switching from one to another one. The overlap is defined
     * as the amount of time start of the second schedule (to) overlaps with the
     * first schedule (from). When the overlap is equal or larger than the schedule
     * period of the 'from' or 'to' schedule, the two schedules overlap completely.
     */
    using NoCScheduleSwitchConstraint = struct NoCScheduleSwitchConstraint
    {
        NoCScheduleProblem *from;
        NoCScheduleProblem *to;
        TTime overlap;
    };

    using NoCScheduleSwitchConstraints = std::list<NoCScheduleSwitchConstraint>;
    using NoCScheduleSwitchConstraintsIter = NoCScheduleSwitchConstraints::iterator;
    using NoCScheduleSwitchConstraintsCIter = NoCScheduleSwitchConstraints::const_iterator;

    /**
     * NoCScheduleProblem
     * A NoC scheduling problem (single set of messages)
     */
    class NoCScheduleProblem
    {
        public:
            // Constructor
            NoCScheduleProblem(const CString& name, CNode *messagesNode, CNode *archGraphNode,
                               CNode *systemUsageNode);

            // Destructor
            ~NoCScheduleProblem();

            // Prevent copying and moving
            NoCScheduleProblem(const NoCScheduleProblem&) = delete;
            NoCScheduleProblem& operator=(const NoCScheduleProblem&) = delete;
            NoCScheduleProblem(NoCScheduleProblem&&) = delete;
            NoCScheduleProblem& operator=(NoCScheduleProblem&&) = delete;

            // Name
            [[nodiscard]] CString getName() const
            {
                return scheduleName;
            };

            // Interconnect graph
            [[nodiscard]] InterconnectGraph *getInterconnectGraph() const
            {
                return interconnectGraph.get();
            };

            // Schedule switch constraint
            void addScheduleSwitchConstraint(NoCScheduleSwitchConstraint &s)
            {
                scheduleSwitchConstraints.push_back(s);
            };

            // Messages
            MessagesIter messagesBegin()
            {
                return messages.begin();
            };
            MessagesIter messagesEnd()
            {
                return messages.end();
            };
            [[nodiscard]] MessagesCIter messagesBegin() const
            {
                return messages.begin();
            };
            [[nodiscard]] MessagesCIter messagesEnd() const
            {
                return messages.end();
            };
            Messages &getMessages()
            {
                return messages;
            };

            // Scheduling entities
            NoCSchedulingEntitiesIter schedulingEntitiesBegin()
            {
                return schedulingEntities.begin();
            };
            NoCSchedulingEntitiesIter schedulingEntitiesEnd()
            {
                return schedulingEntities.end();
            };
            [[nodiscard]] uint nrSchedulingEntities() const
            {
                return static_cast<uint>(schedulingEntities.size());
            };

            // Problem solved?
            [[nodiscard]] bool isProblemSolved() const
            {
                return solvedFlag;
            };
            bool setSolvedFlag(bool flag)
            {
                solvedFlag = flag;
                return solvedFlag;
            };

            // Schedule constraints (this adds all constraints which are given by the
            // this schedule on schedule p to the schedule p).
            void constrainOtherSchedulingProblem(NoCScheduleProblem *p);

            // Scheduling constraints
            NoCScheduleSwitchConstraintsIter scheduleSwitchConstraintsBegin()
            {
                return scheduleSwitchConstraints.begin();
            };
            NoCScheduleSwitchConstraintsIter scheduleSwitchConstraintsEnd()
            {
                return scheduleSwitchConstraints.end();
            };

            // Slot preferences
            void markPreferredSlots(InterconnectGraph *g);

        private:
            // Construct messages
            void constructMessages(CNode *messagesNode, InterconnectGraph *archGraph);

            // Name of the schedule
            CString scheduleName;

            // Period after which schedule repeats
            TTime schedulePeriod;

            // Interconnect graph
            std::unique_ptr<InterconnectGraph> interconnectGraph;

            // Set of messages which must be scheduled
            Messages messages;

            // Schedule switches
            NoCScheduleSwitchConstraints scheduleSwitchConstraints;

            // Scheduling entities
            NoCSchedulingEntities schedulingEntities;

            // Solved flag
            bool solvedFlag;
    };

    using NoCScheduleProblems = std::list<std::unique_ptr<NoCScheduleProblem>>;
    using NoCScheduleProblemsIter = NoCScheduleProblems::iterator;
    using NoCScheduleProblemsCIter = NoCScheduleProblems::const_iterator;

    /**
     * SetOfNoCScheduleProblems
     * A set of NoC scheduling problems
     */
    class SetOfNoCScheduleProblems
    {
        public:
            // Constructor
            SetOfNoCScheduleProblems(CNode *messagesSetNode, CNode *archGraphNode,
                                     CNode *systemUsageNode);

            // Destructor
            ~SetOfNoCScheduleProblems() = default;

            // Prevent copying and moving
            SetOfNoCScheduleProblems(const SetOfNoCScheduleProblems&) = delete;
            SetOfNoCScheduleProblems& operator=(const SetOfNoCScheduleProblems&) = delete;
            SetOfNoCScheduleProblems(SetOfNoCScheduleProblems&&) = delete;
            SetOfNoCScheduleProblems& operator=(SetOfNoCScheduleProblems&&) = delete;

            // Convert to XML
            CNode *createNetworkMappingNode();
            CNode *createNetworkUsageNode();

            // Schedule problems
            NoCScheduleProblemsIter scheduleProblemsBegin()
            {
                return scheduleProblems.begin();
            };
            NoCScheduleProblemsIter scheduleProblemsEnd()
            {
                return scheduleProblems.end();
            };
            [[nodiscard]] uint nrScheduleProblems() const
            {
                return static_cast<uint>(scheduleProblems.size());
            };

            // Clear the set of scheduling problems
            void clear()
            {
                scheduleProblems.clear();
            };

        private:
            // Schedule problems
            NoCScheduleProblems scheduleProblems;
    };
}//namespace SDF
#endif

