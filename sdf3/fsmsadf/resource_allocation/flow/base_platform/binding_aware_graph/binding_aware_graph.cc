/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *      04-08-11    :   Complete rewrite to separate analysis from mapping
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

#include "binding_aware_graph.h"
#include "fsmsadf/resource_allocation/platform_binding/graph.h"
#include <memory>

namespace FSMSADF {

/**
 * actorMappedToProcessor()
 * The function returns true when the supplied actor is mapped to a
 * processor. Otherwise the function returns false.
 */
bool BindingAwareGraphBase::BindingAwareProperties::actorMappedToProcessor(Actor *a) const {
    return actorToProcessor.find(a) != actorToProcessor.end();
}

/**
 * channelMappedToConnection()
 * The function returns true when the supplied channel is mapped to a
 * connection. Otherwise the function returns false.
 */
bool BindingAwareGraphBase::BindingAwareProperties::channelMappedToConnection(Channel *c) const {
    return channelToConnection.find(c) != channelToConnection.end();
}

/**
 * Construct a binding aware graph based on the supplied platform binding.
 * @param platform binding b
 */
BindingAwareGraphBase::BindingAwareGraphBase(const PlatformBinding *b) :
    Graph(b->getApplicationGraph()), platformBinding(nullptr) {
    Graph *gApp = b->getApplicationGraph();

    // Binding-aware graph is initially equal to the application graph.
    // Hence, actors in the binding-aware graph have actor with the same
    // name in the application graph.
    for (const auto& sgApp : gApp->getScenarioGraphs()) {
        ScenarioGraph *sg = this->getScenarioGraph(sgApp->getName());

        for (const auto& aApp : sgApp->getActors()) {
            ActorRef a = sg->getActor(aApp->getName());
            mapBindingAwareActorToApplicationGraphActor[a] = aApp.get();
        }
    }

    // Platform binding based on which the binding-aware graph is
    // constructed
    setPlatformBinding(b);
}

/**
 * Destructor.
 */
BindingAwareGraphBase::~BindingAwareGraphBase() = default;

/**
 * getScenarioOfScenarioGraph()
 * The function returns the scenario which uses the supplied scenario graph.
 * It is guaranteed that there is only one scenario which uses a scenario
 * graph, since the binding-aware graph uses a graph with isolated scenario.
 */
ScenarioRef BindingAwareGraphBase::getScenarioOfScenarioGraph(const ScenarioGraph& sg) {
    Graph *g = sg.getGraph();

    // Iterate over all scenarios
    for (const auto& s : g->getScenarios()) {
        if (s->getScenarioGraph() == &sg) {
            return s.get();
        }
    }

    throw CException("Scenario graph '" + sg.getName() + "' is not used in any scenario.");
}

/**
 * extractActorMapping ()
 * The function extracts the actor binding and scheduling from the platform
 * graph.
 */
void BindingAwareGraphBase::extractActorMapping() {
    const PlatformBinding *b = getPlatformBinding();

    // Iterate over the tile bindings
    for (const auto& tb : b->getConstTileBindings()) {
        // Iterate over the processor bindings
        for (const auto& pb : tb->getProcessorBindings()) {
            // Iterate over the actor bindings
            for (auto& [s, actors] : pb->getActorBindings()) {
                // Find scenario in the binding-aware graph which
                // corresponds to scenario s.
                ScenarioRef sn = this->getScenarioRef(s->getName());

                // Scenario graph in the binding-aware graph
                ScenarioGraph *sg = sn->getScenarioGraph();

                // Iterate over all actors
                for (auto *a : actors) {
                    // Find corresponding actor in sg
                    ActorRef an = sg->getActor(a->getName());

                    // Store binding of actor an to processor p in the scenario
                    // graph sg
                    bindingAwareProperties[sg].mapActorToProcessor(an, pb->getProcessor());
                }
            }

            // Iterate over the TDMA resource allocations
            for (auto& [s, t] : pb->getWheelsizeAllocations()) {
                // Find scenario in the binding-aware graph which
                // corresponds to scenario s.
                ScenarioRef sn = this->getScenarioRef(s->getName());

                // Scenario graph in the binding-aware graph
                ScenarioGraph *sg = sn->getScenarioGraph();

                // Store binding of time slice t to processor p
                // in the scenario graph sg
                bindingAwareProperties[sg].mapTimesliceToProcessor(pb->getProcessor(), t);
            }
        }
    }
}

/**
 * extractChannelMapping ()
 * The function extracts the channel binding and scheduling from the platform
 * graph.
 */
void BindingAwareGraphBase::extractChannelMapping() {
    const PlatformBinding *b = getPlatformBinding();

    // Iterate over the connection bindings
    for (const auto& cb : b->getConstConnectionBindings()) {
        // Iterate over all channel bindings
        for (auto& [s, channels] : cb->getChannelBindings()) {
            // Find scenario in the binding-aware graph which
            // corresponds to scenario s.
            ScenarioRef sn = this->getScenarioRef(s->getName());

            // Scenario graph in the binding-aware graph
            ScenarioGraph *sg = sn->getScenarioGraph();

            // Iterate over all channels
            for (auto *c : channels) {
                // Find corresponding channel in sg
                ChannelRef cn = sg->getChannel(c->getName());

                // Store binding of channel cn to connection c in the scenario
                // graph sg
                bindingAwareProperties[sg].mapChannelToConnection(cn, cb->getConnection());
            }
        }
    }
}

/**
 * Construct a binding aware graph based on the supplied platform binding.
 */
void BindingAwareGraphBase::constructBindingAwareGraph(bool encodeSchedule) {
    // Are the scenarios in the graph isolated?
    if (!this->hasIsolatedScenarios()) {
        this->isolateScenarios();
    }

    // Extract actor related binding and scheduling properties
    extractActorMapping();

    // Extract channel related binding and scheduling properties
    extractChannelMapping();

    // Optional custom initialization of the worst-case actors
    initWorstCaseActors();

    // Construct the graph
    modelBindingAndScheduling(encodeSchedule);

    // Add all persistent tokens from other scenarios as dummy to each scenario
    addDummyPersistentTokens();
}

/**
 * The function returns a pointer to the actor in the application graph that
 * corresponds to the actor #a.
 * @param pointer to actor #a
 * @return pointer to corresponding actor in application graph
 */
Actor *BindingAwareGraphBase::getCorrespondingActorInApplicationGraph(Actor *a) const {
    if (mapBindingAwareActorToApplicationGraphActor.find(a)
        != mapBindingAwareActorToApplicationGraphActor.end()) {
        return mapBindingAwareActorToApplicationGraphActor.at(a);
    }

    return nullptr;
}

/**
 * Add one dummy actor plus self-edge to each scenario,
 * and add all persistent tokens NOT in that scenario to that edge.
 * Goal is to avoid "Persistent initial tokens do not match.." errors.
 */
void BindingAwareGraphBase::addDummyPersistentTokens() {
    // Iterate over all scenario graphs in the binding aware graph
    for (auto i = getScenarioGraphs().begin(); i != getScenarioGraphs().end(); i++) {
        CStrings persistentTokenNames;
        unsigned int initialTokenCount = 0;
        ScenarioGraph* sg = i->get();
        // Get scenario which corresponds to this graph (this relation is
        // unique since a graph with isolated scenario is used).
        ScenarioRef s = getScenarioOfScenarioGraph(*sg);

        PersistentTokenMap mpSg = sg->getPersistentTokenMap();

        // Iterate over all scenario graphs in the binding aware graph
        for (auto j = getScenarioGraphs().begin(); j != getScenarioGraphs().end(); j++) {
            // If not this scenario:
            if (std::distance(getScenarioGraphs().begin(), j)
                != std::distance(getScenarioGraphs().begin(), i)) {
                ScenarioGraph* ssg = j->get();
                PersistentTokenMap mpSsg = ssg->getPersistentTokenMap();

                // Create string with persistent token names of other scenario
                CStrings otherPT;
                for (auto& [tokenName, t] : mpSsg) {
                    otherPT.push_back(tokenName);
                }
                // Iterate over names of persistent tokens of other scenario
                for (const auto &k : otherPT) {
                    // Add every persistentToken existing in other scenarios but NOT yet in this one
                    // to the dummy channel
                    if ((mpSg.find(k) == mpSg.end())
                        && (std::find(persistentTokenNames.begin(), persistentTokenNames.end(), k)
                            == persistentTokenNames.end())) {
                        persistentTokenNames.push_back(k);
                        initialTokenCount++;
                    }
                }
            }
        }

        if (!persistentTokenNames.empty()) {
            // Add dummy actor and channel to graph
            ActorRef dummyActor = sg->createActor();
            dummyActor->setName(sg->getName() + "_dummy");
            Time dummyTime = 0;
            dummyActor->setExecutionTimeOfScenario(*s, "dummy", dummyTime);
            dummyActor->setDefaultProcessorType("dummy");

            // Add self-edge on dummy with inital tokens and rate
            ChannelRef dummySelf = sg->createChannel(dummyActor, dummyActor);
            dummySelf->setName(sg->getName() + "-dummySelf");
            dummySelf->setInitialAndFinalTokens(initialTokenCount);
            CString persistentTokenString = "";
            for (const auto &k : persistentTokenNames) {
                if (!persistentTokenString.empty()) {
                    persistentTokenString += ",";
                }
                persistentTokenString = persistentTokenString + k;
            }
            dummySelf->setPersistentAndFinalTokenNames(persistentTokenString);
            dummySelf->getDstPort()->setRateOfScenario(*s, initialTokenCount);
            dummySelf->getSrcPort()->setRateOfScenario(*s, initialTokenCount);

            // Connect dummyActor to any actor in original graph
            ActorRef fa = (sg->getActors().begin())->get();
            ChannelRef dummyConnect = sg->createChannel(dummyActor, fa);
            dummyConnect->setName(sg->getName() + "-dummyConnect");

            // Set repetition vector of new actor
            RepetitionVector repetitionVector = s->computeRepetitionVector();
            dummyActor->setRepetitionsOfScenario(*s, repetitionVector[dummyActor->getId()]);
        }
    }
}

/**
 * Overload this function for custom initialization of the worst-case actors.
 * By default, every actor takes the worst-case TDM scheduling hit.
 */
void BindingAwareGraphBase::initWorstCaseActors() {}

/**
 * Model all binding decisions in a graph.
 */
void BindingAwareGraphBase::modelBindingAndScheduling(bool encodeSchedule) {
    // Iterate over all scenario graphs in the binding aware graph
    for (const auto& sg : getScenarioGraphs()) {
        // Get scenario which corresponds to this graph (this relation is
        // unique since a graph with isolated scenario is used).
        Scenario *s = getScenarioOfScenarioGraph(*sg);

        // Keep lists with original channels in the scenario graph. Only
        // these channels need to be modeled. Channels added
        // by this function should be ignored.
        // Therefore channels are copied here
        ChannelRefs channels;
        for (const auto& ch : sg->getChannels()) {
            channels.push_back(ch.get());
        }

        // Iterate over all actors in the graph
        for (const auto& a : sg->getActors()) {
            Processor *processor = nullptr;

            // No binding of actor to processor exists?
            if (!bindingAwareProperties[sg.get()].actorMappedToProcessor(a.get())) {
                throw CException("All actors must be bound to a processor, '" + a->getName()
                                 + "' in '" + s->getName() + "' is not.");
            }

            // Get processor to which actor is mapped
            processor = bindingAwareProperties[sg.get()].getProcessorMappedToActor(a.get());

            // Model binding of actor a to processor p
            modelActorToTileBinding(a.get(), processor, s);
        }

        // Iterate over all channels in the graph
        for (const auto& c : channels) {
            // @TODO [SANDER] Channel c and scenario s in platform binding are different pointers
            // (but same element) in the binding-aware graph.
            // All lookups should be done based on name, not on pointer!

            // Get binding constraints of channel c
            ScenarioRef sApp = getPlatformBinding()->getApplicationGraph()->getScenarioRef(s->getName());
            ChannelRef cApp = getPlatformBinding()
                                    ->getApplicationGraph()
                                    ->getScenarioRef(s->getName())
                                    ->getScenarioGraph()
                                    ->getChannel(c->getName());
            GraphBindingConstraints *gbc = getPlatformBinding()->getGraphBindingConstraints();
            ChannelBindingConstraint *cbc =
                    gbc->getConstraintsOfScenario(*sApp)->getConstraintsOfChannel(cApp->getName());

            // No binding of channel to connection exists?
            if (!bindingAwareProperties[sg.get()].channelMappedToConnection(c)) {
                // Model binding of channel c to a tile
                modelChannelToTileBinding(c, cbc, s);
            } else {
                Connection *con = bindingAwareProperties[sg.get()].getConnectionMappedToChannel(c);
                // Model binding of channel c to connection co
                modelChannelToConnectionBinding(c, cbc, con, s);
            }
        }
        interModelOrdering(s, sg.get());
    }

    if (encodeSchedule) {
        for (const auto& s : getScenarios()) {
            // Model static-order schedules
            modelStaticOrderScheduleOnProcessor(s.get());
        }
    }
}

/**
 * Model the binding of an actor to a tile inside the binding-aware graph.
 * @param scenario s
 * @param actor a
 * @param processor p
 */
void BindingAwareGraphBase::modelActorToTileBinding(Actor *a, Processor *p, Scenario *s) {

    // Scenario graph
    ScenarioGraph *sg = a->getScenarioGraph();

    // Compute WCRT when using TDMA
    Time execTime = a->getExecutionTimeOfScenario(*s, p->getType());
    Time contextSwitchTime = p->getContextSwitchOverhead();
    Time tdmaTimeSlice = bindingAwareProperties[sg].getTimesliceMappedToProcessors(p);
    Time waitingTime = p->getWheelsize() - tdmaTimeSlice;
    Time wcrt = 0;

    while (execTime > 0) {
        wcrt = wcrt + waitingTime;

        if (execTime < tdmaTimeSlice - contextSwitchTime) {
            wcrt = wcrt + execTime + contextSwitchTime;
            execTime = 0;
        } else {
            wcrt = wcrt + tdmaTimeSlice;
            execTime = execTime - tdmaTimeSlice + contextSwitchTime;
        }
    }

    // Set the execution time of actor a on processor p as its worst-case
    // response time
    a->setExecutionTimeOfScenario(*s, "wcrt", wcrt);
    a->setDefaultProcessorType("wcrt");

    // Add self-loop with one token
    ChannelRef c = a->getScenarioGraph()->createChannel(a, a);
    c->setName(a->getName() + "-selfedge");
    c->getSrcPort()->setRateOfScenario(*s, 1);
    c->getDstPort()->setRateOfScenario(*s, 1);
    c->setInitialAndFinalTokens(1);
    c->setPersistentAndFinalTokenNames(CString("PT-") + c->getName());
}

/**
 * The function models a channel mapped to a connection.
 *
 * The transformation performed by this function on a channel c with n
 * initial tokens and a buffer size distribution b looks as follows
 *
 *          p    c           n               q
 *        s -----------------*---------------> d
 *
 *                           |
 *                           V
 *
 *               c           n
 *        +------------------*--------------------------+
 *        |                                             |
 *      p | p   b[src]-n       b[dst]                 q v q
 *  +---- s <-----*------+ +------*-------------------- d <------+
 *  |   p | p  cs        | |1         cd              q ^ q      |
 *  |     |              | |                            |        |
 *  |     |           1 1| v 1      1      1 1    1     |        |
 *  |     +--------*---> acor ------> acol --> at ------+        |
 *  |         csr  n   1 | ^ 1  crl        clt      ctd          |
 *  |                    | |                                     |
 *  |                  1 * |                                     |
 *  |                    +-+                                     |
 *  |                    ca                                      |
 *  |      n    csd                1    1     cdd                |
 *  +------*-----------------------> ad -------------------------+
 *
 * Actor ad enforces the minimal delay of channel c to be considered in TDMA
 * allocation. This minimal delay creates scheduling freedom for the
 * interconnect scheduling problem.
 *
 * Actors acor and acol models the latency and rrate of sending a token
 * through the channel c. This is a latency-rate model of the connection.
 * The assumption is that buffer space is released in the source channel as
 * soon as the data is sent. At this moment, space is claimed at the
 * destination side (i.e. a transmission on the connection may only start
 * when space is available at the destination).
 *
 * Actor at models the worst-case TDMA timewheel synchronization which may
 * occur when tokens are sent from the source processor to the destination
 * processor.
 *
 * Channels cs and cd model the source and destination buffer of the
 * channel.
 */
void BindingAwareGraphBase::modelChannelToConnectionBinding(Channel *c,
                                                            ChannelBindingConstraint *bc,
                                                            Connection *co,
                                                            Scenario *s) {
    CString persistentTokenNames;

    // Scenario graph
    ScenarioGraph *sg = c->getScenarioGraph();

    // Buffer size of channel c
    BufferSize b = bc->getBufferSize();

    // Processor to which destination actor of channel c is bound
    Processor *pd = bindingAwareProperties[sg].getProcessorMappedToActor(c->getDstActor());

    // Execution times of the actors acol, acor, ad, and al
    Time latency = co->getLatency();
    Time rate = static_cast<Time>(ceil(c->getTokenSizeOfScenario(*s) / bc->getBandwidth()));
    Time tdma = pd->getWheelsize() - bindingAwareProperties[sg].getTimesliceMappedToProcessors(pd);
    Time delay = bc->getDelay();

    // Create an actor acol which models the connection latency
    ActorRef acol = sg->createActor();
    acol->setName(c->getName() + "_latency");
    acol->setExecutionTimeOfScenario(*s, "latency", latency);
    acol->setDefaultProcessorType("latency");

    // Create an actor acor which models the connection rate
    ActorRef acor = sg->createActor();
    acor->setName(c->getName() + "_rate");
    acor->setExecutionTimeOfScenario(*s, "rate", rate);
    acor->setDefaultProcessorType("rate");

    // Create actor ad which models the time wheel synchronization
    ActorRef at = sg->createActor();
    at->setName(c->getName() + "_tdma");
    at->setExecutionTimeOfScenario(*s, "tdma", tdma);
    at->setDefaultProcessorType("tdma");

    // Create actor ad which models the minimal delay of channel c
    ActorRef ad = sg->createActor();
    ad->setName(c->getName() + "_delay");
    ad->setExecutionTimeOfScenario(*s, "delay", delay);
    ad->setDefaultProcessorType("delay");

    // Create channel to model source buffer
    ChannelRef cs = sg->createChannel(acor, c->getSrcActor());
    cs->setName(c->getName() + "-src-buffer");
    cs->getSrcPort()->setRateOfScenario(*s, 1);
    cs->getDstPort()->setRateOfScenario(*s, c->getSrcPort()->getRateOfScenario(*s));
    if (b[static_cast<int>(BufferLocation::Value::Src)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space for initial tokens.");
    }
    cs->setInitialAndFinalTokens(b[static_cast<int>(BufferLocation::Value::Src)]
                                 - c->getInitialTokens());
    persistentTokenNames = "";
    for (uint i = 0; i < cs->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + cs->getName();
    }
    cs->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Create channel to model destination buffer
    ChannelRef cd = sg->createChannel(c->getDstActor(), acor);
    cd->setName(c->getName() + "-dst-buffer");
    cd->getSrcPort()->setRateOfScenario(*s, c->getDstPort()->getRateOfScenario(*s));
    cd->getDstPort()->setRateOfScenario(*s, 1);
    cd->setInitialAndFinalTokens(b[static_cast<int>(BufferLocation::Value::Dst)]);
    persistentTokenNames = "";
    for (uint i = 0; i < cd->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + cd->getName();
    }
    cd->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Create channel from source actor to actor which models connection rate
    ChannelRef csr = sg->createChannel(c->getSrcActor(), acor);
    csr->setName(c->getName() + "-connection-rate");
    csr->getSrcPort()->setRateOfScenario(*s, c->getSrcPort()->getRateOfScenario(*s));
    csr->getDstPort()->setRateOfScenario(*s, 1);
    csr->setInitialAndFinalTokens(c->getInitialTokens());
    persistentTokenNames = "";
    for (uint i = 0; i < csr->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + csr->getName();
    }
    csr->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Add self-loop to actor acor (only 1 token sent at the same time)
    ChannelRef ca = sg->createChannel(acor, acor);
    ca->setName(c->getName() + "-self-edge");
    ca->getSrcPort()->setRateOfScenario(*s, 1);
    ca->getDstPort()->setRateOfScenario(*s, 1);
    ca->setInitialAndFinalTokens(1);
    ca->setPersistentAndFinalTokenNames(CString("PT-") + ca->getName());

    // Create channel from actor which models connection rate to actor which
    // models connection latency
    ChannelRef crl = sg->createChannel(acor, acol);
    crl->setName(c->getName() + "-latency-1");
    crl->getSrcPort()->setRateOfScenario(*s, 1);
    crl->getDstPort()->setRateOfScenario(*s, 1);
    crl->setInitialAndFinalTokens(0);

    // Create channel from actor which models connection latency to actor
    // which models TDMA synchronization
    ChannelRef clt = sg->createChannel(acol, at);
    clt->setName(c->getName() + "-tdma-1");
    clt->getSrcPort()->setRateOfScenario(*s, 1);
    clt->getDstPort()->setRateOfScenario(*s, 1);
    clt->setInitialAndFinalTokens(0);

    // Create channel from actor which models TDMA synchronization to
    // destination actor of the channel
    ChannelRef ctd = sg->createChannel(at, c->getDstActor());
    ctd->setName(c->getName() + "-tdma-2");
    ctd->getSrcPort()->setRateOfScenario(*s, 1);
    ctd->getDstPort()->setRateOfScenario(*s, c->getDstPort()->getRateOfScenario(*s));
    ctd->setInitialAndFinalTokens(0);

    // Create channel from source actor to actor which models channel delay
    ChannelRef csd = sg->createChannel(c->getSrcActor(), ad);
    csd->setName(c->getName() + "-latency-2");
    csd->getSrcPort()->setRateOfScenario(*s, c->getSrcPort()->getRateOfScenario(*s));
    csd->getDstPort()->setRateOfScenario(*s, 1);
    csd->setInitialAndFinalTokens(c->getInitialTokens());
    persistentTokenNames = "";
    for (uint i = 0; i < csd->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + csd->getName();
    }
    csd->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Create channel actor which models channel delay to destination actor
    ChannelRef cdd = sg->createChannel(ad, c->getDstActor());
    cdd->setName(c->getName() + "-latency-3");
    cdd->getSrcPort()->setRateOfScenario(*s, 1);
    cdd->getDstPort()->setRateOfScenario(*s, c->getDstPort()->getRateOfScenario(*s));
    cdd->setInitialAndFinalTokens(0);

    // Set repetition vectors of new actors
    RepetitionVector repetitionVector = s->computeRepetitionVector();
    acol->setRepetitionsOfScenario(*s, repetitionVector[acol->getId()]);
    acor->setRepetitionsOfScenario(*s, repetitionVector[acor->getId()]);
    at->setRepetitionsOfScenario(*s, repetitionVector[at->getId()]);
    ad->setRepetitionsOfScenario(*s, repetitionVector[ad->getId()]);
}

/**
 * The function models a channel mapped to a tile.
 */
void BindingAwareGraphBase::modelChannelToTileBinding(Channel *c,
                                                      ChannelBindingConstraint *bc,
                                                      Scenario *s) {
    CString persistentTokenNames;

    BufferSize b = bc->getBufferSize();

    // Add channel from dst to src actor to model memory space
    ChannelRef m = c->getScenarioGraph()->createChannel(c->getDstActor(), c->getSrcActor());
    m->setName(c->getName() + "-buffer");
    m->getSrcPort()->setRateOfScenario(*s, c->getDstPort()->getRateOfScenario(*s));
    m->getDstPort()->setRateOfScenario(*s, c->getSrcPort()->getRateOfScenario(*s));

    // Initial tokens reflect storage space
    if (b[static_cast<int>(BufferLocation::Value::Mem)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    m->setInitialAndFinalTokens(b[static_cast<int>(BufferLocation::Value::Mem)]
                                - c->getInitialTokens());
    persistentTokenNames = "";
    for (uint i = 0; i < m->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + m->getName();
    }
    m->setPersistentAndFinalTokenNames(persistentTokenNames);
}

/**
 * Enforce possible orderings between the models of actors and/or channels.
 */
void BindingAwareGraphBase::interModelOrdering(Scenario *s, ScenarioGraph *sg) {}

/**
 * Model a static-order schedule inside the binding-aware graph.
 */
void BindingAwareGraphBase::modelStaticOrderScheduleOnProcessor(Scenario *s) {
    /*modelStaticOrderScheduleInGraphUsingBambha(getPlatformBinding(), this,
            mapBindingAwareActorToApplicationGraphActor);*/
}

} // End namespace FSMSADF
