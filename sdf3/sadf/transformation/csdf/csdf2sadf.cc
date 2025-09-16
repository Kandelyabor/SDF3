/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   csdf2sadf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert CSDF Graph in SADF Graph
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

#include "csdf2sadf.h" // IWYU pragma: keep

using namespace CSDF;


std::unique_ptr<SADF_Graph> CSDF2SADF(const TimedCSDFgraph& CSDF) {

    auto SADF = std::make_unique<SADF_Graph>(CSDF.getName(), CSDF.getId());

    // Create Kernels and Detectors

    for (const auto& a : CSDF.getConstActors()) {
        if (a->sequenceLength() == 1) {
            SADF->addKernel(
                    std::make_unique<SADF_Process>(a->getName(), SADF->getNumberOfKernels(), SADF_KERNEL));
        } else {
            SADF->addDetector(
                    std::make_unique<SADF_Process>(a->getName(), SADF->getNumberOfDetectors(), SADF_DETECTOR));
        }
    }

    // Create Channels

    for (const auto& c : CSDF.getConstChannels()) {
        auto *Channel = dynamic_cast<TimedCSDFchannel *>(c.get());
        auto DataChannel = std::make_unique<SADF_Channel>(
                Channel->getName(), SADF->getNumberOfDataChannels(), SADF_DATA_CHANNEL);

        if (Channel->isConnected()) {
            if (SADF->getKernel(Channel->getSrcActor()->getName()) != nullptr) {
                DataChannel->setSource(SADF->getKernel(Channel->getSrcActor()->getName()));
            } else {
                DataChannel->setSource(SADF->getDetector(Channel->getSrcActor()->getName()));
            }

            if (SADF->getKernel(Channel->getDstActor()->getName()) != nullptr) {
                DataChannel->setDestination(SADF->getKernel(Channel->getDstActor()->getName()));
            } else {
                DataChannel->setDestination(SADF->getDetector(Channel->getDstActor()->getName()));
            }

            DataChannel->getSource()->addDataOutputChannel(DataChannel.get());
            DataChannel->getDestination()->addDataInputChannel(DataChannel.get());

        } else {
            throw CException(CString("Channel '") + Channel->getName() + "' of CSDF graph '"
                             + CSDF.getName() + "' is not connected.");
        }

        if (Channel->getInitialTokens() != 0) {
            DataChannel->setNumberOfInitialTokens(static_cast<CId>(Channel->getInitialTokens()));
        }
        if (Channel->getBufferSize().sz != -1) {
            DataChannel->setBufferSize(static_cast<CId>(Channel->getBufferSize().sz));
        }
        if (Channel->getTokenSize() != -1) {
            DataChannel->setTokenSize(static_cast<CId>(Channel->getTokenSize()));
        }

        SADF->addDataChannel(std::move(DataChannel));
    }

    // Finalize creation of Kernels

    for (const auto& a : CSDF.getConstActors()) {
        if (a->sequenceLength() == 1) {
            auto *Actor = dynamic_cast<TimedCSDFactor *>(a.get());
            SADF_Process *Kernel = SADF->getKernel(Actor->getName());

            CId ScenarioID = Kernel->addScenario("default");

            for (const auto& p : Actor->getPorts()) {

                if (p->getType() == CSDFport::In) {
                    SADF->getChannel(p->getChannel()->getName())
                            ->setConsumptionRate(0, p->getRate().at(0));
                }

                if (p->getType() == CSDFport::Out) {
                    SADF->getChannel(p->getChannel()->getName())
                            ->setProductionRate(0, p->getRate().at(0));
                }
            }

            auto Profile = std::make_unique<SADF_Profile>();

            for (const auto& p : Actor->getProcessors()) {
                if (Actor->getDefaultProcessor() == p->type) {
                    Profile->setExecutionTime(static_cast<CDouble>(p->execTime[0]));
                }
            }

            Kernel->addProfile(ScenarioID, std::move(Profile));
        }
    }

    // Finalize creation of Detectors

    for (const auto& a : CSDF.getConstActors()) {
        if (a->sequenceLength() != 1) {

            auto *Actor = dynamic_cast<TimedCSDFactor *>(a.get());
            SADF_Process *Detector = SADF->getDetector(Actor->getName());

            CId ScenarioID = Detector->addScenario("default");

            auto MarkovChain = std::make_unique<SADF_MarkovChain>();

            for (CId j = 0; j != Actor->sequenceLength(); j++) {

                CId SubScenarioID = Detector->addSubScenario("Phase" + CString(j));
                MarkovChain->addState("Phase" + CString(j), SubScenarioID);

                for (const auto& k : Actor->getPorts()) {

                    if (k->getType() == CSDFport::In) {
                        SADF->getChannel(k->getChannel()->getName())
                                ->setConsumptionRate(SubScenarioID,
                                                     k->getRate().at(static_cast<int>(j)));
                    }

                    if (k->getType() == CSDFport::Out) {
                        SADF->getChannel(k->getChannel()->getName())
                                ->setProductionRate(SubScenarioID,
                                                    k->getRate().at(static_cast<int>(j)));
                    }
                }

                auto Profile = std::make_unique<SADF_Profile>();

                for (const auto& k : Actor->getProcessors()) {
                    if (Actor->getDefaultProcessor() == k->type) {
                        Profile->setExecutionTime(
                                static_cast<CDouble>(k->execTime[static_cast<int>(j)]));
                    }
                }

                Detector->addProfile(SubScenarioID, std::move(Profile));
            }

            MarkovChain->setInitialState(Actor->sequenceLength() - 1);

            for (CId j = 0; j != Actor->sequenceLength() - 1; j++) {
                MarkovChain->setTransitionProbability(j, j + 1, 1);
            }

            MarkovChain->setTransitionProbability(Actor->sequenceLength() - 1, 0, 1);

            Detector->setMarkovChain(ScenarioID, std::move(MarkovChain));
        }
    }

    return SADF;
}


