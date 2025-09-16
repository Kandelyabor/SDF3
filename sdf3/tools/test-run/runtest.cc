/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   runtest.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *                      Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   August 10, 2010
 *
 *  Function        :   Program to run all tests
 *
 *  History         :
 *      10-08-10    :   Initial version.
 *      17-11-11    :   Initial version take 2 (mk)
 *      29-10-15    :   Manually merged in tests from GlobalSettings
 *
 * $Id: runtest.cc,v 1.1.4.9 2010-08-18 07:51:51 mgeilen Exp $
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
#include "base/exception/exception.h"
#include "runtest.h"
#include "test_base.h"
#include "test_command.h"
#include "test_output_compare.h"
#include "test_xml_compare.h"
#include <iostream>
#include <memory>

namespace RunTestGlobals {

std::unique_ptr<TestSettings> settings = nullptr; // NOLINT(*global-variables)

} // namespace RunTestGlobals

using namespace RunTestGlobals;

class TestSettings2 : public  TestSettings
{
    public:

        enum TestType { XML_COMPARE, TXT_COMPARE, PROGRAM};

        TestSettings2(const CString& module, const CString& tool)
            :
            TestSettings(module, tool)
        {
            // Type of test.
            addValue("type", "unset", "The type of test to perform", TestSettings::SINGLE);

            // Settings file to load. (ignored for html mode)
            addValue("settings", "sdf3.opt", "The settings file", TestSettings::SINGLE);


            addValue("name", "no name set", "The test name");
            // Program to run (not in html mode)
            addValue("program", "",
                     "The program to run");
            addValue("programOutput", "",
                     "The output(s) of the program under test", TestSettings::MULTIPLE);
            addValue("programTimeout", "10.0",
                     "The maximum runtime off the program under test (double) in seconds.");
            addValue("programMemoryLimit", "1024",
                     "The maximum amount of memory (in 10⁶ bytes) the program is allowed to use.");

            addValue("output", "output.xml", "The test result output file.");

            addValue("ignore", "", "Output compare, ignore", TestSettings::MULTIPLE);
        }


        void printProgramHeader(std::ostream &out) override
        {
            out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
            out << std::endl;
        }

};

namespace {

/**
 * Run test
 */
int runTest()
{
    std::unique_ptr<RunTestBase> tb = nullptr;
    CString type = settings->getValue("type");

    // Switch statement
    if (type == "program")
    {
        tb = std::make_unique<RunTestCommand>();
    }
    else if (type == "xmlcompare")
    {
        tb = std::make_unique<RunTestXmlCompare>();
    }
    else if (type == "outputcompare")
    {
        tb = std::make_unique<RunTestOutputCompare>();
    }
    else
    {
        throw CException("Type: " + type + " is not supported");
    }

    // Run the experiment.
    tb->run();

    // validate result
    int result = tb->validate();

    // Writing
    std::unique_ptr<CDoc> result_doc = CNewDoc();
    CNode *root_node = CNewNode("sdf3");
    result_doc->setRootNode(root_node);

    // Write result output
    tb->output(root_node);

    // Write out file.
    if (settings->getValue("output").empty())
    {
        result_doc->saveFile(std::cout, 1);
    }
    else
    {
        result_doc->saveFile(settings->getValue("output"), 1);
    }
    return result;
}

} // namespace

using namespace std;


/**
 * main ()
 */
/**
 * Run this like sdf3test <testscript>
 * Only 1 test per time is allowed.
 * Tests can be off different types.
 * 1. XML output compare.
 * 2. Output compare.
 * 3. Test program. (so return value is tested).
 *
 * the test file should be structured like:
 *
 * <name>Name of test</name>
 * <type>Type of test </name>
 * <output>file</output>
 * <programOutput>outputfile(s)</programOutput>
 *
 * References files should be named name.ref.
 */
int main(int argc, char **argv)
{
    int exit_status = EXIT_SUCCESS;

    try
    {
        settings = std::make_unique<TestSettings2>(MODULE, SETTINGS_TYPE);

        // parse command line argument.
        if (settings->parseArguments(argc, argv))
        {
            return exit_status;
        }

        // load option file for the test.
        settings->parseOptionFile();

        exit_status = runTest();
    }
    catch (CException &e)
    {
        std::cerr << e;
        exit_status = EXIT_FAILURE;
    }

    // Clear out xml code.
    xmlCleanup();
    return exit_status;
}

