/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   helper.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Aug 9, 2011
 *
 *  Function        :   Output format conversion
 *
 *  History         :
 *      09-08-11    :   Initial version.
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

#include "helper.h"
#include "exception/exception.h"
// regular expression support
#include <regex>

namespace FSMSADF
{
    /**
     * Converts a string of {1,2,3,4} into a vector.
     * Strings in shorthand like {2x1,3,3x4} are turned in to a vector
     * {1,1,3,4,4,4}
     *
     * Throws an #CException if the input cannot be parsed.
     *
     * @param input a list of , separated values and/or shorthand.
     * @return a vector with CSting elements.
     */
    std::vector<CString> expand(const CString& input)
    {
        std::vector<CString> retV;
        CStrings elems = input.split(',');

        for (const auto& element: elems)
        {
            CStrings t = (element).split('x');

            if (t.size() > 1)
            {
                if (t.size() != 2) {
                    throw CException("String is incorrectly formatted");
}

                const CString& times = t.front();
                const CString& value = t.back();

                for (int j = static_cast<int>(times); j > 0; j--)
                {
                    retV.push_back(value);
                }
            }

            else
            {
                CString value = element;
                retV.push_back(value);
            }
        }

        return retV;
    }

    /**
     * Helper function
     * @param child A child #CNode of the the document.
     *
     * This function returns the toplevel node of #child.
     * It does this by going down the tree using the parent pointer.
     *
     * @return the root node of #child
     */
    CNode *getRootNode(CNode *child)
    {
        ASSERT(child != nullptr, CString(__FUNCTION__) + ": Child != FAILED");
        CNode *root = nullptr;
        CNode *temp = child;

        do
        {
            root = temp;
            temp = CGetParentNode(temp);
        }
        while (temp != nullptr);

        return root;
    }

    /**
     * Helper function.
     * Gets the numeric id of the memory.
     * So cmemin4 -> 4, cmemout0->0
     *
     * @param id the name of the memory f.e. cmemin4 or cmemout0
     * @return -1 when not found. >= 0 when found
     */
    int HelperGetMemId(const CString& id)
    {
        int res_id = -1;
        std::cmatch what;
        std::regex expression("(cmemin|cmemout|dma)([0-9+])");

        if (std::regex_match((const char *) id, what, expression))  // NOLINT(*casting)
        {
            res_id = std::strtol(what[2].first, nullptr, 10); // NOLINT(*magic-numbers)
        }

        return res_id;
    }
}  // namespace FSMSADF
