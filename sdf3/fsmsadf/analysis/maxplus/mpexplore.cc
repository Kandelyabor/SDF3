/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.cc
 *
 *  Author          :   Sander Stuijk (s.stuijk@tue.nl)
 *
 *  Date            :   May 5, 2009
 *
 *  Function        :   Max-Plus based state-space exploration algorithms.
 *
 *  History         :
 *      05-05-09    :   Initial version.
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

#include "mpexplore.h"
#include "maxplus/algebra/mpmatrix.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>

namespace FSMSADF::MPExplore {

using namespace MaxPlus;

/**
 * exploreDelayForSchedule()
 * Compute the minimal delay of a delay-period model for give period, starting
 * schedule and steady-state schedule according to the algorithm presented in
 * the paper "Synchronous Dataflow Scenarios".
 */
MPTime Exploration::exploreDelayForSchedule(const MaxPlus::Vector &gamma0,
                                            const MaxPlus::Vector &gamma_s,
                                            MPTime period) const {
    const std::unique_ptr<State> gamma = State::fromVector(*G, gamma0);
    const std::unique_ptr<State> schedule = State::fromVector(*G, gamma_s);
    MPTime tau = MP_MINUS_INFINITY;

    // delta accumulates the total elapsed time minus the number of iterations
    // times period such that the actual schedule equals the normalized schedule
    // + k.period + delta
    MPTime delta = gamma->normalize() - period;

    // do a regular exploration and compute delay tau along the way.
    State *y = nullptr;
    StoredStates S;
    S.store(gamma->copy());

    // make one iteration to determine the schedule;
    ActorCount E(*(G->repetitionVector()));
    const unsigned int sch_length = E.sum();
    std::vector<Actor *> staticSchedule(sch_length);
    unsigned int sch_idx = 0;

    Actors::const_iterator i = G->scenarioGraph->getActors().begin();
    while (!E.isEmpty()) {
        uint iterCnt = 0;
        ActorRef a = nullptr;
        // find an arbitrary enabled actor
        while (true) {
            a = static_cast<ActorRef>(i->get());
            if (E.getValue(a) > 0 && gamma->actorEnabled(G->scenario, a)) {
                break;
            }
            if (++i == G->scenarioGraph->getActors().end()) {
                i = G->scenarioGraph->getActors().begin();
                iterCnt++;
                if (iterCnt > 1) {
                    throw CException("Graph is deadlocked.");
                }
            }
        }
        E.decreaseValue(a);
        staticSchedule[sch_idx++] = a;
        G->fire(gamma.get(), a);
    }

    // determine the difference between the reference schedule and the actual
    // schedule
    auto m = std::unique_ptr<State>(gamma->minus(*schedule));
    tau = MP_MAX(tau, m->norm() + delta);

    // update delta by adding the normalization difference and subtracting
    // period
    delta += gamma->normalize();
    delta -= period;

    // after the first iteration, continue with fixed schedule
    // until a recurrent state is found
    while (!S.includes(gamma.get(), &y)) {
        S.store(gamma->copy());
        for (uint k = 0; k < sch_length; k++) {
            G->fire(gamma.get(), staticSchedule[k]);
        }

        // update tau and delta
        auto m = std::unique_ptr<State>(gamma->minus(*schedule));
        tau = MP_MAX(tau, m->norm() + delta);
        delta += gamma->normalize();
        delta -= period;
    }

    // tau equals the delay we seek.
    return tau;
}

/**
 * exploreEigen()
 * Compute the eigenvector and eigenvalue of the MaxPlus matrix associated with
 * the scenario graph.
 */
void Exploration::exploreEigen(std::unique_ptr<MaxPlus::Vector> *eigenvector,
                               MPTime *eigenvalue) const {
    MPDelay lambda;
    auto gamma = std::unique_ptr<State>(State::zeroState(*G));

    // make one iteration to determine the schedule;
    ActorCount E(*(G->repetitionVector()));
    unsigned int sch_length = E.sum();
    std::vector<Actor *> staticSchedule(sch_length);
    unsigned int sch_idx = 0;

    Actors::const_iterator i = G->scenarioGraph->getActors().begin();
    while (!E.isEmpty()) {
        uint iterCnt = 0;
        ActorRef a = nullptr;
        while (true) // find an enabled actor
        {
            a = static_cast<ActorRef>(i->get());
            if (E.getValue(a) > 0 && gamma->actorEnabled(G->scenario, a)) {
                break;
            }
            if (++i == G->scenarioGraph->getActors().end()) {
                i = G->scenarioGraph->getActors().begin();
                iterCnt++;
                if (iterCnt > 1) {
                    throw CException("Graph is deadlocked.");
                }
            }
        }
        E.decreaseValue(a);
        staticSchedule[sch_idx++] = a;
        G->fire(gamma.get(), a);
    }
    lambda = gamma->normalize();
    auto gamma_prev = std::unique_ptr<State>(gamma->copy());

    // continue with fixed schedule
    bool equal = false;
    do {
        for (uint k = 0; k < sch_length; k++) {
            G->fire(gamma.get(), staticSchedule[k]);
        }
        lambda = gamma->normalize();
        gamma->smooth(gamma_prev.get());
        equal = gamma->equals(gamma_prev.get());
        gamma_prev = std::unique_ptr<State>(gamma->copy());
    } while (!equal);
    *eigenvalue = lambda;
    *eigenvector = gamma->asVector();
}

/**
 * convertEigenvector()
 * The function converts the time stamps in the eigenvector v1 of scenario
 * graph sg1 to an eigenvector for this scenario graph. The eigen vector
 * contains an entry for each initial token in the scenario graph. The
 * entries are ordered based on the channel ids (low to high).
 */
std::unique_ptr<MaxPlus::Vector> Exploration::convertEigenvector(ScenarioGraph *sg1,
                                                                 MaxPlus::Vector *v1) const {
    std::map<CString, std::vector<uint>> mapTokensSG1;
    std::map<CString, std::vector<uint>> mapTokensSG2;
    ScenarioGraph *sg2 = G->scenarioGraph;
    uint idx = 0;

    // Iterate over the channels in the scenario graph sg1 to build index
    idx = 0;
    for (const auto& c : sg1->getChannels()) {
        // Add index entry for each initial token in the channel c
        for (uint j = 0; j < c->getInitialTokens(); j++) {
            mapTokensSG1[c->getName()].push_back(idx++);
        }
    }

    // Iterate over the channels in the scenario graph sg1 to build index
    idx = 0;
    for (const auto& c : sg2->getChannels()) {
        // Add index entry for each initial token in the channel c
        for (uint j = 0; j < c->getInitialTokens(); j++) {
            mapTokensSG2[c->getName()].push_back(idx++);
        }
    }

    // Allocate memory for the eigen vector of scenario graph sg2
    auto v2 = std::make_unique<MaxPlus::Vector>(idx);

    // Initialize all entries to minus infinity (tokens initially available)
    for (uint i = 0; i < idx; i++) {
        v2->put(i, MP_MINUS_INFINITY);
    }

    // Create eigen vector for scenario graph sg2
    for (auto &[cName, value] : mapTokensSG2) {
        // Corresponding channel exists in sg1?
        auto j = mapTokensSG1.find(cName);

        // Channel exists in sg1?
        if (j != mapTokensSG1.end()) {
            // Check that both scenario graphs have same number of entries
            // for the channel (i.e. same number of initial tokens)
            if (value.size() != j->second.size()) {
                throw CException("Channel '" + cName
                                 + "' should have same "
                                   "number of initial tokens in all scenario "
                                   "graphs.");
            }

            // Copy time stamps from vector v1 to vector v2
            for (uint k = 0; k < value.size(); k++) {
                v2->put(value.at(k), v1->get(j->second.at(k)));
            }
        }
    }
    return v2;
}

/**
 * ActorCount()
 * Constructor.
 */
ActorCount::ActorCount(RepetitionVector &v) : RepetitionVector(v) {}

/**
 * isEmpty()
 * The function returns true when all actor counts are zero. Otherwise it
 * returns false.
 */
bool ActorCount::isEmpty() {
    return std::all_of(this->begin(), this->end(), [](const int i) { return i == 0; });
}

/**
 * decreaseValue()
 * Decrement the actor count of actor a.
 */
void ActorCount::decreaseValue(Actor *a) { at(a->getId())--; }

/**
 * getValue()
 * The function returns the actor count of actor a.
 */
uint ActorCount::getValue(Actor *a) { return at(a->getId()); }

/**
 * sum()
 * The function returns the sum of the actor counts.
 */
uint ActorCount::sum() {
    uint s = 0;

    for (const int i : *this) {
        s += i;
    }

    return s;
}

/**
 * SGraph()
 * Constructor.
 */
SGraph::SGraph(ScenarioGraph *g, const Scenario* s) : scenarioGraph(g), scenario(s), newTokenIndex(0) {
    // Compute repetition vector
    RepetitionVector E = scenario->computeRepetitionVector();
    repVec = std::make_unique<ActorCount>(E);

    // copy channels
    int stateIndex = 0;
    int historyIndex = 0;
    int k = 0;
    for (const auto& c : scenarioGraph->getChannels()) {
        Actor *a = c->getSrcActor();
        Actors::const_iterator i;
        unsigned int cRepetitionVectorEntry = 0;
        int m = 0;
        for (i = scenarioGraph->getActors().begin(); i != scenarioGraph->getActors().end();
             i++, m++) {
            const std::unique_ptr<Actor>& b = *i;
            if (a == b.get()) {
                synchStorage[c.get()] = c->getInitialTokens()
                                  + E.at(m) * c->getSrcPort()->getRateOfScenario(*scenario);
                cRepetitionVectorEntry = E.at(m) * c->getSrcPort()->getRateOfScenario(*scenario);
                break;
            }
            assert(i != scenarioGraph->getActors().end());
        }
        stateIndex += static_cast<int>(c->getInitialTokens());
        if (c->getInitialTokens() > cRepetitionVectorEntry) {
            historyIndex += static_cast<int>(c->getInitialTokens())
                            - static_cast<int>(cRepetitionVectorEntry);
        }
        k++;
    }

    this->stateSize = stateIndex;
    this->historySize = historyIndex;
}

/**
 * fire()
 */
void SGraph::fire(State *x, Actor *a) const {
    MPTime time;
    MPTime ftime;
    ftime = MP_MINUS_INFINITY;
    for (const auto& p : a->getPorts()) {
        if (p->getType() == Port::PortType::In) {
            time = x->consume(scenario, p.get());
            ftime = MP_MAX(time, ftime);
        }
    }
    time = ftime + MPTime(a->getExecutionTimeOfScenario(*scenario, a->getDefaultProcessorType()));
    for (const auto& p : a->getPorts()) {
        if (p->getType() == Port::PortType::Out) {
            x->produce(scenario, p.get(), time);
        }
    }
}

/**
 * repetitionVector()
 */
std::unique_ptr<ActorCount> SGraph::repetitionVector() {
    return std::make_unique<ActorCount>(*repVec);
}

/**
 * State()
 * Constructor.
 */
State::State(const SGraph &G) { init(G.scenarioGraph->getChannels().size()); }

/**
 * State()
 * Constructor.
 */
State::State(size_t sz) { init(sz); }

/**
 * init()
 * Initialize the state.
 */
void State::init(size_t sz) {
    this->table.resize(sz);
    for (uint i = 0; i < sz; i++) {
        this->table[i] = std::make_unique<TokenFIFO>(static_cast<uint>(sz));
    }
}

/**
 * ~State()
 * Destructor.
 */
State::~State() = default;

/**
 * actorEnabled()
 * The function returns true when the actor is enabled (i.e. ready to fire).
 * Otherwise the function returns false.
 */

bool State::actorEnabled(const Scenario *s, Actor *a) {
    // Check all ports using std::all_of
    return std::all_of(a->getPorts().begin(), a->getPorts().end(), [this, s](const auto& p) {
        // Only check input ports
        if (p->getType() == Port::PortType::In) {
            // Not enough tokens available?
            if (table[p->getChannel()->getId()]->size() < p->getRateOfScenario(*s)) {
                return false;
            }
        }
        return true;
    });
}

/**
 * zeroState()
 * Initialize this state as an initial state (i.e. channels contains only
 * initial tokens and no time has passed so far).
 */
std::unique_ptr<State> State::zeroState(const SGraph &G) {
    auto s = std::make_unique<State>(G);

    // Iterate over all channels
    for (const auto& c : G.scenarioGraph->getChannels()) {
        s->table[c->getId()] = std::make_unique<TokenFIFO>(G.synchStorage.at(c.get()));

        // Add a token to channel c with production time 0 for each
        // initial tokens which this channel contains
        for (uint j = 0; j < c->getInitialTokens(); j++) {
            s->table[c->getId()]->append(MPTime(0.0));
        }
    }

    // This is the initial state
    s->timestamp = MPTime(0.0);

    return s;
}

/**
 * fromVector()
 * Initialize this state as an initial state (i.e. channels contains only
 * initial tokens and no time has passed so far). The production times of the
 * initial tokens are taken from the vector v.
 */
std::unique_ptr<State> State::fromVector(const SGraph &G, const MaxPlus::Vector &v) {
    auto s = std::make_unique<State>(G);
    uint m = 0;

    for (const auto& c : G.scenarioGraph->getChannels()) {
        s->table[c->getId()] = std::make_unique<TokenFIFO>(G.synchStorage.at(c.get()));

        // Add a token to channel c with the production time given in the vector
        // v for each initial tokens which this channel contains
        for (uint j = 0; j < c->getInitialTokens(); j++) {
            s->table[c->getId()]->append(v.get(m++));
        }
    }

    // This is the initial state
    s->timestamp = MPTime(0.0);

    return s;
}

/**
 * zeroScheduleState()
 * Initialize this state (i.e. channels contains only initial tokens). The
 * production times of the initial tokens are equal to a counter which is
 * raised on each initial tokens that is placed in a FIFO.
 */
std::unique_ptr<State> State::zeroScheduleState(const SGraph &G) {
    auto s = std::make_unique<State>(G);
    int posCount = 0;

    for (const auto& c : G.scenarioGraph->getChannels()) {
        s->table[c->getId()] = std::make_unique<TokenFIFO>(G.synchStorage.at(c.get()));

        // Add a token to channel c with the production time equal to a counter
        // which is raised on each initial tokens that is placed in a FIFO.
        for (uint j = 0; j < c->getInitialTokens(); j++) {
            s->table[c->getId()]->append(MPTime(posCount++));
        }
    }

    return s;
}

/**
 * copy()
 * The function returns a copy of this state.
 */
std::unique_ptr<State> State::copy() {
    auto s = std::make_unique<State>(table.size());

    for (uint i = 0; i < table.size(); i++) {
        s->table[i] = table[i]->copy();
    }

    s->timestamp = timestamp;

    return s;
}

/**
 * smooth()
 * The function performs a smooth operation on all FIFOs.
 */
void State::smooth(State *y) {
    for (uint i = 0; i < table.size(); i++) {
        table[i]->smooth(*(y->table[i]));
    }
}

/**
 * hashValue()
 * The function returns a hash value for this state.
 */
size_t State::hashValue() const {
    size_t h = 0;

    constexpr size_t HashMultiplier = 31;

    for (const auto &i : table) {
        h = HashMultiplier * h + i->hashValue();
    }

    return h;
}

/**
 * equals()
 * The function performs an equals operation on all FIFOs.
 */
bool State::equals(const State *s) const {
    if (table.size() != s->table.size()) {
        return false;
    }

    for (uint i = 0; i < table.size(); i++) {
        if (!table[i]->equals(*(s->table[i]))) {
            return false;
        }
    }

    return true;
}

/**
 * compare()
 * The function implements a comparison to generate a total order
 * return -1 if receiver smaller than s,
 * 0 if equal, otherwise 1
 * performs a lexicographical order on all FIFOs.
 */
int State::compare(const State *s) const {
    if (this->table.size() != s->table.size()) {
        if (this->table.size() < s->table.size()) {
            return -1;
        }
        return 1;
    }

    for (uint i = 0; i < table.size(); i++) {
        int fc = 0;
        fc = table[i]->compare(*(s->table[i]));
        if (fc < 0) {
            return -1;
        }
        if (fc > 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * minus()
 * The function performs a minus operation on all FIFOs.
 */
std::unique_ptr<State> State::minus(const State &v) const {
    auto s = std::make_unique<State>(table.size());

    for (uint i = 0; i < table.size(); i++) {
        TokenFIFO* l = table[i].get();
        s->table[i] = l->minus(*(v.table[i]));
    }

    return s;
}

/**
 * add()
 * The function performs a add operation on all FIFOs.
 */
void State::add(double factor, State *v) {
    for (uint i = 0; i < table.size(); i++) {
        TokenFIFO* l = table[i].get();
        l->add(factor, *(v->table[i]));
    }
}

/**
 * firstToken()
 * The function returns a pointer to the first token in any of the FIFOs. When
 * all FIFOs are empty, the function returns NULL.
 */
Token *State::firstToken() {
    for (const auto &l : table) {
        if (l->size() > 0) {
            return l->first();
        }
    }
    return nullptr;
}

/**
 * normalize()
 * The function normalizes the production time fo the tokens in all FIFOs.
 */
MPDelay State::normalize() {
    MPDelay amount = norm();

    for (auto &i : table) {
        i->decreaseAll(amount);
    }

    timestamp += amount;

    return amount;
}

/**
 * norm()
 * The function returns the norm of the token production times.
 */
MPTime State::norm() {
    auto max = MPTime(0.0);

    for (uint i = 0; i < table.size(); i++) {
        if (i == 0) {
            max = table[i]->getMax();
        } else {
            max = MP_MAX(max, table[i]->getMax());
        }
    }

    return max;
}

/**
 * asVector()
 * The function returns the token production times as a MaxPlus vector.
 */
std::unique_ptr<MaxPlus::Vector> State::asVector() {
    auto v = std::make_unique<Vector>();
    uint k = 0;

    for (auto &i : table) {
        k = i->addToVector(*v, k);
    }

    return v;
}

/**
 * TokenFIFO()
 * Constructor.
 */
TokenFIFO::TokenFIFO(uint sz) : begin(0), end(0) {
    // Fifo should be able to hold sz tokens
    tokens.resize(sz + 1);
}

/**
 * copy()
 * The function returns a copy of the FIFO.
 */
std::unique_ptr<TokenFIFO> TokenFIFO::copy() { return std::make_unique<TokenFIFO>(*this); }

/**
 * hashValue()
 * The function returns a hash value for the FIFO.
 */
constexpr size_t HashMultiplier = 23;
size_t TokenFIFO::hashValue() const {
    size_t h = 0;

    uint i = begin;
    while (i != end) {
        h = HashMultiplier * h
            + static_cast<size_t>(static_cast<CDouble>(tokens[i].productionTime));
        i = (i + 1) % tokens.size();
    }

    return h;
}

/**
 * equals()
 * The function returns true when two FIFO are identical (i.e. same size and
 * same content).
 */
bool TokenFIFO::equals(const TokenFIFO &l) const {
    if (tokens.size() != l.tokens.size()) {
        return false;
    }

    uint i = begin;
    uint j = l.begin;

    while (i != end) {
        // Check equality of production times. Consider production times
        // equal when difference is less than some MP_EPSILON
        // handle -infinity separately
        const MPTime ti = tokens[i].productionTime;
        const MPTime tj = l.tokens[j].productionTime;
        if (MP_IS_MINUS_INFINITY(tj)) {
            if (!MP_IS_MINUS_INFINITY(ti)) {
                return false;
            }
        } else {
            if (MP_IS_MINUS_INFINITY(ti)) {
                return false;
            }

            if (fabs(static_cast<CDouble>(ti - tj)) > static_cast<CDouble>(MP_EPSILON)) {
                return false;
            }
        }
        ++i %= tokens.size();
        ++j %= tokens.size();
    }
    return true;
}

/**
 * compare()
 * impose a total ordering on FIFOs
 */
int TokenFIFO::compare(const TokenFIFO &l) const {
    if (tokens.size() < l.tokens.size()) {
        return -1;
    }
    if (tokens.size() > l.tokens.size()) {
        return 1;
    }

    uint i = begin;
    uint j = l.begin;

    while (i != end) {
        // Check equality of production times. Consider production times
        // equal when difference is less than some MP_EPSILON
        const MPTime ti = tokens[i].productionTime;
        const MPTime tj = l.tokens[j].productionTime;
        if (MP_IS_MINUS_INFINITY(tj)) {
            if (!MP_IS_MINUS_INFINITY(ti)) {
                return 1; // ti>tj
            }
        } else {
            if (MP_IS_MINUS_INFINITY(ti)) {
                return -1; // ti<tj
            }
            MPTime diff = ti - tj;
            if (-diff > MP_EPSILON) {
                return -1; // ti<tj
            }
            if (diff > MP_EPSILON) {
                return 1; // tj<ti
            }
        }
        // Next
        ++i %= tokens.size();
        ++j %= tokens.size();
    }
    return 0;
}

/**
 * minus()
 * The function returns a pointer to a new FIFO which is a copy of the tokens
 * in this FIFO, but with their production times lowered by the amount specified
 * in the FIFO m.
 */
std::unique_ptr<TokenFIFO> TokenFIFO::minus(const TokenFIFO &m) const {
    // Create a copy of the FIFO
    auto l = std::make_unique<TokenFIFO>(*this);

    uint i = begin;
    uint j = m.begin;
    while (i != end) {
        l->tokens[i].productionTime = tokens[i].productionTime - m.tokens[j].productionTime;
        i = (i + 1) % tokens.size();
        j = (j + 1) % tokens.size();
    }

    return l;
}

/**
 * add()
 * The function adds the token production times of the tokens in FIFO m to this
 * fifo. These token production times are however scaled with the supplied
 * factor.
 */
void TokenFIFO::add(double factor, const TokenFIFO &m) {
    uint i = begin;
    uint j = m.begin;

    while (i != end) {
        tokens[i].productionTime += factor * m.tokens[j].productionTime;
        i = (i + 1) % tokens.size();
        j = (j + 1) % tokens.size();
    }
}

/**
 * addToVector()
 * The function adds the token production times of all tokens which are stored
 * in this fifo to the vector v. The first token production time is placed at
 * position k in this vector. The function returns the position after which the
 * last addition to vector v has taken place.
 */
uint TokenFIFO::addToVector(MaxPlus::Vector &v, uint k) const {
    uint i = begin;

    while (i != end) {
        v.put(k++, tokens[i].productionTime);
        i = (i + 1) % tokens.size();
    }

    return k;
}

/**
 * first()
 * The function returns a pointer to the first token in the FIFO.
 */
Token *TokenFIFO::first() { return &(tokens[begin]); }

/**
 * decreaseAll()
 * The function decreases the production time of all tokens in the FIFO by the
 * specified amount.
 */
void TokenFIFO::decreaseAll(MPDelay amount) {
    uint i = begin;

    while (i != end) {
        tokens[i].productionTime -= amount;
        i = (i + 1) % tokens.size();
    }
}

/**
 * getMax()
 * The function returns the largest production time of the tokens in the FIFO.
 */
MPTime TokenFIFO::getMax() {
    auto max = MPTime(0.0);
    uint i = begin;

    while (i != end) {
        if (i == begin) {
            max = tokens[i].productionTime;
        } else {
            max = MP_MAX(max, tokens[i].productionTime);
        }
        i = (i + 1) % tokens.size();
    }

    return max;
}

/**
 * size()
 * The function returns the number of tokens in the FIFO.
 */
uint TokenFIFO::size() const {
    int r = static_cast<int>(end) - static_cast<int>(begin);

    if (r < 0) {
        r += static_cast<int>(tokens.size());
    }

    return static_cast<uint>(r);
}

/**
 * smooth()
 * Average the production time of the tokens in this fifo with the production
 * times in the fifo l.
 */
void TokenFIFO::smooth(const TokenFIFO &l) {
    uint i = begin;
    uint j = l.begin;

    constexpr double HalfWeight = 0.5;

    while (i != end) {
        tokens[i].productionTime =
                HalfWeight * tokens[i].productionTime + HalfWeight * l.tokens[j].productionTime;
        i = (i + 1) % tokens.size();
        j = (j + 1) % tokens.size();
    }
}

} // End namespace FSMSADF::MPExplore
