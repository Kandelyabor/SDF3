/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.h
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

#ifndef FSMSADF_ANALYSIS_MAXPLUS_MPEXPLORE_H_INCLUDED
#define FSMSADF_ANALYSIS_MAXPLUS_MPEXPLORE_H_INCLUDED

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "fsmsadf/base/graph.h"

#include "maxplus/algebra/maxplus.h" // IWYU pragma: keep

namespace FSMSADF::MPExplore {
// forward declarations
class State;

/**
 * class ActorCount
 * Associates with every actor a number, representing a number of
 * firings (e.g., a repetition vector).
 */
class ActorCount : public RepetitionVector {
public:
    // Constructor
    explicit ActorCount(RepetitionVector &v);

    // All counts are zero
    bool isEmpty();

    // Get actor count of actor a
    uint getValue(Actor *a);

    // Decrement actor count of actor a
    void decreaseValue(Actor *a);

    // Get sum of the actor counts
    uint sum();
};

/**
 * class SGraph
 * Decorates a scenario graph with information for MaxPlus analysis.
 */
class SGraph {
public:
    // Constructor
    SGraph(ScenarioGraph *g, const Scenario* s);

    // Get a copy of the repetition vector
    std::unique_ptr<ActorCount> repetitionVector();

    // Fire actor a while in state x
    inline void fire(State *x, Actor *a) const;

    // Some graph annotations...
    unsigned int stateSize;
    unsigned int historySize;
    unsigned int newTokenIndex;
    std::map<Channel *, unsigned int> synchStorage;

    // Scenario graph
    ScenarioGraph *scenarioGraph;

    // Scenario
    const Scenario* scenario;

private:
    // Repetition vector of the graph
    std::unique_ptr<ActorCount> repVec;
};

/**
 * class Token
 * Token represents a MaxPlus timestamped token in a graph.
 */
class Token {
public:
    // Constructor
    Token() { productionTime = MaxPlus::MPTime(0.0); };
    explicit Token(MaxPlus::MPTime t) : productionTime(t){};

    MaxPlus::MPTime productionTime;
};

/**
 * class TokenFIFO
 * A token fifo
 */
class TokenFIFO {
public:
    // Constructor
    explicit TokenFIFO(uint sz);

    /*
     * Read N timestamped tokens from the FIFO, return the max of all read
     * timestamps. To save time it is not checked whether the token contains
     * N tokens, only use this method if you are sure there are.
     */
    MaxPlus::MPTime removeFirstN(uint n) {
        MaxPlus::MPTime time;
        n--;
        Token *r = &(tokens[begin]);
        ++begin %= tokens.size();
        time = r->productionTime;
        while (n-- > 0) {
            Token *r = &(tokens[begin]);
            ++begin %= tokens.size();
            time = MP_MAX(r->productionTime, time);
        }
        return time;
    }

    /* Append n tokens to the FIFO, all timestamped with t. To save time it
     * is not checked whether there is space for the tokens, only use this
     * method if you are sure there is.
     */
    void append(MaxPlus::MPTime t, uint n = 1) {
        while (n-- > 0) {
            tokens[end].productionTime = t;
            ++end %= tokens.size();
        }
    }

    // Create a copy of this FIFO
    std::unique_ptr<TokenFIFO> copy();

    // Compute hash value for this FIFO
    [[nodiscard]] size_t hashValue() const;

    // Compare FIFOs for equality
    bool equals(const TokenFIFO& l) const;

    // Compare FIFOs
    int compare(const TokenFIFO& l) const;

    // Get a pointer to the first token in the FIFO
    Token *first();

    // Get the largest production time of the tokens in the FIFO
    MaxPlus::MPTime getMax();

    // Decrease the production time of all tokens in the FIFO
    void decreaseAll(MaxPlus::MPDelay amount);

    // Average the production times of the tokens with the times in FIFO l
    void smooth(const TokenFIFO& l);

    // Get the number of tokens stored in the FIFO
    [[nodiscard]] uint size() const;

    // Get a new FIFO where all current token production times are lowered
    // by the amount given in FIFO m
    [[nodiscard]] std::unique_ptr<TokenFIFO> minus(const TokenFIFO& m) const;

    // Add the tokens in FIFO m to this FIFO, but scale production times
    // with the supplied factor
    void add(double factor, const TokenFIFO& m);

    // Insert the token production times into the vector v
    uint addToVector(MaxPlus::Vector& v, uint k) const;

private:
    // The actual FIFO
    std::vector<Token> tokens;

