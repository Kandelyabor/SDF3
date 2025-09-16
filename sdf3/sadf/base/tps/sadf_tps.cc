/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_TPS.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Timed Probabilistic System (TPS)
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

#include "sadf_tps.h"

namespace {

// Functions for constructing detector states
void SADF_ConstructKernelStates(SADF_Graph *Graph,
                                SADF_TPS& TPS,
                                CId KernelID,
                                SADF_KernelState *S) {

    // Creation of control and start states;

    if (Graph->getKernel(KernelID)->hasControlInputChannels()) {

        std::list<std::shared_ptr<SADF_KernelState>> ControlStates;

        for (CId i = 0; i != Graph->getKernel(KernelID)->getNumberOfScenarios(); i++) {

            auto ControlState =
                    std::make_shared<SADF_KernelState>(Graph->getKernel(KernelID),
                                         static_cast<CId>(TPS.getKernelStates(KernelID).size()),
                                         i,
                                         SADF_CONTROL_STEP,
                                         SADF_MAX_DOUBLE);
            ControlStates.push_front(ControlState);
            TPS.getKernelStates(KernelID).push_front(ControlState);
            S->addTransition(ControlState.get(), 1); // Choice is determined by environment
        }

        for (const auto& ks : ControlStates) {
            for (CId j = 0; j != Graph->getKernel(KernelID)->getNumberOfProfiles(ks->getScenario());
                 j++) {

                auto StartState = std::make_unique<SADF_KernelState>(
                        Graph->getKernel(KernelID),
                        static_cast<CId>(TPS.getKernelStates(KernelID).size()),
                        ks->getScenario(),
                        SADF_START_STEP,
                        Graph->getKernel(KernelID)
                                ->getProfile(ks->getScenario(), j)
                                ->getExecutionTime());
                StartState->addTransition(S, 1);
                auto* StartStatePtr = StartState.get();
                TPS.getKernelStates(KernelID).push_front(std::move(StartState));
                ks->addTransition(
                        StartStatePtr,
                        Graph->getKernel(KernelID)->getProfile(ks->getScenario(), j)->getWeight());
            }
        }

    } else {

        for (CId i = 0; i != Graph->getKernel(KernelID)->getNumberOfProfiles(0); i++) {

            auto StartState = std::make_unique<SADF_KernelState>(
                    Graph->getKernel(KernelID),
                    static_cast<CId>(TPS.getKernelStates(KernelID).size()),
                    0,
                    SADF_START_STEP,
                    Graph->getKernel(KernelID)->getProfile(0, i)->getExecutionTime());
            StartState->addTransition(S, 1);
            auto* StartStatePtr = StartState.get();
            TPS.getKernelStates(KernelID).push_front(std::move(StartState));
            S->addTransition(StartStatePtr, Graph->getKernel(KernelID)->getProfile(0, i)->getWeight());
        }
    }
}


SADF_DetectorState *SADF_DetectorState_InList(const std::list<std::shared_ptr<SADF_DetectorState>> &List,
                                              SADF_DetectorState *S) {

    for (const auto& ds : List) {
        if (S->getType() == ds->getType() && S->getScenario() == ds->getScenario()
            && S->getSubScenario() == ds->getSubScenario()
            && S->getExecutionTime() == ds->getExecutionTime()
            && S->getMarkovChainStatus() == ds->getMarkovChainStatus()) {
            return ds.get();
        }
    }

    return nullptr;
}


void SADF_ConstructDetectorStates(SADF_Graph *Graph,
                                  SADF_TPS& TPS,
                                  CId DetectorID,
                                  SADF_DetectorState *S) {

    // Create new detect states

    std::list<SADF_DetectorState*> NewDetectStates;

    for (CId i = 0; i != Graph->getDetector(DetectorID)->getNumberOfScenarios(); i++) {
        for (CId j = 0; j != Graph->getDetector(DetectorID)->getMarkovChain(i)->getNumberOfStates();
             j++) {
            if (Graph->getDetector(DetectorID)
                        ->getMarkovChain(i)
                        ->getTransitionProbability(S->getStateOfMarkovChain(i), j)
                > 0) {

                std::vector<CId> NewMarkovChainStatus(S->getMarkovChainStatus());
                NewMarkovChainStatus[i] = j;

                auto DetectState = std::make_unique<SADF_DetectorState>(
                        Graph->getDetector(DetectorID),
                        static_cast<CId>(TPS.getDetectorStates(DetectorID).size()),
                        i,
                        Graph->getDetector(DetectorID)->getMarkovChain(i)->getSubScenario(j),
                        NewMarkovChainStatus,
                        SADF_DETECT_STEP,
                        SADF_MAX_DOUBLE);
                    auto* DetectStatePtr = DetectState.get();
                    SADF_DetectorState *Test =
                        SADF_DetectorState_InList(TPS.getDetectorStates(DetectorID), DetectState.get());

                if (Test == nullptr) {
                    NewDetectStates.push_front(DetectState.get());
                    S->addTransition(
                            DetectStatePtr,
                            Graph->getDetector(DetectorID)
                                    ->getMarkovChain(i)
                                    ->getTransitionProbability(S->getStateOfMarkovChain(i), j));
                    TPS.getDetectorStates(DetectorID).push_front(std::move(DetectState));
                } else {
                    S->addTransition(
                            Test,
                            Graph->getDetector(DetectorID)
                                    ->getMarkovChain(i)
                                    ->getTransitionProbability(S->getStateOfMarkovChain(i), j));
                }
            }
        }
    }

    // Create states corresponding to each new detect state

    std::list<SADF_DetectorState *> NewEndStates;

    for (auto *DetectState : NewDetectStates) {
        // Create end state

        auto EndState =
                std::make_shared<SADF_DetectorState>(Graph->getDetector(DetectorID),
                                       static_cast<CId>(TPS.getDetectorStates(DetectorID).size()),
                                       SADF_UNDEFINED,
                                       SADF_UNDEFINED,
                                       DetectState->getMarkovChainStatus(),
                                       SADF_END_STEP,
                                       SADF_MAX_DOUBLE);
        auto* EndStatePtr = EndState.get();
        SADF_DetectorState *Test =
                SADF_DetectorState_InList(TPS.getDetectorStates(DetectorID), EndState.get());

        if (Test == nullptr) {
            NewEndStates.push_front(EndState.get());
            TPS.getDetectorStates(DetectorID).push_front(EndState);
        } else {
            EndStatePtr = Test;
        }

        // Create start states

        for (CId j = 0;
             j
             != Graph->getDetector(DetectorID)->getNumberOfProfiles(DetectState->getSubScenario());
             j++) {

            auto StartState = std::make_shared<SADF_DetectorState>(
                    Graph->getDetector(DetectorID),
                    static_cast<CId>(TPS.getDetectorStates(DetectorID).size()),
                    DetectState->getScenario(),
                    DetectState->getSubScenario(),
                    DetectState->getMarkovChainStatus(),
                    SADF_START_STEP,
                    Graph->getDetector(DetectorID)
                            ->getProfile(DetectState->getSubScenario(), j)
                            ->getExecutionTime());
            StartState->addTransition(EndStatePtr, 1);
            TPS.getDetectorStates(DetectorID).push_front(StartState);
            DetectState->addTransition(StartState.get(),
                                       Graph->getDetector(DetectorID)
                                               ->getProfile(DetectState->getSubScenario(), j)
                                               ->getWeight());
        }
    }

    // Recursively construct states from new end states

    for (auto *i : NewEndStates) {
        SADF_ConstructDetectorStates(Graph, TPS, DetectorID, i);
    }
}

} // namespace

