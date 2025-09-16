/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   latency.h
*
*  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
*
*  Date            :   May 30, 2018
*
*  Function        :   provides latency analysis functions for regex-based FSMSADF

*  History         :
*      12-09-17    :   Initial version.
*
* $Id:   $
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

#include "fsmsadf/analysis/throughput/thrutils.h"
#include "storagedist.h"
#include <memory>
#include <unordered_set>

namespace FSMSADF {
class BufferSizingScenarioUsingMaxPlus {
public:
    class Buffer {
    public:
        int id;
        Channel *bufferChannel;
        Channel *capacityChannel;

        std::vector<Channel *> dummyBufferChannels;

        TBufSize getminSzStep(const Scenario *s) {
            if (minSzStep.find(s) == minSzStep.end()) {
                return (minSzStep.begin())->second;
            }
            return minSzStep[s];
        }

        std::map<const Scenario *, TBufSize> minSzStep;
    };

    class BufferSet {
    public:
        void insertBuffer(std::unique_ptr<Buffer> b) { _buffers.push_back(std::move(b)); }
        [[nodiscard]] const std::vector<std::unique_ptr<Buffer>> &getBuffers() const {
            return _buffers;
        }

    private:
        std::vector<std::unique_ptr<Buffer>> _buffers;
    };

    explicit BufferSizingScenarioUsingMaxPlus(Graph *gr) : g(gr) {}

    void initialize();

    // CString getChannelName(CString scenarioName, int channelId);

    TDtime throughputAndCriticalBuffers(
            const ScenarioMatrices &scenarioMatrices,
            std::map<CString, std::unordered_set<int>> &criticalCapacityChannels) const;

    TBufSize minSzStep(std::set<CString> &scenarios, int bufferId) const;

    // translate storage distributions to initial tokens on the edges
    void transformDistributionToInitialTokensOnCapacityChannels(StorageDistributionMP &d) const;

    [[nodiscard]] std::unique_ptr<ScenarioMatrices> computeExtendedMatrixRepresentations() const;
    // Matrix* computeMatrixRepresentation();//just for test, should be removed
    // ScenarioGraph* BufferSizingScenarioUsingMaxPlus::addCapacityChannelsToScenarioGraphs(Channels
    // capacityChannels, map <int, int> nrInitTokens, Scenario *s);//, map<unsigned int, unsigned
    // int> &capacityChannelMap);

    // StorageDistributionMPSet *findNextStorageDistributionSet();

    std::unique_ptr<StorageDistributionMPSet> analyze(double thrBound);

    // Bounds on the search space
    void initBoundsSearchSpace();
    void initMinimalChannelSzStep() const;
    void initMinimalChannelSz();
    void initLbDistributionSz();
    void initMaxThroughput(const Graph &g);

    // Storage distributions
    [[nodiscard]] std::unique_ptr<StorageDistributionMP> newStorageDistribution() const;
    void execStorageDistribution(StorageDistributionMP &d) const;
    static void minimizeStorageDistributionsSet(StorageDistributionMPSet &ds);
    bool addStorageDistributionToChecklist(std::unique_ptr<StorageDistributionMP> d);
    void exploreStorageDistribution(StorageDistributionMPSet &ds, StorageDistributionMP &d);
    void exploreStorageDistributionSet(StorageDistributionMPSet &ds);
    void findMinimalStorageDistributions(double thrBound);

    // Storage distributions
    std::unique_ptr<StorageDistributionMPSet> minStorageDistributions;
    StorageDistributionMPSet *lastExploredStorageDistributionSet{};

    // Bounds on the search space
    std::unique_ptr<TBufSize[]> minSz;
    // map < CString, TBufSize> *minSzStep;
    TBufSize lbDistributionSz{};
    TDtime maxThroughput{};

    // Check if throughput th is equal to maximal throughput
    static inline bool checkAgainstMaxThroughput(double th, double maxTh, double margin);

    // static parameters
    // accuracy of throughput comparisons
    // initialized to 1e-10 in the .cc file (because VS wants it)
    static const double ThroughputMargin;

    static int nrThroughputCalls;

    // common between all scenarios map<channelName,map<scenarioName,CapacitychannelID>>
    // map<CString,map<CString,int>> bufferChannels;

    // distribution id map
    // map<Buffer*, int> distributionIdMap;

    Graph *g;

    BufferSet fsmsadfBuffers;
};
} // namespace FSMSADF