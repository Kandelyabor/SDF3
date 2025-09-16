/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdfmcmgraph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 7, 2005
 *
 *  Function        :   Convert HSDF graph to weighted directed graph for MCM
 *                      calculation.
 *
 *  History         :
 *      07-11-05    :   Initial version.
 *
 * $Id: mcmgraph.h,v 1.1.1.1.2.2 2010-08-18 07:51:47 mgeilen Exp $
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

#include "sdfmcmgraph.h" // IWYU pragma: keep
#include "base/exception/exception.h"
#include "sdf/base/hsdf/check.h"
#include "sdf/base/timed/graph.h"
#include <memory>

using namespace SDF;
using namespace MaxPlus::Graphs;

namespace Graphs {

namespace {

/**
 * createInitialMCMgraph ()
 * The function converts an HSDF graph to a weighted directed graph
 * by replacing each actor in the HSDF with a node in the graph and each
 * channel with an edge. Each edge is assigned a weight equal to the
 * execution time of the destination actor of the corresponding channel.
 * The initial tokens are preserved during the conversion from channels to
 * edges.
 */
void createInitialMCMgraph(const TimedSDFgraph &g, MCMgraph &mcmGraph) {
    // create node for every actor
    for (const auto &a : g.getActors()) {
        mcmGraph.addNode(a->getId(), true);
    }

    // create edges
    for (const auto &c : g.getChannels()) {
        CDouble d = c->getInitialTokens();
        CDouble w = (dynamic_cast<TimedSDFactor *>(c->getSrcActor()))->getExecutionTime();

        MCMnode *src = nullptr;
        MCMnode *dst = nullptr;
        for (auto &iterN : mcmGraph.getNodes()) {
            MCMnode *n = &iterN;

            if (n->id == c->getSrcActor()->getId()) {
                src = n;
            }
            if (n->id == c->getDstActor()->getId()) {
                dst = n;
            }

            if (src != nullptr && dst != nullptr) {
                break;
            }
        }
        ASSERT(dst != nullptr, "No dst node found.");
        ASSERT(src != nullptr, "No src node found.");

        mcmGraph.addEdge(c->getId(), *src, *dst, w, d, true);
    }
}

} // namespace

/**
 * transformHSDFtoMCMgraph ()
 * The function converts an HSDF graph to a weighted directed graph
 * used in the MCM algorithm of Karp (and its variants). By default, the
 * a longest path calculation is performed to make the graph suitable
 * as input for an MCM algorithm. Setting the flag 'mcmFormulation' to false
 * result in an MCM graph which is suitable for an MCR (cycle ratio)
 * formulation. This avoids running the longest path algo.
 */
std::unique_ptr<MCMgraph> transformHSDFtoMCMgraph(const TimedSDFgraph &g, bool mcmFormulation) {
    auto mcmGraph = std::make_unique<MCMgraph>();

    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g)) {
        throw CException("Graph is not an HSDF graph.");
    }

    // Create an initial directed weighted graph
    createInitialMCMgraph(g, *mcmGraph);

    // Convert the initial directed weighted graph to the required form
    if (mcmFormulation) {
        addLongestDelayEdgesToMCMgraph(*mcmGraph);
    }

    // std::cerr << "edge: (u, v, w, d)" << std::endl;
    // for (MCMedgesIter iter = mcmGraph.edges.begin();
    //     iter != mcmGraph.edges.end(); iter++)
    // {
    //     MCMedge *e = *iter;

    //     if (!e->visible)
    //         continue;

    //     std::cerr << "(" << e->src->id;
    //     std::cerr << ", " << e->dst->id;
    //     std::cerr << ", " << e->w;
    //     std::cerr << ", " << e->d;
    //     std::cerr << ")" << std::endl;
    // }
    // std::cerr << std::endl;

    return mcmGraph;
}

} // namespace Graphs