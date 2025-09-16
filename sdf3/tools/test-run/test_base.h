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
#ifndef __TEST_BASE__
#define __TEST_BASE__

#include "base/shell/shell.h"
#include "base/xml/xml.h"
#include <memory>

/**
 * RunTestBase
 *
 * Base class used for the different type of testers.
 */
class RunTestBase {
    /**
     * Test status.
     */
public:
    enum TestResult { PASSED, TIMEOUT, MEMORY_LIMIT, FAILED };
    static CString getResult(TestResult t) {
        if (t == PASSED) {
            return "PASSED";
        }
        if (t == TIMEOUT) {
            return "TIMEOUT";
        }
        if (t == MEMORY_LIMIT) {
            return "MEMORY LIMIT";
        }

        return "FAILED";
    }
    static TestResult getResultFromString(const CString &value) {
        if (value == "TIMEOUT") {
            return TIMEOUT;
        }
        if (value == "MEMORY LIMIT") {
            return MEMORY_LIMIT;
        }
        if (value == "PASSED") {
            return PASSED;
        }
        return FAILED;
    }
    static TestResult getResultReturnValue(int rv) {
        if (rv == Shell::RESULT_TIMEOUT) {
            return TIMEOUT;
        }
        if (rv == Shell::RESULT_MEMORY_LIMIT) {
            return MEMORY_LIMIT;
        }
        if (rv == Shell::RESULT_OK) {
            return PASSED;
        }
        return FAILED;
    }

private:
    // Shell used to execute programs.
    std::unique_ptr<Shell> shell;
    // will refer to the stdout and stderr output of the script
    std::unique_ptr<CString> redstdout;
    std::unique_ptr<CString> redstderr;
    // Return value of the script.
    int return_value{};

protected:
    // Constructor.
    RunTestBase() : return_value(-1) {
        shell = createShell(); // NOLINT(*member-initializer)
    }

    // Execute program
    void execute_program();

    // Generate output from the execution off the file.
    void output_shell_testresult(CNode *test_node);

    // get return value
    [[nodiscard]] int get_return_value() const {
        return this->return_value;
    }

    // get shell
    [[nodiscard]] const Shell& get_shell() const {
        return *this->shell;
    }

    // get redstdout
    [[nodiscard]] const CString& get_redstdout() const {
        return *this->redstdout;
    }

    // get redstdout
    [[nodiscard]] const CString& get_redstderr() const {
        return *this->redstderr;
    }

public:
    virtual void run() = 0;
    /// @todo replace this by filename.
    virtual void output(CNode *root_node) = 0;
    virtual int validate();
    // Destructor.
    virtual ~RunTestBase();
    // Prevent copying and moving
    RunTestBase(const RunTestBase&) = delete;
    RunTestBase& operator=(const RunTestBase&) = delete;
    RunTestBase(RunTestBase&&) = delete;
    RunTestBase& operator=(RunTestBase&&) = delete;

    [[nodiscard]] CString getFailureReason() const;
};

#endif
