/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: graph.cc,v 1.4.2.2 2010-05-10 22:25:02 mgeilen Exp $
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
#include <algorithm>
namespace CSDF {
/**
 * CSDFgraph ()
 * Constructor.
 */
CSDFgraph::CSDFgraph(CSDFcomponent &c) : CSDFcomponent(c) {}

/**
 * ~CSDFgraph ()
 * Destructor.
 */
CSDFgraph::~CSDFgraph() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 */
std::unique_ptr<CSDFgraph> CSDFgraph::create(CSDFcomponent &c) const {
    return std::make_unique<CSDFgraph>(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * All properties, actors and channels of the graph are cloned.
 */
std::unique_ptr<CSDFgraph> CSDFgraph::clone(CSDFcomponent &c) const {
    auto g = std::make_unique<CSDFgraph>(c);

    // Properties
    g->setName(getName());
    g->setType(getType());

    // Actors
    for (const auto& iter : actors) {
        CSDFcomponent component = CSDFcomponent(g.get(), g->nrActors());
        std::unique_ptr<CSDFactor> a = iter->clone(component);
        g->addActor(std::move(a));
    }

    // Channels
    for (const auto& iter : channels) {
        CSDFcomponent component = CSDFcomponent(g.get(), g->nrChannels());
        std::unique_ptr<CSDFchannel> ch = iter->clone(component);
        g->addChannel(std::move(ch));
    }

    return g;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * All properties of the graph are copied.
 */
std::unique_ptr<CSDFgraph> CSDFgraph::createCopy(CSDFcomponent &c) const {
    auto g = std::make_unique<CSDFgraph>(c);

    // Properties
    g->setName(getName());
    g->setType(getType());

    return g;
}

/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void CSDFgraph::construct(const CNodePtr graphNode) {
    CSDFgraph *g = this;

    // Name
    if (!CHasAttribute(graphNode, "name")) {
        throw CException("Invalid CSDF graph, missing graph name.");
    }
    g->setName(CGetAttribute(graphNode, "name"));

    // Type
    if (!CHasAttribute(graphNode, "type")) {
        throw CException("Invalid CSDF graph, missing graph type.");
    }
    g->setType(CGetAttribute(graphNode, "type"));
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given id.
 */
CSDFactor *CSDFgraph::getActor(const CId id) const {
    for (const auto& a : actors) {
        if (a->getId() == id) {
            return a.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}

/**
 * getConstActorPtr ()
 * The function returns a shared pointer reference to an actor with the given id.
 */
const CSDFactor* CSDFgraph::getConstActorPtr(const CId &id) const {
    for (const auto& a: this->getConstActors()) {
        if (a->getId() == id) {
            return a.get();
        }
    }
    throw CException("Graph '" + getName() + "' has no actor with id '" + CString(id) + "'.");
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given name.
 */
CSDFactor *CSDFgraph::getActor(const CString &name) const {
    for (const auto& a : actors) {
        if (a->getName() == name) {
            return a.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no actor '" + name + "'.");
}

/**
 * addActor ()
 * Add an actor to a graph.
 */
void CSDFgraph::addActor(std::unique_ptr<CSDFactor> a) {
    actors.push_back(std::move(a));
}

/**
 * removeActor ()
 * Remove an actor from a graph.
 */
void CSDFgraph::removeActor(const CString &name) {
    for (auto iter = actors.begin(); iter != actors.end(); iter++) {
        const CSDFactor& a = *(*iter);

        if (a.getName() == name) {
            actors.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no actor '" + name + "'.");
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
CSDFactor *CSDFgraph::createActor() {
    CSDFcomponent c = CSDFcomponent(this, nrActors());
    auto a = std::make_unique<CSDFactor>(c);
    a->setName(CString("_a") + CString(nrActors() + 1));
    auto* ar = a.get();
    addActor(std::move(a));

    return ar;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
CSDFactor *CSDFgraph::createActor(CSDFcomponent &c) {
    auto a = std::make_unique<CSDFactor>(c);
    auto* ar = a.get();
    addActor(std::move(a));
    return ar;
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given id.
 */
CSDFchannel *CSDFgraph::getChannel(const CId id) {
    for (const auto& c : channels) {
        if (c->getId() == id) {
            return c.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no channel with id '" + CString(id) + "'.");
}

/**
 * getConstChannel ()
 * The function returns a reference to a channel with the given id.
 */
const CSDFchannel *CSDFgraph::getConstChannel(const CId id) const {
    for (const auto &c : channels) {
        if (c->getId() == id) {
            return c.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no channel with id '" + CString(id) + "'.");
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given name.
 */
CSDFchannel *CSDFgraph::getChannel(const CString &name) {
    for (const auto& c : channels) {
        if (c->getName() == name) {
            return c.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no channel '" + name + "'.");
}

/**
 * getConstChannel ()
 * The function returns a reference to a channel with the given name.
 */
const CSDFchannel *CSDFgraph::getConstChannel(const CString &name) const {
    for (const auto& c : channels) {
        if (c->getName() == name) {
            return c.get();
        }
    }

    throw CException("Graph '" + getName() + "' has no channel '" + name + "'.");
}

/**
 * addChannel ()
 * Add a channel to a graph.
 */
void CSDFgraph::addChannel(std::unique_ptr<CSDFchannel> c) {
    channels.push_back(std::move(c));
}

/**
 * removeChannel ()
 * Remove a channel from a graph.
 */
void CSDFgraph::removeChannel(const CString &name) {
    for (auto iter = channels.begin(); iter != channels.end(); iter++) {
        CSDFchannel& c = *(*iter);

        if (c.getName() == name) {
            channels.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no channel '" + name + "'.");
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
CSDFchannel *CSDFgraph::createChannel(CSDFcomponent &c) {
    auto ch = std::make_unique<CSDFchannel>(c);
    auto* chr = ch.get();
    addChannel(std::move(ch));

    return chr;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
CSDFchannel *CSDFgraph::createChannel(CSDFactor *src,
                                      const CSDFrate &rateSrc,
                                      CSDFactor *dst,
                                      const CSDFrate &rateDst) {
    // Create new channel
    CSDFcomponent c = CSDFcomponent(this, nrChannels());
    CSDFchannel *ch = createChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));

    // Create ports on the actors
    CSDFport *srcP = src->createPort("out", rateSrc);
    srcP->setName(CString("_p") + CString(src->nrPorts()));
    CSDFport *dstP = dst->createPort("in", rateDst);
    dstP->setName(CString("_p") + CString(dst->nrPorts()));

    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);

    return ch;
}

/**
 * isConnected ()
 * The function returns true if all actor ports are connected to a channel
 * and all channels are connected to a port. Else it returns false.
 */
bool CSDFgraph::isConnected() const {
    bool result = std::all_of(actors.begin(), actors.end(), [](const auto& a){
        return a->isConnected();
    });
    result = result && std::all_of(channels.begin(), channels.end(), [](const auto& c){
        return c->isConnected();
    });
    return result;
}

/**
 * isConsistent ()
 * The function returns true if the graph is consistent (i.e. the repetition
 * vector is non-zero). Else, it returns false.
 */
bool CSDFgraph::isConsistent() const {
    RepetitionVector vec = getRepetitionVector();

    return vec[0] != 0;
}

/**
 * getRepetitionVector ()
 * The function calculates and returns the repetition vector of the graph.
 */
CSDFgraph::RepetitionVector CSDFgraph::getRepetitionVector() const {
    CFractions fractions(nrActors(), CFraction(-1, 1));

    // Calculate firing ratio (as fraction) for each actor
    for (const auto& a : actors) {
        CFraction &f = fractions[a->getId()];

        if (f == CFraction(-1, 1)) {
            f = CFraction(1, 1);
            calcFractionsConnectedActors(fractions, *a);
        }
    }

    // Calculate repetition vector based on firing ratios
    return calcRepetitionVector(fractions);
}

/**
 * isEmpty ()
 * The function test if all entries in the repetition vector are 0.
 */
bool isEmpty(const CSDFgraph::RepetitionVector &v) {
    auto i = v.begin();
    while (i != v.end()) {
        if ((*i) != 0) {
            return false;
        }
        i++;
    }
    return true;
}

/**
 * calcFractionsConnectedActors ()
 * The function calculates and firing ration (as fractions) of
 * all actors connected to actor 'a' based on its firing rate. In case
 * of an inconsistent graph, all fractions are set to 0.
 */
void CSDFgraph::calcFractionsConnectedActors(CFractions &fractions, const CSDFactor& a) const {
    CFraction fractionA = fractions[a.getId()];

    // Inconsistent graph?
    if (fractionA == CFraction(0, 0)) {
        return;
    }

    // Calculate the rate for each actor 'b' connected to actor 'a'
    for (auto iter = a.portsBegin(); iter != a.portsEnd(); iter++) {
        const CSDFport& pA = *(*iter);

        // Get actor 'b' on other side of channel and its port 'pB'
        CSDFchannel *c = pA.getChannel();
        CSDFport *pB = c->oppositePort(&pA);
        CSDFactor *b = pB->getActor();

        // Tokens produced or consumed by actor A in one firing on average
        CFraction rateA = CFraction(0, 1);
        for (unsigned int i : pA.getRate()) {
            rateA = rateA + CFraction(static_cast<int>(i));
        }
        rateA = rateA / CFraction(static_cast<int>(pA.getRate().size()));

        // Tokens produced or consumed by actor B in one firing on average
        CFraction rateB = CFraction(0, 1);
        for (unsigned int i : pB->getRate()) {
            rateB = rateB + CFraction(static_cast<int>(i));
        }
        rateB = rateB / CFraction(static_cast<int>(pB->getRate().size()));

        // Graph inconsistent?
        if (rateA == CFraction(0) || rateB == CFraction(0)) {
            // Inconsistent graph, set all fractions to 0
            for (auto &fraction : fractions) {
                fraction = CFraction(0, 0);
            }

            return;
        }

        // Calculate firing rate 'b'
        CFraction ratioAB = rateA / rateB;
        CFraction fractionB = fractionA * ratioAB;

        // Known firing rate for 'b'
        CFraction knownFractionB = fractions[b->getId()];

        // Compare known and calculated firing rate of 'b'
        if (knownFractionB != CFraction(-1, 1) && fractionB != knownFractionB) {
            // Inconsistent graph, set all fractions to 0
            for (auto &fraction : fractions) {
                fraction = CFraction(0, 0);
            }

            return;
        }
        if (knownFractionB == CFraction(-1, 1)) {
            // Set the firing rate of actor 'b'
            fractions[b->getId()] = fractionB;

            // Calculate firing rate for all actors connected to 'b'
            calcFractionsConnectedActors(fractions, *b);

            // Is graph inconsistent?
            if (fractions[b->getId()] == CFraction(0, 0)) {
                return;
            }
        }
    }
}

CSDFgraph::RepetitionVector CSDFgraph::calcRepetitionVector(CFractions &fractions) const {
    RepetitionVector repetitionVector(fractions.size(), 0);
    int64_t l = 1;

    // Find lowest common multiple (lcm) of all denominators
    for (auto &f : fractions) {

        l = lcm(l, f.denominator());
    }

    // Zero vector?
    if (l == 0) {
        return repetitionVector;
    }

    // Calculate non-zero repetition vector
    for (uint i = 0; i < fractions.size(); i++) {
        repetitionVector[i] =
                static_cast<int>((fractions[i].numerator() * l) / fractions[i].denominator());
    }

    // Find greatest common divisor (gcd)
    int g = repetitionVector[0];
    for (uint i = 1; i < repetitionVector.size(); i++) {
        g = gcd(g, repetitionVector[i]);
    }

    // Minimize the repetition vector using the gcd
    for (int &i : repetitionVector) {
        i = i / g;
    }

    // find the smallest factor to make all actors exercise a whole number of repetitions of their
    // execution cycle.
    uint ratePeriod = 1;

    // Compute period of repetition for rate vectors
    for (const auto& c : channels) {
        CSDFactor *src = c->getSrcActor();
        uint rs = static_cast<uint>(c->getSrcPort()->getRate().size());
        uint gs = gcd(static_cast<uint>(repetitionVector[src->getId()]), rs);
        ratePeriod = lcm(ratePeriod, rs / gs);

        CSDFactor *dst = c->getDstActor();
        uint rd = static_cast<uint>(c->getDstPort()->getRate().size());
        uint gd = gcd(static_cast<uint>(repetitionVector[dst->getId()]), rd);
        ratePeriod = lcm(ratePeriod, rd / gd);
    }

    // Multiply the repetition vector with the rate period
    for (int &i : repetitionVector) {
        i = i * static_cast<int>(ratePeriod);
    }

    return repetitionVector;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
std::ostream &CSDFgraph::print(std::ostream &out) {
    out << "Graph (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "type:      " << getType() << std::endl;
    out << std::endl;

    for (const auto& a : actors) {
        a->print(out);
    }

    for (const auto& c : channels) {
        c->print(out);
    }

    out << std::endl;

    return out;
}

/**
 * constructCSDFgraph ()
 * Construct an CSDF graph.
 */
std::unique_ptr<CSDFgraph> constructCSDFgraph(const CNodePtr csdfNode) {
    CId actorId = 0;
    CId channelId = 0;
    CId portId = 0;

    // Construct CSDF graph
    CSDFcomponent component = CSDFcomponent(nullptr, 0);
    auto g = std::make_unique<CSDFgraph>(component);
    g->construct(csdfNode);

    // Construct all actors
    for (CNodePtr actorNode = CGetChildNode(csdfNode, "actor"); actorNode != nullptr;
         actorNode = CNextNode(actorNode, "actor")) {
        // Construct actor
        component = CSDFcomponent(g.get(), actorId);
        auto a = std::make_unique<CSDFactor>(component);
        a->construct(actorNode);

        // Construct all ports
        portId = 0;
        for (CNodePtr portNode = CGetChildNode(actorNode, "port"); portNode != nullptr;
             portNode = CNextNode(portNode, "port")) {
            // Construct port
            component = CSDFcomponent(a.get(), portId);
            auto p = std::make_unique<CSDFport>(component);
            p->construct(portNode);

            // Add port to actor
            auto* pr = p.get();
            a->addPort(std::move(p));

            // Is the length of the rate sequence equal to the sequenceLength
            // of the actor?
            if (pr->getRate().size() != a->sequenceLength()) {
                throw CException("[ERROR] length of rate sequences not equal.");
            }

            // Next port
            portId++;
        }

        // Add actor to graph
        g->addActor(std::move(a));

        // Next actor
        actorId++;
    }

    // Construct all channels
    for (CNodePtr channelNode = CGetChildNode(csdfNode, "channel"); channelNode != nullptr;
         channelNode = CNextNode(channelNode, "channel")) {
        // Construct channel
        component = CSDFcomponent(g.get(), channelId);
        auto c = std::make_unique<CSDFchannel>(component);
        c->construct(channelNode);

        // Add channel to graph
        g->addChannel(std::move(c));

        // Next channel
        channelId++;
    }

    return g;
}
} // namespace CSDF
