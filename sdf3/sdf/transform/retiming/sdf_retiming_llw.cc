/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_llw.cc
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


#include "sdf_retiming.h"
#include "sdf_retiming_llw.h"
#include <algorithm>
#include <memory>

namespace SDF {

// #define _PRINT_TRACE

using namespace RETIMING_GLOBALS;

namespace {


TimedSDFactor *maxA; // NOLINT(*global-variables)

} // namespace


SDFretiming_LLW::SDFretiming_LLW(const TimedSDFgraph& sdfg) :
    SDFretiming(sdfg)

{
    exTemp = std::vector<std::vector<int>>(rg->nrActors(), std::vector<int>(0));
    for (auto *iter : rg->getActorRefs()) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);
        int i = static_cast<int>(a->getId());
        exTemp[i] = std::vector<int>(q[i] + 1, -1);
    }
}

bool SDFretiming_LLW::feasibleRetiming(int dip) {
    SDFactorRefs Q1;
    SDFchannelRefs Q2;

    Q1.push_back(maxA);
    Q2.clear();

    int numActors = static_cast<int>(rg->nrActors());
    int notAllQ = numActors;
    bool ex2QV = false;

    while (notAllQ != 0 && !ex2QV) {
        SDFchannel *e = nullptr;
        CId cID = 0;
        CId srcActorID = 0;
        CId dstActorID = 0;
        SDFrate srcPortRate = 0;
        SDFrate dstPortRate = 0;
        int oriDE = 0;
        int newDE = 0;
        notAllQ = numActors;

        while (!Q1.empty()) {
            maxA = dynamic_cast<TimedSDFactor *>(Q1.front());
            Q1.pop_front();

            reVector[maxA->getId()] += 1;
            for (const auto& pr : maxA->getPorts()) {
                if (pr->getTypeAsString() == "out") {
                    e = pr->getChannel();
                    cID = e->getId();
                    srcActorID = maxA->getId();
                    dstActorID = e->getDstActor()->getId();
                    oriDE = static_cast<int>(g.getChannel(cID)->getInitialTokens());
                    srcPortRate = pr->getRate();
                    dstPortRate = e->getDstPort()->getRate();
                    newDE = oriDE + static_cast<int>(dstPortRate) * reVector[dstActorID]
                            - static_cast<int>(srcPortRate) * reVector[srcActorID];
                    if (newDE < 0) {
                        Q2.push_back(e);
                    }
                }
            }
        }

        while (!Q2.empty()) {
            SDFchannel *errE = Q2.front();
            Q2.pop_front();

            cID = errE->getId();
            srcActorID = errE->getSrcActor()->getId();
            dstActorID = errE->getDstActor()->getId();
            oriDE = static_cast<int>(g.getChannel(cID)->getInitialTokens());
            srcPortRate = errE->getSrcPort()->getRate();
            dstPortRate = errE->getDstPort()->getRate();
            int deltaR = static_cast<int>(ceil(
                                 (static_cast<double>(srcPortRate) * reVector[srcActorID] - oriDE)
                                 / dstPortRate))
                         - reVector[dstActorID];
            if (deltaR > 0) {
                reVector[dstActorID] += deltaR;
                maxA = dynamic_cast<TimedSDFactor *>(errE->getDstActor());
                for (const auto& pr : maxA->getPorts()) {
                    if (pr->getTypeAsString() == "out") {
                        e = pr->getChannel();
                        cID = e->getId();
                        srcActorID = maxA->getId();
                        dstActorID = e->getDstActor()->getId();
                        oriDE = static_cast<int>(g.getChannel(cID)->getInitialTokens());
                        srcPortRate = pr->getRate();
                        dstPortRate = e->getDstPort()->getRate();
                        newDE = oriDE + static_cast<int>(dstPortRate) * reVector[dstActorID]
                                - static_cast<int>(srcPortRate) * reVector[srcActorID];
                        if (newDE < 0) {
                            Q2.push_back(e);
                        }
                    }
                }
            }
        }

        if (legalRetimed()) {
            newIP = computeIterationPeriod();

            if (newIP <= dip) {
                return true;
            }

            for (auto *iter : rg->getActorRefs()) {
                auto *a = dynamic_cast<TimedSDFactor *>(iter);
                uint aId = a->getId();
                if (reVector[aId] >= q[aId]) {
                    notAllQ--;
                }
                if (reVector[aId] > 2 * q[aId] * numActors) {
                    ex2QV = true;
                }
                if ((static_cast<uint>(exTemp[aId][q[aId]])) + a->getDiscreteExecutionTime()
                    > static_cast<uint>(dip)) {
                    Q1.push_back(a);
                }
            }
        } else {
            std::cout << "illegal retime" << std::endl;
            return false;
        }
    }

    return false;
}

