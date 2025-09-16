/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   testhtml.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Nov. 16, 2011
 *
 *  Function        :   Program used to convert test results to html
 *
 *  History         :
 *      16-11-11    :   Initial version.
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

#include "tools/test-settings/testsettings.h"
#include "base/log/log.h"
#include "base/shell/dirs.h"
#include "base/xhtml/xhtml.h"
#include "base/xml/xml.h"

// Needed for result conversion.
#include "tools/test-run/test_base.h"
#include <memory>
#include <string>

#include "testhtml.h"

namespace {

std::unique_ptr<TestSettings> settings = nullptr; // NOLINT(*global-variables)

} // namespace

class TestSettingsHTML : public TestSettings {
public:
    TestSettingsHTML(const CString &module, const CString &tool) : TestSettings(module, tool) {
        // HTML generation.
        addValue("inputDirectory",
                 "./",
                 "The directory to read the result files for."
                 "Tool looks for *.xml files in subdirectories.");

        addValue("outputDirectory", "./", "The xhtml output directory.");
    }

    void printProgramHeader(std::ostream &out) override {
        out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
        out << std::endl;
    }
};

class TestDetailXhtml : public Xhtml {
public:
    explicit TestDetailXhtml(CNode *root) {
        // Set some css
        CNode *style_n = CNewNode("style");
        CAddAttribute(style_n, "type", "text/css");
        CSetContent(style_n,
                    "\n"
                    "	div.entry { margin-bottom: 0.4em;}\n"
                    "	div.key   { font-weight: bold;}\n"
                    "	div.value { width: 100%; }\n"
                    "	div.state { display: table-cell; width: 8em; }\n"
                    "   .passed { background-color: #77FF77; }\n"
                    "	.timeout { background-color: #FFFF77; }\n"
                    "	.memoryLimit { background-color: #FFAA77; }\n"
                    "	.failed { background-color: #FF7777; }\n"
                    "	div.value_diff { display: inline-block; width: 48%;} \n"
                    "pre {\n"
                    "	font-size: 12px;\n"
                    "	padding: 0;\n"
                    "	margin: 0;\n"
                    "	background: #f0f0f0;\n"
                    "	border: 1px solid #ccc;\n"
                    "	line-height: 20px; /*--Height of each line of code--*/\n"
                    "	width: auto;\n"
                    "	overflow: auto;\n"
                    "}");
        CAddNode(this->get_head_node(), style_n);

        set_title(root);
        // test node
        CNode *test_node = CFindNode(root, "test");
        // Name
        CString name = CGetAttribute(test_node, "name");
        add_entry("Name", name);

        // Time stamp.
        constexpr size_t BufSize = 256;
        CNode *timestamp_node = CFindNode(test_node, "timestamp");
        if (timestamp_node != nullptr) {
            char buffer[BufSize];
            time_t t_time = static_cast<time_t>(std::stol(CGetNodeContent(timestamp_node)));
            const struct tm *lt = localtime(&t_time); // NOLINT(*mt-unsafe,*deprecated-declarations)
            strftime(buffer, BufSize, "%c", lt); // NOLINT(cert-err33-c,*array-to-pointer-decay,*array-decay)
            add_entry("Test date", buffer);// NOLINT(*array-to-pointer-decay,*array-decay)
        }

        // Result.
        CNode *result_node = CFindNode(test_node, "result");
        RunTestBase::TestResult result =
                RunTestBase::getResultFromString(CGetNodeContent(result_node));
        CNode *rn = CNewNode("div");
        if (result == RunTestBase::PASSED) {
            CAddAttribute(rn, "class", "passed");
        } else if (result == RunTestBase::TIMEOUT) {
            CAddAttribute(rn, "class", "timeout");
        } else if (result == RunTestBase::MEMORY_LIMIT) {
            CAddAttribute(rn, "class", "memoryLimit");
        } else {
            CAddAttribute(rn, "class", "failed");
        }
        add_entry("Result", RunTestBase::getResult(result), false, rn);
        CAddNode(this->get_body_node(), rn);

        RunTestBase::TestResult expected_result = RunTestBase::PASSED;
        CNode *expected_result_node = CFindNode(test_node, "expected");
        if (expected_result_node != nullptr) {
            expected_result =
                    RunTestBase::getResultFromString(CGetNodeContent(expected_result_node));
        }
        if (expected_result != result || result != RunTestBase::PASSED) {
            CNode *rn = CNewNode("div");
            CAddAttribute(rn, "class", "failed");
            add_entry("Expected result", RunTestBase::getResult(expected_result), false, rn);
            CAddNode(this->get_body_node(), rn);
        }

        // Program name
        CNode *program_name_node = CFindNode(test_node, "program");
        if (program_name_node != nullptr) {
            CString program_name = CGetNodeContent(program_name_node);
            add_entry("Program name", program_name);
        }
        // Timeout
        CNode *timeout_node = CFindNode(test_node, "programTimeout");
        if (timeout_node != nullptr) {
            CString timeout = CGetNodeContent(timeout_node);
            add_entry("Timeout", timeout + " Seconds");
        }
        // Description
        CNode *description_node = CFindNode(test_node, "description");
        if (description_node != nullptr) {
            CString description = CGetNodeContent(description_node);
            add_entry("Description", description);
        }
        // Memory Limit
        CNode *memory_limit_node = CFindNode(test_node, "programMemoryLimit");
        if (memory_limit_node != nullptr) {
            CString memory_limit = CGetNodeContent(memory_limit_node);
            add_entry("Memory limit", memory_limit + " MB");
        }
        // Return value
        CNode *ret_v_node = CFindNode(test_node, "return_value");
        if (ret_v_node != nullptr) {
            CString ret_v = CGetNodeContent(ret_v_node);
            add_entry("Return Value", ret_v);
        }

        // stdout of program
        CNode *stdout_node = CFindNode(test_node, "stdout");
        if (stdout_node != nullptr) {
            CString stdout_o = CGetNodeContent(stdout_node);
            add_entry("Program output (stdout)", stdout_o, true);
        }
        // stderr of program.
        CNode *stderr_node = CFindNode(test_node, "stderr");
        if (stderr_node != nullptr) {
            CString stderr_o = CGetNodeContent(stderr_node);
            add_entry("Program output (stderr)", stderr_o, true);
        }
        // Type
        CNode *type_node = CFindNode(test_node, "type");
        CString type = CGetNodeContent(type_node);
        add_entry("Type", type);

        for (CNode *of_n = CFindNode(test_node, "outputFile"); of_n != nullptr;
             of_n = CNextNode(of_n, "outputFile")) {
            CString of_name = CGetAttribute(of_n, "filename");
            add_entry("Output file", of_name);
            CNode *of_res_n = CFindNode(of_n, "result");
            CString of_result = CGetNodeContent(of_res_n);
            add_entry("Output file state", of_result);

            CNode *of_diff_node = CFindNode(of_n, "diff");
            if (of_diff_node != nullptr) {
                CString diff = CGetNodeContent(of_diff_node);
                add_entry("Diff", diff, true);
            } else {
                CNode *of_output_node = CFindNode(of_n, "output");
                CNode *of_reference_node = CFindNode(of_n, "reference");
                if (of_output_node != nullptr) {
                    CString content2 = CGetNodeContent(of_reference_node);
                    CString content = CGetNodeContent(of_output_node);
                    add_entry_diff("Output file diff", content, content2, true);
                }
            }
        }
    }

