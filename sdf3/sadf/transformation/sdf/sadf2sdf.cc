/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2sdf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SADF Graph in SDF Graph
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

#include "sadf2sdf.h"
#include <memory>

using namespace SDF;

std::unique_ptr<TimedSDFgraph> SADF2SDF(const SADF_Graph& SADF) {

    // Check whether conversion is possible

    if (!SADF_Verify_SDF(SADF)) {
        throw CException(CString("Error: SADF Graph '") + SADF.getName()
                         + "' does not represent an SDF Graph.");
    }

    for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {
        if (SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime()
            != static_cast<CDouble>(static_cast<uint64_t>(
                    SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime()))) {
            throw CException(CString("Error: Real-valued execution time for Kernel '")
                             + SADF.getKernel(i)->getName() + "' not supported.");
        }

        if (SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime()
            != static_cast<CDouble>(static_cast<SDFtime>(
                    SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime()))) {
            throw CException(CString("Error: Execution time for Kernel '")
                             + SADF.getKernel(i)->getName() + "' is too large to be supported.");
        }
    }

    // Create SDF graph

    SDFcomponent Component (nullptr, SADF.getIdentity());

    auto SDF = std::make_unique<TimedSDFgraph>(Component);
    SDF->setName(SADF.getName());
    SDF->setType("sdf");

    // Create Actors

    for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {

        TimedSDFactor *Actor = SDF->createActor();
        Actor->setName(SADF.getKernel(i)->getName());
        Actor->setType(SADF.getKernel(i)->getName());

        // Create Input Ports

        SDFportRef Port = nullptr;

        for (auto *Channel : SADF.getKernel(i)->getDataInputChannels()) {

            Port = Actor->createPort(SDFport::SDFportType::In, Channel->getConsumptionRate(0));
            Port->setName(CString("In_") + Channel->getName());

            if (Channel->getBufferSize() != SADF_UNBOUNDED) {
                Port = Actor->createPort(SDFport::SDFportType::Out, Channel->getConsumptionRate(0));
                Port->setName(CString("Reverse_Out_") + Channel->getName());
            }
        }

        // Create Output Ports

        for (auto *Channel : SADF.getKernel(i)->getDataOutputChannels()) {

            Port = Actor->createPort(SDFport::SDFportType::Out, Channel->getProductionRate(0));
            Port->setName(CString("Out_") + Channel->getName());

            if (Channel->getBufferSize() != SADF_UNBOUNDED) {
                Port = Actor->createPort(SDFport::SDFportType::In, Channel->getProductionRate(0));
                Port->setName(CString("Reverse_In_") + Channel->getName());
            }
        }

        // Create Ports for Self-Edge

        Port = Actor->createPort(SDFport::SDFportType::Out, 1);
        Port->setName("Self_Out");

        Port = Actor->createPort(SDFport::SDFportType::In, 1);
        Port->setName("Self_In");

        // Create Processor for Execution Time

        TimedSDFactor::Processor *Processor = Actor->addProcessor(SADF.getKernel(i)->getName());
        Processor->execTime =
                static_cast<SDFtime>(SADF.getKernel(i)->getProfile(0, 0)->getExecutionTime());
        Actor->setDefaultProcessor(SADF.getKernel(i)->getName());
    }

    // Create Channels

    for (CId i = 0; i != SADF.getNumberOfDataChannels(); i++) {

        SDFcomponent Component(SDF.get(), SDF->nrChannels());
        std::unique_ptr<SDFchannel> Channel = std::make_unique<TimedSDFchannel>(Component);
        Channel->setName(SADF.getDataChannel(i)->getName());
        Channel->connectSrc(
                SDF->getActor(SADF.getDataChannel(i)->getSource()->getName())
                        ->getPort(CString("Out_") + SADF.getDataChannel(i)->getName()));
        Channel->connectDst(SDF->getActor(SADF.getDataChannel(i)->getDestination()->getName())
                                    ->getPort(CString("In_") + SADF.getDataChannel(i)->getName()));

        if (SADF.getDataChannel(i)->getNumberOfInitialTokens() != 0) {
            Channel->setInitialTokens(SADF.getDataChannel(i)->getNumberOfInitialTokens());
        }

        dynamic_cast<TimedSDFchannel*>(Channel.get())->setTokenSize(static_cast<int>(SADF.getDataChannel(i)->getTokenSize()));

        SDF->addChannel(std::move(Channel));

        if (SADF.getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED) {
            SDFcomponent Component(SDF.get(), SDF->nrChannels());
            std::unique_ptr<SDFchannel> ReverseChannel = std::make_unique<TimedSDFchannel>(Component);
            ReverseChannel->setName(CString("Reverse_") + SADF.getDataChannel(i)->getName());
            ReverseChannel->connectSrc(
                    SDF->getActor(SADF.getDataChannel(i)->getDestination()->getName())
                            ->getPort(CString("Reverse_Out_")
                                      + SADF.getDataChannel(i)->getName()));
            ReverseChannel->connectDst(
                    SDF->getActor(SADF.getDataChannel(i)->getSource()->getName())
                            ->getPort(CString("Reverse_In_")
                                      + SADF.getDataChannel(i)->getName()));
            ReverseChannel->setInitialTokens(
                    static_cast<uint>(SADF.getDataChannel(i)->getBufferSize()
                           - static_cast<CSize>(SADF.getDataChannel(i)->getNumberOfInitialTokens())));
            dynamic_cast<TimedSDFchannel*>(ReverseChannel.get())->setTokenSize(static_cast<int>(SADF.getDataChannel(i)->getTokenSize()));
            SDF->addChannel(std::move(ReverseChannel));
        }
    }

    for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {
        SDFcomponent Component(SDF.get(), SDF->nrChannels());
        std::unique_ptr<SDFchannel> SelfChannel = std::make_unique<TimedSDFchannel>(Component);
        SelfChannel->setName(CString("Self_") + SADF.getKernel(i)->getName());
        SelfChannel->connectSrc(SDF->getActor(SADF.getKernel(i)->getName())->getPort("Self_Out"));
        SelfChannel->connectDst(SDF->getActor(SADF.getKernel(i)->getName())->getPort("Self_In"));
        SelfChannel->setInitialTokens(1);
        dynamic_cast<TimedSDFchannel*>(SelfChannel.get())->setTokenSize(1);
        SDF->addChannel(std::move(SelfChannel));
    }

    return SDF;
}

