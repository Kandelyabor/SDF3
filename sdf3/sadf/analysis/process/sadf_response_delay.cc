/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_response_delay.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of response delay
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#include "sadf_response_delay.h"

#include <algorithm>

// Functions to analyse response delay

CSize SADF_Analyse_ResponseDelay(SADF_Graph *Graph,
                                 CId ProcessType,
                                 CId ProcessID,
                                 CDouble &Expected,
                                 CDouble &Minimum,
                                 CDouble &Maximum) {

    // Check whether graph satisfied required properties

    if (!SADF_Verify_SingleComponent(Graph)) {
        throw CException(CString("Error: SADF graph '") + Graph->getName()
                         + "' does not consist of a single component.");
    }

    if (!SADF_Verify_Timed(Graph)) {
        throw CException(CString("Error: SADF graph '") + Graph->getName() + "' is not timed.");
    }

    // Construct TPS

    SADF_TPS TPS(Graph);
    SADF_Configuration *initialConfiguration = TPS.getInitialConfiguration().get();

    SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_InterFiringLatency(
            Graph, TPS, initialConfiguration, ProcessType, ProcessID);

    while (!NewConfigurations.empty()) {
        NewConfigurations.pop_front();
}

    // Compute Results

    Expected = 0;
    Minimum = SADF_MAX_DOUBLE;
    Maximum = 0;

    for (const auto& tr: initialConfiguration->getTransitions()) {
        Expected += tr->getProbability() * tr->getTimeSample();

        Minimum = (std::min)(tr->getTimeSample(), Minimum);
        Maximum = (std::max)(tr->getTimeSample(), Maximum);
    }

    // Print Distribution

    /*  std::vector<CDouble> Probabilities;
        std::vector<CDouble> Values;

        for  (std::list<SADF_Transition *>::iterator i =
       InitialConfiguration->getTransitions().begin(); i !=
       InitialConfiguration->getTransitions().end(); i++) {

            int Index = -1;
            for (CId x = 0; (Index == -1) && (x != Values.size()); x++)
                if (Values[x] == (*i)->getTimeSample())
                    Index = x;

            if (Index == -1) {
                Values.push_back((*i)->getTimeSample());
                Probabilities.push_back((*i)->getProbability());
            } else
                Probabilities[Index] += (*i)->getProbability();
        }

        std::ofstream out;
        CString OutputFileName = "response_delay_distribution.txt";
        out.open(OutputFileName.c_str());
        for (CId x = 0; x != Values.size(); x++)
            out << Values[x] << "   " << Probabilities[x] << std::endl;
        out.close();
    */
    CSize NumberOfConfigurations = TPS.getNumberOfConfigurations();

    return NumberOfConfigurations;
}
