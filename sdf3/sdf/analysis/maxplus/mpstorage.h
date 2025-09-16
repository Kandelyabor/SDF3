/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpstorage.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 13, 2008
 *
 *  Function        :   Max-Plus based state representations.
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

#ifndef MPSTORAGE_H_INCLUDED
#define MPSTORAGE_H_INCLUDED

#include "mpexplore.h"
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>

namespace SDF::MaxPlusAnalysis {

using namespace GraphDecoration;

class Token {
public:
    // factory methods
    virtual std::unique_ptr<Token> makeToken(MaxPlus::MPTime t) { return std::make_unique<Token>(t); }
    // Constructor
    Token() { productionTime = MaxPlus::MPTime(0.0); }
    explicit Token(MaxPlus::MPTime t) : productionTime(t) {  }

    // Copy constructor
    Token(const Token& other) = default;

    // Copy assignment operator
    Token& operator=(const Token& other) = default;

    // Move constructor
    Token(Token&& other) = delete;

    // Move assignment operator
    Token& operator=(Token&& other) = default;

    virtual ~Token() = default;
    // Copy
    virtual std::unique_ptr<Token> copy();

    MaxPlus::MPTime productionTime;
};

class TokenEMR : public Token {
public:
    // factory methods
     std::unique_ptr<Token> makeToken(MaxPlus::MPTime t) override;

    // Rule of five: explicitly delete or define copy/move constructors and assignment operators
    TokenEMR(const TokenEMR& other) = delete;
    TokenEMR& operator=(const TokenEMR& other) = delete;
    TokenEMR(TokenEMR&& other) = delete;
    TokenEMR& operator=(TokenEMR&& other) = delete;

     ~TokenEMR() override = default;

    explicit TokenEMR(MaxPlus::MPTime t) :
        Token(MaxPlus::MPTime(t)) //, bufferSet(new unordered_set <int>)
    {}

    /*TokenEMR(MaxPlus::MPTime t, std::unique_ptr<std::unordered_set <int>> bs) : Token(t),
    bufferSet(bs)
    {

    }*/

    TokenEMR(MaxPlus::MPTime t, std::unordered_set<int> bs) : Token(t), bufferSet(std::move(bs)) {}

    std::unique_ptr<Token> copy() override {
        std::unique_ptr<Token> t = Token::copy();
        auto tEmr = std::unique_ptr<TokenEMR>(dynamic_cast<TokenEMR*>(t.release()));
        tEmr->bufferSet = this->bufferSet;
        return tEmr;
    }

    // std::unique_ptr<unordered_set <int>> bufferSet;
    std::unordered_set<int> bufferSet;
};

// Switch between dynamic and static allocation of token FIFOs,
// but static performs best
#define TokenFIFO TokenFIFO_Static
// #define TokenFIFO TokenFIFO_Dynamic

/**
 * class TokenFIFO_Dynamic
 * A dynamically managed token fifo based on std:list
 */
class TokenFIFO_Dynamic : public std::list<std::unique_ptr<Token>> {
public:
    MaxPlus::MPTime removeFirst();
    void append(MaxPlus::MPTime t);
    std::unique_ptr<TokenFIFO_Dynamic> copy();
    [[nodiscard]] size_t hashValue() const;
    bool compare(TokenFIFO_Dynamic *l) const;
    Token* first();
    void decreaseAll(MaxPlus::MPDelay amount);
    void streamOn(std::ostream &s);
};

/**
 * class TokenFIFO_Static
 * A statically managed token fifo
 */
class TokenFIFO_Static {
public:
    explicit TokenFIFO_Static(int sz);
    MaxPlus::MPTime removeFirst();

    /*
     * Read N timestamped tokens from the FIFO, return the max of all read
     * timestamps. To save time it is not checked whether the token contains
     * N tokens, only use this method if you are sure there are.
     */
    MaxPlus::MPTime removeFirstN(unsigned int n) {
        MaxPlus::MPTime time;
        n--;
        Token *r = &(tokens[begin]);
        ++begin %= asize;
        time = r->productionTime;
        while (n-- > 0) {
            Token *r = &(tokens[begin]);
            ++begin %= asize;
            time = (std::max)(r->productionTime, time);
        }
        return time;
    }

