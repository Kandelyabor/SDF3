/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Tile binding and scheduling
 *
 *  History         :
 *      29-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 * $Id: binding.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_BINDING_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_BINDING_H_INCLUDED

#include "graph_binding.h"
#include <memory>

namespace FSMSADF {

/**
 * Tile binding and scheduling
 */
class TileBindingAlgoBase {
public:
    // Cluster Constraints value
    enum class CCValue {
        None = 0,
        Inc,
        Exc,
    };

    // Constructor
    TileBindingAlgoBase();

    // Destructor
    virtual ~TileBindingAlgoBase();

    TileBindingAlgoBase(const TileBindingAlgoBase &) = delete;
    TileBindingAlgoBase &operator=(const TileBindingAlgoBase &) = delete;
    TileBindingAlgoBase(TileBindingAlgoBase &&) = delete;
    TileBindingAlgoBase &operator=(TileBindingAlgoBase &&) = delete;

    // Override this function to get an alternative GraphBinding algorithm.
    virtual std::unique_ptr<GraphBindingBase> createGraphBinding(const PlatformBinding &pb);

    // Initialize algorithm
    void init(BindingMethod bm);

    // Estimate bandwidth requirement
    bool estimateBandwidthConstraint();

    // Bind graph to tiles
    bool bindGraphToTiles(uint maxNrAppBindings);

    // Estimate connection delay
    bool estimateConnectionDelay();

    // Construct schedules
    virtual bool constructTileSchedules(uint maxNrAppBindings);

    ////////////////
    // Accessors //
    //////////////

    // Application graph
    [[nodiscard]] Graph *getApplicationGraph() const { return applicationGraph; };
    void setApplicationGraph(Graph *g) { applicationGraph = g; };

    // Platform graph
    [[nodiscard]] PlatformGraph *getPlatformGraph() const { return platformGraph; };
    void setPlatformGraph(PlatformGraph *g) { platformGraph = g; };

    // Platform bindings
    [[nodiscard]] PlatformBindings *getPlatformBindings() const { return platformBindings; };
    void setPlatformBindings(PlatformBindings *b) { platformBindings = b; };
    [[nodiscard]] PlatformBinding *getPlatformBinding(const CString &name) const;
    [[nodiscard]] PlatformBinding *getInitialPlatformBinding() const;

    CCValue getClusterConstraint(const Scenario *s, const CString &a, const CString &b) const {
        if (clusterConstraints.find(s) != clusterConstraints.end()) {
            std::map<CString, std::map<CString, CCValue>> scenario =
                    clusterConstraints.find(s)->second;
            if (scenario.find(a) != scenario.end()) {
                std::map<CString, CCValue> actor = scenario.find(a)->second;
                if (actor.find(b) != actor.end()) {
                    return actor.find(b)->second;
                }
            }
        }
        return CCValue::None;
    };

protected:
    virtual bool isThroughputConstraintSatisfied(const PlatformBinding &pb);
    // Throughput
    virtual Throughput analyzeThroughput(const PlatformBinding &pb);

    // Sort scenarios on occurrence count
    static std::list<ScenarioRef> sortScenariosOnOccurrenceCount(Graph &g);

    // Allocate actor binding
    bool allocateActorBinding(GraphBindingBase *applicationBinding,
                              CString &name,
                              ActorBinding &actorBinding);

    // Release actor binding
    void releaseActorBinding(GraphBindingBase *applicationBinding, CString &name);

    // Allocate channel binding
    bool allocateChannelBinding(GraphBindingBase *applicationBinding,
                                const CString &name,
                                ChannelBinding &channelBinding);

    // Release channel binding
    void releaseChannelBinding(GraphBindingBase *applicationBinding, CString &name);

    // Find all possible actor bindings given existing (partial) binding
    ActorBindings findActorBindingCandidates(GraphBindingBase *applicationBinding, Actor *a);

    // Find all possible channel bindings given existing (partial) binding
    ChannelBindings findChannelBindingCandidates(GraphBindingBase *applicationBinding, Channel *c);

    // Check wether actor with same name has been bound already
    static bool hasActorBindingBeenFixed(GraphBindingBase *applicationBinding, const CString &name);

    // Check whether an actor within the same constraint has been bound already
    ActorBinding *hasConstraintBindingBeenFixed(GraphBindingBase *applicationBinding,
                                                const CString &name) const;

    // Check if this processor is already used for an excluded cluster
    bool isProcessorExcluded(Processor *p,
                             GraphBindingBase *applicationBinding,
                             const CString &name) const;

    // Get an actor binding for an actor with the supplied name
    static ActorBinding getActorBinding(GraphBindingBase *applicationBinding, const CString &name);

    // Check wether channel with same name has been bound already
    static bool hasChannelBindingBeenFixed(GraphBindingBase *applicationBinding,
                                           const CString &name);

    // Get a channel binding for a channel with the supplied name
    static ChannelBinding getChannelBinding(GraphBindingBase *applicationBinding,
                                            const CString &name);

    // Create all valid bindings of the actor name to the platform
    void createAllBindingsOfActor(CString &name, std::unique_ptr<GraphBindings> &bindings);

    // Create a binding of the channel name to the platform
    bool createBindingOfChannel(const CString &name, GraphBindingBase *b);

    // Pareto minimization of application bindings
    static void
    paretoMinimizationOfApplicationBindings(std::unique_ptr<GraphBindings> &applicationBindings);

    // Reduce the number of application bindings
    static void reduceApplicationBindings(std::unique_ptr<GraphBindings> &applicationBindings,
                                          uint maxNrAppBindings);

    // /Reduce the number of TDM slots on each processor
    void reduceSlotsPerProcessor(GraphBindings *applicationBindings, ProcessorRefs& processors);

    // Replace the platform bindings
    void replacePlatformBindings(const GraphBindings &newBindings);

    // Sort actor names on criticality
    std::list<CString> sortActorNamesOnCriticality();

    // Scenario occurrence count
    static std::map<ScenarioRef, uint> computeScenariosOccurrenceCount(Graph *g);

    // Collect cluster constraints in 2-d matrix
    void initClusterConstraints();

    // Get the cluster constraints for a pair of actors
    CCValue getClusterConstraintForActor(Scenario *s, const CString &a, const CString &b) const;

    // Initialize the relation between the application and platform graph
    void initGraphRelations();

    RepetitionVector& getRepetitionVectorForScenario(const Scenario* s){ return this->repetitionVectors[s];};

    std::map<const Scenario *, RepetitionVector>& getRepetitionVectors(){ return this->repetitionVectors;};

private:
    // Application graph
    Graph *applicationGraph;

    // Platform graph
    PlatformGraph *platformGraph;

    // Platform bindings
    PlatformBindings *platformBindings;

    // Repetition vectors
    std::map<const Scenario *, RepetitionVector> repetitionVectors;

    // Actor names to actors
    std::map<CString, ActorRefs> actorNamesToActors;

    // Actors to scenarios
    std::map<ActorRef, ScenarioRefs> actorToScenarios;

    // Actor names to processor candidates
    std::map<CString, ProcessorRefs> actorNamesToProcessorCandidates;

    // Channel names to channels
    std::map<CString, ChannelRefs> channelNamesToChannels;

    // Channels to scenarios
    std::map<ChannelRef, ScenarioRefs> channelToScenarios;

    // Matrix with clustering constraints in format: [scenario][actor][actor][ClusterConstraint]
    using ClusterConstraints =
            std::map<const Scenario *, std::map<CString, std::map<CString, CCValue>>>;

    ClusterConstraints clusterConstraints;
};

} // End namespace FSMSADF

#endif