    // Prevent copying and moving
    TestDetailXhtml(const TestDetailXhtml&) = delete;
    TestDetailXhtml& operator=(const TestDetailXhtml&) = delete;
    TestDetailXhtml(TestDetailXhtml&&) = delete;
    TestDetailXhtml& operator=(TestDetailXhtml&&) = delete;


private:
    void set_title(CNode *root) {
        CNode *test_node = CFindNode(root, "test");
        CNode *result_node = CFindNode(test_node, "result");
        CString name = CGetAttribute(test_node, "name");
        CString result = CGetNodeContent(result_node);

        setPageTitle(result + ": " + name);
    }
    void add_entry(const CString &key,
                   const CString &value,
                   bool formatted = false,
                   CNode *parent = nullptr) {
        if (parent == nullptr) {
            parent = this->get_body_node();
        }
        CNode *result = CNewNode("div");
        CAddAttribute(result, "class", "entry");

        CNode *key_node = CNewNode("div");
        CAddAttribute(key_node, "class", "key");
        CSetContent(key_node, key);
        CAddNode(result, key_node);

        CNode *value_node = CNewNode("div");
        CAddAttribute(value_node, "class", "value");
        if (formatted) {
            CNode *pre_n = CNewNode("pre");
            CSetContent(pre_n, value);
            CAddNode(value_node, pre_n);
        } else {
            CSetContent(value_node, value);
        }
        CAddNode(result, value_node);

        CAddNode(parent, result);
    }
    void add_entry_diff(const CString &key,
                        const CString &value,
                        const CString &value2,
                        bool formatted = false) {
        CNode *result = CNewNode("div");
        CAddAttribute(result, "class", "entry");

        CNode *key_node = CNewNode("div");
        CAddAttribute(key_node, "class", "key");
        CSetContent(key_node, key);
        CAddNode(result, key_node);

        CNode *value_node = CNewNode("div");
        CAddAttribute(value_node, "class", "value");

        CNode *diff_node = CNewNode("div");
        CAddAttribute(diff_node, "class", "value_diff");
        if (formatted) {
            CNode *pre_n = CNewNode("pre");
            CSetContent(pre_n, value);
            CAddNode(diff_node, pre_n);
        } else {
            CSetContent(diff_node, value);
        }
        CAddNode(value_node, diff_node);

        diff_node = CNewNode("div");
        CAddAttribute(diff_node, "class", "value_diff");
        if (formatted) {
            CNode *pre_n = CNewNode("pre");
            CSetContent(pre_n, value2);
            CAddNode(diff_node, pre_n);
        } else {
            CSetContent(diff_node, value2);
        }
        CAddNode(value_node, diff_node);
        CAddNode(result, value_node);
        CAddNode(this->get_body_node(), result);
    }

public:
    ~TestDetailXhtml() override = default;
};
class TestXhtml : public Xhtml, public DirUtil {
    std::list<std::pair<CString, std::unique_ptr<CDoc>>> test_results;

public:
    TestXhtml() {
        setPageTitle("SDF3 Test program output");

        // Set some css
        CNode *style_n = CNewNode("style");
        CAddAttribute(style_n, "type", "text/css");
        CSetContent(style_n,
                    "\n"
                    "	div.result { padding: 0.1em;width: 70em;  position: relative; display: "
                    "block; }\n"
                    "	div.name  { display: table-cell; width: 42em;}\n"
                    "	div.type { display: table-cell; width: 10em;}\n"
                    "	div.state { display: table-cell; width: 12em; }\n"
                    "	div.link { display: table-cell; width: 4em;}\n"
                    " .passed { padding: 0.1em;background-color: #77FF77; }\n"
                    " .timeout { padding: 0.1em;background-color: #FFFF77; }\n"
                    " .memoryLimit { padding: 0.1em;background-color: #FFAA77; }\n"
                    " .failed { padding: 0.1em;background-color: #Ff7777; }\n"
                    "");
        CAddNode(get_head_node(), style_n);
    }

