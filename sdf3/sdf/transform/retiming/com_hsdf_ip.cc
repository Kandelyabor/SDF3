/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   com_hsdf_ip.cc
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

#include "com_hsdf_ip.h"

#include "hsdf_top_sort.h"
#include <algorithm>


namespace SDF {

/*comHSDFIP()
 * compute max execution time of the longest zero
 *  delay path to each actor in the HSDFG h
 * */
MaxExTimeVector comHSDFIP(const TimedSDFgraph& h) {
    // get topologically sort actors in the hsdf
    SDFactorRefs topSortOfH = hsdfTopSort(h);

    // compute the longest zero delay path execution time for each actor in h,
    // according to the topologically sort
    MaxExTimeVector exTVectorOfH(h.nrActors(), 0);
    for (auto *iter : topSortOfH) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);

        SDFactorRefs zeroPreA; // actors connected to a ,with delay=0
        bool isInitActor = true;
        for (const auto& pr : a->getPorts()) {
            if (pr->getTypeAsString() == "in") {
                uint dE = pr->getChannel()->getInitialTokens();
                if (dE < 1) {
                    isInitActor = false;
                    zeroPreA.push_back(pr->getChannel()->getSrcActor());
                }
            }
        }

        // if each incoming edges of a have dE>=1, a is an initial actor
        if (isInitActor) {
            exTVectorOfH[a->getId()] = static_cast<int>(a->getExecutionTime());

        } else {
            int maxPreExT = 0;
            for (auto *preIter : zeroPreA) {
                auto *preA = dynamic_cast<TimedSDFactor *>(preIter);
                maxPreExT = (std::max)(maxPreExT, exTVectorOfH[preA->getId()]);
            }
            exTVectorOfH[a->getId()] = static_cast<int>(maxPreExT + a->getExecutionTime());
        }
    }
    topSortOfH.clear();
    return exTVectorOfH;
}

} // namespace SDF
