/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Platform graph.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: graph.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_GRAPH_H_INCLUDED

#include "connection.h"
#include "constraint.h"
#include "tile.h"
#include <memory>


namespace FSMSADF {

// Forward class definition
class PlatformBinding;

// List of platform binding pointers
using PlatformBindings = std::list<std::unique_ptr<PlatformBinding>>;

/**
 * PlatformBinding
 * Container for platform binding.
 */
class PlatformBinding : public GraphComponent {
public:
    // Constructor
    PlatformBinding(
            const GraphComponent& c, PlatformGraph *pg, Graph *ag, BindingMethod bm, bool create = true);

    // Destructor
    ~PlatformBinding() override;

    // Copy constructor
    PlatformBinding(const PlatformBinding&) = delete;
    // Copy assignment operator
    PlatformBinding& operator=(const PlatformBinding&) = delete;
    // Move constructor
    PlatformBinding(PlatformBinding&&) = delete;
    // Move assignment operator
    PlatformBinding& operator=(PlatformBinding&&) = delete;

    // Construct
    [[nodiscard]] std::unique_ptr<PlatformBinding> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr mappingNode);

    // Convert
    void convertToXML(CNodePtr mappingNode);

    // Initial binding
    [[nodiscard]] bool isInitialBinding() const;

    // Application graph
    [[nodiscard]] Graph *getApplicationGraph() const { return applicationGraph; };

    // Platform graph
    [[nodiscard]] PlatformGraph *getPlatformGraph() const { return platformGraph; };

    // Tile bindings
    TileBindings &getTileBindings() { return tileBindings; };
    [[nodiscard]] const TileBindings &getConstTileBindings() const { return tileBindings; };
    TileBinding *getTileBinding(const Tile *t);

    // Connection bindings
    [[nodiscard]] ConnectionBindings &getConnectionBindings() { return connectionBindings; };
    [[nodiscard]] const ConnectionBindings &getConstConnectionBindings() const { return connectionBindings; };    ConnectionBinding *getConnectionBinding(const Connection *c);

    // Actor binding
    ProcessorBinding* getProcessorBindingOfActorInScenario(const Scenario& s, Actor *a) const;

    [[nodiscard]] ProcessorBinding* getProcessorBindingOfActorNameInScenario(const Scenario& s, const CString& name) const;

    // Application graph binding constraints
    [[nodiscard]] GraphBindingConstraints *getGraphBindingConstraints() const { return graphBindingConstraints.get(); };
    void setGraphBindingConstraints(std::unique_ptr<GraphBindingConstraints> c) { graphBindingConstraints = std::move(c); };

    // Get method used for calculating size of binding
    [[nodiscard]] BindingMethod getBindingMethod() const { return bindingMethod; }

    // Set method used for calculating size of binding
    void setBindingMethod(BindingMethod bm) { bindingMethod = bm; }

private:
    // Tile binding
    TileBinding *createTileBinding(Tile *t);

    // Connection binding
    ConnectionBinding *createConnectionBinding(Connection *c);

    // Construct
    void constructResourceUsageFromXML(CNodePtr resourceUsageNode);

    // Convert
    void convertResourceUsageToXML(CNodePtr resourceUsageNode);

    // Application graph
    Graph *applicationGraph;

    // Platform graph
    PlatformGraph *platformGraph;

    // Tile bindings
    TileBindings tileBindings;

    // Connection bindings
    ConnectionBindings connectionBindings;

    // Application graph binding constraints
    std::unique_ptr<GraphBindingConstraints> graphBindingConstraints;

    // Method used for calculating size of binding
    BindingMethod bindingMethod;
};

} // End namespace FSMSADF

#endif
