/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   merge-scenarios.h
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   March 16, 2015
 *
 *  Function        :   Merge all scenarios for execution on CompSoC platform
 *
 *  History         :
 *      16-03-16    :   Initial version.
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

#ifndef FSMSADF_TRANSFORM_MERGE_SCENARIOS_H_INCLUDED
#define FSMSADF_TRANSFORM_MERGE_SCENARIOS_H_INCLUDED

#include "fsmsadf/base/graph.h"
#include "fsmsadf/resource_allocation/platform_binding/graph.h"
#include <memory>


namespace FSMSADF
{

    std::unique_ptr<Graph> mergeAllScenarios(PlatformGraph *pg, PlatformBindings *pbs);

    void checkDetectorScenario (Graph *ag, PlatformBinding *pb);
    std::unique_ptr<Graph> reduceApplicationGraph(Graph *ag);

    void mergeSwitchOrSelect(Actor *src, Actor *detect, const CString& oldScenarioName);
    void addActorToScenario(Actor *oldActor, Scenario *oldScenario, Scenario *newScenario, std::set<CString> *addedActors);
    void addChannelToScenario(Channel *oldChannel, Scenario *oldScenario, Scenario *newScenario);

    std::unique_ptr<PlatformBinding> reduceMapping(Graph* oldAg, Graph* newAg, PlatformGraph *pg, PlatformBinding *oldPb);

} // End namespace FSMSADF

#endif

