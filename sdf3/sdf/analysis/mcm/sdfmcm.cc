/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdfmcm.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Maximum cycle mean of a HSDF graph
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: mcm.cc,v 1.1.1.1.2.2 2010-08-18 07:51:46 mgeilen Exp $
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

#include "sdf/analysis/mcm/sdfmcm.h"
#include "base/exception/exception.h"
#include "maxplus/base/analysis/mcm/mcm.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/algo/cycle.h"
#include "sdf/base/algo/graph.h"
#include "sdf/base/hsdf/check.h"
#include "sdf/base/timed/graph.h"
#include <cmath>
#include <memory>

/**
 * maximumCycleMeanCycle ()
 * The function computes the maximum cycle mean of a cycle.
 */

using namespace SDF;
using namespace MaxPlus::Graphs;

namespace Graphs {

namespace {

CDouble maximumCycleMeanCycle(const TimedSDFgraph& /*g*/, SDFgraphCycle &cycle) {
    SDFtime t = 0;
    uint tokens = 0;
    auto cycleIter = cycle.begin();

    while (cycleIter != cycle.end()) {
        auto *u = dynamic_cast<TimedSDFactorRef>(*cycleIter);

        // Add execution time of actor to execution time of the cycle
        t += u->getExecutionTime();

        TimedSDFactorRef v = nullptr;
        cycleIter++;
        if (cycleIter == cycle.end()) {
            // Next actor in cycle is the first actor
            v = dynamic_cast<TimedSDFactorRef>(*cycle.begin());
        } else {
            v = dynamic_cast<TimedSDFactorRef>(*cycleIter);
        }

        // Find channel with minimum number of tokens to next actor in cycle
        int tokensCh = 0;
        bool firstPort = true;
        for (const auto &p : u->getPorts()) {
            // Is port an input port?
            if (p->getType() == SDFport::SDFportType::In) {
                continue;
            }

            // Connects port to next actor?
            if (p->getChannel()->getDstActor()->getId() == v->getId()) {
                if (firstPort) {
                    tokensCh = static_cast<int>(p->getChannel()->getInitialTokens());
                } else {
                    tokensCh = (std::min)(tokensCh, static_cast<int>(p->getChannel()->getInitialTokens()));
                }

                firstPort = false;
            }
        }

        // Add tokens found between u and v to tokens on cycle
        tokens += tokensCh;
    }

    if (tokens == 0) {
        return INFINITY;
    }

    return (static_cast<CDouble>(t)) / (static_cast<CDouble>(tokens));
}

} // namespace

#ifndef __CALC_MCM_PER_CYCLE__

namespace {

/**
 * mcmSimpleCycleVisit ()
 * The visitor function of the DFS based algorithm used for detecting
 * simple cycles and computing the MCM of them.
 */
void mcmSimpleCycleVisit(uint a,
                         const TimedSDFgraph& g,
                         v_uint &nodeId,
                         v_uint &actorId,
                         std::vector<v_uint> &graph,
                         uint size,
                         v_int &color,
                         v_uint &pi,
                         CDouble &mcmCycles) {
    // color[a] <- gray
    color[a] = 1;

    // for each b in Adj(e)
    v_uint adj = mcmGetAdjacentActors(a, nodeId, actorId, graph, size);
    for (unsigned int b : adj) {
        // do if color[b] = white
        if (color[b] == 0) {
            pi[b] = a;
            mcmSimpleCycleVisit(b, g, nodeId, actorId, graph, size, color, pi, mcmCycles);
        } else if (color[b] == 1) {
            CDouble mcmCycle = NAN;
            SDFgraphCycle cycle;

            // Create new cycle
            cycle.push_front(g.getConstActor(a));

            uint u = a;
            while (u != b) {
                u = pi[u];
                cycle.push_front(g.getConstActor(u));
            }

            // MCM of the cycle
            mcmCycle = maximumCycleMeanCycle(g, cycle);

            // Maximum of all cycles
            mcmCycles = (std::max)(mcmCycles, mcmCycle);
        }
    }

    // color[a] <- white
    color[a] = 0;
}


/**
 * mcmSimpleCycles ()
 * The function performs a depth first search on the graph to discover all
 * simple cycles in the graph. The cycle mean of each cycle is computed
 * and the maximum cycle mean is returned.
 */
CDouble mcmSimpleCycles(const TimedSDFgraph& g, bool transpose) {
    CDouble mcmGraph = 0.0;
    v_uint pi(g.nrActors());

    // Split graph in strongly connected components
    SDFgraphComponents components = stronglyConnectedComponents(g);

    if (components.empty()) {
        return INFINITY;
    }

    // Iterate over all components
    for (auto &component : components) {
        v_uint actorId;
        v_uint nodeId;
        std::vector<v_uint> graph;

        // Convert component to graph structure
        sdfComponentToGraph(g, component, nodeId, actorId, graph, transpose);

        // For each a in A' do color[a] <- white
        v_int color(g.nrActors(), 0);

        // For each a in A' do pi[a] <- NIL
        for (uint a = 0; a < g.nrActors(); a++) {
            pi[a] = 0;
        }

        // For an actor a in A' (take the first actor)
        uint a = actorId[0];

        // color[a] <- gray
        color[a] = 1;

        // For each actor b in Adj(a, s)
        v_uint adj = mcmGetAdjacentActors(
                a, nodeId, actorId, graph, static_cast<uint>(component.size()));
        for (unsigned int b : adj) {
            if (color[b] == 0) {
                pi[b] = a;
                mcmSimpleCycleVisit(b,
                                    g,
                                    nodeId,
                                    actorId,
                                    graph,
                                    static_cast<uint>(component.size()),
                                    color,
                                    pi,
                                    mcmGraph);
            } else if (color[b] == 1) {
                CDouble mcmCycle = NAN;
                SDFgraphCycle cycle;

                // Self-loop
                cycle.push_front(g.getConstActor(a));

                // MCM of the cycle
                mcmCycle = maximumCycleMeanCycle(g, cycle);

                // Graph is maximum of all cycles
                mcmGraph = (std::max)(mcmGraph, mcmCycle);
            }
        }
    }

    return mcmGraph;
}

} // namespace


/**
 * maximumCycleMeanCycles ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * the cycle-based definition of the MCM.
 */
CDouble maximumCycleMeanCycles(const TimedSDFgraph& g) {
    CDouble mcmGraph = NAN;

    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g)) {
        throw CException("Graph is not an HSDF graph.");
    }

    // Check that the graph g is a strongly connected graph
    // if (!isStronglyConnectedGraph(g))
    //    throw CException("Graph is not strongly connected.");

    // Find simple cycles in graph and calculate MCM for each cycle
    mcmGraph = mcmSimpleCycles(g, false);

    return mcmGraph;
}

#else

/**
 * maximumCycleMeanCycles ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * the cycle-based definition of the MCM.
 */
CDouble maximumCycleMeanCycles(const TimedSDFgraph& g) {
    CDouble mcmGraph = 0;

    // Check that graph g is an HSDF graph
    if
        throw CException("Graph is not an HSDF graph.");

    // Check that the graph g is a strongly connected graph
    // if (!isStronglyConnectedGraph(g))
    //    throw CException("Graph is not strongly connected.");

    // Find simple cycles in graph
    SDFgraphCycles cycles = findSimpleCycles(g);

    // Calculate MCM for each cycle
    for (SDFgraphCyclesIter iter = cycles.begin(); iter != cycles.end(); iter++) {
        CDouble mcmCycle;
        SDFgraphCycle cycle = *iter;

        // MCM of the cycle
        mcmCycle = maximumCycleMeanCycle(g, cycle);

        // Graph is maximum of all cycles
        mcmGraph = MAX(mcmGraph, mcmCycle);
    }

    return mcmGraph;
}

#endif


} // namespace Graphs