// Constructor

SADF_TPS::SADF_TPS(SADF_Graph *Graph) : NumberOfConfigurations(0) {

    KernelStates.resize(Graph->getNumberOfKernels());
    DetectorStates.resize(Graph->getNumberOfDetectors());
    InitialKernelStates.resize(Graph->getNumberOfKernels());
    InitialDetectorStates.resize(Graph->getNumberOfDetectors());

    // Create Kernel States

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {

        // Creation of initial state

        std::unique_ptr<SADF_KernelState> InitialState = nullptr;

        if (Graph->getKernel(i)->hasControlInputChannels()) {
            InitialState = std::make_unique<SADF_KernelState>(Graph->getKernel(i),
                                                static_cast<CId>(KernelStates[i].size()),
                                                SADF_UNDEFINED,
                                                SADF_END_STEP,
                                                SADF_MAX_DOUBLE);
        } else {
            InitialState = std::make_unique<SADF_KernelState>(Graph->getKernel(i),
                                                static_cast<CId>(KernelStates[i].size()),
                                                0,
                                                SADF_END_STEP,
                                                SADF_MAX_DOUBLE);
        }

        InitialKernelStates[i] = InitialState.get();
        auto* InitialStatePtr = InitialState.get();
        KernelStates[i].push_front(std::move(InitialState));

        SADF_ConstructKernelStates(Graph, *this, i, InitialStatePtr);
    }

    // Create Detector States

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {

        std::vector<CId> MarkovChainStatus(Graph->getDetector(i)->getNumberOfScenarios());

        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {
            MarkovChainStatus[j] = Graph->getDetector(i)->getMarkovChain(j)->getInitialState();
        }

        auto InitialState = std::make_unique<SADF_DetectorState>(Graph->getDetector(i),
                                                    static_cast<CId>(DetectorStates[i].size()),
                                                    SADF_UNDEFINED,
                                                    SADF_UNDEFINED,
                                                    MarkovChainStatus,
                                                    SADF_END_STEP,
                                                    SADF_MAX_DOUBLE);
        InitialDetectorStates[i] = InitialState.get();
        auto* InitialStatePtr = InitialState.get();
        DetectorStates[i].push_front(std::move(InitialState));

        SADF_ConstructDetectorStates(Graph, *this, i, InitialStatePtr);
    }

    InitialConfiguration = std::make_unique<SADF_Configuration>(Graph, *this, SADF_END_STEP);
}

