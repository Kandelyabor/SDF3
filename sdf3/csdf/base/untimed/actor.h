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
 *  Function        :   CSDF actor
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: actor.h,v 1.3.2.1 2010-04-25 01:21:16 mgeilen Exp $
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

#ifndef CSDF_BASE_UNTIMED_ACTOR_H_INCLUDED
#define CSDF_BASE_UNTIMED_ACTOR_H_INCLUDED

#include "port.h"
namespace CSDF {
/**
 * CSDFactor
 * Actor in CSDF graph
 */
class CSDFactor : public CSDFcomponent {
public:
    // Constructor
    explicit CSDFactor(CSDFcomponent &c);

    // Destructor
    ~CSDFactor() override;

    // Prevent copying and moving
    CSDFactor(const CSDFactor&) = delete;
    CSDFactor& operator=(const CSDFactor&) = delete;
    CSDFactor(CSDFactor&&) = delete;
    CSDFactor& operator=(CSDFactor&&) = delete;


    // Construct
    virtual std::unique_ptr<CSDFactor> create(CSDFcomponent &c) const;
    virtual std::unique_ptr<CSDFactor> createCopy(CSDFcomponent &c) const;
    virtual std::unique_ptr<CSDFactor> clone(CSDFcomponent &c) const;
    void construct(CNodePtr actorNode);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Ports
    CSDFport *getPort(CId id);
    CSDFport *getPort(const CString &name);
    CSDFports &getPorts() { return ports; };
    [[nodiscard]] const CSDFports &getConstPorts() const { return ports; };
    [[nodiscard]] uint nrPorts() const { return static_cast<uint>(ports.size()); };
    CSDFportsIter portsBegin() { return ports.begin(); };
    CSDFportsIter portsEnd() { return ports.end(); };
    [[nodiscard]] CSDFportsCIter portsBegin() const { return ports.begin(); };
    [[nodiscard]] CSDFportsCIter portsEnd() const { return ports.end(); };
    void addPort(std::unique_ptr<CSDFport> p);
    void removePort(const CString &name);
    void removePorts();
    virtual CSDFport *createPort(CSDFcomponent &c);
    CSDFport *createPort(const CString &type, const CSDFrate& rate);

    // Sequence
    [[nodiscard]] uint sequenceLength() const {
        if (ports.empty()) {
            return 0;
        }
        return static_cast<uint>(ports.front()->getRate().size());
    };

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Graph
    [[nodiscard]] CSDFgraph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, CSDFactor &a) { return a.print(out); };

private:
    // Type
    CString type;

    // Ports
    CSDFports ports;
};

using CSDFactors = std::list<std::unique_ptr<CSDFactor>>;
using CSDFactorRef = CSDFactor*;
using CSDFactorConstRef = const CSDFactor*;
using CSDFactorRefs = std::list<CSDFactorRef>;
using CSDFactorConstRefs = std::list<CSDFactorConstRef>;
using CSDFactorsIter = CSDFactors::iterator;
using CSDFactorsCIter = CSDFactors::const_iterator;
} // namespace CSDF
#endif
