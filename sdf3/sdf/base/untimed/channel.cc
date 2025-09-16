/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF channel
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: channel.cc,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#include "channel.h"
#include "base/exception/exception.h"
#include "graph.h"
#include <memory>
namespace SDF {

/**
 * SDFchannel ()
 * Constructor.
 */
SDFchannel::SDFchannel(SDFcomponent &c) :
    SDFcomponent(c), src(nullptr), dst(nullptr), initialTokens(0) {}

/**
 * SDFinputchannel ()
 * Constructor.
 */
SDFinputchannel::SDFinputchannel(SDFcomponent &c) :
    SDFcomponent(c), src(nullptr), dst(nullptr), initialTokens(0) {}

/**
 * SDFoutputchannel ()
 * Constructor.
 */
SDFoutputchannel::SDFoutputchannel(SDFcomponent &c) :
    SDFcomponent(c), src(nullptr), dst(nullptr), initialTokens(0) {}

/**
 * ~SDFchannel ()
 * Destructor.
 */
SDFchannel::~SDFchannel() = default;

/**
 * ~SDFinputchannel ()
 * Destructor.
 */
SDFinputchannel::~SDFinputchannel() = default;

/**
 * ~SDFoutputchannel ()
 * Destructor.
 */
SDFoutputchannel::~SDFoutputchannel() = default;

/**
 * getGraph ()
 * Get a pointer to the SDFgraph of which the input channel is a part.
 */

 SDFgraph *SDFinputchannel::getGraph() const {
    return dynamic_cast<SDFgraph *>(getParent());
}

/**
 * getGraph ()
 * Get a pointer to the SDFgraph of which the output channel is a part.
 */

