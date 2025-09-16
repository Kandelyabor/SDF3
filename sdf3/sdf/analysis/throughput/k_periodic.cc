/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   k_periodic.cc
 *
 *  Author          :   Anouk Westhoff
 *
 *  Date            :   May, 2022
 *
 *  Function        :   SDFG throughput analysis using the K-Periodic throughput analysis method
 *                      The implementation is inspired by the code from
 *                      https://github.com/bbodin/kiter
 *                      The algorithm is published in
 *                      Bruno Bodin, Alix Munier-Kordon, and Benoit Dupont de Dinechin. 2016.
 *                      Optimal and fast throughput evaluation of CSDF. In Proceedings of the
 *                      53rd Annual Design Automation Conference (DAC '16). Association for
 *                      Computing Machinery, New York, NY, USA, Article 160, 1-6.
 *                      https://doi.org/10.1145/2897937.2898056
 *
 *  History         :
 *      01-07-22    :   Initial version.
 *
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

#include "k_periodic.h"

#include "sdf/base/algo/components.h"
#include "sdf/base/algo/repetition_vector.h"

#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include "maxplus/base/analysis/mcm/mcmyto.h"

#include <algorithm> // std::max
#include <cmath>
#include <memory>

using namespace ::MaxPlus::Graphs;

namespace SDF {

namespace {

uint gcd(uint a, uint b) {
    if (a == 0) {
        return b;
    }
    if (b == 0) {
        return a;
    }
    if (a == b) {
        return a;
    }
    if (a > b) {
        return gcd(a % b, b);
    }
    return gcd(a, b % a);
}

uint getLCM(uint a, uint b) {
    return static_cast<uint>(((static_cast<ulong>(a)) * (static_cast<ulong>(b)))
                             / (static_cast<ulong>(gcd(a, b))));
}


// integer division that rounds down (floor) instead of rounding towards zero.
int floor_div(int a, int b) {
    int d = a / b;
    int r = a % b;
    return r != 0 ? (d - ((a < 0) ^ (b < 0))) : d;
}

/**
 * getLCMarray ()
 * The function is necessary for the computation of vector P, used for
 * normalization of SDFGs.
 */

unsigned int getLCMArray(RepetitionVector RVec) {
    unsigned int lcm = getLCM(RVec.at(0), RVec.at(1));
    for (int i : RVec) {
        lcm = getLCM(lcm, i);
    }
    return lcm;
}

/**
 * createLiveNormalizedG ()
 * Applies the the useful token assumption and normalization on an SDFG and
 * return it as graph G.
 */

void createLiveNormalizedG(const TimedSDFgraph& SDFgraph, kiterGraph& G) {
    RepetitionVector N = computeRepetitionVector(SDFgraph);

    uint iterN = 0;
    for (const auto &iter : SDFgraph.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        auto tG = std::make_unique<kiterTask>(a->getId(), N[iterN], 1, 0, 0, a->getExecutionTime());
        G.addTask(std::move(tG));
        iterN++;
    }

    unsigned int P = (N.size() > 1) ? getLCMArray(N) : N.at(0);

    for (const auto &c : SDFgraph.getChannels()) {
        SDFport *ti = c->getSrcPort();
        SDFport *tj = c->getDstPort();
        unsigned int usefulTokens = c->getInitialTokens();

        unsigned int ratesGcd = gcd(ti->getRate(), tj->getRate());
        usefulTokens = (usefulTokens / ratesGcd) * ratesGcd;
        c->setInitialTokens(usefulTokens);

        unsigned int oldZ = c->getSrcPort()->getRate();
        unsigned int newZ = P / N[c->getSrcActor()->getId()];
        // use longs to prevent overflows in some of the artificial benchmarks
        c->setInitialTokens(static_cast<uint>((static_cast<ulong>(c->getInitialTokens()))
                                              * (static_cast<ulong>(newZ))
                                              / (static_cast<ulong>(oldZ))));

        auto bG = std::make_unique<kiterBuffer>(c->getId(), c->getInitialTokens());

        for (const auto& tG : G.getTasks()) {

            if (tG->id == c->getSrcActor()->getId()) {
                bG->src = tG.get();
                tG->Z = static_cast<uint>(static_cast<ulong>(ti->getRate())
                                          * (static_cast<ulong>(newZ))
                                          / (static_cast<ulong>(oldZ)));
            }
            if (tG->id == c->getDstActor()->getId()) {
                bG->dst = tG.get();
                tG->Z = static_cast<uint>(static_cast<ulong>(tj->getRate())
                                          * (static_cast<ulong>(newZ))
                                          / (static_cast<ulong>(oldZ)));
            }
            if (bG->src != nullptr && bG->dst != nullptr) {
                break;
            }
        }

        G.addBuffer(std::move(bG));
    }
}

/******************************************************************************
 * partial constraint graph P = (V, E, L, H)
 *****************************************************************************/

/**
 * createPartialConstraintGraphP ()
 * Creates the partial constraint graph with fixed periodicity vector K = 1 for
 * the live normalized graph G.
 */

void createPartialConstraintGraphP(kiterGraph& G, MCMgraph& P) {
    for (const auto& t : G.getTasks()) {
        // add nodes
        MCMnode *n = P.addNode(G.n_nodes, true);
        G.n_nodes++;
        t->nodes.push_back(n);
        // --

        // add E1
        CDouble d = (t->selfLoop != 0) ? t->duration : 0;
        CDouble w = (t->selfLoop != 0) ? t->selfLoop : 0;
        MCMnode *src = t->nodes.back();
        MCMnode *dst = t->nodes.front();
        P.addEdge(G.n_edges, *src, *dst, w, d, true);
        G.n_edges++;
        // --
    }

    // add E3
    for (const auto& b : G.getBuffers()) {
        kiterTask *source = b->src;
        kiterTask *destination = b->dst;

        uint Zi = source->Z;
        uint Zj = destination->Z;
        uint bz = gcd(Zi, Zj);
        uint M = b->M0;

        auto d = static_cast<CDouble>(b->src->duration);
        auto w = static_cast<CDouble>(
                -floor_div(-(static_cast<int>(bz)) - (static_cast<int>(M)) + (static_cast<int>(Zj)),
                           (static_cast<int>(bz)))
                * (static_cast<int>(bz)));
        MCMnode *src = nullptr;
        MCMnode *dst = nullptr;

        for (auto &nn : P.getNodes()) {
            if (nn.id == b->src->id) {
                src = &nn;
            }
            if (nn.id == b->dst->id) {
                dst = &nn;
            }

            if (src != nullptr && dst != nullptr) {
                break;
            }
        }
        P.addEdge(G.n_edges, *src, *dst, w, d, true);
        G.n_edges++;
    }
    // --
}

/**
 * updatePartialConstraintGraphP ()
 * Applies the optimality test, increases K if necessary and updates the
 * partial constraint graph accordingly. If the optimality test is valid, and
 * partial constraint graph P wasn't updated, the function returns true. When the
 * optimality test failed and P was updated, the function returns false.
 */

bool updatePartialConstraintGraphP(kiterGraph& G,
                                   MCMgraph& P,
                                   std::vector<const MCMedge *> &cycle) {
    std::set<kiterTask *> criticalTasks;
    std::set<kiterBuffer *> criticalBuffers;

    // optimality test
    unsigned int gcdN = 0;
    for (const auto& t : G.getTasks()) {
        for (auto *n : t->nodes) {
            for (auto &i : cycle) {
                if (n->id == i->src->id) {
                    criticalTasks.insert(t.get());
                    gcdN = gcd(t->N, gcdN);
                }
            }
        }
    }

    for (auto iterT = criticalTasks.begin(); iterT != criticalTasks.end();) {
        kiterTask *t = *iterT;
        unsigned int gcdNi = t->N / gcdN;
        if (t->K % gcdNi != 0) {
            t->K = getLCM(t->K, gcdNi);
            iterT++;
        } else {
            iterT++;
            criticalTasks.erase(t);
        }
    }
    // ---

    // update P
    if (!criticalTasks.empty()) {
        // for all critical tasks ...
        for (auto *t : criticalTasks) {
            // remove edges

            // collect input buffers to t
            for (const auto& b : t->in) {
                criticalBuffers.insert(b);
            }
            // collect output buffers from t
            for (const auto& b : t->out) {
                criticalBuffers.insert(b);
            }

            // for every node of task t in P
            for (auto *n : t->nodes) {
                // remove incoming edges
                for (auto iterE = n->in.begin(); iterE != n->in.end();) {
                    MCMedge *e = *iterE;
                    iterE++;
                    P.removeEdge(*e);
                }
                // remove outgoing edges
                for (auto iterE = n->out.begin(); iterE != n->out.end();) {
                    MCMedge *e = *iterE;
                    iterE++;
                    P.removeEdge(*e);
                }
            }
            // ---

            // add nodes to match t->K
            size_t nodesSize = t->nodes.size();
            for (size_t iter = 0; iter != t->K - nodesSize; iter++) {
                MCMnode *n = P.addNode(G.n_nodes, true);
                G.n_nodes++;
                t->nodes.push_back(n);
            }
            // ---

            // add E1
            auto d = static_cast<CDouble>((t->selfLoop != 0) ? t->duration : 0);
            auto w = static_cast<CDouble>(
                    (t->selfLoop != 0)
                            ? -floor_div(((static_cast<int>(t->Z)) * (1 - (static_cast<int>(t->K))))
                                                 - (static_cast<int>(t->selfLoop)),
                                         (static_cast<int>(t->K * t->Z)))
                                      * (static_cast<int>(t->K * t->Z))
                            : (static_cast<int>(t->Z * t->K)));

            MCMnode *src = t->nodes.back();
            MCMnode *dst = t->nodes.front();

            P.addEdge(G.n_edges, *src, *dst, w, d, true);
            G.n_edges++;
            // ---

            // add E2
            for (auto iterN = next(t->nodes.begin()), prevN = t->nodes.begin();
                 iterN != t->nodes.end();
                 prevN = iterN, iterN++) {

                auto d = static_cast<CDouble>((t->selfLoop != 0) ? t->duration : 0);
                auto w = static_cast<CDouble>(
                        (t->selfLoop != 0) ? -floor_div((static_cast<int>(t->Z))
                                                                - (static_cast<int>(t->selfLoop)),
                                                        static_cast<int>(t->K * t->Z))
                                                     * (static_cast<int>(t->K * t->Z))
                                           : 0);

                MCMnode *src = *prevN;
                MCMnode *dst = *iterN;

                P.addEdge(G.n_edges, *src, *dst, w, d, true);
                G.n_edges++;
            }
            // ---
        }

        // add E3
        for (auto *b : criticalBuffers) {
            kiterTask *srcT = b->src;
            kiterTask *dstT = b->dst;

            uint Zi = srcT->Z;
            uint Ki = srcT->K;
            uint Zj = dstT->Z;
            uint Kj = dstT->K;
            uint bz = gcd(Zi, Zj);
            uint bk = gcd(Zi * Ki, Zj * Kj);
            uint M = b->M0;

            uint ai = 0;
            int normDapkm1 = 0;
            int normDapk = 0;

            for (auto *srcN : srcT->nodes) {
                ai++;

                uint aj = 0;
                int normdamkp = 0;
                normDapk += static_cast<int>(Zi);

                for (auto *dstN : dstT->nodes) {
                    aj++;
                    normdamkp += static_cast<int>(Zj);

                    int lpimin = ((std::max)(0, (static_cast<int>(Zi)) - (static_cast<int>(Zj)))
                                  - (static_cast<int>(M)) - normDapk + normdamkp);
                    int lpimax = (-(static_cast<int>(M)) - normDapkm1 + normdamkp
                                  - (static_cast<int>(bz)));
                    int rpimin = lpimin % (static_cast<int>(bk));
                    int rpimax = lpimax % (static_cast<int>(bk));

                    int alphamin = (rpimin <= 0) ? (lpimin - rpimin)
                                                 : (lpimin - rpimin + (static_cast<int>(bk)));
                    int alphamax = (rpimax >= 0) ? (lpimax - rpimax)
                                                 : (lpimax - rpimax - (static_cast<int>(bk)));

                    if (alphamin <= alphamax) {

                        auto d = static_cast<CDouble>(srcT->duration);
                        auto w = static_cast<CDouble>(
                                -floor_div(((static_cast<int>(Zi)) - (static_cast<int>(bz))
                                            - (static_cast<int>(M))
                                            - ((static_cast<int>(Zi * ai))
                                               - (static_cast<int>(Zj * aj)))),
                                           static_cast<int>(bk))
                                * (static_cast<int>(bk)));

                        MCMnode *src = srcN;
                        MCMnode *dst = dstN;

                        P.addEdge(G.n_edges, *src, *dst, w, d, true);
                        G.n_edges++;
                    }
                }
                normDapkm1 = normDapk;
            }
        }
        // ---
        return false;
    }
    return true;
}

/******************************************************************************
 * print functions
 *****************************************************************************/

/**
 * printSDFG ()
 * Prints SDFGs.
 */

// SDFgraph.print(std::cout);

/**
 * printG ()
 * Prints live normalized SDFGs.
 */

void printG(kiterGraph& G) {
    std::cout << "Graph G" << std::endl;
    std::cout << std::endl;

    for (const auto& t : G.getTasks()) {
        std::cout << "Task  (" << t->id << ")" << std::endl;
        std::cout << "N=" << t->N << ", ";
        std::cout << "K=" << t->K << ", ";
        std::cout << "Z=" << t->Z << std::endl;

        for (auto *n : t->nodes) {
            std::cout << n->id << ", ";
        }

        std::cout << std::endl;
    }

    for (const auto& b : G.getBuffers()) {

        std::cout << "Buffer  (" << b->id << ")" << std::endl;
        std::cout << b->src->id << " -> " << b->dst->id << std::endl;
        std::cout << "M0(b)=" << b->M0 << std::endl;
        std::cout << std::endl;
    }
}

/**
 * printP ()
 * Prints partial constraint graphs P.
 */

void printP(MCMgraph& P) {
    std::cout << "Graph P" << std::endl;
    std::cout << std::endl;

    for (auto iterN : P.getNodes()) {
        MCMnode *n = &iterN;

        std::cout << "Node  (" << n->id << ")" << std::endl;
        std::cout << std::endl;
    }

    for (auto iterE : P.getEdges()) {
        MCMedge *e = &iterE;

        std::cout << "Edge  (" << e->id << ")" << std::endl;
        std::cout << e->src->id << " -> " << e->dst->id << std::endl;
        std::cout << "w=" << e->w << std::endl;
        std::cout << "d=" << e->d << std::endl;
        std::cout << std::endl;
    }
}


} // namespace

/******************************************************************************
 * Live normalized SDFG G = (T, B, M0)
 *****************************************************************************/

kiterGraph::kiterGraph() : n_nodes(0), n_edges(0) {}

kiterGraph::~kiterGraph() = default;

kiterBuffer::kiterBuffer(CId nId, uint nM0) : id(nId), M0(nM0), src(nullptr), dst(nullptr) {}

kiterTask::kiterTask(CId nId, uint nN, uint nK, uint nZ, uint nselfloop, uint n_duration) :
    id(nId), N(nN), K(nK), Z(nZ), selfLoop(nselfloop), duration(n_duration) {}


/******************************************************************************
 * k-periodic throughput analysis
 *****************************************************************************/

/**
 * kPeriodic ()
 * Implementation of K-periodic schedule throughput analysis as described in:
 * B. Bodin and A. M. Kordon, "Evaluation of the Exact Throughput of a
 * Synchronous Dataflow Graph", 2021.
 */

TDtime SDFkPeriodicAnalysis::kPeriodic(const TimedSDFgraph& SDFgraph) {
    if (SDFgraph.nrActors() == 1) {
        const TimedSDFactor* a = dynamic_cast<TimedSDFactor*>((*SDFgraph.actorsBegin()).get());
        CDouble duration = a->getExecutionTime();
        return 1 / duration;
    }

    // Apply useful token assumption to and normalize the live SDFG
    kiterGraph G;
    createLiveNormalizedG(SDFgraph, G);

    // Compute the partial constraint graph P with periodicity vector K = 1 for
    // live normalized SDFG G.
    MCMgraph P;
    createPartialConstraintGraphP(G, P);

    // Iteratively increases the periodicity vector K until maximum throughput
    // is reached.
    std::vector<const MCMedge *> criticalCircuit;
    CDouble OmegaStar = NAN;
    bool OptCircuitFound = false;
    while (!OptCircuitFound) {
        // Determine the period (equivalent to throughput) of graph P using the
        // YTO maximum time-to-cost ratio algorithm.
        OmegaStar = 1 / minCycleRatioAndCriticalCycleYoungTarjanOrlin(P, &criticalCircuit);

        // Apply the optimality test to P, update P if necessary, and return
        // validness of the optimality test.
        OptCircuitFound = updatePartialConstraintGraphP(G, P, criticalCircuit);
    }

    // Return maximum throughput.
    CDouble throughput = 1 / (OmegaStar * G.getTasks().back()->N * G.getTasks().back()->Z);

    return throughput;
}

/**
 * analyze ()
 * Divides the SDFG into strongly connected components and applies the
 * k-periodic throughput analysis.
 */

TDtime SDFkPeriodicAnalysis::analyze(const TimedSDFgraph& SDFgraph) {
    SDFkPeriodicAnalysis thrAnalysisAlgo;
    RepetitionVector repVec;
    TDtime thr = NAN;

    // Compute repetition vector
    repVec = computeRepetitionVector(SDFgraph);

    // Check that the graph gr is a strongly connected graph
    if (!isStronglyConnectedGraph(SDFgraph)) {

        // Split graph into its strongly connected components
        // and find minimal throughput of all components
        SDFgraphComponents comp = stronglyConnectedComponents(SDFgraph);
        thr = TDTIME_MAX;

        for (auto co : comp) {
            RepetitionVector repVecGC;
            std::unique_ptr<TimedSDFgraph> gc = nullptr;
            TDtime thrGc = NAN;
            CId idFirstActor = 0;
            CId id = 0;

            // Construct graph from component
            gc = std::unique_ptr<TimedSDFgraph>(dynamic_cast<TimedSDFgraph*>(componentToSDFgraph(co).release()));

            // Id of first actor in component
            idFirstActor = (*gc->actorsBegin())->getId();

            // Relabel actors
            id = 0;
            for (const auto& u : gc->getActors()) {
                u->setId(id);
                id++;
            }

            // Relabel channels
            id = 0;
            for (const auto& c : gc->getChannels()) {
                c->setId(id);
                id++;
            }

            // Graph contains at least one channel
            if (gc->nrChannels() > 0) {
                // Compute throughput component
                thrGc = SDF::SDFkPeriodicAnalysis::analyze(*gc);

                // Compute repetition vector of component
                repVecGC = computeRepetitionVector(*gc);

                // Scale throughput wrt repetition vector component vs graph
                thrGc = (thrGc * repVecGC[0]) / static_cast<TDtime>(repVec[idFirstActor]);

                thr = (std::min)(thrGc, thr);
            }
        }
        return thr;
    }

    return SDF::SDFkPeriodicAnalysis::kPeriodic(SDFgraph);
}
} // namespace SDF
