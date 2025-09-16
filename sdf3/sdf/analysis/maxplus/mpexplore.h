/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 13, 2008
 *
 *  Function        :   Max-Plus based state-space exploration algorithms.
 *
 *  History         :
 *      13-05-08    :   Initial version.
 *
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

#ifndef MPEXPLORE_H_INCLUDED
#define MPEXPLORE_H_INCLUDED

#include <list>
#include <memory>
#include <vector>

#include "maxplus/algebra/mpmatrix.h"
#include "base/exception/exception.h"
#include "maxplus/algebra/mptype.h"
#include "sdf/base/algo/repetition_vector.h"
#include "sdf/base/timed/graph.h"


namespace SDF::MaxPlusAnalysis {

// forward declarations
class State;
class SymbolicState;
class SymbolicToken;
/*
 * Name space GraphDecoration is used to store information about an SDF graph
 * related to MaxPlus based analysis.
 */
namespace GraphDecoration {

// Forward class definition
class Graph;

/**
 * class Channel
 * Decorates an SDF channel with information required for MaxPLus analysis
 */
class Channel {
public:
    unsigned int index;
    unsigned int nrInitialTokens;

    // an upper bound for the maximum number of tokens that might accumulate in a channel during the
    // simulation. the bound is calculated as srcActorRate*correspondingRepetitionVectorEntry +
    // nrInitialTokens
    unsigned int synchStorage;
    unsigned int repetitionVectorEntry;

    // where to start storing tokens slots in token table / state
    unsigned int stateStartIndex;

    // where to start storing history variables in token table
    unsigned int historyStartIndex;

    // is this a channel that represents a storage constraint
    bool isStorageChannel;
};

using ChannelRef = Channel*;

/**
 * class ChannelList
 * A list of GraphDecoration::SDFChannels
 */
class ChannelList : public std::vector<std::unique_ptr<Channel>> {
public:
    ChannelList() = default;
    ~ChannelList() = default;
    ChannelList(const ChannelList &other)  { // NOLINT(*copy-constructor-init)
        for (const auto &c : other) {
            this->push_back(std::make_unique<Channel>(*c));
        }
    }
    ChannelList& operator=(const ChannelList&) = delete;
    ChannelList(ChannelList&&) = delete;
    ChannelList& operator=(ChannelList&&) = delete;

};
class ChannelRefList : public std::vector<ChannelRef> {};

/**
 * class InputChannel
 * Decorates an SDF input channel with information required for MaxPLus analysis
 */
class InputChannel {
public:
    unsigned int index;

    // unsigned int nrInitialTokens=0;

    unsigned int repetitionVectorEntry;

    unsigned int inputStartIndex;
};

/**
 * class InputChannelList
 * A list of GraphDecoration::SDFInputChannels
 */
class InputChannelList : public std::vector<std::unique_ptr<InputChannel>> {
public:
    InputChannelList() = default;
    ~InputChannelList() = default;
    InputChannelList(const InputChannelList &other) { // NOLINT(*copy-constructor-init)
        for (const auto &ic : other) {
            this->push_back(std::make_unique<InputChannel>(*ic));
        }
    }
    InputChannelList& operator=(const InputChannelList&) = delete;
    InputChannelList(InputChannelList&&) = delete;
    InputChannelList& operator=(InputChannelList&&) = delete;

};

/**
 * class OutputChannel
 * Decorates an SDF output channel with information required for MaxPLus analysis
 */
class OutputChannel {
public:
    unsigned int index;

    unsigned int repetitionVectorEntry;

