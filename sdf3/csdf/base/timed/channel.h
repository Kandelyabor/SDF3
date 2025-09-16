/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed CSDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
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

#ifndef CSDF_BASE_TIMED_CHANNEL_H_INCLUDED
#define CSDF_BASE_TIMED_CHANNEL_H_INCLUDED

#include "csdf/base/untimed/graph.h" // IWYU pragma: keep
#include "timed_types.h"
namespace CSDF
{
    // Buffer size
    using CSDFbufferSize = struct CSDFbufferSize
    {
        int sz;
        int src;
        int dst;
        int mem;
    };

    /**
     * TimedCSDFchannel
     * Timed CSDF channel.
     */
    class TimedCSDFchannel : public CSDFchannel
    {
        public:

            // Constructor
            explicit TimedCSDFchannel(CSDFcomponent &c);

            // Destructor
            ~TimedCSDFchannel() override;

            // Delete copy/move constructors and assignment operators
            TimedCSDFchannel(const TimedCSDFchannel&) = delete;
            TimedCSDFchannel& operator=(const TimedCSDFchannel&) = delete;
            TimedCSDFchannel(TimedCSDFchannel&&) = delete;
            TimedCSDFchannel& operator=(TimedCSDFchannel&&) = delete;


            // Construct
            std::unique_ptr<CSDFchannel> create(CSDFcomponent &c) const override;
            std::unique_ptr<CSDFchannel> createCopy(CSDFcomponent &c) const override;
            std::unique_ptr<CSDFchannel> clone(CSDFcomponent &c) const override;

            // Buffer sizes
            [[nodiscard]] CSDFbufferSize getBufferSize() const
            {
                return bufferSize;
            };
            void setBufferSize(const CSDFbufferSize s)
            {
                bufferSize = s;
            };
            [[nodiscard]] bool isUnbounded() const;
            [[nodiscard]] TimedCSDFchannel *getStorageSpaceChannel() const
            {
                return modelStorageSpaceCh;
            };
            void setStorageSpaceChannel(TimedCSDFchannel *c)
            {
                modelStorageSpaceCh = c;
            };
            [[nodiscard]] bool modelsStorageSpace() const
            {
                return modelStorageSpaceCh != nullptr;
            };

            // Bandwidth
            [[nodiscard]] double getMinBandwidth() const
            {
                return minBandwidth;
            };
            void setMinBandwidth(const double b)
            {
                minBandwidth = b;
            };

            // Token size
            [[nodiscard]] int getTokenSize() const
            {
                return tokenSize;
            };
            void setTokenSize(const int s)
            {
                tokenSize = s;
            };
            [[nodiscard]] bool isControlToken() const;
            [[nodiscard]] bool isDataToken() const;

            // Token type
            [[nodiscard]] CString getTokenType() const
            {
                return tokenType;
            };
            void setTokenType(const CString &type)
            {
                tokenType = type;
            };

            // Latency
            [[nodiscard]] CSDFtime getMinLatency() const
            {
                return minLatency;
            };
            void setMinLatency(const CSDFtime t)
            {
                minLatency = t;
            };

            // Print
            std::ostream &print(std::ostream &out) const;
            friend std::ostream &operator<<(std::ostream &out, TimedCSDFchannel &c)
            {
                return c.print(out);
            };

        private:
            TimedCSDFchannel *modelStorageSpaceCh;
            CSDFbufferSize bufferSize;
            CString tokenType;
            double minBandwidth;
            int tokenSize;
            CSDFtime minLatency;
    };

    using TimedCSDFchannels = std::list<TimedCSDFchannel *>;
    using TimedCSDFchannelsIter = TimedCSDFchannels::iterator;
    using TimedCSDFchannelsCIter = TimedCSDFchannels::const_iterator;

} // namespace CSDF
#endif
