/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dependency_graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 20, 2006
 *
 *  Function        :   Find the abstract dependency graph for an SDFG
 *
 *  History         :
 *      20-06-06    :   Initial version.
 *
 * $Id: dependency_graph.cc,v 1.1.2.6 2010-08-16 13:26:12 mgeilen Exp $
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

#include "base/exception/exception.h"
#include "sdf/base/algo/components.h"
#include "sdf/base/timed/graph.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <ctime> // IWYU pragma: keep
#include <iostream>
#include <memory>

#ifndef _MSC_VER
#include <sys/resource.h>
#endif

#include "sdf/base/algo/repetition_vector.h"

#define TDTIME_MAX INT_MAX

namespace SDF {

using TCnt = uint16_t;
using TBufSize = uint32_t;
using TDtime = double;

namespace {

// NOLINTBEGIN(*global-variables)

TimedSDFgraph *g;
SDFactor *outputActor;
SDFdiscreteTime SDF_MAX_TIME;
TCnt outputActorRepCnt;

// NOLINTEND(*global-variables)

SDFactorRef findOutputActor(const TimedSDFgraph& g, RepetitionVector repVec) {
    int min = INT_MAX;
    SDFactorRef a = nullptr;

    for (const auto& a : g.getActors()) {
        min = (std::min)(repVec[a->getId()], min);
    }
    return a;
}


SDFtime getMaxExecTime(const TimedSDFgraph& g) {
    SDFtime max = 0;

    for (const auto& iter : g.getActors()) {
        const auto *a = dynamic_cast<TimedSDFactor*>(iter.get());

        max = (std::max)(a->getExecutionTime(), max);
    }

    return max;
}

SDFdiscreteTime getMaxDiscreteExecTime(const TimedSDFgraph& g) {
    SDFdiscreteTime max = 0;

    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        max = (std::max)(a->getDiscreteExecutionTime(), max);
    }

    return max;
}


} // namespace

/******************************************************************************   * State
 *****************************************************************************/
using TBufferSizes = std::unique_ptr<TBufSize[]>;
using TCounts = std::unique_ptr<TCnt[]>;
using TActorCounts = std::unique_ptr<TCounts[]>;

using State = struct State {
    TActorCounts act_clk;
    TDtime glb_clk{};
    TBufferSizes ch;
};

