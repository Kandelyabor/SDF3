/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy_autoconc.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 17, 2005
 *
 *  Function        :   Buffy model for throughput calculation with
 *                      auto-concurrency.
 *
 *  History         :
 *      17-11-05    :   Initial version.
 *
 * $Id: buffy_autoconc.cc,v 1.1.1.1.2.1 2010-04-22 07:18:42 mgeilen Exp $
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

#include <algorithm>

#include "sdf/base/algo/repetition_vector.h"
#include "buffy.h"


namespace SDF {

namespace{

SDFactor *findOutputActor(const TimedSDFgraph& g, RepetitionVector repVec) {
    int min = INT_MAX;
    SDFactor* a = nullptr;

    for (const auto& iter : g.getActors()) {
        if (repVec[iter->getId()] < min) {
            a = iter.get();
            min = repVec[a->getId()];
        }
    }

    return a;
}

SDFtime getMaxExecTime(const TimedSDFgraph& g) {
    SDFtime max = 0;

    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        max = (std::max)(a->getExecutionTime(), max);
    }

    return max;
}

void printDefinitions(const TimedSDFgraph& g,
                             std::ostream &out,
                             uint64_t stackSz,
                             uint64_t hashSz,
                             uint64_t repCntOutputActor) {
    out << "#define SDF_NUM_ACTORS          " << g.nrActors() << std::endl;
    out << "#define SDF_NUM_CHANNELS        " << g.nrChannels() << std::endl;
    out << "#define SDF_MAX_TIME            " << getMaxExecTime(g) << std::endl;
    out << "" << std::endl;
    out << "#define SDF_OUT_ACTOR_REP_CNT   " << repCntOutputActor << std::endl;
    out << "" << std::endl;
    out << "#define STACK_SIZE              " << stackSz << std::endl;
    out << "#define HASH_TABLE_SIZE         " << hashSz << std::endl;
    out << "" << std::endl;
    out << "typedef long TBufSize;" << std::endl;
    out << "typedef short TCnt;" << std::endl;
    out << "typedef double TTime;" << std::endl;
    out << "" << std::endl;
    out << "#define TTIME_MAX INT_MAX" << std::endl;
    out << "" << std::endl;
}

void printMiscFunctions(std::ostream &out) {
    out << "#include <math.h>" << std::endl;
    out << "#include <iostream>" << std::endl;
    out << "#include <assert.h>" << std::endl;
    out << "#include <time.h>" << std::endl;
    out << "#include <sys/resource.h>" << std::endl;
    out << "" << std::endl;
    out << "using std::ostream;" << std::endl;
    out << "using std::cout;" << std::endl;
    out << "using std::cerr;" << std::endl;
    out << "using std::endl;" << std::endl;
    out << "" << std::endl;
    out << "typedef struct _CTimer" << std::endl;
    out << "{" << std::endl;
    out << "    struct rusage rStart;" << std::endl;
    out << "    struct rusage rEnd;" << std::endl;
    out << "    struct timeval time;" << std::endl;
    out << "} CTimer;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * exit ()" << std::endl;
    out << " * Exit the program with an error message." << std::endl;
    out << " */" << std::endl;
    out << "void exit(const char *msg, const int errorno)" << std::endl;
    out << "{" << std::endl;
    out << "    std::cerr << msg << std::endl;" << std::endl;
    out << "    exit(errorno);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * startTimer ()" << std::endl;
    out << " * The function start measuring the elapsed user and system time " << std::endl;
    out << " * from this point onwards." << std::endl;
    out << " */" << std::endl;
    out << "static inline" << std::endl;
    out << "void startTimer(CTimer *t)" << std::endl;
    out << "{" << std::endl;
    out << "    getrusage(RUSAGE_SELF, &(t->rStart));" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * stopTimer ()" << std::endl;
    out << " * The function stops measuring the elapsed user and system time and" << std::endl;
    out << " * it computes the total elapsed time. This time is accessible via " << std::endl;
    out << " * t->time (struct timeval)." << std::endl;
    out << " */" << std::endl;
    out << "static inline" << std::endl;
    out << "void stopTimer(CTimer *t)" << std::endl;
    out << "{" << std::endl;
    out << "    getrusage(RUSAGE_SELF, &(t->rEnd));" << std::endl;
    out << "" << std::endl;
    out << "    // Calculate elapsed time (user + system)" << std::endl;
    out << "    t->time.tv_sec = t->rEnd.ru_utime.tv_sec - t->rStart.ru_utime.tv_sec" << std::endl;
    out << "                        + t->rEnd.ru_stime.tv_sec - t->rEnd.ru_stime.tv_sec;"
        << std::endl;
    out << "    t->time.tv_usec = t->rEnd.ru_utime.tv_usec - t->rStart.ru_utime.tv_usec "
        << std::endl;
    out << "                        + t->rEnd.ru_stime.tv_usec - t->rStart.ru_stime.tv_usec;"
        << std::endl;
    out << "" << std::endl;
    out << "    if (t->time.tv_sec >= 1e6)" << std::endl;
    out << "    {" << std::endl;
    out << "        t->time.tv_sec = t->time.tv_sec + t->time.tv_usec/long(1e6);" << std::endl;
    out << "        t->time.tv_usec = t->time.tv_usec%long(1e6);" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printTimer ()" << std::endl;
    out << " * The function prints the value of the timer in milliseconds to the supplied"
        << std::endl;
    out << " * output stream." << std::endl;
    out << " */" << std::endl;
    out << "static inline" << std::endl;
    out << "void printTimer(std::ostream &out, CTimer *t)" << std::endl;
    out << "{" << std::endl;
    out << "    double s = (t->time.tv_sec * (double)(1000)) " << std::endl;
    out << "                    + (t->time.tv_usec / (double)(1000));" << std::endl;
    out << "    " << std::endl;
    out << "    out << s << \"ms\";" << std::endl;
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
    out << "    TCnt act_clk[SDF_NUM_ACTORS][SDF_MAX_TIME+1];" << std::endl;
    out << "    TTime glb_clk;" << std::endl;
    out << "    TBufSize ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "    TBufSize ckt[SDF_NUM_CHANNELS];" << std::endl;
    out << "} State;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printState ()" << std::endl;
    out << " * The function writes the state to the supplied stream." << std::endl;
    out << " */" << std::endl;
    out << "void printState(std::ostream &out, const State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"State:\" << std::endl;" << std::endl;
    out << "    " << std::endl;
    out << "    for (int i = 0; i < SDF_NUM_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << R"(        out << "act_clk[" << i << "] = {";)" << std::endl;
    out << "        " << std::endl;
    out << "        for (int j = 0; j < SDF_MAX_TIME+1; j++)" << std::endl;
    out << "        {" << std::endl;
    out << "            if (s.act_clk[i][j] != 0)" << std::endl;
    out << R"(                out << "(" << s.act_clk[i][j] << "," << j << "), ";)" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        out << \"}\" << std::endl;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    out << \"glb_clk = \" << s.glb_clk << std::endl;" << std::endl;
    out << "    " << std::endl;
    out << "    for (int i = 0; i < SDF_NUM_CHANNELS; i++)" << std::endl;
    out << R"(        out << "ch[" << i << "] = " << s.ch[i] << std::endl;)" << std::endl;
    out << "    " << std::endl;
    out << "    for (int i = 0; i < SDF_NUM_CHANNELS; i++)" << std::endl;
    out << R"(        out << "ckt[" << i << "] = " << s.ckt[i] << std::endl;)" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearState ()" << std::endl;
    out << " * The function sets the state to zero." << std::endl;
    out << " */" << std::endl;
    out << "void clearState(State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    char *s_ptr = (char*)&s;" << std::endl;
    out << "    " << std::endl;
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
    out << "    " << std::endl;
    out << "    for (int i = 0; i < sizeof(State) >> 2; i++)" << std::endl;
    out << "        if (s1_ptr[i] != s2_ptr[i])" << std::endl;
    out << "            return false;" << std::endl;
    out << "    " << std::endl;
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
    out << "    " << std::endl;
    out << "    if (stack == nullptr)" << std::endl;
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
    out << "    stackPtr = 0;   " << std::endl;
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
    out << "    " << std::endl;
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
    out << "#define INVALID_HASH_KEY    nullptr" << std::endl;
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
    out << "    " << std::endl;
    out << "    if (hashTable == nullptr)" << std::endl;
    out << "        exit(\"Failed creating hash table.\", 1);" << std::endl;
    out << "    " << std::endl;
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
    out << "    " << std::endl;
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
    out << "    " << std::endl;
    out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)"
        << std::endl;
    out << "        {" << std::endl;
    out << "            s_next = s_cur->next;" << std::endl;
    out << "            free(s_cur);" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
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
    out << "    " << std::endl;
    out << "    for (int i = 0; i < sizeof(State); i+=4)" << std::endl;
    out << "    {" << std::endl;
    out << "        key_part = 0;" << std::endl;
    out << "        for (int j = 0; j < 4 && i+j < sizeof(State); j++)" << std::endl;
    out << "        {" << std::endl;
    out << "            key_part = key_part << 8; // radix: 128" << std::endl;
    out << "            key_part = key_part + (long long)(abs(s_ptr[i]));" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
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
    out << "    " << std::endl;
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
    out << "    " << std::endl;
    out << "    for (slot = hashTable[key]; slot != INVALID_HASH_KEY; slot = slot->next)"
        << std::endl;
    out << "    {" << std::endl;
    out << "        StackPos pos = slot->value;" << std::endl;
    out << "        " << std::endl;
    out << "        if (equalStates(stack[pos], s))" << std::endl;
    out << "            return pos;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
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
    out << "    " << std::endl;
    out << "    for (HashSlot *slot = hashTable[key]; slot != nullptr; slot = slot->next)"
        << std::endl;
    out << "    {" << std::endl;
    out << "        printState(out, stack[slot->value]);" << std::endl;
    out << "        out << std::endl;" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << std::endl;
}

