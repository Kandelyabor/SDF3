/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_configuration.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Configuration
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#include "sadf_configuration.h"
#include "sadf_detector_status.h"
#include "sadf_kernel_status.h"
#include "sadf_tps.h"

// Constructors

SADF_Configuration::SADF_Configuration(SADF_Graph *Graph, SADF_TPS& TPS, CId StepType) :
    HashKey(0.0), StepValue(0.0) {

    KernelStatus.resize(Graph->getNumberOfKernels());
    DetectorStatus.resize(Graph->getNumberOfDetectors());
    ChannelStatus.resize(Graph->getNumberOfDataChannels());
    ControlStatus.resize(Graph->getNumberOfControlChannels());

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
        KernelStatus[i] =
                std::make_unique<SADF_KernelStatus>(this, TPS.getInitialKernelState(i), SADF_MAX_DOUBLE);
    }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
        DetectorStatus[i] =
                std::make_unique<SADF_DetectorStatus>(this, TPS.getInitialDetectorState(i), SADF_MAX_DOUBLE);
    }

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
        ChannelStatus[i] = std::make_unique<SADF_ChannelStatus>(Graph->getDataChannel(i));
    }

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
        ControlStatus[i] = std::make_unique<SADF_ControlStatus>(Graph->getControlChannel(i));
    }

    MinimalRemainingExecutionTime = SADF_MAX_DOUBLE;

    setType(StepType);

    computeHashKey();

    Relevant = false;
    Marking = false;
}

SADF_Configuration::SADF_Configuration(SADF_Graph *Graph,
                                       SADF_Configuration *C,
                                       CId StepType,
                                       CDouble Value) :
    HashKey() {

    KernelStatus.resize(Graph->getNumberOfKernels());
    DetectorStatus.resize(Graph->getNumberOfDetectors());
    ChannelStatus.resize(Graph->getNumberOfDataChannels());
    ControlStatus.resize(Graph->getNumberOfControlChannels());

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
        KernelStatus[i] = std::make_unique<SADF_KernelStatus>(this, C->getKernelStatus(i));
    }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
        DetectorStatus[i] = std::make_unique<SADF_DetectorStatus>(this, C->getDetectorStatus(i));
    }

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
        ChannelStatus[i] = std::make_unique<SADF_ChannelStatus>(C->getChannelStatus(i));
    }

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
        ControlStatus[i] = std::make_unique<SADF_ControlStatus>(C->getControlStatus(i));
    }

    MinimalRemainingExecutionTime = C->getMinimalRemainingExecutionTime();

    setType(StepType);
    StepValue = Value;

    Relevant = false;
    Marking = false;
}

// Destructors

SADF_Configuration::~SADF_Configuration() = default;

void SADF_Configuration::deleteContent() {
    KernelStatus.resize(0);
    DetectorStatus.resize(0);
    ChannelStatus.resize(0);
    ControlStatus.resize(0);
}

// Computation of hash key

// NOLINTBEGIN(*-magic-numbers)

void SADF_Configuration::computeHashKey() {

    HashKey = getType() * 0.6180339887;

    for (auto &KernelStatus : KernelStatus) {
        HashKey = HashKey * 39164205.20662217
                  + KernelStatus->getState()->getScenario() * 0.6180339887;

        if (KernelStatus->getRemainingExecutionTime() != SADF_MAX_DOUBLE) {
            HashKey = HashKey * 39164205.20662217
                      + KernelStatus->getRemainingExecutionTime() * 0.6180339887;
        }
    }

    for (auto &DetectorStatus : DetectorStatus) {
        HashKey = HashKey * 39164205.20662217
                  + DetectorStatus->getState()->getScenario() * 0.6180339887;

        if (DetectorStatus->getRemainingExecutionTime() != SADF_MAX_DOUBLE) {
            HashKey = HashKey * 39164205.20662217
                      + DetectorStatus->getRemainingExecutionTime() * 0.6180339887;
        }

        for (CId j = 0; j != DetectorStatus->getState()->getMarkovChainStatus().size(); j++) {
            HashKey = HashKey * 39164205.20662217
                      + DetectorStatus->getState()->getStateOfMarkovChain(j) * 0.6180339887;
        }
    }

    for (auto &ChannelStatus : ChannelStatus) {
        HashKey = HashKey * 39164205.20662217 + ChannelStatus->getAvailableTokens() * 0.6180339887;
    }

    for (auto &ControlStatus : ControlStatus) {
        HashKey = HashKey * 39164205.20662217 + ControlStatus->getAvailableTokens() * 0.6180339887;
    }
}

// NOLINTEND(*-magic-numbers)

// Functions to access transitions

