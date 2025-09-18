/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 25, 2007
 *
 *  Function        :   Buffer sizing for CSDFGs
 *
 *  History         :
 *      25-06-07    :   Initial version.
 *
 * $Id: buffy.cc,v 1.3.2.1 2010-04-25 01:21:16 mgeilen Exp $
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

#include <algorithm>
namespace CSDF {

namespace {

void printMinimalChannelSzStep(const TimedCSDFgraph& g, std::ostream &out) {
    bool first = true;

    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        uint minStepSz = 0;
        CSDFchannel& ch = *(*iter);
        CSDFport *srcPort = ch.getSrcPort();
        CSDFport *dstPort = ch.getDstPort();

        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate().at(0);

        // Step size is equal to the gcd of all production and consumption
        // rates that are possible
        for (unsigned int i : srcPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }
        for (unsigned int i : dstPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }

        if (first) {
            out << "{";
            first = false;
        } else {
            out << ", ";
        }
        out << "Q * " << minStepSz;
    }
    out << "}";
}

void printMinimalChannelSz(const TimedCSDFgraph& g, std::ostream &out) {
    bool first = true;
    uint minSzCh = 0;

    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        CSDFchannel& ch = *(*iter);
        CSDFport *srcPort = ch.getSrcPort();
        CSDFport *dstPort = ch.getDstPort();
        uint ratePeriod =
                static_cast<uint>(gcd(srcPort->getRate().size(), dstPort->getRate().size()));
        uint minStepSz = 0;

        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate().at(0);

        // Step size is equal to the gcd of all production and consumption
        // rates that are possible
        for (unsigned int i : srcPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }
        for (unsigned int i : dstPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }

        // Initialize lower bound to maximal size
        minSzCh = UINT_MAX;

        // Iterate over all production/consumption rate combinations to find
        // lower bound as given for SDFG case.
        for (uint i = 0; i < ratePeriod; i++) {
            uint p = srcPort->getRate().at(static_cast<int>(i % srcPort->getRate().size()));
            uint c = dstPort->getRate().at(static_cast<int>(i % dstPort->getRate().size()));
            uint t = ch.getInitialTokens();
            uint lb = 0;

            // Lower-bound of a self-edge is rate at which data is produced and
            // consumed and the number of initial tokens present
            if (ch.getSrcActor()->getId() == ch.getDstActor()->getId()) {
                lb = p + (c > t ? c : t);
            } else {
                if (gcd(p, c) != 0) {
                    lb = p + c - gcd(p, c) + t % gcd(p, c);
                } else {
                    lb = p + c - gcd(p, c);
                }
                lb = (lb > t ? lb : t);
            }

            minSzCh = (std::min)(lb, minSzCh);
        }

        // Minimal channel size must be a multiple of the minStepSz
        minSzCh = minSzCh - (minSzCh % minStepSz);

        if (first) {
            first = false;
            out << "{" << minSzCh;
        } else {
            out << ", " << minSzCh;
        }
    }
    out << "}";
}

uint getLbDistributionSz(const TimedCSDFgraph& g) {
    uint minSz = 0;
    uint minSzCh = 0;

    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        CSDFchannel& ch = *(*iter);
        CSDFport *srcPort = ch.getSrcPort();
        CSDFport *dstPort = ch.getDstPort();
        uint ratePeriod =
                static_cast<uint>(gcd(srcPort->getRate().size(), dstPort->getRate().size()));
        uint minStepSz = 0;

        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate().at(0);

        // Step size is equal to the gcd of all production and consumption
        // rates that are possible
        for (unsigned int i : srcPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }
        for (unsigned int i : dstPort->getRate()) {
            minStepSz = gcd(minStepSz, i);
        }

        // Initialize lower bound to maximal size
        minSzCh = UINT_MAX;

        // Iterate over all production/consumption rate combinations to find
        // lower bound as given for SDFG case.
        for (uint i = 0; i < ratePeriod; i++) {
            uint p = srcPort->getRate().at(static_cast<int>(i % srcPort->getRate().size()));
            uint c = dstPort->getRate().at(static_cast<int>(i % dstPort->getRate().size()));
            uint t = ch.getInitialTokens();
            uint lb = 0;

            // Lower-bound of a self-edge is rate at which data is produced and
            // consumed and the number of initial tokens present
            if (ch.getSrcActor()->getId() == ch.getDstActor()->getId()) {
                lb = p + (c > t ? c : t);
            } else {
                if (gcd(p, c) != 0) {
                    lb = p + c - gcd(p, c) + t % gcd(p, c);
                } else {
                    lb = p + c - gcd(p, c);
                }
                lb = (lb > t ? lb : t);
            }

            minSzCh = (std::min)(lb, minSzCh);
        }

        // Minimal channel size must be a multiple of the minStepSz
        minSzCh = minSzCh - (minSzCh % minStepSz);

        minSz += minSzCh;
    }

    return minSz;
}

CId getOutputActor(const TimedCSDFgraph& g) {
    CSDFgraph::RepetitionVector repVec = g.getRepetitionVector();
    int min = INT_MAX;
    const CSDFactor *a = nullptr;

    for (auto iter = g.actorsBegin(); iter != g.actorsEnd(); iter++) {
        if (repVec[(*iter)->getId()] < min) {
            a = (*iter).get();
            min = repVec[a->getId()];
        }
    }

    return a->getId();
}

uint getOutputActorRepCnt(const TimedCSDFgraph& g) {
    CSDFgraph::RepetitionVector repVec = g.getRepetitionVector();
    int min = INT_MAX;
    CSDFactor *a = nullptr;

    for (auto iter = g.actorsBegin(); iter != g.actorsEnd(); iter++) {
        if (repVec[(*iter)->getId()] < min) {
            a = (*iter).get();
            min = repVec[a->getId()];
        }
    }

    return repVec[a->getId()];
}

double getMaxThroughput(const TimedCSDFgraph& g) {
    return CSDF::CSDFstateSpaceThroughputAnalysis::analyze(g);
}

uint computeGraphPeriod(const TimedCSDFgraph& g) {
    uint period = 1;

    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        CSDFchannel& c = *(*iter);

        period = lcm(period, static_cast<uint>(c.getSrcPort()->getRate().size()));
        period = lcm(period, static_cast<uint>(c.getDstPort()->getRate().size()));
    }

    for (auto iter = g.actorsBegin(); iter != g.actorsEnd(); iter++) {
        auto *a = dynamic_cast<TimedCSDFactor *>((*iter).get());

        period = lcm(period, static_cast<uint>(a->getExecutionTime().size()));
    }

    return period;
}

uint computeNrActorsHSDFG(const TimedCSDFgraph& g) {
    CSDFgraph::RepetitionVector repVec = g.getRepetitionVector();
    uint cnt = 0;

    for (int i : repVec) {
        cnt += i;
    }

    return cnt;
}

