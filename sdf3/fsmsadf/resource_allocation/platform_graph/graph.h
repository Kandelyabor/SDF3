/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Platform graph.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: graph.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_GRAPH_H_INCLUDED

#include "connection.h"
#include "tile.h"

namespace FSMSADF {

// Forward class definition
class PlatformGraph;

// List of platform graph pointers
using PlatformGraphs = std::list<PlatformGraph *>;

/**
 * PlatformGraph
 * Container for platform graph.
 */
class PlatformGraph : public GraphComponent {
public:
    // Constructor
    explicit PlatformGraph(const GraphComponent &c);
    PlatformGraph();

    // Destructor
    ~PlatformGraph() override;

    PlatformGraph(const PlatformGraph&) = delete;
    PlatformGraph& operator=(const PlatformGraph&) = delete;
    PlatformGraph(PlatformGraph&&) = delete;
    PlatformGraph& operator=(PlatformGraph&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<PlatformGraph> create(const GraphComponent &c);
    std::unique_ptr<PlatformGraph> createCopy(const GraphComponent &c);
    std::unique_ptr<PlatformGraph> clone(const GraphComponent &c);

    // Construct
    void constructFromXML(CNodePtr platformGraphNode);

    // Convert
    void convertToXML(CNodePtr platformGraphNode);

    // Tiles
    Tiles &getTiles() { return tiles; };
    Tile *getTile(CId id);
    Tile *getTile(const CString &name);
    Tile *createTile(const CString &name);
    void addTile(std::unique_ptr<Tile> t);
    void removeTile(Tile *t);

    // Connections
    Connections &getConnections() { return connections; };
    Connection *getConnection(CId id);
    Connection *getConnection(const CString &name);
    Connection *createConnection(const CString &name);
    void addConnection(std::unique_ptr<Connection> c);
    void removeConnection(Connection *c);

private:
    // Tiles
    Tiles tiles;

    // Connections
    Connections connections;
};

} // End namespace FSMSADF

#endif