namespace {

/**
 * clearState ()
 * The function sets the state to zero.
 */
void clearState(State &s) {
    for (uint i = 0; i < g->nrChannels(); i++) {
        s.ch[i] = 0;
    }

    s.glb_clk = 0;

    for (uint i = 0; i < g->nrActors(); i++) {
        for (TCnt t = 0; t < static_cast<TCnt>(SDF_MAX_TIME + 1); t++) {
            s.act_clk[i][t] = 0;
        }
    }
}

/**
 * equalStates ()
 * The function compares to states and returns true if they are equal.
 */
inline bool equalStates(const State &s1, const State &s2) {
    if (s1.glb_clk != s2.glb_clk) {
        return false;
    }

    for (uint i = 0; i < g->nrChannels(); i++) {
        if (s1.ch[i] != s2.ch[i]) {
            return false;
        }
    }

    for (uint i = 0; i < g->nrActors(); i++) {
        for (uint t = 0; t < SDF_MAX_TIME + 1; t++) {
            if (s1.act_clk[i][t] != s2.act_clk[i][t]) {
                return false;
            }
        }
    }

    return true;
}

/**
 * copyState ()
 * The function copies the state.
 */
inline void copyState(State &to, const State &from) {
    to.glb_clk = from.glb_clk;

    for (uint i = 0; i < g->nrChannels(); i++) {
        to.ch[i] = from.ch[i];
    }

    for (uint i = 0; i < g->nrActors(); i++) {
        for (uint t = 0; t < SDF_MAX_TIME + 1; t++) {
            to.act_clk[i][t] = from.act_clk[i][t];
        }
    }
}

/**
 * createState ()
 * The function allocates memory for a new state.
 */
void createState(State &s) {
    s.ch = std::make_unique<TBufSize[]>(g->nrChannels());

    s.act_clk = std::make_unique<TCounts[]>(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++) {
        s.act_clk[i] = std::make_unique<TCnt[]>(SDF_MAX_TIME + 1);
    }
}


/******************************************************************************
 * Stack
 *****************************************************************************/

/**
 * StackPos
 * Index into stack.
 */
using StackPos = int;
using TStack = std::unique_ptr<State[]>;

// NOLINTBEGIN(*global-variables)

/**
 * stack
 * The stack...
 */
TStack stack;

/**
 * stackPtr
 * Index into stack of the first free space.
 */
StackPos stackPtr;
StackPos maxStackPtr;
StackPos STACK_SIZE;

// NOLINTEND(*global-variables)

/**
 * resizeStack ()
 * Enlarge the stack with a factor of 2.
 */
void resizeStack() {
    StackPos newStackSize = 2 * STACK_SIZE;

    // Allocate new stack
    auto newStack = std::make_unique<State[]>(newStackSize);

    // Copy old stack to new stack
    for (StackPos p = 0; p < STACK_SIZE; p++) {

        newStack[p] = std::move(stack[p]);
    }

    // Create state for new part of stack
    for (StackPos p = STACK_SIZE; p < newStackSize; p++) {
        createState(newStack[p]);
    }

    // Cleanup old stack
    stack.reset();

    // Put new stack in place
    stack = std::move(newStack);
    STACK_SIZE = newStackSize;
}

/**
 * createStack ()
 * The function creates a stack.
 */
void createStack() {
    stack = std::make_unique<State[]>(STACK_SIZE);

    if (stack == nullptr) {
        throw CException("Failed creating stack.");
    }

    for (int i = 0; i < STACK_SIZE; i++) {
        createState(stack[i]);
    }

    maxStackPtr = 0;
}

/**
 * clearStack ()
 * Reset the stack to an empty state.
 */
void clearStack() { stackPtr = 0; }

/**
 * popStack ()
 * Remove last element from the stack.
 */
inline void popStack() {
    if (stackPtr > 0) {
        stackPtr--;
    }
}

/**
 * pushStack ()
 * Put an element on the stack.
 */
inline void pushStack(const State &s) {
    if (stackPtr >= STACK_SIZE) {
        std::cerr << "[INFO] buffy resizing stack." << std::endl;
        resizeStack();
    }

    copyState(stack[stackPtr], s);
    stackPtr++;

    maxStackPtr = (std::max)(stackPtr, maxStackPtr);
}

/**
 * topStack ()
 * The function returns the last element of the stack.
 */
inline State &topStack() { return stack[stackPtr - 1]; }

/******************************************************************************
 * Hash
 *****************************************************************************/
#define INVALID_HASH_KEY nullptr
#define NO_SLOT_IN_HASH (-1)

/**
 * HashKey
 *
 */
using HashKey = unsigned int;

/**
 * HashSlot
 *
 */
using HashSlot = struct HashSlot {
    StackPos value{};
    std::unique_ptr<struct HashSlot> next;
};

using THashTable = std::unique_ptr<std::unique_ptr<HashSlot>[]>;

/**
 * hashTable
 * The hash table...
 */
// NOLINTBEGIN(*global-variables)
THashTable hashTable = nullptr;
HashKey HASH_TABLE_SIZE;
// NOLINTEND(*global-variables)

/**
 * createHashTable ()
 * The function constructs a hash table.
 */
void createHashTable() {
    hashTable = std::make_unique<std::unique_ptr<HashSlot>[]>(HASH_TABLE_SIZE);

    if (hashTable == nullptr) {
        throw CException("Failed creating hash table.");
    }

    for (uint i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = INVALID_HASH_KEY;
    }
}

/**
 * clearHashTable ()
 * Resets the hash table to contain no keys.
 */
void clearHashTable() {

    for (uint i = 0; i < HASH_TABLE_SIZE; i++) {
        auto& s_cur = hashTable[i];
        while (s_cur != INVALID_HASH_KEY) {
            std::unique_ptr<HashSlot> nxt = std::move(s_cur->next);
            s_cur.reset();
            s_cur = std::move(nxt);
        }

        hashTable[i] = INVALID_HASH_KEY;
    }
}

// Note: name of function changed from hash to getHash, because gcc
// Indicated the name is ambiguous, because of __gny_cxx::hash

/**
 * getHash ()
 * The hash function. It is a standard multiplication hash.
 */
constexpr double HashingConstant1 = 39164205.20662217; // 2^26 * (sqrt(5) - 1) / 2
constexpr double HashingConstant2 = 0.6180339887; // (sqrt(5) - 1) / 2
inline HashKey getHash(const State &s) {
    double key = 0;
    int64_t key_part = 0;

    key_part = static_cast<int64_t>(s.glb_clk);
    key = key * HashingConstant1 + static_cast<double>(key_part) * HashingConstant2;

    for (uint i = 0; i < g->nrActors(); i++) {
        for (TCnt t = 0; t < static_cast<TCnt>(SDF_MAX_TIME + 1); t++) {
            key_part = static_cast<int64_t>(s.act_clk[i][t]);
            key = key * HashingConstant1 + static_cast<double>(key_part) * HashingConstant2;
        }
    }

    for (uint i = 0; i < g->nrChannels(); i++) {
        key_part = static_cast<int64_t>(s.ch[i]);
        key = key * HashingConstant1 + static_cast<double>(key_part) * HashingConstant2;
    }

    key = fmod(key, 1);
    return static_cast<HashKey>(floor(HASH_TABLE_SIZE * key));
}


/**
 * insertKeyHashTable ()
 * The function inserts a (key, value) pair into the hash table.
 */
inline void insertKeyHashTable(const HashKey key, const StackPos value) {
    auto slot = std::make_unique<HashSlot>();
    slot->value = value;
    slot->next = std::move(hashTable[key]);

    // Insert the stack position in the hash table
    hashTable[key] = std::move(slot);
}

/**
 * findValueInHashTable ()
 * The function returns the StackPos at which the given (key,state) pair is
 * located or NO_SLOT_IN_HASH if unkown.
 */
inline StackPos findValueInHashTable(const HashKey key, const State &s) {

    auto& slot = hashTable[key];
    while (slot != INVALID_HASH_KEY) {
        StackPos pos = slot->value;

        // Check if the state is equal
        if (equalStates(stack[pos], s)) {
            return pos;
        }

        // Move to next slot in the hash table
        slot = std::move(slot->next);
    }

    // Not found in hash table
    return NO_SLOT_IN_HASH;
}


/******************************************************************************
 * Dependencies
 *****************************************************************************/

/**
 * curReachableNodes
 * The array indicates wether an actor b in the current period has a chain
 * of dependencies to an actor a in the previous period. If there is
 * such a dependency the value of curReachableNodes[b][a] == 1, else it is 0.
 *
 * The 'curReachableNodesCh' array maintains the information which back channels
 * are seen on at least one of the paths from b to a.
 *
 * The 'prev' version of both arrays are used during the dependency construction
 * process.
 */

using TReachability = std::unique_ptr<bool[]>;
using TReachableNodes = std::unique_ptr<TReachability[]>;
using TReachableNodesCh = std::unique_ptr<TReachableNodes[]>;

