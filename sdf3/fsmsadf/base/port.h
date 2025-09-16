/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF port
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: port.h,v 1.1.2.3 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_PORT_H_INCLUDED
#define FSMSADF_BASE_PORT_H_INCLUDED

#include "scenario.h"
#include "type.h" // IWYU pragma: keep
#include <memory>


namespace FSMSADF {

// Rate on ports of graphs
using Rate = uint;

// Forward class definition
class Port;
class Actor;
class Channel;
class ScenarioGraph;

// List of port pointers
using Ports = std::list<std::unique_ptr<Port>>;
using PortRefs = std::list<Port*>;

/**
 * Port
 * Port on an actor.
 */
class Port : public GraphComponent {
public:
    enum class PortType { In, Out, Undef };

    // Constructor
    explicit Port(const GraphComponent& c);

    // Destructor
    ~Port() override;

    Port(const Port&) = delete;
    Port& operator=(const Port&) = delete;
    Port(Port&&) = delete;
    Port& operator=(Port&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<Port> create(const GraphComponent& c);
    [[nodiscard]] std::unique_ptr<Port> createCopy(const GraphComponent& c) const;
    [[nodiscard]] std::unique_ptr<Port> clone(const GraphComponent& c) const;

    // Construct
    void constructFromXML(CNodePtr portNode);

    // Convert
    void convertToXML(CNodePtr portNode);

    // Isolate scenario
    void isolateScenario(const Scenario& s);

    // Type
    [[nodiscard]] PortType getType() const { return type; };
    [[nodiscard]] CString getTypeAsString() const;
    void setType(const PortType t) { type = t; };
    void setType(const CString &t);

    // Rate
    [[nodiscard]] const std::map<const Scenario *, Rate> &getRate() const { return rate; };
    void setRate(const std::map<const Scenario *, Rate> &r);
    [[nodiscard]] Rate getRateOfScenario(const Scenario& s) const;
    void setRateOfScenario(const Scenario& s, Rate r);

    // Channel
    [[nodiscard]] Channel *getChannel() const { return channel; };
    void connectToChannel(Channel *c);

    // Actor
    [[nodiscard]] Actor& getActor() const;

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, Port &p) { return p.print(out); };

private:
    // Default scenario
    [[nodiscard]] ScenarioRef getDefaultScenarioRef() const;

    // Port type
    PortType type;

    // Channel
    Channel *channel;

    // Rate
    std::map<const Scenario *, Rate> rate;
};

using PortRef = Port*;

} // End namespace FSMSADF

#endif