// Destructor

SADF_TPS::~SADF_TPS() = default;

// Access to instance variables

void SADF_TPS::addConfiguration(const std::shared_ptr<SADF_Configuration>& C) {

    C->setIdentity(static_cast<CId>(NumberOfConfigurations));

    bool MatchFound = false;

    for (auto &configurations : ConfigurationSpace) {
        if (configurations.front()->getHashKey() == C->getHashKey()) {
            configurations.push_front(C);
            MatchFound = true;
        }
    }

    if (!MatchFound) {
        SADF_ListOfConfigurations NewListOfConfigurations;
        NewListOfConfigurations.push_front(C);
        ConfigurationSpace.push_front(NewListOfConfigurations);
    }

    NumberOfConfigurations++;
}

SADF_Configuration *SADF_TPS::inConfigurationSpace(SADF_Configuration *C) {

    for (auto &configurations : ConfigurationSpace) {
        if (configurations.front()->getHashKey() == C->getHashKey()) {
            for (const auto& c : configurations) {
                if (c->equal(C)) {
                    return c.get();
                }
            }
        }
    }

    return nullptr;
}

void SADF_TPS::deleteContentOfConfigurations() {

    for (auto &configurations : ConfigurationSpace) {
        for (const auto& c : configurations) {
            c->deleteContent();
        }
    }
}

// Functions to determine graph properties

namespace {

void SADF_MarkReachableConfigurations(SADF_Configuration *Source) {

    Source->setMarking(true);

    for (const auto& tr : Source->getTransitions()) {
        if (!tr->getDestination()->isMarked()) {
            SADF_MarkReachableConfigurations(tr->getDestination());
        }
    }
}

} // namespace

