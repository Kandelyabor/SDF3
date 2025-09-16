/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   type.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF type definitions
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: type.cc,v 1.1.2.3 2010-04-25 01:21:17 mgeilen Exp $
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

#include "type.h"
#include <string>

namespace FSMSADF {
StorageDistribution::StorageDistribution(const std::map<CString, Size> &storageMap) {
    for (const auto& [name, sz] : storageMap) {
        // Insert storage space of channel to map
        insert(std::pair<CString, Size>(name, sz));
    }
}
/**
 * constructFromXML()
 * Create a storage distribution from the XML data.
 */
void StorageDistribution::constructFromXML(const CNodePtr storageDistributionNode) {
    // Throughput
    if (!CHasAttribute(storageDistributionNode, "thr")) {
        throw CException("Storage distribution has no thr");
    }
    setThroughput(static_cast<CDouble>(CGetAttribute(storageDistributionNode, "thr")));

    // Channels
    for (CNode *n = CGetChildNode(storageDistributionNode, "channel"); n != nullptr;
         n = CNextNode(n, "channel")) {
        CString name;
        CSize sz = 0;

        // Name
        if (!CHasAttribute(n, "name")) {
            throw CException("Channel in storage distribution has no name.");
        }
        name = CGetAttribute(n, "name");

        // Size
        if (!CHasAttribute(n, "sz")) {
            throw CException("Channel in storage distribution has no sz.");
        }
        sz = std::stoi(CGetAttribute(n, "sz"));

        // Insert storage space of channel to map
        insert(std::pair<CString, Size>(name, static_cast<Size>(sz)));
    }
}

/**
 * convertToXML()
 * Convert the storage distribution to XML data.
 */
void StorageDistribution::convertToXML(const CNodePtr storageDistributionNode) const {
    // Size
    CAddAttribute(storageDistributionNode, "sz", CString(getSize()));

    // Throughput
    CAddAttribute(storageDistributionNode, "thr", CString(throughput));

    // Channels
    for (const auto& [name, sz]: *this) {
        // Channel
        CNode *channelNode = CAddNode(storageDistributionNode, "channel");

        // Name
        CAddAttribute(channelNode, "name", name);

        // Size
        CAddAttribute(channelNode, "sz", CString(sz));
    }
}

/**
 * constructFromXML()
 * Create a storage distribution set from the XML data.
 */
void StorageDistributions::constructFromXML(const CNodePtr storageDistributionsNode) {
    for (CNode *n = CGetChildNode(storageDistributionsNode, "storageDistribution"); n != nullptr;
         n = CNextNode(n, "storageDistribution")) {
        StorageDistribution s;

        // Create a storage distribution
        s.constructFromXML(n);

        // Insert storage distribution in the set
        insert(s);
    }
}

/**
 * convertToXML()
 * Convert the storage distribution set to XML data.
 */
void StorageDistributions::convertToXML(const CNodePtr storageDistributionsNode) {
    // Storage distributions
    for (const auto& s: *this) {
        s.convertToXML(CAddNode(storageDistributionsNode, "storageDistribution"));
    }
}

} // End namespace FSMSADF