 SDFgraph *SDFoutputchannel::getGraph() const {
    return dynamic_cast<SDFgraph *>(getParent());
}

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF channel object.
 */
std::unique_ptr<SDFchannel> SDFchannel::create(SDFcomponent &c) const { return std::make_unique<SDFchannel>(c); }

/**
 * getGraph ()
 * Get a pointer to the SDF graph of which this channel is a part.
 */
SDFgraph *SDFchannel::getGraph() const {
    return dynamic_cast<SDFgraph *>(getParent());
}


/**
 * create ()
 * The function returns a pointer to a newly allocated SDF input channel object.
 */
std::unique_ptr<SDFinputchannel> SDFinputchannel::create(SDFcomponent &c) const { return std::make_unique<SDFinputchannel>(c); }

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF output channel object.
 */
std::unique_ptr<SDFoutputchannel> SDFoutputchannel::create(SDFcomponent &c) const {
    return std::make_unique<SDFoutputchannel>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * All properties and the connection of the channel are cloned.
 */
std::unique_ptr<SDFchannel> SDFchannel::clone(SDFcomponent &c) const {
    auto ch = std::make_unique<SDFchannel>(c);

    // Properties
    ch->setName(getName());

    // Connection
    if (getSrcActor() != nullptr) {
        SDFport *p = ch->getGraph()
                             ->getActor(getSrcActor()->getName())
                             ->getPort(getSrcPort()->getName());
        ch->connectSrc(p);
    }
    if (getDstActor() != nullptr) {
        SDFport *p = ch->getGraph()
                             ->getActor(getDstActor()->getName())
                             ->getPort(getDstPort()->getName());
        ch->connectDst(p);
    }

    // Initial tokens
    ch->setInitialTokens(getInitialTokens());

    return ch;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF input channel object.
 * All properties and the connection of the input channel are cloned.
 */
std::unique_ptr<SDFinputchannel> SDFinputchannel::clone(SDFcomponent &c) const {
    auto ch = std::make_unique<SDFinputchannel>(c);

    // Properties
    ch->setName(getName());

    // Connection
    SDFport *p = ch->getGraph()->getPort(getSrcPort()->getName());
    ch->connectSrc(p);

    if (getDstActor() != nullptr) {
        SDFport *p = ch->getGraph()
                             ->getActor(getDstActor()->getName())
                             ->getPort(getDstPort()->getName());
        ch->connectDst(p);
    }
    // Initial tokens
    ch->setInitialTokens(getInitialTokens());

    return ch;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF output channel object.
 * All properties and the connection of the input channel are cloned.
 */
std::unique_ptr<SDFoutputchannel> SDFoutputchannel::clone(SDFcomponent &c) const {
    auto ch = std::make_unique<SDFoutputchannel>(c);

    // Properties
    ch->setName(getName());

    if (getSrcActor() != nullptr) {
        SDFport *p = ch->getGraph()
                             ->getActor(getSrcActor()->getName())
                             ->getPort(getSrcPort()->getName());
        ch->connectSrc(p);
    }

    SDFport *p = ch->getGraph()->getPort(getDstPort()->getName());
    ch->connectDst(p);

    // Initial tokens
    ch->setInitialTokens(getInitialTokens());

    return ch;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * All properties of the channel are copied.
 */
std::unique_ptr<SDFchannel> SDFchannel::createCopy(SDFcomponent &c) const {
    auto ch = std::make_unique<SDFchannel>(c);

    // Properties
    ch->setName(getName());

    return ch;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF input channel object.
 * All properties of the input channel are copied.
 */
std::unique_ptr<SDFinputchannel> SDFinputchannel::createCopy(SDFcomponent &c) const {
    auto ch = std::make_unique<SDFinputchannel>(c);

    // Properties
    ch->setName(getName());

    return ch;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF output channel object.
 * All properties of the output channel are copied.
 */
std::unique_ptr<SDFoutputchannel> SDFoutputchannel::createCopy(SDFcomponent &c) const {
    auto ch = std::make_unique<SDFoutputchannel>(c);

    // Properties
    ch->setName(getName());

    return ch;
}
/**
 * construct ()
 * The function initializes all channel properties based on the XML data.
 */
void SDFchannel::construct(const CNodePtr channelNode) {
    CString strSrcActor;
    CString strSrcPort;
    CString strDstActor;
    CString strDstPort;
    SDFport *srcPort = nullptr;
    SDFport *dstPort = nullptr;
    SDFgraph *g = nullptr;
    SDFchannel *c = this;

    // Graph
    g = c->getGraph();

    // Name
    if (!CHasAttribute(channelNode, "name")) {
        throw CException("Invalid SDF graph, missing channel name.");
}
    c->setName(CGetAttribute(channelNode, "name"));

    // Initial tokens
    if (CHasAttribute(channelNode, "initialTokens")) {
        c->setInitialTokens(static_cast<uint>(CGetAttribute(channelNode, "initialTokens")));
}

    // Source and destination actor ports
    if (!CHasAttribute(channelNode, "srcActor")) {
        throw CException("Invalid SDF graph, missing channel srcActor.");
}
    strSrcActor = CGetAttribute(channelNode, "srcActor");

    if (!CHasAttribute(channelNode, "srcPort")) {
        throw CException("Invalid SDF graph, missing channel srcPort.");
}
    strSrcPort = CGetAttribute(channelNode, "srcPort");

    if (!CHasAttribute(channelNode, "dstActor")) {
        throw CException("Invalid SDF graph, missing channel dstActor.");
}
    strDstActor = CGetAttribute(channelNode, "dstActor");

    if (!CHasAttribute(channelNode, "dstPort")) {
        throw CException("Invalid SDF graph, missing channel dstPort.");
}
    strDstPort = CGetAttribute(channelNode, "dstPort");

    // Find actors and port
    srcPort = g->getActor(strSrcActor)->getPort(strSrcPort);
    dstPort = g->getActor(strDstActor)->getPort(strDstPort);

    // Connect channel to source and destination ports
    c->connectSrc(srcPort);
    c->connectDst(dstPort);
}

/**
 * construct ()
 * The function initializes all input channel properties based on the XML data.
 */
void SDFinputchannel::construct(const CNodePtr channelNode) {
    CString strSrcInput;
    CString strSrcPort;
    CString strDstActor;
    CString strDstPort;
    SDFport *srcPort = nullptr;
    SDFport *dstPort = nullptr;
    SDFgraph *g = nullptr;
    SDFinputchannel *c = this;

    // Graph
    g = c->getGraph();

    // Name
    if (!CHasAttribute(channelNode, "name")) {
        throw CException("Invalid SDF graph, missing input channel name.");
    }
    c->setName(CGetAttribute(channelNode, "name"));

    // Check for initial tokens
    if (CHasAttribute(channelNode, "initialTokens")) {
        throw CException("Input channel '" + c->getName() + "' cannot have an initial token.");
    }

    // Source and destination ports
    if (!CHasAttribute(channelNode, "srcInput")) {
        throw CException("Invalid SDF graph, missing input channel srcInput.");
    }
    strSrcInput = CGetAttribute(channelNode, "srcInput");

    if (!CHasAttribute(channelNode, "srcPort")) {
        throw CException("Invalid SDF graph, missing input channel srcPort.");
    }
    strSrcPort = CGetAttribute(channelNode, "srcPort");

    if (!CHasAttribute(channelNode, "dstActor")) {
        throw CException("Invalid SDF graph, missing input channel dstActor.");
    }
    strDstActor = CGetAttribute(channelNode, "dstActor");

    if (!CHasAttribute(channelNode, "dstPort")) {
        throw CException("Invalid SDF graph, missing input channel dstPort.");
    }
    strDstPort = CGetAttribute(channelNode, "dstPort");

    // Find actors and port
    srcPort = g->getInput(strSrcInput)->getPort(strSrcPort);
    dstPort = g->getActor(strDstActor)->getPort(strDstPort);

    // Connect channel to source and destination ports
    c->connectSrc(srcPort);
    c->connectDst(dstPort);
}

/**
 * construct ()
 * The function initializes all output channel properties based on the XML data.
 */
void SDFoutputchannel::construct(const CNodePtr channelNode) {
    CString strSrcActor;
    CString strSrcPort;
    CString strDstOutput;
    CString strDstPort;
    SDFport *srcPort = nullptr;
    SDFport *dstPort = nullptr;
    SDFgraph *g = nullptr;
    SDFoutputchannel *c = this;

    // Graph
    g = c->getGraph();

    // Name
    if (!CHasAttribute(channelNode, "name")) {
        throw CException("Invalid SDF graph, missing output channel name.");
    }
    c->setName(CGetAttribute(channelNode, "name"));

    // Check for initial tokens
    if (CHasAttribute(channelNode, "initialTokens")) {
        throw CException("Output channel '" + c->getName() + "' cannot have an initial token.");
    }

    // Source and destination actor ports
    if (!CHasAttribute(channelNode, "srcActor")) {
        throw CException("Invalid SDF graph, missing output channel srcActor.");
    }
    strSrcActor = CGetAttribute(channelNode, "srcActor");

    if (!CHasAttribute(channelNode, "srcPort")) {
        throw CException("Invalid SDF graph, missing output channel srcPort.");
    }
    strSrcPort = CGetAttribute(channelNode, "srcPort");

    if (!CHasAttribute(channelNode, "dstOutput")) {
        throw CException("Invalid SDF graph, missing output channel dstOutput.");
    }
    strDstOutput = CGetAttribute(channelNode, "dstOutput");

    if (!CHasAttribute(channelNode, "dstPort")) {
        throw CException("Invalid SDF graph, missing output channel dstPort.");
    }
    strDstPort = CGetAttribute(channelNode, "dstPort");

    // Find actors and port
    srcPort = g->getActor(strSrcActor)->getPort(strSrcPort);
    dstPort = g->getOutput(strDstOutput)->getPort(strDstPort);

    // Connect channel to source and destination ports
    c->connectSrc(srcPort);
    c->connectDst(dstPort);
}

/**
 * connectSrc ()
 * The function connects the channel to a source port and port to channel.
 */
void SDFchannel::connectSrc(SDFport *p) {
    if (p == nullptr) {
        src->connectToChannel(nullptr);
        src = nullptr;
        return;
    }

    if (getSrcPort() != nullptr) {
        throw CException("Channel '" + getName() + "' already connected.");
}

    if (p->getType() != SDFport::SDFportType::Out) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "channel '"
                         + getName() + "'; conflicting port type.");
    }

    src = p;
    src->connectToChannel(this);
}

/**
 * connectSrc ()
 * The function connects the input channel to a source port and the port to the input channel.
 */
void SDFinputchannel::connectSrc(SDFport *p) {
    if (p == nullptr) {
        src->connectToInputChannel(nullptr);
        src = nullptr;
        return;
    }

    if (getSrcPort() != nullptr) {
        throw CException("Input channel '" + getName() + "' already connected.");
    }

    if (p->getType() != SDFport::SDFportType::Out) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "input channel '"
                         + getName() + "'; conflicting port type.");
    }

    src = p;
    src->connectToInputChannel(this);
}

/**
 * connectSrc ()
 * The function connects the output channel to a source port and the port to the output channel.
 */
void SDFoutputchannel::connectSrc(SDFport *p) {
    if (p == nullptr) {
        src->connectToOutputChannel(nullptr);
        src = nullptr;
        return;
    }

    if (getSrcPort() != nullptr) {
        throw CException("Output channel '" + getName() + "' already connected.");
}

    if (p->getType() != SDFport::SDFportType::Out) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "output channel '"
                         + getName() + "'; conflicting port type.");
    }

    src = p;
    src->connectToOutputChannel(this);
}

/**
 * connectDst ()
 * The function connects the channel to a destination port and port to channel.
 */
void SDFchannel::connectDst(SDFport *p) {
    if (p == nullptr) {
        dst->connectToChannel(nullptr);
        dst = nullptr;
        return;
    }

    if (getDstPort() != nullptr) {
        throw CException("Channel '" + getName() + "' already connected.");
}

    if (p->getType() != SDFport::SDFportType::In) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "channel '"
                         + getName() + "'; conflicting port type.");
    }

    dst = p;
    dst->connectToChannel(this);
}

