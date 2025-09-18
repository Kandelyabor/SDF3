/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 29, 2005
 *
 *  Function        :   Output SDF graph in XML format
 *
 *  History         :
 *      29-07-05    :   Initial version.
 *
 * $Id: xml.cc,v 1.2 2008-03-17 14:07:37 sander Exp $
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

#include "xml.h"
namespace SDF {

namespace {

CNode *createSDFnode(const SDFgraph& g) {
    CNode *sdfNode = nullptr;

    // SDF node
    sdfNode = CNewNode("sdf");
    CAddAttribute(sdfNode, "name", g.getName());
    CAddAttribute(sdfNode, "type", g.getType());

    // Actors
    for (const auto& a : g.getActors()) {
        // Actor node
        CNode *actorNode = CAddNode(sdfNode, "actor");
        CAddAttribute(actorNode, "name", a->getName());
        CAddAttribute(actorNode, "type", a->getType());

        // Ports
        for (const auto& p : a->getPorts()) {
            // Port node
            CNode *portNode = CAddNode(actorNode, "port");
            CAddAttribute(portNode, "name", p->getName());
            CAddAttribute(portNode, "type", p->getTypeAsString());
            CAddAttribute(portNode, "rate", CString(p->getRate()));
        }
    }

    // Channels
    for (const auto& c : g.getChannels()) {
        // Channel node
        CNode *channelNode = CAddNode(sdfNode, "channel");
        CAddAttribute(channelNode, "name", c->getName());
        CAddAttribute(channelNode, "srcActor", c->getSrcActor()->getName());
        CAddAttribute(channelNode, "srcPort", c->getSrcPort()->getName());
        CAddAttribute(channelNode, "dstActor", c->getDstActor()->getName());
        CAddAttribute(channelNode, "dstPort", c->getDstPort()->getName());
        if (c->getInitialTokens() != 0) {
            CAddAttribute(channelNode, "initialTokens", CString(c->getInitialTokens()));
        }
    }

    return sdfNode;
}

CNode *createSDFpropertiesNode(const TimedSDFgraph& g) {
    // SDF properties node
    CNode *sdfPropNode = CNewNode("sdfProperties");

    // Actors
    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        // Actor node
        CNode *actorNode = CAddNode(sdfPropNode, "actorProperties");
        CAddAttribute(actorNode, "actor", a->getName());

        // Processors
        for (const auto& p : a->getProcessors()) {
            // Processor node
            CNode *procNode = CAddNode(actorNode, "processor");
            CAddAttribute(procNode, "type", p->type);

            // Default processor?
            if (a->getDefaultProcessor() == p->type) {
                CAddAttribute(procNode, "default", "true");
            }

            // Execution time
            if (p->execTime != SDFTIME_MAX) {
                CNode *execTimeNode = CAddNode(procNode, "executionTime");
                CAddAttribute(execTimeNode, "time", CString(p->execTime));
            }

            // State size
            if (p->stateSize != CSIZE_MAX) {
                CNode *memoryNode = CAddNode(procNode, "memory");
                CNode *stateSizeNode = CAddNode(memoryNode, "stateSize");
                CAddAttribute(stateSizeNode, "max", CString(p->stateSize));
            }
        }
    }

    // Channels
    for (const auto& iter : g.getChannels()) {
        const auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());
        TimedSDFchannel::BufferSize bufferSize;

        // Channel node
        CNode *channelNode = CAddNode(sdfPropNode, "channelProperties");
        CAddAttribute(channelNode, "channel", c->getName());

        // Buffer size
        bufferSize = c->getBufferSize();
        if (bufferSize.sz != -1) {
            CNode *bufferSzNode = CAddNode(channelNode, "bufferSize");
            CAddAttribute(bufferSzNode, "sz", CString(bufferSize.sz));

            if (bufferSize.src != -1) {
                CAddAttribute(bufferSzNode, "src", CString(bufferSize.src));
            }

            if (bufferSize.dst != -1) {
                CAddAttribute(bufferSzNode, "dst", CString(bufferSize.dst));
            }

            if (bufferSize.mem != -1) {
                CAddAttribute(bufferSzNode, "mem", CString(bufferSize.mem));
            }
        }

        // Token size
        if (c->getTokenSize() != -1) {
            CNode *tokenSzNode = CAddNode(channelNode, "tokenSize");
            CAddAttribute(tokenSzNode, "sz", CString(c->getTokenSize()));
        }

        // Token type
        if (!c->getTokenType().empty()) {
            CNode *tokenTypeNode = CAddNode(channelNode, "tokenType");
            CAddAttribute(tokenTypeNode, "type", c->getTokenType());
        }

        // Bandwidth
        if (c->getMinBandwidth() != 0) {
            CNode *bwNode = CAddNode(channelNode, "bandwidth");
            CAddAttribute(bwNode, "min", CString(c->getMinBandwidth()));
        }

        // Latency
        if (c->getMinLatency() != 0) {
            CNode *latencyNode = CAddNode(channelNode, "latency");
            CAddAttribute(latencyNode, "min", CString(c->getMinLatency()));
        }
    }

    // Graph
    CNode *graphPropNode = CAddNode(sdfPropNode, "graphProperties");

    // Throughput constraint
    if (std::abs(g.getThroughputConstraint()) > SDFthroughputEpsilon) {
        CString thr = CString(g.getThroughputConstraint());
        CNode *timeConstraintsNode = CAddNode(graphPropNode, "timeConstraints");
        CAddNode(timeConstraintsNode, "throughput", thr);
    }

    return sdfPropNode;
}

} // namespace

/**
 * createApplicationGraphNode ()
 * The function returns an SDF graph in XML format.
 */
CNode *createApplicationGraphNode(const TimedSDFgraph& g) {
    // Application graph node
    CNode *appNode = CNewNode("applicationGraph");

    // SDF node
    CAddNode(appNode, createSDFnode(g));

    // SDF properties node
    CAddNode(appNode, createSDFpropertiesNode(g));

    return appNode;
}

/**
 * outputSDFasXML ()
 * The function outputs a SDF graph in XML format.
 */
void outputSDFasXML(const SDFgraph& g, std::ostream &out) {
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // Application graph node
    CNode *appNode = CAddNode(sdf3Node, "applicationGraph");

    // SDF node
    CAddNode(appNode, createSDFnode(g));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}

/**
 * outputSDFasXML ()
 * The function outputs a SDF graph in XML format.
 */
void outputSDFasXML(const TimedSDFgraph& g, std::ostream &out) {
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // Application graph node
    CNode *appNode = CAddNode(sdf3Node, "applicationGraph");

    // SDF node
    CAddNode(appNode, createSDFnode(g));

    // SDF properties node
    CAddNode(appNode, createSDFpropertiesNode(g));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}
} // namespace SDF
