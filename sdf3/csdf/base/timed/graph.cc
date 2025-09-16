/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   timed_graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Timed CSDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.cc,v 1.3 2008-03-22 14:24:21 sander Exp $
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
#include <memory>
#include <string>
namespace CSDF {
/**
 * TimedCSDFgraph ()
 * Constructor.
 */
TimedCSDFgraph::TimedCSDFgraph(CSDFcomponent &c) : CSDFgraph(c) {}

/**
 * ~TimedCSDFgraph ()
 * Destructor.
 */
TimedCSDFgraph::~TimedCSDFgraph() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 */
std::unique_ptr<CSDFgraph> TimedCSDFgraph::create(CSDFcomponent &c) const {
    return std::make_unique<TimedCSDFgraph>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
std::unique_ptr<CSDFgraph> TimedCSDFgraph::clone(CSDFcomponent &c) const {
    std::unique_ptr<CSDFgraph> g = createCopy(c);

    // Actors
    for (auto iter = actorsBegin(); iter != actorsEnd(); iter++) {
        CSDFcomponent component = CSDFcomponent(g.get(), g->nrActors());
        std::unique_ptr<CSDFactor> a = (*iter)->clone(component);
        g->addActor(std::move(a));
    }

    // Channels
    for (auto iter = channelsBegin(); iter != channelsEnd(); iter++) {
        CSDFcomponent component = CSDFcomponent(g.get(), g->nrChannels());
        std::unique_ptr<CSDFchannel> ch = (*iter)->clone(component);
        g->addChannel(std::move(ch));
    }

    return g;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
std::unique_ptr<TimedCSDFgraph> TimedCSDFgraph::clone() const {
    CSDFcomponent comp = CSDFcomponent(getParent(), getId(), getName());
    auto cl = clone(comp);
    auto* rawPtr = dynamic_cast<TimedCSDFgraph*>(cl.release());
    std::unique_ptr<TimedCSDFgraph> tcl(rawPtr);
    return tcl;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
std::unique_ptr<CSDFgraph> TimedCSDFgraph::createCopy(CSDFcomponent &c) const {
    auto g = std::make_unique<TimedCSDFgraph>(c);

    // Properties
    g->setName(getName());
    g->setType(getType());

    return g;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
TimedCSDFactor* TimedCSDFgraph::createActor() {
    CSDFcomponent c = CSDFcomponent(this, nrActors());
    auto a = std::make_unique<TimedCSDFactor>(c);
    a->setName(CString("_a") + CString(nrActors() + 1));
    auto* ar = a.get();
    addActor(std::move(a));
    return ar;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
TimedCSDFactor *TimedCSDFgraph::createActor(CSDFcomponent &c) {
    auto a = std::make_unique<TimedCSDFactor>(c);
    auto* ar = a.get();
    addActor(std::move(a));

    return ar;
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
TimedCSDFchannel *TimedCSDFgraph::createChannel(CSDFcomponent &c) {
    auto ch = std::make_unique<TimedCSDFchannel>(c);
    auto* chr = ch.get();
    addChannel(std::move(ch));

    return chr;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
TimedCSDFchannel *TimedCSDFgraph::createChannel(CSDFactor *src,
                                                const CSDFrate &rateSrc,
                                                CSDFactor *dst,
                                                const CSDFrate &rateDst) {
    // Create new channel
    CSDFcomponent c = CSDFcomponent(this, nrChannels());
    TimedCSDFchannel *ch = createChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));

    // Create ports on the actors
    CSDFport *srcP = src->createPort("out", rateSrc);
    srcP->setName(CString("_p") + CString(src->nrPorts()));
    CSDFport *dstP = dst->createPort("in", rateDst);
    dstP->setName(CString("_p") + CString(dst->nrPorts()));

    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    return ch;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
std::ostream &TimedCSDFgraph::print(std::ostream &out) const {
    out << "Graph (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;
    out << std::endl;

    for (auto iter = actorsBegin(); iter != actorsEnd(); iter++) {
        const CSDFactor& a = *(*iter);

        (dynamic_cast<const TimedCSDFactor&>(a)).print(out);
    }

    for (auto iter = channelsBegin(); iter != channelsEnd(); iter++) {
        const CSDFchannel& c = *(*iter);

        (dynamic_cast<const TimedCSDFchannel&>(c)).print(out);
    }

    out << std::endl;

    return out;
}

std::unique_ptr<TimedCSDFgraph> TimedCSDFgraph::dynamicCast(std::unique_ptr<CSDFgraph> g) {
    return std::unique_ptr<TimedCSDFgraph>(dynamic_cast<TimedCSDFgraph*>(g.release()));
}

namespace {

/**
 * actorProperties ()
 * Set the properties of a timed actor.
 */
void actorProperties(TimedCSDFactor *a, const CNodePtr propertiesNode) {
    TimedCSDFactor::Processor *p = nullptr;
    CStrings execTimes;

    // Processor definitions
    for (CNode *procNode = CGetChildNode(propertiesNode, "processor"); procNode != nullptr;
         procNode = CNextNode(procNode, "processor")) {
        if (!CHasAttribute(procNode, "type")) {
            throw CException("Processor must have a type");
        }

        // Add a processor to the actor
        p = a->addProcessor(CGetAttribute(procNode, "type"));

        // Execution time
        if (CHasChildNode(procNode, "executionTime")) {
            CNode *execTimeNode = CGetChildNode(procNode, "executionTime");

            if (!CHasAttribute(execTimeNode, "time")) {
                throw CException("Execution time not specified");
            }

            p->execTime = CSDFtimeSequence(CGetAttribute(execTimeNode, "time"));

            // Is the length of the execution time sequence equal to the
            // sequenceLength of the actor?
            if (p->execTime.size() != a->sequenceLength()) {
                throw CException("[ERROR] length of execution time sequences "
                                 "not equal.");
            }
        }

        // Memory
        if (CHasChildNode(procNode, "memory")) {
            CNode *memoryNode = nullptr;
            memoryNode = CGetChildNode(procNode, "memory");

            if (CHasChildNode(memoryNode, "stateSize")) {
                CNode *stateSizeNode = nullptr;
                stateSizeNode = CGetChildNode(memoryNode, "stateSize");

                if (!CHasAttribute(stateSizeNode, "max")) {
                    throw CException("No maximum state size given.");
                }

                int sz = std::stoi((CGetAttribute(stateSizeNode, "max")));
                p->stateSize = sz;
            }
        }

        // Default processor?
        if (CHasAttribute(procNode, "default")) {
            a->setDefaultProcessor(p->type);
        }
    }
}


/**
 * channelProperties ()
 * Set the properties of a timed channel.
 */
void channelProperties(TimedCSDFchannel *c, const CNodePtr propertiesNode) {
    // Buffer size
    if (CHasChildNode(propertiesNode, "bufferSize")) {
        CNode *bufferSizeNode = CGetChildNode(propertiesNode, "bufferSize");
        CSDFbufferSize bufferSize;

        bufferSize.sz = -1;
        bufferSize.src = -1;
        bufferSize.dst = -1;
        bufferSize.mem = -1;

        if (CHasAttribute(bufferSizeNode, "sz")) {
            bufferSize.sz = std::stoi(CGetAttribute(bufferSizeNode, "sz"));
        }

        if (CHasAttribute(bufferSizeNode, "src")) {
            bufferSize.src = std::stoi(CGetAttribute(bufferSizeNode, "src"));
        }

        if (CHasAttribute(bufferSizeNode, "dst")) {
            bufferSize.dst = std::stoi(CGetAttribute(bufferSizeNode, "dst"));
        }

        if (CHasAttribute(bufferSizeNode, "mem")) {
            bufferSize.mem = std::stoi(CGetAttribute(bufferSizeNode, "mem"));
        }

        c->setBufferSize(bufferSize);
    }

    // Token size
    if (CHasChildNode(propertiesNode, "tokenSize")) {
        CNode *tokenSizeNode = CGetChildNode(propertiesNode, "tokenSize");

        if (CHasAttribute(tokenSizeNode, "sz")) {
            int sz = std::stoi(CGetAttribute(tokenSizeNode, "sz"));
            c->setTokenSize(sz);
        }
    }

    // Token type
    if (CHasChildNode(propertiesNode, "tokenType")) {
        CNode *tokenTypeNode = CGetChildNode(propertiesNode, "tokenType");

        if (CHasAttribute(tokenTypeNode, "type")) {
            CString type = CGetAttribute(tokenTypeNode, "type");
            c->setTokenType(type);
        }
    }

    // Bandwidth
    if (CHasChildNode(propertiesNode, "bandwidth")) {
        CNode *bwNode = CGetChildNode(propertiesNode, "bandwidth");

        if (CHasAttribute(bwNode, "min")) {
            double sz = std::stod(CGetAttribute(bwNode, "min"));
            c->setMinBandwidth(sz);
        }
    }

    // Latency
    if (CHasChildNode(propertiesNode, "latency")) {
        CNode *latencyNode = CGetChildNode(propertiesNode, "latency");

        if (CHasAttribute(latencyNode, "min")) {
            int sz = std::stoi(CGetAttribute(latencyNode, "min"));
            c->setMinLatency(sz);
        }
    }
}


/**
 * graphProperties ()
 * Set the properties of a timed graph.
 */
void graphProperties(TimedCSDFgraph *g, const CNodePtr propertiesNode) {
    // Timing constraints
    for (CNode *timeNode = CGetChildNode(propertiesNode, "timeConstraints"); timeNode != nullptr;
         timeNode = CNextNode(timeNode, "timeConstraints")) {
        if (CHasChildNode(timeNode, "throughput")) {
            CNode *throughputNode = nullptr;
            throughputNode = CGetChildNode(timeNode, "throughput");

            CString throughput = CGetNodeContent(throughputNode);

            g->setThroughputConstraint(static_cast<CSDFthroughput>(throughput));
        }
    }
}

/**
 * constructTimedSDFgraphStructure ()
 * Construct a timed SDF graph.
 */
std::unique_ptr<TimedCSDFgraph> constructTimedCSDFgraphStructure(const CNodePtr csdfNode) {
    CId actorId = 0;
    CId channelId = 0;
    CId portId = 0;

    // Construct CSDF graph
    CSDFcomponent component = CSDFcomponent(nullptr, 0);
    auto g = std::make_unique<TimedCSDFgraph>(component);
    g->construct(csdfNode);

    // Construct all actors
    for (CNodePtr actorNode = CGetChildNode(csdfNode, "actor"); actorNode != nullptr;
         actorNode = CNextNode(actorNode, "actor")) {
        // Construct actor
        component = CSDFcomponent(g.get(), actorId);
        auto a = std::make_unique<TimedCSDFactor>(component);
        a->construct(actorNode);

        // Construct all ports
        portId = 0;
        for (CNodePtr portNode = CGetChildNode(actorNode, "port"); portNode != nullptr;
             portNode = CNextNode(portNode, "port")) {
            // Construct port
            component = CSDFcomponent(a.get(), portId);
            auto p = std::make_unique<CSDFport>(component);
            p->construct(portNode);

            // Add port to actor
            a->addPort(std::move(p));

            // Next port
            portId++;
        }

        // check port consistency
        if (a->getPorts().size() > 1) {
            uint seqLength = static_cast<uint>((*(a->getPorts().begin()))->getRate().size());
            for (auto i = a->getConstPorts().begin(); i != a->getConstPorts().end(); i++) {
                const CSDFport& p = *(*i);
                if (p.getRate().size() != seqLength) {
                    throw CException("Sequences of port rates of different lengths in actor "
                                     + a->getName());
                }
            }
        }

        // Add actor to graph
        g->addActor(std::move(a));

        // Next actor
        actorId++;
    }

    // Construct all channels
    for (CNodePtr channelNode = CGetChildNode(csdfNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {
        // Construct channel
        component = CSDFcomponent(g.get(), channelId);
        auto c = std::make_unique<TimedCSDFchannel>(component);
        c->construct(channelNode);

        // Add channel to graph
        g->addChannel(std::move(c));

        // Next channel
        channelId++;
    }

    return g;
}

} // namespace

/**
 * constructTimedSDFgraph ()
 * Construct a timed SDF graph.
 */
std::unique_ptr<TimedCSDFgraph> constructTimedCSDFgraph(const CNodePtr csdfNode,
                                                        const CNodePtr csdfPropertiesNode) {

    // Create graph structure
    std::unique_ptr<TimedCSDFgraph> g = constructTimedCSDFgraphStructure(csdfNode);

    // Any properties to be set?
    if (csdfPropertiesNode == nullptr) {
        return std::move(g);
    }

    // Set all properties
    for (CNode *propertiesNode = CGetChildNode(csdfPropertiesNode); propertiesNode != nullptr;
         propertiesNode = CNextNode(propertiesNode)) {
        if (CIsNode(propertiesNode, "actorProperties")) {
            if (!CHasAttribute(propertiesNode, "actor")) {
                throw CException("Missing 'actor' in 'actorProperties'");
            }

            TimedCSDFactor *a = nullptr;
            a = dynamic_cast<TimedCSDFactor *>(g->getActor(CGetAttribute(propertiesNode, "actor")));

            actorProperties(a, propertiesNode);
        } else if (CIsNode(propertiesNode, "channelProperties")) {
            if (!CHasAttribute(propertiesNode, "channel")) {
                throw CException("Missing 'channel' in 'channelProperties'");
            }

            TimedCSDFchannel *c = nullptr;
            c = dynamic_cast<TimedCSDFchannel *>(
                    g->getChannel(CGetAttribute(propertiesNode, "channel")));

            channelProperties(c, propertiesNode);
        } else if (CIsNode(propertiesNode, "graphProperties")) {
            graphProperties(g.get(), propertiesNode);
        }
    }

    return g;
}
} // namespace CSDF
