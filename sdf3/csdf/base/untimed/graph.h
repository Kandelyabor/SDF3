/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
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
 * $Id: graph.h,v 1.1.1.1.2.1 2010-04-25 01:21:16 mgeilen Exp $
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

#ifndef CSDF_BASE_GRAPH_H_INCLUDED
#define CSDF_BASE_GRAPH_H_INCLUDED

#include "channel.h"
namespace CSDF {
/**
 * CSDFgraph
 * Container for CSDF graph.
 */
class CSDFgraph : public CSDFcomponent {
public:
    // Constructor
    explicit CSDFgraph(CSDFcomponent &c);

    // Destructor
    ~CSDFgraph() override;

    // Prevent copying and moving
    CSDFgraph(const CSDFgraph&) = delete;
    CSDFgraph& operator=(const CSDFgraph&) = delete;
    CSDFgraph(CSDFgraph&&) = delete;
    CSDFgraph& operator=(CSDFgraph&&) = delete;

    // Construct
    virtual std::unique_ptr<CSDFgraph> create(CSDFcomponent &c) const;
    virtual std::unique_ptr<CSDFgraph> createCopy(CSDFcomponent &c) const;
    virtual std::unique_ptr<CSDFgraph> clone(CSDFcomponent &c) const;
    void construct(CNodePtr graphNode);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Actors
    [[nodiscard]] CSDFactor *getActor(CId id) const;
    [[nodiscard]] const CSDFactor* getConstActorPtr(const CId &id) const ;
    [[nodiscard]] CSDFactor* getActorPtr(const CString &name);
    [[nodiscard]] const CSDFactor* getConstActorPtr(const CString &name) const ;
    [[nodiscard]] CSDFactor *getActor(const CString &name) const;
    CSDFactors &getActors() { return actors; };
    [[nodiscard]] const CSDFactors &getConstActors() const { return actors; };
    [[nodiscard]] uint nrActors() const { return static_cast<uint>(actors.size()); };
    CSDFactorsIter actorsBegin() { return actors.begin(); };
    CSDFactorsIter actorsEnd() { return actors.end(); };
    [[nodiscard]] CSDFactorsCIter actorsBegin() const { return actors.begin(); };
    [[nodiscard]] CSDFactorsCIter actorsEnd() const { return actors.end(); };
    void addActor(std::unique_ptr<CSDFactor> a);
    void removeActor(const CString &name);
    virtual CSDFactor *createActor();
    virtual CSDFactor *createActor(CSDFcomponent &c);

    // Channels
    CSDFchannel *getChannel(CId id);
    [[nodiscard]] const CSDFchannel *getConstChannel(CId id) const;
    CSDFchannel *getChannel(const CString &name);
    [[nodiscard]] const CSDFchannel *getConstChannel(const CString &name) const;
    CSDFchannels &getChannels() { return channels; };
    [[nodiscard]] const CSDFchannels &getConstChannels() const { return channels; };
    [[nodiscard]] uint nrChannels() const { return static_cast<uint>(channels.size()); };
    CSDFchannelsIter channelsBegin() { return channels.begin(); };
    CSDFchannelsIter channelsEnd() { return channels.end(); };
    [[nodiscard]] CSDFchannelsCIter channelsBegin() const { return channels.begin(); };
    [[nodiscard]] CSDFchannelsCIter channelsEnd() const { return channels.end(); };
    void addChannel(std::unique_ptr<CSDFchannel> c);
    void removeChannel(const CString &name);
    virtual CSDFchannel *createChannel(CSDFcomponent &c);
    CSDFchannel *createChannel(CSDFactor *src, const CSDFrate& rateSrc, CSDFactor *dst, const CSDFrate& rateDst);

    // Properties
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isConsistent() const;

    using RepetitionVector = std::vector<int>;
    using RepetitionVectorIter = RepetitionVector::iterator;
    using RepetitionVectorCIter = RepetitionVector::const_iterator;

    [[nodiscard]] virtual RepetitionVector getRepetitionVector() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, CSDFgraph &g) { return g.print(out); };

private:
    // Repetition vector
    void calcFractionsConnectedActors(CFractions &fractions, const CSDFactor& a) const;
    RepetitionVector calcRepetitionVector(CFractions &fractions) const;

    // Information
    CString type;

    // Actors and channels
    CSDFactors actors;
    CSDFchannels channels;
};

using CSDFgraphs = std::list<CSDFgraph *>;
using CSDFgraphsIter = CSDFgraphs::iterator;
using CSDFgraphsCIter = CSDFgraphs::const_iterator;

/**
 * constructCSDFgraph ()
 * Construct an CSDF graph.
 */
std::unique_ptr<CSDFgraph> constructCSDFgraph( CNodePtr csdfNode);

/**
 * isEmpty ()
 * The function test if all entries in the repetition vector are 0.
 */
bool isEmpty(const CSDFgraph::RepetitionVector &v);

} // namespace CSDF
#endif
