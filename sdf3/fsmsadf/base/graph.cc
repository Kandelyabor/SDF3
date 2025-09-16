/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF graph
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: graph.cc,v 1.3 2010-12-21 21:37:53 sander Exp $
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

#include "graph.h"
#include "component.h"
#include "fsmsadf/base/channel.h"
#include "fsmsadf/base/graph.h"
#include "regexparser_scenarios.h"
#include "scenario.h"
#include "sdf/base/timed/graph.h"
#include <algorithm>
#include <memory>
#include <sstream>

namespace FSMSADF {

using namespace SDF;

/**
 * ScenarioGraph ()
 * Constructor.
 */
ScenarioGraph::ScenarioGraph(const GraphComponent &c) : GraphComponent(c) {}

/**
 * ~ScenarioGraph ()
 * Destructor.
 */
ScenarioGraph::~ScenarioGraph() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated graph object.
 */
std::unique_ptr<ScenarioGraph> ScenarioGraph::create(const GraphComponent &c) {
    return std::make_unique<ScenarioGraph>(c);
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated graph object.
 * All properties of the graph are copied.
 */
std::unique_ptr<ScenarioGraph> ScenarioGraph::createCopy(const GraphComponent &c) const {
    std::unique_ptr<ScenarioGraph> g = create(c);

    // Properties
    g->setName(getName());
    g->setType(getType());

    return g;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated graph object.
 * All properties, actors and channels of the graph are cloned.
 */
std::unique_ptr<ScenarioGraph> ScenarioGraph::clone(const GraphComponent &c) const {
    std::unique_ptr<ScenarioGraph> g = createCopy(c);

    // Actors
    for (const auto &iter : actors) {
        GraphComponent component = GraphComponent(g.get(), iter->getId());
        std::unique_ptr<Actor> a = iter->clone(component);
        g->addActor(std::move(a));
    }

    // Channels
    for (const auto &iter : channels) {
        GraphComponent component = GraphComponent(g.get(), iter->getId());
        std::unique_ptr<Channel> ch = iter->clone(component);
        g->addChannel(std::move(ch));
    }

    // Storage distributions
    for (const auto &[s, sd] : storageDistributions) {
        g->setStorageDistributionsOfScenario(*(g->getGraph()->getScenarioRef(s->getName())), sd);
    }

    return g;
}

Graph *ScenarioGraph::getGraph() const { return dynamic_cast<Graph *>(getParent()); };

/**
 * constructFromXML ()
 * The function initializes all actor properties based on the XML data.
 */
void ScenarioGraph::constructFromXML(const CNodePtr scenarioGraphNode) {
    // Name
    if (!CHasAttribute(scenarioGraphNode, "name")) {
        throw CException("Invalid graph, missing graph name.");
    }
    setName(CGetAttribute(scenarioGraphNode, "name"));

    // Type
    if (!CHasAttribute(scenarioGraphNode, "type")) {
        throw CException("Invalid graph, missing graph type.");
    }
    setType(CGetAttribute(scenarioGraphNode, "type"));

    // Actors
    for (auto *actorNode = CGetChildNode(scenarioGraphNode, "actor"); actorNode != nullptr;
         actorNode = CNextNode(actorNode, "actor")) {
        ActorRef a = createActor(GraphComponent(this, static_cast<CId>(getActors().size())));
        a->constructFromXML(actorNode);
    }

    // Channels
    for (auto *channelNode = CGetChildNode(scenarioGraphNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {
        ChannelRef c = createChannel(GraphComponent(this, static_cast<CId>(getChannels().size())));
        c->constructFromXML(channelNode);
    }
}

/**
 * constructPropertiesFromXML ()
 * The function initializes all actor properties based on the XML data.
 */
void ScenarioGraph::constructPropertiesFromXML(const Scenario &s, const CNodePtr scenarioNode) {
    // Set all properties
    for (CNode *propertiesNode = CGetChildNode(scenarioNode); propertiesNode != nullptr;
         propertiesNode = CNextNode(propertiesNode)) {
        if (CIsNode(propertiesNode, "actorProperties")) {
            if (!CHasAttribute(propertiesNode, "actor")) {
                throw CException("Missing 'actor' in 'actorProperties'");
            }

            if (hasActor(CGetAttribute(propertiesNode, "actor"))) {
                ActorRef a = getActor(CGetAttribute(propertiesNode, "actor"));
                a->constructPropertiesFromXML(s, propertiesNode);
            }
        } else if (CIsNode(propertiesNode, "channelProperties")) {
            if (!CHasAttribute(propertiesNode, "channel")) {
                throw CException("Missing 'channel' in 'channelProperties'");
            }

            if (hasChannel(CGetAttribute(propertiesNode, "channel"))) {
                ChannelRef c = getChannel(CGetAttribute(propertiesNode, "channel"));
                c->constructPropertiesFromXML(s, propertiesNode);
            }
        } else if (CIsNode(propertiesNode, "storageDistributions")) {
            StorageDistributions d;
            d.constructFromXML(propertiesNode);
            setStorageDistributionsOfScenario(s, d);
        }
    }
}

/**
 * setDetectorActor ()
 * This function sets the detectorActor attribute of an actor.
 */
void ScenarioGraph::setDetectorActor(Actor &a) { a.setDetectorActor(true); }

/**
 * convertToXML ()
 * The function converts the graph to XML data.
 */
void ScenarioGraph::convertToXML(const CNodePtr scenarioGraphNode) {
    // Name
    CAddAttribute(scenarioGraphNode, "name", getName());

    // Type
    CAddAttribute(scenarioGraphNode, "type", getType());

    // Actors
    for (const auto &i : actors) {
        i->convertToXML(CAddNode(scenarioGraphNode, "actor"));
    }

    // Channels
    for (const auto &i : channels) {
        i->convertToXML(CAddNode(scenarioGraphNode, "channel"));
    }
}

/**
 * convertPropertiesToXML ()
 * The function converts the graph properties to XML data.
 */
void ScenarioGraph::convertPropertiesToXML(Scenario &s, const CNodePtr scenarioNode) {
    // Actors
    for (const auto &i : actors) {
        CNode *actorPropertiesNode = CAddNode(scenarioNode, "actorProperties");
        i->convertPropertiesToXML(s, actorPropertiesNode);
    }

    // Channels
    for (const auto &i : channels) {
        CNode *channelPropertiesNode = CAddNode(scenarioNode, "channelProperties");
        i->convertPropertiesToXML(s, channelPropertiesNode);
    }

    // Storage distributions
    if (hasStorageDistributionsInScenario(s)) {
        StorageDistributions &d = getStorageDistributionsOfScenario(s);
        d.convertToXML(CAddNode(scenarioNode, "storageDistributions"));
    }
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given id.
 */
ActorRef ScenarioGraph::getActor(const CId id) {
    for (const auto &a : actors) {
        if (a->getId() == id) {
            return a.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given name.
 */
ActorRef ScenarioGraph::getActor(const CString &name) {
    for (const auto &a : actors) {
        if (a->getName() == name) {
            return a.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no actor with name '" + name + "'.");
}

/**
 * hasActor ()
 * The function returns a true when an actor with the given name exists.
 * Otherwise the function returns false.
 */
bool ScenarioGraph::hasActor(const CString &name) {
    return std::any_of(actors.begin(), actors.end(), [name](const std::unique_ptr<Actor> &a) {
        return a->getName() == name;
    });
}

/**
 * addActor ()
 * Add an actor to a graph.
 */
void ScenarioGraph::addActor(std::unique_ptr<Actor> a) { actors.push_back(std::move(a)); }

/**
 * removeActor ()
 * Remove an actor from a graph.
 */
void ScenarioGraph::removeActor(const CString &name) {
    for (auto iter = actors.begin(); iter != actors.end(); iter++) {
        const Actor *a = iter->get();
        if (a->getName() == name) {
            actors.erase(iter);
            return;
        }
    }
    throw CException("Graph '" + getName() + "' has no actor '" + name + "'.");
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
ActorRef ScenarioGraph::createActor() {
    GraphComponent c = GraphComponent(this, static_cast<CId>(getActors().size()));
    auto a = std::make_unique<Actor>(c);
    a->setName(CString("_a") + CString(a->getId()));
    auto *ar = a.get();
    addActor(std::move(a));

    return ar;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
ActorRef ScenarioGraph::createActor(const GraphComponent &c) {
    auto a = std::make_unique<Actor>(c);
    ActorRef ar = a.get();
    addActor(std::move(a));
    return ar;
}

/**
 * reduceActor ()
 * The function attempts to remove the given actor.
 * This is possible only if it has zero execution time,
 * two ports (one in and one out), and if the actors connected
 * to the other ends of those channels have the same
 *  repetition vectors as the given actor.
 */
ChannelRef ScenarioGraph::reduceActor(Scenario *s, Actor *a) {
    // Many checks to see if reduction is possible
    if (!this->hasActor(a->getName())) {
        return nullptr;
    }

    // Actor should have one in and one out port only
    if (a->getPorts().size() != 2) {
        return nullptr;
    }
    Port *inPort = nullptr;
    Port *outPort = nullptr;
    for (const auto &p : a->getPorts()) {
        if (p->getType() == Port::PortType::Out) {
            outPort = p.get();
        } else {
            inPort = p.get();
        }
    }
    if (inPort == nullptr || outPort == nullptr) {
        return nullptr;
    }

    Channel *inChannel = inPort->getChannel();
    Channel *outChannel = outPort->getChannel();

    // All three actors should have same repetition vector and rates
    Actor *inActor = inChannel->getSrcActor();
    Actor *outActor = outChannel->getDstActor();

    // Calculate repetition vector for the graph
    RepetitionVector repetitionVector = s->computeRepetitionVectorOfScenarioGraph(this);

    if ((repetitionVector[a->getId()] != repetitionVector[inActor->getId()])
        || (repetitionVector[a->getId()] != repetitionVector[outActor->getId()])) {
        return nullptr;
    }

    if ((inChannel->getSrcPort()->getRateOfScenario(*s)
         != inChannel->getDstPort()->getRateOfScenario(*s))) {
        return nullptr;
    }

    if ((outChannel->getSrcPort()->getRateOfScenario(*s)
         != outChannel->getDstPort()->getRateOfScenario(*s))) {
        return nullptr;
    }

    const Rate inRate = inChannel->getSrcPort()->getRateOfScenario(*s);
    const Rate outRate = inChannel->getSrcPort()->getRateOfScenario(*s);
    const uint inTokens = inChannel->getInitialTokens();
    const uint outTokens = outChannel->getInitialTokens();
    uint rate = 0;
    uint tokens = 0;

    // Decide which rates to pick
    if ((((inTokens * outRate) % inRate) != 0U) && (((outTokens * inRate) % outRate) != 0U)) {
        return nullptr;
    }
    if (((inTokens * outRate) % inRate) != 0U) { // NOLINT(*branch-clone)
        rate = inRate;
        tokens = inTokens + (outTokens * inRate) / outRate;
    } else if ((((outTokens * inRate) % outRate) != 0U)
               || ((outTokens + (inTokens * outRate) / inRate)
                   <= (inTokens + (outTokens * inRate) / outRate))) {
        rate = outRate;
        tokens = outTokens + (inTokens * outRate) / inRate;
    } else {
        rate = inRate;
        tokens = inTokens + (outTokens * inRate) / outRate;
    }

    // Remove original channels and actor
    CString portName = inChannel->getSrcPort()->getName();
    CString channelName = inChannel->getName();

    inActor->removePort(portName);
    portName = inPort->getName();
    a->removePort(portName);
    this->removeChannel(channelName);

    portName = outChannel->getDstPort()->getName();
    channelName = outChannel->getName();

    outActor->removePort(portName);
    portName = outPort->getName();
    a->removePort(portName);
    this->removeChannel(channelName);

    CString actorName = a->getName();

    // Create new channel between in and out actor
    ChannelRef newChannel = this->createChannel(inActor, outActor);
    newChannel->setName(actorName + "_reduced");

    newChannel->getSrcPort()->setRateOfScenario(*s, rate);
    newChannel->getDstPort()->setRateOfScenario(*s, rate);

    // Set initial tokens and reset persistent token names
    newChannel->setInitialAndFinalTokens(tokens);
    newChannel->resetPersistentAndFinalTokenNames();

    return newChannel;
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given id.
 */
ChannelRef ScenarioGraph::getChannel(const CId id) {
    for (const auto &c : channels) {
        if (c->getId() == id) {
            return c.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no channel with id '" + CString(id) + "'.");
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given name.
 */
ChannelRef ScenarioGraph::getChannel(const CString &name) {
    for (const auto &c : channels) {
        if (c->getName() == name) {
            return c.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no channel with name '" + name + "'.");
}

/**
 * getChannelStartingWith ()
 * The function returns a reference to a channel that starts with the given name.
 */
ChannelRef ScenarioGraph::getChannelStartingWith(const CString &name) {
    for (const auto &c : channels) {
        if (c->getName().compare(0, name.length(), name) == 0) {
            return c.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no channel with name '" + name + "'.");
}

/**
 * hasChannel()
 * The function returns a true when a channel with the given name exists.
 * Otherwise the function returns false.
 */
bool ScenarioGraph::hasChannel(const CString &name) {
    return std::any_of(channels.begin(), channels.end(), [name](const std::unique_ptr<Channel> &c) {
        return c->getName() == name;
    });
}

/**
 * hasChannelStartingWith()
 * The function returns a true when a channel with the given name exists.
 * Otherwise the function returns false.
 */
bool ScenarioGraph::hasChannelStartingWith(const CString &name) {
    return std::any_of(channels.begin(), channels.end(), [name](const std::unique_ptr<Channel> &c) {
        return c->getName().compare(0, name.length(), name) == 0;
    });
}

/**
 * addChannel ()
 * Add a channel to a graph.
 */
void ScenarioGraph::addChannel(std::unique_ptr<Channel> c) { channels.push_back(std::move(c)); }

/**
 * removeChannel ()
 * Remove a channel from a graph.
 */
Channels::iterator ScenarioGraph::removeChannel(const CString &name) {
    for (auto iter = channels.begin(); iter != channels.end(); iter++) {
        Channel *c = iter->get();

        if (c->getName() == name) {
            iter = channels.erase(iter);
            return iter;
        }
    }

    throw CException("Graph '" + getName() + "' has no channel '" + name + "'.");
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
ChannelRef ScenarioGraph::createChannel(const GraphComponent &c) {
    auto ch = std::make_unique<Channel>(c);
    auto *chr = ch.get();
    addChannel(std::move(ch));
    return chr;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
ChannelRef ScenarioGraph::createChannel(Actor *src, Actor *dst) {
    // Create new channel
    GraphComponent c = GraphComponent(this, static_cast<CId>(getChannels().size()));
    ChannelRef ch = createChannel(c);
    ch->setName(CString("_ch") + CString(ch->getId()));

    // Create ports on the actors
    Port *srcP = src->createPort(Port::PortType::Out);
    srcP->setName(CString("_p") + CString(srcP->getId()));
    Port *dstP = dst->createPort(Port::PortType::In);
    dstP->setName(CString("_p") + CString(dstP->getId()));

    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    return ch;
}

ChannelRef ScenarioGraph::createChannel(Actor *src,
                                        Actor *dst,
                                        const std::map<const Scenario *, Rate> &srcRate,
                                        const std::map<const Scenario *, Rate> &dstRate,
                                        const CString &name) {
    // Create new channel
    GraphComponent c = GraphComponent(this, static_cast<CId>(getChannels().size()));
    ChannelRef ch = createChannel(c);
    ch->setName(name);

    // Create ports on the actors
    Port *srcP = src->createPort(Port::PortType::Out);
    srcP->setName(CString("_p") + CString(srcP->getId()));
    srcP->setRate(srcRate);

    Port *dstP = dst->createPort(Port::PortType::In);
    dstP->setName(CString("_p") + CString(dstP->getId()));
    dstP->setRate(dstRate);
    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    return ch;
}
/**
 * extractSDFgraph()
 * The function converts the scenario graph of scenario s to a timed SDF graph.
 */
std::unique_ptr<TimedSDFgraph> ScenarioGraph::extractSDFgraph(const Scenario &s) const {
    auto g = std::make_unique<TimedSDFgraph>();
    SDFcomponent component;

    // Set properties
    g->setName(this->getName() + s.getName());
    g->setType(this->getType());

    // Actors
    for (const auto &a : actors) {
        // Create a new actor
        TimedSDFactor *newA = g->createActor();

        // Properties
        newA->setName(a->getName());
        newA->setType(a->getType());

        // Default processor
        TimedSDFactor::Processor *newProc = newA->addProcessor(a->getDefaultProcessorType());
        newProc->type = a->getDefaultProcessorType();
        newProc->execTime = a->getExecutionTimeOfScenario(s, a->getDefaultProcessorType());
        newA->setDefaultProcessor(newProc->type);
    }

    // Channels
    for (const auto &c : channels) {
        // Create a new channel
        TimedSDFchannel *newC =
                g->createChannel(g->getActor(c->getSrcActor()->getName()),
                                 static_cast<SDFrate>(c->getSrcPort()->getRateOfScenario(s)),
                                 g->getActor(c->getDstActor()->getName()),
                                 static_cast<SDFrate>(c->getDstPort()->getRateOfScenario(s)),
                                 c->getInitialTokens());

        // Properties
        newC->setName(c->getName());

        // Port names
        newC->getSrcPort()->setName(c->getSrcPort()->getName());
        newC->getDstPort()->setName(c->getDstPort()->getName());
        newC->isCapacityChannel = c->isCapacityChannel;
    }

    return g;
}

/**
 * getPartialRepetitionVector()
 * The function get partial repetition vector from spec of weakly consistent fsmsadf
 */
std::unique_ptr<RepetitionVector>
ScenarioGraph::getPartialRepetitionVector(const Scenario &s) const {
    auto repVec = std::make_unique<RepetitionVector>();
    // Actors
    for (const auto &a : actors) {
        repVec->push_back(static_cast<int>(a->getRepetitionsOfScenario(s)));
    }
    return repVec;
}

/**
 * setPartialRepetitionVector()
 * The function sets the partial repetition vector for a scenario
 */
void ScenarioGraph::setPartialRepetitionVector(Scenario *s, const RepetitionVector &v) {
    // Actors
    for (Actors::const_iterator i = actors.begin(); i != actors.end(); i++) {
        Actor *a = i->get();
        a->setRepetitionsOfScenario(*s, v[a->getId()]);
    }
}

/**
 * The function removes all scenario except scenario s from the graph.
 */
void ScenarioGraph::isolateScenario(const Scenario &s) {
    // Actors
    for (const auto &a : actors) {
        a->isolateScenario(s);
    }

    // Channels
    for (const auto &c : channels) {
        c->isolateScenario(s);
    }

    // Storage distributions
    if (hasStorageDistributionsInScenario(s)) {
        StorageDistributions d = getStorageDistributionsOfScenario(s);
        storageDistributions.clear();
        setStorageDistributionsOfScenario(s, d);
    }
}

/**
 * getStorageDistributionsOfScenario ()
 * The function returns the storage distributions of this scenario graph in
 * scenario s, or when scenario s is not specified it returns the storage
 * distributions in the default scenario. When also no default scenario exists,
 * an exception is thrown.
 */
StorageDistributions &ScenarioGraph::getStorageDistributionsOfScenario(const Scenario &s) {
    if (storageDistributions.find(&s) != storageDistributions.end()) {
        return storageDistributions.find(&s)->second;
    }

    if (storageDistributions.find(getDefaultScenario()) == storageDistributions.end()) {
        throw CException("Scenario graph '" + getName() + "' has no storage distributions.");
    }

    return storageDistributions.find(getDefaultScenario())->second;
}

/**
 * setStorageDistributionsOfScenario ()
 * Set the storage distributions of this scenario graph in scenario s.
 */
void ScenarioGraph::setStorageDistributionsOfScenario(const Scenario &s,
                                                      const StorageDistributions &d) {
    storageDistributions[&s] = d;
}

/**
 * hasStorageDistributionsInScenario ()
 * The function returns true when storage distributions have been specified for
 * scenario s or through a default scenario. Otherwise the function returns
 * false.
 */
bool ScenarioGraph::hasStorageDistributionsInScenario(const Scenario &s) const {
    return storageDistributions.find(&s) != storageDistributions.end()
           || storageDistributions.find(getDefaultScenario()) != storageDistributions.end();
}

/**
 * getDefaultScenario ()
 * The function returns a pointer to the default scenario.
 */
ScenarioRef ScenarioGraph::getDefaultScenario() const { return getGraph()->getDefaultScenario(); }

/**
 * The function returns a list with the names of all persistent tokens that
 * are used in the scenario graph
 * @return list with names of persistent tokens
 */
CStrings ScenarioGraph::getPersistentTokens() const {
    CStrings pt;

    // Create list of persistent tokens from persistent token map of first graph
    PersistentTokenMap mp = this->getPersistentTokenMap();
    for (auto &[tokenName, i_s] : mp) {
        pt.push_back(tokenName);
    }

    return pt;
}

/**
 * The function returns a list with the names of all final persistent tokens that
 * are used in the scenario graph
 * @return list with names of persistent tokens
 */
CStrings ScenarioGraph::getPersistentTokensFinal() const {
    CStrings pt;

    // Create list of persistent tokens from persistent token map of first graph
    PersistentTokenMap mp = this->getPersistentTokenMapFinal();
    for (auto &[tokenName, i_s] : mp) {
        pt.push_back(tokenName);
    }

    return pt;
}

/**
 * The function checks of the scenario graph is strongly consistent
 * i.e., if all initial tokens return after one iteration.
 * @return list with names of persistent tokens
 */
bool ScenarioGraph::isStronglyConsistent() const {
    PersistentTokenMap ptm = this->getPersistentTokenMap();
    PersistentTokenMap ptmFinal = this->getPersistentTokenMapFinal();
    return ptm == ptmFinal;
}

/**
 * Test whether the scenario graph is consistent.
 */
bool ScenarioGraph::isWeaklyConsistent(const Scenario &s) const {

    // test relation between initial tokens, repetitions and final tokens.

    // for each channel test tokens
    const Channels &chs = this->getChannels();
    return std::all_of(chs.begin(), chs.end(), [&s](const std::unique_ptr<Channel> &ch) {
        Port *sp = ch->getSrcPort();
        Port *dp = ch->getDstPort();
        uint final = ch->getInitialTokens()
                     + (sp->getRateOfScenario(s) * sp->getActor().getRepetitionsOfScenario(s))
                     - (dp->getRateOfScenario(s) * dp->getActor().getRepetitionsOfScenario(s));

        return final == ch->getFinalTokens();
    });
}

/**
 * The function returns a mapping of persistent token names to a pair of a
 * channel in this scenario graph and a sequence number for the initial token on
 * this channel.
 * @return mapping of persistent token names to a pair of a channel in this
 *         scenario graph and a sequence for the initial token on this channel
 */
PersistentTokenMap ScenarioGraph::getPersistentTokenMap() const {
    PersistentTokenMap mp;

    for (const auto &c : getChannels()) {
        const CStrings &pt = c->getPersistentTokenNames();
        uint ci = 0;
        for (const auto &j : pt) {
            mp[j] = std::pair<Channel *, uint>(c.get(), ci);
            ci++;
        }
    }
    return mp;
}

/**
 * The function returns a mapping of persistent final token names to a pair of a
 * channel in this scenario graph and a sequence number for the initial token on
 * this channel.
 * @return mapping of persistent token names to a pair of a channel in this
 *         scenario graph and a sequence for the initial token on this channel
 */
PersistentTokenMap ScenarioGraph::getPersistentTokenMapFinal() const {
    PersistentTokenMap mp;

    for (const auto &c : getChannels()) {
        const CStrings &pt = c->getPersistentFinalTokenNames();
        uint ci = 0;
        for (const auto &j : pt) {
            mp[j] = std::pair<Channel *, uint>(c.get(), ci);
            ci++;
        }
    }

    return mp;
}

/**
 * The function returns a mapping of persistent token names to a sequence
 * number of the initial tokens in this graph.
 * (This number will correspond to the index of the token in the max-plus matrix.)
 * @return mapping of persistent token names to an index number on the initial tokens
 * in the graph.
 */
PersistentTokenIndexMap ScenarioGraph::getPersistentTokenIndexMap() const {
    PersistentTokenIndexMap mp;

    uint ti = 0;
    for (const auto &c : getChannels()) {
        uint nrTokens = c->getInitialTokens();
        const CStrings &pt = c->getPersistentTokenNames();
        for (const auto &j : pt) {
            mp[j] = ti++;
        }
        ti += nrTokens - static_cast<uint>(pt.size());
    }

    return mp;
}

/**
 * The function returns a mapping of persistent final token names to a sequence
 * number of the final tokens in this graph.
 * (This number will correspond to the index of the token in the max-plus matrix.)
 * @return mapping of persistent final token names to an index number on the final tokens
 * in the graph.
 */
PersistentTokenIndexMap ScenarioGraph::getPersistentTokenIndexMapFinal() const {
    PersistentTokenIndexMap mp;

    uint ti = 0;
    for (const auto &c : getChannels()) {
        uint nrTokens = c->getFinalTokens();
        const CStrings &pt = c->getPersistentFinalTokenNames();
        for (const auto &j : pt) {
            mp[j] = ti++;
        }
        ti += nrTokens - static_cast<uint>(pt.size());
    }
    return mp;
}

/**
 * The function checks if there are channels defining the same persistent token names
 */
void ScenarioGraph::checkDuplicatePersistentTokens() const {
    CStrings pts;
    CStrings ptsFinal;
    for (const auto &c : getChannels()) {
        const CStrings &pt = c->getPersistentTokenNames();
        for (const auto &j : pt) {
            if (std::find(pts.begin(), pts.end(), j) != pts.end()) {
                throw CException("Persistent token " + j
                                 + " occurs multiple times in scenario graph " + this->getName());
            }
            pts.push_back(j);
        }
        const CStrings &ptf = c->getPersistentFinalTokenNames();
        for (const auto &j : ptf) {
            if (std::find(ptsFinal.begin(), ptsFinal.end(), j) != ptsFinal.end()) {
                throw CException("Final persistent token " + j
                                 + " occurs multiple times in scenario graph " + this->getName());
            }
            ptsFinal.push_back(j);
        }
    }
}

/**
 * reducePersistentTokens ()
 * The function attempts to reduce the amount of persistent tokens
 * for this channel by applying two transforms described in
 * the thesis of Damavandpeyma.
 * WARNING: the rates, amount and names of initial tokens
 * and the scenario graph will be changed by this function.
 * This might or might not be acceptable, use at own risk.
 */
void ScenarioGraph::reduceInitialTokens(const Scenario &s, Channel &c) {
    uint initial = c.getInitialTokens();
    const Rate srcRate = c.getSrcPort()->getRateOfScenario(s);
    const Rate dstRate = c.getDstPort()->getRateOfScenario(s);

    // Detect conditions under which reduction does not work
    if ((initial == 0U) || (dstRate == 1 && srcRate == 1)) {
        return;
    }

    if (((initial / dstRate) != 0) && ((initial / dstRate) < initial)
        && ((initial / dstRate) <= (initial / srcRate))) {
        consumerGrouping(s, c);
        initial = c.getInitialTokens();
    }
    if (((initial / srcRate) != 0) && ((initial / srcRate) < initial)) {
        producerGrouping(s, c);
        initial = c.getInitialTokens();

        if (((initial / dstRate) == 0) || ((initial / dstRate) >= initial)) {
            return;
        }

        consumerGrouping(s, c);
    }
}

/**
 * consumerGrouping ()
 * The function attempts to reduce the amount of persistent tokens
 * for this channel by applying the consumer side grouping method
 * proposed in the thesis of Damavandpeyma.
 */
ChannelRef ScenarioGraph::consumerGrouping(const Scenario &s, Channel &c) {
    const Rate dstRate = c.getDstPort()->getRateOfScenario(s);
    Actor *dstActor = c.getDstActor();

    // Consumer side grouping transform
    // Create an actor with zero latency for conversion
    ActorRef rcActor =
            this->createActor(GraphComponent(this, static_cast<CId>(this->getActors().size())));
    rcActor->setName(c.getName() + "_reduceConsumer");
    rcActor->setExecutionTimeOfScenario(s, "reduceConsumer", 0);
    rcActor->setDefaultProcessorType("reduceConsumer");

    // Re-connect the dst of the original channel to the converter
    CString name = c.getDstPort()->getName();
    PortRef newDstPort = rcActor->createPort(Port::PortType::In);
    newDstPort->setName(CString("_p") + CString(newDstPort->getId()));
    newDstPort->setRateOfScenario(s, dstRate);
    c.connectDst(nullptr);
    dstActor->removePort(name);
    c.connectDst(newDstPort);

    // Add a channel between converter and original dst
    ChannelRef rcChannel = this->createChannel(rcActor, dstActor);
    rcChannel->setName(c.getName() + "_reduceConsumer-dst");
    rcChannel->getSrcPort()->setRateOfScenario(s, 1);
    rcChannel->getDstPort()->setRateOfScenario(s, 1);

    // Set initial tokens and reset persistent token names
    rcChannel->setInitialAndFinalTokens(c.getInitialTokens() / dstRate);
    rcChannel->resetPersistentAndFinalTokenNames();

    // Set initial tokens and reset persistent token names
    c.setInitialAndFinalTokens(c.getInitialTokens() % dstRate);
    c.resetPersistentAndFinalTokenNames();

    // Set repetition vector of new actor
    RepetitionVector repetitionVector = s.computeRepetitionVector();
    rcActor->setRepetitionsOfScenario(s, repetitionVector[rcActor->getId()]);

    return rcChannel;
}

/**
 * producerGrouping ()
 * The function attempts to reduce the amount of persistent tokens
 * for this channel by applying the producer side grouping method
 * proposed in the thesis of Damavandpeyma.
 */
ChannelRef ScenarioGraph::producerGrouping(const Scenario &s, Channel &c) {
    const Rate srcRate = c.getSrcPort()->getRateOfScenario(s);
    Actor *srcActor = c.getSrcActor();

    // Producer side grouping transform
    // Create an actor with zero latency for conversion
    ActorRef rpActor =
            this->createActor(GraphComponent(this, static_cast<CId>(this->getActors().size())));
    rpActor->setName(c.getName() + "_reduceProducer");
    rpActor->setExecutionTimeOfScenario(s, "reduceProducer", 0);
    rpActor->setDefaultProcessorType("reduceProducer");

    // Re-connect the src of the original channel to the converter
    CString name = c.getSrcPort()->getName();
    PortRef newSrcPort = rpActor->createPort(Port::PortType::Out);
    newSrcPort->setName(CString("_p") + CString(newSrcPort->getId()));
    newSrcPort->setRateOfScenario(s, srcRate);
    c.connectSrc(nullptr);
    srcActor->removePort(name);
    c.connectSrc(newSrcPort);

    // Add a channel between original src and converter
    ChannelRef rpChannel = this->createChannel(srcActor, rpActor);
    rpChannel->setName(c.getName() + "_src-reduceProducer");
    rpChannel->getSrcPort()->setRateOfScenario(s, 1);
    rpChannel->getDstPort()->setRateOfScenario(s, 1);

    // Set initial tokens and reset persistent token names
    rpChannel->setInitialAndFinalTokens(c.getInitialTokens() / srcRate);
    rpChannel->resetPersistentAndFinalTokenNames();

    // Set initial tokens and persistent reset token names
    c.setInitialAndFinalTokens(c.getInitialTokens() % srcRate);
    c.resetPersistentAndFinalTokenNames();

    // Set repetition vector of new actor using the new scenario graph
    // @TODO [Marc] Shady fix to avoid error in test
    // A new scenario graph is begin constructed, but repetition vector was erroneously
    // computed with the old scenario graph referred in scenario s
    // auto *backup = s.getScenarioGraph();
    auto s2 = s.createCopy(*(s.getParent()));
    s2->setScenarioGraph(this);
    // s.setScenarioGraph(this);
    RepetitionVector repetitionVector = s2->computeRepetitionVector();
    // restore the scenario graph
    // s.setScenarioGraph(backup);
    assert(rpActor->getId() < repetitionVector.size());
    rpActor->setRepetitionsOfScenario(s, repetitionVector[rpActor->getId()]);

    return rpChannel;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
std::ostream &ScenarioGraph::print(std::ostream &out) {
    out << "ScenarioGraph (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;
    out << std::endl;

    for (const auto &a : actors) {
        a->print(out);
    }

    for (const auto &c : channels) {
        c->print(out);
    }

    out << std::endl;
    return out;
}

/**
 * Graph ()
 * Constructor.
 */
Graph::Graph(const GraphComponent &c) :
    GraphComponent(c),
    fsm(nullptr),
    defaultScenario(GraphComponent(this, 0, "_default_")),
    unitsProperty(nullptr) {}

/**
 * Constructor.
 */
Graph::Graph(Graph *g) :
    GraphComponent(nullptr, 0),
    fsm(nullptr),
    defaultScenario(GraphComponent(this, 0, "_default_")),
    unitsProperty(nullptr) {

    // Properties
    setName(g->getName());
    setThroughputConstraint(g->getThroughputConstraint());

    // Scenarios (create copy, but do not set scenario graph as these do not
    // exist yet)
    for (const auto &s : g->getScenarios()) {
        // Do not clone the default scenario
        if (s.get() != getDefaultScenario()) {
            std::unique_ptr<Scenario> ss = s->createCopy(GraphComponent(this, s->getId()));
            addScenario(std::move(ss));
        }
    }

    // Scenario graphs
    for (const auto &i : g->getScenarioGraphs()) {
        std::unique_ptr<ScenarioGraph> sg = i->clone(GraphComponent(this, i->getId()));
        addScenarioGraph(std::move(sg));
    }

    // Link scenarios to scenario graphs
    for (const auto &s : g->getScenarios()) {
        // Skip the default scenario as it is not related to any scenario graph
        if (s.get() != getDefaultScenario()) {
            getScenarioRef(s->getName())
                    ->setScenarioGraph(getScenarioGraph(s->getScenarioGraph()->getName()));
        }
    }

    // FSM
    if (g->getFSM() != nullptr) {
        setFSM(g->getFSM()->clone(GraphComponent(this, g->getFSM()->getId())));
    }
}

/**
 * Graph ()
 * Constructor.
 */
Graph::Graph() :
    GraphComponent(nullptr, 0),
    defaultScenario(GraphComponent(this, 0, "_default_")),
    fsm(nullptr),
    unitsProperty(nullptr) {}

/**
 * ~Graph ()
 * Destructor.
 */
Graph::~Graph() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated graph object.
 */
std::unique_ptr<Graph> Graph::create(const GraphComponent &c) { return std::make_unique<Graph>(c); }

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated graph object.
 * All properties of the graph are copied.
 */
std::unique_ptr<Graph> Graph::createCopy(const GraphComponent &c) const {
    std::unique_ptr<Graph> g = create(c);

    // Properties
    g->setName(getName());
    g->setThroughputConstraint(getThroughputConstraint());

    return g;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated graph object.
 * All properties, actors and channels of the graph are cloned.
 */
std::unique_ptr<Graph> Graph::clone(const GraphComponent &c) const {
    std::unique_ptr<Graph> g = createCopy(c);

    // Scenarios (create copy, but do not set scenario graph as these do not
    // exist yet)
    for (const auto &i : scenarios) {
        // Do not clone the default scenario
        if (i.get() != getDefaultScenario()) {
            std::unique_ptr<Scenario> s = i->createCopy(GraphComponent(g.get(), i->getId()));
            g->addScenario(std::move(s));
        }
    }

    // Scenario graphs
    for (const auto &i : scenarioGraphs) {
        std::unique_ptr<ScenarioGraph> sg = i->clone(GraphComponent(g.get(), i->getId()));
        g->addScenarioGraph(std::move(sg));
    }

    // Link scenarios to scenario graphs
    for (const auto &s : scenarios) {
        // Skip the default scenario as it is not related to any scenario graph
        if (s.get() != getDefaultScenario()) {
            g->getScenarioRef(s->getName())
                    ->setScenarioGraph(g->getScenarioGraph(s->getScenarioGraph()->getName()));
        }
    }

    // FSM
    if (getFSM() != nullptr) {
        g->setFSM(getFSM()->clone(GraphComponent(g.get(), getFSM()->getId())));
    }

    if (!getRegEx().empty()) {
        g->setRegEx(getRegEx());
    }

    // Units
    if (getUnits() != nullptr) {
        g->setUnits(getUnits());
    }

    return g;
}

/**
 * constructFromXML ()
 * The function initializes all actor properties based on the XML data.
 */
void Graph::constructFromXML(const CNodePtr applicationGraphNode) {
    CString name;

    // Name
    if (!CHasAttribute(applicationGraphNode, "name")) {
        throw CException("Application graph has no name.");
    }
    setName(CGetAttribute(applicationGraphNode, "name"));

    // Graph (node: fsmsadf)
    if (!CHasChildNode(applicationGraphNode, "fsmsadf")) {
        throw CException("Graph does not contain an FSM-SADF graph.");
    }
    CNode *fsmsadfNode = CGetChildNode(applicationGraphNode, "fsmsadf");
    for (CNode *sgNode = CGetChildNode(fsmsadfNode, "scenariograph"); sgNode != nullptr;
         sgNode = CNextNode(sgNode, "scenariograph")) {
        auto sg = std::make_unique<ScenarioGraph>(
                GraphComponent(this, static_cast<CId>(getScenarioGraphs().size())));
        sg->constructFromXML(sgNode);
        addScenarioGraph(std::move(sg));
    }

    // Graph properties (node: fsmsadfProperties)
    if (CHasChildNode(applicationGraphNode, "fsmsadfProperties")) {
        CNode *fsmsadfPropertiesNode = CGetChildNode(applicationGraphNode, "fsmsadfProperties");
        for (CNode *propertiesNode = CGetChildNode(fsmsadfPropertiesNode);
             propertiesNode != nullptr;
             propertiesNode = CNextNode(propertiesNode)) {
            if (CIsNode(propertiesNode, "defaultProperties")) {
                // Iterate over all scenario graphs
                for (const auto &i : scenarioGraphs) {
                    i->constructPropertiesFromXML(*getDefaultScenario(), propertiesNode);
                }
            }

            if (CIsNode(propertiesNode, "scenarios")) {
                std::unique_ptr<Scenario> s = nullptr;

                for (CNode *scenarioNode = CGetChildNode(propertiesNode, "scenario");
                     scenarioNode != nullptr;
                     scenarioNode = CNextNode(scenarioNode, "scenario")) {
                    // Name
                    if (!CHasAttribute(scenarioNode, "name")) {
                        throw CException("Scenario has no name.");
                    }
                    name = CGetAttribute(scenarioNode, "name");

                    // Scenario graph
                    if (!CHasAttribute(scenarioNode, "graph")) {
                        throw CException("Scenario " + name + " has no graph.");
                    }
                    auto *sg = getScenarioGraph(CGetAttribute(scenarioNode, "graph"));

                    // reward
                    CDouble reward = 1.0;
                    if (CHasAttribute(scenarioNode, "reward")) {
                        reward = static_cast<CDouble>(
                                std::stod(CGetAttribute(scenarioNode, "reward")));
                    } else {
                        throw CException("Scenario " + name + " has no reward specification.");
                    }
                    // Create a new scenario, add one to skip default scenario
                    s = std::make_unique<Scenario>(
                            GraphComponent(this, static_cast<CId>(getScenarios().size() + 1)));
                    s->setName(name);
                    s->setScenarioGraph(sg);
                    s->setReward(reward);
                    ScenarioRef sr = s.get();
                    addScenario(std::move(s));

                    // Set the properties of the actors and channels for this
                    // scenario
                    sg->constructPropertiesFromXML(*sr, scenarioNode);

                    // compute the repetition vector if repetitions have not been explicitly
                    // specified
                    if (!sr->allActorsHaveRepetitions()) {
                        if (sr->noActorsHaveRepetitions()) {
                            RepetitionVector repetitionVector = sr->computeRepetitionVector();
                            sr->setRepetitionVector(repetitionVector);
                        } else {
                            throw CException(
                                    "Actors repetitions are only partially specified in scenario "
                                    + sr->getName());
                        }
                    }
                }
            }

            if (CIsNode(propertiesNode, "graphProperties")) {
                if (CHasChildNode(propertiesNode, "timeConstraints")) {
                    CNode *timeConstraintsNode = CGetChildNode(propertiesNode, "timeConstraints");
                    if (CHasChildNode(timeConstraintsNode, "throughput")) {
                        CString throughputConstraint =
                                CGetNodeContent(CGetChildNode(timeConstraintsNode, "throughput"));
                        setThroughputConstraint(static_cast<CDouble>(throughputConstraint));
                    }
                }
            }

            if (CIsNode(propertiesNode, "units")) {
                setUnits(CCopyNode(propertiesNode));
            }
        }
    }

    // check if the persistent tokens are correct in the scenario graphs
    // Iterate over all scenario graphs
    for (const auto &i : scenarioGraphs) {
        i->checkDuplicatePersistentTokens();
    }

    // FSM (node: fsm)
    if (CHasChildNode(applicationGraphNode, "fsm")) {
        this->fsm = std::make_unique<FSM>(GraphComponent(this, 0));
        this->fsm->constructFromXML(CGetChildNode(applicationGraphNode, "fsm"));

        // Set the detector actor
        for (const auto &tempSg : scenarioGraphs) {
            if (tempSg->hasActor(fsm->getDetectorName())) {
                FSMSADF::ScenarioGraph::setDetectorActor(
                        *(tempSg->getActor(fsm->getDetectorName())));
            }
        }
    }
    // regex (node : regex)
    if (CHasChildNode(applicationGraphNode, "regex")) {
        CNode *regexNode = CGetChildNode(applicationGraphNode, "regex");
        if (!CHasAttribute(regexNode, "string")) {
            throw CException("regex node does not have a string attribute.");
        }
        regex = CGetAttribute(regexNode, "string");
    }
}

/**
 * convertToXML ()
 * The function converts all the graph to XML data.
 */
void Graph::convertToXML(const CNodePtr applicationGraphNode) const {
    std::set<CString> defaultActors;
    std::set<CString> defaultChannels;

    // Graph (name)
    CAddAttribute(applicationGraphNode, "name", getName());

    // Graph (node: fsmsadf)
    CNode *fsmsadfNode = CAddNode(applicationGraphNode, "fsmsadf");
    for (const auto &sg : scenarioGraphs) {
        CNode *sgNode = CAddNode(fsmsadfNode, "scenariograph");
        CAddAttribute(sgNode, "name", sg->getName());
        sg->convertToXML(sgNode);
    }

    // Graph properties (node: fsmsadfProperties)
    CNode *fsmsadfPropertiesNode = CAddNode(applicationGraphNode, "fsmsadfProperties");

    // Default properties (present on all scenario graphs)
    CNode *defaultPropertiesNode = CAddNode(fsmsadfPropertiesNode, "defaultProperties");
    for (const auto &sg : scenarioGraphs) {
        // Actors
        for (const auto &i : sg->getActors()) {
            // Not seen this actor before?
            if (defaultActors.find(i->getName()) == defaultActors.end()) {
                CNode *actorPropertiesNode = CAddNode(defaultPropertiesNode, "actorProperties");
                i->convertPropertiesToXML(*getDefaultScenario(), actorPropertiesNode);
                defaultActors.insert(i->getName());
            }
        }
    }
    for (const auto &sg : scenarioGraphs) {
        // Channels
        for (const auto &i : sg->getChannels()) {
            // Not seen this channel before?
            if (defaultChannels.find(i->getName()) == defaultChannels.end()) {
                CNode *channelPropertiesNode = CAddNode(defaultPropertiesNode, "channelProperties");
                i->convertPropertiesToXML(*getDefaultScenario(), channelPropertiesNode);
                defaultChannels.insert(i->getName());
            }
        }
    }

    // Scenarios
    CNode *scenariosNode = CAddNode(fsmsadfPropertiesNode, "scenarios");
    for (const auto &s : scenarios) {
        CNode *scenarioNode = CAddNode(scenariosNode, "scenario");
        CAddAttribute(scenarioNode, "name", s->getName());
        CAddAttribute(scenarioNode, "graph", s->getScenarioGraph()->getName());
        CAddAttribute(scenarioNode, "reward", CString(s->getReward()));

        s->convertToXML(scenarioNode);
    }

    // Graph properties
    CNode *graphPropertiesNode = CAddNode(fsmsadfPropertiesNode, "graphProperties");
    CNode *timeConstraintsNode = CAddNode(graphPropertiesNode, "timeConstraints");
    CAddNode(timeConstraintsNode, "throughput", getThroughputConstraint());

    // Unit properties
    if (getUnits() != nullptr) {
        CAddNode(fsmsadfPropertiesNode, getUnits());
    }

    // FSM (node: fsm)
    if (fsm != nullptr) {
        fsm->convertToXML(CAddNode(applicationGraphNode, "fsm"));
    }
}

/**
 * extractSDFgraph()
 * The function converts the scenario graph of scenario s to a timed SDF graph.
 */
std::unique_ptr<TimedSDFgraph> Graph::extractSDFgraph(const Scenario &s) {
    return s.getScenarioGraph()->extractSDFgraph(s);
}

/**
 * getPartialRepetitionVector()
 * The function returns the partial repetition vector as specified for the scenario in a weakly
 * consistent fsmsadf.
 */
std::unique_ptr<RepetitionVector> Graph::getPartialRepetitionVector(const Scenario &s) {
    return s.getScenarioGraph()->getPartialRepetitionVector(s);
}

/**
 * isolateScenarios()
 * The function creates a separate scenario graph for each scenario.
 */
void Graph::isolateScenarios() {
    // Iterate over the set of scenarios
    for (const auto &s : scenarios) {
        // Clone the scenario graph
        std::unique_ptr<ScenarioGraph> sgp = s->getScenarioGraph()->clone(*(s->getScenarioGraph()));
        auto *sg = sgp.get();
        addScenarioGraph(std::move(sgp));

        // Update the name of the scenario graph
        sg->setName(sg->getName() + "-" + s->getName());
        sg->setId(s->getId());

        // Update the scenario graph associated with this scenario
        s->setScenarioGraph(sg);

        // Isolate the scenario s in this scenario graph
        sg->isolateScenario(*s);
    }

    // Remove all scenario graphs which are not used in any scenario
    for (auto i = scenarioGraphs.begin(); i != scenarioGraphs.end();) {
        ScenarioGraph *sg = i->get();
        bool graphUsed = false;

        // Iterate over all scenarios
        for (const auto &s : scenarios) {
            if (s->getScenarioGraph() == sg) {
                graphUsed = true;
                break;
            }
        }

        // Is the graph not used?
        if (!graphUsed) {
            scenarioGraphs.erase(i++);
        } else {
            ++i;
        }
    }
}

/**
 * hasIsolatedScenarios()
 * The function returns true when the scenarios in the graph are isolated (i.e.
 * each scenario graph is used in exactly one scenario). Otherwise the function
 * returns false.
 */
bool Graph::hasIsolatedScenarios() const {
    std::set<ScenarioGraph *> usedScenarioGraphs;

    // Iterate over all scenarios
    for (const auto &s : scenarios) {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Has this scenario graph been used in another scenario?
        if (usedScenarioGraphs.find(sg) != usedScenarioGraphs.end()) {
            return false;
        }

        // Add scenario graph to the set of used graphs
        usedScenarioGraphs.insert(sg);
    }
    return true;
}

/**
 * getScenarioRef()
 * The function returns a pointer to the scenario with the supplied name.
 */
ScenarioRef Graph::getScenarioRef(const CString &name) {
    // Check regular scenarios
    for (const auto &i : scenarios) {
        if (i->getName() == name) {
            return i.get();
        }
    }

    // Check default scenario
    if (getDefaultScenario()->getName() == name) {
        return getDefaultScenario();
    }

    throw CException("Graph does not contain a scenario with name '" + name + "'.");
}

/**
 * getScenarioRef()
 * The function returns a pointer to the scenario with the supplied name.
 */
const Scenario *Graph::getScenarioRef(const CString &name) const {
    // Check regular scenarios
    for (const auto &i : scenarios) {
        if (i->getName() == name) {
            return i.get();
        }
    }

    // Check default scenario
    if (getDefaultScenario()->getName() == name) {
        return getDefaultScenario();
    }

    throw CException("Graph does not contain a scenario with name '" + name + "'.");
}

/**
 * getScenarioGraph()
 * The function returns a pointer to the scenario graph with the supplied name.
 */
ScenarioGraph *Graph::getScenarioGraph(const CString &name) const {
    for (const auto &i : scenarioGraphs) {
        if (i->getName() == name) {
            return i.get();
        }
    }

    throw CException("Graph does not contain a scenario graph with name '" + name + "'.");
}

/**
 * The function replaces all the scenario graph fromG with the scenario graph
 * toG. The function replaces also all references to this scenario graph inside
 * the FSM. The scenario graph fromG will be removed from the graph, but the
 * scenario graph will not be destroyed. The user is responsible for freeing the
 * memory claimed by this object.
 * @param scenario graph fromG
 * @param scenario graph toG
 */
void Graph::replaceScenarioGraph(ScenarioGraph *fromG, std::unique_ptr<ScenarioGraph> toG) {
    // Update all scenarios
    for (const auto &s : scenarios) {
        if (s->getScenarioGraph() == fromG) {
            s->setScenarioGraph(toG.get());
        }
    }

    // Add new scenario graph to the application graph
    this->addScenarioGraph(std::move(toG));

    // Remove old scenario graph from the application graph
    this->scenarioGraphs.removeScenarioGraph(fromG);
}

/**
 * getDefaultScenario()
 * The function returns a pointer to the default scenario.
 */
ScenarioRef Graph::getDefaultScenario() { return &defaultScenario; }

/**
 * getDefaultScenario()
 * The function returns a pointer to the default scenario.
 */
const Scenario *Graph::getDefaultScenario() const { return &defaultScenario; }

bool Graph::isStronglyConsistent() const {
    const ScenarioGraphs &s = this->getConstScenarioGraphs();
    return std::all_of(s.begin(), s.end(), [](const std::unique_ptr<ScenarioGraph> &g) {
        return g->isStronglyConsistent();
    });
}

/**
 * Test whether the scenario specifications are consistent.
 */
bool Graph::isWeaklyConsistent() const {

    // test the individual scenarios for consistency
    const Scenarios &ss = this->getConstScenarios();
    for (const auto &si : ss) {
        if (!(si->getScenarioGraph())->isWeaklyConsistent(*si)) {
            return false;
        }
    }

    // test consistency of the fsm transitions
    FSM *fsm = this->getFSM();
    FSMstates &st = fsm->getStates();
    for (const auto &s : st) {
        const FSMtransitionRefs &tr = s->getTransitions();
        for (const auto &t : tr) {
            FSMstate *sts = t->getSrcState();
            FSMstate *std = t->getDstState();
            ScenarioRef scs = sts->getScenario();
            ScenarioRef scd = std->getScenario();
            CStrings ptd = FSMSADF::Graph::getPersistentTokensOfScenario(*scd);
            CStrings pts = FSMSADF::Graph::getPersistentTokensFinalOfScenario(*scs);
            if (ptd != pts) {
                return false;
            }
        }
    }

    return true;
}

void Graph::checkWeaklyConsistency() {
    // test the individual scenarios for consistency
    const Scenarios &ss = this->getScenarios();
    for (const auto &si : ss) {
        if (!(si->getScenarioGraph())->isWeaklyConsistent(*si)) {
            throw CException("scenario " + si->getName() + " is not weakly consistent");
        }
    }

    // test consistency of the language formalized by the regex
    RegExParserScenarios parser(this->getRegEx(), *this);
    std::unique_ptr<RegEx> regex = parser.parse();
    auto &regExs = dynamic_cast<RegExScenarios &>(*regex);
    std::unique_ptr<RegExPersistentTokens> ts = regExs.checkConsistency();
}

void Graph::checkScenarioNames() {
    // test the individual scenarios for consistency
    const Scenarios &ss = this->getScenarios();

    const std::string forbidden_chars = "+,.@^()*";

    for (const auto &si : ss) {
        std::string::const_iterator s;
        for (s = forbidden_chars.begin(); s != forbidden_chars.end(); ++s) {
            if (si->getName().find(*s) != std::string::npos) {
                throw CException("Scenario " + si->getName()
                                 + " has an invalid name. The following symbols are not allowed: "
                                 + forbidden_chars);
            }
        }
    }
}

/**
 * Report the first inconsistency
 */
CString Graph::reportWeaklyConsistent() const {

    // test the individual scenarios for consistency
    const Scenarios &ss = this->getConstScenarios();
    for (const auto &si : ss) {
        if (!(si->getScenarioGraph())->isWeaklyConsistent(*si)) {
            return "Scenario " + si->getName() + " is not consistent.";
        }
    }

    // test consistency of the fsm transitions
    FSM *fsm = this->getFSM();
    FSMstates &st = fsm->getStates();
    for (const auto &s : st) {
        const FSMtransitionRefs &tr = s->getTransitions();
        for (const auto &t : tr) {
            FSMstate *sts = t->getSrcState();
            FSMstate *std = t->getDstState();
            ScenarioRef scs = sts->getScenario();
            ScenarioRef scd = std->getScenario();
            CStrings ptd = FSMSADF::Graph::getPersistentTokensOfScenario(*scd);
            CStrings pts = FSMSADF::Graph::getPersistentTokensFinalOfScenario(*scs);
            std::stringstream sRep;
            sRep << "The transistion from scenario " << scs->getName() << " to scenario "
                 << scd->getName() << " is not consistent. " << std::endl;
            sRep << "Scenario " << scs->getName() << " ends with the tokens: ";
            sRep << CString::join(pts, ',') << std::endl;
            sRep << "Scenario " << scd->getName() << " starts with the tokens: ";
            sRep << CString::join(ptd, ',');
            if (ptd != pts) {
                return sRep.str();
            }
        }
    }

    return "";
}

bool Graph::hasUnitRewards() const {
    const Scenarios &s = this->getConstScenarios();
    return std::all_of(s.begin(), s.end(), [](const std::unique_ptr<Scenario> &sc) {
        return sc->getReward() == 1.0;
    });
}

/**
 * The function returns a list with the names of all persistent tokens that
 * are used in the graph.
 * @return list with names of persistent tokens
 */
CStrings Graph::getPersistentTokens() const {
    CStrings pt;

    // Graph should have at least one scenario graph
    if (scenarioGraphs.empty()) {
        throw CException("No scenario graphs found.");
    }

    // Create list of persistent tokens from persistent token map of first graph
    PersistentTokenMap mp = scenarioGraphs.front()->getPersistentTokenMap();
    for (auto &[tokenName, num] : mp) {
        pt.push_back(tokenName);
    }

    // Check that all scenario graphs define the same set of persistent tokens
    for (const auto &sg : scenarioGraphs) {
        PersistentTokenMap mpSg = sg->getPersistentTokenMap();

        // Same size
        if (mpSg.size() != pt.size()) {
            throw CException(
                    "Persistent initial tokens size does not match in all scenarios, mpSg= '"
                    + CString(mpSg.size()) + "', pt='" + CString(pt.size()) + "'.");
        }

        // All names exist
        for (const auto &j : pt) {
            if (mpSg.find(j) == mpSg.end()) {
                throw CException("Persistent initial tokens do not match in all scenarios.");
            }
        }
    }

    return pt;
}

/**
 * The function returns a list with the names of all persistent tokens that
 * are used in the given scenario.
 * @return list with names of persistent tokens
 */
CStrings Graph::getPersistentTokensOfScenario(const Scenario &s) {
    ScenarioGraph *sg = s.getScenarioGraph();
    return sg->getPersistentTokens();
}

/**
 * The function returns a list with the names of all persistent final tokens that
 * are used in the given scenario.
 * @return list with names of persistent tokens
 */
CStrings Graph::getPersistentTokensFinalOfScenario(const Scenario &s) {
    ScenarioGraph *sg = s.getScenarioGraph();

    CStrings pt;

    // Create list of persistent tokens from persistent token map of first graph
    PersistentTokenMap mp = sg->getPersistentTokenMapFinal();
    for (auto &[tokenName, num] : mp) {
        pt.push_back(tokenName);
    }

    return pt;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
std::ostream &Graph::print(std::ostream &out) {
    out << "Graph (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << std::endl;

    for (const auto &i : scenarios) {
        i->print(out);
    }

    for (const auto &i : scenarioGraphs) {
        i->print(out);
    }

    if (fsm != nullptr) {
        fsm->print(out);
    }

    out << std::endl;

    return out;
}

/**
 * constructFromSDFG ()
 * Create from an SDFG.
 */
Graph::Graph(const SDF::TimedSDFgraph &g) :
    GraphComponent(nullptr, 0),
    defaultScenario(GraphComponent(this, 0, "_default_")),
    fsm(nullptr),
    unitsProperty(nullptr) {

    // Properties
    setName(g.getName());
    setThroughputConstraint(static_cast<FSMSADF::Throughput>(g.getThroughputConstraint()));

    // Create a scenario for the SDF behavior
    unsigned int cID = 0;
    auto s = std::make_unique<Scenario>(GraphComponent(this, cID++));
    s->setName("s");
    s->setReward(1.0);

    auto sg = std::make_unique<ScenarioGraph>(GraphComponent(this, cID++));

    RepetitionVector sdf_rv = computeRepetitionVector(g);
    FSMSADF::RepetitionVector rv;
    rv.resize(sdf_rv.size());

    // add actors
    unsigned int aCount = 0;
    auto i = g.actorsBegin();
    while (i != g.actorsEnd()) {
        auto *sdf_a = dynamic_cast<TimedSDFactor *>((*i).get());
        auto a = std::make_unique<Actor>(GraphComponent(sg.get(), aCount));
        a->setName(sdf_a->getName());
        a->setDefaultProcessorType("proc");
        a->setExecutionTimeOfScenario(*s, "proc", static_cast<Time>(sdf_a->getExecutionTime()));
        // add ports
        SDF::SDFportsCIter k = sdf_a->portsBegin();
        unsigned int pCount = 0;
        while (k != sdf_a->portsEnd()) {
            SDFport *sdf_p = k->get();
            std::unique_ptr<Port> p = std::make_unique<Port>(GraphComponent(a.get(), pCount++));
            p->setName(sdf_p->getName());
            p->setRateOfScenario(*s, sdf_p->getRate());
            p->setType(sdf_p->getTypeAsString());
            a->addPort(std::move(p));
            k++;
        }
        sg->addActor(std::move(a));

        rv[aCount] = sdf_rv[aCount];
        aCount++;
        i++;
    }

    sg->setPartialRepetitionVector(s.get(), rv);

    // add channels
    auto j = g.channelsBegin();
    while (j != g.channelsEnd()) {
        SDF::SDFchannel *sdf_c = (*j).get();
        auto c = std::make_unique<Channel>(GraphComponent(sg.get(), aCount++));
        ActorRef src_a = sg->getActor(sdf_c->getSrcActor()->getName());
        PortRef src_p = src_a->getPort(sdf_c->getSrcPort()->getName());
        c->connectSrc(src_p);
        ActorRef dst_a = sg->getActor(sdf_c->getDstActor()->getName());
        PortRef dst_p = dst_a->getPort(sdf_c->getDstPort()->getName());
        c->connectDst(dst_p);
        c->setInitialAndFinalTokens(sdf_c->getInitialTokens());
        for (unsigned int m = 0; m < sdf_c->getInitialTokens(); m++) {
            c->addPersistentTokenName(sdf_c->getName() + CString(m));
        }
        c->setTokenSizeOfScenario(*s, 1);
        sg->addChannel(std::move(c));
        j++;
    }

    auto *sgr = sg.get();
    addScenarioGraph(std::move(sg));
    sg->setPartialRepetitionVector(s.get(), rv);

    s->setScenarioGraph(sgr);

    addScenario(std::move(s));

    std::unique_ptr<FSM> fsm = std::make_unique<FSM>(GraphComponent(this, cID++));
    auto state = std::make_unique<FSMstate>(GraphComponent(fsm.get(), 0));
    auto transition = std::make_unique<FSMtransition>(GraphComponent(fsm.get(), 1));
    transition->connect(state.get(), state.get());
    state->setName("s0");
    state->setScenario(s.get());
    auto *state_r = state.get();
    fsm->addState(std::move(state));
    fsm->setInitialState(state_r);
    setFSM(std::move(fsm));
}

} // End namespace FSMSADF
