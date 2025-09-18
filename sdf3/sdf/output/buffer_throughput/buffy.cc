/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   Buffy model
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *
 * $Id: buffy.cc,v 1.1.1.1.2.1 2010-04-22 07:18:41 mgeilen Exp $
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

#include "buffy.h"
#include "sdf/base/algo/repetition_vector.h"
namespace SDF {

namespace{

void printMinimalChannelSzStep(const TimedSDFgraph &g, std::ostream &out) {
    bool first = true;

    for (const auto& ch : g.getChannels()) {
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint minStepSz = 0;

        minStepSz = gcd(p, c);

        if (first) {
            first = false;
            out << "{" << minStepSz;
        } else {
            out << ", " << minStepSz;
        }
    }
    out << "}";
}

void printMaximumChannelSz(const TimedSDFgraph &g, std::ostream &out) {
    bool first = true;

    RepetitionVector repVec = computeRepetitionVector(g);

    for (const auto& ch : g.getChannels()) {
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint ub = 0;

        ub = p * repVec[srcPort->getActor()->getId()] + c * repVec[dstPort->getActor()->getId()]
             + t % gcd(p, c);
        ub = (ub > t ? ub : t);

        if (first) {
            first = false;
            out << "{" << ub;
        } else {
            out << ", " << ub;
        }
    }
    out << "}";
}

uint getUbDistributionSz(const TimedSDFgraph &g) {
    uint ub = 0;

    RepetitionVector repVec = computeRepetitionVector(g);

    for (const auto& ch : g.getChannels()) {
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint ub1 = 0;

        ub1 = p * repVec[srcPort->getActor()->getId()] + c * repVec[dstPort->getActor()->getId()]
              + t % gcd(p, c);
        ub = ub + (ub1 > t ? ub1 : t);
    }

    return ub;
}

void printMinimalChannelSz(const TimedSDFgraph &g, std::ostream &out) {
    bool first = true;

    for (const auto& ch : g.getChannels()) {
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint lb = 0;

        lb = p + c - gcd(p, c) + t % gcd(p, c);

        if (first) {
            first = false;
            out << "{" << lb;
        } else {
            out << ", " << lb;
        }
    }
    out << "}";
}

uint getLbDistributionSz(const TimedSDFgraph &g) {
    uint lb = 0;

    for (const auto& ch : g.getChannels()) {
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();

        lb += p + c - gcd(p, c) + t % gcd(p, c);
    }

    return lb;
}

uint64_t getLbOutputInterval(const TimedSDFgraph &g) {
    uint64_t min = UINT_MAX;

    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
        min = (min < a->getDiscreteExecutionTime() ? min : a->getDiscreteExecutionTime());
    }

