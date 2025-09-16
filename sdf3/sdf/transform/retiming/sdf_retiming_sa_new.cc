/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_sa_new.cc
 *
 *  Author          :   X. Y. Zhu
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
#include "sdf_retiming_sa_new.h"
#include <algorithm>
#include <memory>

/*
 * Retiming according to the method described in the paper
 * X. Y. Zhu, T. Basten, M. Geilen and S. Stuijk, "Efficient Retiming of Multirate DSP Algorithms,"
 * in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 31, no. 6,
 * pp. 831-844, June 2012. doi: 10.1109/TCAD.2011.2182352
 *
 * Alg.1: computeIterationPeriod, computeIterationPeriodBFS, getNextT
 * Alg.3: feasibleRetiming, feasibleRetimingSS
 */

namespace SDF {

// #define _PRINT_TRACE
// #define _DEBUG_RETIMING

using namespace RETIMING_GLOBALS;

namespace {

// NOLINTBEGIN(*global-variables)

std::vector<std::vector<uint>> inDegree;

int theIP;

// NOLINTEND(*global-variables)

} // namespace


#ifdef _DEBUG_RETIMING
int count = 0;
#endif

SDFretiming_SAnew::SDFretiming_SAnew(const TimedSDFgraph& sdfg) : SDFretiming(sdfg) {}

void SDFretiming_SAnew::init() {
    // copy from SDFretiming.cc
    newIP = 0;
    //-------------------------
    exTemp = std::vector<std::vector<int>>(rg->nrActors(), std::vector<int>(0));
    curDelayVector = DelayVector(rg->nrChannels(), 0);
    //--------------------------

    // Initialize reVector
    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        reVector[a->getId()] = 0;
    }
    // Initialize rg, the copy of g
    for (const auto& iter : rg->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());
        CId cID = c->getId();
        curDelayVector[cID] = static_cast<int>(g.getChannel(cID)->getInitialTokens());
        c->setInitialTokens(curDelayVector[cID]);
    }
    // End of copy from SDFretiming.cc
}
/*feasibleRetiming()
 * Algorithm 3, sdfFEAS(G, dip) of the TCAD paper
 * reVector is a feasible retiming for dip if this function returns true
 */
bool SDFretiming_SAnew::feasibleRetiming(int dip) {
    // repeat as often as the total sum of the repetition vector (sumQ)
    for (uint i = 0; i < sumQ; i++) {
        // for each actor do ...
        for (const auto& iter : rg->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            uint aId = a->getId();

            // compute retiming step for a
            //  for each of the firings j of a in the iteration
            int retimingStep = 0;
            int j = 1;
            while (retimingStep == 0 && j <= q[aId]) {
                // if firing j is over the desired iteration period
                if (exTemp[aId][j] > dip) {
                    // push the remaining firings to the next iteration, see Equation (5) of the
                    // paper
                    retimingStep = q[aId] - j + 1;
                }
                j++;
            }
            // add the determined retiming step to the retiming vector
            reVector[aId] += retimingStep;
        }
        // compute the retimed graph according to the current retiming vector
        // according to the theory it must be a legal one, no need to check the result...
        legalRetimed();
        // recompute the iteration period with the updated retimed graph
        newIP = computeIterationPeriod();
        // if we have succeeded to obtain the desired iteration period, stop
        if (newIP <= dip) {
            return true;
        }
    }
    // if we did not succeed in sumQ steps, it is not possible
    return false;
}

/* isRetimableSS(uint dip)
 * This is a template method. The methods it called may be overridden by the derived class
 * use feasibleRetimingSS instead of feasibleRetiming
 * */
