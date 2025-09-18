/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   Generate self-timed scheduling model
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *
 * $Id: schedule.cc,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#include "schedule.h"
namespace SDF {

namespace{


SDFtime getMaximumExecTime(const TimedSDFgraph &g) {
    SDFtime max = 0;

    for (const auto& iter : g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        max = a->getExecutionTime() > max ? a->getExecutionTime() : max;
    }

    return max;
}

void printDefinitions(const TimedSDFgraph &g, std::ostream &out) {
    out << "#define SDF_NUM_ACTORS          " << g.nrActors() << std::endl;
    out << "#define SDF_NUM_CHANNELS        " << g.nrChannels() << std::endl;
    out << "" << std::endl;
    out << "#define ACTOR_MAX_EXEC_TIME     " << getMaximumExecTime(g) << std::endl;
    out << "" << std::endl;
    out << "typedef unsigned int uint;" << std::endl;
    out << "typedef unsigned long long TTime;" << std::endl;
    out << "typedef unsigned long long TSize;" << std::endl;
    out << "" << std::endl;
}

void printMiscFunctions(std::ostream &out) {
    out << "#include <math.h>" << std::endl;
    out << "#include <iostream>" << std::endl;
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
    out << "    std::cerr << msg << std::endl;" << std::endl;
    out << "    exit(errorno);" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printSdfHeader(const TimedSDFgraph& /*g*/, std::ostream &out) {
    out << "/******************************************************************************"
        << std::endl;
    out << " * SDF" << std::endl;
    out << " *****************************************************************************/"
        << std::endl;
    out << "" << std::endl;
    out << "TSize ch[SDF_NUM_CHANNELS];" << std::endl;
    out << "TTime actClk[SDF_NUM_ACTORS][ACTOR_MAX_EXEC_TIME+1];" << std::endl;
    out << "bool actReady[SDF_NUM_ACTORS];" << std::endl;
    out << "" << std::endl;
    out << "#define CH(c)               (ch[c])" << std::endl;
    out << "#define CH_TOKENS(c,n)      (CH(c) >= n)" << std::endl;
    out << "#define CONSUME(c,n)        CH(c) = CH(c) - n;" << std::endl;
    out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << std::endl;
    out << "#define FIRE_ACT(a,t)       actClk[a][t]++; firing = true; actReady[a] = false;"
        << std::endl;
    out << "#define ACT_READY(a)        (concurrent || actReady[a])" << std::endl;
    out << "#define ACT_FIRED(a)        if (actClk[a][0] != 0) actReady[a] = true;" << std::endl;
    out << "" << std::endl;
}

void printSdfGraph(const TimedSDFgraph& g, std::ostream &out) {
    out << "void execSDFgraph(const TTime maxGlbClk, const bool concurrent)" << std::endl;
    out << "{" << std::endl;
    out << "    bool firing;" << std::endl;
    out << "    TTime glbClk = 1;" << std::endl;
    out << "" << std::endl;
    out << "    // No actors firing" << std::endl;
    out << "    for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        for (uint t = 0; t <= ACTOR_MAX_EXEC_TIME; t++)" << std::endl;
    out << "            actClk[a][t] = 0;" << std::endl;
    out << "" << std::endl;
    out << "    // All actors ready to fire" << std::endl;
    out << "    for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        actReady[a] = true;" << std::endl;
    out << "" << std::endl;
    out << "    // Initial tokens" << std::endl;

    for (const auto& ch: g.getChannels()) {
        out << "    ch[" << ch->getId() << "] = " << ch->getInitialTokens() << ";" << std::endl;
    }

    out << "" << std::endl;
    out << "    while (glbClk < maxGlbClk)" << std::endl;
    out << "    {" << std::endl;
    out << "        std::cout << std::endl;" << std::endl;
    out << "        std::cout << glbClk << \": \";" << std::endl;
    out << "" << std::endl;
    out << "        // Fire all actors till no actor fires anymore" << std::endl;
    out << "        do" << std::endl;
    out << "        {" << std::endl;
    out << "            // No actor has fired" << std::endl;
    out << "            firing = false;" << std::endl;
    out << "" << std::endl;
    out << "            // Check each actor once" << std::endl;

    for (const auto& iter: g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        out << "            if (ACT_READY(" << a->getId() << ")";

        for (const auto& p: a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << " && CH_TOKENS(" << ch->getId() << "," << p->getRate();
                out << ")";
            }
        }
        out << ")" << std::endl;
        out << "            {" << std::endl;

        for (const auto& p: a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::In) {
                out << "                CONSUME(" << ch->getId() << "," << p->getRate();
                out << ");" << std::endl;
            }
        }

        out << "                FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << std::endl;
        out << "                std::cout << \"" << a->getName() << "\";" << std::endl;
        out << "            }" << std::endl;
    }

