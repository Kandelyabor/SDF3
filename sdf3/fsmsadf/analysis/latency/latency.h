/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   latency.h
*
*  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
*
*  Date            :   July 30, 2017
*
*  Function        :   provides latency analysis functions for regex-based FSMSADF

*  History         :
*      12-09-17    :   Initial version.
*
* $Id:   $
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


#ifndef FSMSADF_REGEX_LATENCY_H_INCLUDED
#define FSMSADF_REGEX_LATENCY_H_INCLUDED

#include "fsmsadf/analysis/latency/systemmatrices.h"
#include "fsmsadf/base/graph.h"

namespace FSMSADF
{
 
    /**
    computes the system matrices of all scenarios in a weakly-consistent FSMSADF
    */
    SystemsMatrices* computeSystemMatricesFromScenariosForWeakFSMSADF(Graph *g);

    /**
    * latencyAnalysisGraphUsingRegex()
    * Compute the latency of a weakly consistent regex-based SADF graph.
    */
    void latencyAnalysisGraphUsingRegex(Graph *g, MaxPlus::MPTime mu, std::ostream &out);
} // namespace FSMSADF
#endif
