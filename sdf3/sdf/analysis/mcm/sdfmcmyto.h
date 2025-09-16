/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdfmcmyto.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 28, 2011
 *
 *  Function        :   Compute the MCM for an HSDF graph using Young-
 *                      Tarjan-Orlin's algorithm.
 *
 *  History         :
 *      28-03-11    :   Initial version.
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

#ifndef SDF_ANALYSIS_MCM_MCMYTO_H_INCLUDED
#define SDF_ANALYSIS_MCM_MCMYTO_H_INCLUDED

#include "base/basic_types.h"
#include "sdf/base/timed/graph.h"

namespace Graphs {
/**
 * mcmYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Young-Tarjan-Orlin's algorithm.
 */
static CDouble mcmYoungTarjanOrlin(const SDF::TimedSDFgraph& g, bool mcmFormulation);

/**
 * maximumCycleMeanYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Young-Tarjan-Orlin's algorithm.
 */
CDouble maximumCycleYoungTarjanOrlin(const SDF::TimedSDFgraph& g, bool mcmFormulation);
} // namespace Graphs

#endif