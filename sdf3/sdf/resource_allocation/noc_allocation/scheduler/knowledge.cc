/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   knowledge.cc
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
 * $Id: knowledge.cc,v 1.2.2.1 2010-04-25 02:08:51 mgeilen Exp $
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

#include "knowledge.h"

#include <algorithm>

namespace SDF {
/**
 * knowledge ()
 * Global knowledge NoC scheduling algorithm
 */
bool KnowledgeNoCScheduler::knowledge(const CSize maxDetour, const uint maxNrRipups) {
    bool found = false;
    LinksReqs linksReqs;
    uint nrRipups = 0;

    // Compute requirements for all messages
    setRequirementsMessages(linksReqs);

        // // Print requirements for all messages
        // std::cerr << "# Link requirements" << std::endl;
        // for (uint i = 0; i < getInterconnectGraph()->nrLinks(); i++)
        // {
        //     LinkReqs &linkReqs = linksReqs[i];

        //     std::cerr << "Link " << i << ": " << std::endl;

        //     for (LinkReqsIter iter = linkReqs.begin();
        //          iter != linkReqs.end(); iter++)
        //     {
        //         LinkReq *lReq = *iter;


        //         std::cerr << "(" << lReq->startTime << ", " << lReq->endTime << "): ";
        //         std::cerr << lReq->nrSlotsReq << std::endl;

        //     }
        //     std::cerr << std::endl;
        // }

    // Sort all messages using cost function
    sortMessagesOnCost();

    // Iterate over sorted messages
    for (auto iter = messagesBegin(); iter != messagesEnd(); iter++) {
        Message& m = *(*iter);

        // Try to find a scheduling entity for the message
        found = findScheduleEntityForMessageUsingKnowledge(&m, maxDetour, linksReqs);

        // Failed to find a scheduling entity?
        if (!found) {
            if (nrRipups < maxNrRipups) {
                // Ripup some already scheduled message
                ripupScheduleEntity(iter);
                nrRipups++;

                // Re-try current message
                iter--;
            } else {
                // Output slot tables on all links
                // print(std::cerr);
                std::cerr << "Failed finding scheduling entity for message: ";
                m.print(std::cerr);
                std::cerr << std::endl;

                return false;
            }
        }
    }

    // All messages scheduled.
    return true;
}

/**
 * setRequirementsMessages ()
 * The function computes the requirements for all messages and store them in
 * the array linksReqs which contains the requirements for all links in the
 * architecture graph.
 */
void KnowledgeNoCScheduler::setRequirementsMessages(LinksReqs &linksReqs) {
    uint nrLinks = getInterconnectGraph()->nrLinks();
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();

    // Construct array of link requirements
    linksReqs.resize(nrLinks);
    for (uint i = 0; i < nrLinks; i++) {
        auto lReq = std::make_unique<LinkReq>();
        lReq->startTime = 0;
        lReq->endTime = slotTablePeriod - 1;
        lReq->nrSlotsReq = 0;
        linksReqs[i].push_back(std::move(lReq));
    }

    // Iterate over all messages
    for (auto iter = messagesBegin(); iter != messagesEnd(); iter++) {
        Message& m = *(*iter);

        setRequirementsMessage(m, linksReqs);
    }
}

/**
 * updateRequirementsLink ()
 * Add the slot requirements (nrSlotsReq) to the correct time intervals
 * for the link requirements 'lReqs'.
 */
void KnowledgeNoCScheduler::updateRequirementsLink(const TTime startTime,
                                                   const TTime endTime,
                                                   const bool loopPeriod,
                                                   const uint nrSlotsReq,
                                                   LinkReqs &lReqs) {
    for (auto iterR = lReqs.begin(); iterR != lReqs.end(); iterR++) {
        LinkReq& lReq = *(*iterR);

        // Link requirement starts after end message
        if (!loopPeriod && lReq.startTime > endTime) {
            return;
        }

        // Link requirement within time interval message (without loop)
        if (!loopPeriod && lReq.endTime >= startTime) {
            if (lReq.startTime < startTime) {
                // Link requirement starts before message
                auto lnReq = std::make_unique<LinkReq>();
                lnReq->startTime = lReq.startTime;
                lnReq->endTime = startTime - 1;
                lnReq->nrSlotsReq = lReq.nrSlotsReq;
                lReq.startTime = startTime;
                lReqs.insert(iterR, std::move(lnReq));
                iterR--;
            } else if (lReq.endTime <= endTime) {
                // Link requirement ends before message ends
                lReq.nrSlotsReq += nrSlotsReq;
            } else if (endTime < lReq.endTime) {
                // Link requirement ends after message ends
                auto lnReq = std::make_unique<LinkReq>();
                lnReq->startTime = lReq.startTime;
                lnReq->endTime = endTime;
                lnReq->nrSlotsReq = lReq.nrSlotsReq + nrSlotsReq;
                lReq.startTime = endTime + 1;
                lReqs.insert(iterR, std::move(lnReq));
            }
        }

        // Link requirement within time interval message (with loop)
        if (loopPeriod
            && (lReq.endTime >= startTime || (loopPeriod && lReq.startTime <= endTime))) {
            if (lReq.endTime <= endTime) {
                // Link requirement ends before message
                lReq.nrSlotsReq += nrSlotsReq;
            } else if (endTime < lReq.endTime && endTime >= lReq.startTime) {
                // Link requirement ends after message ends
                auto  lnReq = std::make_unique<LinkReq>();
                lnReq->startTime = lReq.startTime;
                lnReq->endTime = endTime;
                lnReq->nrSlotsReq = lReq.nrSlotsReq + nrSlotsReq;
                lReq.startTime = endTime + 1;
                lReqs.insert(iterR, std::move(lnReq));
            }

            if (lReq.startTime >= startTime) {
                // Link requirement starts after message starts
                lReq.nrSlotsReq += nrSlotsReq;
            } else if (lReq.endTime >= startTime) {
                // Link requirement starts before message starts
                auto lnReq = std::make_unique<LinkReq>();
                lnReq->startTime = lReq.startTime;
                lnReq->endTime = startTime - 1;
                lReq.nrSlotsReq = lReq.nrSlotsReq + nrSlotsReq;
                lReq.startTime = startTime;
                lReqs.insert(iterR, std::move(lnReq));
            }
        }
    }
}

/**
 * setRequirementsMessage ()
 * The function computes the requirements for a message and updates the
 * information in the requirements on the links (i.e. nr of slots needed
 * on links)
 */
void KnowledgeNoCScheduler::setRequirementsMessage(Message& m, LinksReqs &linksReqs) {
    // No requirements set for any link regarding this message
    std::vector<bool> setLinkReqs(getInterconnectGraph()->nrLinks(), false);

    // Start and end time of the message
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    TTime startTime = m.getStartTime();
    TTime endTime = (m.getStartTime() + m.getDuration()) % slotTablePeriod;
    bool loopPeriod = false;

    // The scheduling entity loops around the period when its start time
    // is equal or more than its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less than a
    // period.
    if (startTime >= endTime && m.getDuration() != 1) {
        loopPeriod = true;
    }

    // Source and destination node in the architecture graph
    Node *srcNode = getInterconnectGraph()->getNode(m.getSrcNodeId());
    Node *dstNode = getInterconnectGraph()->getNode(m.getDstNodeId());

    // Number of slots required on shortest route (assuming single packet)
    uint nrSlotsReq = nrSlotsRequired(m.getDuration(), m.getSize(), 1);

    Routes routes;

    // Get a list of all shortest routes for the message
    findAllRoutes(srcNode, dstNode, 0, true, routes);

    // Iterate over all routes and set slot requirements on their links
    for (auto &r : routes) {
        for (auto iterL = r.linksBegin(); iterL != r.linksEnd(); iterL++) {
            Link *l = *iterL;
            LinkReqs &lReqs = linksReqs[l->getId()];

            // Already set requirements for this link?
            if (!setLinkReqs[l->getId()]) {
                setLinkReqs[l->getId()] = true;
                updateRequirementsLink(startTime, endTime, loopPeriod, nrSlotsReq, lReqs);
            }
        }
    }
}

/**
 * costLinkForMessage ()
 * The cost for using a link by a message is the maximum number of slots
 * allocated in the time period within which the message can be sent.
 */
double KnowledgeNoCScheduler::costLinkForMessage(Message *m, LinkReqs &linkReqs) {

    // Start and end time of the message
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    TTime startTime = m->getStartTime();
    TTime endTime = (m->getStartTime() + m->getDuration()) % slotTablePeriod;
    bool loopPeriod = (startTime > endTime);

    double maxNrSlots = 0;
    for (auto iter = linkReqs.begin(); iter != linkReqs.end(); iter++) {
        LinkReq& lReq = *(*iter);

        // Link requirements after end of message
        if (!loopPeriod && lReq.startTime > endTime) {
            break;
        }

        // Link requirement during time of message
        if (lReq.endTime >= startTime || (loopPeriod && lReq.startTime <= endTime)) {
            // Nr slots required in this time interval larger than know max?
            maxNrSlots = (std::max)(static_cast<double>(lReq.nrSlotsReq), maxNrSlots);
        }
    }

    return maxNrSlots;
}

/**
 * sortRoutesOnMessageRequirements ()
 * Sort a set of routes based on the resource requirements a message
 * has on the links. Routes are sorted from least used route to
 * heaviest used route.
 */
void KnowledgeNoCScheduler::sortRoutesOnMessageRequirements(Message *m,
                                                            LinksReqs &linksReqs,
                                                            Routes &routes) {
    // Set the cost of each route
    for (auto &r: routes) {
        double cost = 0;

        // Cost of a route is the sum of the link requirements (max within time
        // period of message)
        for (auto iterL = r.linksBegin(); iterL != r.linksEnd(); iterL++) {
            Link *l = *iterL;

            cost += costLinkForMessage(m, linksReqs[l->getId()]);
        }

        r.setCost(cost);
    }

    // Sort the routes
    routes.sort();
}

/**
 * findScheduleEntityForMessageUsingKnowledge ()
 * The function tries to find a schedule entity for the message. On success it
 * returns true and allocates the entity on the NoC. Else it returns false and
 * no resources are claimed.
 */
bool KnowledgeNoCScheduler::findScheduleEntityForMessageUsingKnowledge(Message *m,
                                                                       const CSize maxDetour,
                                                                       LinksReqs &linksReqs) {
    auto e = std::make_shared<NoCSchedulingEntity>(m);

    // NoC properties
    CSize slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // Get src and dst node in the architecture graph
    Node *srcNode = getInterconnectGraph()->getNode(m->getSrcNodeId());
    Node *dstNode = getInterconnectGraph()->getNode(m->getDstNodeId());

    if (srcNode == nullptr || dstNode == nullptr) {
        throw CException("Node used in communication event does not exist in"
                         + CString(" architecture."));
}
    // Compute minimal length between src and dst node
    CSize minLengthRoute = getLengthShortestPathBetweenNodes(srcNode, dstNode);

    // Scheduling entity starts as soon as possible
    e->setStartTime(earliestStartTime(m));

    // Maximal duration when using shortest route
    TTime maxDuration = maximalDuration(m, e->getStartTime(), minLengthRoute);

    // Minimal duration (assuming all slots available)
    SlotReservations slotReservations(static_cast<size_t>(slotTableSize));
    for (uint i = 0; i < slotTableSize; i++) {
        slotReservations[i] = true;
}
    TTime minDuration = minimalDuration(m, e->getStartTime(), slotReservations);

    CSize detour = 0;
    Routes routes;

    do {
        // Cleanup list of routes
        routes.clear();

        // Duration is as long as possible considering length of route
        e->setDuration(maxDuration - detour);

        // Find all routes with the increasing detour and sort them on cost
        findAllRoutes(srcNode, dstNode, detour, true, routes);
        sortRoutesOnMessageRequirements(m, linksReqs, routes);

        // Try all routes in order of decreasing cost
        for (auto &r: routes) {
            // Set the route of the scheduling entity
            auto re = std::make_unique<Route>(r);
            auto* reRef = re.get();
            e->setRoute(std::move(re));

            // Duration is as long as possible considering length of route
            e->setDuration(maxDuration - (reRef->length() - minLengthRoute));

            // Try with decreasing duration to find feasible slot allocation
            while (e->getDuration() >= minDuration) {

                // Found a valid slot assignment?
                if (findSlotsOnRoute(e.get(), slotReservations)) {
                    e->setSlotReservations(slotReservations);

                    // Minimize the duration
                    e->setDuration(minimalDuration(m, e->getStartTime(), slotReservations));

                    // Reserve the resource in the NoC
                    reserveResources(e);

                    // Link scheduling entity to communication event
                    m->setSchedulingEntity(e);

                    // Done
                    return true;
                }

                // Decrease duration and try again
                e->setDuration(e->getDuration() / 2);
            }

            e->setRoute(nullptr);
        }

        // Increase detour
        detour++;
    } while (detour <= maxDetour);

    return false;
}
} // namespace SDF