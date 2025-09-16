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
#ifndef __TEST_XML_COMPARE_H__
#define __TEST_XML_COMPARE_H__

#include "test_base.h"

class RunTestXmlCompare : public RunTestBase
{
    private:
        bool files_match;
        CNode *test_node;
    public:
        RunTestXmlCompare();

        void run() override;
        int validate() override;

        void output(CNode *root_node) override;

        ~RunTestXmlCompare() override;
        // Prevent copying and moving
        RunTestXmlCompare(const RunTestXmlCompare&) = delete;
        RunTestXmlCompare& operator=(const RunTestXmlCompare&) = delete;
        RunTestXmlCompare(RunTestXmlCompare&&) = delete;
        RunTestXmlCompare& operator=(RunTestXmlCompare&&) = delete;

};

#endif