/*
 * can't be used in a graph that is not strong connected
 * */
int SDFretiming_LLW::computeIterationPeriod() {
    int theIP = 0;

    // initialize
    init_t();

    for (auto *iter : rg->getActorRefs()) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);
        int i = static_cast<int>(a->getId());
        //          std::cout<<"0:exTemp["<<a->getName()<<"]["<<q[i]<<"]:"<<exTemp[i][q[i]]<<std::endl;
        exTemp[i][q[i]] = get_t(a, q[i]);
        //          exTVector[i]=exTemp[i][q[i]]+a->getDiscreteExecutionTime();

        if ((static_cast<uint>(theIP))
            < (static_cast<uint>(exTemp[i][q[i]])) + a->getDiscreteExecutionTime()) {
            theIP = exTemp[i][q[i]] + static_cast<int>(a->getDiscreteExecutionTime());
            maxA = a;
        }
    }

    return theIP;
}

void SDFretiming_LLW::init_t() {
    for (auto *iter : rg->getActorRefs()) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);
        int i = static_cast<int>(a->getId());
        for (int j = 1; j < q[i] + 1; j++) {
            exTemp[i][j] = -1;
        }
    }
}

int SDFretiming_LLW::get_t(TimedSDFactor *a, int k) {
    int i = static_cast<int>(a->getId());
    //  std::cout<<"getExOfHSDF:exTemp["<<a->getName()<<"]["<<k<<"]:"<<exTemp[i][k]<<std::endl;
    if (k < 1) {
        return -1 * static_cast<int>(a->getDiscreteExecutionTime());
    }
    if (exTemp[i][k] != -1) {
        return exTemp[i][k];
    }

    int max_t = -1;

    for (const auto& pr : a->getPorts()) {
        if (pr->getTypeAsString() == "in") {
            int cnsE = static_cast<int>(pr->getRate());
            int prdE = static_cast<int>(pr->getChannel()->getSrcPort()->getRate());
            int dE = static_cast<int>(pr->getChannel()->getInitialTokens());

            auto *srcA = dynamic_cast<TimedSDFactor *>(pr->getChannel()->getSrcActor());

            int l = static_cast<int>(ceil((static_cast<double>(k) * cnsE - dE) / prdE));

            int t1 = static_cast<int>(get_t(srcA, l) + srcA->getDiscreteExecutionTime());
            max_t = (std::max)(max_t, t1);
        }
    }
    exTemp[i][k] = max_t;

    return exTemp[i][k];
}

