/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf_top_sort.cc
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


#include "hsdf_top_sort.h"

namespace SDF {
// #define _DEBUG_RETIMING

// in degree of actor
using InDegree = std::vector<uint>;

/*hsdfTopSort(SDFgraph *h)
 * Topologically sort actors in the hsdf graph according to their data dependency
 *
 * */
SDFactorRefs hsdfTopSort(const SDFgraph& h) {
    SDFactorRefs topSortOfH;
    SDFactorRefs initActors;
    InDegree inDegree(h.nrActors(), 0);
    // get initial actor set

    for (auto *a : h.getActorRefs()) {
        bool isInitActor = true;
        for (const auto& pr : a->getPorts()) {
            if (pr->getTypeAsString() == "in") {
                uint dE = pr->getChannel()->getInitialTokens();
                if (dE < 1) {
                    isInitActor = false;

                    // count in degree for each actor
                    inDegree[a->getId()]++;
                }
            }
        }
        // if each incoming edges of a have dE>=1, a is an initial actor
        if (isInitActor) {
            initActors.push_back(a);
        }
    }

#ifdef _DEBUG_RETIMING
    std::cout << "inDegree:  ";
    for (uint i = 0; i < inDegree.size(); i++) {
        std::cout << inDegree[i] << ",";
    }
    std::cout << std::endl;
#endif

    uint count = 0;

    while (!initActors.empty()) {
        count++;

        // extract one actor a from the front of initActors
        SDFactor *a = initActors.front();
        initActors.pop_front();

        // add to Top sort
        topSortOfH.push_back(a);

        // search on a's next actors
        for (const auto& pr : a->getPorts()) {
            if (pr->getTypeAsString() == "out") {
                if (pr->getChannel()->getInitialTokens() < 1) {
                    SDFactor *nextA = pr->getChannel()->getDstActor();
                    inDegree[nextA->getId()]--;
                    if (inDegree[nextA->getId()] == 0) {
                        initActors.push_back(nextA);
                    }
                }
            }
        }
    }

    initActors.clear();
    inDegree.clear();
    if (count == h.nrActors()) {
        return topSortOfH;
    }
    std::cout << "There is a cycle in this HSDF" << std::endl;
    return topSortOfH;
}

} // namespace SDF