bool SADF_TPS::isSingleStronglyConnectedComponent() {

    // Precondition: all configurations in configuration space must not be marked

    bool SingleStronglyConnectedComponent = true;

    SADF_MarkReachableConfigurations(ConfigurationSpace.front().front().get());

    for (auto i = ConfigurationSpace.begin();
         SingleStronglyConnectedComponent && i != ConfigurationSpace.end();
         i++) {
        for (auto j = (*i).begin(); SingleStronglyConnectedComponent && j != (*i).end(); j++) {
            if (!(*j)->isMarked()) {
                SingleStronglyConnectedComponent = false;
            }
        }
    }

    return SingleStronglyConnectedComponent;
}

// Functions to remove transient configurations

namespace {


void SADF_FindRecurrencePoints(SADF_Configuration *Source) {

    Source->setMarking(true);

    for (const auto& tr : Source->getTransitions()) {
        if (!tr->getDestination()->isMarked()) {
            SADF_FindRecurrencePoints(tr->getDestination());
        } else if (tr->getDestination()->isRelevant()) {
            tr->getDestination()->setRelevance(false);
        }
    }
}


void SADF_RemoveTransitionsUntilRecurrencePoints(SADF_Configuration *Source) {

    bool Remove = false;

    for (auto i = Source->getTransitions().begin(); !Remove && i != Source->getTransitions().end();
         i++) {
        if (!(*i)->getDestination()->isRelevant()) {
            Remove = true;
        }
    }

    if (!Remove) {
        for (const auto& tr : Source->getTransitions()) {
            SADF_RemoveTransitionsUntilRecurrencePoints(tr->getDestination());
        }
    }

    Source->removeAllTransitions();
}

} // namespace

void SADF_TPS::removeTransientConfigurations() {

    // Precondition: all configurations in configuration space must not be marked and relevant

    SADF_FindRecurrencePoints(InitialConfiguration.get());
    SADF_RemoveTransitionsUntilRecurrencePoints(InitialConfiguration.get());

    for (auto i = ConfigurationSpace.begin(); i != ConfigurationSpace.end();) {

        auto n = i;
        i++;

        for (auto j = (*n).begin(); j != (*n).end();) {

            auto m = j;
            j++;

            if ((*m)->getTransitions().empty()) {
                NumberOfConfigurations--;
                (*n).erase(m);
            } else {
                (*m)->setMarking(false);
                (*m)->setRelevance(true);
            }
        }

        if ((*n).empty()) {
            ConfigurationSpace.erase(n);
        }
    }

    // Reassign identities

    CId ID = 0;

    for (auto &configurations : ConfigurationSpace) {
        for (const auto& c : configurations) {
            c->setIdentity(ID);
            ID++;
        }
    }
}

// Functions to compute equilibrium distribution

std::vector<CDouble> SADF_TPS::computeEquilibriumDistribution() {

    // Initialise augmented matrix

    SparseMatrix Matrix(static_cast<CId>(NumberOfConfigurations));

    for (auto &configurations : ConfigurationSpace) {
        for (const auto& c : configurations) {
            for (const auto& tr : c->getTransitions()) {
                if (tr->getDestination()->getIdentity() < NumberOfConfigurations - 1) {
                    Matrix.set(tr->getDestination()->getIdentity(),
                                c->getIdentity(),
                                tr->getProbability());
                }
            }
        }
    }

    for (CId i = 0; i != NumberOfConfigurations + 1; i++) {

        Matrix.set(static_cast<CId>(NumberOfConfigurations) - 1, i, 1);

        if (i < NumberOfConfigurations - 1) {
            Matrix.set(i, i, Matrix.get(i, i) - 1);
        }
    }

    std::vector<CDouble> EquilibriumDistribution = Matrix.computeEigenVector();

    return EquilibriumDistribution;
}
