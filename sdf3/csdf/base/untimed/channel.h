/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF channel
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: channel.h,v 1.1.1.1 2007-10-02 10:59:49 sander Exp $
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

#ifndef CSDF_BASE_CHANNEL_H_INCLUDED
#define CSDF_BASE_CHANNEL_H_INCLUDED

#include "actor.h"
namespace CSDF
{
    // Forward class definition
    class CSDFchannel;
    class CSDFgraph;

    /**
     * CSDFchannel
     * CSDF channel.
     */
    class CSDFchannel : public CSDFcomponent
    {
        public:

            // Constructor
            explicit CSDFchannel(CSDFcomponent &c);

            // Destructor
             ~CSDFchannel() override;

            // Prevent copying and moving
            CSDFchannel(const CSDFchannel&) = delete;
            CSDFchannel& operator=(const CSDFchannel&) = delete;
            CSDFchannel(CSDFchannel&&) = delete;
            CSDFchannel& operator=(CSDFchannel&&) = delete;

            // Construct
            virtual std::unique_ptr<CSDFchannel> create(CSDFcomponent &c) const;
            virtual std::unique_ptr<CSDFchannel> createCopy(CSDFcomponent &c) const;
            virtual std::unique_ptr<CSDFchannel> clone(CSDFcomponent &c) const;
            void construct( CNodePtr channelNode);

            // Connections
            [[nodiscard]] CSDFport *getSrcPort() const
            {
                return src;
            };
            [[nodiscard]] CSDFport *getDstPort() const
            {
                return dst;
            };
            [[nodiscard]] CSDFactor *getSrcActor() const
            {
                return (src == nullptr ?  nullptr : getSrcPort()->getActor());
            };
            [[nodiscard]] CSDFactor *getDstActor() const
            {
                return (dst == nullptr ?  nullptr : getDstPort()->getActor());
            };
            void connectSrc(CSDFport *p);
            void connectDst(CSDFport *p);
            CSDFport *oppositePort(const CSDFport *p) const
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

            // Properties
            [[nodiscard]] bool isConnected() const;

            // Graph
            [[nodiscard]] CSDFgraph *getGraph() const;

            // Print
            std::ostream &print(std::ostream &out);
            friend std::ostream &operator<<(std::ostream &out, CSDFchannel &c)
            {
                return c.print(out);
            };

        private:
            CSDFport *src;
            CSDFport *dst;
            uint    initialTokens;
    };

    using CSDFchannels = std::list<std::unique_ptr<CSDFchannel>>;
    using CSDFchannelRefs = std::list<CSDFchannel *>;
    using CSDFchannelsIter = CSDFchannels::iterator;
    using CSDFchannelsCIter = CSDFchannels::const_iterator;
} //namespace CSDF
#endif
