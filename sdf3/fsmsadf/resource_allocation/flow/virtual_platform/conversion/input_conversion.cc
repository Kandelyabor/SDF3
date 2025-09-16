/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 20, 2011
 *
 *  Function        :   Input/Output format conversion
 *
 *  History         :
 *      20-04-11    :   Initial version.
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

#include "input_conversion.h"
#include "exception/exception.h"
#include <cassert>

namespace FSMSADF {

// constructor

InputConversionVirtualPlatform::InputConversionVirtualPlatform() : internal_doc(nullptr) {
    CNode *root = CNewNode("sdf3");
    this->set_output_doc(CNewDoc(root));
    CSetAttribute(root, "type", "fsmsadf");
}

// destructor

InputConversionVirtualPlatform::~InputConversionVirtualPlatform() = default;

void InputConversionVirtualPlatform::readApplicationFile(const std::string input_file) {
    internal_doc = CParseFile(input_file);
    if (internal_doc == nullptr || internal_doc->getXmlDoc()==nullptr) {
        throw CException("Failed to read input file.");
    }
    CNode *root = internal_doc->getRootNode();
    CNode *application_graph_src = CFindNode(root, "applicationGraph");

    CNode *application_graph_dest = CCopyNode(application_graph_src);

    CAddNode(this->get_output_doc()->getRootNode(), application_graph_dest);

    internal_doc = nullptr;
}

void InputConversionVirtualPlatform::readArchitectureFile(const std::string input_file) {
    internal_doc = CParseFile(input_file);
    if (internal_doc == nullptr || internal_doc->getXmlDoc()==nullptr) {
        throw CException("Failed to read input file.");
    }
    CNode *root = internal_doc->getRootNode();
    CNode *application_graph_src = CFindNode(root, "architectureGraph");

    CNode *application_graph_dest = CCopyNode(application_graph_src);

    CAddNode(this->get_output_doc()->getRootNode(), application_graph_dest);

    internal_doc = nullptr;
}
} // namespace FSMSADF