void printDefinitions(const TimedCSDFgraph& g, std::ostream &out) {
    bool first = false;
    uint period = computeGraphPeriod(g);

    out << "#include <float.h>" << std::endl;
    out << "#include <math.h>" << std::endl;
    out << "#include <assert.h>" << std::endl;
    out << "#include <iostream>" << std::endl;
    out << "#include <list>" << std::endl;
    out << "#include <time.h>" << std::endl;
    out << "#include <sys/resource.h>" << std::endl;
    out << "" << std::endl;
    out << "using namespace std;" << std::endl;
    out << "" << std::endl;

    out << "typedef unsigned int   TCnt;" << std::endl;
    out << "typedef unsigned int   TTime;" << std::endl;
    out << "typedef unsigned int   TBufSize;" << std::endl;
    out << "typedef unsigned short CId;" << std::endl;
    out << "typedef double TDtime;" << std::endl;
    out << std::endl;

    out << "// Quantization factor of the step size" << std::endl;
    out << "#ifndef Q" << std::endl;
    out << "#define Q 1" << std::endl;
    out << "#endif" << std::endl;
    out << std::endl;

    out << "#define TDTIME_MAX              DBL_MAX" << std::endl;
    out << "#define TBUFSIZE_MAX            ULONG_MAX" << std::endl;
    out << std::endl;

    out << "#define max(a,b)                ((a)>(b) ? (a) : (b))" << std::endl;
    out << "#define min(a,b)                ((a)<(b) ? (a) : (b))" << std::endl;
    out << std::endl;

    out << "#define SRC                     0" << std::endl;
    out << "#define DST                     1" << std::endl;
    out << "" << std::endl;

    out << "#define NR_ACTORS               " << g.nrActors() << std::endl;
    out << "#define NR_CHANNELS             " << g.nrChannels() << std::endl;
    out << "#define PERIOD                  " << period << std::endl;
    out << std::endl;

    out << "#define NR_ACTORS_HSDFG         " << computeNrActorsHSDFG(g) << std::endl;
    out << std::endl;

    out << "uint channel[" << g.nrChannels() << "][2] = {";

    first = true;
    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        CSDFchannel& c = *(*iter);

        if (!first) {
            out << ", ";
        }
        first = false;

        out << "{" << c.getSrcActor()->getId() << ", ";
        out << c.getDstActor()->getId() << "}";
    }

    out << "};" << std::endl;

    out << "uint channelInitialTokens[" << g.nrChannels() << "] = {";

    first = true;
    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        CSDFchannel& c = *(*iter);

        if (!first) {
            out << ", ";
        }
        first = false;

        out << c.getInitialTokens();
    }

    out << "};" << std::endl;

    out << "uint channelRate[" << g.nrChannels() << "][2][" << period << "] = {";

    first = true;
    for (auto iter = g.channelsBegin(); iter != g.channelsEnd(); iter++) {
        CSDFchannel& c = *(*iter);

        if (!first) {
            out << ", ";
        }
        first = false;

        out << "{";

        out << "{";
        for (int i = 0; i < period; i++) {
            if (i != 0) {
                out << ", ";
            }

            out << c.getSrcPort()->getRate().at(i);
        }
        out << "}";

        out << ", ";

        out << "{"; for (int i = 0; i < period; i++) {
            if (i != 0) {
                out << ", ";
            }

            out << c.getDstPort()->getRate().at(i);
        }
        out << "}";

        out << "}";
    }

    out << "};" << std::endl;

    out << "uint actorExecTime[" << g.nrActors() << "][" << period << "] = {";

    first = true;
    for (auto iter = g.actorsBegin(); iter != g.actorsEnd(); iter++) {
        auto *a = dynamic_cast<TimedCSDFactor *>((*iter).get());

        if (!first) {
            out << ",";
        }
        first = false;

        out << "{";

        for (int i = 0; i < period; i++) {
            if (i != 0) {
                out << ", ";
            }

            out << a->getExecutionTime()[i];
        }

        out << "}";
    }

    out << "};" << std::endl;
    out << std::endl;

    out << "TBufSize minSz[NR_CHANNELS] = ";
    printMinimalChannelSz(g, out);
    out << ";" << std::endl;

    out << "TBufSize minSzStep[NR_CHANNELS] = ";
    printMinimalChannelSzStep(g, out);
    out << ";" << std::endl;

    constexpr std::streamsize Precision = 30;
    out.precision(Precision);
    out << "TBufSize lbDistributionSz = " << getLbDistributionSz(g) << ";" << std::endl;
    out << "TDtime maxThroughput = " << getMaxThroughput(g) << ";" << std::endl;
    out << "CId outputActor = " << getOutputActor(g) << ";" << std::endl;
    out << "TCnt outputActorRepCnt = " << getOutputActorRepCnt(g) << ";" << std::endl;
    out << "" << std::endl;
}