    unsigned int outputStartIndex;
};

/**
 * class OutputChannelList
 * A list of GraphDecoration::SDFOutputChannels
 */
class OutputChannelList : public std::vector<std::unique_ptr<OutputChannel>> {
public:
    OutputChannelList() = default;
    ~OutputChannelList() = default;
    OutputChannelList(const OutputChannelList &other) { // NOLINT(*copy-constructor-init)
        for (const auto &oc : other) {
            this->push_back(std::make_unique<OutputChannel>(*oc));
        }
    }
    OutputChannelList& operator=(const OutputChannelList&) = delete;
    OutputChannelList(OutputChannelList&&) = delete;
    OutputChannelList& operator=(OutputChannelList&&) = delete;
};

/**
 * class Port
 * Decorates an SDF port with information required for MaxPLus analysis.
 */
class Port {
public:
    Port(unsigned int _rate, Channel *_channel);
    unsigned int rate;
    Channel *channel;
};

/**
 * class PortList
 * A list of ports
 */
class PortList : public std::vector<std::unique_ptr<Port>> {
public:
    PortList() = default;
    ~PortList() = default;
    PortList(const PortList &other) { // NOLINT(*copy-constructor-init)
        for (const auto &p : other) {
            this->push_back(std::make_unique<Port>(*p));
        }
    }
    PortList& operator=(const PortList&) = delete;
    PortList(PortList&&) = delete;
    PortList& operator=(PortList&&) = delete;

};

/*
 * class InPort
 * Decorates an SDF port connected to an input channel
 */
class InPort {
public:
    InPort(unsigned int _rate, InputChannel *_inputChannel);
    unsigned int rate;
    InputChannel *inputChannel;
};
/*
 * class InPortList
 * A list of InPorts
 */
class InPortList : public std::vector<std::unique_ptr<InPort>> {
public:
    InPortList() = default;
    ~InPortList() = default;
    InPortList(const InPortList &other) { // NOLINT(*copy-constructor-init)
        for (const auto &ip : other) {
            this->push_back(std::make_unique<InPort>(*ip));
        }
    }
    InPortList& operator=(const InPortList&) = delete;
    InPortList(InPortList&&) = delete;
    InPortList& operator=(InPortList&&) = delete;

};

/*
 * class OutPort
 * Decorates an SDF port connected to an output channel
 */
class OutPort {
public:
    OutPort(unsigned int _rate, OutputChannel *_outputChannel);
    unsigned int rate;
    OutputChannel *outputChannel;
};
/*
 * class OutPortList
 * A list of OutPorts
 */
class OutPortList : public std::vector<std::unique_ptr<OutPort>> {
public:
    OutPortList() = default;
    ~OutPortList() = default;
    OutPortList(const OutPortList &other) { // NOLINT(*copy-constructor-init)
        for (const auto &op : other) {
            this->push_back(std::make_unique<OutPort>(*op));
        }
    }
    OutPortList& operator=(const OutPortList&) = delete;
    OutPortList(OutPortList&&) = delete;
    OutPortList& operator=(OutPortList&&) = delete;

};

/**
 * class Actor
 * Decorates an SDF actor with information for MaxPlus analysis
 */
class Actor {
public:
    unsigned int index;
    MaxPlus::MPDelay executionTime;
    PortList inputPorts;
    PortList  outputPorts;
    InPortList  InPorts;
    OutPortList  OutPorts;
    Actor();
    ~Actor();

    // Delete copy and move constructors and assignment operators
    Actor(const Actor&) = default;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator=(Actor&&) = delete;
};

using  ActorRef = Actor*;

/**
 * class ActorList
 * A list of actors
 */
class ActorList : public std::vector<std::unique_ptr<Actor>>{
    public:
    ActorList() = default;
    ~ActorList() = default;
    ActorList(const ActorList &other) { // NOLINT(*copy-constructor-init)
        for (const auto &a : other) {
            this->push_back(std::make_unique<Actor>(*a));
        }
    }
    ActorList& operator=(const ActorList&) = delete;
    ActorList(ActorList&&) = delete;
    ActorList& operator=(ActorList&&) = delete;

};
class ActorRefList : public std::vector<ActorRef>{};

/**
 * class TokenLabel
 * Decorates an SDF graph with individually labelled initial tokens.
 * Tokens on the same channel are ordered by their label and with an ID
 */
class TokenLabel {
public :
    CString name;       // token name
    uint index;         // general index
    bool channelFirst;  // index on channel
    bool channelLast;  // index on channel
    SDFchannel *channel;   // channel on which the token resides
};

/**
 * class ActorCount
 * Associates with every actor a number, representing a number of firings
 * (e.g., a repetition vector)
 */
class ActorCount {
public:
    explicit ActorCount(const Graph& G);
    explicit ActorCount(size_t sz);
    ActorCount() = default;
    ~ActorCount();

    // copy and move constructors and assignment operators
    ActorCount(const ActorCount&);
    ActorCount& operator=(const ActorCount&) = delete;
    ActorCount(ActorCount&&) = delete;
    ActorCount& operator=(ActorCount&&) = delete;