 // NOLINTBEGIN(*global-variables)
TReachableNodes curReachableNodes;
TReachableNodes prevReachableNodes;
TReachableNodesCh curReachableNodesCh;
TReachableNodesCh prevReachableNodesCh;
// NOLINTEND(*global-variables)


/**
 * createDependencies ()
 * The function creates an initial abstract dependency graph.
 */
void createDependencies() {

    curReachableNodes = std::make_unique<TReachability[]>(g->nrActors());
    prevReachableNodes = std::make_unique<TReachability[]>(g->nrActors());
    curReachableNodesCh = std::make_unique<TReachableNodes[]>(g->nrActors());
    prevReachableNodesCh = std::make_unique<TReachableNodes[]>(g->nrActors());

    for (uint a = 0; a < g->nrActors(); a++) {
        curReachableNodes[a] = std::make_unique<bool[]>(g->nrActors());
        prevReachableNodes[a] = std::make_unique<bool[]>(g->nrActors());

        curReachableNodesCh[a] = std::make_unique<TReachability[]>(g->nrActors());
        prevReachableNodesCh[a] = std::make_unique<TReachability[]>(g->nrActors());

        for (uint b = 0; b < g->nrActors(); b++) {
            curReachableNodesCh[a][b] = std::make_unique<bool[]>(g->nrChannels());
            prevReachableNodesCh[a][b] = std::make_unique<bool[]>(g->nrChannels());
        }
    }
}

/**
 * initDependencies ()
 * The function initializes the dependency stack and the last dependency
 * node pointers.
 */
inline void initDependencies() {
    for (uint a = 0; a < g->nrActors(); a++) {
        for (uint b = 0; b < g->nrActors(); b++) {
            if (a == b) {
                curReachableNodes[a][b] = true;
                prevReachableNodes[a][b] = true;
            } else {
                curReachableNodes[a][b] = false;
                prevReachableNodes[a][b] = false;
            }
        }
    }

    for (uint a = 0; a < g->nrActors(); a++) {
        for (uint b = 0; b < g->nrActors(); b++) {
            for (uint c = 0; c < g->nrChannels(); c++) {
                curReachableNodesCh[a][b][c] = false;
                prevReachableNodesCh[a][b][c] = false;
            }
        }
    }
}

/**
 * createDependencyNode ()
 * Create a new dependency node for the actor a and add the dependency
 * to the list 'curDependencyNode'.
 */
inline void createDependencyNode(int a) {
    for (uint b = 0; b < g->nrActors(); b++) {
        curReachableNodes[b][a] = false;
    }

    for (uint b = 0; b < g->nrActors(); b++) {
        for (uint c = 0; c < g->nrChannels(); c++) {
            curReachableNodesCh[b][a][c] = false;
        }
    }
}

/**
 * addDependencyEdgeForActor ()
 * Add a dependency edge to the abstract dependency graph which expresses a
 * dependency of actor a to the previous firing of actor a.
 */
inline void addDependencyEdgeForActor(CId a) {
    for (uint b = 0; b < g->nrActors(); b++) {
        curReachableNodes[b][a] = curReachableNodes[b][a] || prevReachableNodes[b][a];
    }

    for (uint b = 0; b < g->nrActors(); b++) {
        for (uint c = 0; c < g->nrChannels(); c++) {
            curReachableNodesCh[b][a][c] =
                    curReachableNodesCh[b][a][c] || prevReachableNodesCh[b][a][c];
        }
    }
}

/**
 * addDependencyEdgeForChTokens ()
 * Add a dependency edge to the abstract dependency graph which expresses a
 * dependency of actor s to tokens produced on channel c by the previous
 * firing of actor a.
 */
inline void addDependencyEdgeForChTokens(CId a, CId s) {
    for (uint b = 0; b < g->nrActors(); b++) {
        curReachableNodes[b][s] = curReachableNodes[b][s] || prevReachableNodes[b][a];
    }

    for (uint b = 0; b < g->nrActors(); b++) {
        for (uint c = 0; c < g->nrChannels(); c++) {
            curReachableNodesCh[b][s][c] =
                    curReachableNodesCh[b][s][c] || prevReachableNodesCh[b][a][c];
        }
    }
}


/******************************************************************************
 * SDF
 *****************************************************************************/
/**
 * outputIntervalCycle ()
 * The function calculates the output interval of the states on the cycle. Its
 * value is equal to the average time between two firings in the cycle.
 */
inline TDtime outputIntervalCycle(const StackPos cyclePos) {
    int nr_fire = 0;
    TDtime time = 0;

    // Check all state from stack till cycle complete
    for (StackPos ptr = stackPtr - 1; ptr >= cyclePos; ptr--) {
        // Output actor fired?
        if (stack[ptr].act_clk[outputActor->getId()][0] > 0) {
            nr_fire++;
        }

        // Time between previous state
        time += stack[ptr].glb_clk;
    }

    return time / static_cast<TDtime>(nr_fire);
}

/**
 * storeState ()
 * The function stores the given state at the stack and insert an entry
 * into the hash table. Cycle detection is performed, as well as a
 * check to verify wether a cycle is valid or not.
 *
 * Return values:
 *      -1      - added state to stack
 *      >= 0    - output interval of detected cycle
 */
inline TDtime storeState(State &sdfState) {
    // Calculate the key in the hash table
    HashKey key = getHash(sdfState);

    // Check occurrence in hash table
    StackPos stackPos = findValueInHashTable(key, sdfState);
    if (stackPos == NO_SLOT_IN_HASH) {
        // State not in hash table (i.e. state not seen before)
        // Insert state in hash table
        insertKeyHashTable(key, stackPtr);

        // Insert state in stack
        pushStack(sdfState);
    } else {
        // State has been visited before (i.e. found cycle)
        return outputIntervalCycle(stackPos);
    }

    return -1;
}

// NOLINTBEGIN(*macro-usage)

#define GLB_CLK sdfState.glb_clk
#define ACT_CLK(a, t) sdfState.act_clk[a][t]
#define CH(c) sdfState.ch[c]

#define FIRE_ACT(a, t) ACT_CLK(a, t)++;
#define FIRE_ACT_END(a) ACT_CLK(a, 0)--;
#define CH_TOKENS(c, n) (CH(c) >= (n))
#define CONSUME(c, n) CH(c) = CH(c) - (n);
#define PRODUCE(c, n) CH(c) = CH(c) + (n);
#define ACT_END_FIRE(a) (ACT_CLK(a, 0) != 0)
#define ADVANCE_CLK GLB_CLK = GLB_CLK + 1;
#define NEXT_ITER GLB_CLK = 0;
#define LOWER_CLK(a)                                                                               \
    for (TCnt t = 0; t < static_cast<TCnt>(SDF_MAX_TIME); t++)                                                        \
        ACT_CLK(a, t) = ACT_CLK(a, t + 1);                                                         \
    ACT_CLK(a, SDF_MAX_TIME) = 0;

#define CH_TOKENS_PREV(c, n) (prevStateP.ch[c] >= (n))

// NOLINTEND(*macro-usage)

// NOLINTBEGIN(*global-variables)
State sdfState;
State prevState;
State prevStateP;
// NOLINTEND(*global-variables)

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find the abstract dependency
 * graph.
 */
void analyzePeriodicPhase() {
    bool outputActorFound = false;
    bool completedPeriod = false;
    State periodicState;
    int repCnt = 0;

    // Current sdf state is a periodic state
    createState(periodicState);
    copyState(periodicState, sdfState);

    // Initialize the dependencies
    initDependencies();

    // Start new iteration of the periodic phase
    NEXT_ITER;

    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (const auto& a : g->getActors()) {
        if (!outputActorFound && a->getId() == outputActor->getId()) {
            outputActorFound = true;
}

        while (outputActorFound && ACT_END_FIRE(a->getId())) {
            if (a->getId() == outputActor->getId()) {
                repCnt++;
                if (repCnt == outputActorRepCnt) {
                    NEXT_ITER;
                    repCnt = 0;
                }
            }

            for (const auto& p: a->getPorts()) {
                SDFchannel *ch = p->getChannel();

                if (p->getType() == SDFport::SDFportType::Out) {
                    PRODUCE(ch->getId(), p->getRate());
                }
            }
            FIRE_ACT_END(a->getId());
        }
    }

    // Fire the actors
    while (true) {
        // Update previous actor dependencies (abstract dependency graph)
        for (uint a = 0; a < g->nrActors(); a++) {
            for (uint b = 0; b < g->nrActors(); b++) {
                prevReachableNodes[a][b] = curReachableNodes[a][b];
}
}

        for (uint a = 0; a < g->nrActors(); a++) {
            for (uint b = 0; b < g->nrActors(); b++) {
                for (uint c = 0; c < g->nrChannels(); c++) {
                    prevReachableNodesCh[a][b][c] = curReachableNodesCh[a][b][c];
}
}
}

        // Start actor firings
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            while (true) {
                bool fire = true;

                // Check all input ports for tokens
                for (const auto& p: a->getPorts()) {
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::SDFportType::In) {
                        if (!CH_TOKENS(ch->getId(), p->getRate())) {
                            fire = false;
                            break;
                        }
                    }
                }

                // Ready to fire?
                if (fire) {
                    // Which dependencies were resolved in previous time step?
                    // Check tokens on each channel
                    for (const auto& p: a->getPorts()) {
                        const SDFchannel& ch = *(p->getChannel());
                        const SDFactor& b = *(ch.oppositePort(p.get())->getActor());

                        if (b.getId() == a->getId()) {
                            // Previous firing of actor not finished?
                            if (!CH_TOKENS_PREV(ch.getId(), p->getRate())) {
                                addDependencyEdgeForActor(a->getId());
                            }
                        } else if (p->getType() == SDFport::SDFportType::In) {
                            if (!CH_TOKENS_PREV(ch.getId(), p->getRate())) {
                                addDependencyEdgeForChTokens(b.getId(), a->getId());
                            }
                        }
                    }

                    // Consume tokens on each input and space on each output
                    for (const auto& p: a->getPorts()) {
                        SDFchannel *ch = p->getChannel();

                        if (p->getType() == SDFport::SDFportType::In) {
                            CONSUME(ch->getId(), p->getRate());
                        }
                    }

                    // Fire the actor
                    FIRE_ACT(a->getId(), a->getDiscreteExecutionTime());
                } else {
                    break;
                }
            }
        }

