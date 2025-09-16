/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2sdf.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   2 December 2009
 *
 *  Function        :   Abstract SADF Graph in FSM-based SADF Graph
 *
 *  History         :
 *      13-09-06    :   Initial version.
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

#include "sadf2fsmsadf.h"
#include "fsmsadf/base/type.h"
#include <memory>

using namespace SDF;

std::unique_ptr<FSMSADF::Graph> SADF2FSMSADF(const SADF_Graph& SADF) {

    // Check whether conversion is possible

    std::list<std::unique_ptr<SDFgraph>> Collection;

    if (SADF_Verify_SimpleStronglyConsistentSDFCollection(SADF, Collection)) {

        for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {
            if (SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime()
                != static_cast<CDouble>(static_cast<uint64_t>(
                        SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime()))) {
                throw CException(CString("Error: Real-valued execution time for Kernel '")
                                 + SADF.getKernel(i)->getName() + "' not supported.");
            }
        }

        for (CId i = 0; i != SADF.getNumberOfDetectors(); i++) {
            if (SADF.getDetector(i)->getProfile(0, 0)->getExecutionTime()
                != static_cast<CDouble>(static_cast<uint64_t>(
                        SADF.getDetector(i)->getProfile(0, 0)->getExecutionTime()))) {
                throw CException(CString("Error: Real-valued execution time for Detector '")
                                 + SADF.getDetector(i)->getName() + "' not supported.");
            }
        }

    } else {
        throw CException(CString("Error: SADF graph '") + SADF.getName()
                         + "' is not a strongly consistent collection of SDF graphs.");
    }

    // Check uniqueness of initial scenario name

    if (SADF.getNumberOfDetectors() == 1) {
        for (CId i = 0; i != SADF.getDetector(0)->getNumberOfSubScenarios(); i++) {
            if (SADF.getDetector(0)->getSubScenarioName(i)
                == CString("initial_") + SADF.getName()) {
                throw CException(CString("Error: Please rename subscenario name '")
                                 + SADF.getDetector(0)->getSubScenarioName(i) + "' of Detector '"
                                 + SADF.getDetector(0)->getName() + "' of SADF graph '"
                                 + SADF.getName() + "'.");
            }
        }
    }

    // Create FSM-based SADF graph

    auto FS = std::make_unique<FSMSADF::Graph>();
    FS->setName(SADF.getName());

    // Create Scenario Graphs

    for (const auto& g : Collection) {

        auto *SDF = dynamic_cast<TimedSDFgraph*>(g.get());

        auto S = std::make_unique<FSMSADF::Scenario>(FSMSADF::GraphComponent(
                FS.get(), static_cast<CId>(FS->getScenarios().size()), SDF->getName()));
        auto *SPtr = S.get();
        FS->addScenario(std::move(S));

        auto SG = std::make_unique<FSMSADF::ScenarioGraph>(FSMSADF::GraphComponent(
                FS.get(), static_cast<CId>(FS->getScenarioGraphs().size()), SDF->getName()));
        auto* SGPtr = SG.get();
        FS->addScenarioGraph(std::move(SG));
        SPtr->setScenarioGraph(SGPtr);
        SGPtr->setType(SDF->getName());

        // Create Actors

        for (const auto& a : SDF->getActors()) {

            auto Actor = std::make_unique<FSMSADF::Actor>(FSMSADF::GraphComponent(
                    SGPtr, static_cast<CId>(SGPtr->getActors().size()), (a)->getName()));
            auto* ActorPtr = Actor.get();
            SGPtr->addActor(std::move(Actor));
            ActorPtr->setType((a)->getName());
            ActorPtr->setExecutionTimeOfScenario(
                    *SPtr,
                    (dynamic_cast<TimedSDFactor*>(a.get()))->getDefaultProcessor(),
                    static_cast<FSMSADF::Time>(
                            (dynamic_cast<TimedSDFactor*>(a.get()))->getExecutionTime()));
            ActorPtr->setDefaultProcessorType(
                    (dynamic_cast<TimedSDFactor*>(a.get()))->getDefaultProcessor());

            for (const auto& p : (a)->getPorts()) {
                auto Port = std::make_unique<FSMSADF::Port>(FSMSADF::GraphComponent(
                        ActorPtr, static_cast<CId>(ActorPtr->getPorts().size()), p->getName()));
                Port->setType(p->getTypeAsString());
                ActorPtr->addPort(std::move(Port));
                ActorPtr->getPort(p->getName())->setRateOfScenario(*SPtr, p->getRate());
            }
        }

        // Create Channels

        for (const auto& c : SDF->getChannels()) {

            auto Channel = std::make_unique<FSMSADF::Channel>(FSMSADF::GraphComponent(
                    SGPtr, static_cast<CId>(SGPtr->getChannels().size()), c->getName()));
            auto* ChannelPtr = Channel.get();
            SGPtr->addChannel(std::move(Channel));
            ChannelPtr->setInitialTokens(c->getInitialTokens());
            ChannelPtr->setTokenSizeOfScenario(*SPtr,
                                            (dynamic_cast<TimedSDFchannel*>(c.get()))->getTokenSize());
            ChannelPtr->connectSrc(
                    SGPtr->getActor(c->getSrcActor()->getName())->getPort(c->getSrcPort()->getName()));
            ChannelPtr->connectDst(
                    SGPtr->getActor(c->getDstActor()->getName())->getPort(c->getDstPort()->getName()));

            if (c->getInitialTokens() > 0) {
                CString PersistentNames;
                for (CId i = 0; i != c->getInitialTokens(); i++) {
                    CString TokenName = c->getName();
                    if (i != 0) {
                        PersistentNames = PersistentNames + ",";
                    }
                    PersistentNames = PersistentNames + TokenName + CString(i);
                }
                ChannelPtr->setPersistentTokenNames(PersistentNames);
            }
        }
    }

    if (SADF.getNumberOfDetectors() == 0) {

        // Create FSM

        auto FSM = std::make_unique<FSMSADF::FSM>(FSMSADF::GraphComponent(FS.get(), 0, "default"));
        auto* FSMPtr = FSM.get();
        FS->setFSM(std::move(FSM));

        auto InitialState = std::make_unique<FSMSADF::FSMstate>(
                FSMSADF::GraphComponent(FSMPtr, static_cast<CId>(FSMPtr->getStates().size()), "default"));
        auto* InitialStatePtr = InitialState.get();
        FSMPtr->addState(std::move(InitialState));
        FSMPtr->setInitialState(InitialStatePtr);
        InitialStatePtr->setScenario(FS->getScenarioRef(Collection.front()->getName()));

        auto Transition = std::make_unique<FSMSADF::FSMtransition>(FSMSADF::GraphComponent(FSMPtr, 0));
        Transition->connect(FSMPtr->getInitialState(), FSMPtr->getInitialState());
        FSMPtr->addTransition(std::move(Transition));
    }

    if (SADF.getNumberOfDetectors() == 1) {

        // Create Initial Scenario
        auto InitialScenario = std::make_unique<FSMSADF::Scenario>(
                FSMSADF::GraphComponent(FS.get(),
                                        static_cast<CId>(FS->getScenarios().size()),
                                        CString("initial_") + SADF.getName()));

        auto *InitialScenarioPtr = InitialScenario.get();
        FS->addScenario(std::move(InitialScenario));

        auto SG = std::make_unique<FSMSADF::ScenarioGraph>(
                FSMSADF::GraphComponent(FS.get(),
                                        static_cast<CId>(FS->getScenarioGraphs().size()),
                                        CString("initial_") + SADF.getName()));
        auto* SGPtr = SG.get();
        FS->addScenarioGraph(std::move(SG));
        InitialScenarioPtr->setScenarioGraph(SGPtr);
        SGPtr->setType(CString("initial_") + SADF.getName());

        auto Actor =
                std::make_unique<FSMSADF::Actor>(FSMSADF::GraphComponent(SGPtr,
                                                           static_cast<CId>(SGPtr->getActors().size()),
                                                           CString("initial_") + SADF.getName()));
        auto* ActorPtr = Actor.get();
        SGPtr->addActor(std::move(Actor));
        ActorPtr->setType(CString("initial_") + SADF.getName());
        ActorPtr->setExecutionTimeOfScenario(
                *InitialScenarioPtr, CString("initial_") + SADF.getName(), 0);
        ActorPtr->setDefaultProcessorType(CString("initial_") + SADF.getName());

        for (CId i = 0; i != SADF.getNumberOfDataChannels(); i++) {
            if (SADF.getDataChannel(i)->getNumberOfInitialTokens() > 0) {

                CString ChannelName = SADF.getDataChannel(i)->getName();

                auto InPort = std::make_unique<FSMSADF::Port>(
                        FSMSADF::GraphComponent(ActorPtr,
                                                static_cast<CId>(ActorPtr->getPorts().size()),
                                                CString("In_") + ChannelName));
                auto* InPortPtr = InPort.get();
                ActorPtr->addPort(std::move(InPort));
                InPortPtr->setType(FSMSADF::Port::PortType::In);
                InPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

                auto OutPort = std::make_unique<FSMSADF::Port>(
                        FSMSADF::GraphComponent(ActorPtr,
                                                static_cast<CId>(ActorPtr->getPorts().size()),
                                                CString("Out_") + ChannelName));
                auto* OutPortPtr = OutPort.get();
                ActorPtr->addPort(std::move(OutPort));
                OutPortPtr->setType(FSMSADF::Port::PortType::Out);
                OutPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

                auto SelfChannel = std::make_unique<FSMSADF::Channel>(FSMSADF::GraphComponent(
                        SGPtr, static_cast<CId>(SGPtr->getChannels().size()), ChannelName));
                auto* SelfChannelPtr = SelfChannel.get();
                SGPtr->addChannel(std::move(SelfChannel));
                SelfChannelPtr->connectSrc(OutPortPtr);
                SelfChannelPtr->connectDst(InPortPtr);
                SelfChannelPtr->setInitialTokens(SADF.getDataChannel(i)->getNumberOfInitialTokens());
                SelfChannelPtr->setTokenSizeOfScenario(*InitialScenarioPtr, 1);

                CString PersistentNames;
                for (CId j = 0; j != SADF.getDataChannel(i)->getNumberOfInitialTokens(); j++) {
                    if (j != 0) {
                        PersistentNames = PersistentNames + ",";
                    }
                    PersistentNames = PersistentNames + ChannelName + CString(j);
                }
                SelfChannelPtr->setPersistentTokenNames(PersistentNames);
            }
        }

        for (CId i = 0; i != SADF.getNumberOfControlChannels(); i++) {
            if (SADF.getControlChannel(i)->getNumberOfInitialTokens() > 0) {

                CString ChannelName = SADF.getControlChannel(i)->getName();

                auto InPort = std::make_unique<FSMSADF::Port>(
                        FSMSADF::GraphComponent(ActorPtr,
                                                static_cast<CId>(ActorPtr->getPorts().size()),
                                                CString("In_") + ChannelName));
                auto* InPortPtr = InPort.get();
                ActorPtr->addPort(std::move(InPort));
                InPortPtr->setType(FSMSADF::Port::PortType::In);
                InPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

                auto OutPort = std::make_unique<FSMSADF::Port>(
                        FSMSADF::GraphComponent(ActorPtr,
                                                static_cast<CId>(ActorPtr->getPorts().size()),
                                                CString("Out_") + ChannelName));
                auto* OutPortPtr = OutPort.get();
                ActorPtr->addPort(std::move(OutPort));
                OutPortPtr->setType(FSMSADF::Port::PortType::Out);
                OutPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

                auto SelfChannel = std::make_unique<FSMSADF::Channel>(FSMSADF::GraphComponent(
                        SGPtr, static_cast<CId>(SGPtr->getChannels().size()), ChannelName));
                auto* SelfChannelPtr = SelfChannel.get();
                SGPtr->addChannel(std::move(SelfChannel));
                SelfChannelPtr->connectSrc(OutPortPtr);
                SelfChannelPtr->connectDst(InPortPtr);
                SelfChannelPtr->setInitialTokens(
                        SADF.getControlChannel(i)->getNumberOfInitialTokens());
                SelfChannelPtr->setTokenSizeOfScenario(*InitialScenarioPtr, 1);

                CString PersistentNames;
                for (CId j = 0; j != SADF.getControlChannel(i)->getNumberOfInitialTokens(); j++) {
                    if (j != 0) {
                        PersistentNames = PersistentNames + ",";
                    }
                    PersistentNames = PersistentNames + ChannelName + CString(j);
                }
                SelfChannelPtr->setPersistentTokenNames(PersistentNames);
            }
        }

        for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {

            CString ChannelName = SADF.getKernel(i)->getName();

            auto InPort = std::make_unique<FSMSADF::Port>(
                    FSMSADF::GraphComponent(ActorPtr,
                                            static_cast<CId>(ActorPtr->getPorts().size()),
                                            CString("In_") + ChannelName));
            auto* InPortPtr = InPort.get();
            ActorPtr->addPort(std::move(InPort));
            InPortPtr->setType(FSMSADF::Port::PortType::In);
            InPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

            auto OutPort = std::make_unique<FSMSADF::Port>(
                    FSMSADF::GraphComponent(ActorPtr,
                                            static_cast<CId>(ActorPtr->getPorts().size()),
                                            CString("Out_") + ChannelName));
            auto* OutPortPtr = OutPort.get();
            ActorPtr->addPort(std::move(OutPort));
            OutPortPtr->setType(FSMSADF::Port::PortType::Out);
            OutPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

            auto SelfChannel = std::make_unique<FSMSADF::Channel>(FSMSADF::GraphComponent(
                    SGPtr, static_cast<CId>(SGPtr->getChannels().size()), ChannelName));
            auto* SelfChannelPtr = SelfChannel.get();
            SGPtr->addChannel(std::move(SelfChannel));
            SelfChannelPtr->connectSrc(OutPortPtr);
            SelfChannelPtr->connectDst(InPortPtr);
            SelfChannelPtr->setInitialTokens(1);
            SelfChannelPtr->setTokenSizeOfScenario(*InitialScenarioPtr, 1);
            SelfChannelPtr->setPersistentTokenNames(CString("Self_") + ChannelName + "0");
        }

        for (CId i = 0; i != SADF.getNumberOfDetectors(); i++) {

            CString ChannelName = SADF.getDetector(i)->getName();

            auto InPort = std::make_unique<FSMSADF::Port>(
                    FSMSADF::GraphComponent(ActorPtr,
                                            static_cast<CId>(ActorPtr->getPorts().size()),
                                            CString("In_") + ChannelName));
            auto* InPortPtr = InPort.get();
            ActorPtr->addPort(std::move(InPort));
            InPortPtr->setType(FSMSADF::Port::PortType::In);
            InPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

            auto OutPort = std::make_unique<FSMSADF::Port>(
                    FSMSADF::GraphComponent(ActorPtr,
                                            static_cast<CId>(ActorPtr->getPorts().size()),
                                            CString("Out_") + ChannelName));
            auto* OutPortPtr = OutPort.get();
            ActorPtr->addPort(std::move(OutPort));
            OutPortPtr->setType(FSMSADF::Port::PortType::Out);
            OutPortPtr->setRateOfScenario(*InitialScenarioPtr, 1);

            auto SelfChannel = std::make_unique<FSMSADF::Channel>(FSMSADF::GraphComponent(
                    SGPtr, static_cast<CId>(SGPtr->getChannels().size()), ChannelName));
            auto* SelfChannelPtr = SelfChannel.get();
            SGPtr->addChannel(std::move(SelfChannel));
            SelfChannelPtr->connectSrc(OutPortPtr);
            SelfChannelPtr->connectDst(InPortPtr);
            SelfChannelPtr->setInitialTokens(1);
            SelfChannelPtr->setTokenSizeOfScenario(*InitialScenarioPtr, 1);
            SelfChannelPtr->setPersistentTokenNames(CString("Self_") + ChannelName + "0");
        }

        // Create FSM

        auto FSM = std::make_unique<FSMSADF::FSM>(
                FSMSADF::GraphComponent(FS.get(), 0, SADF.getDetector(0)->getScenarioName(0)));
        auto* FSMPtr = FSM.get();
        FS->setFSM(std::move(FSM));

        SADF_MarkovChain *MarkovChain = SADF.getDetector(0)->getMarkovChain(0);

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {

            auto State = std::make_unique<FSMSADF::FSMstate>(FSMSADF::GraphComponent(
                    FSMPtr, static_cast<CId>(FSMPtr->getStates().size()), MarkovChain->getStateName(i)));
            auto* StatePtr = State.get();
            FSMPtr->addState(std::move(State));
            StatePtr->setScenario(FS->getScenarioRef(
                    SADF.getDetector(0)->getSubScenarioName(MarkovChain->getSubScenario(i))));
        }

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
            for (CId j = 0; j != MarkovChain->getNumberOfStates(); j++) {
                if (MarkovChain->getTransitionProbability(i, j) > 0) {

                    auto Transition = std::make_unique<FSMSADF::FSMtransition>(FSMSADF::GraphComponent(FSMPtr, 0));
                    auto *TransitionPtr = Transition.get();
                    FSMPtr->addTransition(std::move(Transition));
                    TransitionPtr->connect(FSMPtr->getState(MarkovChain->getStateName(i)),
                                        FSMPtr->getState(MarkovChain->getStateName(j)));
                }
            }
        }

        auto InitialState = std::make_unique<FSMSADF::FSMstate>(FSMSADF::GraphComponent(
                FSMPtr, static_cast<CId>(FSMPtr->getStates().size()), CString("initial_") + SADF.getName()));
        auto* InitialStatePtr = InitialState.get();
        FSMPtr->addState(std::move(InitialState));
        FSMPtr->setInitialState(InitialStatePtr);
        InitialStatePtr->setScenario(InitialScenarioPtr);

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
            if (MarkovChain->getTransitionProbability(MarkovChain->getInitialState(), i) > 0) {

                auto *Transition =
                        new FSMSADF::FSMtransition(FSMSADF::GraphComponent(FSMPtr, 0));
                Transition->connect(InitialStatePtr, FSMPtr->getState(MarkovChain->getStateName(i)));
            }
}
    }

    return FS;
}
