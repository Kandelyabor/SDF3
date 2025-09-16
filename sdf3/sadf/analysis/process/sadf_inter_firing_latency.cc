/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_inter_firing_latency.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of inter firing time
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

#include "sadf_inter_firing_latency.h"

#include <algorithm>

// Function to construct TPS for analyzing inter-firing latency metrics

namespace {


void SADF_ConstructTPS_InterFiringLatency(SADF_Graph *Graph,
                                          SADF_TPS& TPS,
                                          SADF_Configuration *Source,
                                          CId ProcessType,
                                          CId ProcessID) {

    SADF_ListOfConfigurations NewConfigurations =
            SADF_ProgressTPS_ASAP_InterFiringLatency(Graph, TPS, Source, ProcessType, ProcessID);

    // Determine local results

    Source->initialiseLocalResults(4);

    CDouble AverageInterFiringTime = 0;
    CDouble AverageSquaredInterFiringTime = 0;

    CDouble Minimum = SADF_MAX_DOUBLE;
    CDouble Maximum = 0;

    for (const auto& tr : Source->getTransitions()) {
        CDouble Temp = tr->getProbability() * tr->getTimeSample();

        AverageInterFiringTime += Temp;
        AverageSquaredInterFiringTime += Temp * tr->getTimeSample();

        Maximum = (std::max)(tr->getTimeSample(), Maximum);
        Minimum = (std::min)(tr->getTimeSample(), Minimum);
    }

    Source->setLocalResult(0, AverageInterFiringTime);
    Source->setLocalResult(1, AverageSquaredInterFiringTime);

    Source->setLocalResult(2, Minimum);
    Source->setLocalResult(3, Maximum);

    // Fix transition probabilities (individual time samples lost)

    SADF_ListOfTransitions originalTransitions(Source->getTransitions());

    Source->removeAllTransitions();

    for (const auto& tr : originalTransitions) {

        SADF_Transition *existingTransition = Source->hasTransitionToConfigurationWithIdentity(
                tr->getDestination()->getIdentity());

        if (existingTransition == nullptr) {
            Source->addTransition(tr->getDestination(), tr->getProbability(), 0);
        } else {
            existingTransition->setProbability(existingTransition->getProbability()
                                               + tr->getProbability());
        }

    }

    // Proceed constructing TPS from new configurations

    while (!NewConfigurations.empty()) {
        SADF_ConstructTPS_InterFiringLatency(
                Graph, TPS, NewConfigurations.front().get(), ProcessType, ProcessID);
        NewConfigurations.pop_front();
    }
}

} // namespace


// Function to analyse inter-firing latency

CSize SADF_Analyse_LongRunInterFiringLatency(
        SADF_Graph *Graph, CId ProcessType, CId ProcessID, CDouble &Average, CDouble &Variance) {

    // Check whether graph satisfied required properties

    if (!SADF_Verify_SingleComponent(Graph)) {
        throw CException(CString("Error: SADF graph '") + Graph->getName()
                         + "' does not consist of a single component.");
    }

    if (!SADF_Verify_Timed(Graph)) {
        throw CException(CString("Error: SADF graph '") + Graph->getName() + "' is not timed.");
    }

    bool Ergodic = SADF_Verify_SimpleErgodicity(
            Graph); // Only in case Ergodic is true, the SADF graph is ergodic for sure. Otherwise,
                    // ergodicity test needed after generating TPS

    // Construct TPS

    SADF_TPS TPS(Graph);
    SADF_ConstructTPS_InterFiringLatency(
            Graph, TPS, TPS.getInitialConfiguration().get(), ProcessType, ProcessID);

    TPS.deleteContentOfConfigurations();
    TPS.removeTransientConfigurations();

    if (!Ergodic) {
        if (!TPS.isSingleStronglyConnectedComponent()) {
            throw CException(CString("Error: SADF graph '") + Graph->getName()
                             + "' does not imply an ergodic Markov chain.");
        }
    }

    CSize NumberOfConfigurations = TPS.getNumberOfConfigurations();

    std::vector<CDouble> EquilibriumDistribution = TPS.computeEquilibriumDistribution();

    // Print Distribution

    /*  std::vector<CDouble> Probabilities;
        std::vector<CDouble> Values;

       for (SADF_HashedListOfConfigurations::iterator i = TPS.getConfigurationSpace().begin(); i !=
       TPS.getConfigurationSpace().end(); i++) for (SADF_ListOfConfigurations::iterator j =
       (*i).begin(); j != (*i).end(); j++) {

                int Index = -1;
                for (CId x = 0; (Index == -1) && (x != Values.size()); x++)
                    if (Values[x] == (*j)->getLocalResult(0))
                        Index = x;

                if (Index == -1) {
                    Values.push_back((*j)->getLocalResult(0));
                    Probabilities.push_back(EquilibriumDistribution[(*j)->getIdentity()]);
                } else
                    Probabilities[Index] += EquilibriumDistribution[(*j)->getIdentity()];
            }

        std::ofstream out;
        CString OutputFileName = "inter_firing_delay_distribution.txt";
        out.open(OutputFileName.c_str());
        for (CId x = 0; x != Values.size(); x++)
            out << Values[x] << "   " << Probabilities[x] << std::endl;
        out.close();
    */
    // Compute Results

    Average = 0;
    CDouble AverageSquared = 0;

    for (const auto &configurations : TPS.getConfigurationSpace()) {
        for (const auto& c : configurations) {
            Average += EquilibriumDistribution[c->getIdentity()] * c->getLocalResult(0);
            AverageSquared += EquilibriumDistribution[c->getIdentity()] * c->getLocalResult(1);
        }
    }

    Variance = AverageSquared - (Average * Average);

    // Circumvent rounding errors
    Variance = std::max<CDouble>(Variance, 0);

    return NumberOfConfigurations;
}

// Function to analyse extreme inter-firing latency

CSize SADF_Analyse_ExtremeInterFiringLatency(
        SADF_Graph *Graph, CId ProcessType, CId ProcessID, CDouble &Minimum, CDouble &Maximum) {

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
    SADF_ConstructTPS_InterFiringLatency(
            Graph, TPS, TPS.getInitialConfiguration().get(), ProcessType, ProcessID);

    // Compute Results

    Minimum = SADF_MAX_DOUBLE;
    Maximum = 0;

    CSize NumberOfConfigurations = TPS.getNumberOfConfigurations();

    for (const auto &configurations : TPS.getConfigurationSpace()) {
        for (const auto& c : configurations) {

            Minimum = (std::min)(c->getLocalResult(2), Minimum);
            Maximum = (std::max)(c->getLocalResult(3), Maximum);
        }
    }

    return NumberOfConfigurations;
}
