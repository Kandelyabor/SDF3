/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_cluster.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (cluster class)
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

#include "sadf2poosl_cluster.h" // IWYU pragma: keep

void SADF2POOSL_Cluster_p4r(const SADF_Graph& Graph,
                            const SADF_SimulationSettings& Settings,
                            const CString &LogFileName,
                            std::ostream &out) {

    CId NumberOfMonitors = 1;

    out << "(";

    // Instantiate Kernels

    for (CId i = 0; i != Graph.getNumberOfKernels(); i++) {

        out << "Kernel" << Graph.getKernel(i)->getIdentity() << ":Kernel"
            << Graph.getKernel(i)->getIdentity() << "(";

        if (Settings.getKernelSettings(i)->getMonitor()) {
            out << "true, " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, nil, ";
        }

        if (Settings.getKernelSettings(i)->getTrace()) {
            out << "true, \"" << Graph.getKernel(i)->getName() << "\")[";
        } else {
            out << "false, \"" << Graph.getKernel(i)->getName() << "\")[";
        }

        for (const auto &j : Graph.getKernel(i)->getDataOutputChannels()) {
            out << "DI" << j->getIdentity() << "/DO" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getKernel(i)->getDataInputChannels()) {
            out << "DO" << j->getIdentity() << "/DI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getKernel(i)->getControlInputChannels()) {
            out << "CO" << j->getIdentity() << "/CI" << j->getIdentity() << ", ";
        }

        out << "Monitor/Monitor]";

        if (i < Graph.getNumberOfKernels() - 1) {
            out << " ||" << std::endl;
        }
    }

    if (Graph.getNumberOfKernels() != 0 && Graph.getNumberOfDetectors() != 0) {
        out << "||" << std::endl;
    }

    // Instantiate Detectors

    for (CId i = 0; i != Graph.getNumberOfDetectors(); i++) {

        out << "Detector" << Graph.getDetector(i)->getIdentity() << ":Detector"
            << Graph.getDetector(i)->getIdentity() << "(";

        if (Settings.getDetectorSettings(i)->getMonitor()) {
            out << "true, " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, nil, ";
        }

        if (Settings.getDetectorSettings(i)->getTrace()) {
            out << "true, \"" << Graph.getDetector(i)->getName() << "\")[";
        } else {
            out << "false, \"" << Graph.getDetector(i)->getName() << "\")[";
        }

        for (const auto &j : Graph.getDetector(i)->getDataOutputChannels()) {
            out << "DI" << j->getIdentity() << "/DO" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getDetector(i)->getControlOutputChannels()) {
            out << "CI" << j->getIdentity() << "/CO" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getDetector(i)->getDataInputChannels()) {
            out << "DO" << j->getIdentity() << "/DI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getDetector(i)->getControlInputChannels()) {
            out << "CO" << j->getIdentity() << "/CI" << j->getIdentity() << ", ";
        }

        out << "Monitor/Monitor]";

        if (i < Graph.getNumberOfDetectors() - 1) {
            out << " ||" << std::endl;
        }
    }

    if (Graph.getNumberOfDataChannels() != 0 || Graph.getNumberOfControlChannels() != 0) {
        out << " ||" << std::endl;
    }

    // Instantiate Data Channels

    for (CId i = 0; i != Graph.getNumberOfDataChannels(); i++) {

        out << "DataBuffer" << i << ":DataBuffer(";

        if (Graph.getDataChannel(i)->getBufferSize() == SADF_UNBOUNDED) {
            out << "-1, ";
        } else {
            out << Graph.getDataChannel(i)->getBufferSize() << ", ";
        }

        out << Graph.getDataChannel(i)->getNumberOfInitialTokens() << ", "
            << Graph.getDataChannel(i)->getTokenSize() << ", ";

        if (Settings.getDataChannelSettings(i)->getMonitor()) {
            out << "true, " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, nil, ";
        }

        if (Settings.getDataChannelSettings(i)->getTrace()) {
            out << "true, \"" << Graph.getDataChannel(i)->getName() << "\")[";
        } else {
            out << "false, \"" << Graph.getDataChannel(i)->getName() << "\")[";
        }

        out << "DI" << Graph.getDataChannel(i)->getIdentity() << "/In, DO"
            << Graph.getDataChannel(i)->getIdentity() << "/Out, Monitor/Monitor]";

        if (i < Graph.getNumberOfDataChannels() - 1) {
            out << " ||" << std::endl;
        }
    }

    if (Graph.getNumberOfDataChannels() != 0 && Graph.getNumberOfControlChannels() != 0) {
        out << " ||" << std::endl;
    }

    // Instantiate Control Channels

    for (CId i = 0; i != Graph.getNumberOfControlChannels(); i++) {

        out << "ControlBuffer" << i << ":ControlBuffer(";

        if (Graph.getControlChannel(i)->getBufferSize() == SADF_UNBOUNDED) {
            out << "-1, ";
        } else {
            out << Graph.getControlChannel(i)->getBufferSize() << ", ";
        }

        if (Graph.getControlChannel(i)->getNumberOfInitialTokens() == 0) {
            out << "new(Queue) init, new(Queue) init, ";
        } else {
            out << "new(Queue) init ";

            CQueue NumbersQueue(Graph.getControlChannel(i)->getNumbersQueue());

            while (!NumbersQueue.empty()) {
                out << "put(" << NumbersQueue.front() << ") ";
                NumbersQueue.pop();
            }

            out << ", new(Queue) init ";

            CQueue ContentQueue(Graph.getControlChannel(i)->getContentQueue());

            while (!ContentQueue.empty()) {
                out << "put(" << ContentQueue.front() << ") ";
                ContentQueue.pop();
            }

            out << ", ";
        }

        out << Graph.getControlChannel(i)->getTokenSize() << ", ";

        if (Settings.getControlChannelSettings(i)->getMonitor()) {
            out << "true, " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, nil, ";
        }

        if (Settings.getControlChannelSettings(i)->getTrace()) {
            out << "true, \"" << Graph.getControlChannel(i)->getName() << "\")[";
        } else {
            out << "false, \"" << Graph.getControlChannel(i)->getName() << "\")[";
        }

        out << "CI" << Graph.getControlChannel(i)->getIdentity() << "/In, CO"
            << Graph.getControlChannel(i)->getIdentity() << "/Out, Monitor/Monitor]";

        if (i < Graph.getNumberOfControlChannels() - 1) {
            out << " ||" << std::endl;
        }
    }

    // Instantiate Simulation Controller

    out << " ||" << std::endl;
    out << "SimulationController:SimulationController(" << NumberOfMonitors - 1 << ", \""
        << LogFileName << "\")[Monitor/Monitor]" << std::endl;

    out << ")\\{";

    // Hide Channels

    for (CId i = 0; i != Graph.getNumberOfDataChannels(); i++) {
        out << "DI" << Graph.getDataChannel(i)->getIdentity() << ", "
            << "DO" << Graph.getDataChannel(i)->getIdentity() << ", ";
    }

    for (CId i = 0; i != Graph.getNumberOfControlChannels(); i++) {
        out << "CI" << Graph.getControlChannel(i)->getIdentity() << ", "
            << "CO" << Graph.getControlChannel(i)->getIdentity() << ", ";
    }

    out << "Monitor}" << std::endl << std::endl;
}