    void reset(const Graph& G);
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] unsigned int getValue(const Actor& a) const;
    void setValue(const Actor& a, unsigned int v);
    void setValue(unsigned int i, unsigned int v);
    void decreaseValue(const Actor& a);
    std::unique_ptr<ActorCount> copy();
    [[nodiscard]] unsigned int sum() const;

private:
    size_t size{};
    std::unique_ptr<unsigned int[]> table;
};

/**
 * class Vector sequence
 * Creates a list of vectors, representing a sequence of graph iterations
 */
class VectorSequence : public std::vector<std::unique_ptr<MaxPlus::Vector>> {
public:
    VectorSequence() = default;
    ~VectorSequence() = default;
    VectorSequence(const VectorSequence &other) { // NOLINT(*copy-constructor-init)
        for (const auto &v : other) {
            this->push_back(std::make_unique<MaxPlus::Vector>(*v));
        }
    }
    VectorSequence& operator=(const VectorSequence&) = delete;
    VectorSequence(VectorSequence&&) = default;
    VectorSequence& operator=(VectorSequence&&) = delete;

};

/**
 * class Graph
 * Decorates an SDF graph with information for MaxPlus analysis.
 */
class Graph {
public:
    Graph();
    virtual ~Graph();
    Graph(const Graph&);
    Graph& operator=(const Graph&) = delete;
    Graph(Graph&&) = delete;
    Graph& operator=(Graph&&) = delete;
    explicit Graph(const TimedSDFgraph& g, bool weakConsistent = false, RepetitionVector *q = nullptr);
    static std::pair<std::unique_ptr<Graph>, std::unique_ptr<Graph>> OpenGraphConversion(const TimedSDFgraph& g);
    ActorList actors;
    ActorRefList outputActors;
    ChannelList channels;
    InputChannelList inputChannels;
    OutputChannelList outputChannels;
    [[nodiscard]] const ActorCount& repetitionVector() const;

    // equals the total number of initial tokens on the graph. statsize determines the size of the
    // symbolic time stamps
    unsigned int stateSize;
    unsigned int inputSize;
    unsigned int outputSize;
    unsigned int historySize;
    unsigned int newTokenIndex;
    static inline void fire(State& x, const Actor& a);
    static void fire(State& x, const Actor& a, double precision);
    void fireSymbolic(SymbolicState& x, const Actor& a);
    void fireSymbolicCaptureTracingInfo(SymbolicState& x,
                                        const Actor& a,
                                        MaxPlus::Matrix *MH,
                                        unsigned int actorFiringsCounter) const;
    void fireSymbolicCaptureOutputMatrix(SymbolicState& x, const Actor& a, MaxPlus::Matrix &MO) const;
    static void fire_reverse(State& x, const Actor& a);
    //    CString fireSchedule(State* x, Actor* a, ActorCount *remainingVector,
    //            ostream& sout);
    virtual std::unique_ptr<SymbolicToken> makeMinusInfinityToken(unsigned int s);

private:
    ActorCount repVec;
    void init();
};

class GraphEMR : public Graph {
public:
    explicit GraphEMR(const TimedSDFgraph& g,
                      bool weakConsistent = false,
                      RepetitionVector *q = nullptr) :
        Graph(g, weakConsistent, q) {}
    // virtual void fireSymbolic(SymbolicStateEMR *x, const Actor& a);
    std::unique_ptr<SymbolicToken> makeMinusInfinityToken(unsigned int s) override;
};
} // namespace GraphDecoration

/**
 * class CDeadlockException
 * Thrown in symbolic execution when the graph deadlocks
 */
class CDeadlockException : public CException {
public:
    explicit CDeadlockException(const CString &message) : CException(message){};
};

/**
 * class Exploration
 * A MaxPlusExploration object encapsulates MaxPlus analysis algorithms.
 */
class Exploration {
public:
    Exploration() = default;
    virtual ~Exploration() = default;

    // Delete copy and move constructors and assignment operators
    Exploration(const Exploration&) = delete;
    Exploration& operator=(const Exploration&) = delete;
    Exploration(Exploration&&) = delete;
    Exploration& operator=(Exploration&&) = delete;

