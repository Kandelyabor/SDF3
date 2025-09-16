/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2html.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in html or php Format
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

#include "sadf2html.h"

#include <cmath>

namespace {


void SADF_Graph2AnnotatedDOT(SADF_Graph *Graph, bool php, std::ostream &out) {

    CId i = 0;

    out << "digraph " << Graph->getName() << " {" << std::endl;

    out << std::endl;

    // Output all kernels

    for (i = 0; i != Graph->getNumberOfKernels(); i++) {
        out << "    K" << Graph->getKernel(i)->getIdentity() << " [ label=\""
            << Graph->getKernel(i)->getName();
        if (php) {
            out << "\", URL=\"?id=";
        } else {
            { out << "\", URL=\"#"; }
        }
        out << Graph->getKernel(i)->getName() << "\" ];" << std::endl;
    }

    out << std::endl;

    // Output all detectors

    for (i = 0; i != Graph->getNumberOfDetectors(); i++) {
        out << "    D" << Graph->getDetector(i)->getIdentity() << " [ label=\""
            << Graph->getDetector(i)->getName();
        if (php) {
            out << "\", style=dotted , URL=\"?id=";
        } else {
            out << "\", style=dotted , URL=\"#";
        }
        out << Graph->getDetector(i)->getName() << "\" ];" << std::endl;
    }

    out << std::endl;

    // Output all data channels

    for (i = 0; i != Graph->getNumberOfDataChannels(); i++) {

        bool FixedProductionRate = true;
        bool FixedConsumptionRate = true;
        CId ProductionRate = SADF_UNDEFINED;
        CId ConsumptionRate = SADF_UNDEFINED;

        SADF_Channel *Channel = Graph->getDataChannel(i);
        SADF_Process *Process = Channel->getSource();

        if (Process->getType() == SADF_KERNEL) {
            for (CId n = 0; FixedProductionRate && n != Process->getNumberOfScenarios(); n++) {
                if (Channel->getProductionRate(n) != 0 && ProductionRate == SADF_UNDEFINED) {
                    ProductionRate = Channel->getProductionRate(n);
                } else {
                    if (Channel->getProductionRate(n) != ProductionRate) {
                        FixedProductionRate = false;
                    }
                }
            }
        }

        if (Process->getType() == SADF_DETECTOR) {
            for (CId n = 0; FixedProductionRate && n != Process->getNumberOfSubScenarios(); n++) {
                if (Channel->getProductionRate(n) != 0 && ProductionRate == SADF_UNDEFINED) {
                    ProductionRate = Channel->getProductionRate(n);
                } else {
                    if (Channel->getProductionRate(n) != ProductionRate) {
                        FixedProductionRate = false;
                    }
                }
            }
        }

        Process = Channel->getDestination();

        if (Process->getType() == SADF_KERNEL) {
            for (CId n = 0; FixedConsumptionRate && n != Process->getNumberOfScenarios(); n++) {
                if (Channel->getConsumptionRate(n) != 0 && ConsumptionRate == SADF_UNDEFINED) {
                    ConsumptionRate = Channel->getConsumptionRate(n);
                } else {
                    if (Channel->getConsumptionRate(n) != ConsumptionRate) {
                        FixedConsumptionRate = false;
                    }
                }
            }
        }

        if (Process->getType() == SADF_DETECTOR) {
            for (CId n = 0; FixedConsumptionRate && n != Process->getNumberOfSubScenarios(); n++) {
                if (Channel->getConsumptionRate(n) != 0 && ConsumptionRate == SADF_UNDEFINED) {
                    ConsumptionRate = Channel->getConsumptionRate(n);
                } else {
                    if (Channel->getConsumptionRate(n) != ConsumptionRate) {
                        FixedConsumptionRate = false;
                    }
                }
            }
        }

        if (Channel->getSource()->getType() == SADF_KERNEL) {
            out << "    K";
        } else {
            out << "    D";
        }

        out << Channel->getSource()->getIdentity() << " -> ";

        if (Channel->getNumberOfInitialTokens() != 0) {

#ifdef __DOT_EXPLICIT_INITIAL_TOKENS

            out << "DC" << Channel->getIdentity() << " [ label=\"" << Channel->getName() << "\"";

            if (FixedProductionRate)
                out << ", taillabel=\"" << ProductionRate << "\"";

            if (php)
                out << ", arrowhead=none, URL=\"?id=" << Channel->getName() << "\" ];" << std::endl;
            else
                out << ", arrowhead=none, URL=\"#" << Channel->getName() << "\" ];" << std::endl;

            out << "    DC" << Channel->getIdentity();

            if (php)
                out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";
            else
                out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";

            out << Channel->getName() << "\" ];" << std::endl;
            out << "    DC" << Channel->getIdentity() << " -> ";

            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";
            out << Channel->getDestination()->getIdentity() << " [ taillabel=\""
                << Channel->getNumberOfInitialTokens() << "\"";

            if (FixedConsumptionRate)
                out << ", headlabel=\"" << ConsumptionRate << "\"";

            if (php)
                out << ", URL=\"?id=" << Channel->getName() << "\" ];" << std::endl;
            else
                out << ", URL=\"#" << Channel->getName() << "\" ];" << std::endl;

#else // __DOT_EXPLICIT_INITIAL_TOKENS

            if (Channel->getDestination()->getType() == SADF_KERNEL) {
                out << "K";
            } else {
                out << "D";
            }

            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName()
                << "(" << Channel->getNumberOfInitialTokens() << ")\"";

            if (FixedProductionRate) {
                out << ", taillabel=\"" << ProductionRate << "\"";
            }

            if (FixedConsumptionRate) {
                out << ", headlabel=\"" << ConsumptionRate << "\"";
            }

            if (php) {
                out << ", URL=\"?id=" << Channel->getName() << "\" ];" << std::endl;
            } else {
                out << ", URL=\"#" << Channel->getName() << "\" ];" << std::endl;
            }

#endif // __DOT_EXPLICIT_INITIAL_TOKENS

        } else {
            if (Channel->getDestination()->getType() == SADF_KERNEL) {
                out << "K";
            } else {
                out << "D";
            }

            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName()
                << "\"";

            if (FixedProductionRate) {
                out << ", taillabel=\"" << ProductionRate << "\"";
            }

            if (FixedConsumptionRate) {
                out << ", headlabel=\"" << ConsumptionRate << "\"";
            }

            if (php) {
                out << ", URL=\"?id=" << Channel->getName() << "\" ];" << std::endl;
            } else {
                out << ", URL=\"#" << Channel->getName() << "\" ];" << std::endl;
            }
        }
    }

    out << std::endl;

    // Output all control channels

    for (i = 0; i != Graph->getNumberOfControlChannels(); i++) {

        SADF_Channel *Channel = Graph->getControlChannel(i);
        SADF_Process *Detector = Channel->getSource();

        bool FixedRate = true;
        CId Rate = SADF_UNDEFINED;

        for (CId n = 0; FixedRate && n != Detector->getNumberOfSubScenarios(); n++) {
            if (Channel->getProductionRate(n) != 0 && Rate == SADF_UNDEFINED) {
                Rate = Channel->getProductionRate(n);
            } else if (Channel->getProductionRate(n) != Rate) {
                FixedRate = false;
            }
        }

        out << "    D" << Detector->getIdentity() << " -> ";

        if (Channel->getNumberOfInitialTokens() != 0) {

#ifdef __DOT_EXPLICIT_INITIAL_TOKENS

            out << "CC" << Channel->getIdentity() << " [ label=\"" << Channel->getName() << "\"";

            if (FixedRate)
                out << ", taillabel=\"" << Rate << "\"";

            if (php)
                out << ", style=dotted, arrowhead=none, URL=\"?id=" << Channel->getName() << "\" ];"
                    << std::endl;
            else
                out << ", style=dotted, arrowhead=none, URL=\"#" << Channel->getName() << "\" ];"
                    << std::endl;

            out << "    CC" << Channel->getIdentity();

            if (php)
                out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";
            else
                out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";

            out << Channel->getName() << "\" ];" << std::endl;

            out << "    CC" << Channel->getIdentity() << " -> ";

            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";

            out << Channel->getDestination()->getIdentity();

            out << " [ taillabel=\"";

            CQueue NumbersQueue(Channel->getNumbersQueue());
            CQueue ContentQueue(Channel->getContentQueue());

            while (!NumbersQueue.empty()) {

                CId ID = ContentQueue.front();
                CString TokenValue;

                SADF_Process *Destination = Channel->getDestination();

                if (Destination->hasMultipleControlInputChannels()) {

                    bool ScenarioNotFound = true;

                    CId ChannelID = Destination->getControlInputChannelID(Channel->getName());

                    for (CId j = 0; ScenarioNotFound && j != Destination->getNumberOfScenarios();
                         j++)
                        if (Destination->getControlID(j, ChannelID) == ID) {
                            ScenarioNotFound = false;
                            TokenValue = Destination->getControlValue(j, ChannelID);
                        }

                } else
                    TokenValue = Destination->getScenarioName(ID);

                if (NumbersQueue.front() == 1)
                    out << TokenValue;
                else
                    out << NumbersQueue.front() << "*" << TokenValue;

                if (NumbersQueue.size() != 1)
                    out << ", ";

                NumbersQueue.pop();
                ContentQueue.pop();
            }

            out << "\", headlabel=\"1\", style=dotted";

            if (php)
                out << ", URL=\"?id=" << Channel->getName() << "\" ];" << std::endl;
            else
                out << ", URL=\"#" << Channel->getName() << "\" ];" << std::endl;

#else // __DOT_EXPLICIT_INITIAL_TOKENS

            if (Channel->getDestination()->getType() == SADF_KERNEL) {
                out << "K";
            } else {
                out << "D";
            }

            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName()
                << "(";

            CQueue NumbersQueue(Channel->getNumbersQueue());
            CQueue ContentQueue(Channel->getContentQueue());

            while (!NumbersQueue.empty()) {

                CId ID = ContentQueue.front();
                CString TokenValue;

                SADF_Process *Destination = Channel->getDestination();

                if (Destination->hasMultipleControlInputChannels()) {

                    bool ScenarioNotFound = true;

                    CId ChannelID = Destination->getControlInputChannelID(Channel->getName());

                    for (CId j = 0; ScenarioNotFound && j != Destination->getNumberOfScenarios();
                         j++) {
                        if (Destination->getControlID(j, ChannelID) == ID) {
                            ScenarioNotFound = false;
                            TokenValue = Destination->getControlValue(j, ChannelID);
                        }
                    }

                } else {
                    TokenValue = Destination->getScenarioName(ID);
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

            out << ")\"";

            if (FixedRate) {
                out << ", taillabel=\"" << Rate << "\"";
            }

            out << ", headlabel=\"1\", style=dotted";

            if (php) {
                out << ", URL=\"?id=" << Channel->getName() << "\" ];" << std::endl;
            } else {
                out << ", URL=\"#" << Channel->getName() << "\" ];" << std::endl;
            }

#endif // __DOT_EXPLICIT_INITIAL_TOKENS

        } else {
            if (Channel->getDestination()->getType() == SADF_KERNEL) {
                out << "K";
            } else {
                out << "D";
            }

            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName()
                << "\"";

            if (FixedRate) {
                out << ", taillabel=\"" << Rate << "\"";
            }

            if (php) {
                out << R"(, headlabel="1", style=dotted, URL="?id=)" << Channel->getName()
                    << "\" ];" << std::endl;
            } else {
                out << R"(, headlabel="1", style=dotted, URL="#)" << Channel->getName() << "\" ];"
                    << std::endl;
            }
        }
    }

    out << "}" << std::endl;
}


void SADF_MarkovChain2DOT(SADF_Process *Detector, CId ScenarioID, std::ostream &out) {

    SADF_MarkovChain *MarkovChain = Detector->getMarkovChain(ScenarioID);

    out << "digraph D" << Detector->getIdentity() << "x" << ScenarioID << " {" << std::endl;

    out << std::endl;

    // Output all states

    for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
        out << "    S" << i << " [ label=\"" << MarkovChain->getStateName(i) << "\"";
        if (i == MarkovChain->getInitialState()) {
            out << ", style=\"bold\" ];" << std::endl;
        } else {
            out << " ];" << std::endl;
        }
    }

    out << std::endl;

    // Output all transitions

    for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
        for (CId j = 0; j != MarkovChain->getNumberOfStates(); j++) {
            if (MarkovChain->getTransitionProbability(i, j) != 0) {
                out << "    S" << i << " -> S" << j << " [ label=\""
                    << MarkovChain->getTransitionProbability(i, j) << "\" ];" << std::endl;
            }
        }
    }

