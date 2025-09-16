/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2xml.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in xml Format
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

#include "sadf2xml.h"

void SADF2XML(const SADF_Graph& Graph, std::ostream &out) {

    // Write preamble

    out << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
    out << "<sdf3 type='sadf' version='1.0' xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance' "
           "xmlns='uri:sadf' xsi:schemaLocation='uri:sadf "
           "https://www.es.ele.tue.nl/sadf/sdf3-sadf.xsd'>"
        << std::endl
        << std::endl;
    out << "<sadf name='" << Graph.getName() << "'>" << std::endl << std::endl;

    // Write Structure

    out << "<structure>" << std::endl;

    for (CId i = 0; i != Graph.getNumberOfKernels(); i++) {
        out << "  <kernel name='" << Graph.getKernel(i)->getName() << "'/>" << std::endl;
    }

    for (CId i = 0; i != Graph.getNumberOfDetectors(); i++) {
        out << "  <detector name='" << Graph.getDetector(i)->getName() << "'/>" << std::endl;
    }

    for (CId i = 0; i != Graph.getNumberOfDataChannels(); i++) {
        out << "  <channel name='" << Graph.getDataChannel(i)->getName()
            << "' type='data' source='" << Graph.getDataChannel(i)->getSource()->getName()
            << "' destination='" << Graph.getDataChannel(i)->getDestination()->getName() << "'/>"
            << std::endl;
    }

    for (CId i = 0; i != Graph.getNumberOfControlChannels(); i++) {
        out << "  <channel name='" << Graph.getControlChannel(i)->getName()
            << "' type='control' source='" << Graph.getControlChannel(i)->getSource()->getName()
            << "' destination='" << Graph.getControlChannel(i)->getDestination()->getName()
            << "'/>" << std::endl;
    }

    out << "</structure>" << std::endl << std::endl;

    // Write Properties

    out << "<properties>" << std::endl;

    for (CId i = 0; i != Graph.getNumberOfKernels(); i++) {

        SADF_Process *Kernel = Graph.getKernel(i);

        out << "  <kernel_properties kernel='" << Kernel->getName() << "'>" << std::endl;

        for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++) {
            if (Kernel->isActive(j) || Kernel->hasMultipleControlInputChannels()
                || Kernel->getNumberOfProfiles(j) > 1
                || (Kernel->getNumberOfProfiles(j) == 1
                    && Kernel->getProfile(j, 0)->getExecutionTime() > 0)) {

                out << "    <scenario name='" << Kernel->getScenarioName(j) << "'>" << std::endl;

                if (Kernel->hasMultipleControlInputChannels()) {
                    for (CId k = 0; k != Kernel->getControlInputChannels().size(); k++) {
                        out << "      <control channel='"
                            << Kernel->getControlInputChannels()[k]->getName() << "' value='"
                            << Kernel->getControlValue(j, k) << "'/>" << std::endl;
                    }
                }

                for (const auto &k : Kernel->getDataInputChannels()) {
                    if (k->getConsumptionRate(j) != 0) {
                        out << "      <consume channel='" << k->getName() << "' tokens='"
                            << k->getConsumptionRate(j) << "'/>" << std::endl;
                    }
                }

                for (const auto &k : Kernel->getDataOutputChannels()) {
                    if (k->getProductionRate(j) != 0) {
                        out << "      <produce channel='" << k->getName() << "' tokens='"
                            << k->getProductionRate(j) << "'/>" << std::endl;
                    }
                }

                for (CId k = 0; k != Kernel->getNumberOfProfiles(j); k++) {
                    if (Kernel->getProfile(j, k)->getExecutionTime() != 0) {

                        if (Kernel->getProfile(j, k)->getExecutionTime()
                            != static_cast<CDouble>(static_cast<uint64_t>(
                                    Kernel->getProfile(j, k)->getExecutionTime()))) {
                            out << "      <profile execution_time='"
                                << Kernel->getProfile(j, k)->getExecutionTime() << "'";
                        } else {
                            out << "      <profile execution_time='"
                                << (static_cast<uint64_t>(
                                           Kernel->getProfile(j, k)->getExecutionTime()))
                                << "'";
                        }

                        if (Kernel->getProfile(j, k)->getWeight() != 1) {
                            out << " weight='" << Kernel->getProfile(j, k)->getWeight() << "'";
                        }

                        out << "/>" << std::endl;
                    }
                }

                out << "    </scenario>" << std::endl;
            }
        }

        out << "  </kernel_properties>" << std::endl << std::endl;
    }

    for (CId i = 0; i != Graph.getNumberOfDetectors(); i++) {

        SADF_Process *Detector = Graph.getDetector(i);

        out << "  <detector_properties detector='" << Detector->getName() << "'>" << std::endl;

        if (Detector->hasMultipleControlInputChannels()) {
            for (CId j = 0; j != Detector->getNumberOfScenarios(); j++) {

                out << "    <scenario name='" << Detector->getScenarioName(j) << "'>" << std::endl;

                for (CId k = 0; k != Detector->getControlInputChannels().size(); k++) {
                    out << "      <control channel='"
                        << Detector->getControlInputChannels()[k]->getName() << "' value'"
                        << Detector->getControlValue(j, k) << "'/>" << std::endl;
                }

                out << "    </scenario>" << std::endl;
            }
        }

        for (CId j = 0; j != Detector->getNumberOfSubScenarios(); j++) {

            out << "    <subscenario name='" << Detector->getSubScenarioName(j) << "'>"
                << std::endl;

            for (const auto &k : Detector->getDataInputChannels()) {
                if (k->getConsumptionRate(j) != 0) {
                    out << "      <consume channel='" << k->getName() << "' tokens='"
                        << k->getConsumptionRate(j) << "'/>" << std::endl;
                }
            }

            for (const auto &k : Detector->getDataOutputChannels()) {
                if (k->getProductionRate(j) != 0) {
                    out << "      <produce channel='" << k->getName() << "' tokens='"
                        << k->getProductionRate(j) << "'/>" << std::endl;
                }
            }

            for (CId k = 0; k != Detector->getControlOutputChannels().size(); k++) {

                out << "      <produce channel='"
                    << Detector->getControlOutputChannels()[k]->getName() << "' tokens='"
                    << Detector->getControlOutputChannels()[k]->getProductionRate(j) << "'";

                if (Detector->getControlOutputChannels()[k]->getProductionScenarioName(j)
                    != Detector->getSubScenarioName(j)) {
                    out << " value='"
                        << Detector->getControlOutputChannels()[k]->getProductionScenarioName(j)
                        << "'";
                }

                out << "/>" << std::endl;
            }

            for (CId k = 0; k != Detector->getNumberOfProfiles(j); k++) {
                if (Detector->getProfile(j, k)->getExecutionTime() != 0) {

                    if (Detector->getProfile(j, k)->getExecutionTime()
                        != static_cast<CDouble>(static_cast<uint64_t>(
                                Detector->getProfile(j, k)->getExecutionTime()))) {
                        out << "      <profile execution_time='"
                            << Detector->getProfile(j, k)->getExecutionTime() << "'";
                    } else {
                        out << "      <profile execution_time='"
                            << (static_cast<uint64_t>(
                                       Detector->getProfile(j, k)->getExecutionTime()))
                            << "'";
                    }

                    if (Detector->getProfile(j, k)->getWeight() != 1) {
                        out << " weight='" << Detector->getProfile(j, k)->getWeight() << "'";
                    }

                    out << "/>" << std::endl;
                }
            }

            out << "    </subscenario>" << std::endl;
        }

        for (CId j = 0; j != Detector->getNumberOfScenarios(); j++) {

            out << "    <markov_chain ";

            if (Detector->hasControlInputChannels()) {
                out << "scenario='" << Detector->getScenarioName(j) << "' ";
            }

            out << "initial_state='"
                << Detector->getMarkovChain(j)->getStateName(
                           Detector->getMarkovChain(j)->getInitialState())
                << "'>" << std::endl;

            for (CId k = 0; k != Detector->getMarkovChain(j)->getNumberOfStates(); k++) {

                out << "      <state name='" << Detector->getMarkovChain(j)->getStateName(k)
                    << "' subscenario='"
                    << Detector->getSubScenarioName(Detector->getMarkovChain(j)->getSubScenario(k))
                    << "'>" << std::endl;

                for (CId n = 0; n != Detector->getMarkovChain(j)->getNumberOfStates(); n++) {
                    if (Detector->getMarkovChain(j)->getTransitionProbability(k, n) != 0) {
                        out << "        <transition destination='"
                            << Detector->getMarkovChain(j)->getStateName(n) << "' weight='"
                            << Detector->getMarkovChain(j)->getTransitionProbability(k, n) << "'/>"
                            << std::endl;
                    }
                }

                out << "      </state>" << std::endl;
            }

            out << "    </markov_chain>" << std::endl;
        }

        out << "  </detector_properties>" << std::endl << std::endl;
    }

    for (CId i = 0; i != Graph.getNumberOfDataChannels(); i++) {
        if (Graph.getDataChannel(i)->getNumberOfInitialTokens() != 0
            || Graph.getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED
            || Graph.getDataChannel(i)->getTokenSize() != 1) {

            out << "  <channel_properties channel='" << Graph.getDataChannel(i)->getName() << "'";

            if (Graph.getDataChannel(i)->getNumberOfInitialTokens() != 0) {
                out << " number_of_initial_tokens='"
                    << Graph.getDataChannel(i)->getNumberOfInitialTokens() << "'";
            }

            if (Graph.getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED) {
                out << " buffer_size='" << Graph.getDataChannel(i)->getBufferSize() << "'"
                    << std::endl;
            }

            if (Graph.getDataChannel(i)->getTokenSize() != 1) {
                out << " token_size='" << Graph.getDataChannel(i)->getTokenSize() << "'"
                    << std::endl;
            }

            out << "/>" << std::endl;
        }
    }

    for (CId i = 0; i != Graph.getNumberOfControlChannels(); i++) {
        if (Graph.getControlChannel(i)->getNumberOfInitialTokens() != 0
            || Graph.getControlChannel(i)->getBufferSize() != SADF_UNBOUNDED
            || Graph.getControlChannel(i)->getTokenSize() != 1) {

            SADF_Channel *Channel = Graph.getControlChannel(i);

            out << "  <channel_properties channel='" << Channel->getName() << "'";

            if (Channel->getBufferSize() != SADF_UNBOUNDED) {
                out << " buffer_size='" << Channel->getBufferSize() << "'" << std::endl;
            }

            if (Channel->getTokenSize() != 1) {
                out << " token_size='" << Channel->getTokenSize() << "'" << std::endl;
            }

            if (Channel->getNumberOfInitialTokens() != 0) {

                CQueue NumbersQueue(Channel->getNumbersQueue());
                CQueue ContentQueue(Channel->getContentQueue());

                out << " initial_tokens='";

                while (!NumbersQueue.empty()) {

                    CId ID = ContentQueue.front();
                    CString TokenValue;

                    if (Channel->getDestination()->hasMultipleControlInputChannels()) {

                        bool ScenarioNotFound = true;

                        CId ChannelID = Channel->getDestination()->getControlInputChannelID(
                                Channel->getName());

                        for (CId j = 0; ScenarioNotFound
                                        && j != Channel->getDestination()->getNumberOfScenarios();
                             j++) {
                            if (Channel->getDestination()->getControlID(j, ChannelID) == ID) {
                                ScenarioNotFound = false;
                                TokenValue =
                                        Channel->getDestination()->getControlValue(j, ChannelID);
                            }
                        }

                    } else {
                        TokenValue = Channel->getDestination()->getScenarioName(ID);
                    }

                    if (NumbersQueue.front() == 1) {
                        out << TokenValue;
                    } else {
                        out << NumbersQueue.front() << "*" << TokenValue;
                    }

                    if (NumbersQueue.size() != 1) {
                        out << ", ";
                    }

                    NumbersQueue.pop();
                    ContentQueue.pop();
                }

                out << "'";
            }

            out << "/>" << std::endl;
        }
    }

    out << "</properties>" << std::endl << std::endl;

    // Write postamble

    out << "</sadf>" << std::endl;
    out << "</sdf3>" << std::endl;
}

void FSMSADF2XML(const FSMSADF::Graph& g, std::ostream &out) {
    // Create XML
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "fsmsadf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

    // Application graph node
    g.convertToXML(CAddNode(sdf3Node, "applicationGraph"));

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}
