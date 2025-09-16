/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming.cc
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
#include <algorithm>
#include <memory>

namespace SDF {

// #define _PRINT_TRACE

// NOLINTBEGIN(*global-variables)

namespace RETIMING_GLOBALS {

// NOLINTBEGIN(*internal-linkage)

//-------------------------
std::vector<std::vector<int>> exTemp;
//-------------------------
RetimingVector optRe;

// NOLINTEND(*internal-linkage)

} // namespace RETIMING_GLOBALS

namespace {


MaxExTimeVector tempT;
EofZDVector eofZD;

// NOLINTEND(*global-variables)

} // namespace

// SDFactors eofZDactors;
SDFretiming::SDFretiming(const TimedSDFgraph& sdfg) : g(sdfg), maxExTimeOfActor(0), sumQ(0), rg(nullptr), newIP(0) {

    q = RepetitionVector(g.nrActors(), 0);
    q = computeRepetitionVector(g);

    // Initialize maxExTimeOfActor and sumQ
    for (const auto& iter : g.getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        int exTimeOfA = static_cast<int>(a->getDiscreteExecutionTime());
        maxExTimeOfActor = (std::max)(maxExTimeOfActor, exTimeOfA);
        sumQ += q[a->getId()];
    }
    // Clone a graph for transformation
    rg = g.clone();
    rg->setName("re_" + g.getName());

    reVector = RetimingVector(g.nrActors(), 0);
    exTVector = MaxExTimeVector(g.nrActors(), 0);
}

SDFretiming::~SDFretiming() {

    q.clear();
    reVector.clear();
    exTVector.clear();
}

/*init()
 * Initialize variables
 * */
void SDFretiming::init() {
    newIP = 0;

    // Initialize reVector
    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        reVector[a->getId()] = 0;
    }
    // Initialize rg, the copy of g
    for (const auto& iter : rg->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());
        c->setInitialTokens(g.getChannel(c->getId())->getInitialTokens());
    }

    eofZD = EofZDVector(g.nrActors(), false);
}
/*isRetimable(uint dip)
 * This is a template method. The methods it called may be overridden by the derived class
 *
 * */
bool SDFretiming::isRetimable(int dip) {

    init();

    // if dip less than the max actor execution time
    if (dip < maxExTimeOfActor) {
        return false;
    }

    newIP = computeIterationPeriod();

    // if current ip is not larger than the dip
    if (newIP <= dip) {
        return true;
    }

    return feasibleRetiming(dip);
}

bool SDFretiming::feasibleRetiming(int dip) {

    for (uint i = 0; i <= sumQ; i++) {

        for (const auto& iter : rg->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            int aID = static_cast<int>(a->getId());

            if (eofZD[aID] && exTVector[aID] > dip) {
                rePrev(a, dip);
            }
        }
        // perform retiming transformation on g
        if (!legalRetimed()) {
            return false;
        }
        newIP = computeIterationPeriod();

        if (newIP <= dip) {
            return true;
        }
    }
    return false;
}

/* retimingTransformation()
 *
 * return the retimed graph according current retiming vector reVector
 * It should be called after isRetimable()
 * */

TimedSDFgraph* SDFretiming::getRetimedSDFG() { return rg.get(); }
/* printFeasibleRetimingVector()
 * print current retiming vector and delay distribution
 * It should be called after isRetimable()
 * */

/* retimingTransformation()
 *
 * return the retimed graph according current retiming vector reVector
 * Hands over ownership of the object and invalidates the graph internally.
 * It should be called after isRetimable()
 * */

std::unique_ptr<TimedSDFgraph> SDFretiming::takeRetimedSDFG() { return std::move(rg); }

/* printFeasibleRetimingVector()
 * print current retiming vector and delay distribution
 * It should be called after isRetimable()
 * */

void SDFretiming::printFeasibleRetimingVector() {

    std::cout << "======print Feasible Retiming Vector& New Delay Vector======" << std::endl;
    printRetimingVector();
    std::cout << "New IP=" << newIP << std::endl;
    printDelayVector();
}

/* legalRetimed()
 * Compute new delay distribution according to current retiming vector reVector.
 * This function makes rg a retimed graph according reVector
 * */