    ~TestXhtml() override = default;

    // Prevent copying and moving
    TestXhtml(const TestXhtml&) = delete;
    TestXhtml& operator=(const TestXhtml&) = delete;
    TestXhtml(TestXhtml&&) = delete;
    TestXhtml& operator=(TestXhtml&&) = delete;

    void Add(std::unique_ptr<CDoc> result, const CString &filename) { test_results.emplace_back(filename, std::move(result)); }
    /**
     * Custom sort function. This sorts the list by result. (failed/mem limit etc)
     */
    static bool my_test_sort(const std::pair<CString, std::unique_ptr<CDoc>> &a,
                             const std::pair<CString, std::unique_ptr<CDoc>> &b) {
        CNode *result_a = CFindNode(a.second->getRootNode(), "result");
        CNode *result_b = CFindNode(b.second->getRootNode(), "result");
        CString ra = CGetNodeContent(result_a);
        CString rb = CGetNodeContent(result_b);
        return (RunTestBase::getResultFromString(ra) > RunTestBase::getResultFromString(rb));
    }

    static CNode *AddRow() {
        CNode *result_node = CNewNode("div");
        CAddAttribute(result_node, "class", "result");
        return result_node;
    }
    static CNode *AddRowResultType(RunTestBase::TestResult result,
                                   RunTestBase::TestResult expected) {
        CNode *ds = CNewNode("div");
        if (result != expected) {
            CAddAttribute(ds, "class", "failed");
        } else {
            if (result == RunTestBase::PASSED) {
                CAddAttribute(ds, "class", "passed");
            } else if (result == RunTestBase::TIMEOUT) {
                CAddAttribute(ds, "class", "timeout");
            } else if (result == RunTestBase::MEMORY_LIMIT) {
                CAddAttribute(ds, "class", "memoryLimit");
            } else {
                CAddAttribute(ds, "class", "failed");
            }
        }
        return ds;
    }

    static void AddCell(CNode *n, const CString &class_n, const CString &value) {
        CNode *type_n = CNewNode("div");
        CAddAttribute(type_n, "class", class_n);
        CSetContent(type_n, value);
        CAddNode(n, type_n);
    }
    /**
     * Add  subnode link to N.
     * <div class="link"><a href="link">name</a></div>
     */
    static void AddLink(CNode *n, const CString &name, const CString &link) {
        CNode *div_n = CNewNode("div");
        CAddAttribute(div_n, "class", "link");
        CNode *type_n = CNewNode("a");
        CAddAttribute(type_n, "href", link);
        CSetContent(type_n, name);
        CAddNode(div_n, type_n);
        CAddNode(n, div_n);
    }

