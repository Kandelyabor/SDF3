/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   test_xml_compare.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   November 4, 2011
 *
 *  Function        :   Test Run XML compare
 *
 *  History         :
 *      04-11-11    :   Initial version.
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
#include "test_xml_compare.h"
#include "test_base.h"

// Base
#include "base/xml/xml.h"
#include "base/xmlcompare/xmlcompare.h"
#include "tools/test-run/runtest.h"
#include "tools/test-settings/testsettings.h"


using namespace RunTestGlobals;

RunTestXmlCompare::RunTestXmlCompare() : files_match(true) {
    // start result
    test_node = CNewNode("test");
    CAddAttribute(test_node, "name", settings->getValue("name"));

    // type
    CNode *out_n = CNewNode("type");
    CSetContent(out_n, "Xml Compare");
    CAddNode(test_node, out_n);
}

int RunTestXmlCompare::validate() {
    if (this->get_return_value() != 0) {
        return EXIT_FAILURE;
    }

    int result = EXIT_SUCCESS;

    CStrings outputFiles = settings->getValues("programOutput");
    for (const auto &filename : outputFiles) {
        std::unique_ptr<CDoc> a = nullptr;
        std::unique_ptr<CDoc> b = nullptr;
        // Check if files exists
        a = CParseFile(filename);
        b = CParseFile(filename + ".ref");

        // Output XML
        CNode *file_node = CNewNode("outputFile");
        CAddAttribute(file_node, "filename", filename);
        CNode *res_node = CNewNode("result");

        if (a == nullptr || b == nullptr || a->getXmlDoc()==nullptr || b->getXmlDoc()==nullptr) {
            files_match = false;
            result = EXIT_FAILURE;

            CSetContent(res_node, "INVALID");
            CNode *file_a = CNewNode("output");
            if (a == nullptr) {
                CSetContent(file_a, "Invalid XML");
            }
            CNode *file_b = CNewNode("reference");
            if (b == nullptr) {
                CSetContent(file_b, "Invalid XML");
            }

            // Add files.
            CAddNode(file_node, file_a);
            CAddNode(file_node, file_b);
            // Add nodes.
            CAddNode(file_node, res_node);
            CAddNode(test_node, file_node);
            break;
        }
        XmlCompare xc(a->getRootNode(), b->getRootNode());

        bool equal = xc.compare();
        files_match = (files_match) ? equal : files_match;

        if (equal) {
            CSetContent(res_node, "EQUAL");
        } else {
            result = EXIT_FAILURE;
            CSetContent(res_node, "DIFFER");
            xc.purge();

            CNode *file_a = CNewNode("output");
            CNode *file_b = CNewNode("reference");
            CString file_a_ser = xc.getDocA()->serializeFile(1);
            CString file_b_ser = xc.getDocB()->serializeFile(1);
            // When removing similar nodes, libxml leaves the whitespace.
            // This hack removes this.
            // Test if this works in windows as well.
            file_a_ser = file_a_ser.regexReplaceMultiLine("^[ \t]*\n", "");
            file_b_ser = file_b_ser.regexReplaceMultiLine("^[ \t]*\n", "");
            CSetContent(file_a, file_a_ser);
            CSetContent(file_b, file_b_ser);

            CAddNode(file_node, file_a);
            CAddNode(file_node, file_b);
        }
        CAddNode(file_node, res_node);

        // Store full output files.
        CNode *file_a_full = CNewNode("output_full");
        CNode *file_b_full = CNewNode("reference_full");
        CSetContent(file_a_full, b->serializeFile(1));
        CSetContent(file_b_full, b->serializeFile(1));
        CAddNode(file_node, file_a_full);
        CAddNode(file_node, file_b_full);

        CAddNode(test_node, file_node);
    }
    return result;
}

void RunTestXmlCompare::run() {
    execute_program();
    output_shell_testresult(test_node);
}

void RunTestXmlCompare::output(CNode *root_node) {
    // Result
    CNode *res_n = CNewNode("result");
    if (this->get_return_value() == Shell::RESULT_OK) {
        // Do output compare.
        if (files_match) {
            CSetContent(res_n, getResult(PASSED));
            CNode *fr = CNewNode("description");
            CSetContent(fr, "The test ran without errors.");
            CAddNode(test_node, fr);
        } else {
            CSetContent(res_n, getResult(FAILED));
            CNode *fr = CNewNode("description");
            CSetContent(fr, "The generated output files did not match with the reference files.");
            CAddNode(test_node, fr);
        }
    } else {
        CString failure_reason = this->getFailureReason();
        CNode *fr = CNewNode("description");
        CSetContent(fr, failure_reason);
        CAddNode(root_node, fr);
        CSetContent(res_n, RunTestBase::getResult(getResultReturnValue(this->get_return_value())));
    }
    CAddNode(test_node, res_n);

    CAddNode(root_node, CCopyNode(test_node));
}

RunTestXmlCompare::~RunTestXmlCompare() {
    if (test_node != nullptr) {
        CRemoveNode(test_node);
    }
}
