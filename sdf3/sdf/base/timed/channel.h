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
 *  Function        :   Timed SDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: channel.h,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_TIMED_CHANNEL_H_INCLUDED
#define SDF_BASE_TIMED_CHANNEL_H_INCLUDED

#include "sdf/base/untimed/graph.h" // IWYU pragma: keep
#include "timed_types.h"
namespace SDF {

/**
 * TimedSDFchannel
 * Timed SDF channel.
 */
class TimedSDFchannel : public SDFchannel {
public:
    // Buffer size
    using BufferSize = struct BufferSize {
        int sz;
        int src;
        int dst;
        int mem;
    };

    // Constructor
    explicit TimedSDFchannel(SDFcomponent &c);

    // Destructor
    ~TimedSDFchannel() override;

    TimedSDFchannel(const TimedSDFchannel&) = delete;
    TimedSDFchannel& operator=(const TimedSDFchannel&) = delete;
    TimedSDFchannel(TimedSDFchannel&&) = delete;
    TimedSDFchannel& operator=(TimedSDFchannel&&) = delete;


    // Construct
    std::unique_ptr<SDFchannel> create(SDFcomponent &c) const override;
    std::unique_ptr<SDFchannel> createCopy(SDFcomponent &c) const override;
    std::unique_ptr<SDFchannel> clone(SDFcomponent &c) const override;

    // Properties
    void setProperties(CNodePtr propertiesNode);

    // Buffer sizes
    [[nodiscard]] BufferSize getBufferSize() const { return bufferSize; };
    void setBufferSize(const BufferSize s) { bufferSize = s; };
    [[nodiscard]] bool isUnbounded() const;
    [[nodiscard]] TimedSDFchannel *getStorageSpaceChannel() const { return modelStorageSpaceCh; };
    void setStorageSpaceChannel(TimedSDFchannel *c) { modelStorageSpaceCh = c; };
    [[nodiscard]] bool modelsStorageSpace() const { return modelStorageSpaceCh != nullptr; };

    // Bandwidth
    [[nodiscard]] double getMinBandwidth() const { return minBandwidth; };
    void setMinBandwidth(const double b) { minBandwidth = b; };

    // Token size
    [[nodiscard]] int getTokenSize() const { return tokenSize; };
    void setTokenSize(const int s) { tokenSize = s; };
    [[nodiscard]] bool isControlToken() const;
    [[nodiscard]] bool isDataToken() const;

    // Token type
    [[nodiscard]] CString getTokenType() const { return tokenType; };
    void setTokenType(const CString &type) { tokenType = type; };

    // Latency
    [[nodiscard]] SDFtime getMinLatency() const { return minLatency; };
    void setMinLatency(const SDFtime t) { minLatency = t; };

    // dynamic cast
    static std::unique_ptr<TimedSDFchannel> dynamicCast(std::unique_ptr<SDFchannel> c);

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, TimedSDFchannel &c) { return c.print(out); };

    bool isCapacityChannel;

private:
    TimedSDFchannel *modelStorageSpaceCh;
    BufferSize bufferSize;
    CString tokenType;
    double minBandwidth;
    int tokenSize;
    SDFtime minLatency;
};

} // namespace SDF
#endif
