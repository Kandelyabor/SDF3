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
#ifndef __TEST_BASE_COMMAND__
#define __TEST_BASE_COMMAND__
#include "test_base.h"

/**
 * RunTestCommand
 *
 * Class for unit tests. A test is failed/passed based
 * on the return value of the test.
 */
class RunTestCommand: public RunTestBase
{
    public:
        RunTestCommand() = default;

        // Run it.
        void run() override;

        // output.
        void output(CNode *root_node) override;

        // Deconstructor.
        ~RunTestCommand() override = default;

        // Prevent copying and moving
        RunTestCommand(const RunTestCommand&) = delete;
        RunTestCommand& operator=(const RunTestCommand&) = delete;
        RunTestCommand(RunTestCommand&&) = delete;
        RunTestCommand& operator=(RunTestCommand&&) = delete;

};
#endif