void printSdfHeader(const TimedSDFgraph& g, std::ostream &out, CString &outputActor) {
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
            out << "        if (stack[ptr].act_clk[" << a->getId() << "][0] > 0)" << std::endl;
            out << "            nr_fire++;" << std::endl;
        }
    }

    out << "" << std::endl;
    out << "        // Time between previous state" << std::endl;
    out << "        time += stack[ptr].glb_clk;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    return time/(TTime)(nr_fire);" << std::endl;
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
    out << "        " << std::endl;
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
    out << "#define ACT_CLK(a,t)        sdfState.act_clk[a][t]" << std::endl;
    out << "#define CH(c)               sdfState.ch[c]" << std::endl;
    out << "#define CKT(c)              sdfState.ckt[c]" << std::endl;
    out << "" << std::endl;
    out << "#define FIRE_ACT(a,t)       ACT_CLK(a,t)++;" << std::endl;
    out << "#define FIRE_ACT_END(a)     ACT_CLK(a,0)--;" << std::endl;
    out << "#define CH_TOKENS(c,n)      (CH(c)-CKT(c) >= n)" << std::endl;
    out << "#define CK_TOKENS(c,n)      CKT(c) = CKT(c) + n;" << std::endl;
    out << "#define CONSUME(c,n)        CH(c) = CH(c) - n; CKT(c) = CKT(c) - n;" << std::endl;
    out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << std::endl;
    out << "#define ACT_END_FIRE(a)     (ACT_CLK(a,0) != 0)" << std::endl;
    out << "#define ADVANCE_CLK         GLB_CLK = GLB_CLK + 1;" << std::endl;
    out << "#define NEXT_ITER           GLB_CLK = 0;" << std::endl;
    out << "#define LOWER_CLK(a)        memmove(&ACT_CLK(a,0),&ACT_CLK(a,1), \\" << std::endl;
    out << "                                sizeof(TCnt)*SDF_MAX_TIME); \\" << std::endl;
    out << "                            ACT_CLK(a,SDF_MAX_TIME) = 0;" << std::endl;
    out << "" << std::endl;
    out << "State sdfState;" << std::endl;
    out << "State prevState;" << std::endl;
    out << "" << std::endl;
}

