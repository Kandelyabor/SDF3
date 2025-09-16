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
 *  Function        :   Generate CSDF graphs
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: generate.cc,v 1.3.2.1 2010-04-25 01:21:16 mgeilen Exp $
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

#include "generate.h"
#include "csdf/base/timed/graph.h"
#include "csdf/transform/to_sdf/sdftocsdf.h"
namespace CSDF {

using namespace SDF;

namespace {

// Random number generator
MTRand mtRand; // NOLINT(*global-variables,cert-err58-cpp)

/**
 * distributeExecutionTimeOverPeriod ()
 * The function splits the execution time of the port in a sequence of length
 * period.
 */
void distributeExecutionTimeOverPeriod(const TimedCSDFactor& a, const uint period) {
    CSDFtimeSequence execTime;
    CSDFtime sdfExecTime = 0;

    for (auto iter = a.processorsBegin(); iter != a.processorsEnd(); iter++) {
        TimedCSDFactor::Processor *p = (*iter).get();

        // The SDF execution time of the actor
        sdfExecTime = static_cast<CSDFtime>(p->execTime[0]);

        // Distribute execution time over a sequence
        for (uint i = period - 1; i > 0; i--) {
            execTime.push_back(mtRand.rand(sdfExecTime));
            sdfExecTime = sdfExecTime - static_cast<CSDFtime>(execTime.back());
        }
        execTime.push_back(sdfExecTime);

        // Update the execution time of the processor
        p->execTime = execTime;
    }
}

/**
 * distributeRateOverPeriod ()
 * The function splits the rate of the port in a sequence of length period.
 */
void distributeRateOverPeriod(CSDFport& p, const uint period) {
    SDFrate sdfRate = p.getRate().at(0);
    CString rate;
    uint value = 0;

    // Distribute rate over a sequence
    for (uint i = period - 1; i > 0; i--) {
        value = mtRand.randInt(sdfRate);
        sdfRate -= value;

        if (i != 0) {
            rate = rate + ",";
        }
        rate = rate + CString(value);
    }
    if (period != 1) {
        rate = rate + ",";
    }
    rate = rate + CString(sdfRate);

    // Update the rate of the port
    p.setRate(CSDFrate(rate));
}

} // namespace

/**
 * generateCSDFgraph ()
 * Generate a random CSDF graph.
 */
std::unique_ptr<TimedCSDFgraph> generateCSDFgraph(const uint period,
                                  const uint nrActors,
                                  const double avgInDegree,
                                  const double varInDegree,
                                  const double minInDegree,
                                  const double maxInDegree,
                                  const double avgOutDegree,
                                  const double varOutDegree,
                                  const double minOutDegree,
                                  const double maxOutDegree,
                                  const double avgRate,
                                  const double varRate,
                                  const double minRate,
                                  const double maxRate,
                                  const bool acyclic,
                                  const bool stronglyConnected,
                                  const double initialTokenProp,
                                  const double tokenFreeProp,
                                  const bool tokenFreeCycle,
                                  const uint repetitionVectorSum,
                                  const bool execTime,
                                  const uint nrProcTypes,
                                  const double mapChance,
                                  const double avgExecTime,
                                  const double varExecTime,
                                  const double minExecTime,
                                  const double maxExecTime,
                                  const bool stateSize,
                                  const double avgStateSize,
                                  const double varStateSize,
                                  const double minStateSize,
                                  const double maxStateSize,
                                  const bool tokenSize,
                                  const double avgTokenSize,
                                  const double varTokenSize,
                                  const double minTokenSize,
                                  const double maxTokenSize,
                                  const bool throughputConstraint,
                                  const uint autoConcurrencyDegree,
                                  const double throughputScaleFactor,
                                  const bool bandwidthRequirement,
                                  const double avgBandwidth,
                                  const double varBandwidth,
                                  const double minBandwidth,
                                  const double maxBandwidth,
                                  const bool bufferSize,
                                  const bool latencyRequirement,
                                  const double avgLatency,
                                  const double varLatency,
                                  const double minLatency,
                                  const double maxLatency,
                                  const bool multigraph,
                                  const bool integerMCM) {

    // Create an SDFG
    std::unique_ptr<TimedSDFgraph> sdfGraph = generateSDFgraph(nrActors,
                                avgInDegree,
                                varInDegree,
                                minInDegree,
                                maxInDegree,
                                avgOutDegree,
                                varOutDegree,
                                minOutDegree,
                                maxOutDegree,
                                avgRate,
                                varRate,
                                minRate,
                                maxRate,
                                acyclic,
                                stronglyConnected,
                                initialTokenProp,
                                tokenFreeProp,
                                tokenFreeCycle,
                                repetitionVectorSum / period,
                                multigraph);

    // Add properties to the SDFG
    generateSDFgraphProperties(*sdfGraph,
                               execTime,
                               nrProcTypes,
                               mapChance,
                               avgExecTime,
                               varExecTime,
                               minExecTime,
                               maxExecTime,
                               stateSize,
                               avgStateSize,
                               varStateSize,
                               minStateSize,
                               maxStateSize,
                               tokenSize,
                               avgTokenSize,
                               varTokenSize,
                               minTokenSize,
                               maxTokenSize,
                               throughputConstraint,
                               autoConcurrencyDegree,
                               throughputScaleFactor,
                               bandwidthRequirement,
                               avgBandwidth,
                               varBandwidth,
                               minBandwidth,
                               maxBandwidth,
                               bufferSize,
                               latencyRequirement,
                               avgLatency,
                               varLatency,
                               minLatency,
                               maxLatency,
                               integerMCM);

    // Convert the SDFG to a CSDFG
    std::unique_ptr<TimedCSDFgraph> csdfGraph = convertSDFGtoCSDFG(*sdfGraph);

    // Distribute the rates and execution time over a sequence
    for (auto iter = csdfGraph->actorsBegin(); iter != csdfGraph->actorsEnd(); iter++) {
        const auto& a = dynamic_cast<const TimedCSDFactor&>(*(*iter));

        distributeExecutionTimeOverPeriod(a, period);

        for (const auto& p : a.getConstPorts()) {
            distributeRateOverPeriod(*p, period);
        }
    }

    // Done
    return csdfGraph;
}
} // namespace CSDF
