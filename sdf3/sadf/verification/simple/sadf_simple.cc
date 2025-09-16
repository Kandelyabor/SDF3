/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_basic.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Basic Properties of SADF Graphs
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

#include "sadf_simple.h"

// Functions to determine whether SADF graph consists of single component

namespace {

void SADF_VisitConnectedProcesses(SADF_Process *Process,
                                  std::vector<CId> &KernelColors,
                                  std::vector<CId> &DetectorColors) {

    if (Process->getType() == SADF_KERNEL) {
        KernelColors[Process->getIdentity()] = 1;
    } else {
        DetectorColors[Process->getIdentity()] = 1;
    }

    SADF_Channel *Channel = nullptr;

    for (const auto &i : Process->getDataInputChannels()) {

        Channel = i;

        if (((Channel->getSource()->getType() == SADF_KERNEL)
             && (KernelColors[Channel->getSource()->getIdentity()] == 0))
            || ((Channel->getSource()->getType() == SADF_DETECTOR)
                && (DetectorColors[Channel->getSource()->getIdentity()] == 0))) {
            SADF_VisitConnectedProcesses(Channel->getSource(), KernelColors, DetectorColors);
        }
    }

    for (const auto &i : Process->getDataOutputChannels()) {

        Channel = i;

        if (((Channel->getDestination()->getType() == SADF_KERNEL)
             && (KernelColors[Channel->getDestination()->getIdentity()] == 0))
            || ((Channel->getDestination()->getType() == SADF_DETECTOR)
                && (DetectorColors[Channel->getDestination()->getIdentity()] == 0))) {
            SADF_VisitConnectedProcesses(Channel->getDestination(), KernelColors, DetectorColors);
        }
    }

    for (const auto &i : Process->getControlInputChannels()) {

        Channel = i;

        if (((Channel->getSource()->getType() == SADF_KERNEL)
             && (KernelColors[Channel->getSource()->getIdentity()] == 0))
            || ((Channel->getSource()->getType() == SADF_DETECTOR)
                && (DetectorColors[Channel->getSource()->getIdentity()] == 0))) {
            SADF_VisitConnectedProcesses(Channel->getSource(), KernelColors, DetectorColors);
        }
    }

    for (const auto &i : Process->getControlOutputChannels()) {

        Channel = i;

        if (((Channel->getDestination()->getType() == SADF_KERNEL)
             && (KernelColors[Channel->getDestination()->getIdentity()] == 0))
            || ((Channel->getDestination()->getType() == SADF_DETECTOR)
                && (DetectorColors[Channel->getDestination()->getIdentity()] == 0))) {
            SADF_VisitConnectedProcesses(Channel->getDestination(), KernelColors, DetectorColors);
        }
    }
}

} // namespace

bool SADF_Verify_SingleComponent(SADF_Graph *Graph) {

    std::vector<CId> KernelColors(Graph->getNumberOfKernels(), 0);
    std::vector<CId> DetectorColors(Graph->getNumberOfDetectors(), 0);

    SADF_VisitConnectedProcesses(Graph->getKernel(0), KernelColors, DetectorColors);

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
        if (KernelColors[i] == 0) {
            return false;
        }
    }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
        if (DetectorColors[i] == 0) {
            return false;
        }
    }

    return true;
}

// Functions to determine whether SADF graph is timed

bool SADF_Verify_Timed(SADF_Graph *Graph) {

    bool Timed = false;

    for (CId i = 0; !Timed && i != Graph->getNumberOfKernels(); i++) {
        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
            for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++) {
                if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() != 0) {
                    Timed = true;
                }
            }
        }
    }

    for (CId i = 0; !Timed && i != Graph->getNumberOfDetectors(); i++) {
        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
            for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++) {
                if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() != 0) {
                    Timed = true;
                }
            }
        }
    }

    return Timed;
}
