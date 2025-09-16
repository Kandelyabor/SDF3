/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   basic_types.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF basic type definitions
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: basic_types.h,v 1.2.2.1 2009-03-20 20:33:51 sander Exp $
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

#ifndef SDF_BASIC_TYPES_H_INCLUDED
#define SDF_BASIC_TYPES_H_INCLUDED

#include <cstdint>

using TCnt = uint16_t;
using TBufSize = uint32_t;
using TTime = uint64_t;
using TDtime = double;

#define TDTIME_MAX      ((std::numeric_limits<double>::max)())
#define TBUFSIZE_MAX    ((std::numeric_limits<uint32_t>::max)())
#define TTIME_MAX       ((std::numeric_limits<uint64_t>::max)())


#endif

