/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dsm_encode.h
 *
 *  Author          :   Morteza Damavandpeyma (m.damavandpeyma@tue.nl)
 *
 *  Date            :   March 23, 2011
 *
 *  Function        :   Decision state modelling for
 *                       static periodic schedule encoding in (SDF) graph
 *
 *  History         :
 *      23-03-11    :   Initial version.
 *      02-11-15    :   Adaptation and refactoring for use in fsm-sadf (Reinier)
 *
 * $Id: schedule_encode.h,v 1.0.0.0 2011-03-23 10:59:47 morteza Exp $
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

#ifndef FSMSADF_TRANSFORM_DSM_ENCODE_H_INCLUDED
#define FSMSADF_TRANSFORM_DSM_ENCODE_H_INCLUDED

#include "dp_max_progress.h" // IWYU pragma: keep
#include "decision_points.h" // IWYU pragma: keep
#include <memory>

namespace FSMSADF
    {

    std::unique_ptr<ScenarioGraph> limitInterIterationExecution (Scenario *s, const CString& procName, const CString& schedName, ScenarioGraph *sg, StaticOrderSchedule sos);

    std::unique_ptr<ScenarioGraph> addScheduleEncodingComponents (Scenario *s, const CString& procName, const CString& schedName, ScenarioGraph *sg, const StaticOrderSchedule& sos, std::vector<std::vector<CString> > dps);

    std::unique_ptr<ScenarioGraph> encodeSchedule(Scenario *s, const CString& procName, const CString& schedName, ScenarioGraph *sg, const StaticOrderSchedule& sos);

    void setChannelProperties (Scenario *s, Channel *c, Rate srcRate, Rate dstRate, uint initialTokens, const CString& procName, const CString& schedName, ScenarioGraph *newSg);

}    // End namespace FSMSADF

#endif