bool SDFretiming_SAnew::isRetimableSS(int dip) {
    init();

    // if dip less than the max actor execution time, it is not retimable
    if (dip < maxExTimeOfActor) {
        return false;
    }
    // determine the current iteration period
    newIP = computeIterationPeriod();
    // if current ip is not larger than the dip it is trivially retimable
    if (newIP <= dip) {
        return true;
    }

    // check if there is a feasible retiming with sdfFEAS
    if (feasibleRetimingSS(dip)) {
#ifdef _DEBUG_RETIMING
        std::cout << "count FEAS_SS:" << count << std::endl;
#endif
        return true; // NOLINT(*simplify-boolean-expr)
    }
#ifdef _DEBUG_RETIMING
    std::cout << "count FEAS_SS:" << count << std::endl;
#endif
    return false;
}

/* feasibleRetimingSS()
 * Algorithm 3, sdfFEAS(G, dip) of the TCAD paper
 * reVector is a feasible retiming for dip if this function returns true
 * This version includes the sharp termination condition described in Section V.D
 * Note that this function requires that computeIterationPeriod has been called before and exTemp
 * has been set!
 * @TODO: remove this dependency on the call to computeIterationPeriod / better handle it
 */
bool SDFretiming_SAnew::feasibleRetimingSS(int dip) {
    // get the initial delay vector
    DelayVector dv = getCurrentDelayVector();
    // create a collection of delay vectors to detect recurrence in the delay vectors explored (the
    // sharper termination condition)
    DelayVectors DVs;
    // initialize it with the singe current delay vector
    DVs.push_back(dv);
    // hasCycle indicates if a recurrent delay vector was found
    bool hasCycle = false;
    // to efficiently check recurrent vectors keep track of which actors are retimed beyond the
    // first iteration
    std::vector<bool> testR(rg->nrActors(), false);
    // counter to track how many actors remain to go over the first iteration, init to the number of
    // actors
    int allRoverQ = static_cast<int>(rg->nrActors());

    // until the termination condition holds
    while (!hasCycle) {
#ifdef _DEBUG_RETIMING
        count++;
#endif
        // for each actor a
        for (const auto& iter : rg->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            uint aId = a->getId();

            // compute retiming step for a
            //  for each of the firings j of a in the iteration
            int retimingStep = 0;
            int j = 1;
            while (retimingStep == 0 && j <= q[aId]) {
                // if firing j is over the desired iteration period
                if (exTemp[aId][j] > dip) {
                    // push the remaining firings to the next iteration, see Equation (5) of the
                    // paper
                    retimingStep = q[aId] - j + 1;
                }
                j++;
            }
            // add the determined retiming step to the retiming vector
            reVector[aId] += retimingStep;

            // test whether reVector[aId] has been beyond q[aId],
            if (!testR[aId] && reVector[aId] >= q[aId]) {
                testR[aId] = true;
                allRoverQ--;
            }
        }
        // compute the retimed graph according to the current retiming vector
        // according to the theory it must be a legal one, no need to check the result...
        legalRetimed();
        // recompute the iteration period with the updated retimed graph
        newIP = computeIterationPeriod();
        // if we have succeeded to obtain the desired iteration period, stop
        if (newIP <= dip) {
            return true;
        }

        dv = getCurrentDelayVector();
        // only when all reVector[aId]  beyond q[aId], begin to compare delay vectors
        if (allRoverQ == 0) {
            // Find state in the list of stored states
            for (const auto &x : DVs) {
                // State s at position iter in the list?
                if (x == dv) {
                    // a cycle has been found, this will terminate the search
                    hasCycle = true;
                }
            }
        }

        // store state at the end of the list
        if (!hasCycle) {
            DVs.push_back(dv);
        }
    }
    // a feasible retiming does not exist
    return false;
}

/*optimalRetiming()
 *
 * use LLW's outer loop
 * See SDFretiming_LLW::optimalRetiming in SDFretiming_LLW.cc
 */
