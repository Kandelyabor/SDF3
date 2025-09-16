/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   test_base.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   November 4, 2011
 *
 *  Function        :   Test base
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
#include "test_base.h"
#include "base/log/log.h"
#include "tools/test-run/runtest.h"

using namespace RunTestGlobals;

constexpr double MillisecondsPerSecond = 1000;

void RunTestBase::execute_program() {
    logMsg("Running experiment");
    // Current directory
    CString directory = "./";
    // Command to execute.
    CString command = settings->getValue("program");
    // Memory limit
    double memory = static_cast<double>(settings->getValue("programMemoryLimit"));
    // Timeout
    double timeout = static_cast<double>(settings->getValue("programTimeout"));
    // Execute.
    return_value = shell->execute(directory,
                                  command,
                                  &redstdout,
                                  &redstderr,
                                  static_cast<uint>(timeout * MillisecondsPerSecond),
                                  static_cast<uint>(memory));
    // Report failure
    if (return_value != EXIT_SUCCESS) {
        logMsg("Test failed: " + getFailureReason());
        logMsg("Output: " + *redstdout);
        logMsg("Error Output: " + *redstderr);
    }
}

int RunTestBase::validate() { return EXIT_SUCCESS; }

RunTestBase::~RunTestBase() = default;

void RunTestBase::output_shell_testresult(CNode *test_node) {
    // Return value
    CNode *outNode = CNewNode("return_value");
    CSetContent(outNode, CString(return_value));
    CAddNode(test_node, outNode);

    CNode *outExpected = CNewNode("expected");
    if (settings->hasKey("expected")) {
        CSetContent(outExpected, settings->getValue("expected"));
    } else {
        CSetContent(outExpected, "PASSED");
    }
    CAddNode(test_node, outExpected);
    // STDOUT
    outNode = CNewNode("stdout");
    CSetContent(outNode, *redstdout);
    CAddNode(test_node, outNode);
    // STDERR
    outNode = CNewNode("stderr");
    CSetContent(outNode, *redstderr);
    CAddNode(test_node, outNode);

    // Write out timestamp of test.
    outNode = CNewNode("timestamp");
    constexpr size_t BufferSize = 128;
    char buffer[BufferSize];
    time_t ctime = time(nullptr);
    snprintf(buffer, BufferSize, "%lu", static_cast<unsigned long>(ctime)); // NOLINT(cert-err33-c,*decay,*vararg)
    CSetContent(outNode, CString(buffer)); // NOLINT(*decay)
    CAddNode(test_node, outNode);

    // Default outputs.
    // output
    outNode = CNewNode("program");
    CSetContent(outNode, settings->getValue("program"));
    CAddNode(test_node, outNode);

    // Timeout
    CString timeout = settings->getValue("programTimeout");
    outNode = CNewNode("programTimeout");
    CSetContent(outNode, timeout);
    CAddNode(test_node, outNode);

    // Memory limit
    CString memory_limit = settings->getValue("programMemoryLimit");
    outNode = CNewNode("programMemoryLimit");
    CSetContent(outNode, memory_limit);
    CAddNode(test_node, outNode);
}

constexpr int ReturnValueCommandNotFound = 127;

CString RunTestBase::getFailureReason() const {
    TestResult t = getResultReturnValue(this->return_value);
    if (t == TIMEOUT) {
        return "The runtime of the program under test exceeded the set timeout";
    }
    if (t == MEMORY_LIMIT) {
        return "The memory usage of the program under test exceeded the set limit";
    }
    if (t == FAILED) {
        if (return_value == ReturnValueCommandNotFound) {
            return "The program under test returned 'Command not found'";
        }

        if (return_value > 0) {
            return "The program under test returned with failure: " + CString(return_value);
        }

        return "The program under test was aborted by the operating system with code: "
               + CString(return_value);
    }

    return "The test ran with no detected errors.";
}