    out << "}" << std::endl;
}

// NOLINTBEGIN(concurrency-mt-unsafe,cert-env33-c)

void SADF_Graph2JPG(SADF_Graph *Graph,
                    bool php,
                    const CString &dot,
                    const CString &target,
                    const CString &FileName) {

    std::ofstream OutputFile;

    OutputFile.open(target + FileName + ".dot");
    SADF_Graph2AnnotatedDOT(Graph, php, OutputFile);
    OutputFile.close();

    int exit = system((dot + "dot -Tcmapx -o" + target + FileName + ".map -Tjpg -o" + target
                       + FileName + ".jpg " + target + FileName + ".dot")
                              .c_str());

    if (exit != 0) {
        throw CException(CString("Error: Conversion from dot format to jpg format unsuccessful "
                                 "for SADF Graph '")
                         + Graph->getName() + "'.");
    }

    exit = system((("rm ") + target + FileName + ".dot").c_str());
}

void SADF_MarkovChain2JPG(const CString &GraphName,
                          SADF_Process *Detector,
                          CId ScenarioID,
                          const CString &dot,
                          const CString &target,
                          const CString &FileName) {

    std::ofstream OutputFile;

    OutputFile.open(target + FileName + ".dot");
    SADF_MarkovChain2DOT(Detector, ScenarioID, OutputFile);
    OutputFile.close();

    int exit =
            system((dot + "dot -Tjpg -o" + target + FileName + ".jpg " + target + FileName + ".dot")
                           .c_str());

    if (exit != 0) {
        throw CException(CString("Error: Conversion from dot format to jpg format unsuccessful "
                                 "for Markov Chain corresponding to Scenario '")
                         + Detector->getScenarioName(ScenarioID) + "' of Detector '"
                         + Detector->getName() + "' of SADF Graph '" + GraphName + "'.");
    }

    exit = system((("rm ") + target + FileName + ".dot").c_str());
}



} // namespace