void printMiscFunctions(std::ostream &out) {
    out << "// Performance counters" << std::endl;
    out << "unsigned long long maxNrStatesVisited = 0, maxNrStatesStored = 0;" << std::endl;
    out << "unsigned long long nrDistributionsChecked = 0;" << std::endl;
    out << "unsigned long long nrStatesVisited, nrStatesStored;" << std::endl;
    out << "" << std::endl;
    out << "// Timer" << std::endl;
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
    out << "static" << std::endl;
    out << "void exit(const char *msg, const int errorno)" << std::endl;
    out << "{" << std::endl;
    out << "    cerr << msg << std::endl;" << std::endl;
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
    out << " * it computes the total elapsed time. This time is accesible via " << std::endl;
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
    out << "typedef struct _StateActorFiring" << std::endl;
    out << "{" << std::endl;
    out << "    TTime execTime;" << std::endl;
    out << "    TCnt  seqPos;" << std::endl;
    out << "} StateActorFiring;" << std::endl;
    out << "" << std::endl;
    out << "bool operator==(const StateActorFiring &s1, const StateActorFiring &s2)" << std::endl;
    out << "{" << std::endl;
    out << "    if (s1.execTime != s2.execTime || s1.seqPos != s2.seqPos)" << std::endl;
    out << "        return false;" << std::endl;
    out << "        " << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "typedef struct _State" << std::endl;
    out << "{" << std::endl;
    out << "    std::list<StateActorFiring> actClk[NR_ACTORS];" << std::endl;
    out << "    TCnt actSeqPos[NR_ACTORS];" << std::endl;
    out << "    TBufSize ch[NR_CHANNELS];" << std::endl;
    out << "    TBufSize sp[NR_CHANNELS];" << std::endl;
    out << "    unsigned long glbClk;  " << std::endl;
    out << "} State;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printState ()" << std::endl;
    out << " * Print the state to the supplied stream." << std::endl;
    out << " */" << std::endl;
    out << "void printState(const State &s, std::ostream &out)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"### State ###\" << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << R"(        out << "actClk[" << i << "] =";)" << std::endl;
    out << "        " << std::endl;
    out << "        for (std::list<StateActorFiring>::const_iterator iter = s.actClk[i].begin();"
        << std::endl;
    out << "                iter != s.actClk[i].end(); iter++)" << std::endl;
    out << "        {" << std::endl;
    out << R"(            out << " (" << (*iter).execTime << ", ";)" << std::endl;
    out << "            out << (*iter).seqPos << \"),\";" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        out << std::endl;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << R"(        out << "actSeqPos[" << i << "] = " << s.actSeqPos[i] << std::endl;)"
        << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << R"(        out << "ch[" << i << "] = " << s.ch[i] << std::endl;)" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << R"(        out << "sp[" << i << "] = " << s.sp[i] << std::endl;)" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    out << \"glbClk = \" << s.glbClk << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearState ()" << std::endl;
    out << " * The function sets the state to zero." << std::endl;
    out << " */" << std::endl;
    out << "void clearState(State &s)" << std::endl;
    out << "{" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        s.actClk[i].clear();" << std::endl;
    out << "        s.actSeqPos[i] = 0;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        s.ch[i] = 0;" << std::endl;
    out << "        s.sp[i] = 0;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    s.glbClk = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * equalStates ()" << std::endl;
    out << " * The function compares to states and returns true if they are equal." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "bool equalStates(const State &s1, const State &s2)" << std::endl;
    out << "{" << std::endl;
    out << "    if (s1.glbClk != s2.glbClk)" << std::endl;
    out << "        return false;" << std::endl;
    out << "    " << std::endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        if (s1.ch[i] != s2.ch[i] || s1.sp[i] != s2.sp[i])" << std::endl;
    out << "            return false;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        if (s1.actClk[i] != s2.actClk[i])" << std::endl;
    out << "            return false;" << std::endl;
    out << "        " << std::endl;
    out << "        if (s1.actSeqPos[i] != s2.actSeqPos[i])" << std::endl;
    out << "            return false;" << std::endl;
    out << "    }" << std::endl;
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
    out << "    to.glbClk = from.glbClk;" << std::endl;
    out << "    " << std::endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        to.ch[i] = from.ch[i];" << std::endl;
    out << "        to.sp[i] = from.sp[i];" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        to.actClk[i] = from.actClk[i];" << std::endl;
    out << "        to.actSeqPos[i] = from.actSeqPos[i];" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/******************************************************************************"
        << std::endl;
    out << " * States" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "typedef std::list<State>         States;" << std::endl;
    out << "typedef States::iterator    StatesIter;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * storedStates" << std::endl;
    out << " * List of visited states that are stored." << std::endl;
    out << " */" << std::endl;
    out << "static States storedStates;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * storeState ()" << std::endl;
    out << " * The function stores the state s on whenever s is not already in the" << std::endl;
    out << " * list of storedStates. When s is stored, the function returns true. When the"
        << std::endl;
    out << " * state s is already in the list, the state s is not stored. The function"
        << std::endl;
    out << " * returns false. The function always sets the pos variable to the position"
        << std::endl;
    out << " * where the state s is in the list." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "bool storeState(State &s, StatesIter &pos)" << std::endl;
    out << "{" << std::endl;
    out << "    // Find state in the list of stored states" << std::endl;
    out << "    for (StatesIter iter = storedStates.begin();" << std::endl;
    out << "            iter != storedStates.end(); iter++)" << std::endl;
    out << "    {" << std::endl;
    out << "        State &x = *iter;" << std::endl;
    out << "        " << std::endl;
    out << "        // State s at position iter in the list?" << std::endl;
    out << "        if (equalStates(x, s))" << std::endl;
    out << "        {" << std::endl;
    out << "            pos = iter;" << std::endl;
    out << "            return false;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // State not found, store it at the end of the list" << std::endl;
    out << "    storedStates.push_back(s);" << std::endl;
    out << "    " << std::endl;
    out << "    // Added state to the end of the list" << std::endl;
    out << "    pos = storedStates.end();" << std::endl;
    out << "    " << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clearStoredStates ()" << std::endl;
    out << " * The function clears the list of stored states." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void clearStoredStates()" << std::endl;
    out << "{" << std::endl;
    out << "    storedStates.clear();" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printDependencies(std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * Dependencies" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * dfsVisitDependencies ()" << std::endl;
    out << " * The function performs a DFS on the abstract dependency graph from a node a to"
        << std::endl;
    out << " * find all cycles of which a is part. Channels on a cycle are when needed"
        << std::endl;
    out << " * marked to have a storage dependency." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void dfsVisitDependencies(uint a, int *color, int *pi," << std::endl;
    out << "        bool **abstractDepGraph, bool *dep)" << std::endl;
    out << "{" << std::endl;
    out << "    uint c, d;" << std::endl;
    out << "" << std::endl;
    out << "    // Mark node a as visited" << std::endl;
    out << "    color[a] = 1;" << std::endl;
    out << "    " << std::endl;
    out << "    // Visit all nodes reachable from a" << std::endl;
    out << "    for (uint b = 0; b < NR_ACTORS; b++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Node b reachable from a?" << std::endl;
    out << "        if (abstractDepGraph[a][b])" << std::endl;
    out << "        {" << std::endl;
    out << "            // Visited node b before?" << std::endl;
    out << "            if (color[b] == 1)" << std::endl;
    out << "            {" << std::endl;
    out << "                // Found a cycle in the graph containing node b" << std::endl;
    out << "                c = a;" << std::endl;
    out << "                d = b;           " << std::endl;
    out << "                do" << std::endl;
    out << "                {" << std::endl;
    out << "                    // All channels from d to c in the SDFG have " << std::endl;
    out << "                    // storage dependency" << std::endl;
    out << "                    for (uint ch = 0; ch < NR_CHANNELS; ch++)" << std::endl;
    out << "                    {" << std::endl;
    out << "                        CId srcId = channel[ch][SRC];" << std::endl;
    out << "                        CId dstId = channel[ch][DST];" << std::endl;
    out << "                        " << std::endl;
    out << "                        if (dstId == d && srcId == c)" << std::endl;
    out << "                            dep[ch] = true;" << std::endl;
    out << "                    }" << std::endl;
    out << "" << std::endl;
    out << "                    // Next" << std::endl;
    out << "                    d = c;" << std::endl;
    out << "                    c = pi[d];" << std::endl;
    out << "                } while (d != b);" << std::endl;
    out << "            }" << std::endl;
    out << "            else" << std::endl;
    out << "            {" << std::endl;
    out << "                // Visit node b" << std::endl;
    out << "                pi[b] = a;" << std::endl;
    out << "                dfsVisitDependencies(b, color, pi, abstractDepGraph, dep);"
        << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Discovered all cycles which actor a is part of. Remove its edges to avoid"
        << std::endl;
    out << "    // re-discovering same cycles again." << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        abstractDepGraph[i][a] = false;" << std::endl;
    out << "        abstractDepGraph[a][i] = false;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Mark node a as not visited" << std::endl;
    out << "    color[a] = 0;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findStorageDependencies ()" << std::endl;
    out << " * The function find all cycles in the abstract dependency graph. Any channel"
        << std::endl;
    out << " * modeling storage space which is part of a cycle is marked to have a storage"
        << std::endl;
    out << " * dependency." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void findStorageDependencies(bool **abstractDepGraph, bool *dep)" << std::endl;
    out << "{" << std::endl;
    out << "    int *color, *pi;" << std::endl;
    out << "    " << std::endl;
    out << "    // Initialize DFS data structures" << std::endl;
    out << "    color = new int [NR_ACTORS];" << std::endl;
    out << "    pi = new int [NR_ACTORS];" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "        color[i] = 0;" << std::endl;
    out << "        " << std::endl;
    out << "    // Initialize storage dependencies" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "        dep[c] = false;" << std::endl;
    out << "    " << std::endl;
    out << "    // DFS from every node in the graph to find all cycles" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        pi[i] = i;" << std::endl;
    out << "        dfsVisitDependencies(i, color, pi, abstractDepGraph, dep);" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // Cleanup" << std::endl;
    out << "    delete [] color;" << std::endl;
    out << "    delete [] pi;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printCSDFGheader(std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * SDF" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "#define CH(c)               currentState.ch[c]" << std::endl;
    out << "#define SP(c)               currentState.sp[c]  " << std::endl;
    out << "#define CH_TOKENS(c,n)      (CH(c) >= n)  " << std::endl;
    out << "#define CH_SPACE(c,n)       (SP(c) >= n)" << std::endl;
    out << "#define CONSUME(c,n)        CH(c) = CH(c) - n;" << std::endl;
    out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << std::endl;
    out << "#define CONSUME_SP(c,n)     SP(c) = SP(c) - n;" << std::endl;
    out << "#define PRODUCE_SP(c,n)     SP(c) = SP(c) + n;" << std::endl;
    out << "" << std::endl;
    out << "#define ACT_SEQ_POS(a)      currentState.actSeqPos[a]" << std::endl;
    out << "#define ADV_ACT_SEQ_POS(a)  ACT_SEQ_POS(a) = (ACT_SEQ_POS(a) + 1) % PERIOD;"
        << std::endl;
    out << "" << std::endl;
    out << "#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)  " << std::endl;
    out << "#define CH_SPACE_PREV(c,n)  (previousState.sp[c] >= n)" << std::endl;
    out << "" << std::endl;
    out << "static State currentState;  " << std::endl;
    out << "static State previousState;  " << std::endl;
    out << "" << std::endl;
    out << "/**  " << std::endl;
    out << " * computeThroughput ()  " << std::endl;
    out << " * The function calculates the throughput of the states on the cycle. Its  "
        << std::endl;
    out << " * value is equal to the average number of firings of an actor per time unit."
        << std::endl;
    out << " */  " << std::endl;
    out << "static inline  " << std::endl;
    out << "TDtime computeThroughput(const StatesIter cycleIter)  " << std::endl;
    out << "{  " << std::endl;
    out << "    int nr_fire = 0;" << std::endl;
    out << "    TDtime time = 0;  " << std::endl;
    out << "" << std::endl;
    out << "    // Check all state from stack till cycle complete  " << std::endl;
    out << "    for (StatesIter iter = cycleIter; iter != storedStates.end(); iter++)" << std::endl;
    out << "    {" << std::endl;
    out << "        State &s = *iter;" << std::endl;
    out << "        " << std::endl;
    out << "        // Number of states in cycle is equal to number of iterations " << std::endl;
    out << "        // in the period" << std::endl;
    out << "        nr_fire++;" << std::endl;
    out << "        " << std::endl;
    out << "        // Time between previous state  " << std::endl;
    out << "        time += s.glbClk;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    return (TDtime)(nr_fire)/(time);  " << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * actorReadyToFire ()" << std::endl;
    out << " * The function returns true when the actor is ready to fire in state" << std::endl;
    out << " * s. Else it returns false." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "bool actorReadyToFire(const uint a)" << std::endl;
    out << "{" << std::endl;
    out << "    // Check all input ports for tokens and output ports" << std::endl;
    out << "    // for space" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Actor is destination of the channel?" << std::endl;
    out << "        if (channel[c][DST] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            if (!CH_TOKENS(c, channelRate[c][DST][ACT_SEQ_POS(a)]))" << std::endl;
    out << "            {" << std::endl;
    out << "                return false;" << std::endl;
    out << "            }    " << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Actor is source of the channel" << std::endl;
    out << "        if (channel[c][SRC] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            if (!CH_SPACE(c, channelRate[c][SRC][ACT_SEQ_POS(a)]))" << std::endl;
    out << "            {" << std::endl;
    out << "                return false;" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * startActorFiring ()" << std::endl;
    out << " * Start the actor firing. Remove tokens from all input channels and" << std::endl;
    out << " * reserve space on all output channels. Finally, add the actor firing" << std::endl;
    out << " * to the list of active actor firings and advance sequence position." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void startActorFiring(const uint a)" << std::endl;
    out << "{" << std::endl;
    out << "    StateActorFiring actFiring;" << std::endl;
    out << "    bool addedFiring = false;" << std::endl;
    out << "" << std::endl;
    out << "    // Set properties of this actor firing" << std::endl;
    out << "    actFiring.execTime = actorExecTime[a][ACT_SEQ_POS(a)];" << std::endl;
    out << "    actFiring.seqPos = ACT_SEQ_POS(a);" << std::endl;
    out << "    " << std::endl;
    out << "    // Consume tokens from inputs and space for output tokens" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Actor is destination of the channel?" << std::endl;
    out << "        if (channel[c][DST] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            CONSUME(c, channelRate[c][DST][ACT_SEQ_POS(a)]);" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Actor is source of the channel" << std::endl;
    out << "        if (channel[c][SRC] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            CONSUME_SP(c, channelRate[c][SRC][ACT_SEQ_POS(a)]);" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Add actor firing to the list of active firings of this actor" << std::endl;
    out << "    for (std::list<StateActorFiring>::iterator iter = currentState.actClk[a].begin();"
        << std::endl;
    out << "                iter != currentState.actClk[a].end(); iter++)" << std::endl;
    out << "    {" << std::endl;
    out << "        if ((*iter).execTime >= actFiring.execTime)" << std::endl;
    out << "        {" << std::endl;
    out << "            currentState.actClk[a].insert(iter, actFiring);" << std::endl;
    out << "            addedFiring = true;" << std::endl;
    out << "            break;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "    if (!addedFiring)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Not yet inserted, put it at the end" << std::endl;
    out << "        currentState.actClk[a].push_back(actFiring);" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Advance the sequence position of the actor" << std::endl;
    out << "    ADV_ACT_SEQ_POS(a);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * actorReadyToEnd ()" << std::endl;
    out << " * The function returns true when the actor is ready to end its firing. Else"
        << std::endl;
    out << " * the function returns false." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "bool actorReadyToEnd(const uint a)" << std::endl;
    out << "{" << std::endl;
    out << "    // Actor currently not firing?" << std::endl;
    out << "    if (currentState.actClk[a].empty())" << std::endl;
    out << "        return false;" << std::endl;
    out << "    " << std::endl;
    out << "    // First actor firing in sorted list has execution time left?" << std::endl;
    out << "    if (currentState.actClk[a].front().execTime != 0)" << std::endl;
    out << "        return false;" << std::endl;
    out << "    " << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * endActorFiring ()" << std::endl;
    out << " * Produce tokens on all output channels and release space on all input" << std::endl;
    out << " * channels. Finally, remove the actor firing from the list of active" << std::endl;
    out << " * firings." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "void endActorFiring(const uint a)" << std::endl;
    out << "{" << std::endl;
    out << "    StateActorFiring &actFiring = currentState.actClk[a].front();" << std::endl;
    out << "   " << std::endl;
    out << "    // Produce tokens on outputs and space on inputs" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Actor is source of the channel?" << std::endl;
    out << "        if (channel[c][SRC] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            PRODUCE(c, channelRate[c][SRC][actFiring.seqPos]);" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Actor is destination of the channel?" << std::endl;
    out << "        if (channel[c][DST] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            PRODUCE_SP(c, channelRate[c][DST][actFiring.seqPos]);" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Remove the firing from the list of active actor firings" << std::endl;
    out << "    currentState.actClk[a].pop_front();" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * clockStep ()" << std::endl;
    out << " * The function progresses time till the first end of firing transition" << std::endl;
    out << " * becomes enabled. The time step is returned. In case of deadlock, the" << std::endl;
    out << " * time step is equal to UINT_MAX." << std::endl;
    out << " */" << std::endl;
    out << "inline" << std::endl;
    out << "TTime clockStep()" << std::endl;
    out << "{" << std::endl;
    out << "    TTime step = UINT_MAX;" << std::endl;
    out << "    " << std::endl;
    out << "    // Find maximal time progress" << std::endl;
    out << "    for (uint a = 0; a < NR_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        if (!currentState.actClk[a].empty())" << std::endl;
    out << "        {" << std::endl;
    out << "            TTime actClk = currentState.actClk[a].front().execTime;" << std::endl;
    out << "" << std::endl;
    out << "            if (step > actClk)" << std::endl;
    out << "                step = actClk;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Still actors ready to end their firing?" << std::endl;
    out << "    if (step == 0)" << std::endl;
    out << "        return 0;" << std::endl;
    out << "" << std::endl;
    out << "    // Check for progress (i.e. no deadlock) " << std::endl;
    out << "    if (step == UINT_MAX)" << std::endl;
    out << "        return UINT_MAX;" << std::endl;
    out << "" << std::endl;
    out << "    // Lower remaining execution time actors" << std::endl;
    out << "    for (uint a = 0; a < NR_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        for (std::list<StateActorFiring>::iterator" << std::endl;
    out << "                iter = currentState.actClk[a].begin();" << std::endl;
    out << "                    iter != currentState.actClk[a].end(); iter++)" << std::endl;
    out << "        {" << std::endl;
    out << "            StateActorFiring &actFiring = *iter;" << std::endl;
    out << "            " << std::endl;
    out << "            // Lower remaining execution time of the actor firing" << std::endl;
    out << "            actFiring.execTime -= step;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Advance the global clock  " << std::endl;
    out << "    currentState.glbClk += step;" << std::endl;
    out << "    " << std::endl;
    out << "    return step;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findCausalDependencies ()" << std::endl;
    out << " * The function tracks all causal dependencies in the actor firing of actor a."
        << std::endl;
    out << " * Any causal dependency that is found is added to the abstract dependency"
        << std::endl;
    out << " * graph." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void findCausalDependencies(const uint a, bool **abstractDepGraph)" << std::endl;
    out << "{" << std::endl;
    out << "    // Check all input ports for tokens and output ports for space" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Actor is destination of the channel?" << std::endl;
    out << "        if (channel[c][DST] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Not enough tokens in the previous state?" << std::endl;
    out << "            if (!CH_TOKENS_PREV(c, channelRate[c][DST][ACT_SEQ_POS(a)]))" << std::endl;
    out << "            {" << std::endl;
    out << "                abstractDepGraph[channel[c][DST]][channel[c][SRC]] = true;"
        << std::endl;
    out << "            }    " << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Actor is source of the channel" << std::endl;
    out << "        if (channel[c][SRC] == a)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Not enough space in the previous state?" << std::endl;
    out << "            if (!CH_SPACE_PREV(c, channelRate[c][SRC][ACT_SEQ_POS(a)]))" << std::endl;
    out << "            {" << std::endl;
    out << "                abstractDepGraph[channel[c][SRC]][channel[c][DST]] = true;"
        << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printAnalyzePeriodicPhase(std::ostream &out) {
    out << "/**" << std::endl;
    out << " * analyzePeriodicPhase ()" << std::endl;
    out << " * Analyze the periodic phase of the schedule to find all blocked channels. This"
        << std::endl;
    out << " * is done using the abstract dependency graph." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void analyzePeriodicPhase(const TBufSize *sp, bool *dep)" << std::endl;
    out << "{" << std::endl;
    out << "    bool outputActorFound = false;" << std::endl;
    out << "    bool **abstractDepGraph;" << std::endl;
    out << "    State periodicState;" << std::endl;
    out << "    TTime clkStep;" << std::endl;
    out << "    int repCnt;  " << std::endl;
    out << "" << std::endl;
    out << "    // Current state is a periodic state" << std::endl;
    out << "    copyState(periodicState, currentState);" << std::endl;
    out << "" << std::endl;
    out << "    // Abstract dependency graph" << std::endl;
    out << "    abstractDepGraph = new bool* [NR_ACTORS];" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        abstractDepGraph[i] = new bool [NR_ACTORS];" << std::endl;
    out << "        for (uint j = 0; j < NR_ACTORS; j++)" << std::endl;
    out << "            abstractDepGraph[i][j] = false;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Start new iteration of the periodic phase" << std::endl;
    out << "    currentState.glbClk = 0;" << std::endl;
    out << "    " << std::endl;
    out << "    // Still need to complete the last firing of the output actor" << std::endl;
    out << "    // before period really ends" << std::endl;
    out << "    repCnt = -1;" << std::endl;
    out << "" << std::endl;
    out << "    // Complete the remaining actor firings" << std::endl;
    out << "    for (uint a = outputActor; a < NR_ACTORS; a++)" << std::endl;
    out << "    {" << std::endl;
    out << "        while (actorReadyToEnd(a))  " << std::endl;
    out << "        {" << std::endl;
    out << "            if (a == outputActor)" << std::endl;
    out << "            {" << std::endl;
    out << "                repCnt++;" << std::endl;
    out << "                if (repCnt == outputActorRepCnt)" << std::endl;
    out << "                {" << std::endl;
    out << "                    currentState.glbClk = 0;" << std::endl;
    out << "                    repCnt = 0;" << std::endl;
    out << "                }" << std::endl;
    out << "            }" << std::endl;
    out << "" << std::endl;
    out << "            endActorFiring(a);" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Fire the actors" << std::endl;
    out << "    while (true)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Start actor firings" << std::endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Ready to fire actor a?" << std::endl;
    out << "            while (actorReadyToFire(a))" << std::endl;
    out << "            {" << std::endl;
    out << "                // Track causal dependencies on firing of actor a" << std::endl;
    out << "                findCausalDependencies(a, abstractDepGraph);" << std::endl;
    out << "                " << std::endl;
    out << "                // Fire actor a" << std::endl;
    out << "                startActorFiring(a);" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Performance counter" << std::endl;
    out << "        nrStatesVisited++; " << std::endl;
    out << "" << std::endl;
    out << "        // Clock step" << std::endl;
    out << "        clkStep = clockStep();" << std::endl;
    out << "" << std::endl;
    out << "        // Store partial state to check for progress" << std::endl;
    out << "        for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "        {" << std::endl;
    out << "            previousState.ch[i] = currentState.ch[i];" << std::endl;
    out << "            previousState.sp[i] = currentState.sp[i];" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Finish actor firings  " << std::endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            while (actorReadyToEnd(a))  " << std::endl;
    out << "            {" << std::endl;
    out << "                if (outputActor == a)" << std::endl;
    out << "                {" << std::endl;
    out << "                    repCnt++;" << std::endl;
    out << "                    if (repCnt == outputActorRepCnt)  " << std::endl;
    out << "                    { " << std::endl;
    out << "                        // Found periodic state" << std::endl;
    out << "                        if (equalStates(currentState, periodicState))" << std::endl;
    out << "                        {" << std::endl;
    out << "                            // Cycles in the dependency graph indicate storage"
        << std::endl;
    out << "                            // dependencies" << std::endl;
    out << "                            findStorageDependencies(abstractDepGraph, dep);"
        << std::endl;
    out << "" << std::endl;
    out << "                            // Cleanup" << std::endl;
    out << "                            for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "                                delete [] abstractDepGraph[i];" << std::endl;
    out << "                            delete [] abstractDepGraph;" << std::endl;
    out << "" << std::endl;
    out << "                            // Done" << std::endl;
    out << "                            return;" << std::endl;
    out << "                        }" << std::endl;
    out << "                        currentState.glbClk = 0;" << std::endl;
    out << "                        repCnt = 0;  " << std::endl;
    out << "                    }  " << std::endl;
    out << "                }" << std::endl;
    out << "" << std::endl;
    out << "                // End the actor firing" << std::endl;
    out << "                endActorFiring(a);" << std::endl;
    out << "            }  " << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printAnalyzeDeadlock(std::ostream &out) {
    out << "/**" << std::endl;
    out << " * analyzeDeadlock ()" << std::endl;
    out << " * Analyze the deadlock in the schedule." << std::endl;
    out << " */" << std::endl;
    out << "static " << std::endl;
    out << "void analyzeDeadlock(const TBufSize *sp, bool *dep)" << std::endl;
    out << "{" << std::endl;
    out << "    bool **abstractDepGraph;" << std::endl;
    out << "    " << std::endl;
    out << "    // Abstract dependency graph" << std::endl;
    out << "    abstractDepGraph = new bool* [NR_ACTORS];" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "    {" << std::endl;
    out << "        abstractDepGraph[i] = new bool [NR_ACTORS];" << std::endl;
    out << "        for (uint j = 0; j < NR_ACTORS; j++)" << std::endl;
    out << "            abstractDepGraph[i][j] = false;" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Check number of tokens on every channel in the graph" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        CId srcActor = channel[c][SRC];" << std::endl;
    out << "        CId dstActor = channel[c][DST];" << std::endl;
    out << "        " << std::endl;
    out << "        // Insufficient tokens to fire destination actor" << std::endl;
    out << "        if (!CH_TOKENS(c, channelRate[c][DST][ACT_SEQ_POS(dstActor)]))" << std::endl;
    out << "        {" << std::endl;
    out << "            abstractDepGraph[dstActor][srcActor] = true;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Insufficient space to fire source actor" << std::endl;
    out << "        if (!CH_SPACE(c, channelRate[c][SRC][ACT_SEQ_POS(srcActor)]))" << std::endl;
    out << "        {" << std::endl;
    out << "            abstractDepGraph[srcActor][dstActor] = true;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Cycles in the dependency graph indicate storage dependencies" << std::endl;
    out << "    findStorageDependencies(abstractDepGraph, dep);" << std::endl;
    out << "    " << std::endl;
    out << "    // Cleanup" << std::endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << std::endl;
    out << "        delete [] abstractDepGraph[i];" << std::endl;
    out << "    delete [] abstractDepGraph;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printExecCSDFG(std::ostream &out) {
    out << "/**  " << std::endl;
    out << " * execCSDFgraph()  " << std::endl;
    out << " * Execute the CSDF graph till a deadlock is found or a recurrent state.  "
        << std::endl;
    out << " * The throughput is returned.  " << std::endl;
    out << " */  " << std::endl;
    out << "static" << std::endl;
    out << "TDtime execCSDFgraph(const TBufSize *sp, bool *dep)  " << std::endl;
    out << "{" << std::endl;
    out << "    StatesIter recurrentState;" << std::endl;
    out << "    TTime clkStep;" << std::endl;
    out << "    int repCnt = 0;  " << std::endl;
    out << "" << std::endl;
    out << "    // Create initial state" << std::endl;
    out << "    clearState(currentState);  " << std::endl;
    out << "    clearState(previousState);  " << std::endl;
    out << "" << std::endl;
    out << "    // Initial tokens and space" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Not enough space for initial tokens?" << std::endl;
    out << "        if (sp[c] < channelInitialTokens[c])" << std::endl;
    out << "        {" << std::endl;
    out << "            dep[c] = true;" << std::endl;
    out << "            return 0;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        CH(c) = channelInitialTokens[c];" << std::endl;
    out << "        SP(c) = sp[c] - channelInitialTokens[c];" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    // Fire the actors  " << std::endl;
    out << "    while (true)  " << std::endl;
    out << "    {" << std::endl;
    out << "        // Performance counter" << std::endl;
    out << "        nrStatesVisited++; " << std::endl;
    out << "" << std::endl;
    out << "        // Store partial state to check for progress" << std::endl;
    out << "        for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "        {" << std::endl;
    out << "            previousState.ch[i] = currentState.ch[i];" << std::endl;
    out << "            previousState.sp[i] = currentState.sp[i];" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Finish actor firings  " << std::endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            while (actorReadyToEnd(a))  " << std::endl;
    out << "            {" << std::endl;
    out << "                if (outputActor == a)" << std::endl;
    out << "                {" << std::endl;
    out << "                    repCnt++;" << std::endl;
    out << "                    if (repCnt == outputActorRepCnt)  " << std::endl;
    out << "                    { " << std::endl;
    out << "                        // Add state to hash of visited states  " << std::endl;
    out << "                        if (!storeState(currentState, recurrentState))" << std::endl;
    out << "                        {" << std::endl;
    out << "                            // Find storage dependencies in periodic phase"
        << std::endl;
    out << "                            analyzePeriodicPhase(sp, dep);" << std::endl;
    out << "" << std::endl;
    out << "                            return computeThroughput(recurrentState);" << std::endl;
    out << "                        }" << std::endl;
    out << "                        currentState.glbClk = 0;" << std::endl;
    out << "                        repCnt = 0;" << std::endl;
    out << "                        " << std::endl;
    out << "                        // Performance counter" << std::endl;
    out << "                        nrStatesStored++;" << std::endl;
    out << "                    }  " << std::endl;
    out << "                }" << std::endl;
    out << "" << std::endl;
    out << "                // End the actor firing" << std::endl;
    out << "                endActorFiring(a);" << std::endl;
    out << "            }  " << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Start actor firings  " << std::endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Ready to fire actor a?" << std::endl;
    out << "            while (actorReadyToFire(a))" << std::endl;
    out << "            {" << std::endl;
    out << "                // Fire actor a" << std::endl;
    out << "                startActorFiring(a);" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Clock step" << std::endl;
    out << "        clkStep = clockStep();" << std::endl;
    out << "" << std::endl;
    out << "        // Deadlocked?" << std::endl;
    out << "        if (clkStep == UINT_MAX)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Find cause of deadlock" << std::endl;
    out << "            analyzeDeadlock(sp, dep);" << std::endl;
    out << "            return 0;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    return 0;" << std::endl;
    out << "}  " << std::endl;
    out << "" << std::endl;
}

void printCSDFGexecution(std::ostream &out) {
    printCSDFGheader(out);
    printAnalyzePeriodicPhase(out);
    printAnalyzeDeadlock(out);
    printExecCSDFG(out);
}

void printDistribution(std::ostream &out) {
    out << "/*******************************************************************************"
        << std::endl;
    out << " * Distributions" << std::endl;
    out << " ******************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * Distribution" << std::endl;
    out << " * A storage distribution." << std::endl;
    out << " */" << std::endl;
    out << "typedef struct _Distribution" << std::endl;
    out << "{" << std::endl;
    out << "    TBufSize sz;" << std::endl;
    out << "    TBufSize sp[NR_CHANNELS];" << std::endl;
    out << "    TDtime thr;" << std::endl;
    out << "    bool dep[NR_CHANNELS];" << std::endl;
    out << "    struct _Distribution *prev;" << std::endl;
    out << "    struct _Distribution *next;" << std::endl;
    out << "} Distribution;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * DistributionSet" << std::endl;
    out << " * A container for a linked-list of storage distributions with the same" << std::endl;
    out << " * size. The container can also be used to build a linked-list of" << std::endl;
    out << " * distributions of different size." << std::endl;
    out << " */" << std::endl;
    out << "typedef struct _DistributionSet" << std::endl;
    out << "{" << std::endl;
    out << "    TDtime thr;" << std::endl;
    out << "    TBufSize sz;" << std::endl;
    out << "    Distribution *distributions;" << std::endl;
    out << "    struct _DistributionSet *prev;" << std::endl;
    out << "    struct _DistributionSet *next;" << std::endl;
    out << "} DistributionSet;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * minStorageDistributions" << std::endl;
    out << " * List of all minimal storage distributions." << std::endl;
    out << " */" << std::endl;
    out << "static DistributionSet *minStorageDistributions = NULL;" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * newDistribution ()" << std::endl;
    out << " * Allocate memory for a new storage distribution." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "Distribution *newDistribution()" << std::endl;
    out << "{" << std::endl;
    out << "    return new Distribution;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * deleteDistribution ()" << std::endl;
    out << " * Deallocate memory for a storage distribution." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void deleteDistribution(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    delete d;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * execDistribution ()" << std::endl;
    out << " * Compute throughput and storage dependencies of the given storage" << std::endl;
    out << " * distribution." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void execDistribution(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    // Clear list of stored states" << std::endl;
    out << "    clearStoredStates();" << std::endl;
    out << "" << std::endl;
    out << "    // Initialize performance counters" << std::endl;
    out << "    nrStatesVisited = 0;" << std::endl;
    out << "    nrStatesStored = 0;" << std::endl;
    out << "    nrDistributionsChecked++;" << std::endl;
    out << "    " << std::endl;
    out << "    // Initialize blocking channels" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "        d->dep[c] = false;" << std::endl;
    out << "" << std::endl;
    out << "    // Execute the CSDF graph to find its output interval" << std::endl;
    out << "    d->thr = execCSDFgraph(d->sp, d->dep);" << std::endl;
    out << "" << std::endl;
    out << "    // Update performance counters" << std::endl;
    out << "    if (maxNrStatesVisited < nrStatesVisited)" << std::endl;
    out << "        maxNrStatesVisited = nrStatesVisited;" << std::endl;
    out << "    if (maxNrStatesStored < nrStatesStored)" << std::endl;
    out << "        maxNrStatesStored = nrStatesStored;" << std::endl;
    out << "" << std::endl;
    out << "    //cerr << d->sz << \" \" << d->thr << std::endl;" << std::endl;
    out << "    //for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    //    cerr << d->sp[c] << \" \" << d->dep[c] << std::endl;" << std::endl;
    out << "    //cerr << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * minimizeMinStorageDistributions ()" << std::endl;
    out << " * The function removes all storage distributions within the supplied" << std::endl;
    out << " * set of storage distributions which are non-minimal. All storage distributions"
        << std::endl;
    out << " * within the set should have the same size." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void minimizeStorageDistributionsSet(DistributionSet *ds)" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *d, *t;" << std::endl;
    out << "    " << std::endl;
    out << "    // Maximal throughput with current distribution size equal to previous"
        << std::endl;
    out << "    // throughput with previous (smaller) distribution size" << std::endl;
    out << "    if (ds->prev != NULL && ds->prev->thr == ds->thr)" << std::endl;
    out << "    {" << std::endl;
    out << "        // No minimal storage distributions exist in this list" << std::endl;
    out << "        // Iterate over the list of storage distributions    " << std::endl;
    out << "        d = ds->distributions;" << std::endl;
    out << "        while (d != NULL)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Temporary reference to next element in list" << std::endl;
    out << "            t = d->next;" << std::endl;
    out << "" << std::endl;
    out << "            // Cleanup d" << std::endl;
    out << "            deleteDistribution(d);" << std::endl;
    out << "" << std::endl;
    out << "            // Next" << std::endl;
    out << "            d = t;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        // No distributions left" << std::endl;
    out << "        ds->distributions = NULL;" << std::endl;
    out << "    }" << std::endl;
    out << "    else" << std::endl;
    out << "    {" << std::endl;
    out << "        // Iterate over the list of storage distributions    " << std::endl;
    out << "        d = ds->distributions;" << std::endl;
    out << "        while (d != NULL)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Throughput of distribution smaller than maximum throughput with"
        << std::endl;
    out << "            // same distribution size?" << std::endl;
    out << "            if (d->thr < ds->thr)" << std::endl;
    out << "            {" << std::endl;
    out << "                // Remove d from linked list" << std::endl;
    out << "                if (d->prev != NULL)" << std::endl;
    out << "                    d->prev->next = d->next;" << std::endl;
    out << "                else" << std::endl;
    out << "                    ds->distributions = d->next;" << std::endl;
    out << "                if (d->next != NULL)" << std::endl;
    out << "                    d->next->prev = d->prev;" << std::endl;
    out << "" << std::endl;
    out << "                // Temporary reference to next element in list" << std::endl;
    out << "                t = d->next;" << std::endl;
    out << "" << std::endl;
    out << "                // Cleanup d" << std::endl;
    out << "                deleteDistribution(d);" << std::endl;
    out << "" << std::endl;
    out << "                // Next" << std::endl;
    out << "                d = t;" << std::endl;
    out << "            }" << std::endl;
    out << "            else" << std::endl;
    out << "            {" << std::endl;
    out << "                // Next" << std::endl;
    out << "                d = d->next;" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * addDistributionToChecklist ()" << std::endl;
    out << " * The function add the storage distribution 'd' to the list of" << std::endl;
    out << " * storage distributions which must still be checked. This list is" << std::endl;
    out << " * ordered wrt to the size of the storage distribution. A storage" << std::endl;
    out << " * distribution is only added if it is not already contained in the" << std::endl;
    out << " * list. When the distribution is added to the list, the function returns" << std::endl;
    out << " * 'true', else the function returns 'false'." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "bool addDistributionToChecklist(Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    DistributionSet *ds, *dsNew;" << std::endl;
    out << "    Distribution *di;" << std::endl;
    out << "    bool equalDistr;" << std::endl;
    out << "    " << std::endl;
    out << "    // First distribution ever added?" << std::endl;
    out << "    if (minStorageDistributions == NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Create new set of storage distributions" << std::endl;
    out << "        dsNew = new DistributionSet;" << std::endl;
    out << "        dsNew->sz = d->sz;" << std::endl;
    out << "        dsNew->thr = 0;" << std::endl;
    out << "        dsNew->distributions = d;" << std::endl;
    out << "        dsNew->next = NULL;" << std::endl;
    out << "        dsNew->prev = NULL;" << std::endl;
    out << "        " << std::endl;
    out << "        // Set is first in list to be checked" << std::endl;
    out << "        minStorageDistributions = dsNew;" << std::endl;
    out << "        " << std::endl;
    out << "        // Done" << std::endl;
    out << "        return true;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // Find set of distributions with the same size" << std::endl;
    out << "    ds = minStorageDistributions;" << std::endl;
    out << "    while (ds->next != NULL && ds->next->sz <= d->sz)" << std::endl;
    out << "    {" << std::endl;
    out << "        ds = ds->next;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // Set of storage distribution with same size as d exists?" << std::endl;
    out << "    if (ds->sz == d->sz)" << std::endl;
    out << "    {" << std::endl;
    out << "" << std::endl;
    out << "        // Check that distribution d does not exist in the set" << std::endl;
    out << "        di = ds->distributions;" << std::endl;
    out << "        while (di != NULL)" << std::endl;
    out << "        {" << std::endl;
    out << "            equalDistr = true;" << std::endl;
    out << "" << std::endl;
    out << "            // All channels have same storage space allocation?" << std::endl;
    out << "            for (uint c = 0; equalDistr && c < NR_CHANNELS; c++)" << std::endl;
    out << "            {" << std::endl;
    out << "                if (di->sp[c] != d->sp[c])" << std::endl;
    out << "                    equalDistr = false;" << std::endl;
    out << "            }" << std::endl;
    out << "" << std::endl;
    out << "            // Found equal distribution" << std::endl;
    out << "            if (equalDistr)" << std::endl;
    out << "                return false;" << std::endl;
    out << "" << std::endl;
    out << "            // Next" << std::endl;
    out << "            di = di->next;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Distribution 'd' not yet in the set, so let's add it" << std::endl;
    out << "        ds->distributions->prev = d;" << std::endl;
    out << "        d->next = ds->distributions;" << std::endl;
    out << "        ds->distributions = d;" << std::endl;
    out << "    }" << std::endl;
    out << "    else if (ds->next == NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        // No set of distribution in the list with same or larger size? " << std::endl;
    out << "        " << std::endl;
    out << "        // Create new set of storage distributions" << std::endl;
    out << "        dsNew = new DistributionSet;" << std::endl;
    out << "        dsNew->sz = d->sz;" << std::endl;
    out << "        dsNew->thr = 0;" << std::endl;
    out << "        dsNew->distributions = d;" << std::endl;
    out << "        dsNew->next = NULL;" << std::endl;
    out << "        dsNew->prev = ds;" << std::endl;
    out << "        " << std::endl;
    out << "        // Add set to the list of sets" << std::endl;
    out << "        ds->next = dsNew;" << std::endl;
    out << "    }    " << std::endl;
    out << "    else" << std::endl;
    out << "    {" << std::endl;
    out << "        // In this case 'ds->next' has a size larger than 'd' and 'ds' has"
        << std::endl;
    out << "        // a size smaller than 'd'. Insert a new set of storage" << std::endl;
    out << "        // distributions between 'ds' and 'ds->next' containing storage" << std::endl;
    out << "        // distribution 'd'." << std::endl;
    out << "" << std::endl;
    out << "        // Create new set of storage distributions" << std::endl;
    out << "        dsNew = new DistributionSet;" << std::endl;
    out << "        dsNew->sz = d->sz;" << std::endl;
    out << "        dsNew->thr = 0;" << std::endl;
    out << "        dsNew->distributions = d;" << std::endl;
    out << "        dsNew->next = ds->next;" << std::endl;
    out << "        dsNew->prev = ds;" << std::endl;
    out << "" << std::endl;
    out << "        // Add set to the list of sets" << std::endl;
    out << "        ds->next->prev = dsNew;" << std::endl;
    out << "        ds->next = dsNew;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // Done" << std::endl;
    out << "    return true;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * exploreDistribution ()" << std::endl;
    out << " * The function computes the throughput of the storage distribution" << std::endl;
    out << " * 'd' and adds new storage distributions to the list of distributions" << std::endl;
    out << " * which must be checked based on the storage dependencies found in d." << std::endl;
    out << " * The function also updates the maximal throughput of the set of" << std::endl;
    out << " * storage distributions when needed." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void exploreDistribution(DistributionSet *ds, Distribution *d)" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *dNew;" << std::endl;
    out << "    " << std::endl;
    out << "    // Compute throughput and storage dependencies" << std::endl;
    out << "    execDistribution(d);" << std::endl;
    out << "    " << std::endl;
    out << "    // Throughput of d larger than current maximum of the set" << std::endl;
    out << "    if (d->thr > ds->thr)" << std::endl;
    out << "        ds->thr = d->thr;" << std::endl;
    out << "    " << std::endl;
    out << "    // Create new storage distributions for every channel which" << std::endl;
    out << "    // has a storage dependency in d" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Channel c has storage dependency?" << std::endl;
    out << "        if (d->dep[c])" << std::endl;
    out << "        {" << std::endl;
    out << "            // Do not enlarge the channel if its a self-edge" << std::endl;
    out << "            if (channel[c][SRC] == channel[c][DST])" << std::endl;
    out << "                continue;" << std::endl;
    out << "                " << std::endl;
    out << "            // Create new storage distribution with channel c enlarged" << std::endl;
    out << "            dNew = newDistribution();" << std::endl;
    out << "            dNew->sz = d->sz + minSzStep[c];" << std::endl;
    out << "            dNew->thr = 0;" << std::endl;
    out << "            for (uint i = 0; i < NR_CHANNELS; i++)" << std::endl;
    out << "            {" << std::endl;
    out << "                dNew->sp[i] = d->sp[i];" << std::endl;
    out << "                if (i == c)" << std::endl;
    out << "                    dNew->sp[i] += minSzStep[c];" << std::endl;
    out << "            }" << std::endl;
    out << "            dNew->next = NULL;" << std::endl;
    out << "            dNew->prev = NULL;" << std::endl;
    out << "            " << std::endl;
    out << "            // Add storage distribution to set of distributions to be checked"
        << std::endl;
    out << "            if (!addDistributionToChecklist(dNew))" << std::endl;
    out << "            {" << std::endl;
    out << "                // Distribution already in check list" << std::endl;
    out << "                deleteDistribution(dNew);" << std::endl;
    out << "            }" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * exploreDistributionSet ()" << std::endl;
    out << " * Explore all distributions within the set and remove all non-minimal" << std::endl;
    out << " * distributions from it." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void exploreDistributionSet(DistributionSet *ds)" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *d;" << std::endl;
    out << "" << std::endl;
    out << "    // Explore all storage distributions contained in the set    " << std::endl;
    out << "    d = ds->distributions;" << std::endl;
    out << "    while (d != NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Explore distribution d" << std::endl;
    out << "        exploreDistribution(ds, d);" << std::endl;
    out << "        " << std::endl;
    out << "        // Next" << std::endl;
    out << "        d = d->next;" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // Remove all non-minimal storage distributions from the set" << std::endl;
    out << "    minimizeStorageDistributionsSet(ds);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * findMinimalStorageDistributions ()" << std::endl;
    out << " * Explore the throughput/storage-size trade-off space till either" << std::endl;
    out << " * all minimal storage distributions are found or the throughput bound is" << std::endl;
    out << " * reached." << std::endl;
    out << " */" << std::endl;
    out << "static" << std::endl;
    out << "void findMinimalStorageDistributions(const double thrBound)" << std::endl;
    out << "{" << std::endl;
    out << "    Distribution *d, *t;" << std::endl;
    out << "    DistributionSet *ds, *dt;" << std::endl;
    out << "    " << std::endl;
    out << "    // Construct storage distribution with lower bound storage space" << std::endl;
    out << "    d = newDistribution();" << std::endl;
    out << "    d->thr = 0;" << std::endl;
    out << "    d->sz = lbDistributionSz;" << std::endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "        d->sp[c] = minSz[c];" << std::endl;
    out << "    d->next = NULL;" << std::endl;
    out << "    d->prev = NULL;" << std::endl;
    out << "    " << std::endl;
    out << "    // Add distribution to set of distributions which must be checked" << std::endl;
    out << "    addDistributionToChecklist(d);" << std::endl;
    out << "    " << std::endl;
    out << "    // Check sets of storage distributions till no distributions left to check,"
        << std::endl;
    out << "    // or throughput bound exceeded, or maximal throughput reached" << std::endl;
    out << "    ds = minStorageDistributions;" << std::endl;
    out << "    while (ds != NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Explore all distributions with size 'ds->sz'" << std::endl;
    out << "        exploreDistributionSet(ds);" << std::endl;
    out << "" << std::endl;
    out << "        // Reached maximum throughput or exceed thrBound" << std::endl;
    out << "        if (ds->thr >= thrBound || ds->thr >= maxThroughput)" << std::endl;
    out << "            break;" << std::endl;
    out << "" << std::endl;
    out << "        // Temporary pointer to set" << std::endl;
    out << "        dt = ds;" << std::endl;
    out << "" << std::endl;
    out << "        // Next" << std::endl;
    out << "        ds = ds->next;" << std::endl;
    out << "" << std::endl;
    out << "        // No distributions left with the given distribution size?" << std::endl;
    out << "        if (dt->distributions == NULL)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Remove distr from linked list" << std::endl;
    out << "            if (dt->prev != NULL)" << std::endl;
    out << "                dt->prev->next = dt->next;" << std::endl;
    out << "            else" << std::endl;
    out << "                minStorageDistributions = dt->next;" << std::endl;
    out << "            if (dt->next != NULL)" << std::endl;
    out << "                dt->next->prev = dt->prev;" << std::endl;
    out << "" << std::endl;
    out << "            // Cleanup dt" << std::endl;
    out << "            delete dt;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "   " << std::endl;
    out << "    // Unexplored distributions left?" << std::endl;
    out << "    if (ds != NULL && ds->next != NULL)" << std::endl;
    out << "    {" << std::endl;
    out << "        // Pointer to first set which must be removed" << std::endl;
    out << "        ds = ds->next;" << std::endl;
    out << "        " << std::endl;
    out << "        // Mark previous set as last set in list of minimal storage distr" << std::endl;
    out << "        ds->prev->next = NULL;" << std::endl;
    out << "        " << std::endl;
    out << "        // Remove all unexplored distributions (and sets)" << std::endl;
    out << "        while (ds != NULL)" << std::endl;
    out << "        {" << std::endl;
    out << "            // Remove all distributions within the set ds" << std::endl;
    out << "            d = ds->distributions;" << std::endl;
    out << "            while (d != NULL)" << std::endl;
    out << "            {" << std::endl;
    out << "                t = d->next;" << std::endl;
    out << "                deleteDistribution(d);" << std::endl;
    out << "                d = t;" << std::endl;
    out << "            }" << std::endl;
    out << "" << std::endl;
    out << "            // Temporary pointer to set" << std::endl;
    out << "            dt = ds;" << std::endl;
    out << "            " << std::endl;
    out << "            // Next" << std::endl;
    out << "            ds = ds->next;" << std::endl;
    out << "" << std::endl;
    out << "            // Cleanup dt" << std::endl;
    out << "            delete dt;" << std::endl;
    out << "        }" << std::endl;
    out << "    }" << std::endl;
    out << "    " << std::endl;
    out << "    // Lower bound on storage space (which is used in the beginning) is not a"
        << std::endl;
    out << "    // minimal storage distribution if it deadlocks. The distribution <0,...,0>"
        << std::endl;
    out << "    // is the actual minimal storage distribution for this throughput" << std::endl;
    out << "    if (minStorageDistributions->thr == 0)" << std::endl;
    out << "    {" << std::endl;
    out << "        minStorageDistributions->sz = 0;" << std::endl;
    out << "        minStorageDistributions->distributions->sz = 0;" << std::endl;
    out << "        for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "            minStorageDistributions->distributions->sp[c] = 0;" << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
    out << "/**" << std::endl;
    out << " * printMinimalDistributions ()" << std::endl;
    out << " * Output all minimal distributions to the supplied stream." << std::endl;
    out << " */" << std::endl;
    out << "void printMinimalDistributions(std::ostream &out)" << std::endl;
    out << "{" << std::endl;
    out << "    out << \"<?xml version='1.0' encoding='UTF-8'?>\" << std::endl;" << std::endl;
    out << "    out << \"<sdf3 type='sdf' version='1.0'\" << std::endl;" << std::endl;
    out << "    out << \"xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance'\" << std::endl;"
        << std::endl;
    out << "    out << "
           "\"xsi:noNamespaceSchemaLocation='https://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd'>\" "
           "<< std::endl;"
        << std::endl;
    out << "    out << \"    <storageThroughputTradeOffs>\" << std::endl;" << std::endl;
    out << "" << std::endl;
    out << "    for (DistributionSet *p = minStorageDistributions; p != NULL;" << std::endl;
    out << "            p = p->next)" << std::endl;
    out << "    {" << std::endl;
    out << "        out << \"        <distributionsSet thr='\" << p->thr;" << std::endl;
    out << R"(        out << "' sz='" << p->sz << "'>" << std::endl;)" << std::endl;
    out << "        for (Distribution *d = p->distributions; d != NULL; d = d->next)" << std::endl;
    out << "        {" << std::endl;
    out << "            out << \"            <distribution>\" << std::endl;" << std::endl;
    out << "            for (uint c = 0; c < NR_CHANNELS; c++)" << std::endl;
    out << "            {" << std::endl;
    out << R"(                out << "            <ch name='" << c << "' sz='" << d->sp[c];)"
        << std::endl;
    out << "                out << \"'/>\" << std::endl;" << std::endl;
    out << "            }" << std::endl;
    out << "            out << \"            </distribution>\" << std::endl;" << std::endl;
    out << "        }" << std::endl;
    out << "        out << \"        </distributionsSet>\" << std::endl;" << std::endl;
    out << "    }" << std::endl;
    out << "    out << \"    </storageThroughputTradeOffs>\" << std::endl;" << std::endl;
    out << "    out << \"</sdf3>\" << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printMain(std::ostream &out) {
    out << "int main(int argc, char **argv)" << std::endl;
    out << "{" << std::endl;
    out << "    CTimer timer;" << std::endl;
    out << "    " << std::endl;
    out << "    // Start timer" << std::endl;
    out << "    startTimer(&timer);" << std::endl;
    out << "    " << std::endl;
    out << "    // Search the space" << std::endl;
    out << "    findMinimalStorageDistributions(TDTIME_MAX);" << std::endl;
    out << "" << std::endl;
    out << "    // Stop timer" << std::endl;
    out << "    stopTimer(&timer);" << std::endl;
    out << "    " << std::endl;
    out << "    // Output the minimal storage distributions" << std::endl;
    out << "    printMinimalDistributions(std::cout);" << std::endl;
    out << " " << std::endl;
    out << "    // Output performance data" << std::endl;
    out << "    cerr << \"Analysis time:      \";" << std::endl;
    out << "    printTimer(cerr, &timer);" << std::endl;
    out << "    cerr << std::endl;" << std::endl;
    out << "    cerr << \"#Distr. checked:    \" << nrDistributionsChecked << std::endl;"
        << std::endl;
    out << "    cerr << \"Max stored states:  \" << maxNrStatesStored << std::endl;" << std::endl;
    out << "    cerr << \"Max visited states: \" << maxNrStatesVisited << std::endl;" << std::endl;
    out << "    cerr << \"#actors:            \" << NR_ACTORS << std::endl;" << std::endl;
    out << "    cerr << \"#channels:          \" << NR_CHANNELS << std::endl;" << std::endl;
    out << "    cerr << \"#actors (HSDFG):    \" << NR_ACTORS_HSDFG << std::endl;" << std::endl;
    out << "    " << std::endl;
    out << "    return 0;" << std::endl;
    out << "}" << std::endl;
}

} // namespace

extern void outputCSDFGasBuffyModel(const TimedCSDFgraph& g, std::ostream &out) {
    printDefinitions(g, out);
    printMiscFunctions(out);
    printState(out);
    printDependencies(out);
    printCSDFGexecution(out);
    printDistribution(out);
    printMain(out);
}
} // namespace CSDF
