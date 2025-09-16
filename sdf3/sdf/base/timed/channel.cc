/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed SDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
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
#include <memory>
namespace SDF {

/**
 * TimedSDFchannel ()
 * Constructor.
 */
TimedSDFchannel::TimedSDFchannel(SDFcomponent &c) :
    SDFchannel(c),
    isCapacityChannel(false),
    modelStorageSpaceCh(nullptr),
    minBandwidth(0),
    tokenSize(SDF_INFINITE_SIZE),
    minLatency(0),
    // Unbounded
    bufferSize({SDF_INFINITE_SIZE, SDF_INFINITE_SIZE, SDF_INFINITE_SIZE, SDF_INFINITE_SIZE}) {

    // No delay
}

/**
 * ~TimedSDFchannel ()
 * Destructor.
 */
TimedSDFchannel::~TimedSDFchannel() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF channel object.
 */
std::unique_ptr<SDFchannel> TimedSDFchannel::create(SDFcomponent &c) const {
    return std::make_unique<TimedSDFchannel>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * The properties of the object are cloned.
 */
std::unique_ptr<SDFchannel> TimedSDFchannel::clone(SDFcomponent &c) const {
    std::unique_ptr<TimedSDFchannel> ch = TimedSDFchannel::dynamicCast(createCopy(c));

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
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * The properties of the object are copied.
 */
std::unique_ptr<SDFchannel> TimedSDFchannel::createCopy(SDFcomponent &c) const {
    auto ch = std::make_unique<TimedSDFchannel>(c);

    // Properties
    ch->setName(getName());
    ch->setBufferSize(getBufferSize());
    ch->setTokenSize(getTokenSize());
    ch->setTokenType(getTokenType());
    ch->setMinBandwidth(getMinBandwidth());
    ch->setStorageSpaceChannel(getStorageSpaceChannel());
    ch->setMinLatency(getMinLatency());

    return ch;
}

/**
 * dynamicCast ()
 * Dynamic cast of untimed base type to timed derived type.
 */
std::unique_ptr<TimedSDFchannel> TimedSDFchannel::dynamicCast(std::unique_ptr<SDFchannel> c) {
    return std::unique_ptr<TimedSDFchannel>(dynamic_cast<TimedSDFchannel*>(c.release()));
}

/**
 * setProperties ()
 * Set the properties of a timed channel.
 */
void TimedSDFchannel::setProperties(const CNodePtr propertiesNode) {
    // Buffer size
    if (CHasChildNode(propertiesNode, "bufferSize")) {
        CNode *bufferSizeNode = CGetChildNode(propertiesNode, "bufferSize");
        BufferSize bufferSize;

        bufferSize.sz = SDF_INFINITE_SIZE;
        bufferSize.src = SDF_INFINITE_SIZE;
        bufferSize.dst = SDF_INFINITE_SIZE;
        bufferSize.mem = SDF_INFINITE_SIZE;

        if (CHasAttribute(bufferSizeNode, "sz")) {
            bufferSize.sz = static_cast<int>(CGetAttribute(bufferSizeNode, "sz"));
        }

        if (CHasAttribute(bufferSizeNode, "src")) {
            bufferSize.src = static_cast<int>(CGetAttribute(bufferSizeNode, "src"));
        }

        if (CHasAttribute(bufferSizeNode, "dst")) {
            bufferSize.dst = static_cast<int>(CGetAttribute(bufferSizeNode, "dst"));
        }

        if (CHasAttribute(bufferSizeNode, "mem")) {
            bufferSize.mem = static_cast<int>(CGetAttribute(bufferSizeNode, "mem"));
        }

        setBufferSize(bufferSize);
    }

    // Token size
    if (CHasChildNode(propertiesNode, "tokenSize")) {
        CNode *tokenSizeNode = CGetChildNode(propertiesNode, "tokenSize");

        if (CHasAttribute(tokenSizeNode, "sz")) {
            int sz = static_cast<int>(CGetAttribute(tokenSizeNode, "sz"));
            setTokenSize(sz);
        }
    }

    // Token type
    if (CHasChildNode(propertiesNode, "tokenType")) {
        CNode *tokenTypeNode = CGetChildNode(propertiesNode, "tokenType");

        if (CHasAttribute(tokenTypeNode, "type")) {
            CString type = CGetAttribute(tokenTypeNode, "type");
            setTokenType(type);
        }
    }

    // Bandwidth
    if (CHasChildNode(propertiesNode, "bandwidth")) {
        CNode *bwNode = CGetChildNode(propertiesNode, "bandwidth");

        if (CHasAttribute(bwNode, "min")) {
            double sz = static_cast<double>(CGetAttribute(bwNode, "min"));
            setMinBandwidth(sz);
        }
    }

    // Latency
    if (CHasChildNode(propertiesNode, "latency")) {
        CNode *latencyNode = CGetChildNode(propertiesNode, "latency");

        if (CHasAttribute(latencyNode, "min")) {
            int sz = static_cast<int>(CGetAttribute(latencyNode, "min"));
            setMinLatency(sz);
        }
    }
}

/**
 * isUnbounded ()
 * The function returns true if the channel is unbounded. If the size is bounded
 * it returns false.
 */
bool TimedSDFchannel::isUnbounded() const { return getBufferSize().sz == SDF_INFINITE_SIZE; }

/**
 * isControlToken ()
 * The function returns true if the channel is contains control tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedSDFchannel::isControlToken() const { return getTokenSize() == 0; }

/**
 * isDataToken ()
 * The function returns true if the channel is contains data tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedSDFchannel::isDataToken() const { return getTokenSize() > 0; }

/**
 * print ()
 * Print the channel to the supplied output stream.
 */
std::ostream &TimedSDFchannel::print(std::ostream &out) {
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

    TimedSDFchannel::BufferSize bufferSize = getBufferSize();
    if (bufferSize.sz != SDF_INFINITE_SIZE) {
        out << "buffer size:    " << bufferSize.sz << std::endl;
    }

    if (getTokenSize() != SDF_INFINITE_SIZE) {
        out << "token size:     " << getTokenSize() << std::endl;
    }

    if (getMinLatency() != 0) {
        out << "min latency:    " << getMinLatency() << std::endl;
    }

    if (getMinBandwidth() != 0) {
        out << "min bandwidth:  " << getMinBandwidth() << std::endl;
    }

    if (!getTokenType().empty()) {
        out << "token type:     " << getTokenType() << std::endl;
    }

    out << std::endl;

    return out;
}
} // namespace SDF
