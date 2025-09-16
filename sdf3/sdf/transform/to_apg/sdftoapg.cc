/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdftoapg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 2, 2005
 *
 *  Function        :   Convert a timed SDF graph to an acyclic precedence graph
 *
 *  History         :
 *      02-08-05    :   Initial version.
 *
 * $Id: sdftoapg.cc,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#include "sdftoapg.h"
#include "sdf/base/hsdf/check.h"
#include "sdf/transform/hsdf/unfold.h"
#include <memory>
namespace SDF {

/**
 * transformHSDFtoAPG ()
 * The function transforms an HSDF graph to an acyclic precedence graph.
 * The 'id' of the nodes in the APG corresponds to the 'id' of the actors in
 * the HSDF graph.
 */
std::unique_ptr<APGgraph> transformHSDFtoAPG(const TimedSDFgraph &hsdf, const uint blockingFactor) {
    // Check SDF graph
    if (!isHSDFgraph(hsdf)) {
        throw CException("SDF graph is not a HSDF graph.");
    }

    // Unfold HSDF with blocking factor
    auto h = TimedSDFgraph::dynamicCast(unfoldHSDF(hsdf, blockingFactor));

    // Construct new graph
    auto g = std::make_unique<APGgraph>(h->getId());

    // Create node in g for each actor in h
    for (const auto& iter : h->getActors()) {
        auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        APGnode *n = g->newNode(a->getId());

        n->setWeight(static_cast<int>(a->getExecutionTime()));
    }

    // Create edges between the nodes for each channel with no tokens
    for (const auto& iter : h->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());
        APGnode *src = g->getNode(c->getSrcActor()->getId());
        APGnode *dst = g->getNode(c->getDstActor()->getId());

        if (c->getInitialTokens() == 0) {
            g->newEdge(c->getId(), src, dst);
        }
    }

    return g;
}
} // namespace SDF
