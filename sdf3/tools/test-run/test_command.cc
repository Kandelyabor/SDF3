/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   test_command.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   November 4, 2011
 *
 *  Function        :   Test Run Command
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
#include "tools/test-settings/testsettings.h"
#include "test_command.h"
#include "tools/test-run/runtest.h"


using namespace RunTestGlobals;

void RunTestCommand::output(CNode *root_node)
{
    CNode *test = CNewNode("test");
    CAddAttribute(test, "name", settings->getValue("name"));

    // type
    CNode *outNode = CNewNode("type");
    CSetContent(outNode, "Command Test");
    CAddNode(test, outNode);
    // Result
    CNode *resultNode = CNewNode("result");
    CSetContent(resultNode,
                RunTestBase::getResult(getResultReturnValue(this->get_return_value())));
    CAddNode(test, resultNode);
    output_shell_testresult(test);

    CString failure_reason = this->getFailureReason();
    CNode *fr = CNewNode("description");
    CSetContent(fr, failure_reason);
    CAddNode(test, fr);

    CAddNode(root_node, test);
}

void RunTestCommand::run()
{
    this->execute_program();
}