int SDFretiming_SAnew::optimalRetiming_INS() {
    init();
    // initialize retiming vector to 0
    optRe = RetimingVector(g.nrActors(), 0);
    // keep track of corresponding IP
    int curOptIP = computeIterationPeriod();

    int numActors = static_cast<int>(rg->nrActors());
    // keep track off how many actors have been retimed beyond one iteration (MG: ???)
    // @TODO: initialization is overwritten in while loop, generally suspicious...
    int notAllQ = numActors;
    // check if any retiming is larger than 2*q(v)*|V| for stopping criterion
    bool ex2QV = false;
    //  bool stopIncrease=false;
    //-----------------
    // std::cout<<" SDFretiming_SAnew::optimalRetiming()";
    //-----------------

    //  while(notAllQ!=0 && !ex2QV && !stopIncrease){
    while (!ex2QV && notAllQ != 0) {
        notAllQ = numActors;
        //      stopIncrease=true;

        for (const auto& iter : rg->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
            uint aId = a->getId();
            int retimingStep = 0;
            int j = 1;
            while (retimingStep == 0 && j <= q[aId]) {
                if (exTemp[aId][j] > curOptIP - 1) {
                    retimingStep = q[aId] - j + 1;
                }
                j++;
            }

            reVector[aId] += retimingStep;
        }

        if (legalRetimed()) {
            newIP = computeIterationPeriod();
            if (newIP < curOptIP) {
                curOptIP = newIP;

                for (const auto& iter : rg->getActors()) {
                    const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                    uint aId = a->getId();
                    optRe[aId] = reVector[aId];
                }
            }

            for (const auto& iter : rg->getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                uint aId = a->getId();

                if (reVector[aId] > 2 * q[aId] * numActors) {
                    ex2QV = true;
                    break;
                }
                if (reVector[aId] >= q[aId]) {
                    notAllQ--;
                }
            }
        } else {
            std::cout << "illegal retime" << std::endl;
            return 0;
        }
        //------------
    }
    return curOptIP;
}

/* optimalRetiming_1()
 * compute an optimal retiming of the graph, i.e. with minimal dip
 * curOptIP decrease by one as new dip
 * sdfOPT(), Algorithm 4 in the TCAP paper, with the linear search method
 * see optimalRetiming_BS for the binary search version
 * returns the optimal iteration period
 */
int SDFretiming_SAnew::optimalRetiming_1() {
    init();
    // initialize retiming vector to 0
    optRe = RetimingVector(g.nrActors(), 0);
    // compute the initial iteration period
    int curOptIP = computeIterationPeriod();
    // if the iteration period is equal to the maximum execution time then it must be optimal
    if (curOptIP == maxExTimeOfActor) {
        return curOptIP;
    }
    // try to decrease IP until it is no longer feasible, definitely when it is below the max
    // execution time
    while (curOptIP > maxExTimeOfActor) {

#ifdef _DEBUG_RETIMING
        std::cout << "curOptIP:" << curOptIP << std::endl;
#endif
        // check if a feasible retiming exists for curOptIP -1
        if (feasibleRetiming(curOptIP - 1))
        // if (feasibleRetimingSS(curOptIP - 1))
        {
            // yes, then this is th new optimal IP
            curOptIP = newIP;
            // copy the corresponding vector to optRe
            // MG: would optRE = reVector also work instead of the loop?
            // MG: do we need to copy inside the while loop? Or would once after the loop be enough?
            for (const auto& iter : rg->getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                uint aId = a->getId();
                optRe[aId] = reVector[aId];
            }

        }
        // if it is not feasible we have found the optimum
        else {
            break;
        }
    }
#ifdef _DEBUG_RETIMING
    std::cout << "count OPT:" << count << std::endl;
#endif

    // return the optimal IP
    return curOptIP;
}

/*optimalRetiming_BS()
 *
 *curOptIP decrease by binary search
 *
 */

