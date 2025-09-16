/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   sdfmcmgraph.h
*
*  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
*
*  Date            :   November 7, 2005
*
*  Function        :   Convert HSDF graph to weighted directed graph for MCM
*                      calculation.
*
*  History         :
*      07-11-05    :   Initial version.
*
* $Id: mcmgraph.h,v 1.1.1.1.2.2 2010-08-18 07:51:47 mgeilen Exp $
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

#ifndef SDF_ANALYSIS_MCM_MCMGRAPH_H_INCLUDED
#define SDF_ANALYSIS_MCM_MCMGRAPH_H_INCLUDED


#include "sdf/base/timed/graph.h"
#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include <memory>

namespace Graphs {

    /**
    * transformHSDFtoMCMgraph ()
    * The function converts an HSDF graph to a weighted directed graph
    * used in the MCM algorithm of Karp (and its variants). By default, the
    * a longest path calculation is performed to make the graph suitable
    * as input for an MCM algorithm. Setting the flag 'mcmFormulation' to false
    * result in an MCM graph which is suitable for an MCR (cycle ratio)
    * formulation. This avoids running the longest path algo.
    */
    std::unique_ptr<::MaxPlus::Graphs::MCMgraph> transformHSDFtoMCMgraph(const SDF::TimedSDFgraph& g, bool mcmFormulation = true);

} // namespace Graphs

#endif