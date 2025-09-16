/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
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
 * $Id: channel.h,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_UNTIMED_CHANNEL_H_INCLUDED
#define SDF_BASE_UNTIMED_CHANNEL_H_INCLUDED

#include "actor.h"
#include "input.h"
#include "output.h"
#include "port.h"
#include <memory>
namespace SDF {

// Forward class definition
class SDFinputchannel;
class SDFoutputchannel;
class SDFchannel;
class SDFgraph;

/**
 * SDFchannel
 * SDF channel.
 */
class SDFchannel : public SDFcomponent {
public:
    // Constructor
    explicit SDFchannel(SDFcomponent &c);

    // Copy constructor
    SDFchannel(const SDFchannel& other) = delete;

    // Copy assignment operator
    SDFchannel& operator=(const SDFchannel& other) = delete;

    // Move constructor
    SDFchannel(SDFchannel&& other) noexcept = delete;

    // Move assignment operator
    SDFchannel& operator=(SDFchannel&& other) noexcept = default;

    // Destructor
    ~SDFchannel() override;

    // Construct
    virtual std::unique_ptr<SDFchannel> create(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFchannel> createCopy(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFchannel> clone(SDFcomponent &c) const;
    void construct(CNodePtr channelNode);

    // Connections
    [[nodiscard]] SDFportRef getSrcPort() const { return src; };
    [[nodiscard]] SDFportRef getDstPort() const { return dst; };
    [[nodiscard]] SDFactor *getSrcActor() const { return (src == nullptr ? nullptr : getSrcPort()->getActor()); };
    [[nodiscard]] SDFactor *getDstActor() const { return (dst == nullptr ? nullptr : getDstPort()->getActor()); };
    void connectSrc(SDFportRef p);
    void connectDst(SDFportRef p);
    SDFportRef oppositePort(SDFportRef p) const { return (src == p ? dst : src); };

    // Initial tokens
    [[nodiscard]] uint getInitialTokens() const { return initialTokens; };
    void setInitialTokens(const uint t) { initialTokens = t; };

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Graph
    [[nodiscard]] SDFgraph *getGraph() const ;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFchannel &c) { return c.print(out); };

private:
    SDFportRef src;
    SDFportRef dst;
    uint initialTokens;
};

using SDFchannelRef = SDFchannel*;

class SDFinputchannel : public SDFcomponent {
public:
    explicit SDFinputchannel(SDFcomponent &c);

    // Copy constructor
    SDFinputchannel(const SDFinputchannel& other) = delete;

    // Copy assignment operator
    SDFinputchannel& operator=(const SDFinputchannel& other) = delete;

    // Move constructor
    SDFinputchannel(SDFinputchannel&& other) noexcept = delete;

    // Move assignment operator
    SDFinputchannel& operator=(SDFinputchannel&& other) noexcept = default;

    ~SDFinputchannel() override;
    virtual std::unique_ptr<SDFinputchannel> create(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFinputchannel> createCopy(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFinputchannel> clone(SDFcomponent &c) const;
    void construct( CNodePtr channelNode);

    // Connections
    [[nodiscard]] SDFportRef getSrcPort() const { return src; };
    [[nodiscard]] SDFportRef getDstPort() const { return dst; };
    [[nodiscard]] SDFinput *getSrcInput() const { return (src == nullptr ? nullptr : getSrcPort()->getInput()); };
    [[nodiscard]] SDFactor *getDstActor() const { return (dst == nullptr ? nullptr : getDstPort()->getActor()); };
    void connectSrc(SDFportRef p);
    void connectDst(SDFportRef p);
    SDFportRef oppositePort(SDFportRef p) const { return (src == p ? dst : src); };

    // Initial tokens
    [[nodiscard]] uint getInitialTokens() const { return initialTokens; };
    void setInitialTokens(const uint t) { initialTokens = t; };

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Graph
    [[nodiscard]] SDFgraph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFinputchannel &c) { return c.print(out); };

private:
    SDFportRef src;
    SDFportRef dst;
    uint initialTokens;
};

using SDFinputchannelRef = SDFinputchannel*;

class SDFoutputchannel : public SDFcomponent {
public:
    explicit SDFoutputchannel(SDFcomponent &c);

    // Copy constructor
    SDFoutputchannel(const SDFoutputchannel& other) = delete;

    // Copy assignment operator
    SDFoutputchannel& operator=(const SDFoutputchannel& other) = delete;

    // Move constructor
    SDFoutputchannel(SDFoutputchannel&& other) noexcept = delete;

    // Move assignment operator
    SDFoutputchannel& operator=(SDFoutputchannel&& other) noexcept = default;

    ~SDFoutputchannel() override;
    virtual std::unique_ptr<SDFoutputchannel> create(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFoutputchannel> createCopy(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFoutputchannel> clone(SDFcomponent &c) const;
    void construct( CNodePtr channelNode);

    // Connections
    [[nodiscard]] SDFportRef getSrcPort() const { return src; };
    [[nodiscard]] SDFportRef getDstPort() const { return dst; };
    [[nodiscard]] SDFactor *getSrcActor() const { return (src == nullptr ? nullptr : getSrcPort()->getActor()); };
    [[nodiscard]] SDFoutput *getDstOutput() const { return (dst == nullptr ? nullptr : getDstPort()->getOutput()); };
    void connectSrc(SDFportRef p);
    void connectDst(SDFportRef p);
    SDFportRef oppositePort(SDFportRef p) const { return (src == p ? dst : src); };

    // Initial tokens
    [[nodiscard]] uint getInitialTokens() const { return initialTokens; };
    void setInitialTokens(const uint t) { initialTokens = t; };

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Graph
    [[nodiscard]] SDFgraph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFoutputchannel &c) {
        return c.print(out);
    };

private:
    SDFportRef src;
    SDFportRef dst;
    uint initialTokens;
};

using SDFoutputchannelRef = SDFoutputchannel*;

using SDFchannels = std::list<std::unique_ptr<SDFchannel>>;
using SDFchannelRefs = std::list<SDFchannel *>;
using SDFchannelsIter = SDFchannels::iterator;
using SDFchannelsCIter = SDFchannels::const_iterator;

using SDFinputchannels = std::list<std::unique_ptr<SDFinputchannel>>;
using SDFinputchannelRefs = std::list<SDFinputchannel *>;
using SDFinputchannelsIter = SDFinputchannels::iterator;
using SDFinputchannelsCIter = SDFinputchannels::const_iterator;

using SDFoutputchannels = std::list<std::unique_ptr<SDFoutputchannel>>;
using SDFoutputchannelRefs = std::list<SDFoutputchannel *>;
using SDFoutputchannelsIter = SDFoutputchannels::iterator;
using SDFoutputchannelsCIter = SDFoutputchannels::const_iterator;

} // namespace SDF
#endif
