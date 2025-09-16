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
 * $Id: graph.cc,v 1.2.2.1 2010-04-25 02:08:51 mgeilen Exp $
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
#include "tile.h"
#include <memory>
namespace SDF {
/**
 * PlatformGraph ()
 * Constructor.
 */
PlatformGraph::PlatformGraph(ArchComponent &c) : ArchComponent(c) {}

/**
 * ~PlatformGraph ()
 * Destructor.
 */
PlatformGraph::~PlatformGraph() = default;

/**
 * getTile ()
 * The function returns a pointer to a tile with the given id or nullptr if
 * the tile does not exist.
 */
const Tile *PlatformGraph::getTile(const CId id) const {
    for (const auto &t : tiles) {
        if (t->getId() == id) {
            return t.get();
        }
    }

    return nullptr;
}

/**
 * getTile ()
 * The function returns a pointer to a tile with the given id or nullptr if
 * the tile does not exist.
 */
Tile *PlatformGraph::getTile(const CId id) {
    for (auto &t : tiles) {
        if (t->getId() == id) {
            return t.get();
        }
    }

    return nullptr;
}

/**
 * getTile ()
 * The function returns a pointer to a tile with the given name or nullptr if
 * the tile does not exist.
 */
Tile *PlatformGraph::getTile(const CString &name) {
    for (auto &t : tiles) {
        if (t->getName() == name) {
            return t.get();
        }
    }

    return nullptr;
}

/**
 * getTile ()
 * The function returns a pointer to a tile with the given name or nullptr if
 * the tile does not exist.
 */
const Tile *PlatformGraph::getTile(const CString &name) const {
    for (const auto &t : tiles) {
        if (t->getName() == name) {
            return t.get();
        }
    }

    return nullptr;
}

TileRefs PlatformGraph::getTileRefs() const {
    TileRefs result;
    for (const auto& t: this->tiles) {
        result.push_back(t.get());
    }
    return result;
}


/**
 * createTile ()
 * The adds a tile to the platform graph and returns a pointer to the
 * newly constructed tile.
 */
Tile *PlatformGraph::createTile(const CString &name) {
    ArchComponent component(this, static_cast<CId>(tiles.size()), name);

    // Create new tile
    auto t = std::make_unique<Tile>(component);

    auto *tileRef = t.get();

    // Add tile to graph
    tiles.push_back(std::move(t));

    return tileRef;
}

/**
 * getConnections ()
 * The function returns a list of all connections from the source to
 * the destination tile.
 */
ConnectionRefs PlatformGraph::getConnections(const Tile *srcTile, const Tile *dstTile) {
    ConnectionRefs connections;

    for (auto iter = srcTile->outConnectionsBegin();
         iter != srcTile->outConnectionsEnd();
         iter++) {
        Connection* c = *iter;

        if (c->getDstTile() == dstTile) {
            connections.push_back(c);
        }
    }

    return connections;
}

/**
 * getConnection ()
 * The function returns the first connection  it finds from the source to
 * the destination tile.
 */
Connection *PlatformGraph::getConnection(const Tile *srcTile, const Tile *dstTile) {
    for (auto iter = srcTile->outConnectionsBegin();
         iter != srcTile->outConnectionsEnd();
         iter++) {
        Connection* c = *iter;

        if (c->getDstTile() == dstTile) {
            return c;
        }
    }

    return nullptr;
}

/**
 * getConnection ()
 * The function returns a pointer to a connection with the given id or nullptr if
 * the connection does not exist.
 */
Connection *PlatformGraph::getConnection(const CId id) {
    if (id >= connections.size()) {
        return nullptr;
    }

    return connections[id].get();
}

/**
 * getConnection ()
 * The function returns a pointer to a connection with the given name or nullptr if
 * the connection does not exist.
 */
Connection *PlatformGraph::getConnection(const CString &name) {
    for (ConnectionsCIter iter = connectionsBegin(); iter != connectionsEnd(); iter++) {
        const std::unique_ptr<Connection> &c = *iter;

        if (c->getName() == name) {
            return c.get();
        }
    }

    return nullptr;
}

/**
 * createConnection ()
 * The adds a connection to the platform graph and returns a pointer to the
 * newly constructed connection.
 */
Connection *PlatformGraph::createConnection(const CString &name) {
    ArchComponent component(this, static_cast<CId>(connections.size()), name);

    // Create new connection
    auto c = std::make_unique<Connection>(component);

    auto *connRef = c.get();

    // Add connection to graph
    connections.push_back(std::move(c));

    return connRef;
}

/**
 * constructPlatformGraph ()
 * Construct an platform graph.
 */
std::unique_ptr<PlatformGraph> constructPlatformGraph(const CNodePtr archNode) {

    // Graph
    if (!CHasAttribute(archNode, "name")) {
        throw CException("Architecture graph must have a name");
    }
    CString name = CGetAttribute(archNode, "name");
    ArchComponent component = ArchComponent(nullptr, 0, name);
    auto g = std::make_unique<PlatformGraph>(component);

    // Tiles
    for (CNode *tileNode = CGetChildNode(archNode, "tile"); tileNode != nullptr;
         tileNode = CNextNode(tileNode, "tile")) {
        if (!CHasAttribute(tileNode, "name")) {
            throw CException("Tile must have a name");
        }
        name = CGetAttribute(tileNode, "name");

        // Create tile
        Tile *t = g->createTile(name);

        // Processor
        CNode *procNode = CGetChildNode(tileNode, "processor");
        if (procNode != nullptr) {
            if (!CHasAttribute(procNode, "name")) {
                throw CException("Processor must have a name.");
            }
            name = CGetAttribute(procNode, "name");
            Processor *p = t->createProcessor(name);
            if (!CHasAttribute(procNode, "type")) {
                throw CException("Processor must have a type.");
            }
            p->setType(CGetAttribute(procNode, "type"));
            if (!CHasChildNode(procNode, "arbitration")) {
                throw CException("Processor must have arbitration mechanism.");
            }
            CNode *arbiterNode = CGetChildNode(procNode, "arbitration");
            if (CGetAttribute(arbiterNode, "type") != "TDMA") {
                throw CException("Only TDMA arbitration supported.");
            }
            if (!CHasAttribute(arbiterNode, "wheelsize")) {
                throw CException("TDMA arbitration needs wheelsize.");
            }
            p->setTimewheelSize(static_cast<CSize>(CGetAttribute(arbiterNode, "wheelsize")));
        }

        // Memory
        CNode *memNode = CGetChildNode(tileNode, "memory");
        if (memNode != nullptr) {
            if (!CHasAttribute(memNode, "name")) {
                throw CException("Memory must have a name.");
            }
            name = CGetAttribute(memNode, "name");
            Memory *m = t->createMemory(name);
            if (!CHasAttribute(memNode, "size")) {
                throw CException("Memory must have a size.");
            }
            m->setSize(static_cast<CSize>(CGetAttribute(memNode, "size")));
        }

        // Network interface
        CNode *niNode = CGetChildNode(tileNode, "networkInterface");
        if (niNode != nullptr) {
            if (!CHasAttribute(niNode, "name")) {
                throw CException("Network interface  must have a name.");
            }
            name = CGetAttribute(niNode, "name");
            NetworkInterface *ni = t->createNetworkInterface(name);
            if (!CHasAttribute(niNode, "nrConnections")) {
                throw CException("Network interface must have nrConnections.");
            }
            CSize nrConnections = static_cast<CSize>(CGetAttribute(niNode, "nrConnections"));
            if (!CHasAttribute(niNode, "inBandwidth")) {
                throw CException("Network interface must have inBandwidth.");
            }
            CSize inBandwidth = static_cast<CSize>(CGetAttribute(niNode, "inBandwidth"));
            if (!CHasAttribute(niNode, "outBandwidth")) {
                throw CException("Network interface must have outBandwidth.");
            }
            CSize outBandwidth = static_cast<CSize>(CGetAttribute(niNode, "outBandwidth"));
            ni->setConnections(nrConnections,
                               static_cast<double>(inBandwidth),
                               static_cast<double>(outBandwidth));
        }
    }

    // Connections
    for (CNode *connectionNode = CGetChildNode(archNode, "connection"); connectionNode != nullptr;
         connectionNode = CNextNode(connectionNode, "connection")) {
        // Create connection
        if (!CHasAttribute(connectionNode, "name")) {
            throw CException("Connection must have a name");
        }
        name = CGetAttribute(connectionNode, "name");
        Connection *c = g->createConnection(name);

        // Set properties
        if (!CHasAttribute(connectionNode, "delay")) {
            throw CException("Connection must have a delay");
        }
        c->setLatency(static_cast<TTime>(CGetAttribute(connectionNode, "delay")));

        // Connect it to source tile
        if (!CHasAttribute(connectionNode, "srcTile")) {
            throw CException("Connection must have a srcTile");
        }
        Tile *t = g->getTile(CGetAttribute(connectionNode, "srcTile"));
        if (t == nullptr) {
            throw CException("srcTile does not exist.");
        }
        c->setSrcTile(t);
        t->addOutConnection(c);

        // Connect it to destination tile
        if (!CHasAttribute(connectionNode, "dstTile")) {
            throw CException("Connection must have a dstTile");
        }
        t = g->getTile(CGetAttribute(connectionNode, "dstTile"));
        if (t == nullptr) {
            throw CException("dstTile does not exist.");
        }
        c->setDstTile(t);
        t->addInConnection(c);
    }

    return g;
}

/**
 * setUsagePlatformGraph ()
 * Reserve resource in platform graph.
 */
void setUsagePlatformGraph(PlatformGraph* g, const CNodePtr archNode) {

    // Tiles
    for (CNode *tileNode = CGetChildNode(archNode, "tile"); tileNode != nullptr;
         tileNode = CNextNode(tileNode, "tile")) {
        if (!CHasAttribute(tileNode, "name")) {
            throw CException("Tile must have a name");
        }
        CString name = CGetAttribute(tileNode, "name");

        // Find tile in architecture graph
        Tile *t = g->getTile(name);
        if (t == nullptr) {
            throw CException("Cannot set system usage. Tile does not exist.");
        }

        // Processor
        if (CHasChildNode(tileNode, "processor")) {
            // Processor in graph
            Processor *p = t->getProcessor();
            if (p == nullptr) {
                throw CException("Tile contains no processor.");
            }

            // Set used time slice
            CNode *procNode = CGetChildNode(tileNode, "processor");
            if (!CHasAttribute(procNode, "timeSlice")) {
                throw CException("Processor must specify used timeSlice");
            }
            p->setOccupiedTimeSlice(static_cast<CSize>(CGetAttribute(procNode, "timeSlice")));
        }

        // Memory
        if (CHasChildNode(tileNode, "memory")) {
            // Memory in graph
            Memory *m = t->getMemory();
            if (m == nullptr) {
                throw CException("Tile contains no memory.");
            }

            // Set used memory size
            CNode *memNode = CGetChildNode(tileNode, "memory");
            if (!CHasAttribute(memNode, "size")) {
                throw CException("Memory must specify used size");
            }
            m->reserveMemory(static_cast<CSize>(CGetAttribute(memNode, "size")));
        }

        // Network interface
        if (CHasChildNode(tileNode, "networkInterface")) {
            // Memory in graph
            NetworkInterface *ni = t->getNetworkInterface();
            if (ni == nullptr) {
                throw CException("Tile contains no network interface.");
            }

            // Set used connections and bandwidth
            CNode *niNode = CGetChildNode(tileNode, "networkInterface");
            if (!CHasAttribute(niNode, "nrConnections")) {
                throw CException("NI must specify used nrConnections");
            }
            CSize nrConnections = static_cast<CSize>(CGetAttribute(niNode, "nrConnections"));
            if (!CHasAttribute(niNode, "inBandwidth")) {
                throw CException("NI must specify used inBandwidth");
            }
            CSize inBandwidth = static_cast<CSize>(CGetAttribute(niNode, "inBandwidth"));
            if (!CHasAttribute(niNode, "outBandwidth")) {
                throw CException("NI must specify used outBandwidth");
            }
            CSize outBandwidth = static_cast<CSize>(CGetAttribute(niNode, "outBandwidth"));
            ni->reserveConnection(nullptr,
                                  nrConnections,
                                  static_cast<double>(inBandwidth),
                                  static_cast<double>(outBandwidth));
        }
    }
}

namespace{

/**
 * setMappingProcessor ()
 * The function sets the mapping of the application graph to the processor.
 */
void setMappingProcessor(Processor *p, TimedSDFgraph *g, const CNodePtr procNode) {
    StaticOrderSchedule s;

    if (p == nullptr) {
        throw CException("[ERROR] processor does not exist.");
    }

    // Reserve timeslice on processor
    if (!p->reserveTimeSlice(static_cast<CSize>(CGetAttribute(procNode, "timeslice")))) {
        throw CException("[ERROR] failed allocating timeslice on processor.");
    }

    // Iterate over all actors bound to the processor
    for (CNode *actorNode = CGetChildNode(procNode, "actor"); actorNode != nullptr;
         actorNode = CNextNode(actorNode, "actor")) {
        TimedSDFactor *a =
                dynamic_cast<TimedSDFactor *>(g->getActor(CGetAttribute(actorNode, "name")));

        if (a == nullptr) {
            throw CException("[ERROR] actor does not exist.");
        }

        if (!p->bindActor(a)) {
            throw CException("[ERROR] failed binding actor to processor.");
        }
    }

    // Construct schedule and assign it to the processor
    if (CHasChildNode(procNode, "schedule")) {
        CNode *scheduleNode = CGetChildNode(procNode, "schedule");

        // Construct states of the schedule
        for (CNode *stateNode = CGetChildNode(scheduleNode, "state"); stateNode != nullptr;
             stateNode = CNextNode(stateNode, "state")) {
            // Add the actor to the schedule
            s.appendActor(g->getActor(CGetAttribute(stateNode, "actor")));

            // Is this schedule entry the start of the periodic regime?
            if (CHasAttribute(stateNode, "startOfPeriodicRegime")
                && CGetAttribute(stateNode, "startOfPeriodicRegime") == "true") {
                s.setStartPeriodicSchedule(static_cast<uint>(s.size()) - 1);
            }
        }

        // Link schedule to processor
        p->setSchedule(s);
    }
}

/**
 * setMappingMemory ()
 * The function sets the mapping of the application graph to the memory.
 */
void setMappingMemory(Memory *m, TimedSDFgraph *g, const CNodePtr memNode) {
    if (m == nullptr) {
        throw CException("[ERROR] memory does not exist.");
    }

    // Iterate over all channels bound to the memory
    for (CNode *channelNode = CGetChildNode(memNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {
        SDFchannel *ch = g->getChannel(CGetAttribute(channelNode, "name"));

        if (ch == nullptr) {
            throw CException("[ERROR] channel does not exist.");
        }

        if (!m->reserveMemory(ch, static_cast<CSize>(CGetAttribute(channelNode, "size")))) {
            throw CException("[ERROR] failed binding channel to memory.");
        }
    }

    // Iterate over all actors bound to the memory
    for (CNode *actorNode = CGetChildNode(memNode, "actor"); actorNode != nullptr;
         actorNode = CNextNode(actorNode, "actor")) {
        SDFactor *a = g->getActor(CGetAttribute(actorNode, "name"));

        if (a == nullptr) {
            throw CException("[ERROR] actor does not exist.");
        }

        if (!m->reserveMemory(a, static_cast<CSize>(CGetAttribute(actorNode, "size")))) {
            throw CException("[ERROR] failed binding actor to memory.");
        }
    }
}


/**
 * setMappingNetworkInterface ()
 * The function sets the mapping of the application graph to the NI.
 */
void setMappingNetworkInterface(NetworkInterface *ni, TimedSDFgraph *g, const CNodePtr niNode) {
    if (ni == nullptr) {
        throw CException("[ERROR] network interface does not exist.");
    }

    // Iterate over all channels bound to the network interface
    for (CNode *channelNode = CGetChildNode(niNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {
        TimedSDFchannel *ch =
                dynamic_cast<TimedSDFchannel *>(g->getChannel(CGetAttribute(channelNode, "name")));

        if (ch == nullptr) {
            throw CException("[ERROR] channel does not exist.");
        }

        if (!ni->reserveConnection(
                    ch,
                    static_cast<CSize>(CGetAttribute(channelNode, "nrConnections")),
                    static_cast<CDouble>(CGetAttribute(channelNode, "inBandwidth")),
                    static_cast<CDouble>(CGetAttribute(channelNode, "outBandwidth")))) {
            throw CException("[ERROR] failed binding channel to network "
                             "interface.");
        }
    }
}


/**
 * setMappingTile ()
 * The function sets the mapping of the application graph to the tile.
 */
void setMappingTile(Tile *t, TimedSDFgraph *g, const CNodePtr tileNode) {

    if (t == nullptr) {
        throw CException("[ERROR] tile does not exist.");
    }

    // Processor
    if (CHasChildNode(tileNode, "processor")) {
        Processor *p = t->getProcessor();
        setMappingProcessor(p, g, CGetChildNode(tileNode, "processor"));
    }

    // Memory
    if (CHasChildNode(tileNode, "memory")) {
        Memory *m = t->getMemory();
        setMappingMemory(m, g, CGetChildNode(tileNode, "memory"));
    }

    // Network interface
    if (CHasChildNode(tileNode, "networkInterface")) {
        NetworkInterface *ni = t->getNetworkInterface();
        setMappingNetworkInterface(ni, g, CGetChildNode(tileNode, "networkInterface"));
    }
}

/**
 * setMappingConnection ()
 * The function sets the mapping of the application graph to the connection.
 */
void setMappingConnection(Connection *c, TimedSDFgraph *g, const CNodePtr connectionNode) {
    if (c == nullptr) {
        throw CException("[ERROR] connection does not exist.");
    }

    // Iterate over all channels bound to the connection
    for (CNode *channelNode = CGetChildNode(connectionNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {
        TimedSDFchannel *ch =
                dynamic_cast<TimedSDFchannel *>(g->getChannel(CGetAttribute(channelNode, "name")));

        if (ch == nullptr) {
            throw CException("[ERROR] channel does not exist.");
        }

        if (!c->bindChannel(ch)) {
            throw CException("[ERROR] failed binding channel to connection.");
        }
    }
}

} // namespace

/**
 * setMappingPlatformGraph ()
 * The function sets the mapping of the application graph to the platform
 * graph as specified in the mapping node.
 */
void setMappingPlatformGraph(const std::unique_ptr<PlatformGraph>& ar,
                             TimedSDFgraph *ap,
                             const CNodePtr mappingNode) {
    // Name of application graph does not match?
    if (CGetAttribute(mappingNode, "appGraph") != ap->getName()) {
        throw CException("[ERROR] Name of application graph does not match.");
    }

    // Name of architecture graph does not match?
    if (CGetAttribute(mappingNode, "archGraph") != ar->getName()) {
        throw CException("[ERROR] Name of architecture graph does not match.");
    }

    // Iterate over the tiles in the mapping node
    for (CNode *tileNode = CGetChildNode(mappingNode, "tile"); tileNode != nullptr;
         tileNode = CNextNode(tileNode, "tile")) {
        Tile *t = ar->getTile(CGetAttribute(tileNode, "name"));
        setMappingTile(t, ap, tileNode);
    }

    // Iterate over the connections in the mapping node
    for (CNode *connectionNode = CGetChildNode(mappingNode, "connection");
         connectionNode != nullptr;
         connectionNode = CNextNode(connectionNode, "connection")) {
        Connection *c = ar->getConnection(CGetAttribute(connectionNode, "name"));
        setMappingConnection(c, ap, connectionNode);
    }
}
} // namespace SDF