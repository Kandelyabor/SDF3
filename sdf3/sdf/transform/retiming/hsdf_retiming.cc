/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf_retiming.cc
 *
 *  Author          :   Xue-Yang Zhu
 *
 *  Date            :   2012
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


#include "hsdf_retiming.h"
#include "com_hsdf_ip.h"
#include "hsdf_top_sort.h"
#include <algorithm>


namespace SDF {

// #define _PRINT_TRACE

#define EXT_MAX UINT_MAX

HSDFretiming::HSDFretiming(const TimedSDFgraph& hsdfg) : SDFretiming(hsdfg) {}

/*feasibleRetiming()
 * reVector is a feasible retiming if this function return true
 *
 * */
bool HSDFretiming::feasibleRetiming(int dip) {

    for (uint i = 1; i < rg->nrActors(); i++) {

        for (const auto& iter : rg->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            CId aId = a->getId();

            if (exTVector[aId] > dip) {
                reVector[aId]++;
            }
        }
        if (!legalRetimed()) {
            return false;
        }
        newIP = computeIterationPeriod();

#ifdef _PRINT_TRACE
        std::cout << "HSDFretiming after " << i << "th retiming---newIP=" << newIP << std::endl;
        printRetimingVector();
        std::cout << std::endl;
#endif

        if (newIP <= dip) {
            return true;
        }
    }

    return false;
}

/*computeIterationPeriod()
 *
 *Compute Iteration Period of rg.
 *
 */
int HSDFretiming::computeIterationPeriod() {
    int theIP = 0;

    // compute the longest zero delay path execution time for each actor in HSDFG
    exTVector = comHSDFIP(*rg);

    // max of exTVectorOfH is the iteration period of rg
    for (int i : exTVector) {
        theIP = (std::max)(theIP, i);
    }

    return theIP;
}

} // namespace SDF