    return min;
}

void printDefinitions(const TimedSDFgraph &g,
                             std::ostream &out,
                             uint64_t stackSz,
                             uint64_t hashSz,
                             uint64_t depStackSz) {
    out << "#define SDF_NUM_ACTORS          " << g.nrActors() << std::endl;
    out << "#define SDF_NUM_CHANNELS        " << g.nrChannels() << std::endl;
    out << "" << std::endl;
    out << "#define LB_DISTRIBUTION_SZ      " << getLbDistributionSz(g) << std::endl;
    out << "#define UB_DISTRIBUTION_SZ      " << getUbDistributionSz(g) << std::endl;
    out << "" << std::endl;
    out << "#define LB_OUTPUT_INTERVAL      " << getLbOutputInterval(g) << std::endl;
    out << "" << std::endl;
    out << "#define MIN_CHANNEL_SZ          ";

    printMinimalChannelSz(g, out);

    out << std::endl;
    out << "#define MAX_CHANNEL_SZ          ";

    printMaximumChannelSz(g, out);

    out << std::endl;
    out << "#define MIN_CHANNEL_SZ_STEP     ";

    printMinimalChannelSzStep(g, out);

    out << std::endl;
    out << "" << std::endl;
    out << "#define STACK_SIZE              " << stackSz << std::endl;
    out << "#define HASH_TABLE_SIZE         " << hashSz << std::endl;
    out << "#define DEPENDENCY_STACK_SIZE   " << depStackSz << std::endl;
    out << "" << std::endl;
    out << "typedef short TBufSize;" << std::endl;
    out << "typedef double TTime;" << std::endl;
    out << std::endl;
    out << "#define TTIME_MAX INT_MAX" << std::endl;
    out << std::endl;
    out << "TBufSize minSz[] = MIN_CHANNEL_SZ;" << std::endl;
    out << "TBufSize maxSz[] = MAX_CHANNEL_SZ;" << std::endl;
    out << "TBufSize minSzStep[] = MIN_CHANNEL_SZ_STEP;" << std::endl;
    out << "" << std::endl;
    out << "TBufSize maxDistributionSz = UB_DISTRIBUTION_SZ;" << std::endl;
    out << "TTime lbOutputInterval = LB_OUTPUT_INTERVAL;" << std::endl;
    out << "" << std::endl;
}

void printMiscFunctions(std::ostream &out) {
    out << "#include <math.h>" << std::endl;
    out << "#include <istd::ostream>" << std::endl;
    out << "#include <assert.h>" << std::endl;
    out << "" << std::endl;
    out << "using std::ostream;" << std::endl;
    out << "using std::cout;" << std::endl;
    out << "using std::cerr;" << std::endl;
    out << "using std::endl;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * exit ()" << std::endl;
    out << " * Exit the program with an error message." << std::endl;
    out << " */" << std::endl;
    out << "void exit(const char *msg, const int errorno)" << std::endl;
    out << "{" << std::endl;
    out << "    cerr << msg << std::endl;" << std::endl;
    out << "    exit(errorno);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printState(std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * State" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "typedef struct _State" << std::endl;
    out << "{" << std::endl;
    out << "    TTime act_clk[SDF_NUM_ACTORS];" << std::endl;
    out << "    TTime glb_clk;" << std::endl;
    out << "    TBufSize ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "} State;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printState ()" << std::endl;
    out << " * The function writes the state to the supplied stream." << std::endl;
    out << " */" << std::endl;
    out << "void printState(std::ostream &out, const State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"State:\" << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < SDF_NUM_ACTORS; i++)" << std::endl;
    out << R"(        out << "act_clk[" << i << "] = " << s.act_clk[i] << std::endl;)" << std::endl;
    out << "" << std::endl;
    out << "    out << \"glb_clk = \" << s.glb_clk << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < SDF_NUM_CHANNELS; i++)" << std::endl;
    out << R"(        out << "ch[" << i << "] = " << s.ch[i] << std::endl;)" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearState ()" << std::endl;
    out << " * The function sets the state to zero." << std::endl;
    out << " */" << std::endl;
    out << "void clearState(State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    char *s_ptr = (char*)&s;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < sizeof(State); i++)" << std::endl;
    out << "        s_ptr[i] = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * equalStates ()" << std::endl;
    out << " * The function compares to states and returns true if they are equal." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "bool equalStates(const State &s1, const State &s2)" << std::endl;
    out << "{" << std::endl;
    out << "    int *s1_ptr = (int*)&s1;" << std::endl;
    out << "    int *s2_ptr = (int*)&s2;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < sizeof(State) >> 2; i++)" << std::endl;
    out << "        if (s1_ptr[i] != s2_ptr[i])" << std::endl;
    out << "            return false;" << std::endl;
    out << "" << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * copyState ()" << std::endl;
    out << " * The function copies the state." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void copyState(State &to, const State &from)" << std::endl;
    out << "{" << std::endl;
    out << "    memcpy(&to, &from, sizeof(State));" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printStack(std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * Stack" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * StackPos" << std::endl;
    out << " * Index into stack." << std::endl;
    out << " */" << std::endl;
    out << "typedef long long StackPos;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * stack" << std::endl;
    out << " * The stack..." << std::endl;
    out << " */" << std::endl;
    out << "State *stack;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * stackPtr" << std::endl;
    out << " * Index into stack of the first free space." << std::endl;
    out << " */" << std::endl;
    out << "StackPos stackPtr;" << std::endl;
    out << "StackPos maxStackPtr;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * createStack ()" << std::endl;
    out << " * The function creates a stack." << std::endl;
    out << " */" << std::endl;
    out << "void createStack()" << std::endl;
    out << "{" << std::endl;
    out << "    stack = (State*)malloc(sizeof(State)*STACK_SIZE);" << std::endl;
    out << "" << std::endl;
    out << "    if (stack == NULL)" << std::endl;
    out << "        exit(\"Failed creating stack.\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    maxStackPtr = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * destroyStack()" << std::endl;
    out << " * The function destroys a stack." << std::endl;
    out << " */" << std::endl;
    out << "void destroyStack()" << std::endl;
    out << "{" << std::endl;
    out << "    free(stack);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearStack ()" << std::endl;
    out << " * Reset the stack to an empty state." << std::endl;
    out << " */" << std::endl;
    out << "void clearStack()" << std::endl;
    out << "{" << std::endl;
    out << "    stackPtr = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * popStack ()" << std::endl;
    out << " * Remove last element from the stack." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void popStack()" << std::endl;
    out << "{" << std::endl;
    out << "    if (stackPtr > 0)" << std::endl;
    out << "        stackPtr--;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * pushStack ()" << std::endl;
    out << " * Put an element on the stack." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void pushStack(const State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    if (stackPtr >= STACK_SIZE)" << std::endl;
    out << "        exit(\"Stack overflow.\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    copyState(stack[stackPtr], s);" << std::endl;
    out << "    stackPtr++;" << std::endl;
    out << "" << std::endl;
    out << "    if (stackPtr > maxStackPtr) maxStackPtr = stackPtr;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * topStack ()" << std::endl;
    out << " * The function returns the last element of the stack." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "State &topStack()" << std::endl;
    out << "{" << std::endl;
    out << "    return stack[stackPtr-1];" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printHash(std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * Hash" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "#define INVALID_HASH_KEY    NULL" << std::endl;
    out << "#define NO_SLOT_IN_HASH     -1" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * HashKey" << std::endl;
    out << " *" << std::endl;
    out << " */" << std::endl;
    out << "typedef unsigned long long HashKey;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * HashSlot" << std::endl;
    out << " *" << std::endl;
    out << " */" << std::endl;
    out << "typedef struct _HashSlot" << std::endl;
    out << "{" << std::endl;
    out << "    StackPos value;" << std::endl;
    out << "    struct _HashSlot *next;" << std::endl;
    out << "} HashSlot;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * hashTable" << std::endl;
    out << " * The hash table..." << std::endl;
    out << " */" << std::endl;
    out << "HashSlot **hashTable;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * createHashTable ()" << std::endl;
    out << " * The function constructs a hash table." << std::endl;
    out << " */" << std::endl;
    out << "void createHashTable()" << std::endl;
    out << "{" << std::endl;
    out << "    hashTable = (HashSlot**)malloc(sizeof(HashSlot*)*HASH_TABLE_SIZE);" << std::endl;
    out << "" << std::endl;
    out << "    if (hashTable == NULL)" << std::endl;
    out << "        exit(\"Failed creating hash table.\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << std::endl;
    out << "        hashTable[i] = INVALID_HASH_KEY;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * destroyHashTable ()" << std::endl;
    out << " * The function destroys a hash table." << std::endl;
    out << " */" << std::endl;
    out << "void destroyHashTable()" << std::endl;
    out << "{" << std::endl;
    out << "    HashSlot *s_cur, *s_next;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)"
        << std::endl;
    out << "        {" << std::endl;
    out << "            s_next = s_cur->next;" << std::endl;
    out << "            free(s_cur);" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    free(hashTable);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearHashTable ()" << std::endl;
    out << " * Resets the hash table to contain no keys." << std::endl;
    out << " */" << std::endl;
    out << "void clearHashTable()" << std::endl;
    out << "{" << std::endl;
    out << "    HashSlot *s_cur, *s_next;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)"
        << std::endl;
    out << "        {" << std::endl;
    out << "            s_next = s_cur->next;" << std::endl;
    out << "            free(s_cur);" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        hashTable[i] = INVALID_HASH_KEY;" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * hash ()" << std::endl;
    out << " * The hash function. It is a standard multiplication hash." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "HashKey hash(const State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    double key = 0;" << std::endl;
    out << "    char *s_ptr = (char*)&s;" << std::endl;
    out << "    long long key_part;" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < sizeof(State); i+=4)" << std::endl;
    out << "    {" << std::endl;
    out << "        key_part = 0;" << std::endl;
    out << "        for (int j = 0; j < 4 && i+j < sizeof(State); j++)" << std::endl;
    out << "        {" << std::endl;
    out << "            key_part = key_part << 8; // radix: 128" << std::endl;
    out << "            key_part = key_part + (long long)(abs(s_ptr[i]));" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        key = key * 39164205.20662217 + double(key_part) * 0.6180339887;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    key = fmod(key, 1);" << std::endl;
    out << "    return (HashKey) floor(HASH_TABLE_SIZE * key);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * insertKeyHashTable ()" << std::endl;
    out << " * The function inserts a (key, value) pair into the hash table." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void insertKeyHashTable(const HashKey key, const StackPos value)" << std::endl;
    out << "{" << std::endl;
    out << "    HashSlot *slot = (HashSlot*)malloc(sizeof(HashSlot));" << std::endl;
    out << "    slot->value = value;" << std::endl;
    out << "    slot->next = hashTable[key];" << std::endl;
    out << "" << std::endl;
    out << "    // Insert the stack position in the hash table" << std::endl;
    out << "    hashTable[key] = slot;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findValueInHashTable ()" << std::endl;
    out << " * The function returns the StackPos at which the given (key,state) pair is"
        << std::endl;
    out << " * located or NO_SLOT_IN_HASH if unkown." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "StackPos findValueInHashTable(const HashKey key, const State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    HashSlot *slot;" << std::endl;
    out << "" << std::endl;
    out << "    for (slot = hashTable[key]; slot != INVALID_HASH_KEY; slot = slot->next)"
        << std::endl;
    out << "    {" << std::endl;
    out << "        StackPos pos = slot->value;" << std::endl;
    out << "" << std::endl;
    out << "        if (equalStates(stack[pos], s))" << std::endl;
    out << "            return pos;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    return NO_SLOT_IN_HASH;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printHashKey ()" << std::endl;
    out << " * The function outputs the all hash slots with the given key." << std::endl;
    out << " */" << std::endl;
    out << "void printHashKey(std::ostream &out, const HashKey key)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"Key: \" << key <<std::endl;" << std::endl;
    out << "    out << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (HashSlot *slot = hashTable[key]; slot != NULL; slot = slot->next)"
        << std::endl;
    out << "    {" << std::endl;
    out << "        printState(out, stack[slot->value]);" << std::endl;
    out << "        out << std::endl;" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << std::endl;
}

void printDependencies(std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * Dependencies" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "#define NODE_PREV_PERIOD -1" << std::endl;
    out << "#define NODE_INVALID -2" << std::endl;
    out << "" << std::endl;
    out << "typedef long long DepPtr;" << std::endl;
    out << "" << std::endl;
    out << "typedef struct _DependencyEdge" << std::endl;
    out << "{" << std::endl;
    out << "    bool actor;" << std::endl;
    out << "    bool ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "    bool ch_back[SDF_NUM_CHANNELS];" << std::endl;
    out << "    DepPtr  node;" << std::endl;
    out << "} DependencyEdge;" << std::endl;
    out << "" << std::endl;
    out << "typedef struct _DependencyNode" << std::endl;
    out << "{" << std::endl;
    out << "    int actor;" << std::endl;
    out << "    DepPtr prev;" << std::endl;
    out << "    bool depFlag;" << std::endl;
    out << "    DependencyEdge deps[SDF_NUM_ACTORS];" << std::endl;
    out << "} DependencyNode;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * dependencyStack" << std::endl;
    out << " * The dependency stack..." << std::endl;
    out << " */" << std::endl;
    out << "DependencyNode *dependencyStack;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * dependencyStackPtr" << std::endl;
    out << " * Index into dependency stack of the first free space." << std::endl;
    out << " */" << std::endl;
    out << "DepPtr dependencyStackPtr;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * prevDependencyNode" << std::endl;
    out << " * The index of the last dependency node created for each actor" << std::endl;
    out << " * before the current time-step." << std::endl;
    out << " */" << std::endl;
    out << "DepPtr prevDependencyNode[SDF_NUM_ACTORS];" << std::endl;
    out << "DepPtr curDependencyNode[SDF_NUM_ACTORS];" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * dependencyDFScolor" << std::endl;
    out << " * Array to administer dependency node visits during DFS." << std::endl;
    out << " * before the current time-step." << std::endl;
    out << " */" << std::endl;
    out << "int *dependencyDFScolor;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * curReachableNodes" << std::endl;
    out << " * The array indicates wether an actor b in the current period has a chain"
        << std::endl;
    out << " * of dependencies to an actor a in the previous period. If there is" << std::endl;
    out << " * such a dependency the value of curReachableNodes[b][a] == 1, else it is 0."
        << std::endl;
    out << " *" << std::endl;
    out << " * The 'curReachableNodesCh' array maintains the information which back channels"
        << std::endl;
    out << " * are seen on at least one of the paths from b to a." << std::endl;
    out << " *" << std::endl;
    out << " * The 'prev' version of both arrays are used during the dependency construction"
        << std::endl;
    out << " * process." << std::endl;
    out << " */" << std::endl;
    out << "bool curReachableNodes[SDF_NUM_ACTORS][SDF_NUM_ACTORS];" << std::endl;
    out << "bool prevReachableNodes[SDF_NUM_ACTORS][SDF_NUM_ACTORS];" << std::endl;
    out << "" << std::endl;
    out << "bool curReachableNodesCh[SDF_NUM_ACTORS][SDF_NUM_ACTORS][SDF_NUM_CHANNELS];"
        << std::endl;
    out << "bool prevReachableNodesCh[SDF_NUM_ACTORS][SDF_NUM_ACTORS][SDF_NUM_CHANNELS];"
        << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * dependenciesDFScolor" << std::endl;
    out << " * Array to administer dependency node visits during DFS." << std::endl;
    out << " * before the current time-step." << std::endl;
    out << " */" << std::endl;
    out << "int dependenciesDFScolor[SDF_NUM_ACTORS];" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearDependencyNode ()" << std::endl;
    out << " * Set all dependencies in a dependency node to false and the" << std::endl;
    out << " * previous dependency node pointer is set as invalid." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void clearDependencyNode(DepPtr d)" << std::endl;
    out << "{" << std::endl;
    out << "    char *s_ptr = (char*)&dependencyStack[d];" << std::endl;
    out << "" << std::endl;
    out << "    for (int i = 0; i < sizeof(DependencyNode); i++)" << std::endl;
    out << "        s_ptr[i] = 0;" << std::endl;
    out << "" << std::endl;
    out << "    dependencyStack[d].prev = NODE_INVALID;" << std::endl;
    out << "" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        dependencyStack[d].deps[a].node = NODE_INVALID;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "void printDependencyEdge(std::ostream &out, DependencyEdge &e)" << std::endl;
    out << "{" << std::endl;
    out << "    if (e.node == NODE_INVALID)" << std::endl;
    out << "        return;" << std::endl;
    out << "    else if (e.node == NODE_PREV_PERIOD)" << std::endl;
    out << "        out << \"        dep: <previous period>\" << std::endl;" << std::endl;
    out << "    else" << std::endl;
    out << "        out << \"        dep: \" << e.node << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    if (e.actor)" << std::endl;
    out << "        out << \"        actor:   true\" << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "        if (e.ch[c])" << std::endl;
    out << "            out << \"        ch:      \" << c << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "        if (e.ch_back[c])" << std::endl;
    out << "            out << \"        ch_back: \" << c << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "void printDependencyNode(std::ostream &out, DependencyNode &d)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"actor:        \" << d.actor << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    if (d.prev == NODE_PREV_PERIOD)" << std::endl;
    out << "        out << \"prev:         <previous period>\" << std::endl;" << std::endl;
    out << "    else if (d.prev == NODE_INVALID)" << std::endl;
    out << "        out << \"prev:         <invalid>\" << std::endl;" << std::endl;
    out << "    else" << std::endl;
    out << "        out << \"prev:         \" << d.prev << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    out << \"dependencies: \" << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    if (d.depFlag == false)" << std::endl;
    out << "        out << \"    none.\" << std::endl;" << std::endl;
    out << "    else" << std::endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            out << \"    to actor: \" << a << std::endl;" << std::endl;
    out << "            printDependencyEdge(out, d.deps[a]);" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "    out << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * createDependencyStack ()" << std::endl;
    out << " * The function creates a dependency stack." << std::endl;
    out << " */" << std::endl;
    out << "void createDependencyStack()" << std::endl;
    out << "{" << std::endl;
    out << "    dependencyStackPtr = 0;" << std::endl;
    out << "    dependencyStack = (DependencyNode*) malloc(sizeof(DependencyNode)" << std::endl;
    out << "                            *DEPENDENCY_STACK_SIZE);" << std::endl;
    out << "" << std::endl;
    out << "    if (dependencyStack == NULL)" << std::endl;
    out << "        exit(\"Failed creating dependency stack.\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    dependencyDFScolor = (int*) malloc(sizeof(int)" << std::endl;
    out << "                            *DEPENDENCY_STACK_SIZE);" << std::endl;
    out << "" << std::endl;
    out << "    if (dependencyDFScolor == NULL)" << std::endl;
    out << "        exit(\"Failed creating dependency stack.\", 2);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * destroyDependencyStack()" << std::endl;
    out << " * The function destroys the dependency stack." << std::endl;
    out << " */" << std::endl;
    out << "void destroyDependencyStack()" << std::endl;
    out << "{" << std::endl;
    out << "    free(dependencyStack);" << std::endl;
    out << "    free(dependencyDFScolor);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearDependencyStack ()" << std::endl;
    out << " * Reset the dependency stack to an empty state." << std::endl;
    out << " */" << std::endl;
    out << "void clearDependencyStack()" << std::endl;
    out << "{" << std::endl;
    out << "    dependencyStackPtr = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * popDependencyStack ()" << std::endl;
    out << " * Remove last element from the dependency stack." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void popDependencyStack()" << std::endl;
    out << "{" << std::endl;
    out << "    if (dependencyStackPtr > 0)" << std::endl;
    out << "        dependencyStack--;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * pushDependencyStack ()" << std::endl;
    out << " * Put an element on the dependency stack." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "DepPtr pushDependencyStack()" << std::endl;
    out << "{" << std::endl;
    out << "    DepPtr d = dependencyStackPtr;" << std::endl;
    out << "" << std::endl;
    out << "    if (dependencyStackPtr >= DEPENDENCY_STACK_SIZE)" << std::endl;
    out << "        exit(\"Dependency stack overflow.\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    clearDependencyNode(dependencyStackPtr);" << std::endl;
    out << "    dependencyStackPtr++;" << std::endl;
    out << "" << std::endl;
    out << "    return d;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * initDependencies ()" << std::endl;
    out << " * The function initializes the dependency stack and the last dependency" << std::endl;
    out << " * node pointers." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void initDependencies()" << std::endl;
    out << "{" << std::endl;
    out << "    clearDependencyStack();" << std::endl;
    out << "" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        prevDependencyNode[a] = NODE_PREV_PERIOD;" << std::endl;
    out << "        curDependencyNode[a] = NODE_INVALID;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    for (int d = 0; d < DEPENDENCY_STACK_SIZE; d++)" << std::endl;
    out << "        dependencyDFScolor[d] = 0;" << std::endl;
    out << "" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "            if (a == b)" << std::endl;
    out << "                curReachableNodes[a][b] = true;" << std::endl;
    out << "            else" << std::endl;
    out << "                curReachableNodes[a][b] = false;" << std::endl;
    out << "" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                curReachableNodesCh[a][b][c] = false;" << std::endl;
    out << "" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        dependenciesDFScolor[a] = 0;" << std::endl;
    out << "" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * createDependencyNode ()" << std::endl;
    out << " * Create a new dependency node for the actor a and add the dependency" << std::endl;
    out << " * to the list 'curDependencyNode'." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "DepPtr createDependencyNode(int a)" << std::endl;
    out << "{" << std::endl;
    out << "    DepPtr d;" << std::endl;
    out << "" << std::endl;
    out << "    // Get a dependency node from the stack" << std::endl;
    out << "    d = pushDependencyStack();" << std::endl;
    out << "" << std::endl;
    out << "    dependencyStack[d].actor = a;" << std::endl;
    out << "    dependencyStack[d].depFlag = false;" << std::endl;
    out << "    dependencyStack[d].prev = prevDependencyNode[a];" << std::endl;
    out << "" << std::endl;
    out << "    curDependencyNode[a] = d;" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        curReachableNodes[b][a] = false;" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            curReachableNodesCh[b][a][c] = false;" << std::endl;
    out << "" << std::endl;
    out << "    return d;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * addDependencyEdgeForActor ()" << std::endl;
    out << " * Add a dependency edge to the dependency node d which expresses a" << std::endl;
    out << " * dependency of d to the previous firing of actor a." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void addDependencyEdgeForActor(DepPtr d, int a)" << std::endl;
    out << "{" << std::endl;
    out << "    // No dependency edge needed if the dependency to which the edge" << std::endl;
    out << "    // goes has no dependencies and does not belong to the previous period."
        << std::endl;
    out << "    DepPtr prev = prevDependencyNode[a];" << std::endl;
    out << "    if (prev != NODE_PREV_PERIOD && dependencyStack[prev].depFlag == false)"
        << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    dependencyStack[d].depFlag = true;" << std::endl;
    out << "    dependencyStack[d].deps[a].actor = true;" << std::endl;
    out << "    dependencyStack[d].deps[a].node = prevDependencyNode[a];" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        curReachableNodes[b][dependencyStack[d].actor] =" << std::endl;
    out << "                        curReachableNodes[b][dependencyStack[d].actor]" << std::endl;
    out << "                            || prevReachableNodes[b][a];" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            curReachableNodesCh[b][dependencyStack[d].actor][c] =" << std::endl;
    out << "                        curReachableNodesCh[b][dependencyStack[d].actor][c]"
        << std::endl;
    out << "                            || prevReachableNodesCh[b][a][c];" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * addDependencyEdgeChSpace ()" << std::endl;
    out << " * Add a dependency edge to the dependency node d which expresses a" << std::endl;
    out << " * dependency of d to tokens consumed on channel c by the previous firing of"
        << std::endl;
    out << " * actor a." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void addDependencyEdgeForChSpace(DepPtr d, int a, int c)" << std::endl;
    out << "{" << std::endl;
    out << "    // No dependency edge needed if the dependency to which the edge" << std::endl;
    out << "    // goes has no dependencies and does not belong to the previous period."
        << std::endl;
    out << "    DepPtr prev = prevDependencyNode[a];" << std::endl;
    out << "    if (prev != NODE_PREV_PERIOD && dependencyStack[prev].depFlag == false)"
        << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    dependencyStack[d].depFlag = true;" << std::endl;
    out << "    dependencyStack[d].deps[a].ch_back[c] = true;" << std::endl;
    out << "    dependencyStack[d].deps[a].node = prevDependencyNode[a];" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        curReachableNodes[b][dependencyStack[d].actor] =" << std::endl;
    out << "                        curReachableNodes[b][dependencyStack[d].actor]" << std::endl;
    out << "                            || prevReachableNodes[b][a];" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            curReachableNodesCh[b][dependencyStack[d].actor][c] =" << std::endl;
    out << "                        curReachableNodesCh[b][dependencyStack[d].actor][c]"
        << std::endl;
    out << "                            || prevReachableNodesCh[b][a][c];" << std::endl;
    out << "" << std::endl;
    out << "    // Add the back edge to all channels a connects to in previous period" << std::endl;
    out << "    for (int x = 0; x < SDF_NUM_ACTORS; x++)" << std::endl;
    out << "    {" << std::endl;
    out << "        if (curReachableNodes[x][a])" << std::endl;
    out << "        {" << std::endl;
    out << "            curReachableNodesCh[x][dependencyStack[d].actor][c] = true;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * addDependencyEdgeForChTokens ()" << std::endl;
    out << " * Add a dependency edge to the dependency node d which expresses a" << std::endl;
    out << " * dependency of d to tokens produced on channel c by the previous firing of"
        << std::endl;
    out << " * actor a." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void addDependencyEdgeForChTokens(DepPtr d, int a, int c)" << std::endl;
    out << "{" << std::endl;
    out << "    // No dependency edge needed if the dependency to which the edge" << std::endl;
    out << "    // goes has no dependencies and does not belong to the previous period."
        << std::endl;
    out << "    DepPtr prev = prevDependencyNode[a];" << std::endl;
    out << "    if (prev != NODE_PREV_PERIOD && dependencyStack[prev].depFlag == false)"
        << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    dependencyStack[d].depFlag = true;" << std::endl;
    out << "    dependencyStack[d].deps[a].ch[c] = true;" << std::endl;
    out << "    dependencyStack[d].deps[a].node = prevDependencyNode[a];" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        curReachableNodes[b][dependencyStack[d].actor] =" << std::endl;
    out << "                        curReachableNodes[b][dependencyStack[d].actor]" << std::endl;
    out << "                            || prevReachableNodes[b][a];" << std::endl;
    out << "" << std::endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            curReachableNodesCh[b][dependencyStack[d].actor][c] =" << std::endl;
    out << "                        curReachableNodesCh[b][dependencyStack[d].actor][c]"
        << std::endl;
    out << "                            || prevReachableNodesCh[b][a][c];" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findAllDependencyPaths ()" << std::endl;
    out << " * The function performs a DFS to find all paths that go from the actor to"
        << std::endl;
    out << " * itself in the previous period (i.e. it looks for a path in the dependency"
        << std::endl;
    out << " * nodes from the last dependency node of the actor in the current period to"
        << std::endl;
    out << " * a dependency node of the same actor in the previous firing). When such a"
        << std::endl;
    out << " * path is found, the list of blocked channels is updated. Requirement is that"
        << std::endl;
    out << " * a channel must be blocked on at least one path." << std::endl;
    out << " */" << std::endl;
    out << "void findAllDependencyPaths(const int actor, DepPtr d, bool *chP, bool *block)"
        << std::endl;
    out << "{" << std::endl;
    out << "    DependencyNode *n = &dependencyStack[d];" << std::endl;
    out << "    bool ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "" << std::endl;
    out << "    // Does dependency node have dependencies?" << std::endl;
    out << "    if (n->depFlag == false)" << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    // Visited dependency node before?" << std::endl;
    out << "    if (dependencyDFScolor[d] == 1)" << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    dependencyDFScolor[d] = 1;" << std::endl;
    out << "" << std::endl;
    out << "    // Recursion" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Is there no dependency with a?" << std::endl;
    out << "        if (n->deps[a].node == NODE_INVALID)" << std::endl;
    out << "            continue;" << std::endl;
    out << "" << std::endl;
    out << "        // Add all back channels in the dependency to the set ch" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            ch[c] = chP[c] || n->deps[a].ch_back[c];" << std::endl;
    out << "" << std::endl;
    out << "        if (n->deps[a].node == NODE_PREV_PERIOD)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Found a cycle in the dependencies?" << std::endl;
    out << "            if (a == actor)" << std::endl;
    out << "            {" << std::endl;
    out << "                for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                    block[c] = block[c] || ch[c];" << std::endl;
    out << "            }" << std::endl;
    out << "            else" << std::endl;
    out << "            {" << std::endl;
    out << "                // Continue with last dependency node of the actor" << std::endl;
    out << "                // in the previous period" << std::endl;
    out << "                findAllDependencyPaths(actor, prevDependencyNode[a], ch, block);"
        << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "        else" << std::endl;
    out << "        {" << std::endl;
    out << "            findAllDependencyPaths(actor, n->deps[a].node, ch, block);" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    dependencyDFScolor[d] = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findAllDependencies ()" << std::endl;
    out << " * The function performs a DFS to find all paths that go from the actor to"
        << std::endl;
    out << " * itself in the previous period (i.e. it looks for a path in the dependency"
        << std::endl;
    out << " * nodes from the last dependency node of the actor in the current period to"
        << std::endl;
    out << " * a dependency node of the same actor in the previous firing). When such a"
        << std::endl;
    out << " * path is found, the list of blocked channels is updated. Requirement is that"
        << std::endl;
    out << " * a channel must be blocked on at least one path." << std::endl;
    out << " */" << std::endl;
    out << "void findAllDependencies(const int actor, const int b, bool *chP, bool *block)"
        << std::endl;
    out << "{" << std::endl;
    out << "    bool ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "" << std::endl;
    out << "    // Visited node b before?" << std::endl;
    out << "    if (dependenciesDFScolor[b])" << std::endl;
    out << "    {" << std::endl;
    out << "        // Is the node b the actor we are looking for?" << std::endl;
    out << "        if (actor == b)" << std::endl;
    out << "        {" << std::endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                block[c] = block[c] || chP[c];" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        return;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    dependenciesDFScolor[b] = 1;" << std::endl;
    out << "" << std::endl;
    out << "    // Iterate over all nodes to find nodes reachable from b" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Is node a reachable from b?" << std::endl;
    out << "        if (curReachableNodes[b][a])" << std::endl;
    out << "        {" << std::endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                ch[c] = chP[c] || curReachableNodesCh[b][a][c];" << std::endl;
    out << "" << std::endl;
    out << "            findAllDependencies(actor, a, ch, block);" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    dependenciesDFScolor[b] = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printSdfHeader(const TimedSDFgraph &g, std::ostream &out, CString &outputActor) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * SDF" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * outputIntervalCycle ()" << std::endl;
    out << " * The function calculates the output interval of the states on the cycle. Its"
        << std::endl;
    out << " * value is equal to the average time between two firings in the cycle." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "TTime outputIntervalCycle(const StackPos cyclePos)" << std::endl;
    out << "{" << std::endl;
    out << "    int nr_fire = 0;" << std::endl;
    out << "    TTime time = 0;" << std::endl;
    out << "" << std::endl;
    out << "    // Check all state from stack till cycle complete" << std::endl;
    out << "    for (StackPos ptr = stackPtr -1; ptr >= cyclePos; ptr--)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Output actor fired?" << std::endl;

    for (const auto& a : g.getActors()) {
        if (outputActor == a->getName()) {
            out << "        if (stack[ptr].act_clk[";
            out << a->getId();
            out << "] == 1)" << std::endl;
        }
    }

    out << "            nr_fire++;" << std::endl;
    out << "" << std::endl;
    out << "        // Time between previous state" << std::endl;
    out << "        time += stack[ptr].glb_clk;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    return time/nr_fire;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * storeState ()" << std::endl;
    out << " * The function stores the given state at the stack and insert an entry" << std::endl;
    out << " * into the hash table. Cycle detection is performed, as well as a" << std::endl;
    out << " * check to verify wether a cycle is valid or not." << std::endl;
    out << " *" << std::endl;
    out << " * Return values:" << std::endl;
    out << " *      -1      - added state to stack" << std::endl;
    out << " *      >= 0    - output interval of detected cycle" << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "TTime storeState(State &sdfState)" << std::endl;
    out << "{" << std::endl;
    out << "    // Calculate the key in the hash table" << std::endl;
    out << "    HashKey key = hash(sdfState);" << std::endl;
    out << "" << std::endl;
    out << "    // Check occurrence in hash table" << std::endl;
    out << "    StackPos stackPos = findValueInHashTable(key, sdfState);" << std::endl;
    out << "    if (stackPos == NO_SLOT_IN_HASH)" << std::endl;
    out << "    {" << std::endl;
    out << "        // State not in hash table (i.e. state not seen before)" << std::endl;
    out << "        // Insert state in hash table" << std::endl;
    out << "        insertKeyHashTable(key, stackPtr);" << std::endl;
    out << "" << std::endl;
    out << "        // Insert state in stack" << std::endl;
    out << "        pushStack(sdfState);" << std::endl;
    out << "    }" << std::endl;
    out << "    else" << std::endl;
    out << "    {" << std::endl;
    out << "        // State has been visited before (i.e. found cycle)" << std::endl;
    out << "        return outputIntervalCycle(stackPos);" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    return -1;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "#define GLB_CLK             sdfState.glb_clk" << std::endl;
    out << "#define ACT_CLK(a)          sdfState.act_clk[a]" << std::endl;
    out << "#define CH(c)               sdfState.ch[c]" << std::endl;
    out << "" << std::endl;
    out << "#define ACT_READY(a)        (ACT_CLK(a) == 0)" << std::endl;
    out << "#define CH_TOKENS(c,n)      (CH(c) >= n)" << std::endl;
    out << "#define CH_SPACE(c,n)       (CH(c) <= sz[c] - n)" << std::endl;
    out << "#define FIRE_ACT(a,t)       ACT_CLK(a) = t;" << std::endl;
    out << "#define CONSUME(c,n)        CH(c) = CH(c) - n;" << std::endl;
    out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << std::endl;
    out << "#define LOWER_CLK(a)        if (ACT_CLK(a) > 0) { ACT_CLK(a) = ACT_CLK(a) - 1; }"
        << std::endl;
    out << "#define ACT_END_FIRE(a)     (ACT_CLK(a) == 1)" << std::endl;
    out << "#define ADVANCE_CLK         GLB_CLK = GLB_CLK + 1;" << std::endl;
    out << "#define NEXT_ITER           GLB_CLK = 0;" << std::endl;
    out << "" << std::endl;
    out << "#define ACT_READY_PREV(a)   (prevStateP.act_clk[a] == 0)" << std::endl;
    out << "#define CH_TOKENS_PREV(c,n) (prevStateP.ch[c] >= n)" << std::endl;
    out << "#define CH_SPACE_PREV(c,n)  (prevStateP.ch[c] <= sz[c] - n)" << std::endl;
    out << "" << std::endl;
    out << "State sdfState;" << std::endl;
    out << "State prevState;" << std::endl;
    out << "State prevStateP;" << std::endl;
    out << "" << std::endl;
}

void
printAnalyzePeriodicPhase(const TimedSDFgraph &g, std::ostream &out, CString &outputActor) {
    out << "/**" << std::endl;
    out << " * analyzePeriodicPhase ()" << std::endl;
    out << " * Analyze the periodic phase of the schedule to find all critical cycles."
        << std::endl;
    out << " */" << std::endl;
    out << "void analyzePeriodicPhase(const TBufSize *sz, bool *block)" << std::endl;
    out << "{" << std::endl;
    out << "    State periodicState;" << std::endl;
    out << "    DepPtr d;" << std::endl;
    out << "" << std::endl;
    out << "    // Current sdf state is a periodic state" << std::endl;
    out << "    copyState(periodicState, sdfState);" << std::endl;
    out << "" << std::endl;
    out << "    // Initialize the dependencies" << std::endl;
    out << "    initDependencies();" << std::endl;
    out << "" << std::endl;
    out << "    // Start new iteration of the periodic phase" << std::endl;
    out << "    NEXT_ITER;" << std::endl;
    out << "" << std::endl;
    out << "    // Complete the remaining actor firings" << std::endl;

    bool foundOutputActor = false;
    for (const auto& a : g.getActors()) {
        if (foundOutputActor) {
            out << "        if (ACT_END_FIRE(" << a->getId() << "))" << std::endl;
            out << "        {" << std::endl;

            for (const auto& p : a->getPorts()) {
                SDFchannel *ch = p->getChannel();

                if (p->getType() == SDFport::SDFportType::In) {
                    out << "            CONSUME(" << ch->getId() << ",";
                    out << p->getRate() << ");" << std::endl;
                } else {
                    out << "            PRODUCE(" << ch->getId() << ",";
                    out << p->getRate() << ");" << std::endl;
                }
            }
            out << "        }" << std::endl;
        }

        // Actor a is the output actor?
        if (outputActor == a->getName()) {
            foundOutputActor = true;
        }
    }
    out << "" << std::endl;

    out << "    // Fire the actors" << std::endl;
    out << "    while (true)" << std::endl;
    out << "    {" << std::endl;

    out << "        // Lower clocks" << std::endl;
    for (const auto& a : g.getActors()) {
        out << "        LOWER_CLK(" << a->getId() << ");" << std::endl;
    }
    out << "" << std::endl;

    out << "        // Advance the global clock" << std::endl;
    out << "        ADVANCE_CLK;" << std::endl;
    out << "" << std::endl;
    out << "        // Update previous actor dependencies" << std::endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "            for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "                prevReachableNodes[a][b] = curReachableNodes[a][b];" << std::endl;
    out << "" << std::endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "            for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "                for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                prevReachableNodesCh[a][b][c] = curReachableNodesCh[a][b][c];"
        << std::endl;
    out << "" << std::endl;
    out << "        // Start actor firings" << std::endl;
    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        out << "        if (ACT_READY(" << a->getId() << "))" << std::endl;
        out << "        {" << std::endl;
        out << "            bool fire = true;" << std::endl;
        out << "" << std::endl;
        out << "            // Check space on each output" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::Out) {
                out << "            if (!CH_SPACE(" << ch->getId() << "," << p->getRate();
                out << "))" << std::endl;
                out << "                fire = false;" << std::endl;
            }
        }

