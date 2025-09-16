/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   knowledge.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Global knowledge NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: knowledge.h,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_KNOWLEDGE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_KNOWLEDGE_H_INCLUDED

#include "noc_scheduler.h"
namespace SDF
{
    using LinkReq = struct LinkReq
    {
        TTime startTime;
        TTime endTime;
        uint nrSlotsReq;
    };

    using LinkReqs = std::list<std::shared_ptr<LinkReq>>;
    using LinkReqsIter = LinkReqs::iterator;
    using LinkReqsCIter = LinkReqs::const_iterator;
    using LinksReqs = std::vector<LinkReqs>;

    /**
     * KnowledgeNoCScheduler ()
     * Knowledge NoC scheduling algorithm
     */
    class KnowledgeNoCScheduler : public NoCScheduler
    {
        public:
            // Constructor
            KnowledgeNoCScheduler(const CSize maxDetour, const uint maxNrRipups)
                : maxDetour(maxDetour), maxNrRipups(maxNrRipups) {};

            // Destructor
            ~KnowledgeNoCScheduler() override = default;

            // Prevent copying and moving
            KnowledgeNoCScheduler(const KnowledgeNoCScheduler&) = delete;
            KnowledgeNoCScheduler& operator=(const KnowledgeNoCScheduler&) = delete;
            KnowledgeNoCScheduler(KnowledgeNoCScheduler&&) = delete;
            KnowledgeNoCScheduler& operator=(KnowledgeNoCScheduler&&) = delete;

            // Schedule function
            bool solve() override
            {
                bool success = knowledge(maxDetour, maxNrRipups);
                return getSchedulingProblem()->setSolvedFlag(success);
            };

        private:
            // Knowledge schedule function
            bool knowledge( CSize maxDetour,  uint maxNrRipups);

            // Estimated requirements
            void setRequirementsMessages(LinksReqs &linksReqs);
            void setRequirementsMessage(Message& m, LinksReqs &linksReqs);
            static void updateRequirementsLink( TTime startTime,  TTime endTime,
                                         bool loopPeriod,  uint nrSlotsReq, LinkReqs &lReqs);

            // Links
            double costLinkForMessage(Message *m, LinkReqs &linkReqs);

            // Schedule streams and messages
            bool findScheduleEntityForMessageUsingKnowledge(Message *m,
                     CSize maxDetour, LinksReqs &linksReqs);
            void sortRoutesOnMessageRequirements(Message *m, LinksReqs &linksReqs,
                                                 Routes &routes);


            CSize maxDetour;
            uint maxNrRipups;
    };
}//namespace SDF
#endif