bool SDFretiming::legalRetimed() {

    const auto& channels = rg->getChannels();
    return std::all_of(channels.begin(), channels.end(), [this](const auto& ch){
        auto* c = dynamic_cast<TimedSDFchannel*>(ch.get());

        CId cID = c->getId();
        CId srcActorID = c->getSrcActor()->getId();
        CId dstActorID = c->getDstActor()->getId();
        SDFrate srcPortRate = c->getSrcPort()->getRate();
        SDFrate dstPortRate = c->getDstPort()->getRate();

        // oldDelay is the delay in g, rather than rg
        uint oldDelay = g.getChannel(cID)->getInitialTokens();

        int newDelay =
                static_cast<int>(oldDelay) + (static_cast<int>(dstPortRate) * reVector[dstActorID]) - (static_cast<int>(srcPortRate) * reVector[srcActorID]);
        if (newDelay >= 0) {
            c->setInitialTokens(newDelay);
        } else {
            c->setInitialTokens(0);
            return false;
        }
        return true;
    });
}

/*computeIterationPeriod()
 *
 *Compute Iteration Period of rg.
 *This function is an implementation of algorithm IP in date paper
 */
int SDFretiming::computeIterationPeriod() {
    int theIP = 0;
    SDFactorConstRefs initActors;
    tempT = MaxExTimeVector(g.nrActors(), 0);
    ////////////////////////////
    //  eofZDactors.clear();
    //////////////////////
    // initialize  pathTime & find V0 (initActors)
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        int i = static_cast<int>(a->getId());
        exTVector[i] = static_cast<int>(a->getDiscreteExecutionTime());
        tempT[i] = static_cast<int>(a->getDiscreteExecutionTime());
        eofZD[i] = false;

        bool isInitActor = true;
        for (const auto& pr : a->getPorts()) {
            if (pr->getTypeAsString() == "in") {
                uint cnsE = pr->getRate();
                uint dE = pr->getChannel()->getInitialTokens();
                if (dE < cnsE) {
                    isInitActor = false;
                }
            }
        }
        // if for all incoming edges of a, there are dE>=cns, then a is an initial actor
        if (isInitActor) {
            initActors.push_back(a);
        }
    }

    // begin to compute IP by dfs
    for (const auto& iter : initActors) {
        const auto* a = dynamic_cast<const TimedSDFactor*>(iter);
        int i = static_cast<int>(a->getId());
        tempT[i] = static_cast<int>(a->getDiscreteExecutionTime());
        getNextT(a, 1);
    }

    for (int i : exTVector) {
        theIP = (std::max)(theIP, i);
    }
    return theIP;
}
/*getNextT()
 * find the longest zero delay path from (actor,l)
 * */
void SDFretiming::getNextT(const TimedSDFactor *actor, int l) {
    bool isEof = true;
    // currT store current level
    int currT = tempT[actor->getId()];
    int aID = static_cast<int>(actor->getId());

    // for all outgoing edges of actor
    for (const auto& pr : actor->getPorts()) {
        if (pr->getTypeAsString() == "out") {
            uint dE = pr->getChannel()->getInitialTokens();
            uint prdE = pr->getRate();
            uint cnsE = pr->getChannel()->getDstPort()->getRate();
            auto *nextActor = dynamic_cast<TimedSDFactor *>(pr->getChannel()->getDstActor());
            int nextAID = static_cast<int>(nextActor->getId());

            // @TODO MG redundant type conversions and floor?
            int nextL =
                    static_cast<int>(floor((static_cast<float>(((l - 1) * prdE) + dE) / static_cast<float>(cnsE)))) + 1;

            if (nextL <= q[nextAID]) {
                isEof = false;

                exTVector[nextAID] = (std::max)(exTVector[nextAID], tempT[aID] + static_cast<int>(nextActor->getDiscreteExecutionTime()));
                tempT[nextAID] = tempT[aID] + static_cast<int>(nextActor->getDiscreteExecutionTime());

                getNextT(nextActor, nextL);
            }
            tempT[aID] = currT;
        }
    }
    // if all actor's successors have nextL>q[nextAID]), actor is an end of zero delay path
    if (isEof) {
        eofZD[aID] = true;
        //      eofZDactors.push_back(actor);
    }
}

/* rePrev()
 * check actors backward to increase the retiming vector if only possible
 * It is the implementation of algorithm rePrev in date paper
 * */
