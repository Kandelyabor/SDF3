/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 3, 2006
 *
 *  Function        :   Output mapping and system usage in XML format.
 *
 *  History         :
 *      03-04-06    :   Initial version.
 *
 * $Id: xml.cc,v 1.2 2008-03-06 10:49:45 sander Exp $
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
/**
 * createMappingNode ()
 * Create an XML node which describes the mapping of an application graph
 * onto an platform graph.
 */
CNode *createMappingNode(PlatformGraph *g, SDFgraph *appGraph) {
    // Mapping node
    CNode *mappingNode = CNewNode("mapping");
    CAddAttribute(mappingNode, "appGraph", appGraph->getName());
    CAddAttribute(mappingNode, "archGraph", g->getName());

    // Tiles
    for (const auto& t: g->getTiles()) {
        // Tile
        CNode *tileNode = CAddNode(mappingNode, "tile");
        CAddAttribute(tileNode, "name", t->getName());

        // Processor
        Processor *p = t->getProcessor();
        if (p != nullptr) {
            CNode *procNode = CAddNode(tileNode, "processor");
            CAddAttribute(procNode, "name", p->getName());
            CAddAttribute(procNode, "timeslice", CString(p->getReservedTimeSlice()));

            // Bindings present?
            if (p->getActorBindings() != nullptr) {
                // Actors mapped to processor
                for (auto& b: *(p->getActorBindings())) {
                    CNode *actorNode = CAddNode(procNode, "actor");
                    CAddAttribute(actorNode, "name", b->getComponent()->getName());
                }
            }

            // Schedule
            CAddNode(procNode, p->getSchedule().convertToXML());
        }

        // Memory
        Memory *m = t->getMemory();
        if (m != nullptr) {
            CNode *memNode = CAddNode(tileNode, "memory");
            CAddAttribute(memNode, "name", m->getName());

            // Actor bindings present?
            if (m->getActorBindings() != nullptr) {

                // Actors mapped to memory
                for (auto& b: *(m->getActorBindings())) {
                    const SDFcomponent *c = b->getComponent();

                    if (c == nullptr) {
                        continue;
}

                    CString strValue = CString(b->getValue(0));
                    CNode *actorNode = CAddNode(memNode, "actor");
                    CAddAttribute(actorNode, "name", c->getName());
                    CAddAttribute(actorNode, "size", strValue);
                }
            }

            // Channel bindings present?
            if (m->getChannelBindings() != nullptr) {
                // Channels mapped to memory
                for (auto& b: *(m->getChannelBindings())) {
                    const SDFcomponent *c = b->getComponent();

                    if (c == nullptr) {
                        continue;
                    }

                    CString strValue = CString(b->getValue(0));
                    CNode *channelNode = CAddNode(memNode, "channel");
                    CAddAttribute(channelNode, "name", c->getName());
                    CAddAttribute(channelNode, "size", strValue);
                }
            }
        }

        // Network interface
        NetworkInterface *ni = t->getNetworkInterface();
        if (ni != nullptr) {
            CNode *niNode = CAddNode(tileNode, "networkInterface");
            CAddAttribute(niNode, "name", ni->getName());

            // Bindings present?
            if (ni->getBindings() != nullptr) {
                // Channels mapped to network interface
                for (auto& b : *(ni->getBindings())) {
                    const SDFcomponent *c = b->getComponent();

                    if (c == nullptr) {
                        continue;
                    }

                    CNode *channelNode = CAddNode(niNode, "channel");
                    CAddAttribute(channelNode, "name", c->getName());
                    CString strValue = CString(b->getValue(0));
                    CAddAttribute(channelNode, "nrConnections", strValue);
                    strValue = CString(b->getValue(1));
                    CAddAttribute(channelNode, "inBandwidth", strValue);
                    strValue = CString(b->getValue(2));
                    CAddAttribute(channelNode, "outBandwidth", strValue);
                }
            }
        }
    }

    // Connections
    for (const auto& c: g->getConnections()) {
        // Connection
        CNode *connectionNode = CAddNode(mappingNode, "connection");
        CAddAttribute(connectionNode, "name", c->getName());

        // Bindings present?
        if (c->getChannelBindings() != nullptr) {
            // Channels mapped to connection
            for (auto& b: *(c->getChannelBindings())) {
                CNode* channelNode = CAddNode(connectionNode, "channel");
                CAddAttribute(channelNode, "name", b->getComponent()->getName());
            }
        }
    }

    return mappingNode;
}

/**
 * outputBindingAsXML ()
 * Output the binding of an SDFG to an platform graph in XML
 * format.
 */
void outputBindingAsXML(PlatformGraph *g, SDFgraph *appGraph, std::ostream &out) {
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // Mapping node
    CAddNode(sdf3Node, createMappingNode(g, appGraph));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}

