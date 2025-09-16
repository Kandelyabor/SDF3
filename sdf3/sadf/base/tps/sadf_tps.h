/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_tps.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF - Timed Probabilistic System (TPS)
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#ifndef SADF_TPS_H_INCLUDED
#define SADF_TPS_H_INCLUDED

// Include type definitions

#include "sadf_configuration.h"
#include "sadf_detector_status.h"
#include "sadf_kernel_status.h"

// SADF_TPS Definition

class SADF_TPS {

public:
    // Constructor

    explicit SADF_TPS(SADF_Graph *Graph);

    // Destructor
    ~SADF_TPS();

    SADF_TPS(const SADF_TPS&) = delete;
    SADF_TPS& operator=(const SADF_TPS&) = delete;
    SADF_TPS(SADF_TPS&&) = delete;
    SADF_TPS& operator=(SADF_TPS&&) = delete;


    // Access to instance variables

    [[nodiscard]] CSize getNumberOfConfigurations() const { return NumberOfConfigurations; };
    void addConfiguration(const std::shared_ptr<SADF_Configuration>& C);
    [[nodiscard]] std::shared_ptr<SADF_Configuration> getInitialConfiguration() const {
        return InitialConfiguration;
    };
    std::list<SADF_ListOfConfigurations> &getConfigurationSpace() { return ConfigurationSpace; };
    SADF_Configuration *inConfigurationSpace(SADF_Configuration *C);
    void deleteContentOfConfigurations();

    [[nodiscard]] SADF_KernelState *getInitialKernelState(CId KernelID) const {
        return InitialKernelStates[KernelID];
    };
    std::list<std::shared_ptr<SADF_KernelState>> &getKernelStates(CId KernelID) { return KernelStates[KernelID]; };
    [[nodiscard]] SADF_DetectorState *getInitialDetectorState(CId DetectorID) const {
        return InitialDetectorStates[DetectorID];
    };
    std::list<std::shared_ptr<SADF_DetectorState>> &getDetectorStates(CId DetectorID) {
        return DetectorStates[DetectorID];
    };

    // Functions for analysis

    void
    removeTransientConfigurations(); // Precondition: all configurations in configuration space must
                                     // not be marked and relevant, while transitions exist
    bool isSingleStronglyConnectedComponent(); // Precondition: all configurations in configuration
                                               // space must not be marked
    std::vector<CDouble> computeEquilibriumDistribution();

private:
    // Instance Variables

    CSize NumberOfConfigurations;

    std::vector<std::list<std::shared_ptr<SADF_KernelState>>> KernelStates;
    std::vector<std::list<std::shared_ptr<SADF_DetectorState>>> DetectorStates;
    std::vector<SADF_KernelState *> InitialKernelStates;
    std::vector<SADF_DetectorState *> InitialDetectorStates;

    std::shared_ptr<SADF_Configuration> InitialConfiguration;
    SADF_HashedListOfConfigurations ConfigurationSpace;
};

#endif
