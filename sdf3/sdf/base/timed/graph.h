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
 *  Function        :   Timed SDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.2 2008-03-17 14:07:37 sander Exp $
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

#ifndef SDF_BASE_TIMED_GRAPH_H_INCLUDED
#define SDF_BASE_TIMED_GRAPH_H_INCLUDED

#include "actor.h"
#include "channel.h"
#include "input.h"
#include <memory>

namespace SDF {

/**
 * TimedSDFgraph
 * Container for timed SDF graph.
 */
class TimedSDFgraph : public SDFgraph {
public:
    // Constructor
    explicit TimedSDFgraph(SDFcomponent &c);
    TimedSDFgraph();

    // Copy constructor
    TimedSDFgraph(const TimedSDFgraph& other) = delete;
    // Copy assignment operator
    TimedSDFgraph& operator=(const TimedSDFgraph& other) = delete;
    // Move constructor
    TimedSDFgraph(TimedSDFgraph&& other) noexcept = delete;
    // Move assignment operator
    TimedSDFgraph& operator=(TimedSDFgraph&& other) noexcept = delete;

    // Destructor
    ~TimedSDFgraph() override;

    // Construct
    std::unique_ptr<SDFgraph> create(SDFcomponent &c) const override;
    std::unique_ptr<SDFgraph> createCopy(SDFcomponent &c) const override;
    std::unique_ptr<SDFgraph> clone(SDFcomponent &c) const override;
    [[nodiscard]] std::unique_ptr<TimedSDFgraph> clone() const;
    void construct(CNodePtr sdfNode, CNodePtr sdfPropertiesNode);

    // Properties
    void setProperties( CNodePtr propertiesNode);
    [[nodiscard]] bool isDiscreteTimeGraph() const;

    // Actors
    TimedSDFactor *createActor() override;
    TimedSDFactor *createActor(SDFcomponent &c) override;

    // Inputs
    TimedSDFinput *createInput() override;
    TimedSDFinput *createInput(SDFcomponent &c) override;

    // Channels
    TimedSDFchannel *createChannel(SDFcomponent &c) override;
    TimedSDFchannel *createChannel(
            SDFactor *src, SDFrate rateSrc, SDFactor *dst, SDFrate rateDst, uint initialTokens);

    // Time constraints
    [[nodiscard]] SDFthroughput getThroughputConstraint() const { return throughputConstraint; };
    void setThroughputConstraint(const SDFthroughput t) { throughputConstraint = t; };

    // dynamic cast
    static std::unique_ptr<TimedSDFgraph> dynamicCast(std::unique_ptr<SDFgraph> g);

    // Print
    std::ostream &print(std::ostream &out) const;
    friend std::ostream &operator<<(std::ostream &out, TimedSDFgraph &g) { return g.print(out); };

private:
    // Throughput
    SDFthroughput throughputConstraint = 0.0;
    // Flag possible changed during parsing to check if all execution times are integer
    bool isDiscreteTime = true;
};

} // namespace SDF
#endif
