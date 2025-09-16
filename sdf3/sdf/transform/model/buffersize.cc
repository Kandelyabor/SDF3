/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffersize.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Model buffer sizes in the graph with explicit channels.
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: buffersize.cc,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#include "buffersize.h"
#include <memory>
namespace SDF {

/**
 * modelBufferSizeInSDFgraph ()
 * Create a new timed SDF graph in which channel sizes are modelled
 * through explicit channels (all buffer sizes are set to unbounded).
 */
std::unique_ptr<TimedSDFgraph> modelBufferSizeInSDFgraph(const TimedSDFgraph &graph) {
    TimedSDFchannel::BufferSize unboundedBuffer;

    // Create a copy of the original graph
    SDFcomponent comp(graph.getParent(), graph.getId());
    std::unique_ptr<TimedSDFgraph> g = TimedSDFgraph::dynamicCast(graph.clone(comp));

    // Unbounded buffer size
    unboundedBuffer.sz = -1;
    unboundedBuffer.mem = -1;
    unboundedBuffer.src = -1;
    unboundedBuffer.dst = -1;

    // Iterate over all channels in the graph
    for (const auto& iter : g->getChannels()) {
        auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

        // Source and destination actor and port of channel
        SDFactor *srcActor = c->getSrcActor();
        SDFport *srcPort = c->getSrcPort();
        SDFactor *dstActor = c->getDstActor();
        SDFport *dstPort = c->getDstPort();

        // Is channel bounded?
        if (!c->isUnbounded()) {
            // Create a backward channel to model buffer size
            comp = SDFcomponent(g.get(), g->nrChannels());

            std::unique_ptr<TimedSDFchannel> cN = TimedSDFchannel::dynamicCast(c->create(comp));
            cN->setName(CString("_") + c->getName() + CString("b"));

            // Initial tokens on channel to model buffer size of channel c
            cN->setInitialTokens((c->getBufferSize().sz) - c->getInitialTokens());

            // Set channels to be unbounded
            c->setBufferSize(unboundedBuffer);
            cN->setBufferSize(unboundedBuffer);

            // Create new ports on the source and destination actors
            comp = SDFcomponent(dstActor, dstActor->nrPorts());
            std::unique_ptr<SDFport> srcPortN = dstPort->create(comp);
            srcPortN->setName(CString("_p") + CString(dstActor->nrPorts() + 1));
            srcPortN->setType("out");
            srcPortN->setRate(dstPort->getRate());
            auto* srcPortNPtr = srcPortN.get();
            dstActor->addPort(std::move(srcPortN));

            comp = SDFcomponent(srcActor, srcActor->nrPorts());
            std::unique_ptr<SDFport> dstPortN = srcPort->create(comp);
            dstPortN->setName(CString("_p") + CString(srcActor->nrPorts() + 1));
            dstPortN->setType("in");
            dstPortN->setRate(srcPort->getRate());
            auto* dstPortNPtr = dstPortN.get();
            srcActor->addPort(std::move(dstPortN));

            // Connect ports to channel
            cN->connectSrc(srcPortNPtr);
            cN->connectDst(dstPortNPtr);

            // Create relation between buffer channel and channel c
            cN->setStorageSpaceChannel(c);

            // Add new channel to the graph
            g->addChannel(std::move(cN));
        }
    }

    return g;
}
} // namespace SDF