namespace {

/**
 * createTileUsageNode ()
 * Create an XML node which describes the resource usage of a tile in the
 * platform graph.
 */
CNode *createTileUsageNode(Tile *t) {
    // Tile node
    CNode *tileNode = CNewNode("tile");
    CAddAttribute(tileNode, "name", t->getName());

    // Processor
    Processor *p = t->getProcessor();
    if (p != nullptr) {
        CNode *procNode = CAddNode(tileNode, "processor");
        CAddAttribute(procNode, "name", p->getName());
        CAddAttribute(procNode,
                      "timeSlice",
                      CString(p->getTimewheelSize() - p->availableTimewheelSize()));
    }

    // Memory
    Memory *m = t->getMemory();
    if (m != nullptr) {
        CNode *memNode = CAddNode(tileNode, "memory");
        CAddAttribute(memNode, "name", m->getName());
        CAddAttribute(memNode, "size", CString(m->getSize() - m->availableMemorySize()));
    }

    // Network interface
    NetworkInterface *ni = t->getNetworkInterface();
    if (ni != nullptr) {
        CNode *niNode = CAddNode(tileNode, "networkInterface");
        CAddAttribute(niNode, "name", ni->getName());
        CAddAttribute(niNode,
                      "nrConnections",
                      CString(ni->getNrConnections() - ni->availableNrConnections()));
        CAddAttribute(
                niNode, "inBandwidth", CString(ni->getInBandwidth() - ni->availableInBandwidth()));
        CAddAttribute(niNode,
                      "outBandwidth",
                      CString(ni->getOutBandwidth() - ni->availableOutBandwidth()));
    }

    return tileNode;
}

/**
 * createTileNode ()
 * Create an XML node which describes the tile t in an platform graph.
 */
CNode *createTileNode(Tile *t) {
    // Tile node
    CNode* tileNode = CNewNode("tile");
    CAddAttribute(tileNode, "name", t->getName());

    // Processor
    Processor *p = t->getProcessor();
    if (p != nullptr) {
        CNode* procNode = CAddNode(tileNode, "processor");
        CAddAttribute(procNode, "name", p->getName());
        CAddAttribute(procNode, "type", p->getType());
        CNode* arbiterNode = CAddNode(procNode, "arbitration");
        CAddAttribute(arbiterNode, "type", "TDMA");
        CAddAttribute(arbiterNode, "wheelsize", CString(p->getTimewheelSize()));
    }

    // Memory
    Memory *m = t->getMemory();
    if (m != nullptr) {
        CNode* memNode = CAddNode(tileNode, "memory");
        CAddAttribute(memNode, "name", m->getName());
        CAddAttribute(memNode, "size", CString(m->getSize()));
    }

    // Network interface
    NetworkInterface *ni = t->getNetworkInterface();
    if (ni != nullptr) {
        CNode* niNode = CAddNode(tileNode, "networkInterface");
        CAddAttribute(niNode, "name", ni->getName());
        CAddAttribute(niNode, "nrConnections", CString(ni->getNrConnections()));
        CAddAttribute(niNode, "inBandwidth", CString(ni->getInBandwidth()));
        CAddAttribute(niNode, "outBandwidth", CString(ni->getOutBandwidth()));
    }

    return tileNode;
}

/**
 * createConnectionNode ()
 * Create an XML node which describes the connection c in an platform graph.
 */
CNode *createConnectionNode(Connection *c) {
    // Connection node
    CNode *connNode = CNewNode("connection");
    CAddAttribute(connNode, "name", c->getName());
    CAddAttribute(connNode, "srcTile", c->getSrcTile()->getName());
    CAddAttribute(connNode, "dstTile", c->getDstTile()->getName());
    CAddAttribute(connNode, "delay", CString(c->getLatency()));

    return connNode;
}


} // namespace

/**
 * createSystemUsageNode ()
 * The function constructs a system usage node in XML format.
 */
CNode *createSystemUsageNode(PlatformGraph *g) {
    CNode *usageNode = CNewNode("systemUsage");
    CAddAttribute(usageNode, "archGraph", g->getName());

    for (const auto& t: g->getTiles()) {
        CAddNode(usageNode, createTileUsageNode(t.get()));
    }

    return usageNode;
}

/**
 * outputSystemUsageAsXML ()
 * Output the usage of an platform graph in XML format.
 */
void outputSystemUsageAsXML(PlatformGraph *g, std::ostream &out) {
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // System usage node
    CAddNode(sdf3Node, createSystemUsageNode(g));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}


/**
 * createPlatformGraphNode ()
 * Create an XML node which describes the platform graph.
 */
CNode *createPlatformGraphNode(PlatformGraph *g) {
    // Architecture graph node
    CNode *archNode = CNewNode("architectureGraph");
    CAddAttribute(archNode, "name", g->getName());

    // Tiles
    for (const auto& t: g->getTiles()) {
        CAddNode(archNode, createTileNode(t.get()));
    }

    // Connections
    for (const auto& c: g->getConnections()) {
        CAddNode(archNode, createConnectionNode(c.get()));
    }

    return archNode;
}

/**
 * outputPlatformGraphAsXML ()
 * Output the platform graph in XML format.
 */
void outputPlatformGraphAsXML(PlatformGraph *g, std::ostream &out) {
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // Architecture graph node
    CAddNode(sdf3Node, createPlatformGraphNode(g));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}
} // namespace SDF