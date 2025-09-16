/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   to_binding_aware.cc
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   December 17, 2015
 *
 *  Function        :   Transform FSM-based SADF to binding-aware graph
 *
 *  History         :
 *      17-12-15    :   Initial version.
 *
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

#include "to_binding_aware.h"
#include "fsmsadf/resource_allocation/flow/compsoc_platform/binding_aware_graph/binding_aware_graph.h"
#include <memory>

namespace FSMSADF {

/**
 * toBindingAware ()
 * The function converts the graph to a binding-aware graph.
 */
std::unique_ptr<Graph> toBindingAware(Graph *ag, PlatformGraph *pg, PlatformBindings *pbs) {
    std::unique_ptr<Graph> transG = ag->clone(*ag);

    // Check if all graphs are available
    if (pg == nullptr) {
        throw CException("No architectureGraph found.");
    }

    if (pbs->empty()) {
        throw CException("No mappings found.");
}

    for (const auto& pb: *pbs) {
        if (pb->getName() != "initial") {
            auto bag = std::make_unique<BindingAwareGraphCompSoCPlatform>(pb.get());

            bag->constructBindingAwareGraph(true);

            std::ofstream outfile("bag-" + pb->getName() + ".xml", std::ofstream::binary);
            std::unique_ptr<CDoc> doc = nullptr;

            CNode * sdf3Node = CNewNode("sdf3");

            // Attributes
            CAddAttribute(sdf3Node, "version", "1.0");
            CAddAttribute(sdf3Node, "type", "fsmsadf");
            CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
            CAddAttribute(sdf3Node, "xmlns", "uri:sdf3");
            CAddAttribute(sdf3Node,
                          "xsi:noNamespaceSchemaLocation",
                          "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

            CNode *bagNode = CAddNode(sdf3Node, "applicationGraph");
            bag->convertToXML(bagNode);
            CSetAttribute(bagNode, "name", "bindingAwareGraph");

            pg->convertToXML(CAddNode(sdf3Node, "architectureGraph"));

            pb->convertToXML(CAddNode(sdf3Node, "mapping"));

            doc = CNewDoc(sdf3Node);
            doc->saveFile(outfile, 1);
            outfile.close();
        }
    }
    return transG;
}

} // namespace FSMSADF