int SDFretiming_SAnew::optimalRetiming_BS() {

    init();
    optRe = RetimingVector(g.nrActors(), 0);
    int curOptIP = computeIterationPeriod();
    int h = curOptIP - 1;
    int l = maxExTimeOfActor;

    //  if(curOptIP==maxExTimeOfActor)return curOptIP;

    while (h >= l) {
        int m = static_cast<int>(floor((static_cast<float>(l + h) / 2)));

        if (feasibleRetiming(m)) {
            curOptIP = newIP;

            for (const auto& iter : rg->getActors()) {
                const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                uint aId = a->getId();
                optRe[aId] = reVector[aId];
            }
            h = curOptIP - 1;
        } else {
            l = m + 1;
        }
    }
    return curOptIP;
}

/* legalRetimed
 * compute new delay distribution of rg according to current retiming vector
 * it sets the initial tokens on the SDF graph rg
 * it also sets the values in curDelayVector
 * returns a boolean indicating whether the retiming is valid (leaves no channel with a negative
 * number of tokens)
 */
bool SDFretiming_SAnew::legalRetimed() {
    const auto& channels = rg->getChannels();
    return std::all_of(channels.begin(), channels.end(), [this](const auto& ch){
        auto* c = dynamic_cast<TimedSDFchannel*>(ch.get());
        CId cID = c->getId();
        CId srcActorID = c->getSrcActor()->getId();
        CId dstActorID = c->getDstActor()->getId();
        SDFrate srcPortRate = c->getSrcPort()->getRate();
        SDFrate dstPortRate = c->getDstPort()->getRate();

        // oldDelay is the delay in g, rather than rg!
        uint oldDelay = g.getChannel(cID)->getInitialTokens();
        // compute new delay (# initial tokens) for the channel
        curDelayVector[cID] =
                static_cast<int>(oldDelay) + static_cast<int>(dstPortRate) * reVector[dstActorID] - static_cast<int>(srcPortRate) * reVector[srcActorID];
        // if it is a legal (non-negative) value
        if (curDelayVector[cID] >= 0) {
            // set the number of initial tokens
            c->setInitialTokens(curDelayVector[cID]);
        } else {
            // it is illegal, set the number of initial tokens to 0
            c->setInitialTokens(0);
            // return false, the retiming is illegal
            return false;
        }
        return true;
    });
}

/*computeIterationPeriod()
 *
 * Compute Iteration Period of rg.
 * Algorithm 1 sdfIP of TCAD retiming paper
 */