        // Lower clocks
        for (const auto& a : g->getActors()) {
            LOWER_CLK(a->getId());
        }

        // Advance the global clock
        ADVANCE_CLK;

        // Store state to check for progress
        copyState(prevStateP, sdfState);

        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (ACT_END_FIRE(a->getId())) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        if (equalStates(sdfState, periodicState)) {
                            completedPeriod = true;
                            break;
                        }
                        NEXT_ITER;
                        repCnt = 0;
                    }
                }

                for (const auto& p: a->getPorts()) {
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::SDFportType::Out) {
                        PRODUCE(ch->getId(), p->getRate());
                    }
                }

                FIRE_ACT_END(a->getId());
            }

            if (completedPeriod) {
                break;
}
        }

        if (completedPeriod) {
            break;
}
    }

    // Print the abstract dependency graph
    // for (uint a = 0; a < g->nrActors(); a++)
    //{
    //    std::cerr << a <<": " << std::endl;
    //    for (uint b = 0; b < g->nrActors(); b++)
    //    {
    //        if (curReachableNodes[a][b])
    //            std::cerr << b << ", ";
    //    }
    //    std::cerr << std::endl;
    //}
}


/**
 * execSDFgraph()
 * Execute the SDF graph till a deadlock is found or a recurrent state.
 * The output interval (i.e. inverse of throughput) is returned.
 */
