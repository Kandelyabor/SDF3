/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf-bambha.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 28, 2011
 *
 *  Function        :   Model static-order schedule in graph using technique
 *                      from Bambha.
 *
 *  History         :
 *      28-07-11    :   Initial version.
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

#include "hsdf-bambha.h"
#include <memory>

namespace FSMSADF {

namespace {

/**
 * The function converts a scenario graph, which is in fact a synchronous
 * dataflow graph, to its homogeneous variant.
 * @param pointer to a scenario graph sg
 * @param pointer to a scenario s
 * @param mapping of actors in the original graph to actors in the
 *        homogeneous graph
 * @return pointer to a newly created scenario graph
 */
std::unique_ptr<ScenarioGraph> convertScenarioGraphToHomogeneousGraph(
        ScenarioGraph *sg, const Scenario &s, std::map<ActorRef, std::list<ActorRef>> &sgActorMap) {
    std::unique_ptr<ScenarioGraph> h = nullptr;

    // Construct a new scenario graph
    h = sg->createCopy(GraphComponent(nullptr, sg->getId(), sg->getName()));

    // Calculate repetition vector for the graph
    RepetitionVector repetitionVector = s.computeRepetitionVector();

    // Add actors to graph (number of actors depends on repetition vector)
    for (const auto &sgA : sg->getActors()) {
        for (int i = 0; i < repetitionVector[sgA->getId()]; i++) {
            // Create new actor
            std::unique_ptr<Actor> a = sgA->createCopy(GraphComponent(
                    h.get(), sgA->getId(), sgA->getName() + CString("_") + CString(i)));

            // Add actor to graph
            auto *aPtr = a.get();
            h->addActor(std::move(a));

            // Add actor relation to mapping
            sgActorMap[sgA.get()].push_back(aPtr);
        }
    }

    // Channels and ports
    for (const auto &gC : sg->getChannels()) {
        // Source and destination port and actor in scenario graph
        Port *gSrcP = gC->getSrcPort();
        Port *gDstP = gC->getDstPort();
        Actor &gSrcA = gSrcP->getActor();
        Actor &gDstA = gDstP->getActor();

        uint nA = gSrcP->getRateOfScenario(s);
        uint nB = gDstP->getRateOfScenario(s);
        uint qA = repetitionVector[gSrcA.getId()];
        uint qB = repetitionVector[gDstA.getId()];
        uint d = gC->getInitialTokens();
        for (uint i = 1; i <= qA; i++) {
            // Get pointer to source actor
            ActorRef hSrcA = h->getActor(gSrcA.getName() + CString("_") + CString(i - 1));

            for (uint k = 1; k <= nA; k++) {
                uint l = 1 + ((d + (i - 1) * nA + k - 1) % (nB * qB));
                uint j = 1
                         + static_cast<uint>(
                                 floor(static_cast<double>((d + (i - 1) * nA + k - 1) % (nB * qB))
                                       / static_cast<double>(nB)));

                // Create port on source node
                std::unique_ptr<Port> hSrcP = gSrcP->createCopy(
                        GraphComponent(hSrcA, static_cast<CId>(hSrcA->getPorts().size())));
                hSrcP->setName(gSrcP->getName() + CString("_") + CString(k - 1));
                hSrcP->setRateOfScenario(s, 1);
                auto *hSrcPPtr = hSrcP.get();
                hSrcA->addPort(std::move(hSrcP));

                // Create port on destination node
                ActorRef hDstA = h->getActor(gDstA.getName() + CString("_") + CString(j - 1));
                std::unique_ptr<Port> hDstP = gDstP->createCopy(
                        GraphComponent(hDstA, static_cast<CId>(hDstA->getPorts().size())));
                hDstP->setName(gDstP->getName() + CString("_") + CString(l - 1));
                hDstP->setRateOfScenario(s, 1);
                auto *hDstPPtr = hDstP.get();
                hDstA->addPort(std::move(hDstP));

                // Create channel between actors
                std::unique_ptr<Channel> c = gC->createCopy(
                        GraphComponent(h.get(), static_cast<CId>(h->getChannels().size())));
                c->setName(gC->getName() + CString("_") + CString(((i - 1) * nA) + (k - 1)));
                c->connectSrc(hSrcPPtr);
                c->connectDst(hDstPPtr);

                // Initial tokens
                uint t = static_cast<uint>(floor(static_cast<double>(d + ((i - 1) * nA) + k - 1)
                                                 / static_cast<double>(nB * qB)));
                c->setInitialAndFinalTokens(t);
                // @TODO [SANDER] Token should be marked as persistent - preserve original relation

                // Add channel to graph
                h->addChannel(std::move(c));
            }
        }
    }

    return h;
}

/**
 * Model a static order schedule in the scenario graph using the technique
 * from Bambha. Note that this technique can only be applied to homogeneous
 * graphs.
 * @param pointer to a scenario graph sg
 * @param pointer to scenario s
 * @param reference to a static order schedule so that schedules one
 *        iteration of the graph
 * @throw an exception is thrown when the graph is not a homogeneous graph
 * @throw an exception is thrown when the schedule is longer than one
 *        iteration
 */
void modelStaticOrderScheduleInScenarioGraphUsingBambha(ScenarioGraph *sg,
                                                        const Scenario &s,
                                                        StaticOrderSchedule &so) {
    ActorRef a = nullptr;
    ActorRef aN = nullptr;

    // Technique only works for periodic static-order schedules
    if (!so.isPeriodic()) {
        throw CException("Static-order schedule is not periodic.");
    }

    // Graph should be homogeneous
    RepetitionVector repetitionVector = s.computeRepetitionVector();
    for (int i : repetitionVector) {
        if (i != 1) {
            throw CException("Scenario graph is not homogeneous.");
        }
    }

    // Occurrence count of actors in the original graph
    RepetitionVector occurrenceCount(so.getActorCount().size(), 0);

    // Iterate over the schedule
    for (uint i = so.getStartPeriodicSchedule(); !so.isLast(i); i = so.next(i)) {
        /* Actor pointer in schedule may point to a different scenario
         * graph. Locate the corresponding actor in the supplied scenario
         * graph.
         */
        a = sg->getActor(so[i].actor->getName() + CString("_")
                         + CString(occurrenceCount[so[i].actor->getId()]));

        // Locate next actor in the schedule
        aN = sg->getActor(so[so.next(i)].actor->getName() + CString("_")
                          + CString(occurrenceCount[so[so.next(i)].actor->getId()]));

        // Create a channel from a to aN (no initial token needed)
        sg->createChannel(a, aN);

        // Increase occurrence count of the actor a
        occurrenceCount[so[i].actor->getId()]++;
    }

    // Create back-edge with initial token from end to start of schedule
    a = sg->getActor(so[so.last()].actor->getName() + CString("_")
                     + CString(occurrenceCount[so[so.last()].actor->getId()]));

    // Locate next actor in the schedule
    aN = sg->getActor(so[so.getStartPeriodicSchedule()].actor->getName() + CString("_")
                      + CString(occurrenceCount[so[so.getStartPeriodicSchedule()].actor->getId()]));

    // Create a channel from a to aN (one initial token needed)
    ChannelRef c = sg->createChannel(a, aN);
    c->setInitialAndFinalTokens(1);
    // @TODO [SANDER] Token should be marked as persistent

    // Increase occurrence count of the actor a
    occurrenceCount[so[so.last()].actor->getId()]++;
}

/**
 * The function extends the static-order schedule to include a complete
 * iteration of all actors contained in the schedule. The required
 * occurrence count is determined by the sgActorMap. Each actor in the
 * static-order schedule should appear as often as the length of the
 * corresponding list inside the sgActorMap.
 * @param mapping of old actors (used in static-order schedule) to new
 *        actor references
 * @param static-order schedule
 */
void extendStaticOrderScheduleToIteration(std::map<ActorRef, std::list<ActorRef>> &sgActorMap,
                                          StaticOrderSchedule &so) {
    uint unfoldingFactor = 1;
    std::map<ActorRef, uint> actorOccurrenceCount;

    // Create list with all actors that appear in the static-order schedule
    // and their occurrence count
    for (auto soe : so) {
        ActorRef a = soe.actor;
        if (actorOccurrenceCount.find(a) == actorOccurrenceCount.end()) {
            actorOccurrenceCount[a] = 1;
        } else {
            actorOccurrenceCount[a] += 1;
        }
    }

    // Compute schedule extension based on length of the lists in the
    // sgActorMap
    for (auto [a, cnt] : actorOccurrenceCount) {
        // Extension of the schedule needed?
        if (cnt * unfoldingFactor != static_cast<int>(sgActorMap[a].size())) {
            // Extension only allowed if schedule has not been extended
            // before
            if (unfoldingFactor != 1) {
                throw CException("Different actors require different "
                                 "static-order schedule unfoldings.");
            }

            // Extension is only possible if required extensions includes
            // a complete number of schedule iterations
            if (sgActorMap[a].size() % cnt != 0) {
                throw CException("Schedule must be extended with full "
                                 "number of iterations");
            }

            unfoldingFactor = (static_cast<uint>(sgActorMap[a].size())) / cnt;
        }
    }

    // No schedule extension needed?
    if (unfoldingFactor == 1) {
        return;
    }

    // Create a copy of the original schedule
    StaticOrderSchedule soOld = so;

    // Clear the old static order schedule (but not the copy)
    so.clear();

    // Extend the schedule
    for (uint i = 0; i < unfoldingFactor; i++) {
        so.insert(so.end(), soOld.begin(), soOld.end());
    }
}


/**
 * The function updates all static-order schedules of scenario s using the
 * relation provided in the sgActorMap parameter. It provides for each actor
 * a list of new actors. The length of the list corresponds to the number
 * of actor firings of the original (old) actor that need to occur in a
 * single iteration of the graph. If needed, the function extends
 * static-orders schedules such that a complete iteration is scheduled.
 * @param platform binding with static-order schedules
 * @param scenario s
 * @param mapping of old actors (used in schedule) to new actors
 * @throw an exception is thrown when a static-order schedule is found that
 *        is longer than one iteration
 */
void convertStaticOrderSchedulesToSingleIterationHomogeneous(
        PlatformBinding *pb,
        const Scenario &s,
        std::map<ActorRef, std::list<ActorRef>> &sgActorMap) {
    // Iterate over all tiles
    for (TileBindings::const_iterator i = pb->getTileBindings().begin();
         i != pb->getTileBindings().end();
         i++) {
        TileBinding *tb = (*i).get();

        // Iterate over all processors
        for (ProcessorBindings::const_iterator j = tb->getProcessorBindings().begin();
             j != tb->getProcessorBindings().end();
             j++) {
            const std::unique_ptr<ProcessorBinding> &prB = *j;
            StaticOrderSchedule &so = prB->getStaticOrderSchedule(s);

            // Technique only works for periodic static-order schedules
            if (!so.isPeriodic()) {
                throw CException("Static-order schedule is not periodic.");
            }

            // Extend the static-order schedule to a complete iteration
            extendStaticOrderScheduleToIteration(sgActorMap, so);

            // Iterate over the static-order schedule
            for (uint k = 0; !so.isLast(k); k++) {
                ASSERT(!sgActorMap[so[k].actor].empty(), "Schedule not long enough.");

                // Remove pointer to new actor from the list
                ActorRef aNew = sgActorMap[so[k].actor].front();
                sgActorMap[so[k].actor].pop_front();

                // Update the static-order schedule
                so[k].actor = aNew;
            }
        }
    }
}

} // namespace

/**
 * Models the static-order schedules of all processors in the graph g using
 * the technique from Bambha.
 * This function will create a separate scenario graph for each scenario.
 * These scenario graphs are converted to their corresponding HSDFGs and
 * additional edges are introduced to model the static-order schedules.
 */
void modelStaticOrderScheduleInGraphUsingBambha(
        PlatformBinding *pb,
        Graph *g,
        std::map<ActorRef, ActorRef> & /*mapActorToActorInputGraph*/) {
    // Clone platform binding
    std::unique_ptr<PlatformBinding> pbNew = pb->clone(GraphComponent(nullptr, 0));

    // Isolate scenarios
    g->isolateScenarios();

    // Construct homogeneous graph for each scenario
    for (const auto &s : g->getScenarios()) {
        std::map<ActorRef, std::list<ActorRef>> sgActorMap;

        // Scenario and its scenario graph
        ScenarioGraph *sg = s->getScenarioGraph();

        // Create homogeneous graph
        std::unique_ptr<ScenarioGraph> sgHomo =
                convertScenarioGraphToHomogeneousGraph(sg, *s, sgActorMap);

        // Replace existing scenario graph
        g->replaceScenarioGraph(sg, std::move(sgHomo));

        // Update actor pointers in static order schedules
        convertStaticOrderSchedulesToSingleIterationHomogeneous(pbNew.get(), *s, sgActorMap);
    }

    // Add static-order scheduling constraints to the graph
    for (TileBindings::const_iterator i = pb->getTileBindings().begin();
         i != pb->getTileBindings().end();
         i++) {
        TileBinding *tb = (*i).get();

        for (ProcessorBindings::const_iterator j = tb->getProcessorBindings().begin();
             j != tb->getProcessorBindings().end();
             j++) {
            const std::unique_ptr<ProcessorBinding> &prB = *j;

            for (auto &[s, so] : prB->getStaticOrderSchedules()) {
                // Find corresponding scenario graph
                ScenarioGraph *sg = g->getScenarioGraph(s->getName());

                // Model static order schedule in the scenario graph
                modelStaticOrderScheduleInScenarioGraphUsingBambha(sg, *s, so);
            }
        }
    }
}

/**
 * Create a graph in which the static-order schedules of all processors are
 * modelled explicitly using the technique from Bambha.
 * This function will create a separate scenario graph for each scenario.
 * These scenario graphs are converted to their corresponding HSDFGs and
 * additional edges are introduced to model the static-order schedules.
 */
std::unique_ptr<Graph> modelStaticOrderScheduleInGraphUsingBambha(PlatformBinding *pb) {
    std::map<ActorRef, ActorRef> mapActorToActorInputGraph;

    // Clone application graph
    Graph *g = pb->getApplicationGraph();
    std::unique_ptr<Graph> gNew = g->clone(GraphComponent(nullptr, g->getId(), g->getName()));

    // Transform the graph
    modelStaticOrderScheduleInGraphUsingBambha(pb, gNew.get(), mapActorToActorInputGraph);

    return gNew;
}

} // End namespace FSMSADF
