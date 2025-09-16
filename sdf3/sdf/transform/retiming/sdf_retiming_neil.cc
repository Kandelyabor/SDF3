/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_neil.cc
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


#include "sdf_retiming_neil.h"
#include "sdf/transform/hsdf/hsdf.h"
#include "com_hsdf_ip.h"
#include <algorithm>
#include <memory>

namespace SDF {

// #define _PRINT_TRACE
#define EXT_MAX UINT_MAX

SDFretiming_Neil::SDFretiming_Neil(const TimedSDFgraph& sdfg) :
    SDFretiming(sdfg)

{}

/*feasibleRetiming()
 * reVector is a feasible retiming if this function return true
 *
 * */
bool SDFretiming_Neil::feasibleRetiming(int dip) {

    for (uint i = 1; i <= rg->nrActors(); i++) {

#ifdef _PRINT_TRACE
        printMaxExTimeVector();
#endif

        for (const auto& iter : rg->getActors()) {
            const auto* a = dynamic_cast<const TimedSDFactor*>(iter.get());
            CId aId = a->getId();

            if (exTVector[aId] <= dip) {
                reVector[aId] -= getRetimingStep(a);
            }
        }

#ifdef _PRINT_TRACE
        printRetimingVector();
#endif

        if (!legalRetimed()) {
            return false;
        }
        newIP = computeIterationPeriod();

#ifdef _PRINT_TRACE
        std::cout << "after " << i << "th retiming---newIP=" << newIP << std::endl;
        printRetimingVector();
        std::cout << std::endl;
#endif

        if (newIP <= dip) {
            return true;
        }
    }

    return false;
}

/* getRetimingStep()
 * compute retiming step for actor a
 * */
int SDFretiming_Neil::getRetimingStep(const TimedSDFactor *a) {
    CId aId = a->getId();
    int dE = 0;
    int prdE = 0;
    int temp1 = 0;
    int maxOut = 0;
#ifdef _PRINT_TRACE
    std::cout << a->getName() << ":" << std::endl;

#endif
    // maxOut=max{ceil((q(a)*p(e)-dr(e))/p(e)) | for all e: outgoing edges of a}
    for (const auto& pr : a->getPorts()) {
        dE = static_cast<int>(pr->getChannel()->getInitialTokens());

        if (pr->getTypeAsString() == "out") {
            prdE = static_cast<int>(pr->getRate());
            temp1 = static_cast<int>(ceil(static_cast<double>((q[aId] * prdE) - dE) / prdE));
            maxOut = (std::max)(maxOut, temp1);
#ifdef _PRINT_TRACE
            std::cout << "    ->" << pr->getChannel()->getDstActor()->getName() << ": dE=" << dE
                      << std::endl;

            std::cout << "  ceil((q(a)*p(e)-dr(e))/p(e))=" << temp1 << std::endl;
#endif
        }
    }
#ifdef _PRINT_TRACE
    std::cout << "-----maxOut of " << a->getName() << ":" << maxOut << std::endl;

#endif
    return maxOut;
}

/*computeIterationPeriod()
 *
 *Compute Iteration Period of rg.
 *
 */
int SDFretiming_Neil::computeIterationPeriod() {
    int theIP = 0;

    // transform rg to its equivalent hsdf graph
    auto h = TimedSDFgraph::dynamicCast(transformSDFtoHSDF(*rg));

    MaxExTimeVector exTVectorOfH(h->nrActors(), 0);
    // compute the longest zero delay path execution time for each actor in HSDFG
    exTVectorOfH = comHSDFIP(*h);

    // max of exTVectorOfH is the iteration period of rg
    for (int i : exTVectorOfH) {
        theIP = (std::max)(theIP, i);
    }

    // set the value of exTVector[a] equal to the longest path length of
    // any of its copies in its HSDFG h
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        CString aName = a->getName();
        CId aId = a->getId();

        exTVector[aId] = static_cast<int>(a->getExecutionTime());

        bool noZeroD = true;
        for (const auto& iterH : h->getActors()) {
            const auto* hA = dynamic_cast<TimedSDFactor*>(iterH.get());
            CString aHName = hA->getName();
            CId hAId = hA->getId();
            // if hA is a copy of a
            if (aHName.substr(0, aName.length() + 1) == aName + "_") {
                for (const auto& pr : hA->getPorts()) {
                    uint dE = pr->getChannel()->getInitialTokens();
                    if (dE < 1) {
                        noZeroD = false;
                    }
                }
                exTVector[aId] = (std::max)(exTVector[aId], exTVectorOfH[hAId]);
            }
        }
        // if all copies of a have no zero delay edge, isolated to other actors
        if (noZeroD) {
            exTVector[aId] = EXT_MAX;
        }
    }
    return theIP;
}

std::unique_ptr<TimedSDFgraph> retime_Neil(const TimedSDFgraph& g, uint dip) {
    TimedSDFgraph result;
    SDFretiming_Neil rtm(g);
    rtm.init();
    if (rtm.feasibleRetiming(static_cast<int>(dip))) {
        return rtm.getRetimedSDFG()->clone();
    }
    return nullptr;
}

} // namespace SDF