TDtime execSDFgraph() {
    TDtime output_interval = NAN;
    int repCnt = 0;

    createState(sdfState);
    createState(prevState);
    createState(prevStateP);

    // Create initial state and add it to stack
    clearState(sdfState);
    copyState(prevState, sdfState);
    prevState.glb_clk = TDTIME_MAX;

    // Initial tokens and space
    for (const auto& ch: g->getChannels()) {
        CH(ch->getId()) = ch->getInitialTokens();
    }

    // Fire the actors
    while (true) {
        // Store state to find actor activity in periodic phase
        copyState(prevStateP, sdfState);

        // Finish actor firings
        for (const auto& a : g->getActors()) {
            while (ACT_END_FIRE(a->getId())) {
                if (outputActor->getId() == a->getId()) {
                    repCnt++;
                    if (repCnt == outputActorRepCnt) {
                        // Add state to hash of visited states
                        output_interval = storeState(sdfState);
                        if (output_interval != -1) {
                            analyzePeriodicPhase();
                            return output_interval;
                        }
                        NEXT_ITER;
                        repCnt = 0;
                    }
                }

                for (const auto& p: a->getPorts()) {
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::SDFportType::Out) {
                        PRODUCE(ch->getId(), p->getRate());
                    }
                }

                FIRE_ACT_END(a->getId());
            }
        }

        // Start actor firings
        for (const auto& iter : g->getActors()) {
            const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            while (true) {
                bool fire = true;

                // Check all input ports for tokens
                for (const auto& p: a->getPorts()) {
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::SDFportType::In) {
                        if (!CH_TOKENS(ch->getId(), p->getRate())) {
                            fire = false;
                            break;
                        }
                    }
                }

                // Ready to fire?
                if (!fire) {
                    break;
                }

                // Consume tokens on each input and space on each output
                for (const auto& p: a->getPorts()) {
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::SDFportType::In) {
                        CONSUME(ch->getId(), p->getRate());
                    }
                }

                // Fire the actor
                FIRE_ACT(a->getId(), a->getDiscreteExecutionTime());
            }
        }

        // Lower clocks
        for (const auto& a : g->getActors()) {
            LOWER_CLK(a->getId());
        }

        // Check for progress (i.e. no deadlock)
        if (equalStates(prevState, sdfState)) {
            return TDTIME_MAX;
        }

        // Advance the global clock
        ADVANCE_CLK;

        // Store state to check for progress
        copyState(prevState, sdfState);
    }

    return TDTIME_MAX;
}

} // namespace