void printExecSdfGraph(const TimedSDFgraph& g, std::ostream &out, CString &outputActor) {
    out << "/**" << std::endl;
    out << " * execSDFGraph()" << std::endl;
    out << " * Execute the SDF graph till a deadlock is found or a recurrent state." << std::endl;
    out << " * The output interval (i.e. inverse of throughput) is returned." << std::endl;
    out << " */" << std::endl;
    out << "TTime execSDFGraph()" << std::endl;
    out << "{" << std::endl;
    out << "    TTime output_interval;" << std::endl;
    out << "    int repCnt = 0;" << std::endl;
    out << "    " << std::endl;
    out << "    // Create initial state and add it to stack" << std::endl;
    out << "    clearState(sdfState);" << std::endl;
    out << "    copyState(prevState, sdfState);" << std::endl;
    out << "    prevState.glb_clk = TTIME_MAX;" << std::endl;
    out << "    " << std::endl;
    out << "    // Initial tokens" << std::endl;

    for (const auto& ch : g.getChannels()) {
        if (ch->getInitialTokens() != 0) {
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
        out << "        while (ACT_END_FIRE(" << a->getId() << "))" << std::endl;
        out << "        {" << std::endl;

        if (outputActor == a->getName()) {
            out << "            repCnt++;" << std::endl;
            out << "            if (repCnt == SDF_OUT_ACTOR_REP_CNT)" << std::endl;
            out << "            {" << std::endl;
            out << "                // Add state to hash of visited states" << std::endl;
            out << "                output_interval = storeState(sdfState);" << std::endl;
            out << "                if (output_interval != -1)" << std::endl;
            out << "                    return output_interval;" << std::endl;
            out << "                NEXT_ITER;" << std::endl;
            out << "                repCnt = 0;" << std::endl;
            out << "            }" << std::endl;
            out << "            " << std::endl;
        }

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

        out << "            FIRE_ACT_END(" << a->getId() << ");" << std::endl;
        out << "        }" << std::endl;
    }
    out << "        " << std::endl;

    out << "        // Start actor firings" << std::endl;
    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        out << "        while (true";
        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << " && CH_TOKENS(" << ch->getId() << "," << p->getRate() << ")";
            }
        }
        out << ")" << std::endl;

        out << "        {" << std::endl;
        out << "            // Claim tokens on each input" << std::endl;
        for (const auto& p : a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "            CK_TOKENS(" << ch->getId() << "," << p->getRate() << ");"
                    << std::endl;
            }
        }
        out << "            " << std::endl;
        out << "            // Fire the actor" << std::endl;
        out << "            FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << std::endl;
        out << "        }" << std::endl;
    }
    out << "        " << std::endl;

    out << "        // Lower clocks" << std::endl;
    for (const auto& a : g.getActors()) {
        out << "        LOWER_CLK(" << a->getId() << ");" << std::endl;
    }
    out << "" << std::endl;

    out << "        // Check for progress (i.e. no deadlock)" << std::endl;
    out << "        if (equalStates(prevState, sdfState))" << std::endl;
    out << "            return TTIME_MAX;" << std::endl;
    out << "        " << std::endl;

    out << "        // Advance the global clock" << std::endl;
    out << "        ADVANCE_CLK;" << std::endl;
    out << "        " << std::endl;

    out << "        // Store state to check for progress" << std::endl;
    out << "        copyState(prevState, sdfState);" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printSdf(const TimedSDFgraph& g, std::ostream &out, CString &outputActor) {
    printSdfHeader(g, out, outputActor);
    printExecSdfGraph(g, out, outputActor);
}