    MaxPlus::MPTime removeLast();
    void append(MaxPlus::MPTime t);

    /* Append n tokens to the FIFO, all timestamped with t. To save time it is
     * not checked whether there is space for the tokens, only use this method
     * if you are sure there is.
     */
    void append(MaxPlus::MPTime t, unsigned int n) {
        while (n-- > 0) {
            tokens[end].productionTime = t;
            ++end %= asize;
        }
    }

    void append_front(MaxPlus::MPTime t);
    std::unique_ptr<TokenFIFO_Static> copy();
    [[nodiscard]] size_t hashValue() const;
    [[nodiscard]] bool compare(const TokenFIFO_Static& l) const;
    Token *first();
    MaxPlus::MPTime getMax();
    void decreaseAll(MaxPlus::MPDelay amount);
    void smooth(const TokenFIFO_Static& l);
    void streamOn(std::ostream &s) const;
    [[nodiscard]] unsigned int size() const;
    [[nodiscard]] bool equals(const TokenFIFO_Static& l) const;
    [[nodiscard]] std::unique_ptr<TokenFIFO_Static> minus(const TokenFIFO_Static& m) const;
    void add(double factor, const TokenFIFO_Static& m);
    void maxWith(double offset, const TokenFIFO_Static& l);
    void addToStdVector(std::vector<double>& v) const;
    unsigned int addToVector(MaxPlus::Vector& v, unsigned int k) const;

private:
    std::vector<Token> tokens;
    unsigned int asize; // size of the array
    unsigned int begin;
    unsigned int end;
};

/**
 * class State
 * State represents a distribution of timestamped tokens over the channels of
 * the SDFG
 */
class State {
public:
    explicit State(const Graph& G);
    explicit State(size_t sz);
    ~State();

    // Prevent copying and moving
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State(State&&) = delete;
    State& operator=(State&&) = delete;

    std::unique_ptr<State> copy();
    static std::unique_ptr<State> zeroState(const Graph& G);
    static std::unique_ptr<State> fromVector(const Graph& G, MaxPlus::Vector *v);
    static std::unique_ptr<State> zeroScheduleState(const Graph& G);
    MaxPlus::MPTime consume(const Port& p) {
        Channel* c = p.channel;
        std::unique_ptr<TokenFIFO>& l = table->at(c->index);
        return l->removeFirstN(p.rate);
    }
    MaxPlus::MPTime consume(const InPort& ip) {
        InputChannel *ic = ip.inputChannel;
        std::unique_ptr<TokenFIFO>& l = table->at(ic->index);
        return l->removeFirstN(ip.rate);
    }
    MaxPlus::MPTime consume(const Port& p, int n);
    MaxPlus::MPTime consume_back(const Port& p);
    void produce(const Port& p, MaxPlus::MPTime t) {
        Channel *c = p.channel;
        std::unique_ptr<TokenFIFO>& l = table->at(c->index);
        l->append(t, p.rate);
    }
    void produce_front(const Port& p, MaxPlus::MPTime t);
    bool actorEnabled(const Actor& a);
    bool actorReverseEnabled(const Actor& a);
    [[nodiscard]] size_t hashValue() const;
    [[nodiscard]] bool compare(const State& s) const;
    [[nodiscard]] bool equals(const State& s) const;
    [[nodiscard]] std::unique_ptr<State> minus(const State& v) const;
    void add(double factor, const State& v);
    void maxWith(double offset, const State& v);
    Token *firstToken();
    MaxPlus::MPDelay normalize();
    MaxPlus::MPTime norm();
    void smooth(State& y);
    void streamOn(std::ostream &s);
    std::unique_ptr<std::vector<double>> asStdVector();
    std::unique_ptr<MaxPlus::Vector> asVector();
    MaxPlus::MPTime timestamp;
    unsigned int count;

private:
    size_t size;
    std::unique_ptr<std::vector<std::unique_ptr<TokenFIFO>>> table;
    void initSize(size_t sz);
};

using StateRef = State*;

/**
 * class TokenList
 */
class TokenList : public std::vector<std::unique_ptr<Token>> {
public:
    // Prevent copying and moving
    TokenList(const TokenList&) = delete;
    TokenList& operator=(const TokenList&) = delete;
    TokenList(TokenList&&) = delete;
    TokenList& operator=(TokenList&&) = delete;