        out << "" << std::endl;
        out << "            // Check tokens on each input" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "            if (!CH_TOKENS(" << ch->getId() << "," << p->getRate();
                out << "))" << std::endl;
                out << "                fire = false;" << std::endl;
            }
        }

        out << "" << std::endl;
        out << "            // Ready to fire?" << std::endl;
        out << "            if (fire)" << std::endl;
        out << "            {" << std::endl;
        out << "                // Create a dependency node for this actor firing" << std::endl;
        out << "                d = createDependencyNode(" << a->getId() << ");" << std::endl;
        out << "" << std::endl;
        out << "                // Which dependencies were resolved in previous time step?"
            << std::endl;

        out << "                if (!ACT_READY_PREV(" << a->getId() << "))" << std::endl;
        out << "                {" << std::endl;
        out << "                    // Previous firing of actor was not finished" << std::endl;
        out << "                    addDependencyEdgeForActor(d," << a->getId() << ");"
            << std::endl;
        out << "                }" << std::endl;
        out << "                else" << std::endl;
        out << "                {" << std::endl;
        out << "                    // Check tokens and space on each channel" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::Out) {
                out << "                    if (!CH_SPACE_PREV(" << ch->getId() << ","
                    << p->getRate();
                out << "))" << std::endl;
                out << "                        addDependencyEdgeForChSpace(d,";
                out << ch->oppositePort(p.get())->getActor()->getId() << ",";
                out << ch->getId() << ");" << std::endl;
            } else {
                out << "                    if (!CH_TOKENS_PREV(" << ch->getId() << ","
                    << p->getRate();
                out << "))" << std::endl;
                out << "                        addDependencyEdgeForChTokens(d,";
                out << ch->oppositePort(p.get())->getActor()->getId() << ",";
                out << ch->getId() << ");" << std::endl;
            }
        }

        out << "                }" << std::endl;
        out << "" << std::endl;
        out << "                // Fire actor" << std::endl;
        out << "                FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << std::endl;
        out << "            }" << std::endl;
        out << "        }" << std::endl;
    }
    out << "" << std::endl;

    out << "        // Update previous actor dependency nodes" << std::endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Current node is a valid node?" << std::endl;
    out << "            if (curDependencyNode[a] != NODE_INVALID)" << std::endl;
    out << "            {" << std::endl;
    out << "                prevDependencyNode[a] = curDependencyNode[a];" << std::endl;
    out << "                curDependencyNode[a] = NODE_INVALID;" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Store state to check for progress" << std::endl;
    out << "        copyState(prevStateP, sdfState);" << std::endl;
    out << "" << std::endl;

    out << "        // Finish actor firings" << std::endl;
    for (const auto& a : g.getActors()) {
        out << "        if (ACT_END_FIRE(" << a->getId() << "))" << std::endl;
        out << "        {" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "            CONSUME(" << ch->getId() << ",";
                out << p->getRate() << ");" << std::endl;
            } else {
                out << "            PRODUCE(" << ch->getId() << ",";
                out << p->getRate() << ");" << std::endl;
            }
        }

        if (outputActor == a->getName()) {
            out << "" << std::endl;
            out << "            // Reached periodic state?" << std::endl;
            out << "            if (equalStates(periodicState, sdfState))" << std::endl;
            out << "                break;" << std::endl;
            out << "" << std::endl;
            out << "            NEXT_ITER;" << std::endl;
        }

        out << "        }" << std::endl;
    }

    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Print all dependencies" << std::endl;
    out << "    //for (DepPtr d = 0; d < dependencyStackPtr; d++)" << std::endl;
    out << "    //{" << std::endl;
    out << "    //    cerr << \"Dependency: \" << d << std::endl;" << std::endl;
    out << "    //    printDependencyNode(cerr, dependencyStack[d]);" << std::endl;
    out << "    //}" << std::endl;
    out << "    //cerr << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    // Print reachable nodes" << std::endl;
    out << "    //for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    //{" << std::endl;
    out << "    //    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << std::endl;
    out << "    //    {" << std::endl;
    out << "    //        if (curReachableNodes[a][b])" << std::endl;
    out << "    //        {" << std::endl;
    out << "    //            cerr << \"prev(\" << a << \") -> act(\" << b << \")\" << std::endl;"
        << std::endl;
    out << "    //" << std::endl;
    out << "    //            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "    //            {" << std::endl;
    out << "    //                if (curReachableNodesCh[a][b][c])" << std::endl;
    out << "    //                    cerr << \"   ch: \" << c << std::endl;" << std::endl;
    out << "    //            }" << std::endl;
    out << "    //            cerr << std::endl;" << std::endl;
    out << "    //        }" << std::endl;
    out << "    //    }" << std::endl;
    out << "    //}" << std::endl;
    out << "" << std::endl;
    out << "    // Find all paths from an actor to itself in the state-space" << std::endl;
    out << "    // This is a path from the actor to a dependency of the same actor" << std::endl;
    out << "    // in a previous period (This version does not find the critical cycle)."
        << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        bool ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            ch[c] = false;" << std::endl;
    out << "" << std::endl;
    out << "        findAllDependencies(a, a, ch ,block);" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Find all paths from an actor to itself in the state-space" << std::endl;
    out << "    // This is a path from the last dependency node (i.e. entry in the" << std::endl;
    out << "    // prevDependencyNode array) to a dependency of the same node in a" << std::endl;
    out << "    // previous period (This version finds the critical cycle)." << std::endl;
    out << "    //for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    //{" << std::endl;
    out << "    //    bool ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "    //" << std::endl;
    out << "    //    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "    //        ch[c] = false;" << std::endl;
    out << "    //" << std::endl;
    out << "    //    findAllDependencyPaths(a, prevDependencyNode[a], ch ,block);" << std::endl;
    out << "    //}" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printAnalyzeDeadlock(const TimedSDFgraph &g, std::ostream &out) {
    out << "/**" << std::endl;
    out << " * analyzeDeadlock ()" << std::endl;
    out << " * Analyze the deadlock in the schedule." << std::endl;
    out << " */" << std::endl;
    out << "void analyzeDeadlock(const TBufSize *sz, bool *block)" << std::endl;
    out << "{" << std::endl;
    out << "    DepPtr d;" << std::endl;
    out << "" << std::endl;
    out << "    // Initialize the dependencies" << std::endl;
    out << "    initDependencies();" << std::endl;
    out << "" << std::endl;
    out << "    // Create a dependency node for each actor to find" << std::endl;
    out << "    // the cause of it blocking." << std::endl;

    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        out << "    // Actor: " << a->getId() << std::endl;

        out << "    // Create a dependency node for this actor firing" << std::endl;
        out << "    d = createDependencyNode(" << a->getId() << ");" << std::endl;
        out << "    dependencyStack[d].depFlag = true;" << std::endl;
        out << "" << std::endl;
        out << "    // Check space on each output" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::Out) {
                out << "    if (!CH_SPACE(" << ch->getId() << "," << p->getRate();
                out << "))" << std::endl;
                out << "        addDependencyEdgeForChSpace(d,";
                out << ch->oppositePort(p.get())->getActor()->getId() << ",";
                out << ch->getId() << ");" << std::endl;
            }
        }

        out << "" << std::endl;
        out << "    // Check tokens on each input" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "    if (!CH_TOKENS(" << ch->getId() << "," << p->getRate();
                out << "))" << std::endl;
                out << "        addDependencyEdgeForChTokens(d,";
                out << ch->oppositePort(p.get())->getActor()->getId() << ",";
                out << ch->getId() << ");" << std::endl;
            }
        }
    }

    out << "    // Update previous actor dependency nodes" << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Current node is a valid node?" << std::endl;
    out << "        if (curDependencyNode[a] != NODE_INVALID)" << std::endl;
    out << "        {" << std::endl;
    out << "            prevDependencyNode[a] = curDependencyNode[a];" << std::endl;
    out << "            curDependencyNode[a] = NODE_INVALID;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Print all dependencies" << std::endl;
    out << "    //for (DepPtr d = 0; d < dependencyStackPtr; d++)" << std::endl;
    out << "    //{" << std::endl;
    out << "    //    cerr << \"Dependency: \" << d << std::endl;" << std::endl;
    out << "    //    printDependencyNode(cerr, dependencyStack[d]);" << std::endl;
    out << "    //}" << std::endl;
    out << "    //cerr << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    // Find all paths from an actor to itself in the state-space" << std::endl;
    out << "    // This is a path from the last dependency node (i.e. entry in the" << std::endl;
    out << "    // prevDependencyNode array) to a dependency of the same node in the" << std::endl;
    out << "    // previous period." << std::endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        bool ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            ch[c] = false;" << std::endl;
    out << "" << std::endl;
    out << "        findAllDependencyPaths(a, prevDependencyNode[a], ch ,block);" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printExecSdfGraph(const TimedSDFgraph &g, std::ostream &out, CString &outputActor) {
    out << "/**" << std::endl;
    out << " * execSDFGraph()" << std::endl;
    out << " * Execute the SDF graph till a deadlock is found or a recurrent state. The output "
           "interval"
        << std::endl;
    out << " * (i.e. inverse of throughput) is returned." << std::endl;
    out << " */" << std::endl;
    out << "TTime execSDFGraph(const TBufSize *sz, bool *block)" << std::endl;
    out << "{" << std::endl;
    out << "    TTime output_interval;" << std::endl;
    out << "" << std::endl;
    out << "    // Create initial state and add it to stack" << std::endl;
    out << "    clearState(sdfState);" << std::endl;
    out << "    copyState(prevState, sdfState);" << std::endl;
    out << "    prevState.glb_clk = TTIME_MAX;" << std::endl;
    out << "" << std::endl;
    out << "    // Initial tokens" << std::endl;

    for (const auto& ch : g.getChannels()) {
        if (ch->getInitialTokens() != 0) {
            out << "    if (!CH_SPACE(" << ch->getId() << ",";
            out << ch->getInitialTokens() << "))" << std::endl;
            out << "    {" << std::endl;
            out << "        block[" << ch->getId() << "] = true;" << std::endl;
            out << "        return TTIME_MAX;" << std::endl;
            out << "    }" << std::endl;
            out << "    PRODUCE(" << ch->getId() << ",";
            out << ch->getInitialTokens() << ");" << std::endl;
        }
    }

    out << "" << std::endl;
    out << "    // Fire the actors" << std::endl;
    out << "    while (true)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Finish actor firings" << std::endl;

    for (const auto& a : g.getActors()) {
        out << "        if (ACT_END_FIRE(" << a->getId() << "))" << std::endl;
        out << "        {" << std::endl;

        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "            CONSUME(" << ch->getId() << ",";
                out << p->getRate() << ");" << std::endl;
            } else {
                out << "            PRODUCE(" << ch->getId() << ",";
                out << p->getRate() << ");" << std::endl;
            }
        }

        if (outputActor == a->getName()) {
            out << "" << std::endl;
            out << "            // Add state to hash of visited states" << std::endl;
            out << "            output_interval = storeState(sdfState);" << std::endl;
            out << "            if (output_interval != -1)" << std::endl;
            out << "            {" << std::endl;
            out << "#ifndef SEARCH_FULL_SPACE" << std::endl;
            out << "                analyzePeriodicPhase(sz, block);" << std::endl;
            out << "#endif" << std::endl;
            out << "                return output_interval;" << std::endl;
            out << "            }" << std::endl;
            out << "            NEXT_ITER;" << std::endl;
        }

        out << "        }" << std::endl;
    }

    out << std::endl;

    out << "        // Lower clocks" << std::endl;
    for (const auto& a : g.getActors()) {
        out << "        LOWER_CLK(" << a->getId() << ");" << std::endl;
    }
    out << "" << std::endl;

    out << "        // Check for progress (i.e. no deadlock)" << std::endl;
    out << "        if (equalStates(prevState, sdfState))" << std::endl;
    out << "        {" << std::endl;
    out << "            // Find cause of deadlock" << std::endl;
    out << "            analyzeDeadlock(sz, block);" << std::endl;
    out << "" << std::endl;
    out << "            return TTIME_MAX;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;

    out << "        // Advance the global clock" << std::endl;
    out << "        ADVANCE_CLK;" << std::endl;
    out << "" << std::endl;

    out << "        // Store state to check for progress" << std::endl;
    out << "        copyState(prevState, sdfState);" << std::endl;
    out << "" << std::endl;

    out << "        // Start actor firings" << std::endl;
    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        out << "        if (ACT_READY(" << a->getId() << "))" << std::endl;
        out << "        {" << std::endl;
        out << "            bool fire = true;" << std::endl;
        out << "" << std::endl;

        out << "            // Check tokens on each input" << std::endl;
        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "            if (!CH_TOKENS(" << ch->getId() << "," << p->getRate() << "))"
                    << std::endl;
                out << "                fire = false;" << std::endl;
            }
        }
        out << "" << std::endl;

        out << "            // Check space on each output" << std::endl;
        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::Out) {
                out << "            if (!CH_SPACE(" << ch->getId() << "," << p->getRate() << "))"
                    << std::endl;
                out << "                fire = false;" << std::endl;
            }
        }
        out << "" << std::endl;

        out << "            // Ready to fire?" << std::endl;
        out << "            if (fire)" << std::endl;
        out << "                FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << std::endl;
        out << "        }" << std::endl;
    }
    out << "" << std::endl;
    out << "        // Store state to find actor activity in periodic phase" << std::endl;
    out << "        copyState(prevStateP, sdfState);" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printSdf(const TimedSDFgraph &g, std::ostream &out, CString &outputActor) {
    printSdfHeader(g, out, outputActor);
    printAnalyzePeriodicPhase(g, out, outputActor);
    printAnalyzeDeadlock(g, out);
    printExecSdfGraph(g, out, outputActor);
}

