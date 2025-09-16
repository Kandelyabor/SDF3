/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_by_hsdf.cc
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


#include "sdf_retiming_by_hsdf.h"
#include "hsdf_retiming.h"
#include "sdf/transform/hsdf/hsdf.h"
#include <memory>

namespace SDF {

// #define _PRINT_TRACE
#define EXT_MAX UINT_MAX

SDFretiming_byHSDF::SDFretiming_byHSDF(const TimedSDFgraph& sdfg) :
    SDFretiming(sdfg) {
        h = (TimedSDFgraph::dynamicCast(transformSDFtoHSDF(sdfg)));
    }

SDFretiming_byHSDF::~SDFretiming_byHSDF() = default;
/*isRetimable(uint dip)
 * This is a template method. The methods it called may be overridden by the derived class
 *
 * */

bool SDFretiming_byHSDF::isRetimable(int dip) {

    init();

    //   std::cout << " Retiming ( SDFretiming_byHSDF )"<<std::endl;

    HSDFretiming hsdfR(*h);
    // if dip less than the max actor execution time
    if (dip < maxExTimeOfActor) {
        return false;
    }

    if (hsdfR.isRetimable(dip)) {
        reVectorOfH = RetimingVector(h->nrActors(), 0);
        reVectorOfH = hsdfR.getRetimingVector();
        newIP = static_cast<int>(hsdfR.getCurrIP());
        return true;
    }
    return false;
}

/*feasibleRetiming()
 * reVector is a feasible retiming if this function return true
 *
 * */
bool SDFretiming_byHSDF::feasibleRetiming(int  /*dip*/) { return true; }

void SDFretiming_byHSDF::computeRetimingVector() {

    for (auto *iter : g.getActorRefs()) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);
        CString aName = a->getName();
        uint aId = a->getId();

        for (auto *iterH : h->getActorRefs()) {
            auto *hA = dynamic_cast<TimedSDFactor *>(iterH);
            CString aHName = hA->getName();
            uint hAId = hA->getId();
            // if hA is a copy of a
            if (aHName.substr(0, aName.length() + 1) == aName + "_") {
                reVector[aId] += reVectorOfH[hAId];
            }
        }
    }
    /*   std::cout<<std::endl;
         std::cout<<"reVectorOfH:";
         for(SDFactorsIter iterH = h->actorsBegin();iterH != h->actorsEnd();iterH++){
                    TimedSDFactor *hA = (TimedSDFactor*)(*iterH);
                    CString aHName=hA->getName();
                    uint hAId=hA->getId();
                    std::cout<<"("<<aHName<<","<<reVectorOfH[hAId]<<"),";
         }
         std::cout<<std::endl;
    */
}
/* retimingTransformation()
 *
 * return the retimed graph according current retiming vector reVector
 * It should be called after isRetimable()
 * */

TimedSDFgraph* SDFretiming_byHSDF::getRetimedSDFG() {

    if (legalRetimed()) {
        return rg.get();
    }
    return nullptr;
}

void SDFretiming_byHSDF::printFeasibleRetimingVector() {

    std::cout << "======print Feasible Retiming Vector======" << std::endl;
    computeRetimingVector();
    if (!legalRetimed()) {
        std::cout << "Illegal retimed!!" << std::endl;
        return;
    }

    std::cout << "Retiming Vector :(";
    for (auto *iter: rg->getActorRefs()) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);
        std::cout << reVector[a->getId()] << ",";
    }
    std::cout << ")" << std::endl;

    std::cout << "Delay distribution:(";
    for (auto *iter: rg->getChannelRefs()) {
        auto *c = dynamic_cast<TimedSDFchannel *>(iter);
        std::cout << c->getInitialTokens() << ",";
    }
    std::cout << ")" << std::endl;

    std::cout << "New IP=" << newIP << std::endl;
}

} // namespace SDF