void SADF2HTML(SADF_Graph *Graph,
               bool php,
               const CString &dot,
               const CString &target,
               const CString &url,
               std::ostream &out) {

    bool FixedProductionRate = true;
    bool FixedConsumptionRate = true;
    bool FixedExecutionTime = true;
    bool AlreadyListed = false;
    CId ProductionRate = 0;
    CId ConsumptionRate = 0;
    CDouble ExecutionTime = NAN;

    std::ifstream InputFile;

    char c = 0;

    // Generation of html

    out << "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' "
           "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>"
        << std::endl;
    out << "<html xmlns='http://www.w3.org/1999/xhtml'>" << std::endl << std::endl;
    out << "<head>" << std::endl;
    out << "  <title>Scenario-Aware Dataflow</title>" << std::endl;
    out << "  <meta http-equiv='Content-type' content='text/html;charset=iso-8859-1'/>"
        << std::endl;
    out << "  <link rel='stylesheet' type='text/css' media='screen' "
           "href='https://www.es.ele.tue.nl/sadf/style/master.css'/>"
        << std::endl;
    out << "</head>" << std::endl << std::endl;
    out << "<body>" << std::endl;

    out << "<a name='" << Graph->getName() << "'></a><h1>SADF Graph: " << Graph->getName()
        << "</h1>" << std::endl
        << std::endl;

    CString GraphFileName = Graph->getName();
    SADF_Graph2JPG(Graph, php, dot, target, GraphFileName);

    InputFile.open(target + GraphFileName + ".map");
    c = static_cast<char>(InputFile.get());
    while (!InputFile.eof()) {
        out << c;
        c = static_cast<char>(InputFile.get());
    }
    InputFile.close();

    int exit = system((("rm ") + target + GraphFileName + ".map").c_str());

    if (exit != 0) {
        throw CException(CString("Error: Couldn't delete temporary picture map file."));
    }

    out << "<img border='0' src='" << url << GraphFileName << ".jpg' usemap='#" << Graph->getName()
        << "'/>" << std::endl
        << std::endl;

    if (php) {
        out << "<p>Click on an object in the image to see its properties</p>" << std::endl
            << std::endl;
        out << "<?php" << std::endl << std::endl << "$id = $_GET['id'];" << std::endl << std::endl;
    }

    // Output all kernel specifications

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {

        if (php) {
            out << "if ($id == " << Graph->getKernel(i)->getName() << ") { ?>" << std::endl;
        }

        out << "<hr /><h2><a name='" << Graph->getKernel(i)->getName()
            << "'</a>Kernel: " << Graph->getKernel(i)->getName() << "</h2>" << std::endl;

        // Print Scenarios

        out << "<h3>Scenarios</h3>" << std::endl << std::endl << "<p>";

        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
            out << Graph->getKernel(i)->getScenarioName(j);
            if (j < Graph->getKernel(i)->getNumberOfScenarios() - 1) {
                out << ", ";
            }
        }

        out << "</p>" << std::endl << std::endl;

        // Print Scenario Definitions

        if (Graph->getKernel(i)->hasMultipleControlInputChannels()) {

            out << "<h3>Scenario Definitions</h3>" << std::endl << std::endl;

            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr><th>Scenario</th><th>Channel</th><th>Value</th></tr>" << std::endl;

            for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {

                AlreadyListed = false;

                for (CId k = 0; k != Graph->getKernel(i)->getControlInputChannels().size(); k++) {
                    out << "<tr>" << std::endl;
                    if (!AlreadyListed) {
                        out << "  <td rowspan='"
                            << Graph->getKernel(i)->getControlInputChannels().size() << "'>"
                            << Graph->getKernel(i)->getScenarioName(j) << "</td>" << std::endl;
                    }
                    out << "  <td>" << Graph->getKernel(i)->getControlInputChannels()[k]->getName()
                        << "</td>" << std::endl;
                    out << "  <td>" << Graph->getKernel(i)->getControlValue(j, k) << "</td>"
                        << std::endl;
                    out << "</tr>" << std::endl;
                    AlreadyListed = true;
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        // Print Parameterised Rates

        for (CId j = 0; j != Graph->getKernel(i)->getDataInputChannels().size(); j++) {

            ConsumptionRate = Graph->getKernel(i)->getDataInputChannels()[j]->getConsumptionRate(0);
            FixedConsumptionRate = true;

            for (CId s = 0;
                 FixedConsumptionRate && s != Graph->getKernel(i)->getNumberOfScenarios();
                 s++) {
                if (Graph->getKernel(i)->getDataInputChannels()[j]->getConsumptionRate(s)
                    != ConsumptionRate) {
                    FixedConsumptionRate = false;
                }
            }
        }

        for (CId j = 0; j != Graph->getKernel(i)->getDataOutputChannels().size(); j++) {

            ProductionRate = Graph->getKernel(i)->getDataOutputChannels()[j]->getProductionRate(0);
            FixedProductionRate = true;

            for (CId s = 0; FixedProductionRate && s != Graph->getKernel(i)->getNumberOfScenarios();
                 s++) {
                if (Graph->getKernel(i)->getDataOutputChannels()[j]->getProductionRate(s)
                    != ProductionRate) {
                    FixedProductionRate = false;
                }
            }
        }

        if ((!FixedConsumptionRate | !FixedProductionRate) != 0) { // NOLINT(*bool-conversion)

            out << "<h3>Parameterised Rates</h3>" << std::endl << std::endl;
            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr>" << std::endl;
            out << "  <th>Channel</th>" << std::endl;
            out << "  <th>Scenario</th>" << std::endl;
            out << "  <th>Rate</th>" << std::endl;
            out << "</tr>" << std::endl;

            for (CId j = 0; j != Graph->getKernel(i)->getDataInputChannels().size(); j++) {

                ConsumptionRate =
                        Graph->getKernel(i)->getDataInputChannels()[j]->getConsumptionRate(0);
                FixedConsumptionRate = true;

                for (CId s = 0;
                     FixedConsumptionRate && s != Graph->getKernel(i)->getNumberOfScenarios();
                     s++) {
                    if (Graph->getKernel(i)->getDataInputChannels()[j]->getConsumptionRate(s)
                        != ConsumptionRate) {
                        FixedConsumptionRate = false;
                    }
                }

                AlreadyListed = false;

                if (!FixedConsumptionRate) {
                    for (CId s = 0; s != Graph->getKernel(i)->getNumberOfScenarios(); s++) {
                        out << "<tr>" << std::endl;
                        if (!AlreadyListed) {
                            out << "  <td rowspan='" << Graph->getKernel(i)->getNumberOfScenarios()
                                << "'>" << Graph->getKernel(i)->getDataInputChannels()[j]->getName()
                                << "</td>" << std::endl;
                        }
                        out << "  <td>" << Graph->getKernel(i)->getScenarioName(s) << "</td>"
                            << std::endl;
                        out << "  <td>"
                            << Graph->getKernel(i)->getDataInputChannels()[j]->getConsumptionRate(s)
                            << "</td>" << std::endl;
                        out << "</tr>" << std::endl;
                        AlreadyListed = true;
                    }
                }
            }

            for (CId j = 0; j != Graph->getKernel(i)->getDataOutputChannels().size(); j++) {

                ProductionRate =
                        Graph->getKernel(i)->getDataOutputChannels()[j]->getProductionRate(0);
                FixedProductionRate = true;

                for (CId s = 0;
                     FixedProductionRate && s != Graph->getKernel(i)->getNumberOfScenarios();
                     s++) {
                    if (Graph->getKernel(i)->getDataOutputChannels()[j]->getProductionRate(s)
                        != ProductionRate) {
                        FixedProductionRate = false;
                    }
                }

                AlreadyListed = false;

                if (!FixedProductionRate) {
                    for (CId s = 0; s != Graph->getKernel(i)->getNumberOfScenarios(); s++) {
                        out << "<tr>" << std::endl;
                        if (!AlreadyListed) {
                            out << "  <td rowspan='" << Graph->getKernel(i)->getNumberOfScenarios()
                                << "'>"
                                << Graph->getKernel(i)->getDataOutputChannels()[j]->getName()
                                << "</td>" << std::endl;
                        }
                        out << "  <td>" << Graph->getKernel(i)->getScenarioName(s) << "</td>"
                            << std::endl;
                        out << "  <td>"
                            << Graph->getKernel(i)->getDataOutputChannels()[j]->getProductionRate(s)
                            << "</td>" << std::endl;
                        out << "</tr>" << std::endl;
                        AlreadyListed = true;
                    }
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        // Print Execution Time Distributions

        ExecutionTime = Graph->getKernel(i)->getProfile(0, 0)->getExecutionTime();
        FixedExecutionTime = true;

        for (CId j = 0; FixedExecutionTime && j != Graph->getKernel(i)->getNumberOfScenarios();
             j++) {
            for (CId k = 0; FixedExecutionTime && k != Graph->getKernel(i)->getNumberOfProfiles(j);
                 k++) {
                if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() != ExecutionTime) {
                    FixedExecutionTime = false;
                }
            }
        }

        if (FixedExecutionTime) {

            out << "<h3>Execution Time</h3>" << std::endl
                << "<p>ExecutionTime: " << ExecutionTime << "</p>" << std::endl
                << std::endl;

        } else {

            out << "<h3>Execution Time Distributions</h3>" << std::endl << std::endl;
            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr>" << std::endl;
            out << "  <th>Scenario</th>" << std::endl;
            out << "  <th>Execution Time</th>" << std::endl;
            out << "  <th>Probability</th>" << std::endl;
            out << "</tr>" << std::endl;

            for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {

                AlreadyListed = false;

                for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++) {
                    out << "<tr>" << std::endl;
                    if (!AlreadyListed) {
                        out << "  <td rowspan='" << Graph->getKernel(i)->getNumberOfProfiles(j)
                            << "'>" << Graph->getKernel(i)->getScenarioName(j) << "</td>"
                            << std::endl;
                    }
                    out << "  <td>" << Graph->getKernel(i)->getProfile(j, k)->getExecutionTime()
                        << "</td>" << std::endl;
                    out << "  <td>" << Graph->getKernel(i)->getProfile(j, k)->getWeight() << "</td>"
                        << std::endl;
                    out << "</tr>" << std::endl;
                    AlreadyListed = true;
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        if (php) {
            out << "<?php }" << std::endl << std::endl;
        } else {
            out << "<a href='#" << Graph->getName() << "'>Back to Top</a>" << std::endl
                << std::endl;
        }
    }

    // Output all detector specifications

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {

        if (php) {
            out << "if ($id == " << Graph->getDetector(i)->getName() << ") { ?>" << std::endl;
        }

        out << "<hr /><h2><a name='" << Graph->getDetector(i)->getName()
            << "'</a>Detector: " << Graph->getDetector(i)->getName() << "</h2>" << std::endl;

        // Print Scenarios and SubScenarios

        out << "<h3>Scenarios</h3>" << std::endl << "<p>";

        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {
            out << "<a href='#" << Graph->getDetector(i)->getName() << "_"
                << Graph->getDetector(i)->getScenarioName(j) << "'>"
                << Graph->getDetector(i)->getScenarioName(j) << "</a>";
            if (j < Graph->getDetector(i)->getNumberOfScenarios() - 1) {
                out << ", ";
            }
        }

        out << "</p>" << std::endl << std::endl;

        // Print Scenario Definitions

        if (Graph->getDetector(i)->hasMultipleControlInputChannels()) {

            out << "<h3>Scenario Definitions</h3>" << std::endl << std::endl;

            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr><th>Scenario</th><th>Channel</th><th>Value</th></tr>" << std::endl;

            for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {

                AlreadyListed = false;

                for (CId k = 0; k != Graph->getDetector(i)->getControlInputChannels().size(); k++) {
                    out << "<tr>" << std::endl;
                    if (!AlreadyListed) {
                        out << "  <td rowspan='"
                            << Graph->getDetector(i)->getControlInputChannels().size() << "'>"
                            << Graph->getDetector(i)->getScenarioName(j) << "</td>" << std::endl;
                    }
                    out << "  <td>"
                        << Graph->getDetector(i)->getControlInputChannels()[k]->getName() << "</td>"
                        << std::endl;
                    out << "  <td>" << Graph->getDetector(i)->getControlValue(j, k) << "</td>"
                        << std::endl;
                    out << "</tr>" << std::endl;
                    AlreadyListed = true;
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        out << "<h3>SubScenarios</h3>" << std::endl << "<p>";

        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
            out << Graph->getDetector(i)->getSubScenarioName(j);
            if (j < Graph->getDetector(i)->getNumberOfSubScenarios() - 1) {
                out << ", ";
            }
        }

        out << "</p>" << std::endl << std::endl;

        // Print Parameterised Rates

        for (CId j = 0; j != Graph->getDetector(i)->getDataInputChannels().size(); j++) {

            ConsumptionRate =
                    Graph->getDetector(i)->getDataInputChannels()[j]->getConsumptionRate(0);
            FixedConsumptionRate = true;

            for (CId s = 0;
                 FixedConsumptionRate && s != Graph->getDetector(i)->getNumberOfSubScenarios();
                 s++) {
                if (Graph->getDetector(i)->getDataInputChannels()[j]->getConsumptionRate(s)
                    != ConsumptionRate) {
                    FixedConsumptionRate = false;
                }
            }
        }

        for (CId j = 0; j != Graph->getDetector(i)->getDataOutputChannels().size(); j++) {

            ProductionRate =
                    Graph->getDetector(i)->getDataOutputChannels()[j]->getProductionRate(0);
            FixedProductionRate = true;

            for (CId s = 0;
                 FixedProductionRate && s != Graph->getDetector(i)->getNumberOfSubScenarios();
                 s++) {
                if (Graph->getDetector(i)->getDataOutputChannels()[j]->getProductionRate(s)
                    != ProductionRate) {
                    FixedProductionRate = false;
                }
            }
        }

        for (CId j = 0; j != Graph->getDetector(i)->getControlOutputChannels().size(); j++) {

            ProductionRate =
                    Graph->getDetector(i)->getControlOutputChannels()[j]->getProductionRate(0);

            for (CId s = 0;
                 FixedProductionRate && s != Graph->getDetector(i)->getNumberOfSubScenarios();
                 s++) {
                if (Graph->getDetector(i)->getControlOutputChannels()[j]->getProductionRate(s)
                    != ProductionRate) {
                    FixedProductionRate = false;
                }
            }
        }

        if ((!FixedConsumptionRate | !FixedProductionRate) != 0) { // NOLINT(*bool-conversion)

            out << "<h3>Parameterised Rates</h3>" << std::endl << std::endl;
            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr>" << std::endl;
            out << "  <th>Channel</th>" << std::endl;
            out << "  <th>SubScenario</th>" << std::endl;
            out << "  <th>Rate</th>" << std::endl;
            out << "</tr>" << std::endl;

            for (CId j = 0; j != Graph->getDetector(i)->getDataInputChannels().size(); j++) {

                ConsumptionRate =
                        Graph->getDetector(i)->getDataInputChannels()[j]->getConsumptionRate(0);
                FixedConsumptionRate = true;

                for (CId s = 0;
                     FixedConsumptionRate && s != Graph->getDetector(i)->getNumberOfSubScenarios();
                     s++) {
                    if (Graph->getDetector(i)->getDataInputChannels()[j]->getConsumptionRate(s)
                        != ConsumptionRate) {
                        FixedConsumptionRate = false;
                    }
                }

                AlreadyListed = false;

                if (!FixedConsumptionRate) {
                    for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
                        out << "<tr>" << std::endl;
                        if (!AlreadyListed) {
                            out << "  <td rowspan='"
                                << Graph->getDetector(i)->getNumberOfSubScenarios() << "'>"
                                << Graph->getDetector(i)->getDataInputChannels()[j]->getName()
                                << "</td>" << std::endl;
                        }
                        out << "  <td>" << Graph->getDetector(i)->getSubScenarioName(s) << "</td>"
                            << std::endl;
                        out << "  <td>"
                            << Graph->getDetector(i)->getDataInputChannels()[j]->getConsumptionRate(
                                       s)
                            << "</td>" << std::endl;
                        out << "</tr>" << std::endl;
                        AlreadyListed = true;
                    }
                }
            }

            for (CId j = 0; j != Graph->getDetector(i)->getDataOutputChannels().size(); j++) {

                ProductionRate =
                        Graph->getDetector(i)->getDataOutputChannels()[j]->getProductionRate(0);
                FixedProductionRate = true;

                for (CId s = 0;
                     FixedProductionRate && s != Graph->getDetector(i)->getNumberOfSubScenarios();
                     s++) {
                    if (Graph->getDetector(i)->getDataOutputChannels()[j]->getProductionRate(s)
                        != ProductionRate) {
                        FixedProductionRate = false;
                    }
                }

                AlreadyListed = false;

                if (!FixedProductionRate) {
                    for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
                        out << "<tr>" << std::endl;
                        if (!AlreadyListed) {
                            out << "  <td rowspan='"
                                << Graph->getDetector(i)->getNumberOfSubScenarios() << "'>"
                                << Graph->getDetector(i)->getDataOutputChannels()[j]->getName()
                                << "</td>" << std::endl;
                        }
                        out << "  <td>" << Graph->getDetector(i)->getSubScenarioName(s) << "</td>"
                            << std::endl;
                        out << "  <td>"
                            << Graph->getDetector(i)->getDataOutputChannels()[j]->getProductionRate(
                                       s)
                            << "</td>" << std::endl;
                        out << "</tr>" << std::endl;
                        AlreadyListed = true;
                    }
                }
            }

            for (CId j = 0; j != Graph->getDetector(i)->getControlOutputChannels().size(); j++) {

                ProductionRate =
                        Graph->getDetector(i)->getControlOutputChannels()[j]->getProductionRate(0);
                FixedProductionRate = true;

                for (CId s = 0;
                     FixedProductionRate && s != Graph->getDetector(i)->getNumberOfSubScenarios();
                     s++) {
                    if (Graph->getDetector(i)->getControlOutputChannels()[j]->getProductionRate(s)
                        != ProductionRate) {
                        FixedProductionRate = false;
                    }
                }

                AlreadyListed = false;

                if (!FixedProductionRate) {
                    for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
                        out << "<tr>" << std::endl;
                        if (!AlreadyListed) {
                            out << "  <td rowspan='"
                                << Graph->getDetector(i)->getNumberOfSubScenarios() << "'>"
                                << Graph->getDetector(i)->getControlOutputChannels()[j]->getName()
                                << "</td>" << std::endl;
                        }
                        out << "  <td>" << Graph->getDetector(i)->getSubScenarioName(s) << "</td>"
                            << std::endl;
                        out << "  <td>"
                            << Graph->getDetector(i)
                                        ->getControlOutputChannels()[j]
                                        ->getProductionRate(s)
                            << "</td>" << std::endl;
                        out << "</tr>" << std::endl;
                        AlreadyListed = true;
                    }
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        // Print Produced Control Tokens

        if (!Graph->getDetector(i)->getControlOutputChannels().empty()) {

            out << "<h3>Produced Control Tokens</h3>" << std::endl << std::endl;
            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr><th>Channel</th><th>SubScenario</th><th>Value</th></tr>" << std::endl;

            for (CId j = 0; j != Graph->getDetector(i)->getControlOutputChannels().size(); j++) {

                AlreadyListed = false;

                for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
                    out << "<tr>" << std::endl;
                    if (!AlreadyListed) {
                        out << "  <td rowspan='" << Graph->getDetector(i)->getNumberOfSubScenarios()
                            << "'>"
                            << Graph->getDetector(i)->getControlOutputChannels()[j]->getName()
                            << "</td>" << std::endl;
                    }
                    out << "  <td>" << Graph->getDetector(i)->getSubScenarioName(s) << "</td>"
                        << std::endl;
                    out << "  <td>"
                        << Graph->getDetector(i)
                                    ->getControlOutputChannels()[j]
                                    ->getProductionScenarioName(s)
                        << "</td>" << std::endl;
                    out << "</tr>" << std::endl;
                    AlreadyListed = true;
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        // Print Execution Time Distributions

        ExecutionTime = Graph->getDetector(i)->getProfile(0, 0)->getExecutionTime();
        FixedExecutionTime = true;

        for (CId j = 0; FixedExecutionTime && j != Graph->getDetector(i)->getNumberOfSubScenarios();
             j++) {
            for (CId k = 0;
                 FixedExecutionTime && k != Graph->getDetector(i)->getNumberOfProfiles(j);
                 k++) {
                if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() != ExecutionTime) {
                    FixedExecutionTime = false;
                }
            }
        }

        if (FixedExecutionTime) {

            out << "<h3>Execution Time</h3>" << std::endl
                << "<p>ExecutionTime: " << ExecutionTime << "</p>" << std::endl
                << std::endl;

        } else {

            out << "<h3>Execution Time Distributions</h3>" << std::endl << std::endl;
            out << "<table align='center' border='1' frame='hsides' cellpadding='5'>" << std::endl;
            out << "<tr>" << std::endl;
            out << "  <th>SubScenario</th>" << std::endl;
            out << "  <th>Execution Time</th>" << std::endl;
            out << "  <th>Probability</th>" << std::endl;
            out << "</tr>" << std::endl;

            for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {

                AlreadyListed = false;

                for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++) {
                    out << "<tr>" << std::endl;
                    if (!AlreadyListed) {
                        out << "  <td rowspan='" << Graph->getDetector(i)->getNumberOfProfiles(j)
                            << "'>" << Graph->getDetector(i)->getSubScenarioName(j) << "</td>"
                            << std::endl;
                    }
                    out << "  <td>" << Graph->getDetector(i)->getProfile(j, k)->getExecutionTime()
                        << "</td>" << std::endl;
                    out << "  <td>" << Graph->getDetector(i)->getProfile(j, k)->getWeight()
                        << "</td>" << std::endl;
                    out << "</tr>" << std::endl;
                    AlreadyListed = true;
                }
            }

            out << "</table>" << std::endl << std::endl;
        }

        // Print Markov Chains

        out << "<h3>Markov Chains</h3>" << std::endl;

        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {

            CString MarkovChainFileName = Graph->getName() + "_" + Graph->getDetector(i)->getName()
                                          + "_" + Graph->getDetector(i)->getScenarioName(j);

            SADF_MarkovChain2JPG(
                    Graph->getName(), Graph->getDetector(i), j, dot, target, MarkovChainFileName);

            out << "<table align='center' border='1' cellpadding='5'>" << std::endl;
            out << "<tr><th colspan='3'><a name='" << Graph->getDetector(i)->getName() << "_"
                << Graph->getDetector(i)->getScenarioName(j)
                << "'></a>Scenario: " << Graph->getDetector(i)->getScenarioName(j) << "</th></tr>";
            out << "<tr><td rowspan='"
                << Graph->getDetector(i)->getMarkovChain(j)->getNumberOfStates() + 1
                << "'><img border='0' src='" << url << MarkovChainFileName << ".jpg'/></td>"
                << "<th>State</th><th>SubScenario</th></tr>" << std::endl;

            for (CId s = 0; s != Graph->getDetector(i)->getMarkovChain(j)->getNumberOfStates();
                 s++) {

                out << "  <tr><td>" << Graph->getDetector(i)->getMarkovChain(j)->getStateName(s)
                    << "</td><td>"
                    << Graph->getDetector(i)->getSubScenarioName(
                               Graph->getDetector(i)->getMarkovChain(j)->getSubScenario(s))
                    << "</td></tr>" << std::endl;
            }

            out << "</table>" << std::endl << std::endl;
        }

        if (php) {
            out << "<?php }" << std::endl << std::endl;
        } else {
            out << "<a href='#" << Graph->getName() << "'>Back to Top</a>" << std::endl
                << std::endl;
        }
    }

    // Output all data channels

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {

        if (php) {
            out << "if ($id == " << Graph->getDataChannel(i)->getName() << ") { ?>" << std::endl;
        }

        out << "<hr /><h2><a name='" << Graph->getDataChannel(i)->getName() << "'></a>"
            << Graph->getDataChannel(i)->getName() << "</h2>" << std::endl;

        out << "<table align='center' border='1' cellpadding='5'>" << std::endl;

        out << "<tr><th>Buffer Size</th><th>Number of Initial Tokens</th><th>Token Size</th></tr>";

        if (Graph->getDataChannel(i)->getBufferSize() == SADF_UNBOUNDED) {
            out << "<tr><td>Unbounded</td>";
        } else {
            out << "<tr><td>" << Graph->getDataChannel(i)->getBufferSize() << "</td>";
        }

        out << "<td>" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << "</td><td>"
            << Graph->getDataChannel(i)->getTokenSize() << "</td></tr>" << std::endl;

        out << "</table>" << std::endl;

        if (php) {
            out << "<?php }" << std::endl << std::endl;
        } else {
            out << "<a href='#" << Graph->getName() << "'>Back to Top</a>" << std::endl
                << std::endl;
        }
    }

    // Output all control channels

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {

        if (php) {
            out << "if ($id == " << Graph->getControlChannel(i)->getName() << ") { ?>" << std::endl;
        }

        out << "<hr /><h2><a name='" << Graph->getControlChannel(i)->getName() << "'></a>"
            << Graph->getControlChannel(i)->getName() << "</h2>" << std::endl;

        out << "<table align='center' border='1' cellpadding='5'>" << std::endl;

        out << "<tr><th>Buffer Size</th><th>Initial Tokens</th><th>Token Size</th></tr>";

        if (Graph->getControlChannel(i)->getBufferSize() == SADF_UNBOUNDED) {
            out << "<tr><td>Unbounded</td>";
        } else {
            out << "<tr><td>" << Graph->getControlChannel(i)->getBufferSize() << "</td>";
        }

        out << "<td>";

        if (Graph->getControlChannel(i)->getNumberOfInitialTokens() != 0) {

            CQueue NumbersQueue(Graph->getControlChannel(i)->getNumbersQueue());
            CQueue ContentQueue(Graph->getControlChannel(i)->getContentQueue());

            while (!NumbersQueue.empty()) {

                CId ID = ContentQueue.front();
                CString TokenValue;

                if (Graph->getControlChannel(i)
                            ->getDestination()
                            ->hasMultipleControlInputChannels()) {

                    bool ScenarioNotFound = true;

                    CId ChannelID =
                            Graph->getControlChannel(i)->getDestination()->getControlInputChannelID(
                                    Graph->getControlChannel(i)->getName());

                    for (CId j = 0; ScenarioNotFound
                                    && j
                                               != Graph->getControlChannel(i)
                                                          ->getDestination()
                                                          ->getNumberOfScenarios();
                         j++) {
                        if (Graph->getControlChannel(i)->getDestination()->getControlID(j,
                                                                                        ChannelID)
                            == ID) {
                            ScenarioNotFound = false;
                            TokenValue =
                                    Graph->getControlChannel(i)->getDestination()->getControlValue(
                                            j, ChannelID);
                        }
                    }

                } else {
                    TokenValue = Graph->getControlChannel(i)->getDestination()->getScenarioName(ID);
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

        } else {
            out << "-";
        }

        out << "</td><td>" << Graph->getControlChannel(i)->getTokenSize() << "</td></tr>"
            << std::endl;

        out << "</table>" << std::endl;

        if (php) {
            out << "<?php }" << std::endl << std::endl;
        } else {
            out << "<a href='#" << Graph->getName() << "'>Back to Top</a>" << std::endl
                << std::endl;
        }
    }

    if (php) {
        out << "?>" << std::endl;
    }

    out << "</body>" << std::endl;
    out << "</html>" << std::endl;
}

// NOLINTEND(concurrency-mt-unsafe,cert-env33-c)
