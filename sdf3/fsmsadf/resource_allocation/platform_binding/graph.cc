/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Platform graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: graph.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "graph.h"

namespace FSMSADF {

/**
 * PlatformBinding ()
 * Constructor.
 */
PlatformBinding::PlatformBinding(
        const GraphComponent &c, PlatformGraph *pg, Graph *ag, BindingMethod bm, bool create) :
    GraphComponent(c),
    applicationGraph(ag),
    platformGraph(pg),
    bindingMethod(bm),
    graphBindingConstraints(nullptr) {
    // Create tile and connection binding object for all corresponding elements
    // in the platform graph? (i.e. constructor not called through clone method)
    if (create) {
        // Application graph binding constraints
        setGraphBindingConstraints(std::make_unique<GraphBindingConstraints>());

        // Create binding object for all tiles
        for (const auto& t : pg->getTiles()) {
            createTileBinding(t.get());
        }

        // Create binding object for all connections
        for (const auto& c : pg->getConnections()) {
            createConnectionBinding(c.get());
        }
    }
}

/**
 * ~PlatformBinding ()
 * Destructor.
 */
PlatformBinding::~PlatformBinding() = default;

/**
 * clone()
 * Create a clone of this platform binding.
 */
std::unique_ptr<PlatformBinding> PlatformBinding::clone(const GraphComponent &c) const {
    auto pb = std::make_unique<PlatformBinding>(
            c, getPlatformGraph(), getApplicationGraph(), getBindingMethod(), false);

    // Tile bindings
    for (const auto& tb : tileBindings) {
        std::unique_ptr<TileBinding> tbNew = tb->clone(GraphComponent(pb.get(), tb->getId(), tb->getName()));
        pb->getTileBindings().push_back(std::move(tbNew));
    }

    // Connection bindings
    for (const auto& cb : connectionBindings) {
        std::unique_ptr<ConnectionBinding> cbNew = cb->clone(GraphComponent(pb.get(), cb->getId(), cb->getName()));
        pb->getConnectionBindings().push_back(std::move(cbNew));
    }

    // Application graph binding constraints
    pb->setGraphBindingConstraints(getGraphBindingConstraints()->clone(pb->getApplicationGraph()));

    return pb;
}

/**
 * constructFromXML()
 * This function creates the binding as specified by the mappingNode.
 */
void PlatformBinding::constructFromXML(const CNodePtr mappingNode) {
    // Name
    if (CHasAttribute(mappingNode, "name")) {
        setName(CGetAttribute(mappingNode, "name"));
    }

    // Set binding method before constructing tileBindings
    if (CHasChildNode(mappingNode, "resourceUsage")) {

        CNodePtr resourceUsageNode = CGetChildNode(mappingNode, "resourceUsage");
        if (!CHasAttribute(resourceUsageNode, "method")) {
            throw CException("No resource usage calculation method specified for this mapping.");
        }

        // Set binding method
        setBindingMethod(BindingMethod(CGetAttribute(resourceUsageNode, "method")));
    }

    // Iterate over all scenario nodes in the node
    for (CNode *sn = CGetChildNode(mappingNode, "scenario"); sn != nullptr;
         sn = CNextNode(sn, "scenario")) {
        // Find scenario in the application graph
        if (!CHasAttribute(sn, "name")) {
            throw CException("Scenario has no name identifier.");
        }
        ScenarioRef s = getApplicationGraph()->getScenarioRef(CGetAttribute(sn, "name"));

        // Iterate over all child nodes in the scenario to get the constraints,
        //  construction of tiles depends on this so it should happen first
        for (CNode *n = CGetChildNode(sn); n != nullptr; n = CNextNode(n)) {
            // Constraint
            if (CIsNode(n, "constraints")) {
                getGraphBindingConstraints()->constructFromXML(*s, n);
            }
        }

        // Iterate over all child nodes in the scenario to get the rest
        for (CNode *n = CGetChildNode(sn); n != nullptr; n = CNextNode(n)) {
            // Tile?
            if (CIsNode(n, "tile")) {
                // Name?
                if (!CHasAttribute(n, "name")) {
                    throw CException("Tile has no name identifier.");
                }

                // Locate tile
                Tile *t = getPlatformGraph()->getTile(CGetAttribute(n, "name"));

                // Set binding method
                getTileBinding(t)->setBindingMethod(getBindingMethod());

                // Set resource usage
                getTileBinding(t)->constructFromXML(*s, n);
            }

            // Connection?
            if (CIsNode(n, "connection")) {
                // Name?
                if (!CHasAttribute(n, "name")) {
                    throw CException("Connection has no name identifier.");
                }

                // Locate connection
                Connection *c = getPlatformGraph()->getConnection(CGetAttribute(n, "name"));

                // Set resource usage
                getConnectionBinding(c)->constructFromXML(*s, n);
            }
        }
    }

    // Resource usage
    if (CHasChildNode(mappingNode, "resourceUsage")) {
        constructResourceUsageFromXML(CGetChildNode(mappingNode, "resourceUsage"));
    }
}

/**
 * constructResourceUsageFromXML()
 * This function sets the resource usage of the platform binding.
 */
void PlatformBinding::constructResourceUsageFromXML(const CNodePtr resourceUsageNode) {
    // Iterate over all child nodes in the node
    for (CNode *n = CGetChildNode(resourceUsageNode); n != nullptr; n = CNextNode(n)) {
        // Tile?
        if (CIsNode(n, "tile")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Tile has no name identifier.");
            }

            // Locate tile
            Tile *t = getPlatformGraph()->getTile(CGetAttribute(n, "name"));

            // Set binding method
            getTileBinding(t)->setBindingMethod(getBindingMethod());

            // Set resource usage
            getTileBinding(t)->constructResourceUsageFromXML(n);
        }

        // Connection?
        if (CIsNode(n, "connection")) {
            // Name?
            if (!CHasAttribute(n, "name")) {
                throw CException("Connection has no name identifier.");
            }

            // Locate connection
            Connection *c = getPlatformGraph()->getConnection(CGetAttribute(n, "name"));

            // Set resource usage
            getConnectionBinding(c)->constructResourceUsageFromXML(n);
        }
    }
}

