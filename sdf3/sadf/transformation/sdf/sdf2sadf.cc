/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf2sadf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SDF Graph in SADF Graph
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

#include "sdf2sadf.h"
#include <memory>

using namespace SDF;

std::unique_ptr<SADF_Graph> SDF2SADF(const TimedSDFgraph& SDF) {

    auto SADF = std::make_unique<SADF_Graph>(SDF.getName(), SDF.getId());

    // Create Kernels

    for (const auto& i : SDF.getActors()) {
        SADF->addKernel(std::make_unique<SADF_Process>(i->getName(), SADF->getNumberOfKernels(), SADF_KERNEL));
    }

    // Create Channels

    for (const auto& i : SDF.getChannels()) {

        auto* Channel = dynamic_cast<TimedSDFchannel*>(i.get());
        auto DataChannel = std::make_unique<SADF_Channel>(
                Channel->getName(), SADF->getNumberOfDataChannels(), SADF_DATA_CHANNEL);

        if (Channel->isConnected()) {

            DataChannel->setSource(SADF->getKernel(Channel->getSrcActor()->getName()));
            DataChannel->setDestination(SADF->getKernel(Channel->getDstActor()->getName()));

            DataChannel->getSource()->addDataOutputChannel(DataChannel.get());
            DataChannel->getDestination()->addDataInputChannel(DataChannel.get());

        } else {
            throw CException(CString("Channel '") + Channel->getName() + "' of SDF graph '"
                             + SDF.getName() + "' is not connected.");
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

    // Finalise Creation of Kernels

    for (const auto& i : SDF.getActors()) {

        auto* Actor = dynamic_cast<TimedSDFactor*>(i.get());
        SADF_Process *Kernel = SADF->getKernel(Actor->getName());

        CId ScenarioID = Kernel->addScenario("default");

        auto Profile = std::make_unique<SADF_Profile>();

        for (const auto& j : Actor->getProcessors()) {
            if (Actor->getDefaultProcessor() == j->type) {
                Profile->setExecutionTime(static_cast<CDouble>(j->execTime));
            }
        }

        Kernel->addProfile(ScenarioID, std::move(Profile));
    }

    // Finalise Creation of Channels

    for (const auto& i : SDF.getChannels()) {

        auto* Channel = dynamic_cast<TimedSDFchannel*>(i.get());
        SADF_Channel *DataChannel = SADF->getChannel(Channel->getName());

        DataChannel->setConsumptionRate(0, Channel->getDstPort()->getRate());
        DataChannel->setProductionRate(0, Channel->getSrcPort()->getRate());
    }

    return SADF;
}
