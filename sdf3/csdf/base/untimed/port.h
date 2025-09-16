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
 *  Function        :   CSDF port
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: port.h,v 1.3 2008-03-22 14:24:21 sander Exp $
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

#ifndef CSDF_BASE_PORT_H_INCLUDED
#define CSDF_BASE_PORT_H_INCLUDED

#include "component.h"
#include "rate.h"
namespace CSDF {
// Forward class definitions
class CSDFport;
class CSDFactor;
class CSDFchannel;
class CSDFgraph;

/**
 * CSDFport
 * Port on an CSDF actor.
 */
class CSDFport : public CSDFcomponent {
public:
    enum CSDFportType { In, Out };

    // Constructor
    explicit CSDFport(CSDFcomponent &c);

    // Destructor
    ~CSDFport() override;

    // Delete copy/move constructors and assignment operators
    CSDFport(const CSDFport&) = delete;
    CSDFport& operator=(const CSDFport&) = delete;
    CSDFport(CSDFport&&) = delete;
    CSDFport& operator=(CSDFport&&) = delete;

    // Construct
    virtual std::unique_ptr<CSDFport> create(CSDFcomponent &c) const;
    virtual std::unique_ptr<CSDFport> createCopy(CSDFcomponent &c) const;
    virtual std::unique_ptr<CSDFport> clone(CSDFcomponent &c) const;
    void construct(CNodePtr portNode);

    // Type
    [[nodiscard]] CSDFportType getType() const { return type; };
    [[nodiscard]] CString getTypeAsString() const {
        if (type == In) {
            return "in";
        }
        return "out";
    };
    void setType(const CSDFportType t) { type = t; };
    void setType(const CString &t) {
        if (t == "in") {
            type = In;
        } else if (t == "out") {
            type = Out;
        }
    };

    // Rate
    [[nodiscard]] const CSDFrate &getRate() const { return rate; };
    void setRate(const CSDFrate &r) { rate = r; };

    // Channel
    [[nodiscard]] CSDFchannel *getChannel() const { return channel; };
    void connectToChannel(CSDFchannel *c);

    // Actor
    [[nodiscard]] CSDFactor *getActor() const;

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, CSDFport &p) { return p.print(out); };

private:
    CSDFportType type;
    CSDFrate rate;
    CSDFchannel *channel;
};

using CSDFports = std::list<std::unique_ptr<CSDFport>>;
using CSDFportRefs = std::list<CSDFport *>;
using CSDFportsIter = CSDFports::iterator;
using CSDFportsCIter = CSDFports::const_iterator;
} // namespace CSDF
#endif
