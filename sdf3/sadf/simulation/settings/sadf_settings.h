/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_settings.h
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

#ifndef SADF_SETTINGS_H_INCLUDED
#define SADF_SETTINGS_H_INCLUDED

#include "sadf/base/sadf/sadf_graph.h"

class SADF_SimulationSetting {

public:
    // Constructor

    SADF_SimulationSetting(bool M, bool T);

    // Destructor

    ~SADF_SimulationSetting() = default;

    SADF_SimulationSetting(const SADF_SimulationSetting&) = delete;
    SADF_SimulationSetting& operator=(const SADF_SimulationSetting&) = delete;
    SADF_SimulationSetting(SADF_SimulationSetting&&) = delete;
    SADF_SimulationSetting& operator=(SADF_SimulationSetting&&) = delete;


    // Access to instance variables

    void resetMonitor() { Monitor = false; };
    void setTrace() { Trace = true; };

    [[nodiscard]] bool getTrace() const { return Trace; };
    [[nodiscard]] bool getMonitor() const { return Monitor; };

private:
    // Instance variables

    bool Monitor;
    bool Trace;
};

class SADF_SimulationSettings {

public:
    // Constructor

    explicit SADF_SimulationSettings(SADF_Graph *Graph);

    // Destructor

    ~SADF_SimulationSettings();

    SADF_SimulationSettings(const SADF_SimulationSettings&) = delete;
    SADF_SimulationSettings& operator=(const SADF_SimulationSettings&) = delete;
    SADF_SimulationSettings(SADF_SimulationSettings&&) = delete;
    SADF_SimulationSettings& operator=(SADF_SimulationSettings&&) = delete;


    // Access to instance variables

    void setKernelTrace(CId KernelID) { KernelSettings[KernelID]->setTrace(); };
    void setDetectorTrace(CId DetectorID) { DetectorSettings[DetectorID]->setTrace(); };
    void setDataChannelTrace(CId ChannelID) { DataChannelSettings[ChannelID]->setTrace(); };
    void setControlChannelTrace(CId ChannelID) { ControlChannelSettings[ChannelID]->setTrace(); };

    void resetKernelMonitor(CId KernelID) { KernelSettings[KernelID]->resetMonitor(); };
    void resetDetectorMonitor(CId DetectorID) { DetectorSettings[DetectorID]->resetMonitor(); };
    void resetDataChannelMonitor(CId ChannelID) { DataChannelSettings[ChannelID]->resetMonitor(); };
    void resetControlChannelMonitor(CId ChannelID) {
        ControlChannelSettings[ChannelID]->resetMonitor();
    };

    [[nodiscard]] SADF_SimulationSetting *getKernelSettings(CId KernelID) const { return KernelSettings[KernelID].get(); };
    [[nodiscard]] SADF_SimulationSetting *getDetectorSettings(CId DetectorID) const {
        return DetectorSettings[DetectorID].get();
    };
    [[nodiscard]] SADF_SimulationSetting *getDataChannelSettings(CId ChannelID) const {
        return DataChannelSettings[ChannelID].get();
    };
    [[nodiscard]] SADF_SimulationSetting *getControlChannelSettings(CId ChannelID) const {
        return ControlChannelSettings[ChannelID].get();
    };

    void setMaximumModelTime(CDouble T) { MaximumModelTime = T; };
    void setMaximumSimulationTime(CDouble T) { MaximumSimulationTime = T; };

    [[nodiscard]] CDouble getMaximumModelTime() const { return MaximumModelTime; };
    [[nodiscard]] CDouble getMaximumSimulationTime() const { return MaximumSimulationTime; };

private:
    // Instance variables

    std::vector<std::unique_ptr<SADF_SimulationSetting>> KernelSettings;
    std::vector<std::unique_ptr<SADF_SimulationSetting>> DetectorSettings;
    std::vector<std::unique_ptr<SADF_SimulationSetting>> DataChannelSettings;
    std::vector<std::unique_ptr<SADF_SimulationSetting>> ControlChannelSettings;

    CDouble MaximumModelTime;
    CDouble MaximumSimulationTime;
};

// SADF_ParseSimulationSettings parses XML file of simulation settings

void SADF_ParseSimulationSettings(SADF_Graph *Graph,
                                  SADF_SimulationSettings &Settings,
                                  CNodePtr SettingsNode);

#endif