    // Destructor
    TokenList() = default;
    virtual ~TokenList() = default;
};

/**
 * class SymbolicToken
 SymbolicToken class implements the symbolic time stamps. It is basically a vector of pointers to
 the Token class which stores the production time of tokens.
 */
class SymbolicToken : public TokenList {
public:
    SymbolicToken() = default;
    // factory methods
    virtual std::unique_ptr<SymbolicToken> makeSymbolicToken() { return std::make_unique<SymbolicToken>(); }

    virtual std::unique_ptr<Token> makeToken(MaxPlus::MPTime t) { return std::make_unique<Token>(t); }

    static std::unique_ptr<SymbolicToken> zeroToken(unsigned int sz);
    static std::unique_ptr<SymbolicToken> minusInfinityToken(unsigned int sz);
    static std::unique_ptr<SymbolicToken> initialToken(unsigned int sz, unsigned int n);
    virtual std::unique_ptr<SymbolicToken> copy();
    virtual std::unique_ptr<SymbolicToken> maxWith(const SymbolicToken& t);
    virtual std::unique_ptr<SymbolicToken> add(MaxPlus::MPTime a);
    void streamOn(std::ostream &s);
};

class SymbolicTokenEMR : public SymbolicToken {
public:
    // factory methods
     std::unique_ptr<SymbolicToken> makeSymbolicToken() override { return std::make_unique<SymbolicTokenEMR>(); }

    std::unique_ptr<Token> makeToken(MaxPlus::MPTime t) override { return std::make_unique<TokenEMR>(t); }

    static std::unique_ptr<SymbolicToken> zeroToken(unsigned int sz);
    static std::unique_ptr<SymbolicToken> minusInfinityToken(unsigned int sz);
    static std::unique_ptr<SymbolicToken> initialToken(unsigned int sz, unsigned int n);
    std::unique_ptr<SymbolicToken> copy() override;
    std::unique_ptr<SymbolicToken> maxWith(const SymbolicToken& t) override;
    std::unique_ptr<SymbolicToken> add(MaxPlus::MPTime a) override;
};

/**
 * class SymbolicTokenFIFO
 SymbolicTokenFIFO class implements a circular buffer with an array of fixed size. It stores
 pointers to SymbolicToken objects. These pointers are initially set to nullptrs when
 SymbolicTokenFIFO is initialized. Each SymbolicToken that is produced to (consumed form) a
 SymbolicTokenFIFO, increments the end(begin) indicator. In fact, the end(begin) indicator points to
 the element of the array that the elements before (after and including) that store non-null
 SymbolicToken pointers. When an actor completes a firing, a SymbolicToken object is created by
 calling add(MaxPlus::MPTime exeTime) which adds the execution time of the actor to the start time
 of the firing and creates a new SymbolicToken object. All tokens that are produced by this firing
 should be able to access the created symbolicToken, because it will be required when the produced
 tokens are being consumed. The access can be done either by copying the SymbolicToken pointer or by
 deep copying the SymbolicToken object. Deep copying the symbolicTokens may lead to huge memory
 consumption during runtime because actors might have large output rates. On the other hand, it
 makes the memory management easy, because once a token is consumed, the correspondent symbolicToken
 object can be removed. In contrast, copying just the pointer minimizes the memory consumption, but
 it makes the memory management very difficult. If the actor has several outgoing channels, then
 pointer to the created symbolicToken  will be copied to several SymbolicTokenFIFOs. Since the
 tokens are consumed from different FIFOs with different consumption rates, it makes it very
 difficult to predict the life time of the created symbolicToken. //Smart pointers might be helpful
 in such a circumstances.

 The current implementation takes advantage of both aspects. It does a deep copy of the created
 SymbolicToken to each SymbolicTokenFIFO of the outgoing channels. Then it copies the pointer to the
 copied SymbolicToken within each SymbolicTokenFIFO. Note that we still need to remove the copies of
 the created SymbolicToken when we notice their last usage. To do so, we take advantage of the fact
 that the SymbolicToken  pointer copies are all consecutively stored in a SymbolicTokenFIFO . This
 means we can always find the last usage of the pointer by comparing the pointer with the next
 pointer in a FIFO channel. If they are not pointing to the same object or if the next pointer is a
 nullptr, then the last pointer to the object has been found and the copy object can be removed. If
 they are pointing to the same object, then the pointers are set to nullptr but the object is not
 removed. Note that when the SymbolicTokenFIFO objects are destroyed, the same methodology is used
 to remove the SymbolicToken objects remaining on the SymbolicTokenFIFO.
 */
class SymbolicTokenFIFO {
public:
    SymbolicTokenFIFO(int fsz, int tsz);
    virtual ~SymbolicTokenFIFO();