int SDFretiming_SAnew::computeIterationPeriod() {
    // iteration period cannot be smaller than the maximum execution time of an actor
    // theIP is a global variable that holds the computed IP value during the computation
    theIP = maxExTimeOfActor;

    // vActors are the actors that are initially enabled
    SDFactorConstRefs initActors;

    // initialize exTemp (critT) and find initActors (V0 in Alg.1 in the paper)
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        int i = static_cast<int>(a->getId());

        // initialize criT of the algorithm in the paper to -1 for all actor firings
        // exTemp[i] for actor i is vector with an entry for each firing of a in the repetition
        // vector MG: why is there one extra entry in the vector? Because the counting starts
        // from 1...
        exTemp[i] = std::vector<int>(q[i] + 1, -1);

        // check if actor a is enabled in the initial state in the following for loop.
        // initialize the result to true and check if it is false
        bool isInitActor = true;
        // check all input ports of the actor if they have enough tokens
        for (auto iterP = a->portsBegin(); isInitActor && (iterP != a->portsEnd());
             iterP++) {
            const SDFport* pr = iterP->get();
            if (pr->getType() == SDFport::SDFportType::In)
            // if (pr->getTypeAsString() == "in")
            {
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
    // set crit(T) for the first firings of the vActors to their execution times.
    for (const auto& iter : initActors) {
        const auto* a = dynamic_cast<const TimedSDFactor*>(iter);
        // MG: why start at 1 in the exTemp vector instead of 0?
        exTemp[a->getId()][1] = static_cast<int>(a->getDiscreteExecutionTime());
        // recursively compute in dfs the longest paths with the getNextT algorithm of the paper
        getNextT(a, 1);
    }

    // return the computed iteration period
    return theIP;
}
/*getNextT()
 * find the longest zero delay path from actor 'actor', firing number l (starting from 1)
 * this implements Alg.1 getNextT from the TCAD paper.
 * */
void SDFretiming_SAnew::getNextT(const TimedSDFactor *actor, uint l) {

    // get the actor's id
    int aID = static_cast<int>(actor->getId());

    // for all outgoing edges of the actor
    for (const auto& pr : actor->getPorts()) {
        if (pr->getType() == SDFport::SDFportType::Out) {
            SDFchannel *ch = pr->getChannel();
            uint dE = ch->getInitialTokens();
            uint prdE = pr->getRate();
            uint cnsE = ch->getDstPort()->getRate();
            auto *nextActor = dynamic_cast<TimedSDFactor *>(ch->getDstActor());
            int nextAID = static_cast<int>(nextActor->getId());

            // compute the consuming firing according to Eq (4) of the TCAD paper
            int nextL =
                    static_cast<int>(floor((static_cast<float>(((l - 1) * prdE) + dE) / static_cast<float>(cnsE)))) + 1;

            // if the firing is within the same iteration
            if (nextL <= q[nextAID]) {
                // Alg. line 13
                if (exTemp[nextAID][nextL]
                    < exTemp[aID][l] + static_cast<int>(nextActor->getDiscreteExecutionTime())) {
                    // Alg. line 14
                    exTemp[nextAID][nextL] = exTemp[aID][l] + static_cast<int>(nextActor->getDiscreteExecutionTime());
                    // keep track of the minimum iteration period (corresponds to line 7 in function
                    // sdfIP in the algorithm in the paper)
                    theIP = (std::max)(theIP, exTemp[nextAID][nextL]);
                    // recursively in dfs compute other values (line 15 of the algorithm in the
                    // paper)
                    getNextT(nextActor, nextL);
                }
            }
        }
    }
}

// 2011-2-2. Scan two passes

/*
 * compute the iteration period by a breadth-first search
 * MG: Is it BFS or topological order?
 */

int SDFretiming_SAnew::computeIterationPeriodBFS() {
    // iteration period cannot be smaller than the maximum execution time of an actor
    // theIP is a global variable that holds the computed IP value during the computation
    theIP = maxExTimeOfActor;

    // vActors are the actors whose firings are to be explored in BFS order
    SDFactorConstRefs vActors;
    // vNums wil hold the firing numbers (starting from 1) of the corresponding actors in the list
    // vActors
    std::list<int> vNums;

    // inDegree holds for every actor and every firing the number of predecessor firings that have
    // been computed (?)
    inDegree = std::vector<std::vector<uint>>(rg->nrActors(), std::vector<uint>(0));

    // initialize exTemp and find V0 (vActors)
    // for all actors
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        int aID = static_cast<int>(a->getId());

        // initialize criT of the algorithm in the paper to -1 for all actor firings
        // exTemp[i] for actor i is vector with an entry for each firing of a in the repetition
        // vector MG: why is there one extra entry in the vector? Because the counting starts
        // from 1...
        exTemp[aID] = std::vector<int>(q[aID] + 1, -1);
        // inDegree is initialized to 0 for all firings
        inDegree[aID] = std::vector<uint>(q[aID] + 1, 0);

        // check if actor a is enabled in the initial state in the following for loop.
        // initialize the result to true and check if it is false
        bool isInitActor = true;
        // check all input ports of the actor if they have enough tokens
        for (auto iterP = a->portsBegin(); isInitActor && (iterP != a->portsEnd());
             iterP++) {
            const std::unique_ptr<SDFport>& pr = *iterP;
            if (pr->getType() == SDFport::SDFportType::In)
            // if (pr->getTypeAsString() == "in")
            {
                uint cnsE = pr->getRate();
                uint dE = pr->getChannel()->getInitialTokens();
                if (dE < cnsE) {
                    isInitActor = false;
                }
            }
        }

        // if for all incoming edges of a, there are dE>=cns, then a is an initial actor
        if (isInitActor) {
            // add the first firing of the initially enabled actor to the list of firings to be
            // visited
            vActors.push_back(a);
            vNums.push_back(1);
        }
    }

    // begin to compute the critical paths
    // this will set the in degree for each firing to be used for the BFS (topological order?)
    // visits of the critical firings for all the initially enabled actor firings...
    //
    for (const auto *iter : vActors) {
        const auto *a = dynamic_cast<const TimedSDFactor*>(iter);
        //      exTemp[a->getId()][1]=a->getDiscreteExecutionTime();
        findNextCriticalActor(a, 1);
    }

    ///////////////////////////
    /*  std::cout<<"inDegree&exTemp---before"<<std::endl;
        for (SDFactorsIter iter = rg->actorsBegin();
                iter != rg->actorsEnd(); iter++){
            TimedSDFactor *a = (TimedSDFactor*)*iter;
            int aId=a->getId();
            std::cout<<a->getName()<<":";
            for(int i=1;i<=q[aId];i++){
             std::cout<<inDegree[aId][i]<<" & "<<exTemp[aId][i]<<",";
            }
            std::cout<<std::endl;
        }*/
    /////////////////////////

    // begin to compute IP by bfs
    // while there are actor firings to be analyzed...
    while (!vActors.empty()) {
        // pop the first firing actor and firing number
        const auto *a = dynamic_cast<const TimedSDFactor*>(vActors.front());
        vActors.pop_front();
        int l = vNums.front();
        vNums.pop_front();

        // get the actor ID
        int aID = static_cast<int>(a->getId());

        //----------------------
        //  std::cout<<"visiting "<<a->getName()<<","<<l<<std::endl;

        // if exTemp (critT of the algorithm in the paper) is untouched, set it to the actor
        // execution time
        if (exTemp[aID][l] == -1) {
            exTemp[aID][l] = static_cast<int>(a->getDiscreteExecutionTime());
        }

        // for all outgoing edges of the actor firing (a,l)
        for (auto iterP = a->portsBegin(); iterP != a->portsEnd(); iterP++) {
            const std::unique_ptr<SDFport>& pr = *iterP;
            if (pr->getType() == SDFport::SDFportType::Out)
            // if (pr->getTypeAsString() == "out")
            {
                // determine the critical successor firing
                int dE = static_cast<int>(pr->getChannel()->getInitialTokens());
                int prdE = static_cast<int>(pr->getRate());
                int cnsE = static_cast<int>(pr->getChannel()->getDstPort()->getRate());
                auto *nextActor = dynamic_cast<TimedSDFactor *>(pr->getChannel()->getDstActor());
                int nextAID = static_cast<int>(nextActor->getId());
                int nextL =
                        static_cast<int>(floor((static_cast<float>(((l - 1) * prdE) + dE) / static_cast<float>(cnsE))))
                        + 1;

                // if the critical successor firing is still within the first iteration
                if (nextL <= q[nextAID]) {
                    // account for the visit using inDegree
                    inDegree[nextAID][nextL]--;

                    // if the new critical path to nextAID is longer than previous,
                    // replace exTemp
                    if (exTemp[nextAID][nextL]
                        < exTemp[aID][l] + static_cast<int>(nextActor->getDiscreteExecutionTime())) {
                        exTemp[nextAID][nextL] = exTemp[aID][l] + static_cast<int>(nextActor->getDiscreteExecutionTime());
                        // update the iteration period if necessary
                        theIP = (std::max)(theIP, exTemp[nextAID][nextL]);
                    }

                    // if all previous actors of nextAID are visited, add nextAID to the list
                    if (inDegree[nextAID][nextL] == 0) {
                        //----------------------
                        // std::cout<<" nextActor:"<<nextActor->getName()<<","<<nextL<<std::endl;
                        //----------------------
                        vActors.push_back(nextActor);
                        vNums.push_back(nextL);
                    }
                }
            }
        }

        //----------------------
        // std::cout<<"End of visiting "<<a->getName()<<","<<l<<std::endl;
        //----------------------
    }

    // return the compute iteration period
    return theIP;
}

/*
 * recursively compute the inDegree-s for all firings to be used in the BFS search
 * for firing (actor, l) determine the critical successor firings within the same iteration
 * update their inDegree-s and recursively continue when they are visited the first time
 * MG: why is it called 'findNextCriticalActor'?
 */
void SDFretiming_SAnew::findNextCriticalActor(const TimedSDFactor *actor, uint l) {
    // for all outgoing edges of actor
    for (auto iterP = actor->portsBegin(); iterP != actor->portsEnd(); iterP++) {
        const std::unique_ptr<SDFport>& pr = *iterP;
        if (pr->getType() == SDFport::SDFportType::Out)
        // if (pr->getTypeAsString() == "out")
        {
            // determine the critical successor firing on this port
            SDFchannel *ch = pr->getChannel();
            uint dE = ch->getInitialTokens();
            uint prdE = pr->getRate();
            uint cnsE = ch->getDstPort()->getRate();
            auto *nextActor = dynamic_cast<TimedSDFactor *>(ch->getDstActor());
            int nextAID = static_cast<int>(nextActor->getId());
            int nextL =
                    static_cast<int>(floor((static_cast<float>(((l - 1) * prdE) + dE) / static_cast<float>(cnsE)))) + 1;

            // if the next firing is still within the first iteration
            if (nextL <= q[nextAID]) {
                // update the in degree of the critical successor firing
                inDegree[nextAID][nextL]++;
                // continue recursively if this is the first time this firing is visited
                if (inDegree[nextAID][nextL] == 1) {
                    findNextCriticalActor(nextActor, nextL);
                }
            }
        }
    }
}

/*
 * a wrong implementation of bfs
 * MG: remove it then?
 */
int SDFretiming_SAnew::computeIterationPeriodBFSerr() {
    theIP = maxExTimeOfActor;
    SDFactorConstRefs vActors;
    std::list<int> vNums;

    TimedSDFactor *emptyActor = nullptr;

    SDFactorConstRefs preActors;
    std::list<int> preNums;

    //  std::vector< std::vector<bool> > visited;

    // initialize  pathTime & find V0 (vActors)
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        int aID = static_cast<int>(a->getId());

        //--------------------------
        exTemp[aID] = std::vector<int>(q[aID] + 1, -1);
        //  visited[aID]=std::vector<int>(q[aID]+1,false);
        //--------------------------
        bool isInitActor = true;
        for (auto iterP = a->portsBegin(); iterP != a->portsEnd(); iterP++) {
            const std::unique_ptr<SDFport>& pr = *iterP;
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
            vActors.push_back(a);
            vNums.push_back(1);
            // std::cout<<"1"<<std::endl;
            preActors.push_back(emptyActor);
            // std::cout<<"2"<<std::endl;
            preNums.push_back(0);

            //          visit[aID][1]=true;
            //          exTemp[aID][1]=a->getDiscreteExecutionTime();
        }
    }

    // begin to compute IP by bfs
    while (!vActors.empty()) {
        const auto *a = dynamic_cast<const TimedSDFactor *>(vActors.front());
        vActors.pop_front();

        int l = vNums.front();
        vNums.pop_front();

        // std::cout<<"3"<<std::endl;
        const auto *preA = dynamic_cast<const TimedSDFactor *>(preActors.front());
        preActors.pop_front();
        // std::cout<<"4"<<std::endl;
        int preL = preNums.front();
        preNums.pop_front();

        int aID = static_cast<int>(a->getId());
        //----------------------
        //  std::cout<<"visiting "<<a->getName()<<","<<l<<std::endl;
        if (preA == emptyActor) {
            exTemp[aID][l] = static_cast<int>(a->getDiscreteExecutionTime());

        } else {
            int preAID = static_cast<int>(preA->getId());
            if (exTemp[aID][l] < exTemp[preAID][preL] + static_cast<int>(a->getDiscreteExecutionTime())) {
                exTemp[aID][l] = exTemp[preAID][preL] + static_cast<int>(a->getDiscreteExecutionTime());

                theIP = (std::max)(theIP, exTemp[aID][l]);
            }
        }

        for (auto iterP = a->portsBegin(); iterP != a->portsEnd(); iterP++) {
            const std::unique_ptr<SDFport>& pr = *iterP;
            if (pr->getTypeAsString() == "out") {
                int dE = static_cast<int>(pr->getChannel()->getInitialTokens());
                int prdE = static_cast<int>(pr->getRate());
                int cnsE = static_cast<int>(pr->getChannel()->getDstPort()->getRate());
                auto *nextActor = dynamic_cast<TimedSDFactor *>(pr->getChannel()->getDstActor());
                int nextAID = static_cast<int>(nextActor->getId());

                int nextL =
                        static_cast<int>(floor((static_cast<float>(((l - 1) * prdE) + dE) / static_cast<float>(cnsE))))
                        + 1;

                if (nextL <= q[nextAID]) {

                    // if nextAID is not visited, add it to the list
                    if (exTemp[nextAID][nextL] == -1) {
                        //----------------------
                        // std::cout<<" nextActor:"<<nextActor->getName()<<","<<nextL<<std::endl;
                        //----------------------
                        vActors.push_back(nextActor);
                        vNums.push_back(nextL);
                        preActors.push_back(a);
                        preNums.push_back(l);
                    }

                    // if the new critical path to nextAID is longer than previous,
                    // replace exTemp
                    if (exTemp[nextAID][nextL]
                        < exTemp[aID][l] + static_cast<int>(nextActor->getDiscreteExecutionTime())) {
                        exTemp[nextAID][nextL] = exTemp[aID][l] + static_cast<int>(nextActor->getDiscreteExecutionTime());

                        theIP = (std::max)(theIP, exTemp[nextAID][nextL]);
                    }
                }
            }
        }

        //----------------------
        // std::cout<<"End of visiting "<<a->getName()<<","<<l<<std::endl;
        //----------------------
    }

    return theIP;
}

void SDFretiming_SAnew::printOptReVector() {
    std::cout << "optimal Retiming  Vector :(";
    for (const auto& iter : rg->getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        std::cout << optRe[a->getId()] << ",";
    }
    std::cout << ")" << std::endl;
}

// find a feasible retiming for the desired iteration period dip, if it exists
std::unique_ptr<TimedSDFgraph> retime_SAnew(const TimedSDFgraph& g, uint dip) {
    TimedSDFgraph result;
    SDFretiming_SAnew rtm(g);
    rtm.init();
    rtm.computeIterationPeriodBFS();
    if (rtm.feasibleRetimingSS(static_cast<int>(dip))) {
        return rtm.getRetimedSDFG()->clone();
    }
    return nullptr;
}

// find an optimal retiming for the graph (i.e., with minimal iteration period)
std::unique_ptr<TimedSDFgraph> retime_SAnew(const TimedSDFgraph& g) {
    TimedSDFgraph result;
    SDFretiming_SAnew rtm(g);
    // find the optimal retiming
    // MG, alternatively one could use optimalRetiming_BS, which one performs better?
    rtm.optimalRetiming_1();
    return rtm.getRetimedSDFG()->clone();
}

} // namespace SDF