void SADF2POOSL_Cluster(const SADF_Graph& Graph,
                        const SADF_SimulationSettings& Settings,
                        const CString &LogFileName,
                        std::ostream &out) {

    // Channels

    out << "channels" << std::endl;

    for (CId i = 0; i != Graph.getNumberOfDataChannels(); i++) {
        out << "DI" << Graph.getDataChannel(i)->getIdentity() << ", DO"
            << Graph.getDataChannel(i)->getIdentity() << ", ";
    }

    for (CId i = 0; i != Graph.getNumberOfControlChannels(); i++) {
        out << "CI" << Graph.getControlChannel(i)->getIdentity() << ", CO"
            << Graph.getControlChannel(i)->getIdentity() << ", ";
    }

    out << "Monitor" << std::endl << std::endl;

    CId NumberOfMonitors = 1;

    // Instantiate Kernels

    for (CId i = 0; i != Graph.getNumberOfKernels(); i++) {

        out << "instance Kernel" << Graph.getKernel(i)->getIdentity() << ":Kernel"
            << Graph.getKernel(i)->getIdentity() << "(Monitor := ";

        if (Settings.getKernelSettings(i)->getMonitor()) {
            out << "true, MonitorID := " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, MonitorID := nil, ";
        }

        if (Settings.getKernelSettings(i)->getTrace()) {
            out << "Trace := true, Name := \"" << Graph.getKernel(i)->getName() << "\")"
                << std::endl;
        } else {
            out << "Trace := false, Name := \"" << Graph.getKernel(i)->getName() << "\")"
                << std::endl;
        }

        out << "connections" << std::endl;

        for (const auto &j : Graph.getKernel(i)->getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << " -> "
                << "DI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getKernel(i)->getDataInputChannels()) {
            out << "DI" << j->getIdentity() << " -> "
                << "DO" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getKernel(i)->getControlInputChannels()) {
            out << "CI" << j->getIdentity() << " -> "
                << "CO" << j->getIdentity() << ", ";
        }

        out << "Monitor -> Monitor" << std::endl;

        out << std::endl;
    }

    // Instantiate Detectors

    for (CId i = 0; i != Graph.getNumberOfDetectors(); i++) {

        out << "instance Detector" << Graph.getDetector(i)->getIdentity() << ":Detector"
            << Graph.getDetector(i)->getIdentity() << "(Monitor := ";

        if (Settings.getDetectorSettings(i)->getMonitor()) {
            out << "true, MonitorID := " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, MonitorID := nil, ";
        }

        if (Settings.getDetectorSettings(i)->getTrace()) {
            out << "Trace := true, Name := \"" << Graph.getDetector(i)->getName() << "\")"
                << std::endl;
        } else {
            out << "Trace := false, Name := \"" << Graph.getDetector(i)->getName() << "\")"
                << std::endl;
        }

        out << "connections" << std::endl;

        for (const auto &j : Graph.getDetector(i)->getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << " -> "
                << "DI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getDetector(i)->getControlOutputChannels()) {
            out << "CO" << j->getIdentity() << " -> "
                << "CI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getDetector(i)->getDataInputChannels()) {
            out << "DI" << j->getIdentity() << " -> "
                << "DO" << j->getIdentity() << ", ";
        }

        for (const auto &j : Graph.getDetector(i)->getControlInputChannels()) {
            out << "CI" << j->getIdentity() << " -> "
                << "CO" << j->getIdentity() << ", ";
        }

        out << "Monitor -> Monitor" << std::endl << std::endl;
    }

    // Instantiate Data Channels

    for (CId i = 0; i != Graph.getNumberOfDataChannels(); i++) {

        out << "instance DataBuffer" << i << ":DataBuffer(BufferSize := ";

        if (Graph.getDataChannel(i)->getBufferSize() == SADF_UNBOUNDED) {
            out << "-1, InitialTokens := ";
        } else {
            out << Graph.getDataChannel(i)->getBufferSize() << ", InitialTokens := ";
        }

        out << Graph.getDataChannel(i)->getNumberOfInitialTokens()
            << ", TokenSize := " << Graph.getDataChannel(i)->getTokenSize() << ", Monitor := ";

        if (Settings.getDataChannelSettings(i)->getMonitor()) {
            out << "true, MonitorID := " << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "false, MonitorID := nil, ";
        }

        if (Settings.getDataChannelSettings(i)->getTrace()) {
            out << "Trace := true, Name := \"" << Graph.getDataChannel(i)->getName() << "\")"
                << std::endl;
        } else {
            out << "Trace := false, Name := \"" << Graph.getDataChannel(i)->getName() << "\")"
                << std::endl;
        }

        out << "connections" << std::endl;
        out << "Monitor -> Monitor, In -> DI" << Graph.getDataChannel(i)->getIdentity()
            << ", Out -> DO" << Graph.getDataChannel(i)->getIdentity() << std::endl
            << std::endl;
    }

    // Instantiate Control Channels

    for (CId i = 0; i != Graph.getNumberOfControlChannels(); i++) {

        out << "instance ControlBuffer" << i << ":ControlBuffer(BufferSize := ";

        if (Graph.getControlChannel(i)->getBufferSize() == SADF_UNBOUNDED) {
            out << "-1, NumbersInitialTokens := ";
        } else {
            out << Graph.getControlChannel(i)->getBufferSize() << ", NumbersInitialTokens := ";
        }

        if (Graph.getControlChannel(i)->getNumberOfInitialTokens() == 0) {
            out << "new(Queue) init, ContentInitialTokens := new(Queue) init, TokenSize := ";
        } else {
            out << "new(Queue) init ";

            CQueue NumbersQueue(Graph.getControlChannel(i)->getNumbersQueue());

            while (!NumbersQueue.empty()) {
                out << "put(" << NumbersQueue.front() << ") ";
                NumbersQueue.pop();
            }

            out << ", ContentInitialTokens := new(Queue) init ";

            CQueue ContentQueue(Graph.getControlChannel(i)->getContentQueue());

            while (!ContentQueue.empty()) {
                out << "put(" << ContentQueue.front() << ") ";
                ContentQueue.pop();
            }

            out << ", TokenSize := ";
        }

        out << Graph.getControlChannel(i)->getTokenSize() << ", ";

        if (Settings.getControlChannelSettings(i)->getMonitor()) {
            out << "Monitor := true, MonitorID" << NumberOfMonitors << ", ";
            NumberOfMonitors++;
        } else {
            out << "Monitor := false, MonitorID := nil, ";
        }

        if (Settings.getControlChannelSettings(i)->getTrace()) {
            out << "Trace := true, Name := \"" << Graph.getControlChannel(i)->getName() << "\")"
                << std::endl;
        } else {
            out << "Trace := false, Name := \"" << Graph.getControlChannel(i)->getName() << "\")"
                << std::endl;
        }

        out << "connections" << std::endl;
        out << "Monitor -> Monitor,  In -> CI" << Graph.getControlChannel(i)->getIdentity()
            << ", Out -> CO" << Graph.getControlChannel(i)->getIdentity() << std::endl
            << std::endl;
    }

    // Instantiate Simulation Controller

    out << "instance SimulationController:SimulationController(NumberOfMonitors := "
        << NumberOfMonitors - 1 << ", LogFile := \"" << LogFileName << "\")" << std::endl;
    out << "connections" << std::endl;
    out << "Monitor -> Monitor" << std::endl << std::endl;
}