    out << "" << std::endl;
    out << "        }" << std::endl;
    out << "        while (firing == true);  " << std::endl;
    out << "" << std::endl;
    out << "        // Move to next time step" << std::endl;
    out << "        glbClk++;" << std::endl;
    out << "        for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "        {" << std::endl;
    out << "            for (uint t = 0; t < ACTOR_MAX_EXEC_TIME; t++)" << std::endl;
    out << "                actClk[a][t] = actClk[a][t+1];" << std::endl;
    out << "            actClk[a][ACTOR_MAX_EXEC_TIME] = 0;" << std::endl;
    out << "        }" << std::endl;
    out << "" << std::endl;
    out << "        // Produce tokens for all completed actor firings" << std::endl;

    for (const auto& iter: g.getActors()) {
        const auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

        for (const auto& p: a->getPorts()) {
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::SDFportType::Out) {
                out << "        PRODUCE(" << ch->getId() << ", actClk[" << a->getId() << "][0]*"
                    << p->getRate();
                out << ");" << std::endl;
            }
        }
    }

    out << "" << std::endl;
    out << "        // Actors which firing has finished are ready to fire again" << std::endl;
    out << "        for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << std::endl;
    out << "            ACT_FIRED(a);" << std::endl;
    out << "" << std::endl;
    out << "    }" << std::endl;
    out << "" << std::endl;
    out << "    std::cerr << std::endl;" << std::endl;
    out << "}" << std::endl;
    out << "" << std::endl;
}

void printSdf(const TimedSDFgraph& g, std::ostream &out) {
    printSdfHeader(g, out);
    printSdfGraph(g, out);
}

void printMain(std::ostream &out) {
    out << "int main(int argc, char **argv)" << std::endl;
    out << "{" << std::endl;
    out << "    TTime maxGlbClk;" << std::endl;
    out << "    bool conc = false;" << std::endl;
    out << "" << std::endl;
    out << "    // Get maximum global clock" << std::endl;
    out << "    if (argc < 2)" << std::endl;
    out << "        exit(\"No maximum global clock specified.\", 1);" << std::endl;
    out << "" << std::endl;
    out << "    maxGlbClk = strtoull(argv[1], NULL, 10);" << std::endl;
    out << "" << std::endl;
    out << "    // Concurrent execution?" << std::endl;
    out << "    if (argc == 3 && strcmp(\"--conc\", argv[2]) == 0)" << std::endl;
    out << "        conc = true;" << std::endl;
    out << "" << std::endl;
    out << "    // Execute the SDF graph" << std::endl;
    out << "    execSDFgraph(maxGlbClk, conc);" << std::endl;
    out << "" << std::endl;
    out << "    return 0;" << std::endl;
    out << "}" << std::endl;
}

} // namespace

/**
 * outputSDFasSelfTimedScheduleModel ()
 * Output the SDF graph as a self-timed scheduling model.
 */
extern void outputSDFasSelfTimedScheduleModel(const TimedSDFgraph& g, std::ostream &out) {
    printDefinitions(g, out);
    printMiscFunctions(out);
    printSdf(g, out);
    printMain(out);
}

} // namespace SDF
