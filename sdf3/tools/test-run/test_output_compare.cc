/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   test_output_compare.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   November 4, 2011
 *
 *  Function        :   Test Run output compare
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
#include "test_output_compare.h"
#include "base/exception/exception.h"
#include "test_base.h"


// Base
#include "tools/test-settings/testsettings.h"

#include "base/textdiff/textdiff.h"
#include "tools/test-run/runtest.h"

using namespace RunTestGlobals;

RunTestOutputCompare::RunTestOutputCompare() : matches(true) {
    test = CNewNode("test");
    CAddAttribute(test, "name", settings->getValue("name"));
    CNode *out_n = CNewNode("type");
    CSetContent(out_n, "Output Compare");
    CAddNode(test, out_n);
}

RunTestOutputCompare::~RunTestOutputCompare() {
    CRemoveNode(test);
    test = nullptr;
}

void RunTestOutputCompare::run() { this->execute_program(); }
CString RunTestOutputCompare::cleanup_input_string(const CString &input) {
    CString ret_v = input;

    // normalize all line endings
    ret_v.replace("\r\n", "\n");

    CStrings ignores = settings->getValues("ignore");
    for (const auto &iter : ignores) {
        ret_v = ret_v.regexReplaceMultiLine(iter, "");
    }
    // Trim whitespace.
    ret_v = ret_v.trim();
    return ret_v;
}

void RunTestOutputCompare::validate_files() {
    CStrings outputFiles = settings->getValues("programOutput");
    for (auto &filename : outputFiles) {
        CNode *file_node = CNewNode("outputFile");
        CAddAttribute(file_node, "filename", filename);
        CNode *res_node = CNewNode("result");

        try {
            std::unique_ptr<CString> file_out = this->get_shell().readFile(filename);
            std::unique_ptr<CString> ref_out = this->get_shell().readFile(filename + ".ref");

            // Cleanup
            CString res_stdout = cleanup_input_string(*file_out);
            CString references = cleanup_input_string(*ref_out);

            if (references == res_stdout) {
                CSetContent(res_node, "EQUAL");
                CNode *fr = CNewNode("description");
                CSetContent(fr, "The test ran without errors.");
                CAddNode(test, fr);
            } else {
                this->matches = false;

                CSetContent(res_node, "DIFFER");
                CNode *file_a = CNewNode("diff");

                CString diff = TextDiff(res_stdout, references);
                CSetContent(file_a, diff);

                CAddNode(file_node, file_a);
            }
            // Store full output files.
            CNode *file_a_full = CNewNode("output_full");
            CNode *file_b_full = CNewNode("reference_full");
            CSetContent(file_a_full, res_stdout);
            CSetContent(file_b_full, references);
            CAddNode(file_node, file_a_full);
            CAddNode(file_node, file_b_full);

        } catch (CException &e) {
            CSetContent(res_node, "FAILED");
            CNode *file_a = CNewNode("output");
            CNode *file_b = CNewNode("reference");
            CSetContent(file_a, e.getMessage());
            CSetContent(file_b, "");
            CAddNode(file_node, file_a);
            CAddNode(file_node, file_b);
            this->matches = false;
        }
        CAddNode(file_node, res_node);
        CAddNode(test, file_node);
    }
}

void RunTestOutputCompare::validate_stdout() {
    if (settings->hasKey("reference") && settings->getValue("reference").length() > 0) {
        CString res_stdout = cleanup_input_string(this->get_redstdout());
        CString references = cleanup_input_string(settings->getValue("reference"));

        CNode *file_node = CNewNode("outputFile");
        CAddAttribute(file_node, "filename", "");
        CNode *res_node = CNewNode("result");
        if (references == res_stdout) {
            CSetContent(res_node, "EQUAL");
            CNode *fr = CNewNode("description");
            CSetContent(fr, "The test ran without errors.");
            CAddNode(test, fr);
        } else {
            this->matches = false;

            CSetContent(res_node, "DIFFER");
            CNode *file_a = CNewNode("diff");

            CString diff = TextDiff(res_stdout, references);
            CSetContent(file_a, diff);

            CAddNode(file_node, file_a);
        }

        CAddNode(file_node, res_node);
        // Store full output files.
        CNode *file_a_full = CNewNode("output_full");
        CNode *file_b_full = CNewNode("reference_full");
        CSetContent(file_a_full, res_stdout);
        CSetContent(file_b_full, references);
        CAddNode(file_node, file_a_full);
        CAddNode(file_node, file_b_full);

        CAddNode(test, file_node);
    }
}

int RunTestOutputCompare::validate() {
    // Run checks.
    this->validate_stdout();
    this->validate_files();

    RunTestBase::TestResult t = getResultReturnValue(this->get_return_value());

    CNode *result_node = CNewNode("result");
    CSetContent(result_node, getResult(t));
    CAddNode(test, result_node);

    if (t != PASSED) {
        CString failure_reason = this->getFailureReason();
        CNode *fr = CNewNode("description");
        CSetContent(fr, failure_reason);
        CAddNode(test, fr);

        if (settings->hasKey("expected")) {
            if (settings->getValue("expected") == RunTestOutputCompare::getResult(t)) {
                return EXIT_SUCCESS;
            }
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
    }

    if (this->matches) {
        // Everything passed fine.
        CNode *fr = CNewNode("description");
        CSetContent(fr, "The test ran without errors.");
        CAddNode(test, fr);
        return EXIT_SUCCESS;
    }

    // Failed to match file.
    // set result to failed.
    CSetContent(result_node, getResult(FAILED));
    // Set description
    CNode *fr = CNewNode("description");
    CSetContent(fr, "The generated output (files) did not match with the reference (files).");
    CAddNode(test, fr);
    return EXIT_FAILURE;
}

void RunTestOutputCompare::output(CNode *root_node) {
    this->output_shell_testresult(test);

    /// @todo specific output.

    CAddNode(root_node, CCopyNode(test));
}