/**
 * convertToXML()
 * This function converts the platform binding to an XML object.
 */
void PlatformBinding::convertToXML(const CNodePtr mappingNode) {
    // Name
    CAddAttribute(mappingNode, "name", getName());

    // Application graph
    CAddAttribute(mappingNode, "appGraph", applicationGraph->getName());

    // Platform graph
    CAddAttribute(mappingNode, "archGraph", platformGraph->getName());

    // Iterate over the scenarios
    for (const auto& s : getApplicationGraph()->getScenarios()) {
        // Create node for this scenario
        CNode *scenarioNode = CAddNode(mappingNode, "scenario");
        CAddAttribute(scenarioNode, "name", s->getName());

        // Iterate over all tile bindings
        for (const auto& tb : tileBindings) {
            tb->convertToXML(*s, CAddNode(scenarioNode, "tile"));
        }

        // Iterate over all connection bindings
        for (const auto& cb : connectionBindings) {
            cb->convertToXML(*s, CAddNode(scenarioNode, "connection"));
        }

        // Constraints
        getGraphBindingConstraints()->convertToXML(s.get(), CAddNode(scenarioNode, "constraints"));
    }

    // Resource usage
    convertResourceUsageToXML(CAddNode(mappingNode, "resourceUsage"));
}

/**
 * convertResourceUsageToXML()
 * This function converts the resource usage of this platform binding
 * to an XML object.
 */
void PlatformBinding::convertResourceUsageToXML(const CNodePtr resourceUsageNode) {
    // Set bindingMethod
    CAddAttribute(resourceUsageNode, "method", getBindingMethod().asString());

    // Iterate over all tile bindings
    for (const auto& tb : tileBindings) {
        tb->convertResourceUsageToXML(CAddNode(resourceUsageNode, "tile"));
    }

    // Iterate over all connection bindings
    for (const auto& cb : connectionBindings) {
        cb->convertResourceUsageToXML(CAddNode(resourceUsageNode, "connection"));
    }
}

/**
 * isInitialBinding()
 * The function returns true if this platform binding is the initial
 * binding. The name of a default binding has the value "initial".
 */
bool PlatformBinding::isInitialBinding() const { return getName() == "initial"; }

/**
 * getTileBinding()
 * The function returns a pointer to the tile binding object of tile t.
 */
TileBinding *PlatformBinding::getTileBinding(const Tile *t) {
    for (const auto& tb : tileBindings) {
        if (tb->getTile() == t) {
            return tb.get();
        }
    }

    throw CException("Tile '" + t->getName() + "' has no binding.");
}

/**
 * getConnectionBinding()
 * The function returns a pointer to the connection binding object of
 * connection c.
 */
ConnectionBinding *PlatformBinding::getConnectionBinding(const Connection *c) {
    for (const auto& cb : connectionBindings) {
        if (cb->getConnection() == c) {
            return cb.get();
        }
    }

    throw CException("Connection '" + c->getName() + "' has no binding.");
}

/**
 * The function returns a pointer to the processor binding object associated
 * with the actor a in scenario s. A nullptr pointer is returned when the actor is
 * not bound to any processor in the given scenario.
 * @param scenario s
 * @param actor a
 * @return pointer to processor binding or nullptr in case actor is not bound.
 */
ProcessorBinding* PlatformBinding::getProcessorBindingOfActorInScenario(const Scenario& s, Actor *a) const {
    // Iterate over all tiles
    for (const auto& tb : getConstTileBindings()) {
        // Iterate over all processors
        for (const auto& pb : tb->getProcessorBindings()) {
            // Actor bound in scenario s to this processor
            if (pb->hasActorBinding(s, a)) {
                return pb.get();
            }
        }
    }

    return nullptr;
}

/**
 * The function returns the ProcessorBinding of the given actor name
 */
ProcessorBinding* PlatformBinding::getProcessorBindingOfActorNameInScenario(const Scenario& s,
                                                                            const CString &name) const {
    // Iterate over all tiles
    for (const auto& tb : getConstTileBindings()) {
        // Iterate over all processors
        for (const auto& pb : tb->getProcessorBindings()) {
            // Actor bound in scenario s to this processor
            if (pb->hasActorBindingName(s, name)) {
                return pb.get();
            }
        }
    }

    return nullptr;
}

/**
 * createTileBinding()
 * The function creates a tile binding object for tile t and adds it to the
 * list of tile bindings associated with this platform binding.
 */
TileBinding* PlatformBinding::createTileBinding(Tile *t) {
    auto tb =
            std::make_unique<TileBinding>(GraphComponent(this, t->getId(), t->getName()), t, getBindingMethod());
    auto *tbr = tb.get();
    tileBindings.push_back(std::move(tb));

    return tbr;
}

/**
 * createConnectionBinding()
 * The function creates a connection binding object for tile t and adds it to
 * the list of connection bindings associated with this platform binding.
 */
ConnectionBinding *PlatformBinding::createConnectionBinding(Connection *c) {
    auto cb = std::make_unique<ConnectionBinding>(GraphComponent(this, c->getId(), c->getName()), c);
    auto *cbr = cb.get();
    connectionBindings.push_back(std::move(cb));

    return cbr;
}

} // End namespace FSMSADF