    void OutputTest(CNode *root, const CString &detail_file) {
        // Get the root node of the test.
        CNode *src_test_node = CFindNode(root, "test");
        ASSERT(src_test_node != nullptr, "Failed to find test in the result file");

        // Test status.
        CNode *src_result_node = CFindNode(src_test_node, "result");
        ASSERT(src_result_node != nullptr, "Failed to find the test result in the result file");
        RunTestBase::TestResult result =
                RunTestBase::getResultFromString(CGetNodeContent(src_result_node));

        CNode *src_expected_result_node = CFindNode(src_test_node, "expected");

        RunTestBase::TestResult expected_result = RunTestBase::PASSED;
        if (src_expected_result_node != nullptr) {
            expected_result =
                    RunTestBase::getResultFromString(CGetNodeContent(src_expected_result_node));
        }

        // XHTML right div for result. (colors)
        CNode *result_node = AddRow();
        CNode *ds = AddRowResultType(result, expected_result);

        // XHTML: Result string
        AddCell(ds, "state", RunTestBase::getResult(result));

        // Test Type
        CNode *type_node = CFindNode(src_test_node, "type");
        CString type = CGetNodeContent(type_node);
        // XHTML Type
        AddCell(ds, "type", type);

        // XHTML Name
        CString name = CGetAttribute(src_test_node, "name");
        AddCell(ds, "name", name);

        AddLink(ds, "detail", detail_file);
        // xhtml add to body.
        CAddNode(result_node, ds);
        CAddNode(this->get_body_node(), result_node);
    }

    void process() {
        auto num_test = static_cast<int>(test_results.size());
        CNode *stat_node = CNewNode("div");
        CAddAttribute(stat_node, "class", "stat");
        CSetContent(stat_node, CString("Number of tests: ") + CString(num_test));
        CAddNode(this->get_body_node(), stat_node);

        test_results.sort(my_test_sort);
        for (auto &[detail_file, doc] : test_results) {
            CString outputFile = settings->getValue("outputDirectory");
            CNode *root = doc->getRootNode();
            detail_file.replace("xml", "xhtml");
            OutputTest(root, detail_file);

            TestDetailXhtml dx(root);
            dx.write(outputFile + "/" + detail_file);
        }
    }

    //////////////////// DIR UTIL //////////////////////////////
    // called for every file found
    void fileFound(const CString &path, const CString &file) override {
        std::unique_ptr<CDoc> doc = CParseFile(path + "/" + file);
        this->Add(std::move(doc), file);
    }
    void newDir(const CString &path, const CString &dir) override {}

    int hasError() {
        int exit_status = EXIT_SUCCESS;
        for (auto &[tr_f, doc] : test_results) {
            CNode *resultNode = CFindNode(doc->getRootNode(), "result");
            CString result_str = CGetNodeContent(resultNode);
            RunTestBase::TestResult result = RunTestBase::getResultFromString(result_str);
            RunTestBase::TestResult expected = result;

            CNode *expected_resultNode = CFindNode(doc->getRootNode(), "expected");
            if (expected_resultNode != nullptr) {
                CString expected_result_str = CGetNodeContent(expected_resultNode);
                expected = RunTestBase::getResultFromString(expected_result_str);
            }
            if (result != expected) {
                CNode *testNode = CFindNode(doc->getRootNode(), "test");
                CString testName = CGetAttribute(testNode, "name");
                logInfo("Warning: test '" + testName + "' failed.");

                if (CHasChildNode(testNode, "description")) {
                    CNode *descriptionNode = CFindNode(testNode, "description");
                    CString descriptionString = CGetNodeContent(descriptionNode);
                    logInfo("Description: '" + descriptionString + "'.");
                } else if (CHasChildNode(testNode, "stderr")) {
                    CNode *stderrNode = CFindNode(testNode, "stderr");
                    CString stderrString = CGetNodeContent(stderrNode);
                    logInfo("stderr: ");
                    logInfo(stderrString);
                }
                exit_status = EXIT_FAILURE;
            }
        }
        return exit_status;
    }
};

namespace {

/**
 * Generate html output.
 */
int generateHtml() {
    TestXhtml x;

    CString outputFile = settings->getValue("outputDirectory");
    CString inputDirectory = settings->getValue("inputDirectory");

    // walk the directories.
    x.find(inputDirectory, "*.xml", FT_FILE);

    x.process();
    // Write output file.
    x.write(outputFile + "/index.xhtml");

    return x.hasError();
}

} // namespace

int main(int argc, char **argv) {
    int exit_status = EXIT_SUCCESS;

    try {
        settings = std::make_unique<TestSettingsHTML>(MODULE, SETTINGS_TYPE);

        // parse command line argument.
        if (settings->parseArguments(argc, argv)) {
            return exit_status;
        }

        // Generate the html.
        exit_status = generateHtml();
    } catch (CException &e) {
        std::cerr << e;
        exit_status = EXIT_FAILURE;
    }

    // Clear out xml code.
    xmlCleanup();
    return exit_status;
}
