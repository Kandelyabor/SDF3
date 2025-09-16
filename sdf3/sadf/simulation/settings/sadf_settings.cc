/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_settings.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Simulation Settings
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

#include "sadf_settings.h"

// Constructors

SADF_SimulationSetting::SADF_SimulationSetting(bool M, bool T) : Monitor(M), Trace(T) {}

SADF_SimulationSettings::SADF_SimulationSettings(SADF_Graph *Graph) :
    MaximumModelTime(SADF_UNDEFINED), MaximumSimulationTime(SADF_UNDEFINED) {

    KernelSettings.resize(Graph->getNumberOfKernels());
    DetectorSettings.resize(Graph->getNumberOfDetectors());
    DataChannelSettings.resize(Graph->getNumberOfDataChannels());
    ControlChannelSettings.resize(Graph->getNumberOfControlChannels());

    for (auto &KernelSetting : KernelSettings) {
        KernelSetting = std::make_unique<SADF_SimulationSetting>(true, false);
    }

    for (auto &DetectorSetting : DetectorSettings) {
        DetectorSetting = std::make_unique<SADF_SimulationSetting>(true, false);
    }

    for (auto &DataChannelSetting : DataChannelSettings) {
        DataChannelSetting = std::make_unique<SADF_SimulationSetting>(false, false);
    }

    for (auto &ControlChannelSetting : ControlChannelSettings) {
        ControlChannelSetting = std::make_unique<SADF_SimulationSetting>(false, false);
    }
}

// Destructor

SADF_SimulationSettings::~SADF_SimulationSettings() = default;

// Function to parse the XML file with simulation settings

