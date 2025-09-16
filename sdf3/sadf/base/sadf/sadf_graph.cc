/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_graph.cc
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

#include "sadf_graph.h"

// Constructor

SADF_Graph::SADF_Graph(const CString &N, const CId ID) {
    this->setName(N);
    this->setIdentity(ID);
}

// Destructor

SADF_Graph::~SADF_Graph() = default;

// Access to instance variables

SADF_Process *SADF_Graph::getKernel(const CString &KernelName) const {

    for (const auto &Kernel : Kernels) {
        if (Kernel->getName() == KernelName) {
            return Kernel.get();
        }
    }

    return nullptr;
}

SADF_Process *SADF_Graph::getDetector(const CString &DetectorName) const {

    for (const auto &Detector : Detectors) {
        if (Detector->getName() == DetectorName) {
            return Detector.get();
        }
    }

    return nullptr;
}

SADF_Channel *SADF_Graph::getChannel(const CString &ChannelName) const {

    for (const auto &DataChannel : DataChannels) {
        if (DataChannel->getName() == ChannelName) {
            return DataChannel.get();
        }
    }

    for (const auto &ControlChannel : ControlChannels) {
        if (ControlChannel->getName() == ChannelName) {
            return ControlChannel.get();
        }
    }

    return nullptr;
}
