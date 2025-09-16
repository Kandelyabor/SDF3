/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF channel
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: channel.h,v 1.1.2.4 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_CHANNEL_H_INCLUDED
#define FSMSADF_BASE_CHANNEL_H_INCLUDED

#include "actor.h"
#include <memory>

namespace FSMSADF
{

    // Forward class definition
    class Channel;
    class ScenarioGraph;

    // List of channel pointers
    using Channels = std::list<std::unique_ptr<Channel>>;
    using ChannelRef = Channel*;
    using ChannelRefs = std::list<ChannelRef>;

    /**
     * class Channel
     * Channel in an FSM-based SADF graph.
     */
    class Channel : public GraphComponent
    {
        public:

            // Constructor
            explicit Channel(const GraphComponent& c);

            // Copy constructor
            Channel(const Channel& other) = delete;

            // Copy assignment operator
            Channel& operator=(const Channel& other) = delete;

            // Move constructor
            Channel(Channel&& other) noexcept = delete;

            // Move assignment operator
            Channel& operator=(Channel&& other) noexcept = delete;

            // Destructor
            ~Channel() override;

            // Construct
            [[nodiscard]] static std::unique_ptr<Channel> create(const GraphComponent& c);
            [[nodiscard]] std::unique_ptr<Channel> createCopy(const GraphComponent& c) const;
            [[nodiscard]] std::unique_ptr<Channel> clone(const GraphComponent& c) const;

            // Construct
            void constructFromXML(CNodePtr channelNode);
            void constructPropertiesFromXML(const Scenario& s,
                                            CNodePtr channelPropertiesNode);

            // Convert
            void convertToXML(CNodePtr channelNode);
            void convertPropertiesToXML(const Scenario& s,
                                        CNodePtr channelPropertiesNode);

            // Isolate scenario
            void isolateScenario(const Scenario& s);

            // Connections
            [[nodiscard]] Port *getSrcPort() const
            {
                return src;
            };
            [[nodiscard]] Port *getDstPort() const
            {
                return dst;
            };
            [[nodiscard]] Actor *getSrcActor() const
            {
                return (src == nullptr ?  nullptr : &(getSrcPort()->getActor()));
            };
            [[nodiscard]] Actor *getDstActor() const
            {
                return (dst == nullptr ?  nullptr : &(getDstPort()->getActor()));
            };
            void connectSrc(Port *p);
            void connectDst(Port *p);
            Port *oppositePort(Port *p) const
            {
                return (src == p ? dst : src);
            };

            // Initial tokens
            [[nodiscard]] uint getInitialTokens() const
            {
                return initialTokens;
            };
            void setInitialTokens(const uint t)
            {
                initialTokens = t;
            };

            [[nodiscard]] const CStrings &getPersistentTokenNames() const
            {
                return scPersistentTokenNames;
            };
            void setPersistentTokenNames(const CStrings &n);
            void setPersistentTokenNames(const CString &n);
            void addPersistentTokenName(const CString &n);
            void addPersistentFinalTokenName(const CString &n);

            // final tokens for weakly consistent scenarios
            [[nodiscard]] uint getFinalTokens() const
            {
                return finalTokens;
            };
            void setFinalTokens(const uint t)
            {
                finalTokens = t;
            };
            void setInitialAndFinalTokens(const uint t)
            {
                initialTokens = t;
                finalTokens = t;
            };
            [[nodiscard]] const CStrings &getPersistentFinalTokenNames() const
            {
                return scPersistentFinalTokenNames;
            };
            void setPersistentFinalTokenNames(const CStrings &n);
            void setPersistentFinalTokenNames(const CString &n);

            void setPersistentAndFinalTokenNames(const CStrings &n);
            void setPersistentAndFinalTokenNames(const CString &n);
            // Properties
            [[nodiscard]] bool isConnected() const;

            // Token size
            [[nodiscard]] const std::map<const Scenario *, Size> &getTokenSize() const
            {
                return tokenSize;
            };
            void setTokenSize(const std::map<const Scenario *, Size> &s);
            [[nodiscard]] Size getTokenSizeOfScenario(const Scenario& s) const;
            void setTokenSizeOfScenario(const Scenario& s, Size sz);
            [[nodiscard]] bool hasTokenSizeInScenario(const Scenario& s) const;

            void resetPersistentAndFinalTokenNames();

            // Graph
            [[nodiscard]] ScenarioGraph *getScenarioGraph() const;

            // Print
            std::ostream &print(std::ostream &out);
            friend std::ostream &operator<<(std::ostream &out, Channel &c)
            {
                return c.print(out);
            };

            bool isSelfEdge()
            {
                return &(src->getActor()) == &(dst->getActor());
            }

            //is capacity (storage) channel
            bool isCapacityChannel;

            // set channel type
            void setType(const CString& type);

            // get channel type
            [[nodiscard]] CString getType() const;
        private:
            // Default scenario
            [[nodiscard]] ScenarioRef getDefaultScenarioRef() const;

            // Ports
            Port *src;
            Port *dst;

            // Initial tokens
            uint initialTokens;

            // Final tokens
            uint finalTokens;

            // names of the persistent tokens
            CStrings scPersistentTokenNames;
            CStrings scPersistentFinalTokenNames;

            // Token size
            std::map<const Scenario *, Size> tokenSize;

            // Channel Type
            CString type;
    };

} // End namespace FSMSADF

#endif
