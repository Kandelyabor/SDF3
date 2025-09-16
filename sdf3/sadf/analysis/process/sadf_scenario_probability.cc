/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_scenario_probability.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of scenario probabilities
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

#include "sadf_scenario_probability.h"

// Function to construct TPS for analyzing scenario probabilities

namespace {


void SADF_ConstructTPS_ScenarioProbabilities(SADF_Graph *Graph,
                                             SADF_TPS &TPS,
                                             SADF_Configuration *Source,
                                             CId ProcessType,
                                             CId ProcessID) {
    SADF_ListOfConfigurations NewConfigurations =
            SADF_ProgressTPS_ASAP_ScenarioProbabilities(Graph, TPS, Source, ProcessType, ProcessID);

    // Fix transition probabilities

    // copy the original transitions
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
        SADF_ConstructTPS_ScenarioProbabilities(
                Graph, TPS, NewConfigurations.front().get(), ProcessType, ProcessID);
        NewConfigurations.pop_front();
    }
}

} // namespace

// Functions to analyse scenario probabilities

CSize SADF_Analyse_ScenarioProbabilities(SADF_Graph *Graph,
                                         CId ProcessType,
                                         CId ProcessID,
                                         std::vector<CDouble> &probabilities) {

    // Check whether graph satisfied required properties

    if (!SADF_Verify_SingleComponent(Graph)) {
        throw CException(CString("Error: SADF graph '") + Graph->getName()
                         + "' does not consist of a single component.");
    }

    CSize NumberOfConfigurations = 0;

    // Special case: kernel without control inputs

    if (ProcessType == SADF_KERNEL) {
        if (!Graph->getKernel(ProcessID)->hasControlInputChannels()) {
            probabilities[0] = 1;
            return NumberOfConfigurations;
        }
    }

    // Special cases: single Markov chain that controls scenario occurrences

    bool SpecialCase = false;

    if (ProcessType == SADF_KERNEL) {
        if (!Graph->getKernel(ProcessID)->hasMultipleControlInputChannels()) {
            if (!Graph->getKernel(ProcessID)
                         ->getControlInputChannels()[0]
                         ->getSource()
                         ->hasControlInputChannels()) {
                SpecialCase = true;
            }
        }
    }

    if (ProcessType == SADF_DETECTOR) {
        if (!Graph->getDetector(ProcessID)->hasControlInputChannels()) {
            SpecialCase = true;
        }
    }

    if (SpecialCase) {
        SADF_Process *Detector = Graph->getDetector(0);
        SADF_MarkovChain *MarkovChain = Detector->getMarkovChain(0);
        std::vector<CDouble> equilibriumDistribution =
                MarkovChain->computeEquilibriumDistribution();

        if (ProcessType == SADF_KERNEL) {
            for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
                probabilities[Graph->getKernel(ProcessID)
                                      ->getControlInputChannels()[0]
                                      ->getProductionScenarioID(MarkovChain->getSubScenario(i))] +=
                        equilibriumDistribution[i];
            }
        }

        if (ProcessType == SADF_DETECTOR) {
            for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
                probabilities[MarkovChain->getSubScenario(i)] += equilibriumDistribution[i];
            }
        }

        return NumberOfConfigurations;
    }

    // Construct TPS

    bool Ergodic = SADF_Verify_SimpleErgodicity(
            Graph); // Only in case Ergodic is true, the SADF graph is ergodic for sure. Otherwise,
                    // ergodicity test needed after generating TPS

    SADF_TPS TPS(Graph);
    SADF_ConstructTPS_ScenarioProbabilities(
            Graph, TPS, TPS.getInitialConfiguration().get(), ProcessType, ProcessID);

    TPS.removeTransientConfigurations();

    if (!Ergodic) {
        if (!TPS.isSingleStronglyConnectedComponent()) {
            throw CException(CString("Error: SADF graph '") + Graph->getName()
                             + "' does not imply an ergodic Markov chain.");
        }
    }

    NumberOfConfigurations = TPS.getNumberOfConfigurations();

    std::vector<CDouble> EquilibriumDistribution = TPS.computeEquilibriumDistribution();

    // Compute Results

    for (const auto &configurations : TPS.getConfigurationSpace()) {
        for (const auto &c : configurations) {
            if (ProcessType == SADF_KERNEL) {
                probabilities[c->getKernelStatus(ProcessID)->getState()->getScenario()] +=
                        EquilibriumDistribution[c->getIdentity()];
            } else {
                probabilities[c->getDetectorStatus(ProcessID)->getState()->getSubScenario()] +=
                        EquilibriumDistribution[c->getIdentity()];
            }
        }
    }

    return NumberOfConfigurations;
}
