/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   basic_types.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 18, 2002
 *
 *  Function        :   Basic type definitions
 *
 *  History         :
 *      18-06-04    :   Initial version.
 *      26-08-06    :   Additions by Bart Theelen.
 *
 * $Id: basic_types.h,v 1.7.2.2 2009-11-17 13:50:15 mgeilen Exp $
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

#ifndef SDF3_BASE_BASIC_TYPES_H_INCLUDED
#define SDF3_BASE_BASIC_TYPES_H_INCLUDED

#include <cassert>
#include <cstdint>

/* include default STL functionality */
// IWYU pragma: begin_keep
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>
// IWYU pragma: end_keep

/* basic types */
using uint = unsigned int;
using v_int = std::vector<int>;
using v_uint = std::vector<uint>;

/* Id */
using CId = uint;
using CSize = uint64_t;

/* Doubles */
using CDouble = double;

/* Queue */
using CQueue = std::queue<CId>;

#ifdef _MSC_VER
using ulong = uint32_t;
#define CSIZE_MAX ULONG_MAX
#undef LONG_LONG_MAX
#define LONG_LONG_MAX LONG_MAX
#undef LONG_LONG_MIN
#define LONG_LONG_MIN LONG_MIN

#else
#define CSIZE_MAX ULONG_LONG_MAX
#endif

#define CID_MAX                 UINT_MAX



/**
*  TriBool
*    'tristate boolean' - has 'undefined' state.
*/

class TriBool
{
    public:
        TriBool() = default;
        explicit TriBool(bool setVal) : defined(true), val(setVal) {}
        explicit operator bool() const
        {
            assert(defined);
            return val;
        }
        [[nodiscard]] bool isDefined() const
        {
            return defined;
        }
    private:
        bool defined{false}, val{false};
};

#endif

