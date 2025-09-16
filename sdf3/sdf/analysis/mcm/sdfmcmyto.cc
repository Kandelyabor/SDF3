/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdfmcmyto.cc
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

#include "sdf/analysis/mcm/sdfmcmyto.h"

#include "base/basic_types.h"
#include "base/exception/exception.h"
#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include "maxplus/base/analysis/mcm/mcmyto.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/hsdf/check.h"
#include "sdf/base/timed/graph.h"
#include "sdfmcm.h"
#include "sdfmcmgraph.h"
#include <cmath>

using namespace SDF;
using namespace MaxPlus::Graphs;

namespace Graphs {
/**
 * mcmYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Young-Tarjan-Orlin's algorithm.
 */
CDouble mcmYoungTarjanOrlin(const TimedSDFgraph& g, bool mcmFormulation) {
    MCMgraphs components;
    graph ytoGraph;

    // Transform the HSDF to a weighted directed graph
    std::unique_ptr<MCMgraph> mcmGraph = transformHSDFtoMCMgraph(g, mcmFormulation);

    // Extract the strongly connected component from the graph
    // According to the Max-Plus book there is exactly one strongly
    // connected component in the graph when starting from a strongly
    // connected (H)SDF graph.
    stronglyConnectedMCMgraph(*mcmGraph, components);

    // Remove all hidden edges and nodes from the graph and assign new id's
    relabelMCMgraph(*mcmGraph);

    // Find maximum cycle mean
    CDouble mcm = NAN;
    if (mcmFormulation) {
        mcm = maxCycleMeanYoungTarjanOrlin(*mcmGraph);
    } else {
        mcm = maxCycleRatioYoungTarjanOrlin(*mcmGraph);
    }

    return mcm;
}

/**
 * maximumCycleMeanYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Young-Tarjan-Orlin's algorithm.
 */
CDouble maximumCycleYoungTarjanOrlin(const TimedSDFgraph& g, bool mcmFormulation) {

    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g)) {
        throw CException("Graph is not an HSDF graph.");
    }

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g)) {
        throw CException("Graph is not strongly connected.");
    }

    CDouble mcm = mcmYoungTarjanOrlin(g, mcmFormulation);

    return mcm;
}

} // namespace Graphs