    // Prevent copying and moving
    SymbolicTokenFIFO(const SymbolicTokenFIFO&) = delete;
    SymbolicTokenFIFO& operator=(const SymbolicTokenFIFO&) = delete;
    SymbolicTokenFIFO(SymbolicTokenFIFO&&) = delete;
    SymbolicTokenFIFO& operator=(SymbolicTokenFIFO&&) = delete;

    void append(std::shared_ptr<SymbolicToken>& t, int n) {
        // assume there is space, don't check
        while (n-- > 0) {
            tokens[end] = t;
            // SymbolicTokenFIFO is initially designed to be a circular buffer. However in the
            // current implementation, asize is an upper bound on the number of tokens that can
            // accumulate in the FIFO. Therefore end can be equal to the asize in the worst case and
            // the buffer never circulates. This means the mode operation is currently redundant
            // and may be removed in the future if we dont see a use of this class as a circular
            // buffer.
            ++end %= asize;
        }
    }
    std::shared_ptr<SymbolicToken> removeFirstN(int n) {
        // check if n is 0
        if (n == 0) {
            return makeMinusInfinityToken(this->tokenSize);
        }

        // assume there is a token to be read, don't check
        std::shared_ptr<SymbolicToken> time = nullptr;
        SymbolicToken* t = nullptr;
        std::shared_ptr<SymbolicToken> oldTime = nullptr;

        n--;
        time = tokens[begin];
        auto *prevTime = time.get();

        tokens[begin] = nullptr;

        ++begin %= asize;

        while (n-- > 0) {
            t = tokens[begin].get();
            if (prevTime != t) {
                // only if we haven't already processed this token
                oldTime = time;
                time = t->maxWith(*oldTime);
            }

            tokens[begin] = nullptr;
            ++begin %= asize;
        }
        return time;
    }
    SymbolicToken* peek(unsigned int n) { return tokens[(begin + n) % asize].get(); }
    [[nodiscard]] int size() const;
    void streamOn(std::ostream &s);
    virtual std::unique_ptr<SymbolicToken> makeMinusInfinityToken(unsigned int s) {
        return SymbolicToken::minusInfinityToken(s);
    }

private:
    std::vector<std::shared_ptr<SymbolicToken>> tokens;
    int asize; // size of the array
    int begin;
    int end;
    int tokenSize; // size of a single symbolic token in the fifo
                   // positions starting from begin up to but not including end are filled
};

class SymbolicTokenFIFOEMR : public SymbolicTokenFIFO {
public:
    SymbolicTokenFIFOEMR(int fsz, int tsz) : SymbolicTokenFIFO(fsz, tsz) {}
    std::unique_ptr<SymbolicToken> makeMinusInfinityToken(unsigned int s) override {
        return SymbolicTokenEMR::minusInfinityToken(s);
    }
};

/**
 * class SymbolicState
 */
class SymbolicState {
public:
    explicit SymbolicState(const Graph& G);
    SymbolicState(size_t sz, size_t stSz);
    virtual ~SymbolicState();

