
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   com_hsdf_ip.h
 *
 *  Author          :   Xue-Yang Zhu
 *
 *  Date            :   2012
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

/*comHSDFIP()
 * compute max execution time of the longest zero
 *  delay path to each actor in the HSDFG h
 * */
#ifndef SDF_COMPUTE_HSDF_IP_H_INCLUDED
#define SDF_COMPUTE_HSDF_IP_H_INCLUDED
#include "sdf/base/timed/graph.h"

namespace SDF
{

/**
 * Max Execution time  vector
 */
using MaxExTimeVector = std::vector<int>;

MaxExTimeVector comHSDFIP(const TimedSDFgraph& h);

} // namespace SDF
#endif