    std::unique_ptr<GraphDecoration::Graph> G;
    std::unique_ptr<GraphDecoration::Graph> OG;
    const TimedSDFgraph* g{};
    std::unique_ptr<std::list<CString>> actorsFiringOrder;
    MaxPlus::MPThroughput explore();
    MaxPlus::MPThroughput explore_backward();
    [[nodiscard]] CStrings getMatrixActorLabelling() const;
    [[nodiscard]] CStrings getMatrixStateLabelling() const;
    [[nodiscard]] CStrings getMatrixInputLabelling() const;
    [[nodiscard]] CStrings getMatrixOutputLabelling() const;
    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> convertToMaxPlusMatrix() const;
    [[nodiscard]] std::list<std::unique_ptr<GraphDecoration::TokenLabel>> getTokenDecoration() const;
    std::unique_ptr<std::array<std::unique_ptr<MaxPlus::Matrix>, 2>> convertToTracingInfo();
    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> convertToMaxPlusMatrixForWeakFSMSADF();
    std::unique_ptr<std::array<std::unique_ptr<MaxPlus::Matrix>, 2>> convertToTracingInfoWeakConsistency();
    [[nodiscard]] std::pair<std::unique_ptr<MaxPlus::Matrix>, std::unique_ptr<MaxPlus::Matrix>> computeStateAndOutputMatrices() const;
    [[nodiscard]] bool isLinearStateSpaceDeadlocking() const;
    [[nodiscard]] std::tuple<std::unique_ptr<MaxPlus::Matrix>, std::unique_ptr<MaxPlus::Matrix>,
                             std::unique_ptr<MaxPlus::Matrix>, std::unique_ptr<MaxPlus::Matrix>,
                             std::unique_ptr<MaxPlus::Matrix>, std::unique_ptr<MaxPlus::Matrix>>
                             computeLinearStateSpaceMatrices() const;
    [[nodiscard]] std::tuple<GraphDecoration::VectorSequence,
                             GraphDecoration::VectorSequence,
                             GraphDecoration::VectorSequence>
                             selfTimedStateSpaceSchedule(uint iterations) const;
    [[nodiscard]] std::unique_ptr<TimedSDFgraph> convertToHSDFByMP() const;
    [[nodiscard]] MaxPlus::MPTime exploreDelayForSchedule(MaxPlus::Vector *gamma0,
                                            MaxPlus::Vector *gamma_s,
                                            MaxPlus::MPTime period);
    void exploreEigen(std::unique_ptr<MaxPlus::Vector> *eigenvector, MaxPlus::MPTime *eigenvalue);
    std::unique_ptr<GraphDecoration::ActorRef[]> staticSchedule;
    [[nodiscard]] static MaxPlus::MPDelay timeProgress(State *x);
    static MaxPlus::MPDelay normalize(State& x);

    void limitTheMaxNumberOfStates() { this->limitMaxNumberOfStates = true; }
    void setMaxNumberOfStates(uint64_t n) { this->maxNumberOfStates = n; }
    [[nodiscard]] bool maxNumberOfStatesWasReached() const { return this->maxNumberOfStatesReached; }

    // factory methods
    virtual std::unique_ptr<SymbolicState> makeSymbolicState();
    virtual std::unique_ptr<MaxPlus::Matrix> makeMatrix(unsigned int nrows, unsigned int nCols);

private:
    static CString &replaceString(CString &s, const CString &fnd, const CString &rep);
    bool limitMaxNumberOfStates = false;
    bool maxNumberOfStatesReached = false;
    uint64_t maxNumberOfStates = 0;
};

class ExplorationEMR : public Exploration {
public:
    ExplorationEMR() = default;
    ~ExplorationEMR() override = default;

    // Delete copy and move constructors and assignment operators
    ExplorationEMR(const ExplorationEMR&) = delete;
    ExplorationEMR& operator=(const ExplorationEMR&) = delete;
    ExplorationEMR(ExplorationEMR&&) = delete;
    ExplorationEMR& operator=(ExplorationEMR&&) = delete;

    // factory methods
    std::unique_ptr<SymbolicState> makeSymbolicState() override;
    std::unique_ptr<MaxPlus::Matrix> makeMatrix(unsigned int nrows, unsigned int nCols) override;
};
/**
 * mpStateSpaceThroughputAnalysis()
 * Compute throughput by MaxPlus exploration.
 */
MaxPlus::MPTime mpStateSpaceThroughputAnalysis(const TimedSDFgraph& g, std::ostream &sout);

/**
 * mpStateSpaceThroughputAnalysis_backward()
 * Compute throughput by backward MaxPlus exploration
 */
MaxPlus::MPTime mpStateSpaceThroughputAnalysis_backward(const TimedSDFgraph& g, std::ostream &sout);

/**
 * mpStateSpaceDelayAnalysis()
 * Compute the initial delay to obtain a delay-period linear model for a given
 * period and a given initial schedule and steady-state schedule
 */
MaxPlus::MPTime mpStateSpaceDelayAnalysis(const TimedSDFgraph& g,
                                          MaxPlus::Vector *gamma0,
                                          MaxPlus::Vector *gamma_s,
                                          MaxPlus::MPTime period,
                                          std::ostream &sout);

/**
 * mpStateSpaceEigenAnalysis()
 * Compute the eigenvector and eigenvalue of an SDF graph.
 */
void mpStateSpaceEigenAnalysis(const TimedSDFgraph& g,
                               std::unique_ptr<MaxPlus::Vector> *eigenvector,
                               MaxPlus::MPTime *eigenvalue,
                               std::ostream &sout);

} // namespace SDF::MaxPlusAnalysis

#endif