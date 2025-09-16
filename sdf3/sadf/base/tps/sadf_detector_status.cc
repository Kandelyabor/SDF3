/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_detector_status.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Detector Status
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

#include "sadf_detector_status.h"
#include "sadf_tps.h"
#include <utility>

// Constructors

SADF_DetectorTransition::SADF_DetectorTransition(SADF_DetectorState *D, const CDouble P) :
    Destination(D), Probability(P) {}

SADF_DetectorState::SADF_DetectorState(SADF_Process *D,
                                       CId ID,
                                       CId ScenarioID,
                                       CId SubScenarioID,
                                       std::vector<CId> MCStatus,
                                       CId ActionType,
                                       CDouble Time) :
    Detector(D),
    Scenario(ScenarioID),
    SubScenario(SubScenarioID),
    ExecutionTime(Time),
    MarkovChainStatus(std::move(MCStatus)) {

    setIdentity(ID);

    setType(ActionType);
}

SADF_DetectorStatus::SADF_DetectorStatus(SADF_Configuration *C, SADF_DetectorState *S, CDouble T) :
    Configuration(C), State(S), RemainingExecutionTime(T) {}

SADF_DetectorStatus::SADF_DetectorStatus(SADF_Configuration *C, SADF_DetectorStatus *S) :
    Configuration(C),
    State(S->getState()),
    RemainingExecutionTime(S->getRemainingExecutionTime()) {}

// Destructor

SADF_DetectorState::~SADF_DetectorState() = default;

// Access to instance variables

void SADF_DetectorState::addTransition(SADF_DetectorState *D, CDouble P) {

    Transitions.push_front(std::make_unique<SADF_DetectorTransition>(D, P));
}

// Functions to determine current status

bool SADF_DetectorStatus::isReadyToFire() {

    bool ReadyToFire = State->getType() == SADF_END_STEP;

    for (CId i = 0; ReadyToFire && i != State->getDetector()->getControlInputChannels().size();
         i++) {
        if (Configuration
                    ->getControlStatus(
                            State->getDetector()->getControlInputChannels()[i]->getIdentity())
                    ->getAvailableTokens()
            == 0) {
            ReadyToFire = false;
        }
    }

    return ReadyToFire;
}
bool SADF_DetectorStatus::isReadyToStart() {

    bool ReadyToStart = State->getType() == SADF_DETECT_STEP;

    if (ReadyToStart) {

        CId SubScenario = State->getSubScenario();

        for (CId i = 0; ReadyToStart && i != State->getDetector()->getDataInputChannels().size();
             i++) {
            if (Configuration
                        ->getChannelStatus(
                                State->getDetector()->getDataInputChannels()[i]->getIdentity())
                        ->getAvailableTokens()
                < State->getDetector()->getDataInputChannels()[i]->getConsumptionRate(
                        SubScenario)) {
                ReadyToStart = false;
            }
        }

        for (CId i = 0; ReadyToStart && i != State->getDetector()->getDataOutputChannels().size();
             i++) {
            if (State->getDetector()->getDataOutputChannels()[i]->getBufferSize()
                != SADF_UNBOUNDED) {
                if (Configuration->getChannelStatus(State->getDetector()
                                                            ->getDataOutputChannels()[i]
                                                            ->getIdentity())
                                    ->getAvailableTokens()
                            + State->getDetector()->getDataOutputChannels()[i]->getProductionRate(
                                    SubScenario)
                    > State->getDetector()->getDataOutputChannels()[i]->getBufferSize()) {
                    ReadyToStart = false;
                }
            }
        }

        for (CId i = 0;
             ReadyToStart && i != State->getDetector()->getControlOutputChannels().size();
             i++) {
            if (State->getDetector()->getControlOutputChannels()[i]->getBufferSize()
                != SADF_UNBOUNDED) {
                if (Configuration->getControlStatus(State->getDetector()
                                                            ->getControlOutputChannels()[i]
                                                            ->getIdentity())
                                    ->getAvailableTokens()
                            + State->getDetector()
                                      ->getControlOutputChannels()[i]
                                      ->getProductionRate(SubScenario)
                    > State->getDetector()->getControlOutputChannels()[i]->getBufferSize()) {
                    ReadyToStart = false;
                }
            }
        }
    }

    return ReadyToStart;
}

bool SADF_DetectorStatus::isReadyToEnd() {

    return State->getType() == SADF_START_STEP
           && RemainingExecutionTime <= 0; // <= is used to circumvent rounding errors
}

// Functions to change status

SADF_ListOfConfigurations
SADF_DetectorStatus::detect(SADF_Graph *Graph, SADF_TPS& TPS, bool RelevantStep) {

    SADF_ListOfConfigurations NewConfigurations;

    SADF_Process *Detector = State->getDetector();

    // Determine scenario

    CId Scenario = 0;

    if (Detector->hasControlInputChannels()) {
        if (Detector->hasMultipleControlInputChannels()) {

            bool ScenarioNotFound = true;

            for (CId i = 0; ScenarioNotFound && i != Detector->getNumberOfScenarios(); i++) {

                bool MatchingControls = true;

                for (CId j = 0; MatchingControls && j != Detector->getControlInputChannels().size();
                     j++) {
                    if (Configuration
                                ->getControlStatus(
                                        Detector->getControlInputChannels()[j]->getIdentity())
                                ->inspect()
                        != Detector->getControlID(i, j)) {
                        MatchingControls = false;
                    }
                }

                if (MatchingControls) {
                    ScenarioNotFound = false;
                    Scenario = i;
                }
            }
        } else {
            Scenario = Configuration
                               ->getControlStatus(
                                       Detector->getControlInputChannels().front()->getIdentity())
                               ->inspect();
        }
    }

    // Determine subscenario and create corresponding configurations

    for (const auto& tr : State->getTransitions()) {
        if (tr->getDestination()->getScenario() == Scenario) {

            auto NewConfiguration =
                    std::make_shared<SADF_Configuration>(Graph, Configuration, SADF_DETECT_STEP, 0);

            // Update status of detector in new configuration

            NewConfiguration->getDetectorStatus(Detector->getIdentity())
                    ->setState(tr->getDestination());

            // Check existence of new configuration if relevant step

            if (RelevantStep) {

                NewConfiguration->computeHashKey();
                SADF_Configuration *Test = TPS.inConfigurationSpace(NewConfiguration.get());

                if (Test != nullptr) {
                    Configuration->addTransition(Test, tr->getProbability(), 0);
                    NewConfiguration.reset();
                } else {
                    NewConfiguration->setRelevance(true);
                    Configuration->addTransition(NewConfiguration.get(), tr->getProbability(), 0);
                    NewConfigurations.push_front(NewConfiguration);
                    TPS.addConfiguration(NewConfiguration);
                }

            } else {

                Configuration->addTransition(NewConfiguration.get(), tr->getProbability(), 0);
                NewConfigurations.push_front(NewConfiguration);
            }
        }
    }

    return NewConfigurations;
}

