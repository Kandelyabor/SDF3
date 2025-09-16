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
 *
 * $Id: binding.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "binding.h"
#include <cmath>
#include <memory>

namespace FSMSADF {

/**
 * TileBindingAlgo()
 * Constructor.
 */
TileBindingAlgoBase::TileBindingAlgoBase() :
    applicationGraph(nullptr), platformGraph(nullptr), platformBindings(nullptr) {}

/**
 * ~TileBindingAlgo()
 * Destructor.
 */
TileBindingAlgoBase::~TileBindingAlgoBase() = default;

/**
 * initialize()
 * The function initializes the tile binding and scheduling algorithm.
 */
void TileBindingAlgoBase::init(BindingMethod bm) {
    // Has application graph?
    if (applicationGraph == nullptr) {
        throw CException("No application graph given.");
    }

    // Has platform graph?
    if (platformGraph == nullptr) {
        throw CException("No platform graph given.");
    }

    // Has list of platform bindings?
    if (platformBindings == nullptr) {
        throw CException("No list of platform bindings given.");
    }

    // Application graph has no isolated scenarios?
    if (!applicationGraph->hasIsolatedScenarios()) {
        throw CException("Graph has no isolated scenarios.");
    }

    // No platform bindings given?
    if (platformBindings->empty()) {
        // Create an empty initial platform binding
        platformBindings->push_back(std::make_unique<PlatformBinding>(
                GraphComponent(nullptr, 0, "initial"), platformGraph, applicationGraph, bm));
    }

    // Initialize the relation between the application and platform graph
    initGraphRelations();

    initClusterConstraints();
}

/**
 * getPlatformBinding()
 * The function returns a pointer to a platform binding object with the
 * specified name. An exception is thrown when no such object exists.
 */
PlatformBinding* TileBindingAlgoBase::getPlatformBinding(const CString &name) const {
    for (const auto& pb : *platformBindings) {
        if (pb->getName() == name) {
            return pb.get();
        }
    }

    throw CException("No platform binding exists with name '" + name + "'");
}

/**
 * getInitialPlatformBinding()
 * The function returns a pointer to the initial platform binding object. An
 * exception is thrown when no such object exists.
 */
PlatformBinding* TileBindingAlgoBase::getInitialPlatformBinding() const {
    for (const auto& pb : *platformBindings) {
        if (pb->isInitialBinding()) {
            return pb.get();
        }
    }

    throw CException("No initial platform binding exists");
}

/**
 * analyzeThroughput ()
 * The function returns the throughput of the application graph
 * mapped onto the platform graph.
 */
Throughput TileBindingAlgoBase::analyzeThroughput(const PlatformBinding& /*pb*/) {
    throw CException("This function should be implemented by the platform specific implementation");
    return 0;
}

/**
 * isThroughputConstraintSatisfied ()
 * Check wether or not the throughput constraint is satisfied.
 */
bool TileBindingAlgoBase::isThroughputConstraintSatisfied(const PlatformBinding& pb) {
    Throughput thrGraph = 0.0;
    Throughput thrConstraint = 0.0;

    // Compute throughput of mapped graph
    thrGraph = analyzeThroughput(pb);
    thrConstraint = applicationGraph->getThroughputConstraint();

    // Output throughput
    logMsg("Throughput of binding: " + CString(thrGraph));

    // Is throughput constraint met?
    if (thrConstraint > thrGraph) {
        logInfo("Binding does not meet throughput constraint.");
        return false;
    }

    logInfo("Binding satisfies throughput constraint.");

    return true;
}

/**
 * estimateBandwidthConstraint()
 * Estimate the required bandwidth for each channel in the application graph.
 */
bool TileBindingAlgoBase::estimateBandwidthConstraint() {
    // Iterate over all platform bindings (except the initial binding)
    for (const auto& pb : *platformBindings) {
        GraphBindingConstraints *gb = pb->getGraphBindingConstraints();

        if (pb->isInitialBinding()) {
            continue;
        }

        // Estimate bandwidth for binding b
        logMsg("Estimate bandwidth for binding '" + pb->getName() + "'");

        // Iterate over the scenario graphs
        for (const auto& s : applicationGraph->getScenarios()) {
            ScenarioGraph *sg = s->getScenarioGraph();

            // Compute repetition vector of the scenario graph sg in scenario s
            RepetitionVector repVec = s->computeRepetitionVector();

            // Iterate over the channels
            for (const auto& c : sg->getChannels()) {
                /*ceil*/
                Bandwidth b = (c->getSrcPort()->getRateOfScenario(*s)
                               * repVec[c->getSrcActor()->getId()] * c->getTokenSizeOfScenario(*s)
                               * applicationGraph->getThroughputConstraint());

                gb->getConstraintsOfScenario(*s)
                        ->getConstraintsOfChannel(c->getName())
                        ->setBandwidth(b);

                logMsg("Bandwidth channel '" + c->getName() + "' in scenario graph '"
                       + sg->getName() + "': " + CString(b) + " bytes/time-units");
            }
        }
    }

    return true;
}

/**
 * bindGraphToTiles()
 * Bind the application graph to the platform tiles. The function produces
 * Pareto trade-off space with different platform bindings. The number of
 * bindings are outputted by the function does not exceed the maxNrAppBindings.
 */
bool TileBindingAlgoBase::bindGraphToTiles(const uint maxNrAppBindings) {
    auto applicationBindings = std::make_unique<GraphBindings>();
    std::list<CString> sortedActorNames;

    // Iterate over all platform bindings (except the initial binding)
    for (const auto& pb : *platformBindings) {

        if (pb->isInitialBinding()) {
            continue;
        }

        // Create tile binding for binding b
        logMsg("Creating tile binding for binding '" + pb->getName() + "'");

        // Create an application binding given the platform binding pb
        applicationBindings->push_back(createGraphBinding(*pb));

        // Sort actor names on criticality
        sortedActorNames = sortActorNamesOnCriticality();

        // Iterate over the actor names in order of criticality
        for (auto &an : sortedActorNames) {

            // Is there at least one application binding?
            if (applicationBindings->empty()) {
                return false;
            }

            // Create all bindings for the actor
            createAllBindingsOfActor(an, applicationBindings);

            // Iterate over all partial application bindings
            for (auto j = applicationBindings->begin(); j != applicationBindings->end();) {
                GraphBindingBase* b = j->get();
                bool validBinding = true;

                // Iterate over the channel names in arbitrary order
                for (auto &[cn, ch] : channelNamesToChannels) {

                    // Try to bind channel with name cn to platform
                    if (!createBindingOfChannel(cn, b)) {
                        // All possible bindings of channel to platform have a
                        // resource conflict. So, binding b cannot be realized
                        validBinding = false;
                        break;
                    }
                }

                // Channel to platform binding is not possible?
                if (!validBinding) {
                    // Erase binding and continue with next binding
                    applicationBindings->erase(j++);
                } else {
                    // Compute cost of this binding
                    b->computeParetoQuantities(repetitionVectors, false);

                    // Next binding
                    j++;
                }
            }

            // Remove all non-Pareto optimal bindings
            paretoMinimizationOfApplicationBindings(applicationBindings);

            // Reduce the number of application bindings
            reduceApplicationBindings(applicationBindings, maxNrAppBindings);
        }
    }

    // Insert new application bindings to the list of platform bindings
    replacePlatformBindings(*applicationBindings);

    return true;
}

/**
 * estimateConnectionDelay()
 * Estimate the channel delay of channels which have been mapped to a
 * connection.
 */
bool TileBindingAlgoBase::estimateConnectionDelay() {
    // Iterate over all platform bindings (except the initial binding)
    for (const auto& pb : *platformBindings) {
        GraphBindingConstraints *gb = pb->getGraphBindingConstraints();

        if (pb->isInitialBinding()) {
            continue;
        }

        // Estimate connection delay for binding b
        logInfo("Estimate connection delay for binding '" + pb->getName() + "'");

        // Iterate over all scenarios
        for (const auto& s : applicationGraph->getScenarios()) {
            ScenarioGraph *sg = s->getScenarioGraph();

            // Iterate over all channels
            for (const auto& c : sg->getChannels()) {
                gb->getConstraintsOfScenario(*s)->getConstraintsOfChannel(c->getName())->setDelay(0);

                logMsg("Delay of channel '" + c->getName() + "' in scenario" + +" graph '"
                       + sg->getName() + "': 0" + " time-units");
            }
        }
    }

    return true;
}

/**
 * constructTileSchedules()
 * Construct static-order and TDMA schedules for the application on all platform
 * tiles.
 */
bool TileBindingAlgoBase::constructTileSchedules(const uint /*maxNrAppBindings*/) {
    throw CException("This function should be implemented by the platform specific implementation");
    return false;
}

/**
 * allocateActorBinding()
 * The function creates an actor bindings for the actor with the supplied name.
 */
bool TileBindingAlgoBase::allocateActorBinding(GraphBindingBase *applicationBinding,
                                               CString &name,
                                               ActorBinding &actorBinding) {
    // Iterate over the actor with the specified name
    for (const auto& a : actorNamesToActors[name]) {
        // Iterate over all scenarios in which the actor appears
        for (const auto& s : actorToScenarios[a]) {
            if (!applicationBinding->bindActorToTile(*s, a, actorBinding)) {
                return false;
            }
        }
    }

    return true;
}

/**
 * releaseActorBinding()
 * The function removes the specified actor binding.
 */
void TileBindingAlgoBase::releaseActorBinding(GraphBindingBase *applicationBinding, CString &name) {
    // Iterate over the actor with the specified name
    for (const auto& a : actorNamesToActors[name]) {
        // Iterate over all scenarios in which the actor appears
        for (const auto& s : actorToScenarios[a]) {
            applicationBinding->removeActorToTileBinding(*s, a);
        }
    }
}

/**
 * allocateChannelBinding()
 * The function creates a channel binding for all channels which match the
 * supplied name.
 */
bool TileBindingAlgoBase::allocateChannelBinding(GraphBindingBase *applicationBinding,
                                                 const CString &name,
                                                 ChannelBinding &channelBinding) {
    // Iterate over the channel with the specified name
    for (const auto& c : channelNamesToChannels[name]) {
        // Iterate over all scenarios in which the channel appears
        for (const auto& s : channelToScenarios[c]) {
            // Bind channel to connection?
            if (channelBinding.connection != nullptr) {
                if (!applicationBinding->bindChannelToConnection(*s, c, channelBinding)) {
                    return false;
                }
            }
            // Bind channel to tile?
            else if (channelBinding.connToMem == nullptr) {
                if (!applicationBinding->bindChannelToTile(*s, c, channelBinding)) {
                    return false;
                }
            }
            // Bind channel to shared memory?
            else {
                if (!applicationBinding->bindChannelToSharedMemory(*s, c, channelBinding)) {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * releaseChannelBinding()
 * The function removes the specified channel binding.
 */
void TileBindingAlgoBase::releaseChannelBinding(GraphBindingBase *applicationBinding,
                                                CString &name) {
    // Iterate over the channel with the specified name
    for (const auto& c : channelNamesToChannels[name]) {
        // Iterate over all scenarios in which the channel appears
        for (const auto& s : channelToScenarios[c]) {
            applicationBinding->removeChannelBinding(*s, c);
        }
    }
}

/**
 * findActorBindingCandidates()
 * The function returns a list with all possible actor bindings. Note that
 * it is not guaranteed that sufficient resources are available to actually
 * create a binding.
 */
ActorBindings TileBindingAlgoBase::findActorBindingCandidates(GraphBindingBase *applicationBinding,
                                                              Actor *a) {
    ActorBindings bindings;

    // Has an actor with the same name already been mapped?
    if (hasActorBindingBeenFixed(applicationBinding, a->getName())) {
        bindings.push_back(getActorBinding(applicationBinding, a->getName()));
        return bindings;
    }

    // Has an actor that is constrained on the same processor already been mapped?
    ActorBinding *ab = hasConstraintBindingBeenFixed(applicationBinding, a->getName());
    if (ab != nullptr) {
        bindings.push_back(*ab);
        return bindings;
    }

    // Iterate over all possible processors
    for (const auto& p : actorNamesToProcessorCandidates[a->getName()]) {
        Tile *t = p->getTile();

        // Check if an exclusion constraint covers an already mapped actor
        if (!isProcessorExcluded(p, applicationBinding, a->getName())) {
            // Iterate over all memories within the tile
            for (const auto& m : t->getMemories()) {
                bindings.emplace_back(p, m.get());
            }
        }
    }

    return bindings;
}

/**
 * findChannelBindingCandidates()
 * The function returns a list with all possible channel bindings. Note that
 * it is not guaranteed that sufficient resources are available to actually
 * create a binding. Channel binding candidates can only be created when the
 * source and destination actor of the channel are already mapped to a
 * processor.
 */
ChannelBindings
TileBindingAlgoBase::findChannelBindingCandidates(GraphBindingBase *applicationBinding,
                                                  Channel *c) {
    Tile *srcTile = nullptr;
    Tile *dstTile = nullptr;
    ChannelBindings bindings;

    // Has a channel with the same name already been mapped?
    if (hasChannelBindingBeenFixed(applicationBinding, c->getName())) {
        bindings.push_back(getChannelBinding(applicationBinding, c->getName()));
        return bindings;
    }

    // Are the source actor and destination actor bound to a tile?
    if (hasActorBindingBeenFixed(applicationBinding, c->getSrcActor()->getName())
        && hasActorBindingBeenFixed(applicationBinding, c->getDstActor()->getName())) {
        srcTile = getActorBinding(applicationBinding, c->getSrcActor()->getName())
                          .processor->getTile();
        dstTile = getActorBinding(applicationBinding, c->getDstActor()->getName())
                          .processor->getTile();
    } else {
        return bindings;
    }

    // Are the source and destination tile equal?
    if (srcTile == dstTile) {
        // Iterate over all memories within the tile
        for (const auto& m : srcTile->getMemories()) {
            bindings.push_back(ChannelBinding(m.get()));
        }

    } else {
        // Create all possible NI, connection and memory combinations
        for (const auto& nSrc : srcTile->getNetworkInterfaces()) {
            // Iterate over the outgoing connections
            for (const auto& co : nSrc->getOutConnections()) {
                NetworkInterface *nDst = co->getDstNetworkInterface();
                // Connection to the correct destination tile?
                if (nDst->getTile() == dstTile) {
                    // Iterate over all memories in the source tile
                    for (const auto& mSrc : srcTile->getMemories()) {
                        // Iterate over all memories in the destination tile
                        for (const auto& mDst : dstTile->getMemories()) {
                            // Add channel binding
                            bindings.push_back(ChannelBinding(mSrc.get(), nSrc.get(), mDst.get(), nDst, co));
                        }
                    }
                }
            }
        }
    }

    // Add a binding to each shared memory
    for (const auto& sharedTile : platformGraph->getTiles()) {
        if (sharedTile->hasSharedMemories()) {
            for (const auto& nToSrc : srcTile->getNetworkInterfaces()) {
                // Iterate over the outgoing connections
                for (const auto& toMem : nToSrc->getOutConnections()) {
                    NetworkInterface *nToDst = toMem->getDstNetworkInterface();
                    // Connection to the correct destination tile?
                    if (nToDst->getTile() == sharedTile.get()) {
                        // Iterate over all memories in the source tile
                        for (const auto& mSrc : srcTile->getMemories()) {
                            for (const auto& nToDst : dstTile->getNetworkInterfaces()) {
                                // Iterate over the outgoing connections
                                for (const auto& fromMem : nToDst->getOutConnections()) {
                                    NetworkInterface *nFromDst = fromMem->getDstNetworkInterface();
                                    // Connection to the correct destination tile?
                                    if (nFromDst->getTile() == sharedTile.get()) {
                                        // Iterate over all memories in the destination tile
                                        for (const auto& mDst : dstTile->getMemories()) {
                                            // Iterate over all memories in the shared memory tile
                                            std::unique_ptr<MemoryRefs> sharedMemories =
                                                    sharedTile->getSharedMemories();
                                            for (auto *mMem : *sharedMemories) {
                                                // Add channel binding
                                                bindings.emplace_back(mSrc.get(),
                                                                                  nToSrc.get(),
                                                                                  mDst.get(),
                                                                                  nToDst.get(),
                                                                                  mMem,
                                                                                  toMem,
                                                                                  fromMem);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return bindings;
}

/**
 * hasActorBindingBeenFixed()
 * The function returns true when an actor with the supplied name has been
 * bound to the platform in at least one scenario. Otherwise the function
 * return false.
 */
bool TileBindingAlgoBase::hasActorBindingBeenFixed(GraphBindingBase *applicationBinding,
                                                   const CString &name) {
    // Iterate over the actor bindings in the application binding
    for (auto& [sc, binding] : applicationBinding->getActorBindings()) {
        for (auto& [a, ab] : binding) {
            if (a->getName() == name) {
                return true;
            }
        }
    }

    return false;
}

/**
 * hasConstraintBindingBeenFixed()
 * The function returns a binding when an actor with the supplied name has
 * a contraint that forces it on the same processor as another actor,
 * and that other actor has already been mapped. Otherwise the function
 * returns a nullptr pointer.
 */
ActorBinding *
TileBindingAlgoBase::hasConstraintBindingBeenFixed(GraphBindingBase *applicationBinding,
                                                   const CString &name) const {
    // Iterate over the scenarios in the actors binding
    for (auto& [s, binding] : applicationBinding->getActorBindings()) {
        // Iterate over all actors
        for (const auto& ac : s->getScenarioGraph()->getActors()) {
            CString a = ac->getName();

            // Check if constraint exists
            if (getClusterConstraint(s, name, a) == CCValue::Inc) {
                // Find if other actor was already mapped
                for (auto& [act, ab] : binding) {
                    if (act->getName() == a) {
                        return &(ab);
                    }
                }
            }
        }
    }
    return nullptr;
}

/**
 * isProcessorExcluded()
 * The function checks if actors from a cluster that is not allowed to
 * be mapped to the same processor as actor 'name' (exclusion constraint)
 * is already mapped to the given processor. If so, true is returned.
 */
bool TileBindingAlgoBase::isProcessorExcluded(Processor *p,
                                              GraphBindingBase *applicationBinding,
                                              const CString &name) const {
    // Iterate over the scenarios in the actors binding
    for (auto& [s, binding] : applicationBinding->getActorBindings()) {
        // Iterate over all actors
        for (const auto& ac : s->getScenarioGraph()->getActors()) {
            CString a = ac->getName();

            // Check if actor 'name' should exclude 'a'
            if (getClusterConstraint(s, name, a) == CCValue::Exc) {
                // Find if other actor was already mapped to this proc
                for (auto& [act, ab] : binding) {
                    if ((act->getName() == a) && (ab.processor == p)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/**
 * getActorBinding()
 * The function returns an actor binding of an actor with the supplied name.
 * When no binding of an actor with this name exists, an exception is thrown.
 */
ActorBinding TileBindingAlgoBase::getActorBinding(GraphBindingBase *applicationBinding,
                                                  const CString &name) {
    for (auto& [sc, binding] : applicationBinding->getActorBindings()) {
        for (auto& [a, ab] : binding) {
            if (a->getName() == name) {
                return ab;
            }
        }
    }

    throw CException("Actor with name '" + name + "' has not been bound.");
}

/**
 * hasChannelBindingBeenFixed()
 * The function returns true when a channel with the supplied name has been
 * bound to the platform in at least one scenario. Otherwise the function
 * return false.
 */
bool TileBindingAlgoBase::hasChannelBindingBeenFixed(GraphBindingBase *applicationBinding,
                                                     const CString &name) {
    // Iterate over the channel bindings in the application binding
    for (auto& [sc, cBinding] : applicationBinding->getChannelBindings()) {
        for (auto& [c, j_s] : cBinding) {
            if (c->getName() == name) {
                return true;
            }
        }
    }
    return false;
}

/**
 * getChannelBinding()
 * The function returns a channel binding of a channel with the supplied name.
 * When no binding of a channel with this name exists, an exception is thrown.
 */
ChannelBinding TileBindingAlgoBase::getChannelBinding(GraphBindingBase *applicationBinding,
                                                      const CString &name) {
    for (auto& [sc, cBinding] : applicationBinding->getChannelBindings()) {
        for (auto& [c, binding] : cBinding) {
            if (c->getName() == name) {
                return binding;
            }
        }
    }

    throw CException("Channel with name '" + name + "' has not been bound.");
}

/**
 * initGraphRelations()
 * Initialize the relations between the application and platform graph.
 */
void TileBindingAlgoBase::initGraphRelations() {
    // Clear existing repetition vectors
    repetitionVectors.clear();

    // Clear existing mapping of actors names to actors
    actorNamesToActors.clear();

    // Clear existing mapping of actors to scenarios
    actorToScenarios.clear();

    // Clear existing mapping of actor names to processor candidates
    actorNamesToProcessorCandidates.clear();

    // Clear existing mapping of channel names to channels
    channelNamesToChannels.clear();

    // Clear existing mapping of channels to scenarios
    channelToScenarios.clear();

    // Repetition vectors
    for (const auto& s : applicationGraph->getScenarios()) {
        repetitionVectors[s.get()] = s->computeRepetitionVector();
    }

    // Mapping of actor names to actors
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        for (const auto& a : sg->getActors()) {
            actorNamesToActors[a->getName()].push_back(a.get());
            actorToScenarios[a.get()].insert(s.get());
        }
    }

    // Mapping of actor names to processor binding candidates
    for (auto& [actName, act] : actorNamesToActors) {
        std::set<CString> procTypes;

        // Compute set of processor types to which actor with given name can
        // always be mapped.
        for (auto j = act.begin(); j != act.end(); j++) {
            std::set<CString> types;
            Actor* a = *j;

            // Create set of all processor types supported by this actor
            for (const auto &k : a->getProcessorTypes()) {
                types.insert(k.type);
            }

            // Take union of processor types supported by this actor and by
            // actors with the same name
            if (j == act.begin()) {
                procTypes = types;
            } else {
                auto s1 = types.begin();
                auto s2 = procTypes.begin();
                std::set<CString> tmp;

                while (s1 != types.end() && s2 != procTypes.end()) {
                    if (*s1 < *s2) {
                        ++s1;
                    } else if (*s2 < *s1) {
                        ++s2;
                    } else {
                        tmp.insert(*s1++);
                        s2++;
                    }
                }

                procTypes = std::set<CString>(tmp.begin(), tmp.end());
            }
        }

        // Find all processors which are of a type inside the set procTypes
        for (const auto& t : platformGraph->getTiles()) {
            for (const auto& p : t->getProcessors()) {
                // Is the processor of a type supported by the actor?
                if (procTypes.find(p->getType()) != procTypes.end()) {
                    actorNamesToProcessorCandidates[actName].push_back(p.get());
                }
            }
        }
    }

    // Mapping of channel names to channels
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        for (const auto& c : sg->getChannels()) {
            channelNamesToChannels[c->getName()].push_back(c.get());
            channelToScenarios[c.get()].insert(s.get());
        }
    }
}

/**
 * sortActorNamesOnCriticality()
 * The function returns a list of actor names in which those names have been
 * ordered (high-to-low) based on the criticality of the actors.
 */
std::list<CString> TileBindingAlgoBase::sortActorNamesOnCriticality() {
    std::map<ScenarioRef, uint> scenarioCnt;
    std::map<ScenarioRef, std::map<Actor *, Time>> actorExecTime;
    std::map<CString, double> actorCost;
    std::list<CString> actorNames;

    // Compute scenario occurrence count
    scenarioCnt = computeScenariosOccurrenceCount(applicationGraph);

    // Find maximum scenario occurrence count over all scenarios
    uint maxScenarioCnt = 0;
    for (auto& [sc, n] : scenarioCnt) {
        maxScenarioCnt = (std::max)(maxScenarioCnt, n);
    }
    ASSERT(maxScenarioCnt != 0, "max cannot be zero.");

    // Find maximum repetition vector entry
    uint maxActorFiringCnt = 0;
    for (auto& [sc, repVec] : repetitionVectors) {
        for (auto j : repVec) {
            maxActorFiringCnt = (std::max)(maxActorFiringCnt, static_cast<uint>(j));
        }
    }
    ASSERT(maxActorFiringCnt != 0, "max cannot be zero.");

    // Compute maximum actor execution time
    Time maxActorExecTime = 0;
    for (const auto& s : applicationGraph->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();

        for (const auto& a : sg->getActors()) {
            // Iterate over the processor types
            actorExecTime[s.get()][a.get()] = 0;
            for (const auto &p : a->getProcessorTypes()) {

                // Maximum execution time of actor a in scenario s
                actorExecTime[s.get()][a.get()] =
                        (std::max)(actorExecTime[s.get()][a.get()], a->getExecutionTimeOfScenario(*s, p.type));

                // Maximum actor execution time over all actors and scenarios
                maxActorExecTime = (std::max)(maxActorExecTime, actorExecTime[s.get()][a.get()]);
            }
        }
    }
    ASSERT(maxActorExecTime != 0, "max cannot be zero.");

    // Compute actor cost
    for (auto& [an, actors] : actorNamesToActors) {

        // Initialize cost to zero
        actorCost[an] = 0;

        // Iterate over all actors with this name
        for (const auto& a : actors) {

            // Iterate over all scenarios in which this actor appears
            for (const auto& s : actorToScenarios[a]) {
                actorCost[an] = actorCost[an]
                                + ((scenarioCnt[s] / static_cast<double>(maxScenarioCnt))
                                   * (repetitionVectors[s][a->getId()]
                                      / static_cast<double>(maxActorFiringCnt))
                                   * (actorExecTime[s][a] / static_cast<double>(maxActorExecTime)));
            }
        }
    }

    // Sort actors on cost
    for (auto& [a, c] : actorCost) {
        auto j = actorNames.end();

        // Find first actor j in list with lower occurrence count than i
        while (j != actorNames.end() && actorCost[*j] > c) {
            j++;
        }

        // Add actor i to the list before actor j
        actorNames.insert(j, a);
    }

    return actorNames;
}

/**
 * computeScenariosOccurrenceCount()
 * The function returns the occurrence frequency of the scenarios in the FSM.
 */
std::map<ScenarioRef, uint> TileBindingAlgoBase::computeScenariosOccurrenceCount(Graph *g) {
    std::map<ScenarioRef, uint> scenarioCnt;

    // No FSM specified?
    if (g->getFSM() == nullptr) {
        scenarioCnt[g->getDefaultScenario()] = 1;
        return scenarioCnt;
    }

    // Iterate over the states in the FSM
    for (const auto& s : g->getFSM()->getStates()) {
        scenarioCnt[s->getScenario()]++;
    }

    return scenarioCnt;
}

/**
 * sortScenariosOnOccurrenceCount()
 * The function returns a list where the scenarios in the graph g are sorted
 * on their occurrence frequency in the FSM. The ordering is from high to low.
 */
std::list<ScenarioRef> TileBindingAlgoBase::sortScenariosOnOccurrenceCount(Graph& g) {
    std::map<ScenarioRef, uint> scenarioCnt;
    std::list<ScenarioRef> scenarios;

    // No FSM specified?
    if (g.getFSM() == nullptr) {
        scenarios.push_back(g.getDefaultScenario());
        return scenarios;
    }

    // Iterate over the states in the FSM
    for (const auto& s : g.getFSM()->getStates()) {
        scenarioCnt[s->getScenario()]++;
    }

    // Sort scenarios on occurrence frequency
    for (auto& [sc, f] : scenarioCnt) {
        auto j = scenarios.end();

        // Find first scenario j in list with lower occurrence count than i
        while (j != scenarios.end() && scenarioCnt[*j] > f) {
            j++;
        }

        // Add scenario i to the list before scenario j
        scenarios.insert(j, sc);
    }

    return scenarios;
}

/**
 * createAllBindingsOfActor()
 * The function returns all possible bindings of the actors with the supplied
 * name on the platform using the (partial) bindings given in the list
 * bindings.
 */
void TileBindingAlgoBase::createAllBindingsOfActor(CString &name, std::unique_ptr<GraphBindings>& bindings) {
    auto newBindings = std::make_unique<GraphBindings>();

    // Iterate over the existing partial bindings
    for (const auto& b : *bindings) {
        ActorBindings actorBindingCandidates;

        // Find all actor bindings for the actor name. Note that the unified
        // mapping implies that all actors with the same name have the same
        // actor binding candidates.
        ActorRef a = actorNamesToActors[name].front();
        actorBindingCandidates = findActorBindingCandidates(b.get(), a);

        // Binding options available for this actor?
        if (!actorBindingCandidates.empty()) {
            std::unique_ptr<GraphBindingBase> bNew = b->clone();
            bNew->getPlatformBinding()->setName(CString(newBindings->size()));

            // Try all binding candidates
            for (auto &ab : actorBindingCandidates) {

                // Try resource allocation for this actor binding
                if (allocateActorBinding(bNew.get(), name, ab)) {
                    // Add binding to the set of solutions
                    newBindings->push_back(std::move(bNew));

                    // Create a new binding based on binding b
                    bNew = b->clone();
                    bNew->getPlatformBinding()->setName(CString(newBindings->size()));
                }
            }
        }
    }

    // New bindings become bindings
    bindings = std::move(newBindings);
}

/**
 * createBindingOfChannel()
 * The function tries to bind the channels with the supplied name to the
 * platform using the binding b. A binding can only be made when the source
 * and destination actor of the channel are already bound to a tile. The
 * function will try all possible bindings till a valid binding is found. When
 * no valid binding exists (due to resource constraints), the function returns
 * false. Otherwise the function returns true. So, the function also returns
 * true when no binding can be made because the source or destination actor
 * is not bound.
 */
bool TileBindingAlgoBase::createBindingOfChannel(const CString &name, GraphBindingBase *b) {
    ChannelBindings channelBindingCandidates;

    // Find all channel bindings for the channel name. Note that the unified
    // mapping implies that all channels with the same name have the same
    // channel binding candidates.
    ChannelRef c = channelNamesToChannels[name].front();
    channelBindingCandidates = findChannelBindingCandidates(b, c);

    // No channel binding candidates exist?
    if (channelBindingCandidates.empty()) {
        return true;
    }

    // Iterate over the channel binding candidates
    for (auto &cb : channelBindingCandidates) {

        // Does this channel binding produce a valid binding?
        if (allocateChannelBinding(b, name, cb)) {
            return true;
        }
    }

    return false;
}

/**
 * paretoMinimizationOfApplicationBindings()
 * The function removes all non-Pareto application bindings from the list of
 * application bindings. The function implements the Simple Cull algorithm.
 */
void TileBindingAlgoBase::paretoMinimizationOfApplicationBindings(
        std::unique_ptr<GraphBindings> &applicationBindings) {
    auto newBindings = std::make_unique<GraphBindings>();

    // Iterate over all application bindings
    while (!applicationBindings->empty()) {
        auto i = applicationBindings->begin();
        GraphBindingBase* c = i->get();
        bool dominated = false;

        for (auto j = newBindings->begin(); j != newBindings->end();) {
            GraphBindingBase* d = j->get();

            // Check for Pareto dominance
            if (c->paretoDominates(d)) {
                // Binding d is dominated by c, so d can be removed
                // Remove binding from the list
                newBindings->erase(j++);
            } else if (d->paretoDominates(c)) {
                // Binding c is dominated by d
                dominated = true;
                break;
            } else {
                // Bindings c and d do not dominate each other
                j++;
            }
        }

        // Binding c is Pareto optimal?
        if (!dominated) {
            // Add binding to the new list
            newBindings->push_back(c->clone());
        }

        // Remove binding c from the list
        applicationBindings->erase(i++);
    }

    // The new bindings become the application bindings
    applicationBindings = std::move(newBindings);
}

/**
 * reduceApplicationBindings()
 * The function reduces the number of application bindings in the list to the
 * specified maximum.
 */
void TileBindingAlgoBase::reduceApplicationBindings(std::unique_ptr<GraphBindings> &applicationBindings,
                                                    const uint maxNrAppBindings) {
    std::map<CString, GraphBindingBase*> mappingToGraphBinding;
    std::map<CString, double> distance;
    auto bindings = std::make_unique<GraphBindings>();

    // No reduction is needed?
    if (applicationBindings->size() <= maxNrAppBindings) {
        return;
    }

    // Initialize distances to infinity
    for (const auto& b : *applicationBindings) {
        distance[b->getPlatformBinding()->getName()] = INFINITY;
        mappingToGraphBinding[b->getPlatformBinding()->getName()] = b.get();
    }

    // Select application bindings
    while (bindings->size() < maxNrAppBindings) {
        auto i = distance.begin();

        // Select binding with largest distance to bindings already in the list
        for (auto j = i; j != distance.end(); j++) {
            if (j->second > i->second) {
                i = j;
            }
        }

        // Remove the binding i from the old list of bindings
        auto tempBinding = applicationBindings->removeGraphBinding(mappingToGraphBinding[i->first]);

        // Add the binding i to the new list of bindings
        bindings->push_back(std::move(tempBinding));

        // Update minimal distance between bindings in the list bindings and
        // bindings in the list applicationBindings
        for (const auto& b : *applicationBindings) {
            CString mappingName = b->getPlatformBinding()->getName();

            distance[mappingName] =
                    (std::min)(b->getParetoQuantities().distance(
                                mappingToGraphBinding[i->first]->getParetoQuantities()),
                        distance[mappingName]);
        }

        // Remove the binding i from the distance
        distance.erase(i);
    }

    // The new bindings become the application bindings
    applicationBindings = std::move(bindings);
}

/**
 * reduceSlotsPerProcessor()
 * The function attempts to reduce the number of TDN slots used on each processor.
 * It starts with the processor that has the most free slots.
 */
void TileBindingAlgoBase::reduceSlotsPerProcessor(GraphBindings *applicationBindings,
                                                  ProcessorRefs& processors) {
    for (const auto& gb : *applicationBindings) {
        std::map<Processor *, Time> usage;
        for (auto* p : processors) {
            usage[p] = 0;
        }

        // Create list with sum of execution times of all actors per processor
        for (auto& [s, actorBindings] : gb->getActorBindings()) {

            // Iterate over all actor bindings
            for (auto& [a, binding] : actorBindings) {
                ActorBinding &ab = binding;
                Processor *p = ab.processor;

                (void)gb->getAllocatedTDMAslice(*s, *p);
                usage[p] += a->getExecutionTimeOfScenario(*s, p->getType());
            }
        }

        for (const auto& pr : processors) {
            Processor *reduce = usage.begin()->first;

            // Get processor with lowest cpu usage from list
            for (auto& [p, cu] : usage) {
                Time t = cu;

                if (t < usage[reduce]) {
                    reduce = p;
                }
            }
            usage.erase(reduce);

            Scenario* first = applicationGraph->getScenarios().begin()->get();
            Time slotSize = reduce->getContextSwitchOverhead() + reduce->getSlotLength();
            Time allocated = gb->getAllocatedTDMAslice(*first, *reduce);

            while (isThroughputConstraintSatisfied(*(gb->getPlatformBinding()))
                   && (allocated > slotSize)) {
                // Remove a slice in each scenario
                for (const auto& s : applicationGraph->getScenarios()) {
                    gb->allocateTDMAslice(*s, *reduce, allocated - slotSize);
                }
                allocated = allocated - slotSize;
            }

            // Now add one slice again if the throughput is not met
            if (!isThroughputConstraintSatisfied(*(gb->getPlatformBinding()))) {
                for (const auto& s : applicationGraph->getScenarios()) {
                    allocated = gb->getAllocatedTDMAslice(*s, *reduce);
                    gb->allocateTDMAslice(*s, *reduce, allocated + slotSize);
                }
                // Double-check that the throughput constraint is satisfied
                if (!isThroughputConstraintSatisfied(*(gb->getPlatformBinding()))) {
                    throw CException("Throughput constraint not met!");
                }
            }
        }
    }
}

/**
 * replacePlatformBindings()
 * The function replaces the existing platform bindings with the platform
 * bindings given by the graph bindings. The initial binding (if present) is
 * preserved.
 */
void TileBindingAlgoBase::replacePlatformBindings(const GraphBindings& newBindings) {
    // Remove the existing platform bindings (except the initial binding)
    for (auto i = platformBindings->begin(); i != platformBindings->end();) {
        PlatformBinding* pb = i->get();

        if (pb->isInitialBinding()) {
            i++;
        } else {
            platformBindings->erase(i++);
        }
    }

    // Update the list of platform bindings
    for (const auto& gb : newBindings) {
        PlatformBinding* pb = gb->getPlatformBinding();

        // Add a clone of the platform binding to the list of platform bindings
        platformBindings->push_back(pb->clone(*pb));
    }
}
/**
 * This function is used by #TileBindingAlgoBase to create GraphBinding object.
 * So if you want another GraphBinding object, override this function.
 *
 * @param pb a #PlatformBinging
 * @return a new #GraphBindingBase algorithm
 */
std::unique_ptr<GraphBindingBase> TileBindingAlgoBase::createGraphBinding(const PlatformBinding& pb) {
    return std::make_unique<GraphBindingBase>(&pb);
}

/**
 * Build one map for each actor in each scenario that indicates which other actors
 * in that scenario should be bound (included) to the same processor, and which others should
 * explicitly *not* be bound (excluded) to that processor.
 */
void TileBindingAlgoBase::initClusterConstraints() {
    for (const auto& s : getApplicationGraph()->getScenarios()) {
        for (const auto& ac : s->getScenarioGraph()->getActors()) {
            CString a = ac->getName();

            // Double iteration over actors
            for (const auto& acb : s->getScenarioGraph()->getActors()) {
                CString b = acb->getName();

                // Get constraint
                clusterConstraints[s.get()][a][b] = getClusterConstraintForActor(s.get(), a, b);
            }
        }
    }
}

/**
 * Get the specific constraint between two actors in one scenario
 */
TileBindingAlgoBase::CCValue
TileBindingAlgoBase::getClusterConstraintForActor(Scenario *s, const CString &a, const CString &b) const {
    ScenarioBindingConstraints *sbc =
            getInitialPlatformBinding()->getGraphBindingConstraints()->getConstraintsOfScenario(*s);

    for (const auto& cbc : *(sbc->getAllClusterBindingConstraints())) {
        if (cbc->hasActor(a)) {
            for (const auto& j : *(cbc->getActorNames())) {
                if (j == b) {
                    return CCValue::Inc;
                }
            }

            std::list<std::unique_ptr<ClusterExclusionConstraints>> *clusterExclusionConstraints =
                    sbc->getAllClusterExclusionConstraints();
            for (const auto& cec : *clusterExclusionConstraints) {
                if (cec->hasClusterExclusion(cbc->getClusterBindingConstraintName())) {
                    for (const auto &k : *(cec->getClusterExclusionNames())) {
                        if (k != cbc->getClusterBindingConstraintName()) {
                            ClusterBindingConstraints *excludeCbc =
                                    sbc->getClusterBindingConstraints(k);
                            for (const auto &l : *(excludeCbc->getActorNames())) {
                                if (l == b) {
                                    return CCValue::Exc;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return CCValue::None;
}
} // End namespace FSMSADF
