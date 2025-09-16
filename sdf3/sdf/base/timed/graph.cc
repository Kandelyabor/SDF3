/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   timed_graph.cc
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
 * $Id: graph.cc,v 1.2 2008-03-17 14:07:37 sander Exp $
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

#include "graph.h"
#include "base/exception/exception.h"
#include <memory>
namespace SDF
{
    /**
     * TimedSDFgraph ()
     * Constructor.
     */
    TimedSDFgraph::TimedSDFgraph(SDFcomponent &c)
        :
        SDFgraph(c)
    {
    }

    /**
     * TimedSDFgraph ()
     * Constructor.
     */
    TimedSDFgraph::TimedSDFgraph()
    = default;

    /**
     * ~TimedSDFgraph ()
     * Destructor.
     */
    TimedSDFgraph::~TimedSDFgraph()
    = default;

    /**
     * create ()
     * The function returns a pointer to a newly allocated SDF graph object.
     */
    std::unique_ptr<SDFgraph> TimedSDFgraph::create(SDFcomponent &c) const
    {
        return std::make_unique<TimedSDFgraph>(c);
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated SDF graph object.
     * The properties of this graph are also assigned to the new graph.
     */
    std::unique_ptr<SDFgraph> TimedSDFgraph::clone(SDFcomponent &c) const
    {
        std::unique_ptr<TimedSDFgraph> g = TimedSDFgraph::dynamicCast(createCopy(c));

        // Actors
        for (const auto& iter: this->getActors())
        {
            SDFcomponent component = SDFcomponent(g.get(), g->nrActors());
            std::unique_ptr<SDFactor> a = iter->clone(component);
            g->addActor(std::move(a));
        }

        // Inputs
        for (const auto& iter: this->getInputs())
        {
            SDFcomponent component = SDFcomponent(g.get(), g->nrInputs());
            auto i = iter->clone(component);
            g->addInput(std::move(i));
        }

        // Outputs
        for (const auto& iter: this->getOutputs())
        {
            SDFcomponent component = SDFcomponent(g.get(), g->nrOutputs());
            std::unique_ptr<SDFoutput> o = iter->clone(component);
            g->addOutput(std::move(o));
        }

        // Channels
        for (const auto& iter: this->getChannels())
        {
            SDFcomponent component = SDFcomponent(g.get(), g->nrChannels());
            std::unique_ptr<SDFchannel> ch = iter->clone(component);
            g->addChannel(std::move(ch));
        }

        return g;
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated SDF graph object.
     * The properties of this graph are also assigned to the new graph.
     */
    std::unique_ptr<TimedSDFgraph> TimedSDFgraph::clone() const
    {
        SDFcomponent comp = SDFcomponent(getParent(), getId(), getName());
        return TimedSDFgraph::dynamicCast(clone(comp));
    }

    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated SDF graph object.
     * The properties of this graph are also assigned to the new graph.
     */
    std::unique_ptr<SDFgraph> TimedSDFgraph::createCopy(SDFcomponent &c) const
    {
        auto g = std::make_unique<TimedSDFgraph>(c);

        // Properties
        g->setName(getName());
        g->setType(getType());

        return g;
    }

    /**
    * dynamicCast ()
    * Dynamic cast of untimed base type to timed derived type.
    */
    std::unique_ptr<TimedSDFgraph> TimedSDFgraph::dynamicCast(std::unique_ptr<SDFgraph> g) {
        return std::unique_ptr<TimedSDFgraph>(dynamic_cast<TimedSDFgraph*>(g.release()));
    }


    /**
     * construct ()
     * Set the properties of a timed graph.
     */
    void TimedSDFgraph::construct(const CNodePtr sdfNode,
                                  const CNodePtr sdfPropertiesNode)
    {
        // Construct SDFG
        SDFgraph::construct(sdfNode);

        // Any properties to be set?
        if (sdfPropertiesNode == nullptr) {
            return;
        }

        // Set all properties
        for (CNode *propertiesNode = CGetChildNode(sdfPropertiesNode);
             propertiesNode != nullptr; propertiesNode = CNextNode(propertiesNode))
        {
            if (CIsNode(propertiesNode, "actorProperties"))
            {
                if (!CHasAttribute(propertiesNode, "actor")) {
                    throw CException("Missing 'actor' in 'actorProperties'");
                }

                TimedSDFactor *a = nullptr;
                a = dynamic_cast<TimedSDFactor *>(getActor(CGetAttribute(propertiesNode, "actor")));

                a->setProperties(propertiesNode);
            }
            else if (CIsNode(propertiesNode, "inputProperties"))
            {
                if (!CHasAttribute(propertiesNode, "input")) {
                    throw CException("Missing 'input' in 'inputProperties'");
                }

                TimedSDFinput *i = nullptr;
                i = dynamic_cast<TimedSDFinput *>(getInput(CGetAttribute(propertiesNode, "input")));

                i->setProperties(propertiesNode);

            }
            else if (CIsNode(propertiesNode, "channelProperties"))
            {
                if (!CHasAttribute(propertiesNode, "channel")) {
                    throw CException("Missing 'channel' in 'channelProperties'");
                }

                TimedSDFchannel *c = nullptr;
                c = dynamic_cast<TimedSDFchannel *>(getChannel(CGetAttribute(propertiesNode,
                                                  "channel")));

                c->setProperties(propertiesNode);
            }
            else if (CIsNode(propertiesNode, "graphProperties"))
            {
                setProperties(propertiesNode);
            }
        }

        // check if the graph is discrete-time
        for (const auto& a : this->getActors()) {
            const auto* ta = dynamic_cast<TimedSDFactor*>(a.get());
            if (! ta->isDiscreteTimeActor()) {
                this->isDiscreteTime = false;
            }
        }
    }

    /**
     * setProperties ()
     * Set the properties of a timed graph.
     */
    void TimedSDFgraph::setProperties(const CNodePtr propertiesNode)
    {
        // Timing constraints
        for (CNode *timeNode = CGetChildNode(propertiesNode, "timeConstraints");
             timeNode != nullptr; timeNode = CNextNode(timeNode, "timeConstraints"))
        {
            if (CHasChildNode(timeNode, "throughput"))
            {
                CNode *throughputNode = nullptr;
                throughputNode = CGetChildNode(timeNode, "throughput");

                CString throughput = CGetNodeContent(throughputNode);

                setThroughputConstraint(static_cast<double>(throughput));
            }
        }
    }

    bool TimedSDFgraph::isDiscreteTimeGraph() const { return this->isDiscreteTime;}


    /**
     * createActor ()
     * Create a new actor on the graph.
     */
    TimedSDFactor *TimedSDFgraph::createActor()
    {
        SDFcomponent c = SDFcomponent(this, nrActors());
        auto a = std::make_unique<TimedSDFactor>(c);
        a->setName(CString("_a") + CString(nrActors() + 1));
        auto* ar = a.get();
        addActor(std::move(a));

        return ar;
    }

    /**
     * createActor ()
     * Create a new actor on the graph.
     */
    TimedSDFactor *TimedSDFgraph::createActor(SDFcomponent &c)
    {
        auto a = std::make_unique<TimedSDFactor>(c);
        auto* ar = a.get();
        addActor(std::move(a));

        return ar;
    }

    /**
     * createInput ()
     * Create a new input on the graph.
     */
    TimedSDFinput *TimedSDFgraph::createInput()
    {
        SDFcomponent c = SDFcomponent(this, nrInputs());
        auto i = std::make_unique<TimedSDFinput>(c);
        i->setName(CString("_i") + CString(nrInputs() + 1));
        auto* ir = i.get();
        addInput(std::move(i));

        return ir;
    }

    /**
     * createInput ()
     * Create a new input on the graph.
     */
    TimedSDFinput *TimedSDFgraph::createInput(SDFcomponent &c)
    {
        auto i = std::make_unique<TimedSDFinput>(c);
        auto* ir = i.get();
        addInput(std::move(i));

        return ir;
    }

    /**
     * createChannel ()
     * Create a new channel on the graph.
     */
    TimedSDFchannel *TimedSDFgraph::createChannel(SDFcomponent &c)
    {
        auto ch = std::make_unique<TimedSDFchannel>(c);
        auto* chr = ch.get();
        addChannel(std::move(ch));

        return chr;
    }

    /**
     * createChannel ()
     * The function creates a channel between the source and destination actor.
     * Ports with the supplied rates are added to these actors.
     */
    TimedSDFchannel *TimedSDFgraph::createChannel(SDFactor *src, SDFrate rateSrc,
            SDFactor *dst, SDFrate rateDst, uint initialTokens)
    {
        // Create new channel
        SDFcomponent c = SDFcomponent(this, nrChannels());
        TimedSDFchannel *ch = createChannel(c);
        ch->setName(CString("_ch") + CString(nrChannels()));

        // Create ports on the actors
        SDFport *srcP = src->createPort(SDFport::SDFportType::Out, rateSrc);
        srcP->setName(CString("_p") + CString(src->nrPorts()));
        SDFport *dstP = dst->createPort(SDFport::SDFportType::In, rateDst);
        dstP->setName(CString("_p") + CString(dst->nrPorts()));

        // Connect channel
        ch->connectSrc(srcP);
        ch->connectDst(dstP);

        // Initial tokens
        ch->setInitialTokens(initialTokens);

        return ch;
    }

    /**
     * print ()
     * Print the graph to the supplied output stream.
     */
    std::ostream &TimedSDFgraph::print(std::ostream &out) const
    {
        out << "Graph (" << getName() << ")" << std::endl;
        out << "id:        " << getId() << std::endl;
        out << "type:      " << getType() << std::endl;
        out << std::endl;

        for (const auto& a: this->getActors())
        {
            (dynamic_cast<TimedSDFactor*>(a.get()))->print(out);
        }

        for (const auto& c: this->getChannels())
        {
            (dynamic_cast<TimedSDFchannel*>(c.get()))->print(out);
        }

        out << std::endl;

        return out;
    }
}//namespace SDF
