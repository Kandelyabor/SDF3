/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: graph.cc,v 1.1.1.1.2.1 2010-05-10 22:25:02 mgeilen Exp $
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
#include "base/exception/exception.h"
#include <algorithm>
#include <memory>
namespace SDF {

/**
 * SDFgraph ()
 * Constructor.
 */
SDFgraph::SDFgraph(SDFcomponent &c) : SDFcomponent(c) {}

/**
 * SDFgraph ()
 * Constructor.
 */
SDFgraph::SDFgraph()  = default;


/**
 * create ()
 * The function returns a pointer to a newly allocated SDF graph object.
 */
std::unique_ptr<SDFgraph> SDFgraph::create(SDFcomponent &c) const { return std::make_unique<SDFgraph>(c); }

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF graph object.
 * All properties, actors and channels of the graph are cloned.
 */
std::unique_ptr<SDFgraph> SDFgraph::clone(SDFcomponent &c) const {
    auto g = std::make_unique<SDFgraph>(c);

    // Properties
    g->setName(getName());
    g->setType(getType());

    // Actors
    for (const auto& iter: this->getActors()) {
        SDFcomponent component = SDFcomponent(g.get(), g->nrActors());
        std::unique_ptr<SDFactor> a = iter->clone(component);
        g->addActor(std::move(a));
    }

    // Channels
    for (const auto& iter: this->getChannels()) {
        SDFcomponent component = SDFcomponent(g.get(), g->nrChannels());
        std::unique_ptr<SDFchannel> ch = iter->clone(component);
        g->addChannel(std::move(ch));
    }

    // Input channels
    for (const auto& iter: this->getInputChannels()) {
        SDFcomponent component = SDFcomponent(g.get(), g->nrInputChannels());
        std::unique_ptr<SDFinputchannel> ch = iter->clone(component);
        g->addInputChannel(std::move(ch));
    }

    // Output channels
    for (const auto& iter: this->getOutputChannels()) {
        SDFcomponent component = SDFcomponent(g.get(), g->nrOutputChannels());
        std::unique_ptr<SDFoutputchannel> ch = iter->clone(component);
        g->addOutputChannel(std::move(ch));
    }

    // Unconnected ports
    for (const auto& iter: this->getPorts()) {
        SDFcomponent component = SDFcomponent(g.get(), g->nrPorts());
        std::unique_ptr<SDFport> p = iter->clone(component);
        g->addPort(std::
            move(p));
    }

    return g;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF graph object.
 * All properties of the graph are copied.
 */
std::unique_ptr<SDFgraph> SDFgraph::createCopy(SDFcomponent &c) const {
    auto g = std::make_unique<SDFgraph>(c);

    // Properties
    g->setName(getName());
    g->setType(getType());

    return g;
}

/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void SDFgraph::construct(const CNodePtr sdfNode) {
    SDFcomponent component;

    // Name
    if (!CHasAttribute(sdfNode, "name")) {
        throw CException("Invalid SDF graph, missing graph name.");
    }
    setName(CGetAttribute(sdfNode, "name"));

    // Type
    if (!CHasAttribute(sdfNode, "type")) {
        throw CException("Invalid SDF graph, missing graph type.");
    }
    setType(CGetAttribute(sdfNode, "type"));

    // Construct all unconnected ports
    for (CNodePtr portNode = CGetChildNode(sdfNode, "port"); portNode != nullptr;
         portNode = CNextNode(portNode, "port")) {
        // Construct port
        component = SDFcomponent(this, nrPorts());
        SDFport *p = createPort(component);

        if (!CHasAttribute(portNode, "name")) {
            throw CException("Invalid SDF graph, missing port name.");
        }
        p->setName(CGetAttribute(portNode, "name"));

        // Type
        if (!CHasAttribute(portNode, "type")) {
            throw CException("Invalid SDF graph, missing port type.");
        }
        p->setType(CGetAttribute(portNode, "type"));

        // Rate
        if (CHasAttribute(portNode, "rate")) {
            throw CException("Invalid SDF graph, external port must be without rate.");
        }
        p->setRate(1);
    }

    // Construct all actors
    for (CNodePtr actorNode = CGetChildNode(sdfNode, "actor"); actorNode != nullptr;
        actorNode = CNextNode(actorNode, "actor")) {
        // Construct actor
        component = SDFcomponent(this, nrActors());
        SDFactor *a = createActor(component);
        a->construct(actorNode);

        // Construct all ports
        for (CNodePtr portNode = CGetChildNode(actorNode, "port"); portNode != nullptr;
             portNode = CNextNode(portNode, "port")) {
            // Construct port
            component = SDFcomponent(a, a->nrPorts());
            auto p = std::make_unique<SDFport>(component);
            p->construct(portNode);

            // Add port to actor
            a->addPort(std::move(p));
        }
    }

    // Construct all inputs
    for (CNodePtr inputNode = CGetChildNode(sdfNode, "input"); inputNode != nullptr;
        inputNode = CNextNode(inputNode, "input")) {
        component = SDFcomponent(this, nrInputs());
        SDFinput *i = createInput(component);
        i->construct(inputNode);

        // Construct port
        if (CGetNumberOfChildNodes(inputNode, "port") != 1) {
            throw CException("Input '" + i->getName() + "' must have exactly 1 port. '");
        }
        CNodePtr portNode = CGetChildNode(inputNode, "port");
        component = SDFcomponent(i, 0);
        auto p = std::make_unique<SDFport>(component);
        p->construct(portNode);
        i->setPort(std::move(p));
    }

    // Construct all outputs
    for (CNodePtr outputNode = CGetChildNode(sdfNode, "output"); outputNode != nullptr;
        outputNode = CNextNode(outputNode, "output")) {
        component = SDFcomponent(this, nrOutputs());
        SDFoutput *o = createOutput(component);
        o->construct(outputNode);

        // Construct port
        if (CGetNumberOfChildNodes(outputNode, "port") != 1) {
            throw CException("Output '" + o->getName() + "' must have exactly 1 port. '");
        }
        CNodePtr portNode = CGetChildNode(outputNode, "port");
        component = SDFcomponent(o, 0);
        auto p = std::make_unique<SDFport>(component);
        p->construct(portNode);
        o->setPort(std::move(p));
    }

    // Construct all channels
    for (CNodePtr channelNode = CGetChildNode(sdfNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {

        // Construct channel
        if (CHasAttribute(channelNode, "srcActor") && CHasAttribute(channelNode, "dstActor")) {
            component = SDFcomponent(this, nrChannels());
            SDFchannel *c = createChannel(component);
            c->construct(channelNode);
        } else if (CHasAttribute(channelNode, "srcInput") && CHasAttribute(channelNode, "dstActor")) {
            component = SDFcomponent(this, nrInputChannels());
            SDFinputchannel *ic = createInputChannel(component);
            ic->construct(channelNode);
        } else if (CHasAttribute(channelNode, "srcActor") && CHasAttribute(channelNode, "dstOutput")) {
            component = SDFcomponent(this, nrOutputChannels());
            SDFoutputchannel *oc = createOutputChannel(component);
            oc->construct(channelNode);
        }
    }
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given id.
 */
SDFactorRef SDFgraph::getActor(const CId id) {
    for (const auto& a: this->getActors()) {
        if (a->getId() == id) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}

/**
 * getConstActor ()
 * The function returns a reference to an actor with the given id.
 */
SDFactorRef SDFgraph::getConstActor(const CId id) const {
    for (const auto& a: this->getActors()) {
        if (a->getId() == id) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}

/**
 * getActorPtr ()
 * The function returns a pointer reference to an actor with the given id.
 */
SDFactor* SDFgraph::getActorPtr(const CId& id) const {
    for (const auto& a: this->getActors()) {
        if (a->getId() == id) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}

/**
 * getConstActorPtr ()
 * The function returns a pointer reference to an actor with the given id.
 */
SDFactor* SDFgraph::getConstActorPtr(const CId &id) const {
    for (const auto& a: this->getActors()) {
        if (a->getId() == id) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}


/**
 * getActorPtr ()
 * The function returns a pointer reference to an actor with the given id.
 */
SDFactor* SDFgraph::getActorPtr(const CString& name) const {
    for (const auto& a: this->getActors()) {
        if (a->getName() == name) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with name '" + CString(name) + "'.");
}

/**
 * getConstActorPtr ()
 * The function returns a pointer reference to an actor with the given id.
 */
SDFactor* SDFgraph::getConstActorPtr(const CString& name) const {
    for (const auto& a: this->getActors()) {
        if (a->getName() == name) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with name '" + CString(name) + "'.");
}


/**
 * getActor ()
 * The function returns a reference to an actor with the given name.
 */
SDFactorRef SDFgraph::getActor(const CString &name) const {
    for (const auto& a: this->getActors()) {
        if (a->getName() == name) {
            return a.get();
}
    }

    return nullptr;
}

/**
 * getConstActor ()
 * The function returns a reference to an actor with the given name.
 */
SDFactorRef SDFgraph::getConstActor(const CString &name) const {
    for (const auto& a: this->getActors()) {
        if (a->getName() == name) {
            return a.get();
}
    }

    return nullptr;
}

/**
 * addActor ()
 * Add an actor to a graph.
 */
void SDFgraph::addActor(std::unique_ptr<SDFactor> a) {
    actors.push_back(std::move(a));
}

/**
 * removeActor ()
 * Remove an actor from a graph.
 */
void SDFgraph::removeActor(const CString &name) {
    for (auto iter = this->actorsBegin(); iter != this->actorsEnd(); iter++) {
        const std::unique_ptr<SDFactor>& a = *iter;

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
SDFactorRef SDFgraph::createActor() {
    SDFcomponent c = SDFcomponent(this, nrActors());
    auto a = std::make_unique<SDFactor>(c);
    a->setName(CString("_a") + CString(nrActors() + 1));
    auto* ar = a.get();
    addActor(std::move(a));

    return ar;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
SDFactor *SDFgraph::createActor(SDFcomponent &c) {
    auto a = std::make_unique<SDFactor>(c);
    auto* ar = a.get();
    addActor(std::move(a));

    return ar;
}

/**
 * getInput ()
 * The function returns a reference to an input with the given id.
 */
SDFinput *SDFgraph::getInput(const CId id) {
    for (const auto& i: this->getInputs()) {
        if (i->getId() == id) {
            return i.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no input with id '" + CString(id) + "'.");
}

/**
 * getInput ()
 * The function returns a reference to an input with the given name.
 */
SDFinput *SDFgraph::getInput(const CString &name) const {
    for (const auto& i: this->getInputs()) {
        if (i->getName() == name) {
            return i.get();
        }
    }

    return nullptr;
}

/**
 * addInput ()
 * Add an input to a graph.
 */
void SDFgraph::addInput(std::unique_ptr<SDFinput> i) { inputs.push_back(std::move(i)); }

/**
 * removeInput ()
 * Remove an input from a graph.
 */
void SDFgraph::removeInput(const CString &name) {
    for (auto iter = this->inputsBegin(); iter != this->inputsEnd(); iter++) {
        const SDFinput& i = *(*iter);

        if (i.getName() == name) {
            inputs.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no input '" + name + "'.");
}

/**
 * createInput ()
 * Create a new input on the graph.
 */
SDFinput *SDFgraph::createInput() {
    SDFcomponent c = SDFcomponent(this, nrInputs());
    auto i = std::make_unique<SDFinput>(c);
    i->setName(CString("_i") + CString(nrInputs() + 1));
    auto* ir = i.get();
    addInput(std::move(i));

    return ir;
}

/**
 * createInput ()
 * Create a new input on the graph.
 */
SDFinput *SDFgraph::createInput(SDFcomponent &c) {
    auto i = std::make_unique<SDFinput>(c);
    auto* ir = i.get();
    addInput(std::move(i));

    return ir;
}

/**
 * getOutput ()
 * The function returns a reference to an output with the given id.
 */
SDFoutput *SDFgraph::getOutput(const CId id) {
    for (const auto& o: this->getOutputs()) {
        if (o->getId() == id) {
            return o.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no output with id '" + CString(id) + "'.");
}

/**
 * getOutput ()
 * The function returns a reference to an output with the given name.
 */
SDFoutput *SDFgraph::getOutput(const CString &name) const {
    for (const auto& o: this->getOutputs()) {
        if (o->getName() == name) {
            return o.get();
        }
    }

    return nullptr;
}

/**
 * addOutput()
 * Add an output to a graph.
 */
void SDFgraph::addOutput(std::unique_ptr<SDFoutput> o) { outputs.push_back(std::move(o)); }

/**
 * removeOutput ()
 * Remove an output from a graph.
 */
void SDFgraph::removeOutput(const CString &name) {
    for (auto iter = this->outputsBegin(); iter != this->outputsEnd(); iter++) {
        SDFoutput& o = *(*iter);

        if (o.getName() == name) {
            outputs.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no output '" + name + "'.");
}

/**
 * createOutput ()
 * Create a new output on the graph.
 */
SDFoutput *SDFgraph::createOutput() {
    SDFcomponent c = SDFcomponent(this, nrOutputs());
    auto o = std::make_unique<SDFoutput>(c);
    o->setName(CString("_o") + CString(nrOutputs() + 1));
    auto* o_ptr = o.get();
    addOutput(std::move(o));

    return o_ptr;
}

/**
 * createOutput ()
 * Create a new output on the graph.
 */
SDFoutput *SDFgraph::createOutput(SDFcomponent &c) {
    auto o = std::make_unique<SDFoutput>(c);
    auto* o_ptr = o.get();
    addOutput(std::move(o));

    return o_ptr;
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given id.
 */
SDFchannelRef SDFgraph::getChannel(const CId& id) const {
    for (const auto& c: this->getChannels()) {
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
SDFchannelRef SDFgraph::getChannel(const CString &name) const {
    for (const auto& c: this->getChannels()) {
        if (c->getName() == name) {
            return c.get();
        }
    }

    return nullptr;
}


/**
 * addChannel ()
 * Add a channel to a graph.
 */
void SDFgraph::addChannel(std::unique_ptr<SDFchannel> c) {
    channels.push_back(std::move(c));
}

/**
 * removeChannel ()
 * Remove a channel from a graph.
 */
void SDFgraph::removeChannel(const CString &name) {
    for (auto iter = this->channelsBegin(); iter != this->channelsEnd(); iter++) {
        const std::unique_ptr<SDFchannel>& c = *iter;

        if (c->getName() == name) {
            channels.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no channel '" + name + "'.");
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
SDFchannelRef SDFgraph::createChannel(SDFcomponent &c) {
    auto ch = std::make_unique<SDFchannel>(c);
    auto* chr = ch.get();
    addChannel(std::move(ch));

    return chr;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
SDFchannel *SDFgraph::createChannel(
        SDFactor *src, SDFrate rateSrc, SDFactor *dst, SDFrate rateDst, uint initialTokens) {
    // Create new channel
    SDFcomponent c = SDFcomponent(this, nrChannels());
    SDFchannel *ch = createChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));

    // Create ports on the actors
    SDFport *srcP = src->createPort(SDFport::SDFportType::Out, rateSrc);
    srcP->setName(CString("_p") + CString(src->nrPorts()));
    SDFport *dstP = dst->createPort(SDFport::SDFportType::In, rateDst);
    dstP->setName(CString("_p") + CString(dst->nrPorts()));

    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    // Initial tokens
    ch->setInitialTokens(initialTokens);

    return ch;
}

/**
 * getInputChannel ()
 * The function returns a reference to an input channel with the given id.
 */
SDFinputchannelRef SDFgraph::getInputChannel(const CId id) {
    for (const auto& c: this->getInputChannels()) {
        if (c->getId() == id) {
            return c.get();
}
    }

    throw CException("Graph '" + getName() + "' has no input channel with id '" + CString(id)
                     + "'.");
}

/**
 * getInputChannel ()
 * The function returns a reference to an input channel with the given name.
 */
SDFinputchannelRef SDFgraph::getInputChannel(const CString &name) const {
    for (const auto& c: this->getInputChannels()) {
        if (c->getName() == name) {
            return c.get();
}
    }

    return nullptr;
}

/**
 * addInputChannel ()
 * Add an input channel to a graph.
 */
void SDFgraph::addInputChannel(std::unique_ptr<SDFinputchannel> c) { inputchannels.push_back(std::move(c)); }

/**
 * removeInputChannel ()
 * Remove an input channel from a graph.
 */
void SDFgraph::removeInputChannel(const CString &name) {
    for (auto iter = inputchannels.begin(); iter != inputchannels.end(); iter++) {
        const std::unique_ptr<SDFinputchannel>& c = *iter;

        if (c->getName() == name) {
            inputchannels.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no input channel '" + name + "'.");
}

/**
 * createInputChannel ()
 * Create a new  channel on the graph.
 */
SDFinputchannelRef SDFgraph::createInputChannel(SDFcomponent &c) {
    auto ch = std::make_unique<SDFinputchannel>(c);
    auto* chr = ch.get();
    addInputChannel(std::move(ch));

    return chr;
}

/**
 * createInputChannel ()
 * The function creates an input channel between a dummy port and destination actor.
 * Ports with the supplied rates are added.
 */
SDFinputchannel *SDFgraph::createInputChannel(SDFactor *dst, SDFrate rateDst, uint initialTokens) {
    // Create a new input channel
    SDFcomponent c = SDFcomponent(this, nrChannels());
    SDFinputchannel *ch = createInputChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));

    // Create a dummy source port
    SDFport *srcP = createPort(SDFport::SDFportType::Out, 1);
    srcP->setName(CString("_p") + CString(nrPorts()));
    // Create a port on the destination actor
    SDFport *dstP = dst->createPort(SDFport::SDFportType::In, rateDst);
    dstP->setName(CString("_p") + CString(dst->nrPorts()));

    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    // Initial tokens
    ch->setInitialTokens(initialTokens);

    return ch;
}

/**
 * getOutputChannel ()
 * The function returns a reference to an output channel with the given id.
 */
SDFoutputchannelRef SDFgraph::getOutputChannel(const CId id) {
    for (const auto& c: this->getOutputChannels()) {
        if (c->getId() == id) {
            return c.get();
}
    }

    throw CException("Graph '" + getName() + "' has no output channel with id '" + CString(id)
                     + "'.");
}

/**
 * getOutputChannel ()
 * The function returns a reference to an output channel with the given name.
 */
SDFoutputchannel *SDFgraph::getOutputChannel(const CString &name) const     {
    for (const auto& c: this->getOutputChannels()) {
        if (c->getName() == name) {
            return c.get();
}
    }

    return nullptr;
}

/**
 * addOutputChannel ()
 * Add an output channel to a graph.
 */
void SDFgraph::addOutputChannel(std::unique_ptr<SDFoutputchannel> c) { outputchannels.push_back(std::move(c)); }

/**
 * removeOutputChannel ()
 * Remove an input channel from a graph.
 */
void SDFgraph::removeOutputChannel(const CString &name) {
    for (auto iter = outputchannels.begin(); iter != outputchannels.end();
         iter++) {
        const std::unique_ptr<SDFoutputchannel>& c = *iter;

        if (c->getName() == name) {
            outputchannels.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no output channel '" + name + "'.");
}

/**
 * createInputChannel ()
 * Create a new  channel on the graph.
 */
SDFoutputchannelRef SDFgraph::createOutputChannel(SDFcomponent &c) {
    auto ch = std::make_unique<SDFoutputchannel>(c);
    auto* chr = ch.get();
    addOutputChannel(std::move(ch));
    return chr;
}

/**
 * createOutputChannel ()
 * The function creates an Input channel between a dummy port and destination actor.
 * Ports with the supplied rates are added.
 */
SDFoutputchannel *
SDFgraph::createOutputChannel(SDFactor *src, SDFrate rateSrc, uint initialTokens) {
    // Create new channel
    SDFcomponent c = SDFcomponent(this, nrChannels());
    SDFoutputchannel *ch = createOutputChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));

    // Create a port on the source actor
    SDFport *srcP = src->createPort(SDFport::SDFportType::Out, rateSrc);
    srcP->setName(CString("_p") + CString(src->nrPorts()));
    // Create a dummy destination port
    SDFport *dstP = createPort(SDFport::SDFportType::In, 1);
    dstP->setName(CString("_p") + CString(nrPorts()));

    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    // Initial tokens
    ch->setInitialTokens(initialTokens);

    return ch;
}
/**
 * createPort ()
 * Create a new port on the graph.
 */
SDFportRef SDFgraph::createPort(SDFcomponent &c) {
    auto p = std::make_unique<SDFport>(c);
    auto* pr = p.get();
    addPort(std::move(p));

    return pr;
}

/**
 * createPort ()
 * Create a new port on the graph.
 */
SDFport *SDFgraph::createPort(const SDFport::SDFportType type, const SDFrate rate) {
    SDFcomponent c = SDFcomponent(this, nrPorts());
    SDFport *p = createPort(c);

    p->setType(type);
    p->setRate(rate);

    return p;
}

/**
 * isConnected ()
 * The function returns true if all ports are connected to a channel, else
 * it returns false.
 */
bool SDFgraph::isConnected() const {
    return std::all_of(this->portsBegin(), this->portsEnd(), [](const auto& p){
        return p->isConnected();
    });
}

/**
 * getPort ()
 * The function returns a reference to a port with the given id.
 */
SDFportRef SDFgraph::getPort(const CId id) {
    for (const auto& p: this->getPorts()) {
        if (p->getId() == id) {
            return p.get();
}
    }

    throw CException("Graph '" + getName() + "' has no port with id '" + CString(id) + "'.");
}

/**
 * getPort ()
 * The function returns a reference to a port with the given name.
 */
SDFportRef SDFgraph::getPort(const CString &name) {
    for (const auto& p: this->getPorts()) {
        if (p->getName() == name) {
            return p.get();
}
    }

    throw CException("Graph '" + getName() + "' has no port '" + name + "'.");
}

/**
 * addPort ()
 * Add a port to an actor.
 */
void SDFgraph::addPort(std::unique_ptr<SDFport> p) {
    ports.push_back(std::move(p));
}

/**
 * removePort ()
 * Remove a port from an actor and destroy its memory space.
 */
void SDFgraph::removePort(const CString &name) {
    for (auto iter = ports.begin(); iter != ports.end(); iter++) {
        std::unique_ptr<SDFport>& p = *iter;

        if (p->getName() == name) {
            ports.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no port '" + name + "'.");
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
std::ostream &SDFgraph::print(std::ostream &out) {
    out << "Graph (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;
    out << std::endl;

    for (const auto& a: this->getActors()) {
        a->print(out);
    }

    for (const auto& c: this->getChannels()) {
        c->print(out);
    }

    for (const auto& p: this->getPorts()) {
        p->print(out);
    }

    out << std::endl;

    return out;
}
} // namespace SDF
