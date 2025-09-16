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
 https://github.com/bbodin/kiter
 *                      The algorithm is published in
 *                      Bruno Bodin, Alix Munier-Kordon, and Benoit Dupont de Dinechin. 2016.
                        Optimal and fast throughput evaluation of CSDF. In Proceedings of the
                        53rd Annual Design Automation Conference (DAC '16). Association for
                        Computing Machinery, New York, NY, USA, Article 160, 1-6.
                        https://doi.org/10.1145/2897937.2898056
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

#ifndef SDF_ANALYSIS_K_PERIODIC_THROUGHPUT_H_INCLUDED
#define SDF_ANALYSIS_K_PERIODIC_THROUGHPUT_H_INCLUDED

#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include "sdf/base/timed/graph.h"
#include "sdf/basic_types.h"


namespace SDF {
class kiterTask;

class kiterBuffer {
public:
    kiterBuffer(CId nId, uint nM0);
    CId id;
    uint M0;
    kiterTask *src;
    kiterTask *dst;
};

using kiterBuffers = std::list<std::unique_ptr<kiterBuffer>>;
using kiterBuffersIter = kiterBuffers::iterator;
using kiterBuffersCIter = kiterBuffers::const_iterator;
using kiterBufferRefs = std::list<kiterBuffer*>;

class kiterTask {
public:
    kiterTask(CId nId, uint nN, uint nK, uint nZ, uint nSelfLoop, uint duration);
    CId id;
    uint N;
    uint K;
    uint Z;
    uint selfLoop;
    uint duration;
    ::MaxPlus::Graphs::MCMnodeRefs nodes;
    kiterBufferRefs in;
    kiterBufferRefs out;
};

using kiterTasks = std::list<std::unique_ptr<kiterTask>>;
using kiterTasksIter = kiterTasks::iterator;
using kiterTasksCIter = kiterTasks::const_iterator;

class kiterGraph {
public:
    CId n_nodes;
    CId n_edges;

    kiterGraph();
    ~kiterGraph();

    // Delete copy constructor and copy assignment operator
    kiterGraph(const kiterGraph&) = delete;
    kiterGraph& operator=(const kiterGraph&) = delete;
    kiterGraph(kiterGraph&&) = delete;
    kiterGraph& operator=(kiterGraph&&) = delete;

    void addTask(std::unique_ptr<kiterTask> t) { this->tasks.push_back(std::move(t)); }

    void addBuffer(std::unique_ptr<kiterBuffer> b) {
        b->src->out.push_back(b.get());
        b->dst->in.push_back(b.get());
        this->buffers.push_back(std::move(b));
    }

    const kiterTasks &getTasks() { return tasks; };

    const kiterBuffers &getBuffers() { return buffers; };

private:
    kiterTasks tasks;
    kiterBuffers buffers;
};

class SDFkPeriodicAnalysis {
public:
    SDFkPeriodicAnalysis() = default;
    ~SDFkPeriodicAnalysis() = default;

    // Delete copy and move constructors and assignment operators
    SDFkPeriodicAnalysis(const SDFkPeriodicAnalysis&) = delete;
    SDFkPeriodicAnalysis& operator=(const SDFkPeriodicAnalysis&) = delete;
    SDFkPeriodicAnalysis(SDFkPeriodicAnalysis&&) = delete;
    SDFkPeriodicAnalysis& operator=(SDFkPeriodicAnalysis&&) = delete;

    static TDtime kPeriodic(const TimedSDFgraph& SDFgraph);

    static TDtime analyze(const TimedSDFgraph& SDFgraph);

private:
};
} // namespace SDF

#endif
