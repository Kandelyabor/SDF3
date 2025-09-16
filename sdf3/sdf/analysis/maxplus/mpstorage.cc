/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpstorage.cc
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

#include "mpstorage.h"
#include <memory>

namespace SDF::MaxPlusAnalysis {

using namespace GraphDecoration;
using namespace MaxPlus;

constexpr size_t HashConstant = 23;

State::State(const Graph& G) : count(0), size(0), table(nullptr) { this->initSize(G.channels.size()); }

State::State(size_t sz) : count(0), size(0), table(nullptr) { this->initSize(sz); }

void State::initSize(size_t sz) {
    size = sz;
    table = std::make_unique<std::vector<std::unique_ptr<TokenFIFO>>>(size);
    for (uint i = 0; i < sz; i++) {
        (*table)[i] = std::make_unique<TokenFIFO>(static_cast<int>(sz));
    }
}

State::~State() = default;

MPTime State::consume(const Port& p, int n) {
    auto time = MPTime(0);
    Channel *c = p.channel;
    std::unique_ptr<TokenFIFO>& l = table->at(c->index);
    for (int i = 0; i < n; i++) {
        MPTime t = l->removeFirst();
        if (i == 0) {
            time = t;
        } else {
            time = MAX(t, time);
        }
    }
    return time;
}

MPTime State::consume_back(const Port& p) {
    auto time = MPTime(0);
    Channel *c = p.channel;
    std::unique_ptr<TokenFIFO>& l = table->at(c->index);
    for (uint i = 0; i < p.rate; i++) {
        MPTime t;
        t = l->removeLast();
        if (i == 0) {
            time = t;
        } else {
            time = MIN(t, time);
        }
    }
    return time;
}

void State::produce_front(const Port& p, MPTime t) {
    Channel *c = p.channel;
    std::unique_ptr<TokenFIFO>& l = table->at(c->index);
    for (uint i = 0; i < p.rate; i++) {
        l->append_front(t);
    }
}

bool State::actorEnabled(const Actor& a) {
    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        const Port* p = i->get();
        if ((*(this->table))[p->channel->index]->size() < p->rate) {
            return false;
        }
    }
    return true;
}

