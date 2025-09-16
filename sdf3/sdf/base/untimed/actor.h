/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF actor
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: actor.h,v 1.1.1.1.2.2 2010-04-24 23:52:43 mgeilen Exp $
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

#ifndef SDF_BASE_UNTIMED_ACTOR_H_INCLUDED
#define SDF_BASE_UNTIMED_ACTOR_H_INCLUDED

#include "component.h"
#include "port.h"
#include <list>
#include <memory>

namespace SDF {

/**
 * SDFactor
 * Actor in SDF graph
 */
class SDFactor : public SDFcomponent {
public:
    // Constructor
    explicit SDFactor(SDFcomponent &c);

    // Destructor
    ~SDFactor() override = default;

    // Copy constructor
    SDFactor(const SDFactor& other) = delete;

    // Copy assignment operator
    SDFactor& operator=(const SDFactor& other) = delete;

    // Move constructor
    SDFactor(SDFactor&& other) noexcept = delete;

    // Move assignment operator
    SDFactor& operator=(SDFactor&& other) noexcept = delete;

    // Construct
    [[nodiscard]] virtual std::unique_ptr<SDFactor> create(SDFcomponent &c) const;
    [[nodiscard]] virtual std::unique_ptr<SDFactor> createCopy(SDFcomponent &c) const;
    [[nodiscard]] virtual std::unique_ptr<SDFactor> clone(SDFcomponent &c) const;
    void construct(CNodePtr actorNode);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Ports
    SDFport *getPort(CId id);
    SDFport *getPort(const CString &name);
    [[nodiscard]] uint nrPorts() const { return static_cast<uint>(ports.size()); };
    [[nodiscard]] const SDFports &getPorts() const { return this->ports; };
    SDFportsIter portsBegin() { return ports.begin(); };
    SDFportsIter portsEnd() { return ports.end(); };
    [[nodiscard]] SDFportsCIter portsBegin() const { return ports.begin(); };
    [[nodiscard]] SDFportsCIter portsEnd() const { return ports.end(); };
    void addPort(std::unique_ptr<SDFport> p);
    void removePort(const CString &name);
    virtual SDFportRef createPort(SDFcomponent &c);
    SDFportRef createPort(SDFport::SDFportType type, SDFrate rate);

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Graph
    [[nodiscard]] SDFgraph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFactor &a) { return a.print(out); };

private:
    // Type
    CString type;

    // Ports
    SDFports ports;
};

using SDFactorRef = SDFactor *;
using SDFactors = std::list<std::unique_ptr<SDFactor>>;
using SDFactorRefs = std::list<SDFactor *>;
using SDFactorConstRefs = std::list<const SDFactor *>;
using SDFactorsIter = SDFactors::iterator;
using SDFactorsCIter = SDFactors::const_iterator;
using SDFactorRefsIter = SDFactorRefs::iterator;
using SDFactorRefsCIter = SDFactorRefs::const_iterator;

} // namespace SDF
#endif