std::unique_ptr<SDFgraph> SADF2UntimedSDF(const SADF_Graph& SADF) {

    // Conversion is assumed to be possible

    // Create SDF graph

    SDFcomponent Component(nullptr, SADF.getIdentity());

    auto SDF = std::make_unique<SDFgraph>(Component);
    SDF->setName(SADF.getName());
    SDF->setType("sdf");

    // Create Actors

    for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {

        SDFactorRef Actor = SDF->createActor();
        Actor->setName(SADF.getKernel(i)->getName());
        Actor->setType(SADF.getKernel(i)->getName());

        // Create Input Ports

        SDFportRef Port = nullptr;

        for (auto *Channel : SADF.getKernel(i)->getDataInputChannels()) {

             Port = Actor->createPort(SDFport::SDFportType::In, Channel->getConsumptionRate(0));
            Port->setName(CString("In_") + Channel->getName());

            if (Channel->getBufferSize() != SADF_UNBOUNDED) {
                Port = Actor->createPort(SDFport::SDFportType::Out, Channel->getConsumptionRate(0));
                Port->setName(CString("Reverse_Out_") + Channel->getName());
            }
        }

        // Create Output Ports

        for (auto *Channel : SADF.getKernel(i)->getDataOutputChannels()) {

             Port = Actor->createPort(SDFport::SDFportType::Out, Channel->getProductionRate(0));
            Port->setName(CString("Out_") + Channel->getName());

            if (Channel->getBufferSize() != SADF_UNBOUNDED) {
                Port = Actor->createPort(SDFport::SDFportType::In, Channel->getProductionRate(0));
                Port->setName(CString("Reverse_In_") + Channel->getName());
            }
        }

        // Create Ports for Self-Edge

        Port = Actor->createPort(SDFport::SDFportType::Out, 1);
        Port->setName("Self_Out");

        Port = Actor->createPort(SDFport::SDFportType::In, 1);
        Port->setName("Self_In");
    }

    // Create Channels

    for (CId i = 0; i != SADF.getNumberOfDataChannels(); i++) {

        SDFcomponent Component(SDF.get(), SDF->nrChannels());
        auto Channel = std::make_unique<SDFchannel>(Component);
        Channel->setName(SADF.getDataChannel(i)->getName());
        Channel->connectSrc(
                SDF->getActor(SADF.getDataChannel(i)->getSource()->getName())
                        ->getPort(CString("Out_") + SADF.getDataChannel(i)->getName()));
        Channel->connectDst(
                SDF->getActor(SADF.getDataChannel(i)->getDestination()->getName())
                        ->getPort(CString("In_") + SADF.getDataChannel(i)->getName()));

        if (SADF.getDataChannel(i)->getNumberOfInitialTokens() != 0) {
            Channel->setInitialTokens(SADF.getDataChannel(i)->getNumberOfInitialTokens());
}

        SDF->addChannel(std::move(Channel));

        if (SADF.getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED) {
            SDFcomponent Component(SDF.get(), SDF->nrChannels());
            auto ReverseChannel = std::make_unique<SDFchannel>(Component);
            ReverseChannel->setName(CString("Reverse_") + SADF.getDataChannel(i)->getName());
            ReverseChannel->connectSrc(
                    SDF->getActor(SADF.getDataChannel(i)->getDestination()->getName())
                            ->getPort(CString("Reverse_Out_")
                                      + SADF.getDataChannel(i)->getName()));
            ReverseChannel->connectDst(
                    SDF->getActor(SADF.getDataChannel(i)->getSource()->getName())
                            ->getPort(CString("Reverse_In_")
                                      + SADF.getDataChannel(i)->getName()));
            ReverseChannel->setInitialTokens(
                    static_cast<uint>(SADF.getDataChannel(i)->getBufferSize()
                           - static_cast<CSize>(SADF.getDataChannel(i)->getNumberOfInitialTokens())));
            SDF->addChannel(std::move(ReverseChannel));
        }
    }

    for (CId i = 0; i != SADF.getNumberOfKernels(); i++) {
        SDFcomponent Component(SDF.get(), SDF->nrChannels());
        auto SelfChannel = std::make_unique<SDFchannel>(Component);
        SelfChannel->setName(CString("Self_") + SADF.getKernel(i)->getName());
        SelfChannel->connectSrc(SDF->getActor(SADF.getKernel(i)->getName())->getPort("Self_Out"));
        SelfChannel->connectDst(SDF->getActor(SADF.getKernel(i)->getName())->getPort("Self_In"));
        SelfChannel->setInitialTokens(1);
        SDF->addChannel(std::move(SelfChannel));
    }

    return SDF;
}