    // Prevent copying and moving
    SymbolicState(const SymbolicState&) = delete;
    SymbolicState& operator=(const SymbolicState&) = delete;
    SymbolicState(SymbolicState&&) = delete;
    SymbolicState& operator=(SymbolicState&&) = delete;

    static std::unique_ptr<SymbolicState> zeroState(const Graph& G);
    virtual std::shared_ptr<SymbolicToken> consume(const Port& p) {
        Channel *c = p.channel;
        SymbolicTokenFIFO* f = table->at(c->index).get();
        return f->removeFirstN(static_cast<int>(p.rate));
    }
    void produce(const Port& p, std::shared_ptr<SymbolicToken> t) {
        Channel *c = p.channel;
        SymbolicTokenFIFO* f = table->at(c->index).get();
        f->append(t, static_cast<int>(p.rate));
    }
    bool actorEnabled(const Actor&);
    void streamOn(std::ostream &s);
    [[nodiscard]] size_t getSize() const { return size; }
    [[nodiscard]] size_t getSymbolicTokenSize() const { return symbolicTokenSize; }
    const std::unique_ptr<SymbolicTokenFIFO>& get(unsigned int i) { return table->at(i); }
    // to support iterating over all (initial tokens)
    unsigned int numberOfTokens();
    std::unique_ptr<SymbolicToken> firstToken();
    std::unique_ptr<SymbolicToken> nextToken();
    void setSymbolicTokenFIFO(unsigned int k, std::unique_ptr<SymbolicTokenFIFO>& f) {
        (*(this->table))[k] = std::move(f);
    }

private:
    size_t size;
    size_t symbolicTokenSize;
    std::unique_ptr<std::vector<std::unique_ptr<SymbolicTokenFIFO>>> table;
    void initSize(size_t sz);

    // state to support iterating over all tokens
    // make nicer implementation later...
    unsigned int currentChannelIndex;
    unsigned int currentTokenIndex;
};

class SymbolicStateEMR : public SymbolicState {
public:
    explicit SymbolicStateEMR(const Graph& G) : SymbolicState(G) {}
    SymbolicStateEMR(size_t sz, size_t stSz) : SymbolicState(sz, stSz) {}
    static std::unique_ptr<SymbolicState> zeroState(const Graph& G);

    std::shared_ptr<SymbolicToken> consume(const Port& p) override {
        Channel *c = p.channel;
        std::shared_ptr<SymbolicToken> res = SymbolicState::consume(p);

        // if c is a storage channel add it to all non minus infty tokens
        if (c->isStorageChannel) {
            for (auto resIter = res->begin(); resIter != res->end(); resIter++) {
                const std::unique_ptr<Token>& t = *resIter;
                if (t->productionTime != MaxPlus::MP_MINUS_INFINITY) {
                    /*std::unique_ptr<std::unordered_set<int>> bs =
                    std::make_unique<std::unordered_set<int>>(); bs->insert(c->index); TokenEMR*
                    tEMR = dynamic_cast<TokenEMR*>(t); tEMR->bufferSet = bs;*/

                    auto* tEMR = dynamic_cast<TokenEMR*>(t.get());
                    tEMR->bufferSet.insert(static_cast<int>(c->index));
                }
            }
        }

        return res;
    }
};

/**
 * class StateComparator
 */
class StateComparator {
public:
    bool operator()(const std::unique_ptr<State>& s1, const std::unique_ptr<State>& s2) const { return s1->compare(*s2); }
};

/**
 * class StoredStates
 */
class StoredStates : std::set<std::unique_ptr<State>, StateComparator> {
public:
    void store(std::unique_ptr<State> x) { this->insert(std::move(x)); }
    bool includes(const std::unique_ptr<State>& x, StateRef *y) {
        StoredStates::const_iterator i;
        i = this->find(x);
        if (i != this->end()) {
            (*y) = (*i).get();
            return true;
        }
        return false;
    }
    bool includes(std::unique_ptr<State>& x) { return this->find(x) != this->end(); }
    uint64_t size() const { return this->std::set<std::unique_ptr<State>, StateComparator>::size(); }
};

} // namespace SDF::MaxPlusAnalysis

#endif