void SDFretiming::rePrev(const TimedSDFactor *a, int dip) {

    uint prdE = 0;
    uint cnsE = 0;
#ifdef _DEBUG_RETIMING_RETIMING
    std::cout << count++ << "----Actor:  " << a->getName() << std::endl;
#endif
    int rStep = getRetimingStep(a);
    if (rStep == 0) {
        return;
    }
    reVector[a->getId()] += rStep;
    // change the delays on the incoming and outgoing edges of a
    for (const auto& pr : a->getPorts()) {
        uint oldDofE = 0;
        oldDofE = pr->getChannel()->getInitialTokens();
        uint newDofE = 0;
        if (pr->getTypeAsString() == "in") {

            cnsE = pr->getRate();
            newDofE = oldDofE + rStep * cnsE;
            pr->getChannel()->setInitialTokens(newDofE);
#ifdef _DEBUG_RETIMING
            std::cout << "        incoming edge: " << pr->getChannel()->getName() << "--new delay("
                      << oldDofE << "+" << rStep << "*" << cnsE
                      << ")=" << pr->getChannel()->getInitialTokens() << std::endl;
#endif
        }
        if (pr->getTypeAsString() == "out") {

            prdE = pr->getRate();
            newDofE = oldDofE - rStep * prdE;
            pr->getChannel()->setInitialTokens(newDofE);
#ifdef _DEBUG_RETIMING
            std::cout << "        outgoing edge: " << pr->getChannel()->getName()
                      << "--new delay(dE-prdE)=" << pr->getChannel()->getInitialTokens()
                      << std::endl;
#endif
        }
    }
#ifdef _DEBUG_RETIMING
    getchar();
#endif
    // for all incoming edges of actor
    for (const auto& prIN : a->getPorts()) {
        if (prIN->getTypeAsString() == "in") {

            //          cnsE=prIN->getRate();

            auto *inA = dynamic_cast<TimedSDFchannel *>(prIN->getChannel());
            auto *srcA = dynamic_cast<TimedSDFactor *>(inA->getSrcActor());

            //      prdE=inA->getSrcPort()->getRate();
            uint srcId = srcA->getId();
#ifdef _DEBUG_RETIMING
            std::cout << total << "-" << count << "  ==incoming edge of " << a->getName() << ":"
                      << srcA->getName() << "---" << inA->getName() << "---"
                      << inA->getInitialTokens() << "--->" << a->getName() << std::endl;
#endif
            if (exTVector[srcId] > dip && !eofZD[srcId]) {
                //  if(exTVector[srcId]>dip && !eofZD[srcId] &&
                //  prdE<=(cnsE+prIN->getChannel()->getInitialTokens())){
                bool canIncrease = true;
                // for all outgoing edges of srcA
                for (const auto& prOUT : srcA->getPorts()) {
                    if (prOUT->getTypeAsString() == "out") {
                        // if(inId!=prOUT->getChannel()->getId()){

                        prdE = prOUT->getRate();
#ifdef _DEBUG_RETIMING
                        std::cout << total << "-" << count << "==out edge of " << srcA->getName()
                                  << ":" << prOUT->getChannel()->getName()
                                  << "--d=" << prOUT->getChannel()->getInitialTokens()
                                  << ",prd=" << prdE << "--" << std::endl;
#endif
                        if (prOUT->getChannel()->getInitialTokens() < prdE) {
#ifdef _DEBUG_RETIMING
                            std::cout << total << "-" << count << "--- " << srcA->getName()
                                      << " false " << a->getName() << std::endl;
#endif
                            canIncrease = false;
                        }
                        //}
                    }
                }
                if (canIncrease) {
#ifdef _DEBUG_RETIMING
                    std::cout << total << "-" << count << "--- " << a->getName() << " canIncrease"
                              << std::endl;
#endif
                    rePrev(srcA, dip);
                }
            }
        }
    }
}

void SDFretiming::printRetimingVector() {

    std::cout << std::endl;

    std::cout << "Retiming Vector :(";
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        std::cout << reVector[a->getId()] << ",";
    }
    std::cout << ")" << std::endl;
}

void SDFretiming::printMaxExTimeVector() {
    std::cout << "max Execution Vector :(";
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        std::cout << a->getName() << "(" << exTVector[a->getId()] << "),";
    }
    std::cout << ")" << std::endl;
}
void SDFretiming::printDelayVector() const {

    std::cout << "Delay distribution:(";
    for (const auto& iter : rg->getChannels()) {
        const auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());
        std::cout << c->getInitialTokens() << ",";
    }
    std::cout << ")" << std::endl;
}

namespace {

std::unique_ptr<TimedSDFgraph> retime(const TimedSDFgraph& g, uint dip) {
    TimedSDFgraph result;
    SDFretiming rtm(g);
    rtm.init();
    if (rtm.feasibleRetiming(static_cast<int>(dip))) {
        return rtm.takeRetimedSDFG();
    }
    return nullptr;
}

} // namespace

// RetimingVector retimingVector(TimedSDFgraph *g) {
//     RetimingVector res;
//     return res;
// }

} // namespace SDF