int SDFretiming_LLW::optimalRetiming() {
    SDFactorRefs Q1;
    SDFchannelRefs Q2;

    init();
    optRe = RetimingVector(g.nrActors(), 0);
    int TStep = computeIterationPeriod();

    Q1.push_back(maxA);
    Q2.clear();

    int numActors = static_cast<int>(rg->nrActors());
    int notAllQ = numActors;
    bool ex2QV = false;

    //-----------------
    // uint counter=0;
    //-----------------

    while (!ex2QV && notAllQ != 0) {
        SDFchannel *e = nullptr;
        CId cID = 0;
        CId srcActorID = 0;
        CId dstActorID = 0;
        SDFrate srcPortRate = 0;
        SDFrate dstPortRate = 0;
        int oriDE = 0;
        int newDE = 0;
        notAllQ = numActors;
        //-----------------
        // counter++;
        //-----------------

        while (!Q1.empty()) {
            maxA = dynamic_cast<TimedSDFactor *>(Q1.front());
            Q1.pop_front();
            //-----------------
            //  std::cout<<counter<<":"<<maxA->getName()<<","<<Q1.size()<<","<<Q2.size()<<std::endl;
            //-----------------
            reVector[maxA->getId()] += 1;
            for (const auto& pr : maxA->getPorts()) {
                if (pr->getTypeAsString() == "out") {
                    e = pr->getChannel();
                    cID = e->getId();
                    srcActorID = maxA->getId();
                    dstActorID = e->getDstActor()->getId();
                    oriDE = static_cast<int>(g.getChannel(cID)->getInitialTokens());
                    srcPortRate = pr->getRate();
                    dstPortRate = e->getDstPort()->getRate();
                    newDE = oriDE + static_cast<int>(dstPortRate) * reVector[dstActorID]
                            - static_cast<int>(srcPortRate) * reVector[srcActorID];
                    if (newDE < 0) {
                        Q2.push_back(e);
                    }
                }
            }
        }

        while (!Q2.empty()) {
            SDFchannel *errE = Q2.front();
            Q2.pop_front();

            cID = errE->getId();
            srcActorID = errE->getSrcActor()->getId();
            dstActorID = errE->getDstActor()->getId();
            oriDE = static_cast<int>(g.getChannel(cID)->getInitialTokens());
            srcPortRate = errE->getSrcPort()->getRate();
            dstPortRate = errE->getDstPort()->getRate();
            int deltaR =
                    static_cast<int>(ceil((static_cast<double>(srcPortRate) * reVector[srcActorID] - oriDE) / dstPortRate))
                    - reVector[dstActorID];
            if (deltaR > 0) {
                reVector[dstActorID] += deltaR;
                maxA = dynamic_cast<TimedSDFactor *>(errE->getDstActor());
                for (const auto& pr: maxA->getPorts()) {
                    if (pr->getTypeAsString() == "out") {
                        e = pr->getChannel();
                        cID = e->getId();
                        srcActorID = maxA->getId();
                        dstActorID = e->getDstActor()->getId();
                        oriDE = static_cast<int>(g.getChannel(cID)->getInitialTokens());
                        srcPortRate = pr->getRate();
                        dstPortRate = e->getDstPort()->getRate();
                        newDE = oriDE + static_cast<int>(dstPortRate) * reVector[dstActorID]
                                - static_cast<int>(srcPortRate) * reVector[srcActorID];
                        if (newDE < 0) {
                            Q2.push_back(e);
}
                    }
                }
            }
        }

        if (legalRetimed()) {
            newIP = computeIterationPeriod();
            for (auto *iter: rg->getActorRefs()) {
                auto *a = dynamic_cast<TimedSDFactor *>(iter);
                uint aId = a->getId();
                if (reVector[aId] > 2 * q[aId] * numActors) {
                    ex2QV = true;
                    break;
                }
                if (reVector[aId] >= q[aId]) {
                    notAllQ--;
}

                if ((static_cast<uint>(exTemp[aId][q[aId]])) + a->getDiscreteExecutionTime() >= static_cast<uint>(TStep)) {
                    Q1.push_back(a);
}
            }
        } else {
            std::cout << "illegal retime" << std::endl;
            return 0;
        }

        if (newIP < TStep) {
            TStep = newIP;
            Q1.push_back(maxA);

            for (auto *iter: rg->getActorRefs()) {
                auto *a = dynamic_cast<TimedSDFactor *>(iter);
                uint aId = a->getId();
                optRe[aId] = reVector[aId];
            }
        }
    }

    return TStep;
}

void SDFretiming_LLW::printOptReVector() {
    //------------
    std::cout << "optimal Retiming  Vector :(";
    for (auto *iter: rg->getActorRefs()) {
        auto *a = dynamic_cast<TimedSDFactor *>(iter);
        std::cout << optRe[a->getId()] << ",";
    }
    std::cout << ")" << std::endl;

    //------------
}

std::unique_ptr<TimedSDFgraph> retime_LLW(const TimedSDFgraph& g, uint dip) {
    TimedSDFgraph result;
    SDFretiming_LLW rtm(g);
    rtm.init();
    if (rtm.feasibleRetiming(static_cast<int>(dip))) {
        return rtm.getRetimedSDFG()->clone();
    }
    return nullptr;
}

} // namespace SDF