    // Index which indicates begin and end of tokens in FIFO. The FIFO is
    // empty when these indices are equal.
    uint begin;
    uint end;
};

/**
 * class State
 * State represents a distribution of timestamped tokens over the channels
 * of the graph.
 */
class State {
public:
    // Constructor
    explicit State(const SGraph& G);
    explicit State(size_t sz);

    // Copy constructor
    State(const State& other);
    // Copy assignment operator
    State& operator=(const State& other);
    // Move constructor
    State(State&& other) noexcept;
    // Move assignment operator
    State& operator=(State&& other) noexcept;

    // Destructor
    ~State();

    // Create a copy of this state
    std::unique_ptr<State> copy();

    // Initialize the state
    static std::unique_ptr<State> zeroState(const SGraph& G);
    static std::unique_ptr<State> fromVector(const SGraph& G, const MaxPlus::Vector& v);
    static std::unique_ptr<State> zeroScheduleState(const SGraph& G);

    // Consume tokens from port p
    MaxPlus::MPTime consume(const Scenario *s, Port *p) {
        Channel *c = p->getChannel();
        TokenFIFO* l = table[c->getId()].get();
        return l->removeFirstN(p->getRateOfScenario(*s));
    }

    // Produce tokens on port p
    void produce(const Scenario *s, Port *p, MaxPlus::MPTime t) {
        Channel *c = p->getChannel();
        TokenFIFO* l = table[c->getId()].get();
        l->append(t, p->getRateOfScenario(*s));
    }

    // Check whether actor a is enabled
    bool actorEnabled(const Scenario *s, Actor *a);

    // Compute hash value of the state
    [[nodiscard]] size_t hashValue() const;

    // Compare the states using FIFO::equals() on each FIFO
    bool equals(const State *s) const;

    // Compare the states
    int compare(const State *s) const;

    // Perform a FIFO wise subtraction
    [[nodiscard]] std::unique_ptr<State> minus(const State& v) const;

    // Perform a FIFO wise addition
    void add(double factor, State *v);

    // Get a pointer to the first token in any FIFO
    Token *firstToken();

    // Normalize the production times of the tokens in the FIFOs
    MaxPlus::MPDelay normalize();

    // Get the norm of the production times of the tokens in the FIFOs
    MaxPlus::MPTime norm();

    // Smooth the production times of the tokens in the FIFOs
    void smooth(State *y);

    // Get the production times of the tokens in the FIFOs as a MP vector
    std::unique_ptr<MaxPlus::Vector> asVector();

private:
    // Initialize the FIFOs
    void init(size_t sz);

    // Time stamp of the state
    MaxPlus::MPTime timestamp;

    // Token FIFOs
    std::vector<std::unique_ptr<TokenFIFO>> table;
};

/**
 * class StateComparator
 * Compare two states for equality.
 */
class StateComparator {
public:
    bool operator()(const State *s1, const State *s2) const { return s1->compare(s2) == -1; }
};

/**
 * class StoredStates
 * Set of states.
 */
class StoredStates : std::set<State *, StateComparator> {
public:
    void store(std::unique_ptr<State> x) {
        insert(x.get());
        this->states.push_back(std::move(x)); // keep ptr for memory management
    }
    bool includes(State *x, State **y) {
        auto i = find(x);
        if (i != end()) {
            (*y) = (*i);
            return true;
        }
        return false;
    }
    bool includes(State *x) { return find(x) != end(); }
private:
    std::vector<std::unique_ptr<State>> states; // for memory management


};

/**
 * class Exploration
 * A MaxPlusExploration object encapsulates MaxPlus analysis algorithms.
 */
class Exploration {
public:
    std::unique_ptr<SGraph> G;
    Actor **staticSchedule;

    // Compute the minimal delay of a delay-period model for give period,
    // starting schedule and steady-state schedule according to the
    // algorithm presented in the paper "Synchronous Dataflow Scenarios".
    [[nodiscard]] MaxPlus::MPTime
    exploreDelayForSchedule(const MaxPlus::Vector& gamma0, const MaxPlus::Vector& gamma_s, MaxPlus::MPTime period) const;

    // Compute the eigenvector and eigenvalue of the MaxPlus matrix
    // associated with the scenario graph.
    void exploreEigen(std::unique_ptr<MaxPlus::Vector> *eigenvector, MaxPlus::MPTime *eigenvalue) const;

    // Convert the eigenvector v1 of scenario graph sg1 to an eigenvector
    // for this scenario graph.
    std::unique_ptr<MaxPlus::Vector> convertEigenvector(ScenarioGraph *sg1, MaxPlus::Vector *v1) const;
};

} // End namespace FSMSADF::MPExplore

#endif
