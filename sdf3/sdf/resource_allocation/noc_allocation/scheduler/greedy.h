/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   greedy.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Greedy NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: greedy.h,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_GREEDY_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_GREEDY_H_INCLUDED

#include "noc_scheduler.h"
namespace SDF {
/**
 * GreedyNoCScheduler ()
 * Greedy NoC scheduling algorithm
 */
class GreedyNoCScheduler : public NoCScheduler {
public:
    // Constructor
    explicit GreedyNoCScheduler(const CSize maxDetour) : maxDetour(maxDetour){};

    // Destructor
    ~GreedyNoCScheduler() override = default;

    // Prevent copying and moving
    GreedyNoCScheduler(const GreedyNoCScheduler&) = delete;
    GreedyNoCScheduler& operator=(const GreedyNoCScheduler&) = delete;
    GreedyNoCScheduler(GreedyNoCScheduler&&) = delete;
    GreedyNoCScheduler& operator=(GreedyNoCScheduler&&) = delete;

    // Schedule function
    bool solve() override {
        bool success = greedy(maxDetour);
        return getSchedulingProblem()->setSolvedFlag(success);
    };

private:
    // Greedy schedule function
    bool greedy(CSize maxDetour);

    CSize maxDetour;
};
} // namespace SDF
#endif
