/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
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
 * $Id: graph.h,v 1.2 2008-02-29 15:24:49 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_GRAPH_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_GRAPH_H_INCLUDED

#include "connection.h"
#include "tile.h"
#include <memory>

namespace SDF {
#define ACTOR_NOT_BOUND CID_MAX
#define CHANNEL_NOT_BOUND CID_MAX

/**
 * PlatformGraph
 * Container for platform graph.
 */
class PlatformGraph : public ArchComponent {
public:
    // Constructor
    explicit PlatformGraph(ArchComponent &c);

    // Destructor
    ~PlatformGraph() override;

    // Rule of Five: default copy/move constructors and assignment operators
    PlatformGraph(const PlatformGraph&) = delete;
    PlatformGraph& operator=(const PlatformGraph&) = delete;
    PlatformGraph(PlatformGraph&&) = delete;
    PlatformGraph& operator=(PlatformGraph&&) = delete;

    // Tiles
    TilesIter tilesBegin() { return tiles.begin(); };
    [[nodiscard]] TilesCIter tilesBegin() const { return tiles.begin(); };
    TilesIter tilesEnd() { return tiles.end(); };
    [[nodiscard]] TilesCIter tilesEnd() const { return tiles.end(); };
    [[nodiscard]] CSize nrTiles() const { return tiles.size(); };
    [[nodiscard]] const TileCollection& getTiles() const { return tiles; };
    [[nodiscard]] TileRefs getTileRefs() const;
    [[nodiscard]] const Tile *getTile(CId id) const;
    [[nodiscard]] Tile *getTile(CId id);
    [[nodiscard]] const Tile *getTile(const CString &name) const;
    [[nodiscard]] Tile *getTile(const CString &name);
    [[nodiscard]] Tile *createTile(const CString &name);

    // Connections
    ConnectionsIter connectionsBegin() { return connections.begin(); };
    [[nodiscard]] ConnectionsCIter connectionsBegin() const { return connections.begin(); };
    ConnectionsIter connectionsEnd() { return connections.end(); };
    [[nodiscard]] ConnectionsCIter connectionsEnd() const { return connections.end(); };
    [[nodiscard]] const Connections& getConnections() const { return connections; };
    static ConnectionRefs getConnections(const Tile *srcTile, const Tile *dstTile);
    static Connection *getConnection(const Tile *srcTile, const Tile *dstTile);
    Connection *getConnection(CId id);
    Connection *getConnection(const CString &name);
    Connection *createConnection(const CString &name);

private:
    // Tiles
    TileCollection tiles;

    // Connections
    Connections connections;
};

using PlatformGraphs = std::list<PlatformGraph *>;
using PlatformGraphsIter = PlatformGraphs::iterator;
using PlatformGraphsCIter = PlatformGraphs::const_iterator;

/**
 * constructPlatformGraph ()
 * Construct an platform graph.
 */
std::unique_ptr<PlatformGraph> constructPlatformGraph(CNodePtr archNode);

/**
 * setUsagePlatformGraph ()
 * Reserve resource in platform graph.
 */
void setUsagePlatformGraph(PlatformGraph* g, CNodePtr archNode);

/**
 * setMappingPlatformGraph ()
 * The function sets the mapping of the application graph to the platform
 * graph as specified in the mapping node.
 */
void setMappingPlatformGraph(const std::unique_ptr<PlatformGraph>& ar, TimedSDFgraph *ap, CNodePtr mappingNode);
} // namespace SDF
#endif
