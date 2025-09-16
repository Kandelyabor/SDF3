/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_process.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Process
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#include "sadf_process.h"
#include "sadf/base/sadf/sadf_channel.h"
#include <algorithm>

// Constructor

SADF_Process::SADF_Process(const CString &N, const CId ID, const CId T) {

    this->setName(N);
    this->setIdentity(ID);
    this->setType(T);
}

// Destructor

SADF_Process::~SADF_Process() = default;

// Access to Channels

CId SADF_Process::getDataInputChannelID(const CString &ChannelName) {

    for (CId i = 0; i != DataInputChannels.size(); i++) {
        if (DataInputChannels[i]->getName() == ChannelName) {
            return i;
        }
    }

    return SADF_UNDEFINED;
}

CId SADF_Process::getDataOutputChannelID(const CString &ChannelName) {

    for (CId i = 0; i != DataOutputChannels.size(); i++) {
        if (DataOutputChannels[i]->getName() == ChannelName) {
            return i;
        }
    }

    return SADF_UNDEFINED;
}

CId SADF_Process::getControlInputChannelID(const CString &ChannelName) {

    for (CId i = 0; i != ControlInputChannels.size(); i++) {
        if (ControlInputChannels[i]->getName() == ChannelName) {
            return i;
        }
    }

    return SADF_UNDEFINED;
}

CId SADF_Process::getControlOutputChannelID(const CString &ChannelName) {

    for (CId i = 0; i != ControlOutputChannels.size(); i++) {
        if (ControlOutputChannels[i]->getName() == ChannelName) {
            return i;
        }
    }

    return SADF_UNDEFINED;
}

// Access to Profiles

void SADF_Process::deleteProfiles(const CId ScenarioID) {
    Profiles[ScenarioID].resize(0);
}

// Access to Scenarios and SubScenarios

CId SADF_Process::addScenario(const CString &ScenarioName) {

    Scenarios.push_back(ScenarioName);

    if (this->getType() == SADF_KERNEL) {

        ActiveScenario.push_back(true);

        for (auto &DataInputChannel : DataInputChannels) {
            DataInputChannel->addConsumption();
        }

        for (auto &DataOutputChannel : DataOutputChannels) {
            DataOutputChannel->addProduction();
        }

        for (auto &ControlInputChannel : ControlInputChannels) {
            ControlInputChannel->addConsumption();
        }

        std::vector<std::unique_ptr<SADF_Profile>> ProfileVector;
        Profiles.push_back(std::move(ProfileVector));
    }

    if (this->getType() == SADF_DETECTOR) {
        MarkovChains.resize(MarkovChains.size() + 1);
    }

    return static_cast<CId>(Scenarios.size()) - 1;
}

CId SADF_Process::addSubScenario(const CString &SubScenarioName) {

    SubScenarios.push_back(SubScenarioName);

    std::vector<std::unique_ptr<SADF_Profile>> ProfileVector;
    Profiles.push_back(std::move(ProfileVector));

    for (auto &DataInputChannel : DataInputChannels) {
        DataInputChannel->addConsumption();
    }

    for (auto &DataOutputChannel : DataOutputChannels) {
        DataOutputChannel->addProduction();
    }

    for (auto &ControlInputChannel : ControlInputChannels) {
        ControlInputChannel->addConsumption();
    }

    for (auto &ControlOutputChannel : ControlOutputChannels) {
        ControlOutputChannel->addProduction();
    }

    return static_cast<CId>(SubScenarios.size()) - 1;
}

CId SADF_Process::getScenarioID(const CString &ScenarioName) {

    for (CId i = 0; i != Scenarios.size(); i++) {
        if (Scenarios[i] == ScenarioName) {
            return i;
        }
    }

    return SADF_UNDEFINED;
}

CId SADF_Process::getSubScenarioID(const CString &SubScenarioName) {

    for (CId i = 0; i != SubScenarios.size(); i++) {
        if (SubScenarios[i] == SubScenarioName) {
            return i;
        }
    }

    return SADF_UNDEFINED;
}

// Functions

bool SADF_Process::checkActivity(const CId ScenarioID) {

    for (auto &DataInputChannel : DataInputChannels) {
        if (DataInputChannel->getConsumptionRate(ScenarioID) != 0) {
            return true;
        }
    }

    return std::any_of(DataOutputChannels.begin(), DataOutputChannels.end(), [ScenarioID](const SADF_Channel* DataOutputChannel){
        return DataOutputChannel->getProductionRate(ScenarioID) != 0;
    });
}
