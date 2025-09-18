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
 *  Function        :   Output CSDF graph in XML format
 *
 *  History         :
 *      29-07-05    :   Initial version.
 *
 * $Id: xml.cc,v 1.4.2.1 2010-04-25 01:21:16 mgeilen Exp $
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
#include "sdf/base/timed/timed_types.h"

namespace CSDF {

namespace {

CNode *createCSDFnode(const CSDFgraph& g) {
    CNode *csdfNode = nullptr;

    // SDF node
    csdfNode = CNewNode("csdf");
    CAddAttribute(csdfNode, "name", g.getName());
    CAddAttribute(csdfNode, "type", g.getType());

    // Actors
    for (const auto& a : g.getConstActors()) {
        CNode *actorNode = nullptr;

        // Actor node
        actorNode = CAddNode(csdfNode, "actor");
        CAddAttribute(actorNode, "name", a->getName());
        CAddAttribute(actorNode, "type", a->getType());

        // Ports
        for (const auto& p : a->getPorts()) {
            CNode *portNode = nullptr;
            CString rateStr;
            std::queue<CId> counts;
            std::queue<CId> rates;

            // Port node
            portNode = CAddNode(actorNode, "port");
            CAddAttribute(portNode, "name", p->getName());
            CAddAttribute(portNode, "type", p->getTypeAsString());

            // Rate
            counts.push(1);
            rates.push(p->getRate().at(0));

            for (int i = 1; i != p->getRate().size(); i++) {
                if (p->getRate().at(i) != rates.back()) {
                    counts.push(1);
                    rates.push(p->getRate().at(i));
                } else {
                    counts.back()++;
                }
            }

            while (!counts.empty()) {
                if (!rateStr.empty()) {
                    rateStr = rateStr + ",";
                }
                if (counts.front() != 1) {
                    rateStr = rateStr + CString(counts.front()) + "*";
                }
                rateStr = rateStr + CString(rates.front());
                counts.pop();
                rates.pop();
            }

            CAddAttribute(portNode, "rate", rateStr);
        }
    }

    // Channels
    for (const auto& c : g.getConstChannels()) {
        CNode *channelNode = nullptr;

        // Channel node
        channelNode = CAddNode(csdfNode, "channel");
        CAddAttribute(channelNode, "name", c->getName());
        CAddAttribute(channelNode, "srcActor", c->getSrcActor()->getName());
        CAddAttribute(channelNode, "srcPort", c->getSrcPort()->getName());
        CAddAttribute(channelNode, "dstActor", c->getDstActor()->getName());
        CAddAttribute(channelNode, "dstPort", c->getDstPort()->getName());
        if (c->getInitialTokens() != 0) {
            CAddAttribute(channelNode, "initialTokens", CString(c->getInitialTokens()));
        }
    }

    return csdfNode;
}


CNode *createCSDFpropertiesNode(const TimedCSDFgraph& g) {
    CNode *csdfPropNode = nullptr;

    // CSDF properties node
    csdfPropNode = CNewNode("csdfProperties");

    // Actors
    for (const auto& iter : g.getConstActors()) {
        auto *a = dynamic_cast<TimedCSDFactor *>(iter.get());
        CNode *actorNode = nullptr;

        // Actor node
        actorNode = CAddNode(csdfPropNode, "actorProperties");
        CAddAttribute(actorNode, "actor", a->getName());

        // Processors
        for (auto iterP = a->processorsBegin(); iterP != a->processorsEnd(); iterP++) {
            const TimedCSDFactor::Processor& p = *(*iterP);
            CNode *procNode = nullptr;
            CNode *execTimeNode = nullptr;

            // Processor node
            procNode = CAddNode(actorNode, "processor");
            CAddAttribute(procNode, "type", p.type);

            // Default processor?
            if (a->getDefaultProcessor() == p.type) {
                CAddAttribute(procNode, "default", "true");
            }

            // Execution time
            if (p.execTime.at(0) != (SDFTIME_MAX)) {
                CString execTimeStr;
                std::queue<CId> counts;
                std::queue<CId> times;

                execTimeNode = CAddNode(procNode, "executionTime");

                counts.push(1);
                times.push(static_cast<CId>(p.execTime.at(0)));

                for (int i = 1; i != p.execTime.size(); i++) {
                    if (p.execTime.at(i) != times.back()) {
                        counts.push(1);
                        times.push(static_cast<CId>(p.execTime.at(i)));
                    } else {
                        counts.back()++;
                    }
                }

                while (!counts.empty()) {
                    if (!execTimeStr.empty()) {
                        execTimeStr = execTimeStr + ",";
                    }
                    if (counts.front() != 1) {
                        execTimeStr = execTimeStr + CString(counts.front()) + "*";
                    }
                    execTimeStr = execTimeStr + CString(times.front());

                    counts.pop();
                    times.pop();
                }

                CAddAttribute(execTimeNode, "time", execTimeStr);
            }

            // State size
            if (p.stateSize != CSIZE_MAX) {
                CNode *memoryNode = nullptr;
                CNode *stateSizeNode = nullptr;

                memoryNode = CAddNode(procNode, "memory");
                stateSizeNode = CAddNode(memoryNode, "stateSize");
                CAddAttribute(stateSizeNode, "max", CString(p.stateSize));
            }
        }
    }

    // Channels
    for (const auto& iter : g.getConstChannels()) {
        const auto& c = *dynamic_cast<TimedCSDFchannel *>(iter.get());
        CNode *channelNode = nullptr;
        CSDFbufferSize bufferSize;

        // Channel node
        channelNode = CAddNode(csdfPropNode, "channelProperties");
        CAddAttribute(channelNode, "channel", c.getName());

        // Buffer size
        bufferSize = c.getBufferSize();
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
        if (c.getTokenSize() != -1) {
            CNode *tokenSzNode = CAddNode(channelNode, "tokenSize");
            CAddAttribute(tokenSzNode, "sz", CString(c.getTokenSize()));
        }

        // Token type
        if (!c.getTokenType().empty()) {
            CNode *tokenTypeNode = CAddNode(channelNode, "tokenType");
            CAddAttribute(tokenTypeNode, "type", c.getTokenType());
        }

        // Bandwidth
        if (c.getMinBandwidth() != 0) {
            CNode *bwNode = CAddNode(channelNode, "bandwidth");
            CAddAttribute(bwNode, "min", CString(c.getMinBandwidth()));
        }

        // Latency
        if (c.getMinLatency() != 0) {
            CNode *latencyNode = CAddNode(channelNode, "latency");
            CAddAttribute(latencyNode, "min", CString(c.getMinLatency()));
        }
    }

    // Graph
    CNode *graphPropNode = nullptr;
    graphPropNode = CAddNode(csdfPropNode, "graphProperties");

    // Throughput constraint
    if (std::abs(g.getThroughputConstraint()) > CSDFthroughputEpsilon) {
        CNode *timeConstraintsNode = nullptr;
        CString thr = CString(g.getThroughputConstraint());
        timeConstraintsNode = CAddNode(graphPropNode, "timeConstraints");
        CAddNode(timeConstraintsNode, "throughput", thr);
    }

    return csdfPropNode;
}

} // namespace

/**
 * createApplicationGraphNode ()
 * The function returns a CSDF graph in XML format.
 */
CNode *createApplicationGraphNode(const TimedCSDFgraph& g) {
    // Application graph node
    CNode *appNode = CNewNode("applicationGraph");

    // CSDF node
    CAddNode(appNode, createCSDFnode(g));

    // CSDF properties node
    CAddNode(appNode, createCSDFpropertiesNode(g));

    return appNode;
}

/**
 * outputCSDFasXML ()
 * The function outputs a CSDF graph in XML format.
 */
void outputCSDFasXML(const TimedCSDFgraph& g, std::ostream &out) {
    // CSDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "csdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd");

    // Application graph node
    CNode *appNode = CAddNode(sdf3Node, "applicationGraph");

    // CSDF node
    CAddNode(appNode, createCSDFnode(g));

    // CSDF properties node
    CAddNode(appNode, createCSDFpropertiesNode(g));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}
} // namespace CSDF