/**
 * connectDst ()
 * The function connects the channel to a destination port and the port to the input channel.
 */
void SDFinputchannel::connectDst(SDFport *p) {
    if (p == nullptr) {
        dst->connectToInputChannel(nullptr);
        dst = nullptr;
        return;
    }

    if (getDstPort() != nullptr) {
        throw CException("Input channel '" + getName() + "' already connected.");
}

    if (p->getType() != SDFport::SDFportType::In) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "input channel '"
                         + getName() + "'; conflicting port type.");
    }

    dst = p;
    dst->connectToInputChannel(this);
}

/**
 * connectDst ()
 * The function connects the output channel to a destination port and the port to the output
 * channel.
 */
void SDFoutputchannel::connectDst(SDFport *p) {
    if (p == nullptr) {
        dst->connectToOutputChannel(nullptr);
        dst = nullptr;
        return;
    }

    if (getDstPort() != nullptr) {
        throw CException("Channel '" + getName() + "' already connected.");
}

    if (p->getType() != SDFport::SDFportType::In) {
        std::cerr << "type: " << p->getTypeAsString() << std::endl;
        throw CException("Cannot connect port '" + p->getName()
                         + "' to "
                           "channel '"
                         + getName() + "'; conflicting port type.");
    }

    dst = p;
    dst->connectToOutputChannel(this);
}

