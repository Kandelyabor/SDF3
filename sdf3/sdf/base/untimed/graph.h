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
 *  Function        :   SDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.1.1.1.2.1 2010-04-24 23:52:43 mgeilen Exp $
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

#ifndef SDF_BASE_UNTIMED_GRAPH_H_INCLUDED
#define SDF_BASE_UNTIMED_GRAPH_H_INCLUDED

#include "actor.h"
#include "channel.h"
#include "input.h"
#include "output.h"
#include "port.h"
#include <memory>

namespace SDF {

/**
 * SDFgraph
 * Container for SDF graph.
 */
class SDFgraph : public SDFcomponent {
public:
    // Constructor
    explicit SDFgraph(SDFcomponent &c);
    SDFgraph();

    // Copy constructor
    SDFgraph(const SDFgraph& other) = delete;
    // Copy assignment operator
    SDFgraph& operator=(const SDFgraph& other) = delete;
    // Move constructor
    SDFgraph(SDFgraph&& other) noexcept = delete;
    // Move assignment operator
    SDFgraph& operator=(SDFgraph&& other) noexcept = delete;

    // Destructor
    ~SDFgraph() override = default;

    // Construct
    virtual std::unique_ptr<SDFgraph> create(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFgraph> createCopy(SDFcomponent &c) const;
    virtual std::unique_ptr<SDFgraph> clone(SDFcomponent &c) const;
    void construct(CNodePtr sdfNode);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Actors
    SDFactor *getActor(CId id);
    [[nodiscard]] SDFactor *getConstActor(CId id) const;
    [[nodiscard]] SDFactor *getActor(const CString &name) const;
    [[nodiscard]] SDFactor *getConstActor(const CString &name) const;
    [[nodiscard]] SDFactor* getActorPtr(const CId &id) const;
    [[nodiscard]] SDFactor* getConstActorPtr(const CId &id) const ;
    [[nodiscard]] SDFactor* getActorPtr(const CString &name) const;
    [[nodiscard]] SDFactor* getConstActorPtr(const CString &name) const ;
    [[nodiscard]] const SDFactors &getActors() const { return actors; };
    [[nodiscard]] SDFactorRefs getActorRefs() const {
        SDFactorRefs actors;
        for (const auto &actor : this->actors) {
            actors.push_back(actor.get());
        }
        return actors;
    };

    [[nodiscard]] uint nrActors() const { return static_cast<uint>(actors.size()); };
    SDFactorsIter actorsBegin() { return actors.begin(); };
    SDFactorsIter actorsEnd() { return actors.end(); };
    [[nodiscard]] SDFactorsCIter actorsBegin() const { return actors.begin(); };
    [[nodiscard]] SDFactorsCIter actorsEnd() const { return actors.end(); };
    void addActor(std::unique_ptr<SDFactor> a);
    void removeActor(const CString &name);
    virtual SDFactorRef createActor();
    virtual SDFactorRef createActor(SDFcomponent &c);

    // Inputs
    SDFinput *getInput(CId id);
    [[nodiscard]] SDFinput *getInput(const CString &name) const;
    [[nodiscard]] const SDFinputs &getInputs() const { return inputs; };
    [[nodiscard]] uint nrInputs() const { return static_cast<uint>(inputs.size()); }
    SDFinputsIter inputsBegin() { return inputs.begin(); };
    SDFinputsIter inputsEnd() { return inputs.end(); };
    [[nodiscard]] SDFinputsCIter inputsBegin() const { return inputs.begin(); };
    [[nodiscard]] SDFinputsCIter inputsEnd() const { return inputs.end(); };
    void addInput(std::unique_ptr<SDFinput> i);
    void removeInput(const CString &name);
    virtual SDFinput *createInput();
    virtual SDFinput *createInput(SDFcomponent &c);

    // Outputs
    SDFoutput *getOutput(CId id);
    [[nodiscard]] SDFoutput *getOutput(const CString &name) const;
    [[nodiscard]] const SDFoutputs &getOutputs() const { return outputs; };
    [[nodiscard]] uint nrOutputs() const { return static_cast<uint>(outputs.size()); }
    SDFoutputsIter outputsBegin() { return outputs.begin(); };
    SDFoutputsIter outputsEnd() { return outputs.end(); };
    [[nodiscard]] SDFoutputsCIter outputsBegin() const { return outputs.begin(); };
    [[nodiscard]] SDFoutputsCIter outputsEnd() const { return outputs.end(); };
    void addOutput(std::unique_ptr<SDFoutput> o);
    void removeOutput(const CString &name);
    virtual SDFoutput *createOutput();
    virtual SDFoutput *createOutput(SDFcomponent &c);

    // unconnected ports
    SDFport *getPort(CId id);
    SDFport *getPort(const CString &name);
    [[nodiscard]] uint nrPorts() const { return static_cast<uint>(ports.size()); };
    SDFportsIter portsBegin() { return ports.begin(); };
    SDFportsIter portsEnd() { return ports.end(); };
    [[nodiscard]] const SDFports &getPorts() const { return ports; };
    [[nodiscard]] SDFportsCIter portsBegin() const { return ports.begin(); };
    [[nodiscard]] SDFportsCIter portsEnd() const { return ports.end(); };
    void addPort(std::unique_ptr<SDFport> p);
    void removePort(const CString &name);
    virtual SDFportRef createPort(SDFcomponent &c);
    SDFportRef createPort(SDFport::SDFportType type, SDFrate rate);

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Channels
    [[nodiscard]] SDFchannel *getChannel(const CId& id) const;
    [[nodiscard]] SDFchannel *getChannel(const CString &name) const;
    [[nodiscard]] uint nrChannels() const { return static_cast<uint>(channels.size()); };
    [[nodiscard]] const SDFchannels &getChannels() const { return channels; };
    [[nodiscard]] SDFchannelRefs getChannelRefs() const {
        SDFchannelRefs channels;
        for (const auto &channel : this->channels) {
            channels.push_back(channel.get());
        }
        return channels;
    };
    SDFchannelsIter channelsBegin() { return channels.begin(); };
    SDFchannelsIter channelsEnd() { return channels.end(); };
    [[nodiscard]] SDFchannelsCIter channelsBegin() const { return channels.begin(); };
    [[nodiscard]] SDFchannelsCIter channelsEnd() const { return channels.end(); };
    void addChannel(std::unique_ptr<SDFchannel> c);
    void removeChannel(const CString &name);
    virtual SDFchannelRef createChannel(SDFcomponent &c);
    SDFchannelRef createChannel(
            SDFactor *src, SDFrate rateSrc, SDFactor *dst, SDFrate rateDst, uint initialTokens);

    // Input channels
    SDFinputchannel *getInputChannel( CId id);
    [[nodiscard]] SDFinputchannel *getInputChannel(const CString &name) const;
    [[nodiscard]] uint nrInputChannels() const { return static_cast<uint>(inputchannels.size()); };
    SDFinputchannelsIter inputchannelsBegin() { return inputchannels.begin(); };
    SDFinputchannelsIter inputchannelsEnd() { return inputchannels.end(); };
    [[nodiscard]] SDFinputchannelsCIter inputchannelsBegin() const { return inputchannels.begin(); };
    [[nodiscard]] SDFinputchannelsCIter inputchannelsEnd() const { return inputchannels.end(); };
    [[nodiscard]] const SDFinputchannels &getInputChannels() const { return inputchannels; };
    [[nodiscard]] const SDFoutputchannels &getOutputChannels() const { return outputchannels; };
    void addInputChannel(std::unique_ptr<SDFinputchannel> c);
    void removeInputChannel(const CString &name);
    virtual SDFinputchannelRef createInputChannel(SDFcomponent &c);
    SDFinputchannelRef createInputChannel(SDFactor *dst, SDFrate rateDst, uint initialTokens);

    // Output channels
    SDFoutputchannel *getOutputChannel( CId id);
    [[nodiscard]] SDFoutputchannel *getOutputChannel(const CString &name) const;
    [[nodiscard]] uint nrOutputChannels() const { return static_cast<uint>(outputchannels.size()); };
    SDFoutputchannelsIter outputchannelsBegin() { return outputchannels.begin(); };
    SDFoutputchannelsIter outputchannelsEnd() { return outputchannels.end(); };
    [[nodiscard]] SDFoutputchannelsCIter outputchannelsBegin() const { return outputchannels.begin(); };
    [[nodiscard]] SDFoutputchannelsCIter outputchannelsEnd() const { return outputchannels.end(); };
    void addOutputChannel(std::unique_ptr<SDFoutputchannel> c);
    void removeOutputChannel(const CString &name);
    virtual SDFoutputchannelRef createOutputChannel(SDFcomponent &c);

    SDFoutputchannelRef createOutputChannel(SDFactor *src, SDFrate rateSrc, uint initialTokens);

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFgraph &g) { return g.print(out); };

private:
    // Type
    CString type;

    // Unconnected ports
    SDFports ports;

    // Actors and channels
    SDFactors actors;
    SDFinputs inputs;
    SDFoutputs outputs;
    SDFchannels channels;
    SDFinputchannels inputchannels;
    SDFoutputchannels outputchannels;
    //  SDFchannels outchannels;
};

using SDFgraphs = std::list<SDFgraph *>;
using SDFgraphsIter = SDFgraphs::iterator;
using SDFgraphsCIter = SDFgraphs::const_iterator;

} // namespace SDF
#endif
