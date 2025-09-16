/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdfmcmhoward.cc
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

#include "base/exception/exception.h"
#include "maxplus/base/analysis/mcm/mcmhoward.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/hsdf/check.h"
#include "sdfmcm.h"
#include "sdfmcmgraph.h"
#include <algorithm>
#include <cmath>


using namespace SDF;
using namespace MaxPlus::Graphs;

namespace Graphs {

namespace {


/**
 * mcmHoward ()
 * The function computes the maximum cycle mean of a HSDF graph using Howard's
 * algorithm.
 */
CDouble mcmHoward(const TimedSDFgraph& g) {
    CDouble mcm = NAN;
    MCMgraphs components;
    int nrNodes = 0;
    int nrEdges = 0;
    int nrIterations = 0;
    int nrComponents = 0;

    // Transform the HSDF to a weighted directed graph
    std::unique_ptr<MCMgraph> mcmGraph = transformHSDFtoMCMgraph(g);

    // Extract the strongly connected component from the graph
    // According to the Max-Plus book there is exactly one strongly
    // connected component in the graph when starting from a strongly
    // connected (H)SDF graph.
    stronglyConnectedMCMgraph(*mcmGraph, components);

    // Number of nodes and edges in the graph
    nrNodes = static_cast<int>(mcmGraph->nrVisibleNodes());
    nrEdges = static_cast<int>(mcmGraph->nrVisibleEdges());

    // Allocate memory for matrices
    std::unique_ptr<std::vector<int>> IJ = std::make_unique<std::vector<int>>(2 * nrEdges);
    std::unique_ptr<std::vector<double>> A = std::make_unique<std::vector<double>>(nrEdges);
    std::unique_ptr<std::vector<double>> chi = std::make_unique<std::vector<double>>(nrNodes);
    std::unique_ptr<std::vector<double>> v = std::make_unique<std::vector<double>>(nrNodes);
    std::unique_ptr<std::vector<int>> policy = std::make_unique<std::vector<int>>(nrNodes);

    // Convert the graph to a sparse matrix
    convertMCMgraphToMatrix(*mcmGraph, &IJ, &A);

    // Run Howard's algorithm
    Howard(*IJ, *A, nrNodes, nrEdges, &chi, &v, &policy, &nrIterations, &nrComponents);

    // The MCM is equal to maximum entry in the cycle time vector
    mcm = 0;
    for (int i = 0; i < nrNodes; i++) {
        mcm = (std::max)(mcm, chi->at(i));
    }

    return mcm;
}

} //namespace

/**
 * maximumCycleMeanHoward ()
 * The function computes the maximum cycle mean of a HSDF graph using Howard's
 * algorithm.
 */
CDouble maximumCycleMeanHoward(const TimedSDFgraph& g) {
    CDouble mcmGraph = NAN;

    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g)) {
        throw CException("Graph is not an HSDF graph.");
    }

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g)) {
        throw CException("Graph is not strongly connected.");
    }

    mcmGraph = mcmHoward(g);

    return mcmGraph;
}
} // namespace Graphs