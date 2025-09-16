/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   random.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Random-based NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: random.cc,v 1.1.2.1 2010-04-25 02:08:52 mgeilen Exp $
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

#include "random.h"
#include <cmath>

namespace SDF {

namespace {

// Random number generator
MTRand mtRand; // NOLINT(*global-variables,cert-err58-cpp)

} // namespace

/**
 * random ()
 * Random-based NoC scheduling algorithm
 */
bool RandomNoCScheduler::random(const CSize maxDetour,
                                const uint maxNrRipups,
                                const uint maxNrTries) {
    uint nrRipups = 0;
    uint nrTries = 0;
    bool found = false;
    bool solved = false;

    while (nrTries < maxNrTries) {
        // Assume problem solved till proven otherwise
        solved = true;

        // Put messages in a random order
        putMessagesInRandomOrder();

        // Iterate over sorted messages
        for (auto iter = messagesBegin(); iter != messagesEnd(); iter++) {
            Message *m = (*iter).get();

            // Try to find a scheduling entity for the message
            found = findScheduleEntityForMessageUsingRandom(m, maxDetour);

            // Failed to find a scheduling entity?
            if (!found && nrRipups < maxNrRipups && iter != messagesBegin()) {
                // Ripup some already scheduled message
                ripupScheduleEntity(iter);
                nrRipups++;

                // Re-try current message
                iter--;
            } else if (!found) {
                // Failed to schedule all messages
                solved = false;
                break;
            }
        }

        // All messages scheduled?
        if (solved) {
            return true;
        }

        // Next attempt
        nrTries++;

        // Unschedule already scheduled messages
        for (auto iter = messagesBegin(); iter != iter; iter++) {
            Message& m = *(*iter);
            releaseResources(m.getSchedulingEntity());
        }
    }

    // Output slot tables on all links
    // print(std::cerr);
    std::cerr << "Failed finding scheduling entity for all message. " << std::endl;

    // Failed to schedule all messages
    return false;
}

/**
 * putMessagesInRandomOrder ()
 * The function puts all messages in a random order by first assigning a
 * random cost to each messages and then sorting them using this cost.
 */
void RandomNoCScheduler::putMessagesInRandomOrder() {
    double cost = NAN;

    // Assign costs to all messages
    for (auto iter = messagesBegin(); iter != messagesEnd(); iter++) {
        Message& m = *(*iter);

        cost = mtRand.rand();
        m.setCost(cost);
    }

    // Sort the messages
    getSchedulingProblem()->getMessages().sort(MessageLess());
}

/**
 * putRoutesInRandomOrder ()
 * The function puts all routes in a random order by first assigning a
 * random cost to each route and then sorting them using this cost.
 */
void RandomNoCScheduler::putRoutesInRandomOrder(Routes &routes) {
    // Assign costs to all routes
    for (auto iter = routes.begin(); iter != routes.end(); iter++) {
        Route &r = *iter;

        double cost = mtRand.rand();
        r.setCost(cost);
    }

    // Sort the routes
    routes.sort();
}

/**
 * findScheduleEntityForMessageUsingRandom ()
 * The function tries to find a schedule entity for the message. For this it
 * uses random choices on the start time, duration and route. On success it
 * returns true and allocates the entity on the NoC. Else it returns false and
 * no resources are claimed.
 */
bool RandomNoCScheduler::findScheduleEntityForMessageUsingRandom(Message *m,
                                                                 const CSize /*maxDetour*/) {

    // Get src and dst node in the architecture graph
    Node *srcNode = getInterconnectGraph()->getNode(m->getSrcNodeId());
    Node *dstNode = getInterconnectGraph()->getNode(m->getDstNodeId());

    if (srcNode == nullptr || dstNode == nullptr) {
        throw CException("Node used in communication event does not exist in"
                         + CString(" architecture."));
    }

    // Slot table properties
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    TTime slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // Compute minimal length between src and dst node
    CSize minLengthRoute = getLengthShortestPathBetweenNodes(srcNode, dstNode);

    // Minimal and maximal duration when using shortest route
    TTime startTime = earliestStartTime(m);
    SlotReservations slotReservations(static_cast<unsigned int>(slotTableSize));
    for (uint i = 0; i < slotTableSize; i++) {
        slotReservations[i] = true;
    }
    TTime minDuration = slotTableSize + minimalDuration(m, 0, slotReservations);
    TTime maxDuration = maximalDuration(m, startTime, minLengthRoute);
    if (minDuration > maxDuration) {
        return false;
    }

    // Choice a random start time between the bounds
    // startTime += mtRand.randInt(maxDuration - minDuration);
    auto e = std::make_shared<NoCSchedulingEntity>(m);
    e->setStartTime(startTime % slotTablePeriod);
    CSize detour = 0;
    Routes routes;

    // Find all routes with at most the specified detour and put them in a
    // random order
    findAllRoutes(srcNode, dstNode, detour, false, routes);
    putRoutesInRandomOrder(routes);

    // Try all routes
    for (auto &r : routes) {
        // Set the route of the scheduling entity
        auto re = std::make_unique<Route>(r);
        e->setRoute(std::move(re));

        // Choice a random duration between the bounds
        detour = re->length() - minLengthRoute;
        if (static_cast<int32_t>(maxDuration) - static_cast<int32_t>(detour)
            <= static_cast<int32_t>(minDuration)) {
            continue;
        }
        TTime duration = mtRand.randInt(static_cast<MTRand::uint32>(
                maxDuration - static_cast<TTime>(detour) - minDuration));
        e->setDuration(duration + minDuration);

        // Found a valid slot assignment?
        if (findSlotsOnRoute(e.get(), slotReservations)) {
            e->setSlotReservations(slotReservations);

            // Enough time for actual communication?
            if (e->getDuration() < minimalDuration(m, e->getStartTime(), slotReservations)) {
                continue;
            }

            // Minimize the duration
            e->setDuration(minimalDuration(m, e->getStartTime(), slotReservations));

            // Reserve the resource in the NoC
            reserveResources(e);

            // Link scheduling entity to communication event
            m->setSchedulingEntity(e);

            // Done
            return true;
        }

        e->setRoute(nullptr);
    }

    return false;
}
} // namespace SDF