/**
 * isConnected ()
 * The function returns true if the channel is connected to a source and
 * destination port.
 */
bool SDFchannel::isConnected() const {
    return getSrcPort() != nullptr && getDstPort() != nullptr;
}

/**
 * isConnected ()
 * The function returns true if the input channel is connected to a source and
 * destination port.
 */
bool SDFinputchannel::isConnected() const {
    return getSrcPort() != nullptr && getDstPort() != nullptr;
}

/**
 * isConnected ()
 * The function returns true if the output channel is connected to a source and
 * destination port.
 */
bool SDFoutputchannel::isConnected() const {
    return getSrcPort() != nullptr && getDstPort() != nullptr;
}

/**
 * print ()
 * Print the channel to the supplied output stream.
 */
std::ostream &SDFchannel::print(std::ostream &out) {
    out << "Channel (" << getName() << ")" << std::endl;
    out << "id:             " << getId() << std::endl;
    out << "initial tokens: " << getInitialTokens() << std::endl;
    out << "connected:      " << (isConnected() ? "true" : "false") << std::endl;

    if (isConnected()) {
        out << "source:         " << getSrcActor()->getName() << ".";
        out << getSrcPort()->getName() << std::endl;
        out << "destination:    " << getDstActor()->getName() << ".";
        out << getDstPort()->getName() << std::endl;
    }

    out << std::endl;

    return out;
}

/**
 * print ()
 * Print the input channel to the supplied output stream.
 */
std::ostream &SDFinputchannel::print(std::ostream &out) {
    out << "Input channel (" << getName() << ")" << std::endl;
    out << "id:             " << getId() << std::endl;
    out << "initial tokens: " << getInitialTokens() << std::endl;
    out << "connected:      " << (isConnected() ? "true" : "false") << std::endl;

    if (isConnected()) {
        out << "source:         " << getSrcPort()->getName() << std::endl;
        out << "destination:    " << getDstActor()->getName() << ".";
        out << getDstPort()->getName() << std::endl;
    }

    out << std::endl;

    return out;
}

/**
 * print ()
 * Print the output channel to the supplied output stream.
 */
std::ostream &SDFoutputchannel::print(std::ostream &out) {
    out << "Input channel (" << getName() << ")" << std::endl;
    out << "id:             " << getId() << std::endl;
    out << "initial tokens: " << getInitialTokens() << std::endl;
    out << "connected:      " << (isConnected() ? "true" : "false") << std::endl;

    if (isConnected()) {
        out << "source:         " << getSrcActor()->getName() << ".";
        out << getSrcPort()->getName() << std::endl;
        out << "destination:    " << getDstPort()->getName() << std::endl;
    }

    out << std::endl;

    return out;
}

} // namespace SDF
