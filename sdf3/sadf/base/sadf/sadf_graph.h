/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_graph.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Graph
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

#ifndef SADF_GRAPH_H_INCLUDED
#define SADF_GRAPH_H_INCLUDED

// Include type definitions

#include "sadf_channel.h"
#include "sadf_process.h"


// SADF_Graph Definition

class SADF_Graph : public SADF_Component {

public:
    // Constructors

    SADF_Graph(const CString &N, CId ID);

    // Destructor

    virtual ~SADF_Graph();

    SADF_Graph(const SADF_Graph&) = delete;
    SADF_Graph& operator=(const SADF_Graph&) = delete;
    SADF_Graph(SADF_Graph&&) = delete;
    SADF_Graph& operator=(SADF_Graph&&) = delete;

    // Access to instance variables

    void addKernel(std::unique_ptr<SADF_Process> K) { Kernels.push_back(std::move(K)); };
    void addDetector(std::unique_ptr<SADF_Process> D) { Detectors.push_back(std::move(D)); };
    void addDataChannel(std::unique_ptr<SADF_Channel> C) { DataChannels.push_back(std::move(C)); };
    void addControlChannel(std::unique_ptr<SADF_Channel> C) { ControlChannels.push_back(std::move(C)); };

    [[nodiscard]] SADF_Process *getKernel(const CId ID) const { return Kernels[ID].get(); };
    [[nodiscard]] SADF_Process *getDetector(const CId ID) const { return Detectors[ID].get(); };
    [[nodiscard]] SADF_Channel *getDataChannel(const CId ID) const { return DataChannels[ID].get(); };
    [[nodiscard]] SADF_Channel *getControlChannel(const CId ID) const { return ControlChannels[ID].get(); };

    [[nodiscard]] SADF_Process *getKernel(const CString &KernelName) const;
    [[nodiscard]] SADF_Process *getDetector(const CString &DetectorName) const;
    [[nodiscard]] SADF_Channel *getChannel(const CString &ChannelName) const;

    [[nodiscard]] CId getNumberOfKernels() const { return static_cast<CId>(Kernels.size()); };
    [[nodiscard]] CId getNumberOfDetectors() const { return static_cast<CId>(Detectors.size()); };
    [[nodiscard]] CId getNumberOfDataChannels() const {
        return static_cast<CId>(DataChannels.size());
    };
    [[nodiscard]] CId getNumberOfControlChannels() const {
        return static_cast<CId>(ControlChannels.size());
    };

private:
    // Instance variables

    std::vector<std::unique_ptr<SADF_Process>> Kernels;
    std::vector<std::unique_ptr<SADF_Process>> Detectors;
    std::vector<std::unique_ptr<SADF_Channel>> DataChannels;
    std::vector<std::unique_ptr<SADF_Channel>> ControlChannels;
};

#endif