void printMain(std::ostream &out) {
    out << "int main(int argc, char **argv)" << std::endl;
    out << "{" << std::endl;
    out << "    // Create hash and stack" << std::endl;
    out << "    createStack();" << std::endl;
    out << "    createHashTable();" << std::endl;
    out << "    " << std::endl;
    out << "    // Find the maximal throughput" << std::endl;
    out << "    std::cout << \"thr = \" << execSDFGraph() << std::endl;" << std::endl;
    out << "    " << std::endl;
    out << "    // Cleanup" << std::endl;
    out << "    destroyStack();" << std::endl;
    out << "    destroyHashTable();" << std::endl;
    out << "    " << std::endl;
    out << "    return 0;" << std::endl;
    out << "}" << std::endl;
}

} // namespace

/**
 * outputSDFasStateSpaceThroughputModel ()
 * Output the SDF graph as a buffy model to find the maximal
 * throughput for unconstrained buffer sizes and using auto-concurrency.
 */
extern void outputSDFasStateSpaceThroughputModel(const TimedSDFgraph& g,
                                                 std::ostream &out,
                                                 uint64_t stackSz,
                                                 uint64_t hashSz) {
    SDFactor *outputActor = nullptr;
    RepetitionVector repVec;
    CString outActor;

    // Find the output actor and its repetition count
    repVec = computeRepetitionVector(g);
    outputActor = findOutputActor(g, repVec);
    outActor = outputActor->getName();

    printDefinitions(g, out, stackSz, hashSz, repVec[outputActor->getId()]);
    printMiscFunctions(out);
    printState(out);
    printStack(out);
    printHash(out);
    printSdf(g, out, outActor);
    printMain(out);
}
} // namespace SDF
