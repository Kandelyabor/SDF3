/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF port
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: port.h,v 1.1.1.1.2.1 2009-04-06 12:15:32 mgeilen Exp $
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

#ifndef SDF_BASE_UNTIMED_PORT_H_INCLUDED
#define SDF_BASE_UNTIMED_PORT_H_INCLUDED

#include "base/string/cstring.h"
#include "base/xml/xml.h"
#include "component.h"
#include <cstdint>
#include <memory>

namespace SDF {

// Rate on ports of SDF graphs
using SDFrate = uint;

// Forward class definition
class SDFport;
class SDFactor;
class SDFinput;
class SDFoutput;
class SDFchannel;
class SDFinputchannel;
class SDFoutputchannel;
class SDFgraph;

/**
 * SDFport
 * Port on an SDF actor.
 */
class SDFport : public SDFcomponent {
public:
    enum SDFportType : std::uint8_t { In, Out, Undef };

    // Delete copy/move constructors and assignment operators
    SDFport(const SDFport&) = delete;
    SDFport& operator=(const SDFport&) = delete;
    SDFport(SDFport&&) = delete;
    SDFport& operator=(SDFport&&) = delete;

    // Constructor
    explicit SDFport(SDFcomponent &c);

    // Destructor
    ~SDFport() override;

    // Construct
    virtual std::unique_ptr<SDFport> create(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFport> createCopy(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFport> clone(SDFcomponent &c) const;
    void construct(CNodePtr portNode);

    // Type
    [[nodiscard]] SDFportType getType() const { return type; };
    [[nodiscard]] CString getTypeAsString() const {
        if (type == SDFportType::In) {
            return "in";
        }
        if (type == SDFportType::Out) {
            return "out";
        }
        return "undef";
    };
    void setType(const SDFportType t) { type = t; };
    void setType(const CString &t) {
        if (t == CString("in")) {
            type = SDFportType::In;
        } else if (t == CString("out")) {
            type = SDFportType::Out;
        } else {
            type = SDFportType::Undef;
        }
    };

    // Rate
    [[nodiscard]] SDFrate getRate() const { return rate; };
    void setRate(const SDFrate r) { rate = r; };

    // Channel
    [[nodiscard]] SDFchannel *getChannel() const { return channel; };
    [[nodiscard]] SDFinputchannel *getInputChannel() const { return inputchannel; };
    [[nodiscard]] SDFoutputchannel *getOutputChannel() const { return outputchannel; };

    void connectToChannel(SDFchannel *c);
    void connectToInputChannel(SDFinputchannel *c);
    void connectToOutputChannel(SDFoutputchannel *c);

    // Actor / Input / Ouput
    [[nodiscard]] SDFactor *getActor() const;
    [[nodiscard]] SDFinput *getInput() const ;
    [[nodiscard]] SDFoutput *getOutput() const;

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFport &p) { return p.print(out); };

private:
    SDFportType type;
    SDFrate rate;
    SDFchannel *channel;
    SDFinputchannel *inputchannel;
    SDFoutputchannel *outputchannel;
};

using SDFportRef = SDFport*;
using SDFports = std::list<std::unique_ptr<SDFport>>;
using SDFportRefs = std::list<SDFport *>;
using SDFportsIter = SDFports::iterator;
using SDFportsCIter = SDFports::const_iterator;
using SDFportRefsIter = SDFportRefs::iterator;
using SDFportRefsCIter = SDFportRefs::const_iterator;

} // namespace SDF
#endif
