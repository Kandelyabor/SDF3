/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   schedule_encode.h
 *
 *  Author          :   Morteza Damavandpeyma (m.damavandpeyma@tue.nl)
 *
 *  Date            :   June 23, 2011
 *
 *  Function        :   static periodic schedule encoding in (SDF) graph
 *
 *  History         :
 *      23-03-11    :   Initial version.
 *      02-11-15    :   Adaptation and refactoring for use in fsm-sadf (Reinier)
 *
 * $Id: schedule_encode.h,v 1.0.0.0 2011-06-23 10:59:47 morteza Exp $
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

#ifndef FSMSADF_TRANSFORM_DP_MAX_PROGRESS_H_INCLUDED
#define FSMSADF_TRANSFORM_DP_MAX_PROGRESS_H_INCLUDED

#include "fsmsadf/base/graph.h" // IWYU pragma: keep
#include "fsmsadf/resource_allocation/scheduling/static_order/static_order_schedule.h"

namespace FSMSADF {

/**
 * actorAppearanceBefore ()
 * The function counts the number of times that the element a is appeared
 * in vector sos from point 0 in the vector to point sp in the vector.
 */
uint actorAppearanceBefore(StaticOrderSchedule sos, const CString &a, uint sp);

/**
 * actorAppearanceAfter ()
 * The function counts the number of times that the element a is appeared
 * in vector sos from point sp+1 in the vector to the end of the vector.
 */
uint actorAppearanceAfter(StaticOrderSchedule sos, const CString &a, uint sp);

/**
 * isRequired ()
 * The function determines whether actor a in place sp is necessary to
 * consider in decision points list dps.
 */
bool isRequired(const CString &a, uint sp, std::vector<std::vector<CString>> dps);

/**
 * optimizeDecisionPoints ()
 * The function removes unnecessary decision points from decision point list (dps)
 */
std::vector<std::vector<CString>> optimizeDecisionPoints(std::vector<std::vector<CString>> dps);

} // End namespace FSMSADF

#endif