void SADF_ParseSimulationSettings(SADF_Graph *Graph,
                                  SADF_SimulationSettings &Settings,
                                  const CNodePtr SettingsNode) {

    for (CNodePtr KernelNode = CGetChildNode(SettingsNode, "kernel"); KernelNode != nullptr;
         KernelNode = CNextNode(KernelNode, "kernel")) {

        if (!CHasAttribute(KernelNode, "name")) {
            throw CException("Error: Settings for kernel without name specified.");
        }

        if (Graph->getKernel(CGetAttribute(KernelNode, "name").trim()) == nullptr) {
            throw CException(CString("Error: Settings specified for non-existing kernel '")
                             + CGetAttribute(KernelNode, "name").trim() + "'.");
        }

        if (CHasAttribute(KernelNode, "monitor")) {
            if (CGetAttribute(KernelNode, "monitor").trim() == "off") {
                Settings.getKernelSettings(
                                Graph->getKernel(CGetAttribute(KernelNode, "name").trim())
                                        ->getIdentity())
                        ->resetMonitor();
            }
        }

        if (CHasAttribute(KernelNode, "trace")) {
            if (CGetAttribute(KernelNode, "trace").trim() == "on") {
                Settings.getKernelSettings(
                                Graph->getKernel(CGetAttribute(KernelNode, "name").trim())
                                        ->getIdentity())
                        ->setTrace();
            }
        }
    }

    for (CNodePtr DetectorNode = CGetChildNode(SettingsNode, "detector"); DetectorNode != nullptr;
         DetectorNode = CNextNode(DetectorNode, "detector")) {

        if (!CHasAttribute(DetectorNode, "name")) {
            throw CException("Error: Settings for detector without name specified.");
        }

        if (Graph->getDetector(CGetAttribute(DetectorNode, "name").trim()) == nullptr) {
            throw CException(CString("Error: Settings specified for non-existing detector '")
                             + CGetAttribute(DetectorNode, "name").trim() + "'.");
        }

        if (CHasAttribute(DetectorNode, "monitor")) {
            if (CGetAttribute(DetectorNode, "monitor").trim() == "off") {
                Settings.getDetectorSettings(
                                Graph->getDetector(CGetAttribute(DetectorNode, "name").trim())
                                        ->getIdentity())
                        ->resetMonitor();
            }
        }

        if (CHasAttribute(DetectorNode, "trace")) {
            if (CGetAttribute(DetectorNode, "trace").trim() == "on") {
                Settings.getDetectorSettings(
                                Graph->getDetector(CGetAttribute(DetectorNode, "name").trim())
                                        ->getIdentity())
                        ->setTrace();
            }
        }
    }

    for (CNodePtr ChannelNode = CGetChildNode(SettingsNode, "channel"); ChannelNode != nullptr;
         ChannelNode = CNextNode(ChannelNode, "channel")) {

        if (!CHasAttribute(ChannelNode, "name")) {
            throw CException("Error: Settings for channel without name specified.");
        }

        CId ChannelType = 0;

        if (Graph->getDataChannel(static_cast<CId>(CGetAttribute(ChannelNode, "name").trim()))
            != nullptr) {
            ChannelType = SADF_DATA_CHANNEL;
        } else if (Graph->getControlChannel(
                           static_cast<CId>(CGetAttribute(ChannelNode, "name").trim()))
                   != nullptr) {
            ChannelType = SADF_CONTROL_CHANNEL;
        } else {
            throw CException(CString("Error: Settings specified for non-existing channel '")
                             + CGetAttribute(ChannelNode, "name").trim() + "'.");
        }

        if (CHasAttribute(ChannelNode, "monitor")) {
            if (CGetAttribute(ChannelNode, "monitor").trim() == "off") {
                if (ChannelType == SADF_DATA_CHANNEL) {
                    Settings.getDataChannelSettings(
                                    Graph
                                            ->getDataChannel(static_cast<CId>(
                                                    CGetAttribute(ChannelNode, "name").trim()))
                                            ->getIdentity())
                            ->resetMonitor();
                } else {
                    Settings.getControlChannelSettings(
                                    Graph
                                            ->getControlChannel(static_cast<CId>(
                                                    CGetAttribute(ChannelNode, "name").trim()))
                                            ->getIdentity())
                            ->resetMonitor();
                }
            }
        }

        if (CHasAttribute(ChannelNode, "trace")) {
            if (CGetAttribute(ChannelNode, "trace").trim() == "on") {
                if (ChannelType == SADF_DATA_CHANNEL) {
                    Settings.getDataChannelSettings(
                                    Graph
                                            ->getDataChannel(static_cast<CId>(
                                                    CGetAttribute(ChannelNode, "name").trim()))
                                            ->getIdentity())
                            ->setTrace();
                } else {
                    Settings.getControlChannelSettings(
                                    Graph
                                            ->getControlChannel(static_cast<CId>(
                                                    CGetAttribute(ChannelNode, "name").trim()))
                                            ->getIdentity())
                            ->setTrace();
                }
            }
        }
    }

    if (CHasChildNode(SettingsNode, "model_time")) {
        if (CHasAttribute(CGetChildNode(SettingsNode, "model_time"), "maximum")) {
            if (static_cast<CDouble>(
                        CGetAttribute(CGetChildNode(SettingsNode, "model_time"), "maximum"))
                < 0) {
                throw CException("Error: Negative maximum model time specified.");
            }
            Settings.setMaximumModelTime(static_cast<CDouble>(
                    CGetAttribute(CGetChildNode(SettingsNode, "model_time"), "maximum")));
        }
    }

    if (CHasChildNode(SettingsNode, "simulation_time")) {
        if (CHasAttribute(CGetChildNode(SettingsNode, "simulation_time"), "maximum")) {
            if (static_cast<CDouble>(
                        CGetAttribute(CGetChildNode(SettingsNode, "simulation_time"), "maximum"))
                < 0) {
                throw CException("Error: Negative maximum simulation time specified.");
            }
            Settings.setMaximumSimulationTime(static_cast<CDouble>(
                    CGetAttribute(CGetChildNode(SettingsNode, "simulation_time"), "maximum")));
        }
    }
}