SADF_ListOfConfigurations
SADF_DetectorStatus::start(SADF_Graph *Graph, SADF_TPS& TPS, bool RelevantStep) {

    SADF_ListOfConfigurations NewConfigurations;

    SADF_Process *Detector = State->getDetector();

    for (const auto& tr : State->getTransitions()) {

        auto NewConfiguration = std::make_shared<SADF_Configuration>(Graph, Configuration, SADF_START_STEP, 0);

        // Update status of detector in new configuration

        for (const auto &j : Detector->getDataOutputChannels()) {
            NewConfiguration->getChannelStatus(j->getIdentity())
                    ->reserve(j->getProductionRate(State->getSubScenario()));
        }

        for (const auto &j : Detector->getControlOutputChannels()) {
            NewConfiguration->getControlStatus(j->getIdentity())
                    ->reserve(j->getProductionRate(State->getSubScenario()));
        }

        NewConfiguration->getDetectorStatus(Detector->getIdentity())
                ->setState(tr->getDestination());

        CDouble ExecutionTime = NewConfiguration->getDetectorStatus(Detector->getIdentity())
                                        ->getRemainingExecutionTime();

        if (ExecutionTime > 0
            && ExecutionTime < Configuration->getMinimalRemainingExecutionTime()) {
            NewConfiguration->setMinimalRemainingExecutionTime(ExecutionTime);
        }

        // Check existence of new configuration if relevant step

        if (RelevantStep) {

            NewConfiguration->computeHashKey();
            SADF_Configuration *Test = TPS.inConfigurationSpace(NewConfiguration.get());

            if (Test != nullptr) {
                Configuration->addTransition(Test, tr->getProbability(), 0);
                NewConfiguration.reset();
            } else {
                NewConfiguration->setRelevance(true);
                Configuration->addTransition(NewConfiguration.get(), tr->getProbability(), 0);
                TPS.addConfiguration(NewConfiguration);
                NewConfigurations.push_front(NewConfiguration);
            }

        } else {

            Configuration->addTransition(NewConfiguration.get(), tr->getProbability(), 0);
            NewConfigurations.push_front(NewConfiguration);
        }
    }

    return NewConfigurations;
}

SADF_ListOfConfigurations
SADF_DetectorStatus::end(SADF_Graph *Graph, SADF_TPS& TPS, bool RelevantStep) {

    SADF_ListOfConfigurations NewConfigurations;

    auto NewConfiguration = std::make_shared<SADF_Configuration>(Graph, Configuration, SADF_END_STEP, 0);

    // Update status of kernel in new configuration

    SADF_Process *Detector = State->getDetector();

    for (const auto &i : Detector->getControlInputChannels()) {
        NewConfiguration->getControlStatus(i->getIdentity())->remove();
    }

    for (const auto &i : Detector->getDataInputChannels()) {
        NewConfiguration->getChannelStatus(i->getIdentity())
                ->remove(i->getConsumptionRate(State->getSubScenario()));
    }

    for (const auto &i : Detector->getDataOutputChannels()) {
        NewConfiguration->getChannelStatus(i->getIdentity())
                ->write(i->getProductionRate(State->getSubScenario()));
    }

    for (const auto &i : Detector->getControlOutputChannels()) {
        NewConfiguration->getControlStatus(i->getIdentity())
                ->write(i->getProductionRate(State->getSubScenario()),
                        i->getProductionScenarioID(State->getSubScenario()));
    }

    NewConfiguration->getDetectorStatus(Detector->getIdentity())
            ->setState(State->getTransitions().front()->getDestination());

    // Check existence of new configuration if relevant step

    if (RelevantStep) {

        NewConfiguration->computeHashKey();
        SADF_Configuration *Test = TPS.inConfigurationSpace(NewConfiguration.get());

        if (Test != nullptr) {
            Configuration->addTransition(Test, 1, 0);
            NewConfiguration.reset();
        } else {
            NewConfiguration->setRelevance(true);
            Configuration->addTransition(NewConfiguration.get(), 1, 0);
            TPS.addConfiguration(NewConfiguration);
            NewConfigurations.push_front(NewConfiguration);
        }

    } else {

        Configuration->addTransition(NewConfiguration.get(), 1, 0);
        NewConfigurations.push_front(NewConfiguration);
    }

    return NewConfigurations;
}

// Equality operators

bool SADF_DetectorStatus::equal(SADF_DetectorStatus *S) {

    return State->getIdentity() == S->getState()->getIdentity()
           && RemainingExecutionTime == S->getRemainingExecutionTime();
}
