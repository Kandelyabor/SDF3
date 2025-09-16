/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   autoconc.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Model maximum amount of auto-concurrency explicitly.
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: autoconc.cc,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#include "autoconc.h"
#include <memory>
namespace SDF
{


    /**
     * modelAutoConcurrencyInSDFgraph ()
     * Create a new SDF graph in which the maximum amount of auto-concurrency
     * on all actors is made explicit through self-edges.
     */
    std::unique_ptr<SDFgraph> modelAutoConcurrencyInSDFgraph(const SDFgraph& graph,
            const uint maxDegree)
    {
        // Create a copy of the original graph
        SDFcomponent comp(graph.getParent(), graph.getId());
        std::unique_ptr<SDFgraph> g = graph.clone(comp);

        // Iterate over all actors in the graph
        for (const auto& a: g->getActors())
        {
            // Create self-edge on the actor
            g->createChannel(a.get(), 1, a.get(), 1, maxDegree);
        }

        return g;
    }
}//namespace SDF
