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
 *  Function        :   Timed CSDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: channel.cc,v 1.1.1.1 2007-10-02 10:59:49 sander Exp $
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
namespace CSDF {
/**
 * TimedCSDFchannel ()
 * Constructor.
 */
TimedCSDFchannel::TimedCSDFchannel(CSDFcomponent &c) :
    CSDFchannel(c),
    modelStorageSpaceCh(nullptr),
    bufferSize({-1, -1, -1, -1}),
    minBandwidth(0),
    tokenSize(-1),
    minLatency(0) {}

/**
 * ~TimedCSDFchannel ()
 * Destructor.
 */
TimedCSDFchannel::~TimedCSDFchannel() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF channel object.
 */
std::unique_ptr<CSDFchannel> TimedCSDFchannel::create(CSDFcomponent &c) const {
    return std::make_unique<TimedCSDFchannel>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF channel object.
 * The properties of the object are cloned.
 */
std::unique_ptr<CSDFchannel> TimedCSDFchannel::clone(CSDFcomponent &c) const {
    std::unique_ptr<CSDFchannel> ch = createCopy(c);

    // Connection
    if (getSrcActor() != nullptr) {
        CSDFport *p = ch->getGraph()
                              ->getActor(getSrcActor()->getName())
                              ->getPort(getSrcPort()->getName());
        ch->connectSrc(p);
    }
    if (getDstActor() != nullptr) {
        CSDFport *p = ch->getGraph()
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
 * The function returns a pointer to a newly allocated CSDF channel object.
 * The properties of the object are copied.
 */
std::unique_ptr<CSDFchannel> TimedCSDFchannel::createCopy(CSDFcomponent &c) const {
    auto ch = std::make_unique<TimedCSDFchannel>(c);

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
 * isUnbounded ()
 * The function returns true if the channel is unbounded. If the size is bounded
 * it returns false.
 */
bool TimedCSDFchannel::isUnbounded() const { return getBufferSize().sz == -1; }

/**
 * isControlToken ()
 * The function returns true if the channel is contains control tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedCSDFchannel::isControlToken() const { return getTokenSize() == 0; }

/**
 * isDataToken ()
 * The function returns true if the channel is contains data tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedCSDFchannel::isDataToken() const { return getTokenSize() > 0; }

/**
 * print ()
 * Print the channel to the supplied output stream.
 */
std::ostream &TimedCSDFchannel::print(std::ostream &out) const {
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

    CSDFbufferSize bufferSize = getBufferSize();
    if (bufferSize.sz != -1) {
        out << "buffer size:    " << bufferSize.sz << std::endl;
    }

    if (getTokenSize() != -1) {
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
} // namespace CSDF
