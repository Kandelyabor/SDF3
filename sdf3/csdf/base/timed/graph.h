/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   timed_graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Timed CSDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.2 2008-03-22 14:24:21 sander Exp $
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

#ifndef CSDF_BASE_TIMED_GRAPH_H_INCLUDED
#define CSDF_BASE_TIMED_GRAPH_H_INCLUDED

#include "actor.h"
#include "channel.h"

namespace CSDF {
/**
 * TimedCSDFgraph
 * Container for timed CSDF graph.
 */
class TimedCSDFgraph : public CSDFgraph {
public:
    // Constructor
    explicit TimedCSDFgraph(CSDFcomponent &c);

    // Copy constructor
    TimedCSDFgraph(const TimedCSDFgraph& other) = delete;

    // Copy assignment operator
    TimedCSDFgraph& operator=(const TimedCSDFgraph& other) = delete;

    // Move constructor
    TimedCSDFgraph(TimedCSDFgraph&& other) noexcept = delete;

    // Move assignment operator
    TimedCSDFgraph& operator=(TimedCSDFgraph&& other) noexcept = delete;

    // Destructor
    ~TimedCSDFgraph() override;

    // Construct
    std::unique_ptr<CSDFgraph> create(CSDFcomponent &c) const override;
    std::unique_ptr<CSDFgraph> createCopy(CSDFcomponent &c) const override;
    std::unique_ptr<CSDFgraph> clone(CSDFcomponent &c) const override;
    [[nodiscard]] std::unique_ptr<TimedCSDFgraph> clone() const;

    // Actors
    TimedCSDFactor *createActor() override;
    TimedCSDFactor *createActor(CSDFcomponent &c) override;

    // Channels
    TimedCSDFchannel *createChannel(CSDFcomponent &c) override;
    TimedCSDFchannel *
    createChannel(CSDFactor *src, const CSDFrate& rateSrc, CSDFactor *dst, const CSDFrate& rateDst);

    // Time constraints
    [[nodiscard]] CSDFthroughput getThroughputConstraint() const { return throughputConstraint; };
    void setThroughputConstraint(const CSDFthroughput t) { throughputConstraint = t; };

    // dynamic cast
    static std::unique_ptr<TimedCSDFgraph> dynamicCast(std::unique_ptr<CSDFgraph> g);

    // Print
    std::ostream &print(std::ostream &out) const;
    friend std::ostream &operator<<(std::ostream &out, TimedCSDFgraph &g) { return g.print(out); };

private:
    // Throughput
    CSDFthroughput throughputConstraint = 0.0;
};

/**
 * constructTimedCSDFgraph ()
 * Construct a timed CSDF graph.
 */
std::unique_ptr<TimedCSDFgraph> constructTimedCSDFgraph(CNodePtr csdfNode, CNodePtr csdfPropertiesNode);
} // namespace CSDF
#endif