bool State::actorReverseEnabled(const Actor& a) {
    for (auto i = a.outputPorts.begin(); i != a.outputPorts.end(); i++) {
        const Port* p = i->get();
        if ((this->table)->at(p->channel->index)->size() < p->rate) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<State> State::zeroState(const GraphDecoration::Graph& G) {
    auto s = std::make_unique<State>(G);
    ChannelList::const_iterator i;

    int k = 0;
    for (k = 0, i = G.channels.begin(); i != G.channels.end(); i++, k++) {
        const std::unique_ptr<Channel>& c = *i;
        (*(s->table))[k] = std::make_unique<TokenFIFO>(c->synchStorage);
        for (uint j = 0; j < c->nrInitialTokens; j++) {
            (*(s->table))[k]->append(MPTime(0.0));
        }
    }
    s->count = 0;
    s->timestamp = MPTime(0.0);

    return s;
}

std::unique_ptr<State> State::fromVector(const GraphDecoration::Graph& G, MaxPlus::Vector *v) {
    auto s = std::make_unique<State>(G);
    ChannelList::const_iterator i;

    unsigned int k = 0;
    unsigned int m = 0;
    for (k = 0, i = G.channels.begin(); i != G.channels.end(); i++, k++) {
        const std::unique_ptr<Channel>& c = *i;
        (*(s->table))[k] = std::make_unique<TokenFIFO>(c->synchStorage);
        for (uint j = 0; j < c->nrInitialTokens; j++) {
            (*(s->table))[k]->append(v->get(m++));
        }
    }
    s->count = 0;
    s->timestamp = MPTime(0.0);

    return s;
}

std::unique_ptr<State> State::zeroScheduleState(const GraphDecoration::Graph& G) {
    auto s = std::make_unique<State>(G);
    int posCount = 0;
    ChannelList::const_iterator i;
    int k = 0;
    for (k = 0, i = G.channels.begin(); i != G.channels.end(); i++, k++) {
        const Channel& c = *(*i);
        (*(s->table))[k] = std::make_unique<TokenFIFO>(c.synchStorage);
        for (uint j = 0; j < c.nrInitialTokens; j++) {
            (*(s->table))[k]->append(MPTime(posCount++));
        }
    }
    return s;
}

std::unique_ptr<State> State::copy() {
    auto s = std::make_unique<State>(this->size);
    unsigned int i = 0;
    for (i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = table->at(i);
        (*(s->table))[i] = l->copy();
    }
    s->timestamp = this->timestamp;
    s->count = this->count;
    return s;
}

void State::smooth(State& y) {
    unsigned int i = 0;
    for (i = 0; i < this->size; i++) {
        table->at(i)->smooth(*(y.table->at(i)));
    }
}

constexpr size_t HashMultiplier = 31;
size_t State::hashValue() const {
    size_t h = 0;
    for (uint i = 0; i < this->size; i++) {
        h = HashMultiplier * h + (*(this->table))[i]->hashValue();
    }
    return h;
}

bool State::compare(const State& s) const {
    // compare lexicographically
    if (this->size != s.size) {
        throw CException("storing states of different sizes");
    }
    for (uint i = 0; i < this->size; i++) {
        if ((*(this->table))[i]->compare(*(s.table->at(i)))) {
            return true;
        }
        if (s.table->at(i)->compare(*((*(this->table))[i]))) {
            return false;
        }
    }
    return false;
}

bool State::equals(const State& s) const {
    if (this->size != s.size) {
        return false;
    }
    for (uint i = 0; i < this->size; i++) {
        if (!(*(this->table))[i]->equals(*(s.table->at(i)))) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<State> State::minus(const State& v) const {
    auto s = std::make_unique<State>(this->size);
    unsigned int i = 0;
    for (i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = table->at(i);
        (*(s->table))[i] = l->minus(*(v.table->at(i)));
    }
    return s;
}

void State::add(double factor, const State& v) {
    unsigned int i = 0;
    for (i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = table->at(i);
        l->add(factor, *(v.table->at(i)));
    }
}

void State::maxWith(double offset, const State& v) {
    unsigned int i = 0;
    for (i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = table->at(i);
        l->maxWith(offset, *(v.table->at(i)));
    }
}

SymbolicState::SymbolicState(const GraphDecoration::Graph& G) :
    size(0),
    symbolicTokenSize(G.stateSize),
    table(nullptr),
    currentChannelIndex(0),
    currentTokenIndex(0) {
    this->initSize(G.channels.size());
}

SymbolicState::SymbolicState(size_t sz, size_t stSz) :
    size(0), symbolicTokenSize(stSz), table(nullptr), currentChannelIndex(0), currentTokenIndex(0) {
    this->initSize(sz);
}

void SymbolicState::initSize(size_t sz) {
    size = sz;
    table = std::make_unique<std::vector<std::unique_ptr<SymbolicTokenFIFO>>>(size);
}

SymbolicState::~SymbolicState() = default;

std::unique_ptr<SymbolicState> SymbolicState::zeroState(const GraphDecoration::Graph& G) {
    auto s = std::make_unique<SymbolicState>(G);
    ChannelList::const_iterator i;
    unsigned int k = 0;
    unsigned int n = 0;

    for (k = 0, i = G.channels.begin(); i != G.channels.end(); i++, k++) {

        const std::unique_ptr<Channel>& c = *i;

        (*(s->table))[k] = std::make_unique<SymbolicTokenFIFO>(c->synchStorage, G.stateSize);
        for (uint j = 0; j < c->nrInitialTokens; j++) {
            std::shared_ptr<SymbolicToken> t = SymbolicToken::initialToken(G.stateSize, n++);
            (*(s->table))[k]->append(t, 1);
        }
    }

    return s;
}

std::unique_ptr<SymbolicState> SymbolicStateEMR::zeroState(const GraphDecoration::Graph& G) {
    auto s = std::make_unique<SymbolicStateEMR>(G);
    ChannelList::const_iterator i;
    unsigned int k = 0;
    unsigned int n = 0;

    for (k = 0, i = G.channels.begin(); i != G.channels.end(); i++, k++) {
        const std::unique_ptr<Channel>& c = *i;
        auto f = std::make_unique<SymbolicTokenFIFOEMR>(c->synchStorage, G.stateSize);
        std::unique_ptr<SymbolicTokenFIFO> basePtr = std::move(f);
        s->setSymbolicTokenFIFO(k, basePtr);
        for (uint j = 0; j < c->nrInitialTokens; j++) {
            std::shared_ptr<SymbolicToken> t = std::move(SymbolicTokenEMR::initialToken(G.stateSize, n++));
            s->get(k)->append(t, 1);
        }
    }
    return s;
}

bool SymbolicState::actorEnabled(const Actor& a) {
    for (auto i = a.inputPorts.begin(); i != a.inputPorts.end(); i++) {
        Port& p = *(*i);
        if ((*(this->table))[p.channel->index]->size() < static_cast<int>(p.rate)) {
            return false;
        }
    }
    return true;
}

void SymbolicState::streamOn(std::ostream &s) {
    unsigned int i = 0;
    s << "SymbolicState<";
    for (i = 0; i < this->size; i++) {
        (*(this->table))[i]->streamOn(s);
        s << "; ";
    }
    s << ">";
}

std::unique_ptr<SymbolicToken> SymbolicState::firstToken() {
    this->currentChannelIndex = 0;
    this->currentTokenIndex = 0;
    return this->nextToken();
}

std::unique_ptr<SymbolicToken> SymbolicState::nextToken() {
    std::unique_ptr<SymbolicToken> result = nullptr;
    bool found = false;
    do {
        auto& f = this->table->at(this->currentChannelIndex);
        if (this->currentTokenIndex < static_cast<uint>(f->size())) {
            result = (f->peek(this->currentTokenIndex))->copy();
            this->currentTokenIndex++;
            found = true;
        } else {
            this->currentChannelIndex++;
            this->currentTokenIndex = 0;
        }
    } while (!found && this->currentChannelIndex < this->size);
    return result;
}

unsigned int SymbolicState::numberOfTokens() {
    unsigned int total = 0;
    for (unsigned int i = 0; i < this->size; i++) {
        total += (*(this->table))[i]->size();
    }
    return total;
}

Token *State::firstToken() {
    for (uint i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = this->table->at(i);
        if (l->size() > 0) {
            return l->first();
        }
    }
    return nullptr;
}

MPDelay State::normalize() {
    MPDelay amount = this->norm();
    for (uint i = 0; i < this->size; i++) {
        (*(this->table))[i]->decreaseAll(amount);
    }
    this->timestamp += amount;
    return amount;
}

MPTime State::norm() {
    auto max = MPTime(0.0);
    for (uint i = 0; i < this->size; i++) {
        if (i == 0) {
            max = (*(this->table))[i]->getMax();
        } else {
            max = MAX(max, (*(this->table))[i]->getMax());
        }
    }
    return max;
}

void State::streamOn(std::ostream &s) {
    for (uint i = 0; i < this->size; i++) {
        // s << i << ": ";
        std::unique_ptr<TokenFIFO>& l = this->table->at(i);
        l->streamOn(s);
    }
}

std::unique_ptr<std::vector<double>> State::asStdVector() {
    auto v = std::make_unique<std::vector<double>>();
    for (uint i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = this->table->at(i);
        l->addToStdVector(*v);
    }
    return v;
}

std::unique_ptr<MaxPlus::Vector> State::asVector() {
    unsigned int k = 0;
    auto v = std::make_unique<MaxPlus::Vector>();
    for (uint i = 0; i < this->size; i++) {
        std::unique_ptr<TokenFIFO>& l = this->table->at(i);
        k = l->addToVector(*v, k);
    }
    return v;
}

void TokenFIFO_Dynamic::streamOn(std::ostream &s) {
    TokenFIFO_Dynamic::const_iterator j;
    for (j = this->begin(); j != this->end(); j++) {
        const std::unique_ptr<Token>& t = *j;
        s << MPString(t->productionTime) << " ";
    }
}

void TokenFIFO_Dynamic::decreaseAll(MPDelay amount) {
    TokenFIFO_Dynamic::const_iterator j;
    for (j = this->begin(); j != this->end(); j++) {
        const std::unique_ptr<Token>& t = *j;
        t->productionTime = t->productionTime - amount;
    }
}

Token* TokenFIFO_Dynamic::first() { return (this->begin())->get(); }

MPTime TokenFIFO_Dynamic::removeFirst() {
    std::unique_ptr<Token>& t = this->front();
    this->pop_front();
    MPTime tm = t->productionTime;
    return tm;
}

void TokenFIFO_Dynamic::append(MPTime t) { this->push_back(std::make_unique<Token>(t)); }

std::unique_ptr<TokenFIFO_Dynamic> TokenFIFO_Dynamic::copy() {
    auto l = std::make_unique<TokenFIFO_Dynamic>();
    TokenFIFO_Dynamic::iterator i;
    for (i = this->begin(); i != this->end(); i++) {
        std::unique_ptr<Token>& t = *i;
        l->append(t->productionTime);
    }
    return l;
}

size_t TokenFIFO_Dynamic::hashValue() const {
    size_t h = 0;
    TokenFIFO_Dynamic::const_iterator i;
    for (i = this->begin(); i != this->end(); i++) {
        const std::unique_ptr<Token>& t = *i;
        h = HashConstant * h + static_cast<size_t>(static_cast<CDouble>(t->productionTime));
    }
    return h;
}

bool TokenFIFO_Dynamic::compare(TokenFIFO_Dynamic *l) const {
    if (this->size() < l->size()) {
        return true;
    }
    if (l->size() < this->size()) {
        return false;
    }
    TokenFIFO_Dynamic::const_iterator i;
    TokenFIFO_Dynamic::const_iterator j;
    for (i = this->begin(), j = l->begin(); i != this->end(); i++, j++) {
        const std::unique_ptr<Token>& ti = *i;
        const std::unique_ptr<Token>& tj = *j;
        if (ti->productionTime < tj->productionTime) {
            return true;
        }
        if (tj->productionTime < ti->productionTime) {
            return false;
        }
    }
    return false;
}

TokenFIFO_Static::TokenFIFO_Static(int sz) : asize(sz + 1), begin(0), end(0), tokens(sz+1) {
    // needs to be able to hold sz tokens

}

MPTime TokenFIFO_Static::removeFirst() {
    // assume there is a token to be read, don't check
    Token *r = &(tokens[begin]);
    begin = (begin + 1) % asize;
    return r->productionTime;
}

MPTime TokenFIFO_Static::removeLast() {
    // assume there is a token to be read, don't check
    end = (end == 0) ? asize - 1 : end - 1;
    Token *r = &(tokens[end]);
    return r->productionTime;
}

void TokenFIFO_Static::append(MPTime t) {
    // assume there is space, don't check
    tokens[end].productionTime = t;
    ++end %= asize;
}

void TokenFIFO_Static::append_front(MPTime t) {
    // assume there is space, don't check
    begin = (begin == 0) ? asize - 1 : begin - 1;
    tokens[begin].productionTime = t;
}

std::unique_ptr<TokenFIFO_Static> TokenFIFO_Static::copy() {
    auto l = std::make_unique<TokenFIFO_Static>(asize - 1);
    l->asize = this->asize;
    l->begin = this->begin;
    l->end = this->end;
    l->tokens = this->tokens;
    return l;
}

size_t TokenFIFO_Static::hashValue() const {
    size_t h = 0;
    uint i = this->begin;
    while (i != this->end) {
        h = HashConstant * h + static_cast<size_t>(static_cast<CDouble>(this->tokens[i].productionTime));
        i = (i + 1) % this->asize;
    }
    return h;
}

bool TokenFIFO_Static::compare(const TokenFIFO_Static& l) const {
    if (this->asize < l.asize) {
        return true;
    }
    if (l.asize < this->asize) {
        return false;
    }
    uint i = this->begin;
    uint j = l.begin;
    while (i != this->end) {
        if (this->tokens[i].productionTime < l.tokens[j].productionTime) {
            return true;
        }
        if (l.tokens[j].productionTime < this->tokens[i].productionTime) {
            return false;
        }
        i = (i + 1) % this->asize;
        j = (j + 1) % this->asize;
    }
    return false;
}

bool TokenFIFO_Static::equals(const TokenFIFO_Static& l) const {
    if (this->asize != l.asize) {
        return false;
    }
    uint i = this->begin;
    uint j = l.begin;
    while (i != this->end) {
        if (this->tokens[i].productionTime != l.tokens[j].productionTime) {
            return false;
        }
        ++i %= this->asize;
        ++j %= this->asize;
    }
    return true;
}

std::unique_ptr<TokenFIFO_Static> TokenFIFO_Static::minus(const TokenFIFO_Static& m) const {
    auto l = std::make_unique<TokenFIFO_Static>(asize - 1);
    l->asize = this->asize;
    l->begin = this->begin;
    l->end = this->end;
    l->tokens = this->tokens;
    uint i = this->begin;
    uint j = m.begin;
    while (i != this->end) {
        l->tokens[i].productionTime = this->tokens[i].productionTime - m.tokens[j].productionTime;
        i = (i + 1) % this->asize;
        j = (j + 1) % this->asize;
    }
    return l;
}

void TokenFIFO_Static::add(double factor, const TokenFIFO& m) {
    uint i = this->begin;
    uint j = m.begin;
    while (i != this->end) {
        this->tokens[i].productionTime += factor * m.tokens[j].productionTime;
        i = (i + 1) % this->asize;
        j = (j + 1) % this->asize;
    }
}

void TokenFIFO_Static::maxWith(double offset, const TokenFIFO& l) {
    uint i = this->begin;
    uint j = l.begin;
    while (i != this->end) {
        this->tokens[i].productionTime = MPTime(MAX(CDouble(this->tokens[i].productionTime),
                                                    offset + CDouble(l.tokens[j].productionTime)));
        i = (i + 1) % this->asize;
        j = (j + 1) % this->asize;
    }
}

void TokenFIFO::addToStdVector(std::vector<double>& v) const {
    uint i = this->begin;
    while (i != this->end) {
        v.push_back(static_cast<CDouble>(this->tokens[i].productionTime));
        i = (i + 1) % this->asize;
    }
}

unsigned int TokenFIFO::addToVector(MaxPlus::Vector& v, unsigned int k) const {
    uint i = this->begin;
    while (i != this->end) {
        v.put(k++, this->tokens[i].productionTime);
        i = (i + 1) % this->asize;
    }
    return k;
}

Token *TokenFIFO_Static::first() { return &(this->tokens[begin]); }

void TokenFIFO_Static::decreaseAll(MPDelay amount) {
    uint i = this->begin;
    while (i != this->end) {
        this->tokens[i].productionTime -= amount;
        i = (i + 1) % asize;
    }
}

MPTime TokenFIFO_Static::getMax() {
    auto max = MPTime(0.0);
    uint i = this->begin;
    while (i != this->end) {
        if (i == this->begin) {
            max = this->tokens[i].productionTime;
        } else {
            max = MAX(max, this->tokens[i].productionTime);
        }
        i = (i + 1) % asize;
    }
    return max;
}

void TokenFIFO_Static::streamOn(std::ostream &s) const {
    uint i = this->begin;
    while (i != this->end) {
        s << static_cast<CDouble>(this->tokens[i].productionTime) << " ";
        i = (i + 1) % asize;
    }
}

unsigned int TokenFIFO_Static::size() const {
    int r = static_cast<int>(this->end) - static_cast<int>(this->begin);
    if (r < 0) {
        r += static_cast<int>(asize);
}
    return static_cast<unsigned int>(r);
}

constexpr double Half = 0.5;
void TokenFIFO_Static::smooth(const TokenFIFO& l) {
    uint i = this->begin;
    uint j = l.begin;
    while (i != this->end) {
        this->tokens[i].productionTime =
                Half * this->tokens[i].productionTime + Half * l.tokens[j].productionTime;
        i = (i + 1) % this->asize;
        j = (j + 1) % this->asize;
    }
}

SymbolicTokenFIFO::SymbolicTokenFIFO(int fsz, int tsz) :
    asize(fsz + 1), begin(0), end(0), tokenSize(tsz), tokens(fsz+1) {
    // needs to be able to hold sz tokens
    std::fill(this->tokens.begin(), this->tokens.end(), nullptr);
}

SymbolicTokenFIFO::~SymbolicTokenFIFO() = default;

int SymbolicTokenFIFO::size() const {
    int r = this->end - this->begin;
    if (r < 0) {
        r += asize;
    }
    return r;
}

void SymbolicTokenFIFO::streamOn(std::ostream &s) {
    int i = this->begin;
    while (i != this->end) {
        tokens[i]->streamOn(s);
        i = (i + 1) % asize;
    }
}

std::unique_ptr<Token> Token::copy() { return makeToken(productionTime); }

std::unique_ptr<SymbolicToken> SymbolicToken::zeroToken(unsigned int sz) {
    auto t = std::make_unique<SymbolicToken>();
    for (unsigned int i = 0; i < sz; i++) {
        t->push_back(std::make_unique<Token>(MPTime(0.0)));
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicToken::minusInfinityToken(unsigned int sz) {
    auto t = std::make_unique<SymbolicToken>();
    for (unsigned int i = 0; i < sz; i++) {
        t->push_back(std::make_unique<Token>(MP_MINUS_INFINITY));
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicToken::initialToken(unsigned int sz, unsigned int n) {
    auto t = std::make_unique<SymbolicToken>();
    for (unsigned int i = 0; i < sz; i++) {
        if (i == n) {
            t->push_back(std::make_unique<Token>(MPTime(0.0)));
        } else {
            t->push_back(std::make_unique<Token>(MP_MINUS_INFINITY));
        }
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicTokenEMR::zeroToken(unsigned int sz) {
    auto t = std::make_unique<SymbolicTokenEMR>();
    for (unsigned int i = 0; i < sz; i++) {
        t->push_back(std::make_unique<TokenEMR>(MPTime(0.0)));
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicTokenEMR::minusInfinityToken(unsigned int sz) {
    auto t = std::make_unique<SymbolicTokenEMR>();
    for (unsigned int i = 0; i < sz; i++) {
        t->push_back(std::make_unique<TokenEMR>(MP_MINUS_INFINITY));
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicTokenEMR::initialToken(unsigned int sz, unsigned int n) {
    auto t = std::make_unique<SymbolicTokenEMR>();
    for (unsigned int i = 0; i < sz; i++) {
        if (i == n) {
            t->push_back(std::make_unique<TokenEMR>(MPTime(0.0)));
        } else {
            t->push_back(std::make_unique<TokenEMR>(MP_MINUS_INFINITY));
        }
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicToken::add(MPTime a) {
    std::unique_ptr<SymbolicToken> t = makeSymbolicToken();
    TokenList::iterator i;
    for (i = this->begin(); i != this->end(); i++) {
        t->push_back(makeToken((*i)->productionTime + a));
    }
    return t;
}

void SymbolicToken::streamOn(std::ostream &s) {
    TokenList::iterator i;
    s << "SymbolicToken<";
    for (i = this->begin(); i != this->end(); i++) {
        if ((*i)->productionTime >= MPTime(0.0)) {
            s << static_cast<CDouble>((*i)->productionTime);
        } else {
            s << "-infty";
        }
        s << ";";
    }
    s << ">";
}

std::unique_ptr<SymbolicToken> SymbolicToken::copy() {
    std::unique_ptr<SymbolicToken> t = makeSymbolicToken();
    TokenList::iterator i;
    for (i = this->begin(); i != this->end(); i++) {
        t->push_back(makeToken((*i)->productionTime));
    }
    return t;
}

std::unique_ptr<SymbolicToken> SymbolicToken::maxWith(const SymbolicToken& t) {
    std::unique_ptr<SymbolicToken> nt = makeSymbolicToken();
    TokenList::const_iterator i;
    TokenList::const_iterator j;
    for (i = this->begin(), j = t.begin(); i != this->end(); i++, j++) {
        nt->push_back(makeToken(MAX((*i)->productionTime, (*j)->productionTime)));
    }
    return nt;
}

std::unique_ptr<SymbolicToken> SymbolicTokenEMR::copy() {
    std::unique_ptr<SymbolicToken> c = SymbolicToken::copy();
    // add other stuff
    TokenList::iterator i;
    auto j = c->begin();
    for (i = this->begin(); i != this->end(); i++, j++) {
        auto* t = dynamic_cast<TokenEMR*>(i->get());
        auto* u = dynamic_cast<TokenEMR*>(j->get());
        if (!t->bufferSet.empty()) {
            u->bufferSet = t->bufferSet;
        }
    }
    return c;

    // a bit faster
    /*SymbolicToken *c = makeSymbolicToken();
    TokenList::iterator i;
    for (i = this->begin(); i != this->end(); i++)
    {
        TokenEMR* t = dynamic_cast<TokenEMR*> (*i);
        c->push_back(new TokenEMR((*i)->productionTime,t->bufferSet));
    }
    return c;*/
}
std::unique_ptr<SymbolicToken> SymbolicTokenEMR::maxWith(const SymbolicToken& t) {
    std::unique_ptr<SymbolicToken> max = SymbolicToken::maxWith(t);
    TokenList::iterator i;
    auto j = max->begin();
    auto k = t.begin();
    for (i = this->begin(); i != this->end(); i++, j++, k++) {
        auto *thisToken = dynamic_cast<TokenEMR*>(i->get());
        auto *maxToken = dynamic_cast<TokenEMR*>(j->get());
        auto *argToken = dynamic_cast<TokenEMR*>(k->get());

        if (!thisToken->bufferSet.empty()
            && maxToken->productionTime == thisToken->productionTime) {
            for (auto thisTokenBufferIter = thisToken->bufferSet.begin();
                 thisTokenBufferIter != thisToken->bufferSet.end();
                 thisTokenBufferIter++) {
                maxToken->bufferSet.insert(*thisTokenBufferIter);
            }
        }

        if (!argToken->bufferSet.empty() && maxToken->productionTime == argToken->productionTime) {
            for (auto argTokenBufferIter = argToken->bufferSet.begin();
                 argTokenBufferIter != argToken->bufferSet.end();
                 argTokenBufferIter++) {
                maxToken->bufferSet.insert(*argTokenBufferIter);
            }
        }
    }

    return max;
}

std::unique_ptr<SymbolicToken> SymbolicTokenEMR::add(MPTime a) {
    std::unique_ptr<SymbolicToken> added = SymbolicToken::add(a);
    // add other stuff
    TokenList::iterator i;
    auto j = added->begin();
    for (i = this->begin(); i != this->end(); i++, j++) {
        auto *t = dynamic_cast<TokenEMR*>(i->get());
        auto *u = dynamic_cast<TokenEMR*>(j->get());
        if (!t->bufferSet.empty()) {
            u->bufferSet = t->bufferSet;
        }
    }
    return added;
}

std::unique_ptr<Token> TokenEMR::makeToken(MaxPlus::MPTime t) {
    return std::make_unique<TokenEMR>(t);
}


} // namespace SDF::MaxPlusAnalysis