void SADF_Configuration::addTransition(SADF_Configuration *Destination,
                                       CDouble Probability,
                                       CDouble TimeSample) {

    Transitions.push_front(std::make_unique<SADF_Transition>(Destination, Probability, TimeSample));
}

SADF_Transition *SADF_Configuration::hasTransitionToConfigurationWithIdentity(const CId ID) {

    for (const auto& tr : Transitions) {
        if (tr->getDestination()->getIdentity() == ID) {
            return tr.get();
        }
    }

    return nullptr;
}

CDouble SADF_Configuration::getTransitionProbabilityTo(SADF_Configuration *Destination) {

    for (const auto& tr : Transitions) {
        if (tr->getDestination()->equal(Destination)) {
            return tr->getProbability();
        }
    }

    return 0;
}

void SADF_Configuration::removeAllTransitions() {

    while (!Transitions.empty()) {
        Transitions.pop_front();
    }
}

void SADF_Configuration::deleteAllTransitions() {

    while (!Transitions.empty()) {
        Transitions.pop_front();
    }
}

void SADF_Configuration::deleteTransitionsToIrrelevantConfigurations() {

    for (auto i = Transitions.begin(); i != Transitions.end();) {

        auto j = i;
        i++;

        if (!(*j)->getDestination()->isRelevant()) {
            Transitions.erase(j);
        }
    }
}

// Functions to change status

std::shared_ptr<SADF_Configuration> SADF_Configuration::time(SADF_Graph *Graph, SADF_TPS& TPS, bool RelevantStep) {

    auto NewConfiguration =
            std::make_shared<SADF_Configuration>(Graph, this, SADF_TIME_STEP, MinimalRemainingExecutionTime);
    NewConfiguration->setMinimalRemainingExecutionTime(SADF_MAX_DOUBLE);

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
        if (KernelStatus[i]->getState()->getType() == SADF_START_STEP) {

            CDouble RemainingExecutionTime =
                    NewConfiguration->getKernelStatus(i)->getRemainingExecutionTime()
                    - MinimalRemainingExecutionTime;

            if (RemainingExecutionTime > 0) {
                NewConfiguration->getKernelStatus(i)->setRemainingExecutionTime(
                        RemainingExecutionTime);
                if (RemainingExecutionTime < NewConfiguration->getMinimalRemainingExecutionTime()) {
                    NewConfiguration->setMinimalRemainingExecutionTime(RemainingExecutionTime);
                }
            } else {
                NewConfiguration->getKernelStatus(i)->setRemainingExecutionTime(0);
            }
        }
    }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
        if (DetectorStatus[i]->getState()->getType() == SADF_START_STEP) {

            CDouble RemainingExecutionTime =
                    NewConfiguration->getDetectorStatus(i)->getRemainingExecutionTime()
                    - MinimalRemainingExecutionTime;

            if (RemainingExecutionTime > 0) {
                NewConfiguration->getDetectorStatus(i)->setRemainingExecutionTime(
                        RemainingExecutionTime);
                if (RemainingExecutionTime < NewConfiguration->getMinimalRemainingExecutionTime()) {
                    NewConfiguration->setMinimalRemainingExecutionTime(RemainingExecutionTime);
                }
            } else {
                NewConfiguration->getDetectorStatus(i)->setRemainingExecutionTime(0);
            }
        }
    }

    if (RelevantStep) {

        NewConfiguration->computeHashKey();
        SADF_Configuration *Test = TPS.inConfigurationSpace(NewConfiguration.get());

        if (Test != nullptr) {
            addTransition(Test, 1, 0);
            NewConfiguration.reset();
            return nullptr;
        }

        NewConfiguration->setRelevance(true);
        addTransition(NewConfiguration.get(), 1, 0);
        TPS.addConfiguration(NewConfiguration);
        return NewConfiguration;
    }

    addTransition(NewConfiguration.get(), 1, 0);
    return NewConfiguration;
}

// Equality operators

bool SADF_Configuration::equal(SADF_Configuration *C) {

    bool Equal = getType() == C->getType()
                 && MinimalRemainingExecutionTime == C->getMinimalRemainingExecutionTime();

    for (CId i = 0; Equal && i != DetectorStatus.size(); i++) {
        Equal = DetectorStatus[i]->equal(C->getDetectorStatus(i));
    }

    for (CId i = 0; Equal && i != ControlStatus.size(); i++) {
        Equal = ControlStatus[i]->equal(C->getControlStatus(i));
    }

    for (CId i = 0; Equal && i != KernelStatus.size(); i++) {
        Equal = KernelStatus[i]->equal(C->getKernelStatus(i));
    }

    for (CId i = 0; Equal && i != ChannelStatus.size(); i++) {
        Equal = ChannelStatus[i]->equal(C->getChannelStatus(i));
    }

    return Equal;
}
