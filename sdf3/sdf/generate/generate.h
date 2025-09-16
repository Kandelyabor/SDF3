/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2005
 *
 *  Function        :   Generate SDF graphs
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: generate.h,v 1.2 2008-09-18 07:38:21 sander Exp $
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

#ifndef SDF_GENERATE_GENERATE_H_INCLUDED
#define SDF_GENERATE_GENERATE_H_INCLUDED

#include "sdf/base/timed/graph.h"

namespace SDF {
/**
 * generateSDFgraph ()
 * Generate a random SDF graph.
 */
std::unique_ptr<TimedSDFgraph> generateSDFgraph(uint nrActors,
                                double avgInDegree,
                                double varInDegree,
                                double minInDegree,
                                double maxInDegree,
                                double avgOutDegree,
                                double varOutDegree,
                                double minOutDegree,
                                double maxOutDegree,
                                double avgRate,
                                double varRate,
                                double minRate,
                                double maxRate,
                                bool acyclic,
                                bool stronglyConnected,
                                double initialTokenProp,
                                double tokenFreeProp,
                                bool tokenFreeCycle,
                                uint repetitionVectorSum,
                                bool multigraph);

/**
 * generateSDFgraphProperties ()
 * Generate properties for a random SDF graph.
 */
void generateSDFgraphProperties(TimedSDFgraph& g,
                                bool execTime,
                                uint nrProcTypes,
                                double mapChance,
                                double avgExecTime,
                                double varExecTime,
                                double minExecTime,
                                double maxExecTime,
                                bool stateSize,
                                double avgStateSize,
                                double varStateSize,
                                double minStateSize,
                                double maxStateSize,
                                bool tokenSize,
                                double avgTokenSize,
                                double varTokenSize,
                                double minTokenSize,
                                double maxTokenSize,
                                bool throughputConstraint,
                                uint autoConcurrencyDegree,
                                double throughputScaleFactor,
                                bool bandwidthRequirement,
                                double avgBandwidth,
                                double varBandwidth,
                                double minBandwidth,
                                double maxBandwidth,
                                bool bufferSize,
                                bool latencyRequirement,
                                double avgLatency,
                                double varLatency,
                                double minLatency,
                                double maxLatency,
                                bool integerMCM);

} // namespace SDF
#endif
