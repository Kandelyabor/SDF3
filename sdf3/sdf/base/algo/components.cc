/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   components.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Strongly connected components
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: components.cc,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#include "components.h"
#include "base/exception/exception.h"
#include "dfs.h"
#include <algorithm>
#include <memory>
namespace SDF {

namespace{


/**
 * treeVisitChildren ()
 * The function visits all children of the actor 'u'. The parent-child
 * relation is given via the vector 'pi'. Each child is added to the
 * component 'comp'.
 */
void treeVisitChildren(const SDFgraph& g, const std::unique_ptr<SDFactor*[]>& pi, const SDFactor *u, SDFgraphComponent &comp) {
    for (uint i = 0; i < g.nrActors(); i++) {
        // Actor v points to actor u (i.e. v is a child of u)
        if (pi[i] != nullptr && pi[i]->getId() == u->getId()) {
            const SDFactor* v = g.getConstActorPtr(i);

            // Add actor v to the component
            comp.push_back(v);

            // Find all children of v in the tree
            treeVisitChildren(g, pi, v, comp);
        }
    }
}

/**
 * getComponents ()
 * The function determines the strongly connected components in the graph. To do
 * this, it performs depth-first walk on the forest given by 'pi'.
 */
SDFgraphComponents getComponents(const SDFgraph& g, const std::unique_ptr<SDFactor*[]>& pi) {
    SDFgraphComponents components;

    for (uint i = 0; i < g.nrActors(); i++) {
        if (pi[i] == nullptr) {
            const SDFactor* u = g.getConstActorPtr(i);

            // Create new component and add actor u
            SDFgraphComponent comp;
            comp.push_back(u);

            // Find all children of u in the tree
            treeVisitChildren(g, pi, u, comp);

            // Add component to the set of components
            components.push_back(comp);
        }
    }

    return components;
}

} //namespace

/**
 * stronglyConnectedComponents ()
 * The function determines the strongly connected components in the graph.
 */
SDFgraphComponents stronglyConnectedComponents(const SDFgraph& g) {
    SDFgraphComponents components;

    // Initialize
    v_int d(g.nrActors());
    v_int f(g.nrActors(), 0);
    auto pi = std::make_unique<SDFactor*[]>(g.nrActors());
    // auto **pi = static_cast<SDFactor **>(malloc(sizeof(SDFactor *) * g.nrActors()));
    ASSERT(pi != nullptr, "Failed malloc");

    // Call dfs(g) ti compute f[u] for each u
    dfs(g, d, f, pi);

    // Call dfs(gT), consider the vertices in order of decreasing f[u]
    dfs(g, d, f, pi, true);

    // Components are the vertices of the trees in the depth-first forest
    components = getComponents(g, pi);

    return components;
}

/**
 * actorInComponent ()
 * The function check wether an actor is in a component. If so, the
 * function returns true. Else it returns false.
 */
bool actorInComponent(const SDFactor& a, SDFgraphComponent &component) {
    return std::any_of(
            component.begin(), component.end(), [&a](const auto& b) { return b->getId() == a.getId(); });
}

/**
 * isStronglyConnectedGraph ()
 * The function checks that the graph is a strongly connected component.
 */
bool isStronglyConnectedGraph(const SDFgraph& g) {
    SDFgraphComponents c;

    c = stronglyConnectedComponents(g);

    return c.size() == 1;
}

/**
 * componentToSDFgraph ()
 * The function returns an SDF graph containing all actors and channels inside
 * the component.
 */
std::unique_ptr<SDFgraph> componentToSDFgraph(SDFgraphComponent &component) {
    SDFgraph *g = nullptr;
    std::unique_ptr<SDFgraph> gr = nullptr;

    // Get pointer to graph of which the component is a part
    g = (component.front())->getGraph();

    // Clone the graph of which the component is part
    SDFcomponent comp = SDFcomponent(g->getParent(), g->getId());
    gr = g->clone(comp);

    // Remove all channels not part of the component
    for (auto iter = gr->channelsBegin(); iter != gr->channelsEnd();) {
        const std::unique_ptr<SDFchannel>& c = *iter;
        SDFactor *u = c->getSrcActor();
        SDFactor *v = c->getDstActor();

        // Advance iterator
        iter++;

        if (!actorInComponent(*u, component) || !actorInComponent(*v, component)) {
            // Ports
            u->removePort(c->getSrcPort()->getName());
            v->removePort(c->getDstPort()->getName());

            // Channel
            gr->removeChannel(c->getName());
        }
    }

    // Remove all actors not part of the component
    for (auto iter = gr->actorsBegin(); iter != gr->actorsEnd();) {
        const std::unique_ptr<SDFactor>& a = *iter;

        // Advance iterator
        iter++;

        if (!actorInComponent(*a, component)) {
            gr->removeActor(a->getName());
        }
    }

    return gr;
}
} // namespace SDF