void printDistribution(std::ostream &out) {
    out << "/*******************************************************************************"
        << std::endl;
    out << " * Distributions" << std::endl;
    out << " ******************************************************************************/"
        << std::endl;
    out << "typedef struct _Distribution" << std::endl;
    out << "{" << std::endl;
    out << "    TBufSize size;" << std::endl;
    out << "    TTime outputInterval;" << std::endl;
    out << "    TBufSize sz[SDF_NUM_CHANNELS];" << std::endl;
    out << "    bool block[SDF_NUM_CHANNELS];" << std::endl;
    out << "    struct _Distribution *next;" << std::endl;
    out << "    struct _Distribution *check;" << std::endl;
    out << "} Distribution;" << std::endl;
    out << "" << std::endl;
    out << "typedef struct _MinStorageDistr" << std::endl;
    out << "{" << std::endl;
    out << "    TBufSize size;" << std::endl;
    out << "    TTime   outputInterval;" << std::endl;
    out << "    Distribution *distributions;" << std::endl;
    out << "    struct _MinStorageDistr *next;" << std::endl;
    out << "    struct _MinStorageDistr *prev;" << std::endl;
    out << "} MinStorageDistr;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * minStorageDistributions" << std::endl;
    out << " * List of all minimal storage distributions." << std::endl;
    out << " */" << std::endl;
    out << "MinStorageDistr *minStorageDistributions = NULL;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * checkedDistributions" << std::endl;
    out << " * Lists of all checked distributions." << std::endl;
    out << " */" << std::endl;
    out << "Distribution *checkedDistributions = NULL;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * execDistribution ()" << std::endl;
    out << " * Execute one distribution." << std::endl;
    out << " */" << std::endl;
    out << "void execDistribution(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    // Clear stack and hash table" << std::endl;
    out << "    clearStack();" << std::endl;
    out << "    clearHashTable();" << std::endl;
    out << "" << std::endl;
    out << "    // Initialize blocking channels" << std::endl;
    out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "        d->block[c] = false;" << std::endl;
    out << "" << std::endl;
    out << "    // Execute the SDF graph to find its output interval" << std::endl;
    out << "    d->outputInterval = execSDFGraph(d->sz, d->block);" << std::endl;
    out << "" << std::endl;
    out << "    //cerr << d->size << \" \" << d->outputInterval << std::endl;" << std::endl;
    out << "    //for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "    //    cerr << d->sz[c] << \" \" << d->block[c] << std::endl;" << std::endl;
    out << "    //cerr << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findLbOutputInterval ()" << std::endl;
    out << " * Find the lower bound on the output interval by executing the" << std::endl;
    out << " * graph with the upper bounds on the channel sizes." << std::endl;
    out << " */" << std::endl;
    out << "TTime findLbOutputInterval()" << std::endl;
    out << "{" << std::endl;
    out << "    TTime outputInterval;" << std::endl;
    out << "    Distribution d;" << std::endl;
    out << "" << std::endl;
    out << "    // Create distribution with upper bounds" << std::endl;
    out << "    d.next = NULL;" << std::endl;
    out << "    d.check = NULL;" << std::endl;
    out << "    d.size = UB_DISTRIBUTION_SZ;" << std::endl;
    out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "        d.sz[c] = maxSz[c];" << std::endl;
    out << "" << std::endl;
    out << "    // Execute distribution to find output interval" << std::endl;
    out << "    execDistribution(&d);" << std::endl;
    out << "    outputInterval = d.outputInterval;" << std::endl;
    out << "" << std::endl;
    out << "    return outputInterval;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * graphDeadlockFree()" << std::endl;
    out << " * The function checks wether the SDF graph is deadlock free. If not," << std::endl;
    out << " * false is returned. Else, the function returns true." << std::endl;
    out << " */" << std::endl;
    out << "bool graphDeadlockFree()" << std::endl;
    out << "{" << std::endl;
    out << "    TTime outputInterval;" << std::endl;
    out << "" << std::endl;
    out << "    // Find output interval for unbounded graph" << std::endl;
    out << "    outputInterval = findLbOutputInterval();" << std::endl;
    out << "" << std::endl;
    out << "    if (outputInterval == TTIME_MAX)" << std::endl;
    out << "        return false;" << std::endl;
    out << "" << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * updateMinimalDistributions ()" << std::endl;
    out << " * Add the distribution d to the set of minimal storage" << std::endl;
    out << " * distribution (if it is one) and remove all distributions" << std::endl;
    out << " * which can not be a minimal storage distribution because of d." << std::endl;
    out << " */" << std::endl;
    out << "void updateMinimalDistributions(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    MinStorageDistr *dist = minStorageDistributions;" << std::endl;
    out << "    MinStorageDistr *distn;" << std::endl;
    out << "" << std::endl;
    out << "    // Infinite output interval (deadlock)?" << std::endl;
    out << "    if (d->outputInterval == TTIME_MAX)" << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    while (dist != NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Larger throughput with larger size" << std::endl;
    out << "        if (d->outputInterval < dist->outputInterval" << std::endl;
    out << "                && d->size > dist->size)" << std::endl;
    out << "        {" << std::endl;
    out << "            if (dist->next == NULL) break;" << std::endl;
    out << "" << std::endl;
    out << "            // Next distribution" << std::endl;
    out << "            dist = dist->next;" << std::endl;
    out << "            continue;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Smaller throughput with equal or larger size or" << std::endl;
    out << "        // equal throughput with larger size" << std::endl;
    out << "        if ((d->outputInterval > dist->outputInterval && d->size >= dist->size)"
        << std::endl;
    out << "                || (d->outputInterval == dist->outputInterval" << std::endl;
    out << "                        && d->size > dist->size))" << std::endl;
    out << "        {" << std::endl;
    out << "            // Distribution d is not minimal" << std::endl;
    out << "            return;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Equal throughput and size" << std::endl;
    out << "        if (d->outputInterval == dist->outputInterval && d->size == dist->size)"
        << std::endl;
    out << "        {" << std::endl;
    out << "            // Add distribution d to dist" << std::endl;
    out << "            d->next = dist->distributions;" << std::endl;
    out << "            dist->distributions = d;" << std::endl;
    out << "" << std::endl;
    out << "            // done" << std::endl;
    out << "            return;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Smaller throughput with a smaller size" << std::endl;
    out << "        if (d->outputInterval > dist->outputInterval && d->size < dist->size)"
        << std::endl;
    out << "        {" << std::endl;
    out << "            // Add distribution d before dist" << std::endl;
    out << "            distn = new MinStorageDistr;" << std::endl;
    out << "            distn->outputInterval = d->outputInterval;" << std::endl;
    out << "            distn->size = d->size;" << std::endl;
    out << "            distn->distributions = d;" << std::endl;
    out << "            distn->prev = dist->prev;" << std::endl;
    out << "            distn->next = dist;" << std::endl;
    out << "            if (dist->prev != NULL)" << std::endl;
    out << "                dist->prev->next = distn;" << std::endl;
    out << "            dist->prev = distn;" << std::endl;
    out << "" << std::endl;
    out << "            if (distn->prev == NULL)" << std::endl;
    out << "                minStorageDistributions = distn;" << std::endl;
    out << "" << std::endl;
    out << "            // done" << std::endl;
    out << "            return;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Equal throughput with smaller size or" << std::endl;
    out << "        // larger throughput with smaller or equal size" << std::endl;
    out << "        if ((d->outputInterval == dist->outputInterval && d->size < dist->size)"
        << std::endl;
    out << "                || (d->outputInterval < dist->outputInterval" << std::endl;
    out << "                        && d->size <= dist->size))" << std::endl;
    out << "        {" << std::endl;
    out << "            // Distribution d is minimal, but dist is not minimal" << std::endl;
    out << "            // (and possibly other after d are also not minimal)" << std::endl;
    out << "            distn = new MinStorageDistr;" << std::endl;
    out << "            distn->outputInterval = d->outputInterval;" << std::endl;
    out << "            distn->size = d->size;" << std::endl;
    out << "            distn->distributions = d;" << std::endl;
    out << "            distn->prev = dist->prev;" << std::endl;
    out << "            if (dist->prev != NULL)" << std::endl;
    out << "                dist->prev->next = distn;" << std::endl;
    out << "" << std::endl;
    out << "            if (distn->prev == NULL)" << std::endl;
    out << "                minStorageDistributions = distn;" << std::endl;
    out << "" << std::endl;
    out << "            while (dist != NULL && (d->outputInterval < dist->outputInterval"
        << std::endl;
    out << "                    || (d->outputInterval == dist->outputInterval" << std::endl;
    out << "                        && d->size < dist->size)))" << std::endl;
    out << "                dist = dist->next;" << std::endl;
    out << "" << std::endl;
    out << "            distn->next = dist;" << std::endl;
    out << "            if (dist != NULL)" << std::endl;
    out << "                dist->prev = distn;" << std::endl;
    out << "" << std::endl;
    out << "            // done" << std::endl;
    out << "            return;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Add storage distribution to the end of the list" << std::endl;
    out << "    distn = new MinStorageDistr;" << std::endl;
    out << "    distn->outputInterval = d->outputInterval;" << std::endl;
    out << "    distn->size = d->size;" << std::endl;
    out << "    distn->distributions = d;" << std::endl;
    out << "    distn->next = NULL;" << std::endl;
    out << "" << std::endl;
    out << "    if (minStorageDistributions == NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        distn->prev = NULL;" << std::endl;
    out << "        minStorageDistributions = distn;" << std::endl;
    out << "    }" << std::endl;
    out << "    else" << std::endl;
    out << "    {" << std::endl;
    out << "        distn->prev = dist;" << std::endl;
    out << "        dist->next = distn;" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * checkedDistribution ()" << std::endl;
    out << " * The function returns true if the distribution d has been" << std::endl;
    out << " * executed before, else it returns false." << std::endl;
    out << " */" << std::endl;
    out << "bool checkedDistribution(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *dist = checkedDistributions;" << std::endl;
    out << "" << std::endl;
    out << "    while (dist != NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        bool equal = true;" << std::endl;
    out << "" << std::endl;
    out << "        // Compare distributions d and dist" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "        {" << std::endl;
    out << "            if (d->sz[c] != dist->sz[c])" << std::endl;
    out << "            {" << std::endl;
    out << "                equal = false;" << std::endl;
    out << "                break;" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Distributions equal?" << std::endl;
    out << "        if (equal)" << std::endl;
    out << "            return true;" << std::endl;
    out << "" << std::endl;
    out << "        // Next distribution" << std::endl;
    out << "        dist = dist->check;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    return false;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * checkDistribution ()" << std::endl;
    out << " * Compute the output interval of a distribution and enlarge" << std::endl;
    out << " * all channels which block execution (recursively check those" << std::endl;
    out << " * distributions). After computing the output interval, the" << std::endl;
    out << " * list of minimal storage distributions is updated." << std::endl;
    out << " */" << std::endl;
    out << "void checkDistribution(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *dn;" << std::endl;
    out << "" << std::endl;
    out << "    // Add d to list of checked distributions" << std::endl;
    out << "    d->check = checkedDistributions;" << std::endl;
    out << "    checkedDistributions = d;" << std::endl;
    out << "" << std::endl;
    out << "    // Compute output interval of this distribution" << std::endl;
    out << "    execDistribution(d);" << std::endl;
    out << "" << std::endl;
    out << "    if (d->outputInterval == lbOutputInterval" << std::endl;
    out << "            && d->size < maxDistributionSz)" << std::endl;
    out << "        maxDistributionSz = d->size;" << std::endl;
    out << "" << std::endl;
    out << "    // Add the distribution to the set of minimal distributions" << std::endl;
    out << "    updateMinimalDistributions(d);" << std::endl;
    out << "" << std::endl;
    out << "    // Enlarge each of the blocked channels" << std::endl;
    out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        if (d->block[c])" << std::endl;
    out << "        {" << std::endl;
    out << "            if (d->sz[c] < maxSz[c])" << std::endl;
    out << "            {" << std::endl;
    out << "                // Create a new distribution" << std::endl;
    out << "                dn = new Distribution;" << std::endl;
    out << "                dn->next = NULL;" << std::endl;
    out << "                dn->size = d->size + minSzStep[c];" << std::endl;
    out << "                for (int ch = 0; ch < SDF_NUM_CHANNELS; ch++)" << std::endl;
    out << "                    dn->sz[ch] = d->sz[ch];" << std::endl;
    out << "                dn->sz[c] += minSzStep[c];" << std::endl;
    out << "" << std::endl;
    out << "                // Distribution checked before or too large?" << std::endl;
    out << "                if (dn->size > maxDistributionSz || checkedDistribution(dn))"
        << std::endl;
    out << "                {" << std::endl;
    out << "                    delete dn;" << std::endl;
    out << "                }" << std::endl;
    out << "                else" << std::endl;
    out << "                {" << std::endl;
    out << "                    // Check distribution" << std::endl;
    out << "                    checkDistribution(dn);" << std::endl;
    out << "                }" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findMinimalStorageDistributions ()" << std::endl;
    out << " * The function computes all minimal storage distributions." << std::endl;
    out << " */" << std::endl;
    out << "void findMinimalStorageDistributions()" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *d;" << std::endl;
    out << "" << std::endl;
    out << "    // Check deadlock in graph" << std::endl;
    out << "    if (!graphDeadlockFree())" << std::endl;
    out << "        exit(\"Graph is not deadlock free.\",1);" << std::endl;
    out << "" << std::endl;
    out << "    // Find lower bound of the output interval" << std::endl;
    out << "    lbOutputInterval = findLbOutputInterval();" << std::endl;
    out << "" << std::endl;
    out << "    // Create distribution with lower bound" << std::endl;
    out << "    d = new Distribution;" << std::endl;
    out << "    d->next = NULL;" << std::endl;
    out << "    d->size = LB_DISTRIBUTION_SZ;" << std::endl;
    out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "        d->sz[c] = minSz[c];" << std::endl;
    out << "" << std::endl;
    out << "    // Check the distribution" << std::endl;
    out << "    checkDistribution(d);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * checkAllDistributions ()" << std::endl;
    out << " * Check all distributions within the given margins." << std::endl;
    out << " */" << std::endl;
    out << "void checkAllDistributions(short c, TBufSize usedSz, TBufSize requiredMinSz,"
        << std::endl;
    out << "        TBufSize distributionSz, MinStorageDistr *distr)" << std::endl;
    out << "{" << std::endl;
    out << "    static Distribution d;" << std::endl;
    out << "" << std::endl;
    out << "    // Last channel - end of recursion" << std::endl;
    out << "    if (c == (SDF_NUM_CHANNELS - 1))" << std::endl;
    out << "    {" << std::endl;
    out << "        d.sz[c] = distributionSz - usedSz;" << std::endl;
    out << "" << std::endl;
    out << "        // Compute output interval of this distribution" << std::endl;
    out << "        execDistribution(&d);" << std::endl;
    out << "" << std::endl;
    out << "        // Smaller than current minimum?" << std::endl;
    out << "        if (d.outputInterval < distr->outputInterval)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Remove existing distributions" << std::endl;
    out << "            while (distr->distributions != NULL)" << std::endl;
    out << "            {" << std::endl;
    out << "                Distribution *dn = distr->distributions;" << std::endl;
    out << "                distr->distributions = dn->next;" << std::endl;
    out << "                delete dn;" << std::endl;
    out << "            }" << std::endl;
    out << "" << std::endl;
    out << "            // Create first distribution with this output interval" << std::endl;
    out << "            Distribution *dn = new Distribution;" << std::endl;
    out << "            dn->next = NULL;" << std::endl;
    out << "            for (int ch = 0; ch < SDF_NUM_CHANNELS; ch++)" << std::endl;
    out << "                dn->sz[ch] = d.sz[ch];" << std::endl;
    out << "            distr->distributions = dn;" << std::endl;
    out << "            distr->outputInterval = d.outputInterval;" << std::endl;
    out << "        }" << std::endl;
    out << "        else if (d.outputInterval == distr->outputInterval)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Add alternative distribution" << std::endl;
    out << "            Distribution *dn = new Distribution;" << std::endl;
    out << "            dn->next = NULL;" << std::endl;
    out << "            for (int ch = 0; ch < SDF_NUM_CHANNELS; ch++)" << std::endl;
    out << "                dn->sz[ch] = d.sz[ch];" << std::endl;
    out << "            dn->next = distr->distributions;" << std::endl;
    out << "            distr->distributions = dn;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // done" << std::endl;
    out << "        return;" << std::endl;
    out << "" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Enlarge channel c" << std::endl;
    out << "    for (d.sz[c] = minSz[c]; (d.sz[c] + usedSz) <=" << std::endl;
    out << "            (distributionSz-(requiredMinSz-minSz[c])) && d.sz[c] <= maxSz[c];"
        << std::endl;
    out << "                d.sz[c] = d.sz[c] + minSzStep[c])" << std::endl;
    out << "    {" << std::endl;
    out << "        // usedSz must be smaller as distributionSz" << std::endl;
    out << "        if (usedSz+d.sz[c] <= distributionSz)" << std::endl;
    out << "            checkAllDistributions(c+1, usedSz+d.sz[c], requiredMinSz-minSz[c],"
        << std::endl;
    out << "                        distributionSz, distr);" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findMinimumOutputInterval ()" << std::endl;
    out << " * The function checks all storage distributions with |distribution| =" << std::endl;
    out << " * maxDistributionSz to find the minimal output interval." << std::endl;
    out << " */" << std::endl;
    out << "MinStorageDistr *findMinimumOutputInterval(TBufSize distributionSz)" << std::endl;
    out << "{" << std::endl;
    out << "    MinStorageDistr *distr = new MinStorageDistr;" << std::endl;
    out << "    TBufSize requiredMinSz = 0;" << std::endl;
    out << "" << std::endl;
    out << "    // Initialize minimal storage distribution" << std::endl;
    out << "    distr->outputInterval = TTIME_MAX;" << std::endl;
    out << "    distr->size = distributionSz;" << std::endl;
    out << "    distr->distributions = NULL;" << std::endl;
    out << "    distr->next = NULL;" << std::endl;
    out << "    distr->prev = NULL;" << std::endl;
    out << "" << std::endl;
    out << "    // Set the minimal size of all fifos" << std::endl;
    out << "#ifdef SEARCH_FULL_SPACE" << std::endl;
    out << "    if (false)" << std::endl;
    out << "#else" << std::endl;
    out << "    if (minStorageDistributions != NULL)" << std::endl;
    out << "#endif" << std::endl;
    out << "    {" << std::endl;
    out << "        for (Distribution *d = minStorageDistributions->distributions;" << std::endl;
    out << "                d != NULL; d = d->next)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Set minimal size of all channels and" << std::endl;
    out << "            // which channels must be enlarged" << std::endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                minSz[c] = d->sz[c];" << std::endl;
    out << "" << std::endl;
    out << "            // Lower bound distribution size" << std::endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "                requiredMinSz += minSz[c];" << std::endl;
    out << "" << std::endl;
    out << "            checkAllDistributions(0, 0, requiredMinSz, distributionSz, distr);"
        << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "    else" << std::endl;
    out << "    {" << std::endl;
    out << "        // Lower bound distribution size" << std::endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            requiredMinSz += minSz[c];" << std::endl;
    out << "" << std::endl;
    out << "        checkAllDistributions(0, 0, requiredMinSz, distributionSz, distr);"
        << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    return distr;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findMinimalStorageDistributions ()" << std::endl;
    out << " * The function uses linear search from the lower bound to the upper bound"
        << std::endl;
    out << " * to find all minimal storage distributions." << std::endl;
    out << " */" << std::endl;
    out << "void findMinimalStorageDistributions(TBufSize lb_distr_sz, TBufSize ub_distr_sz)"
        << std::endl;
    out << "{" << std::endl;
    out << "    TBufSize m;" << std::endl;
    out << "    MinStorageDistr *d;" << std::endl;
    out << "" << std::endl;
    out << "    // Start with lower bound" << std::endl;
    out << "    m = lb_distr_sz;" << std::endl;
    out << "" << std::endl;
    out << "    // Find lower bound of the output interval" << std::endl;
    out << "    lbOutputInterval = findLbOutputInterval();" << std::endl;
    out << "" << std::endl;
    out << "    do" << std::endl;
    out << "    {" << std::endl;
    out << "        // Find smallest output interval with distribution size m." << std::endl;
    out << "        d = findMinimumOutputInterval(m);" << std::endl;
    out << "" << std::endl;
    out << "        // Is set distributions d a set of minimal distributions?" << std::endl;
    out << "        if (minStorageDistributions == NULL ||" << std::endl;
    out << "                d->outputInterval < minStorageDistributions->outputInterval)"
        << std::endl;
    out << "        {" << std::endl;
    out << "            // Distribution d has output interval smaller than infinite?" << std::endl;
    out << "            if (d->outputInterval < TTIME_MAX)" << std::endl;
    out << "            {" << std::endl;
    out << "                // Add d to the list of minimal distributions" << std::endl;
    out << "                d->next = minStorageDistributions;" << std::endl;
    out << "                minStorageDistributions = d;" << std::endl;
    out << "            }" << std::endl;
    out << "            else" << std::endl;
    out << "            {" << std::endl;
    out << "                // Distribution d is not minimal, remove it" << std::endl;
    out << "                while(d->distributions != NULL)" << std::endl;
    out << "                {" << std::endl;
    out << "                    Distribution *t = d->distributions;" << std::endl;
    out << "                    d->distributions = t->next;" << std::endl;
    out << "                    delete t;" << std::endl;
    out << "                }" << std::endl;
    out << "                delete d;" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "        else" << std::endl;
    out << "        {" << std::endl;
    out << "            // Distribution d is not minimal, remove it" << std::endl;
    out << "            while(d->distributions != NULL)" << std::endl;
    out << "            {" << std::endl;
    out << "                Distribution *t = d->distributions;" << std::endl;
    out << "                d->distributions = t->next;" << std::endl;
    out << "                delete t;" << std::endl;
    out << "            }" << std::endl;
    out << "            delete d;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Lower bound output interval reached?" << std::endl;
    out << "        if (minStorageDistributions != NULL &&" << std::endl;
    out << "            minStorageDistributions->outputInterval <= lbOutputInterval)" << std::endl;
    out << "            return;" << std::endl;
    out << "" << std::endl;
    out << "        // Next distribution size" << std::endl;
    out << "        m++;" << std::endl;
    out << "    } while (ub_distr_sz >= m);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printMinimalDistributions ()" << std::endl;
    out << " * Output all minimal distributions to the supplied stream." << std::endl;
    out << " */" << std::endl;
    out << "void printMinimalDistributions(std::ostream &out)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"<?xml version='1.0' encoding='UTF-8'?>\" << std::endl;" << std::endl;
    out << "    out << \"<!DOCTYPE buffy >\" << std::endl;" << std::endl;
    out << "    out << \"<buffy\";" << std::endl;
    out << R"(    out << " nrChannels='" << SDF_NUM_CHANNELS << "'";)" << std::endl;
    out << "    out << \" lbDistributionSz='\" << LB_DISTRIBUTION_SZ;" << std::endl;
    out << "    out << \"' ubDistributionSz='\" << UB_DISTRIBUTION_SZ;" << std::endl;
    out << R"(    out << "' maxStates='" << maxStackPtr << "'>" << std::endl;)" << std::endl;
    out << "" << std::endl;
    out << "    for (MinStorageDistr *p = minStorageDistributions; p != NULL;" << std::endl;
    out << "            p = p->next)" << std::endl;
    out << "    {" << std::endl;
    out << "        out << \"    <minDistributions outputInterval='\" << p->outputInterval;"
        << std::endl;
    out << R"(        out << "' distributionSz='" << p->size << "'>" << std::endl;)" << std::endl;
    out << "        for (Distribution *d = p->distributions; d != NULL; d = d->next)" << std::endl;
    out << "        {" << std::endl;
    out << "            out << \"        <distribution>\" << std::endl;" << std::endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << std::endl;
    out << "            {" << std::endl;
    out << R"(                out << "            <ch id='" << c << "' sz='" << d->sz[c];)"
        << std::endl;
    out << "                out << \"'/>\" << std::endl;" << std::endl;
    out << "            }" << std::endl;
    out << "            out << \"        </distribution>\" << std::endl;" << std::endl;
    out << "        }" << std::endl;
    out << "        out << \"    </minDistributions>\" << std::endl;" << std::endl;
    out << "    }" << std::endl;
    out << "    out << \"</buffy>\" << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printMain(std::ostream &out, bool paretoSpace) {
    out << "int main(int argc, char **argv)" << std::endl;
    out << "{" << std::endl;
    out << "    // Check bounds" << std::endl;
    out << "    if (LB_DISTRIBUTION_SZ > UB_DISTRIBUTION_SZ)" << std::endl;
    out << "        exit(\"lb_distribution_sz > ub_distribution_sz\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    // Create hash and stack" << std::endl;
    out << "    createStack();" << std::endl;
    out << "    createHashTable();" << std::endl;
    out << "    createDependencyStack();" << std::endl;
    out << "" << std::endl;

    if (paretoSpace) {
        out << "    // Search the space" << std::endl;
        out << "#ifdef SEARCH_FULL_SPACE" << std::endl;
        out << "    findMinimalStorageDistributions(LB_DISTRIBUTION_SZ, UB_DISTRIBUTION_SZ);"
            << std::endl;
        out << "#else" << std::endl;
        out << "    findMinimalStorageDistributions();" << std::endl;
        out << "#endif" << std::endl;
        out << "" << std::endl;
        out << "    // Output all minimal storage distributions" << std::endl;
        out << "    printMinimalDistributions(std::cout);" << std::endl;
    } else {
        out << "    // Find the maximal throughput" << std::endl;
        out << "    TTime lbOutputInterval = findLbOutputInterval();" << std::endl;
        out << "" << std::endl;
        out << "    std::cout << \"lb output interval: \" << lbOutputInterval << std::endl;"
            << std::endl;
    }

    out << "" << std::endl;
    out << "    // Cleanup" << std::endl;
    out << "    destroyStack();" << std::endl;
    out << "    destroyHashTable();" << std::endl;
    out << "    destroyDependencyStack();" << std::endl;
    out << "" << std::endl;
    out << "    return 0;" << std::endl;
    out << "}" << std::endl;
}

} // namespace

extern void outputSDFasBuffyModel(const TimedSDFgraph &g,
                                  std::ostream &out,
                                  CString &outputActor,
                                  uint64_t stackSz,
                                  uint64_t hashSz,
                                  uint64_t depStackSz) {
    printDefinitions(g, out, stackSz, hashSz, depStackSz);
    printMiscFunctions(out);
    printState(out);
    printStack(out);
    printHash(out);
    printDependencies(out);
    printSdf(g, out, outputActor);
    printDistribution(out);
    printMain(out, true);
}

extern void outputSDFasBuffyModelThroughput(const TimedSDFgraph &g,
                                            std::ostream &out,
                                            CString &outputActor,
                                            uint64_t stackSz,
                                            uint64_t hashSz,
                                            uint64_t depStackSz) {
    printDefinitions(g, out, stackSz, hashSz, depStackSz);
    printMiscFunctions(out);
    printState(out);
    printStack(out);
    printHash(out);
    printDependencies(out);
    printSdf(g, out, outputActor);
    printDistribution(out);
    printMain(out, false);
}
} // namespace SDF