/**
 * stateSpaceAbstractDepGraph ()
 * Compute the throughput and abstract dependency graph of an SDFG for
 * unconstrained buffer sizes and using auto-concurrency using a state-space
 * traversal.
 *
 * The abstract dependency graph is stored in the matrices 'nodes' (2D) and
 * 'edges' (3D). The value stored at 'nodes[a][b]' indicates whether there is at
 * least one edge which goes directly from node a to node b. The value stored at
 * 'edges[a][b][c]' indicates wether the channel c in the SDFG has a dependency
 * edge in the abstract dependency graph from node a to node b.
 */
extern double stateSpaceAbstractDepGraph(TimedSDFgraph& gr,
                                         TReachableNodes *nodes,
                                         TReachableNodesCh *edges,
                                         unsigned int stackSz,
                                         unsigned int hashSz) {
    RepetitionVector repVec;
    double thr = NAN;

    // Copy arguments to globals
    g = &gr;
    STACK_SIZE = static_cast<StackPos>(stackSz);
    HASH_TABLE_SIZE = hashSz;

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(*g)) {
        throw CException("Graph is not strongly connected.");
}

    // Find the output actor and its repetition count
    repVec = computeRepetitionVector(*g);
    outputActor = findOutputActor(*g, repVec);
    outputActorRepCnt = repVec[outputActor->getId()];

    // Find largest execution time of all actors
    SDF_MAX_TIME = getMaxDiscreteExecTime(*g);

    // Create hash and stack
    createStack();
    clearStack();
    createHashTable();
    clearHashTable();

    // Allocate memory space for the abstract dependency graph
    createDependencies();
    *nodes = std::move(curReachableNodes);
    *edges = std::move(curReachableNodesCh);

    // Find the maximal throughput
    thr = execSDFgraph();
    if (thr != TDTIME_MAX) {
        thr = (1.0) / thr;
    } else {
        thr = 0;
}

    return thr;
}
} // namespace SDF