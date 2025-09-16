/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   apg.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 2, 2005
 *
 *  Function        :   Acyclic precedence graph
 *
 *  History         :
 *      02-08-05    :   Initial version.
 *
 * $Id: apg.h,v 1.1.1.1.2.1 2010-04-25 01:21:17 mgeilen Exp $
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

#ifndef SDF_TRANSFORM_TO_APG_APG_H_INCLUDED
#define SDF_TRANSFORM_TO_APG_APG_H_INCLUDED

#include "base/base.h" // IWYU pragma: keep

namespace SDF
{


    // Forward class definitions
    class APGnode;
    class APGedge;
    class APGgraph;

    using APGnodes = std::list<std::unique_ptr<APGnode>>;
    using APGnodeRefs = std::list<APGnode*>;
    using APGnodesIter = APGnodes::iterator;
    using APGnodeRefsIter = APGnodeRefs::iterator;

    using APGedges = std::list<std::unique_ptr<APGedge>>;
    using APGedgeRefs = std::list<APGedge*>;
    using APGedgesIter = APGedges::iterator;
    using APGedgeRefsIter = APGedgeRefs::iterator;

    using APGgraphs = std::vector<std::unique_ptr<APGgraph>>;
    using APGgraphsIter = APGgraphs::iterator;

    /**
     * APGnode
     * Node in acyclic precedence graph.
     */
    class APGnode
    {
        public:
            // Constructor
            explicit APGnode(const CId id) : id(id)
            {
            };

            // Destructor
            ~APGnode() = default;

            // Prevent copying and moving
            APGnode(const APGnode&) = delete;
            APGnode& operator=(const APGnode&) = delete;
            APGnode(APGnode&&) = delete;
            APGnode& operator=(APGnode&&) = delete;

            // Id
            [[nodiscard]] CId getId() const
            {
                return id;
            };

            // Weight
            [[nodiscard]] int getWeight() const
            {
                return weight;
            };
            void setWeight(int w)
            {
                weight = w;
            };

            // Edges (management)
            void newEdge(APGedge *e);
            void delEdge(APGedge *e);
            void delAllEdges();

            // Edges (access)
            APGnodeRefsIter adjNodesBegin()
            {
                return adjNodes.begin();
            };
            APGnodeRefsIter adjNodesEnd()
            {
                return adjNodes.end();
            };
            APGedgeRefsIter inEdgesBegin()
            {
                return inEdges.begin();
            };
            APGedgeRefsIter inEdgesEnd()
            {
                return inEdges.end();
            };
            APGedgeRefsIter outEdgesBegin()
            {
                return outEdges.begin();
            };
            APGedgeRefsIter outEdgesEnd()
            {
                return outEdges.end();
            };
            [[nodiscard]] const APGedgeRefs& getEdges() const {
                return edges;
            }
            APGedgeRefsIter edgesBegin()
            {
                return edges.begin();
            };
            APGedgeRefsIter edgesEnd()
            {
                return edges.end();
            };

            // Nodes
            APGnode *opposite(APGedge *e);

            // Degree
            [[nodiscard]] uint inDegree() const
            {
                return static_cast<uint>(inEdges.size());
            };
            [[nodiscard]] uint outDegree() const
            {
                return static_cast<uint>(outEdges.size());
            };
            [[nodiscard]] uint degree() const
            {
                return static_cast<uint>(edges.size());
            };

        private:
            CId         id;
            int         weight{};
            APGnodeRefs adjNodes;
            APGedgeRefs inEdges;
            APGedgeRefs outEdges;
            APGedgeRefs edges;
    };

    /**
     * APGedge
     * Edge in acyclic precedence graph.
     */
    class APGedge
    {
        public:
            // Constructor
            explicit APGedge(const CId id, APGnode *src = nullptr, APGnode *dst = nullptr) : id(id), src(src), dst(dst)
            { };

            // Destructor
            ~APGedge() = default;

            // Prevent copying and moving
            APGedge(const APGedge&) = delete;
            APGedge& operator=(const APGedge&) = delete;
            APGedge(APGedge&&) = delete;
            APGedge& operator=(APGedge&&) = delete;

            // Id
            [[nodiscard]] CId getId() const
            {
                return id;
            };

            // Weight
            [[nodiscard]] int getWeight() const
            {
                return weight;
            };
            void setWeight(int w)
            {
                weight = w;
            };

            // Nodes
            [[nodiscard]] APGnode *getSrc() const
            {
                return src;
            };
            [[nodiscard]] APGnode *getDst() const
            {
                return dst;
            };
            void changeSrc(APGnode *n)
            {
                src = n;
            };
            void changeDst(APGnode *n)
            {
                dst = n;
            };
            APGnode *opposite(APGnode *n)
            {
                return (n->getId() == src->getId() ? dst : src);
            };

        private:
            CId     id;
            int     weight{};
            APGnode *src;
            APGnode *dst;
    };

    /**
     * APGgraph
     * Acyclic precedence graph.
     */
    class APGgraph
    {
        public:
            // Constructor
            explicit APGgraph( CId id = 0);

            // Destructor
            ~APGgraph();

            // Prevent copying and moving
            APGgraph(const APGgraph&) = delete;
            APGgraph& operator=(const APGgraph&) = delete;
            APGgraph(APGgraph&&) = delete;
            APGgraph& operator=(APGgraph&&) = delete;

            // Nodes (management)
            [[nodiscard]] uint nrNodes() const
            {
                return static_cast<uint>(nodes.size());
            };
            APGnode *newNode( CId id);
            void delNode(APGnode *n);
            void delAllNodes();

            // Nodes (access)
            APGnode *getNode(CId id);
            [[nodiscard]] const APGnodes& getNodes() const {
                return nodes;
            }
            APGnodesIter nodesBegin()
            {
                return nodes.begin();
            };
            APGnodesIter nodesEnd()
            {
                return nodes.end();
            };

            // Edges (management)
            [[nodiscard]] uint nrEdges() const
            {
                return static_cast<uint>(edges.size());
            };
            APGedge *newEdge( CId id, APGnode *src, APGnode *dst);
            void delEdge(APGedge *e);
            void delAllEdges();

            // Edges (access)
            APGedge *getEdge(CId id);
            [[nodiscard]] const APGedges& getEdges() const {
                return edges;
            }
            APGedgesIter edgesBegin()
            {
                return edges.begin();
            };
            APGedgesIter edgesEnd()
            {
                return edges.end();
            };

        private:
            CId         id;
            APGnodes    nodes;
            APGedges    edges;
    };

}//namespace SDF
#endif
