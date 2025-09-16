/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   create-detector.h
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   September 15, 2015
 *
 *  Function        :   Create detector scenario that contains the scenario detection actor
 *
 *  History         :
 *      15-09-15    :   Initial version.
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

#ifndef FSMSADF_TRANSFORM_CREATE_SCENARIO_H_INCLUDED
#define FSMSADF_TRANSFORM_CREATE_SCENARIO_H_INCLUDED

#include "fsmsadf/base/graph.h"

namespace FSMSADF
{

    /**
     * createDetectorScenario ()
     * The transformation adds an additional detector scenario that contains
     * the detector actor, whose in- and outgoing edges are connected to the
     * other scenarios by means of switch/select actors.
     */
    void createDetectorScenario(FSM *fsm, Scenarios& scenarios, ScenarioGraphs& scenarioGraphs);

    const Rate stdRate = 1;
    const uint stdTokenSize = 4;

    void checkDetectorActor(FSM *fsm, const Scenarios& scenarios, ScenarioGraphs& scenarioGraphs);
    void addDetectorScenarioToFSM(FSM* fsm, Scenario *ds);
    void extractDetector(const CString& detectorName, const Scenario& s, const Scenario& ds);
    void splitDetector(Actor *detector, Scenario *s);
    void copySwitchesAndSelects(const CString& detectorName, Scenario *s, Scenario *ds);
    void addSynchronizationTokens(Scenario *s, Scenario *ds);
    void resetActorProperties(Scenario *s, Actor *a);
    Port* createPort (Actor* a, Port::PortType t, const CString& n, Scenario* s, Rate r);

} // End namespace FSMSADF

#endif

