/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3analyze.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 23, 2007
 *
 *  Function        :   SDFG analysis algorithms
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *
 * $Id: sdf3analysis.cc,v 1.10 2009-12-23 13:32:08 sander Exp $
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

#include "sdf3analyze.h"
#include "maxplus/algebra/mpmatrix.h"
#include "maxplus/algebra/mptype.h"
#include "sdf/analysis/maxplus/mpexplore.h"
#include "sdf/base/timed/graph.h"
#include "sdf/sdf.h" // IWYU pragma: export - indirectly includes
#include <cmath>
#include <memory>

using namespace ::Graphs;

using namespace SDF;

namespace SDF {

using namespace MaxPlus;
using namespace MaxPlusAnalysis;

using CPair = struct CPair {
    CString key;
    CString value;
};

using CPairs = std::list<CPair>;
using CPairsIter = CPairs::iterator;

/**
 * Settings
 * Struct to store program settings->
 */
using Settings = struct Settings {
    // Input file with graph
    CString graphFile;

    // Output file
    CString outputFile;

    // Switch argument(s) given to analysis algorithm
    CPairs arguments;

    // hold the xml doc
    std::unique_ptr<CDoc> appGraphDoc;

    // hold the xml doc
    std::unique_ptr<CDoc> archGraphDoc;

    // hold the xml doc
    std::unique_ptr<CDoc> mappingDoc;


    // Application graph
    CNode *xmlAppGraph{};

    // Architecture graph
    CNode *xmlArchGraph{};

    // Mapping
    CNode *xmlMapping{};
};

/**
 * settings
 * Program settings->
 */
namespace {
std::unique_ptr<Settings> settings; // NOLINT(*non-const-global-variables)

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << std::endl;
    out << "Usage: " << TOOL << " --graph <file> --algo <algorithm>";
    out << " [--output <file>]";
    out << std::endl;
    out << "   --graph  <file>     input SDF graph" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
    out << "   --algo <algorithm>  analyze the graph with requested algorithm:";
    out << std::endl;
    out << "       consistency" << std::endl;
    out << "       deadlock" << std::endl;
    out << "       simple_cycles" << std::endl;
    out << "       acyclic_graph" << std::endl;
    out << "       connected_graph" << std::endl;
    out << "       repetition_vector" << std::endl;
    out << "       repetition_vector_sum" << std::endl;
    out << "       strongly_connected_components" << std::endl;
    out << "       mcm[(cycle,dasdan,karp,howard,yto,yto-mcr)]" << std::endl;
    out << "       mpthroughput" << std::endl;
    out << "       mpdelay" << std::endl;
    out << "       mpeigen" << std::endl;
    out << "       mpconverthsdf" << std::endl;
    out << "       is_hsdf" << std::endl;
    out << "       statistics" << std::endl;
    out << "       throughput" << std::endl;
    out << "       throughput-mpeigenvalue" << std::endl;
    out << "       buffersize" << std::endl;
    out << "       buffersize_ning_gao" << std::endl;
    out << "       buffersize_capacity_constrained" << std::endl;
    out << "       latency(method,srcActor,dstActor)" << std::endl;
    out << "       binding_aware_throughput([NSoC,MPFlow])" << std::endl;
    out << "       static_periodic_schedule" << std::endl;
    out << "       static_periodic_schedule_chao" << std::endl;
    out << "       k_periodic" << std::endl;
}

/**
 * parseSwitchArgument ()
 * The function parses the string 'arguments' into a sequence of (arg, value)
 * pairs. The syntax as as follows:
 *
 * pair := key(value)
 * arg := pair,pair,...
 *
 * Note: value may be a pair itself, but this is not expanded into a set of
 * pairs (i.e. nested pairs are not supported).
 */
CPairs parseSwitchArgument(CString arguments) {
    CPairs pairs;
    CPair p;

    while (!arguments.empty()) {
        char c = 0;
        p.key = "";
        p.value = "";

        // Get key from argument string
        do {
            c = arguments[0];
            arguments = arguments.substr(1);
            if (c == ',' || c == '(') {
                break;
            }
            p.key += c;
        } while (!arguments.empty());

        // Is next part of argument a value?
        if (c == '(') {
            CString::size_type ePos = 0;
            int level = 1;

            // Find the matching closing brace
            while (level != 0 && ePos < arguments.size()) {
                if (arguments[static_cast<int>(ePos)] == ')') {
                    level--;
                } else if (arguments[static_cast<int>(ePos)] == '(') {
                    level++;
                }

                // Next
                ePos++;
            }

            // Closing brace found?
            if (level != 0) {
                throw CException("Missing closing brace in value of argument.");
            }

            // Get value
            p.value = arguments.substr(0, ePos - 1);

            // More arguments left?
            if (arguments.size() > ePos) {
                arguments = arguments.substr(ePos + 1);
            } else {
                arguments = "";
            }
        }

        // Add pair to list of pairs
        pairs.push_back(p);
    }

    return pairs;
}


/**
 * parseCommandLine ()
 * The function parses the command line arguments and add info to the
 * supplied settings structure.
 */
void parseCommandLine(int argc, char **argv) {
    int arg = 1;

    if (argc == 1) {
        helpMessage(std::cerr);
        throw CException("");
    }

    do {
        // Configuration file
        if (argv[arg] == CString("--graph") && arg + 1 < argc) {
            arg++;
            settings->graphFile = argv[arg];
        } else if (argv[arg] == CString("--output") && arg + 1 < argc) {
            arg++;
            settings->outputFile = argv[arg];
        } else if (argv[arg] == CString("--algo") && arg + 1 < argc) {
            arg++;
            settings->arguments = parseSwitchArgument(argv[arg]);
        } else {
            helpMessage(std::cerr);
            throw CException("");
        }

        // Next argument
        arg++;
    } while (arg < argc);
}

/**
 * loadApplicationGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
CNode *loadApplicationGraphFromFile(CString &file, std::unique_ptr<CDoc>& appGraphDoc, const CString &module) {
    // Open file
    appGraphDoc = CParseFile(file);
    if (appGraphDoc == nullptr || appGraphDoc->getXmlDoc() == nullptr) {
        throw CException("Failed loading application from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    CNode *sdf3Node = appGraphDoc->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get application graph node
    CNode *appGraphNode = CGetChildNode(sdf3Node, "applicationGraph");
    if (appGraphNode == nullptr) {
        throw CException("No application graph in '" + file + "'.");
    }

    return appGraphNode;
}

/**
 * loadArchitectureGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the architecture.
 */
CNode *loadArchitectureGraphFromFile(const CString &file, std::unique_ptr<CDoc>& archGraphDoc, const CString &module) {
    // Open file
    archGraphDoc = CParseFile(file);
    if (archGraphDoc == nullptr || archGraphDoc->getXmlDoc()==nullptr) {
        throw CException("Failed loading application from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    CNode *sdf3Node = archGraphDoc->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get architecture graph node
    CNode *archGraphNode = CGetChildNode(sdf3Node, "architectureGraph");

    return archGraphNode;
}

/**
 * loadMappingFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the mapping.
 */
CNode *loadMappingFromFile(const CString &file, std::unique_ptr<CDoc>& mappingDoc, const CString &module) {
    // Open file
    mappingDoc = CParseFile(file);
    if (mappingDoc == nullptr || mappingDoc->getXmlDoc()==nullptr) {
        throw CException("Failed loading application from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    CNode *sdf3Node = mappingDoc->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get mapping node
    CNode *mappingNode = CGetChildNode(sdf3Node, "mapping");

    return mappingNode;
}

/**
 * initSettings ()
 * The function initializes the program settings->
 */
void initSettings(int argc, char **argv) {
    // Parse the command line
    parseCommandLine(argc, argv);

    // Check required settings
    if (settings->graphFile.empty() || settings->arguments.empty()) {
        helpMessage(std::cerr);
        throw CException("");
    }

    // Load application graph
    settings->xmlAppGraph = loadApplicationGraphFromFile(settings->graphFile, settings->appGraphDoc, MODULE);

    // Load architecture graph
    settings->xmlArchGraph = loadArchitectureGraphFromFile(settings->graphFile, settings->archGraphDoc, MODULE);

    // Load mapping
    settings->xmlMapping = loadMappingFromFile(settings->graphFile, settings->mappingDoc, MODULE);
}

/**
 * analyzeSDFG ()
 * The function analyzes the SDF graph.
 */
void analyzeSDFG(const std::unique_ptr<TimedSDFgraph>& g, CPairs &analyze, std::ostream &out) { // NOLINT(*cognitive-complexity)
    CTimer timer;

    if (analyze.front().key == "consistency") {
        if (!isSDFgraphConsistent(*g)) {
            out << "Graph is not consistent." << std::endl;
        } else {
            out << "Graph is consistent." << std::endl;
        }
    } else if (analyze.front().key == "deadlock") {
        SDFstateSpaceDeadlockAnalysis deadlockAlgo;

        if (!SDFstateSpaceDeadlockAnalysis::isDeadlockFree(g.get())) {
            out << "Graph deadlocks." << std::endl;
        } else {
            out << "Graph is deadlock free." << std::endl;
        }
    } else if (analyze.front().key == "repetition_vector") {
        RepetitionVector repetitionVector;
        repetitionVector = computeRepetitionVector(*g);

        out << "Repetition vector:" << std::endl;
        for (uint i = 0; i < repetitionVector.size(); i++) {
            out << " [" << g->getActor(i)->getName() << "] = ";
            out << repetitionVector[i] << std::endl;
        }
    } else if (analyze.front().key == "repetition_vector_sum") {
        uint sum = 0;

        RepetitionVector repetitionVector;
        repetitionVector = computeRepetitionVector(*g);

        for (int i : repetitionVector) {
            sum += i;
        }

        out << "Repetition vector sum: " << sum << std::endl;
    } else if (analyze.front().key == "strongly_connected_components") {
        SDFgraphComponents components;
        components = stronglyConnectedComponents(*g);

        out << "Strongly connected components:" << std::endl;
        for (auto &comp : components) {
            bool first = true;

            out << "Component: ";
            for (const auto& a : comp) {
                if (!first) {
                    out << ", ";
                } else {
                    first = false;
                }

                out << a->getName();
            }
            out << std::endl;
        }
    } else if (analyze.front().key == "simple_cycles") {
        SDFgraphCycles cycles;
        cycles = findSimpleCycles(*g);

        out << "Simple cycles:" << std::endl;
        for (const auto &cycle : cycles) {
            bool first = true;

            out << "Cycle: ";
            for (auto *a : cycle) {
                if (!first) {
                    out << ", ";
                } else {
                    first = false;
                }

                out << a->getName();
            }
            out << std::endl;
        }
    } else if (analyze.front().key == "mcm") {
        CDouble mcm = NAN;

        // Measure execution time
        startTimer(&timer);

        auto hsdfGraph = TimedSDFgraph::dynamicCast(transformSDFtoHSDF(*g));

        if (analyze.front().value.empty() || analyze.front().value == "cycle") {
            mcm = maximumCycleMeanCycles(*hsdfGraph);
        } else if (analyze.front().value == "dasdan") {
            mcm = maximumCycleMeanDasdanGupta(*hsdfGraph);
        } else if (analyze.front().value == "karp") {
            mcm = maximumCycleMeanKarp(*hsdfGraph);
        } else if (analyze.front().value == "howard") {
            mcm = maximumCycleMeanHoward(*hsdfGraph);
        } else if (analyze.front().value == "yto") {
            mcm = maximumCycleYoungTarjanOrlin(*hsdfGraph, true);
        } else if (analyze.front().value == "yto-mcr") {
            mcm = maximumCycleYoungTarjanOrlin(*hsdfGraph, false);
        } else {
            throw CException("Unknown MCM algorithm.");
        }

        // Measure execution time
        stopTimer(&timer);

        out << "MCM(" << g->getName() << ") = " << mcm << std::endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;

    } else if (analyze.front().key == "throughput") {

        // check that it is a discrete-time graph
        if (!g->isDiscreteTimeGraph()) {
            throw CException("Cannot use the throughput algorithm, because the graph has "
                             "non-integer execution times.");
        }

        // Measure execution time
        startTimer(&timer);

        double thr = SDFstateSpaceThroughputAnalysis::analyze(*g);

        // Measure execution time
        stopTimer(&timer);

        out << "thr(" << g->getName() << ") = " << thr << std::endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "k_periodic") {
        SDFkPeriodicAnalysis thrAnalysisAlgo;

        // Measure execution time
        startTimer(&timer);

        double thr = SDFkPeriodicAnalysis::analyze(*g);

        // Measure execution time
        stopTimer(&timer);

        out << "thr(" << g->getName() << ") = " << thr << std::endl;

        out << "analysis time: ";

        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "acyclic_graph") {
        if (isAcyclic(*g)) {
            out << "Graph is acyclic." << std::endl;
        } else {
            out << "Graph is cyclic." << std::endl;
        }
    } else if (analyze.front().key == "connected_graph") {
        if (isConnected(*g)) {
            out << "Graph is a connected graph." << std::endl;
        } else {
            out << "Graph is not a connected graph." << std::endl;
        }
    } else if (analyze.front().key == "is_hsdf") {
        if (isHSDFgraph(*g)) {
            out << "Graph is a HSDF graph." << std::endl;
        } else {
            out << "Graph is not a HSDF graph." << std::endl;
        }
    } else if (analyze.front().key == "statistics") {
        int32_t actCntHsdf = 0;
        int32_t chCntHsdf = 0;

        RepetitionVector repVec = computeRepetitionVector(*g);

        for (uint i = 0; i < g->nrActors(); i++) {
            actCntHsdf += repVec[i];
        }

        for (const auto& a : g->getActors()) {
            for (const auto& p : a->getPorts()) {
                if (p->getType() == SDFport::SDFportType::In) {
                    chCntHsdf += static_cast<int32_t>(p->getRate() * repVec[a->getId()]);
                }
            }
        }

        out << "#actors:          " << g->nrActors() << std::endl;
        out << "#channels:        " << g->nrChannels() << std::endl;
        out << "#actors (HSDF):   " << actCntHsdf << std::endl;
        out << "#channels (HSDF): " << chCntHsdf << std::endl;
    } else if (analyze.front().key == "buffersize") {

        // check that it is a discrete-time graph
        if (!g->isDiscreteTimeGraph()) {
            throw CException("Cannot use the buffersize algorithm, because the graph has "
                             "non-integer execution times.");
        }

        SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;
        // Measure execution time
        // startTimer(&timer);

        StorageDistributionSet* minStorageDistributions = bufferAnalysisAlgo.analyze(*g);

        // out << "analysis time: ";
        // printTimer(out, &timer);
        // out << std::endl;

        out << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
        out << "<sdf3 type='sdf' version='1.0'" << std::endl;
        out << "     xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance'" << std::endl;
        out << "      "
               "xsi:noNamespaceSchemaLocation='https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>"
            << std::endl;
        out << "    <storageThroughputTradeOffs>" << std::endl;

        for (StorageDistributionSet* p = minStorageDistributions; p != nullptr; p = p->next.get()) {
            out << "        <distributionsSet thr='";
            out << p->thr;
            out << "' sz='" << p->sz << "'>" << std::endl;
            for (StorageDistribution* d = p->distributions.get(); d != nullptr; d = d->next.get()) {
                out << "            <distribution>" << std::endl;
                for (uint c = 0; c < g->nrChannels(); c++) {
                    out << "                <ch name='";
                    out << g->getChannel(c)->getName();
                    out << "' sz='" << d->sp[c];
                    out << "'/>" << std::endl;
                }
                out << "            </distribution>" << std::endl;
            }
            out << "        </distributionsSet>" << std::endl;
        }
        out << "    </storageThroughputTradeOffs>" << std::endl;
        out << "</sdf3>" << std::endl;
    } else if (analyze.front().key == "buffersize_ning_gao") {
        SDFstateSpaceBufferAnalysisNingGao bufferAnalysisAlgo;
        std::vector<SDFdiscreteTime> startTime;

        // Create a capacity constrained model
        auto gConstrained = TimedSDFgraph::dynamicCast(createCapacityConstrainedModel(*g));

        // Analyze storage space requirements of the capacity constrained graph
        std::unique_ptr<StorageDistribution> d = bufferAnalysisAlgo.analyze(gConstrained.get(), startTime);

        // Output the storage distribution
        int sz = 0;
        for (uint a = 0; a < g->nrActors(); a++) {
            sz += static_cast<int>(d->sp[a]);
            out << "b(" << g->getActor(a)->getName() << ") = " << d->sp[a] << std::endl;
        }
        for (uint a = 0; a < g->nrActors(); a++) {
            out << "s(" << g->getActor(a)->getName() << ") = " << startTime[a] << std::endl;
        }
        out << "size =   " << sz << std::endl;
        out << "period = " << 1.0 / d->thr << std::endl;

        // Create a static-periodic schedule
        SDFstateSpaceThroughputAnalysisNingGao thrAnalysis;
        TDtime thr = SDFstateSpaceThroughputAnalysisNingGao::analyze(
                *g, d, static_cast<SDFdiscreteTime>(1.0 / d->thr), startTime);
        if (thr != d->thr) {
            throw CException("Static-periodic schedule is invalid");
        }
    } else if (analyze.front().key == "buffersize_ning_gao_hijdra") {
        SDFstateSpaceBufferAnalysisNingGao bufferAnalysisAlgo;
        std::vector<SDFdiscreteTime> startTime;

        // Measure execution time
        startTimer(&timer);

        // Create a capacity constrained model
        std::unique_ptr<TimedSDFgraph> gConstrained = createCapacityConstrainedModel(*g);

        // Analyze storage space requirements of the capacity constrained graph
        std::unique_ptr<StorageDistribution> d = bufferAnalysisAlgo.analyze(gConstrained.get(), startTime);

        // Measure execution time
        stopTimer(&timer);

        // Output the graph in Hijdra format
        out << "actors" << std::endl;
        out << std::endl;
        for (const auto& iter : g->getActors()) {
            auto* a = dynamic_cast<TimedSDFactor*>(iter.get());

            out << "name=\"" << a->getName() << "\" exec=";
            out << a->getExecutionTime() << ";" << std::endl;
        }
        out << std::endl;

        out << "arcs" << std::endl;
        out << std::endl;
        for (const auto& iter : g->getChannels()) {
            auto* c = dynamic_cast<TimedSDFchannel*>(iter.get());

            out << "src=\"" << c->getSrcActor()->getName() << "\" ";
            out << "dst=\"" << c->getDstActor()->getName() << "\" ";
            out << "delay=" << c->getInitialTokens() << " ";
            out << "buffersize=" << d->sp[c->getSrcActor()->getId()];

            // Self-edge?
            if (c->getSrcActor()->getId() == c->getDstActor()->getId()) {
                out << " tokensize=0;" << std::endl;
            } else {
                out << ";" << std::endl;
            }
        }
        out << std::endl;

        out << "constraints" << std::endl;
        out << std::endl;
        out << "mud=" << 1.0 / d->thr << std::endl;
        out << "(*" << std::endl;
        outputSDFasXML(*g, out);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
        out << "*)" << std::endl;
        out << "end" << std::endl;
    } else if (analyze.front().key == "buffersize_capacity_constrained") {
        // Exclude auto-concurrency
        auto gNoConcurrency = TimedSDFgraph::dynamicCast(modelAutoConcurrencyInSDFgraph(*g, 1));

        // Compute maximal throughput and MCM
        CDouble mcm = maximumCycleMeanCycles(*gNoConcurrency);
        // mcm = mcm.lowestTerm();
        // maxThr = 1.0 / mcm.value();
        double maxThr = 1.0 / mcm;

        /// @todo
        /// what is a reasonable precision value?
        /// is lowestTerm necessary after doubleToFraction?
        CFraction mcm_f;
        mcm_f.doubleToFraction(mcm, CFraction::DoubleToFractionPrecision);
        mcm_f = mcm_f.lowestTerm();

        // MCM must be an integer value
        if (mcm_f.denominator() != 1) {
            throw CException("MCM must have an integer value.");
        }

        // Create capacity constrained model
        auto gConstrained = TimedSDFgraph::dynamicCast(modelCapacityConstrainedBuffer(*gNoConcurrency, static_cast<uint>(mcm_f.numerator())));

        // Analyze storage space requirements of the capacity constrained graph
        std::unique_ptr<StorageDistributionSet> minStorageDistributions =
                stateSpaceBufferAnalysisCapacityConstrainedModel(*gConstrained, maxThr);

        // Output all minimal storage distributions
        out << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
        out << "<sdf3 type='sdf' version='1.0'" << std::endl;
        out << "     xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance'";
        out << std::endl;
        out << "     "
               "xsi:noNamespaceSchemaLocation='https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>"
            << std::endl;
        out << "    <storageThroughputTradeOffs>" << std::endl;

        for (StorageDistributionSet* p = minStorageDistributions.get(); p != nullptr; p = p->next.get()) {
            out << "        <distributionsSet thr='";
            out << p->thr;
            out << "' sz='" << p->sz << "'>" << std::endl;
            for (StorageDistribution* d = p->distributions.get(); d != nullptr; d = d->next.get()) {
                out << "            <distribution>" << std::endl;
                for (uint c = 0; c < gConstrained->nrChannels(); c++) {
                    // Channel models storage space?
                    if ((dynamic_cast<TimedSDFchannel *>(gConstrained->getChannel(c)))
                                ->modelsStorageSpace()) {
                        auto *ch = dynamic_cast<TimedSDFchannel *>(gConstrained->getChannel(c));
                        ch = ch->getStorageSpaceChannel();

                        // Channel ch of which storage space is modeled with
                        // channel c appears in the original graph g?
                        if (g->getChannel(ch->getName()) != nullptr) {
                            // Channel is not a self-edge added to remove
                            // auto-concurrency
                            out << "                <ch name='";
                            out << ch->getName();
                            out << "' sz='" << d->sp[c];
                            out << "'/>" << std::endl;
                        }
                    }
                }
                out << "            </distribution>" << std::endl;
            }
            out << "        </distributionsSet>" << std::endl;
        }
        out << "    </storageThroughputTradeOffs>" << std::endl;
        out << "</sdf3>" << std::endl;

    } else if (analyze.front().key == "latency") {
        CStrings options;

        // Extract options supplied with latency switch
        stringtok(options, analyze.front().value, ",");
        if (options.size() != 3) {
            throw CException("Incorrect number of options given.");
        }
        CString method = options.front();
        options.pop_front();
        CString srcActorName = options.front();
        options.pop_front();
        CString dstActorName = options.front();
        options.pop_front();

        // Find pointer to src and dst actor in graph
        SDFactor *srcActor = g->getActor(srcActorName);
        if (srcActor == nullptr) {
            throw CException("No source actor found.");
        }
        SDFactor *dstActor = g->getActor(dstActorName);
        if (dstActor == nullptr) {
            throw CException("No destination actor found.");
        }

        // Measure execution time
        startTimer(&timer);

        // Compute latency
        double latency = NAN;
        double thr = -1.0;
        if (method == "st") {
            SDFstateSpaceSelfTimedLatencyAnalysis::analyze(g.get(), srcActor, dstActor, latency, thr);
        } else if (method == "sp") {
            SDFstateSpaceMinimalLatencyAnalysis stateSpaceMinLatencyAnalysis;

            SDFstateSpaceMinimalLatencyAnalysis::analyzeSingleProc(*g, *srcActor, *dstActor, latency);
        } else if (method == "min") {
            SDFstateSpaceMinimalLatencyAnalysis stateSpaceMinLatencyAnalysis;

            SDFstateSpaceMinimalLatencyAnalysis::analyze(*g, *srcActor, *dstActor, latency, thr);
        } else if (method == "ro") {

            constexpr uint NrAttempts = 100;
            latency = latencyAnalysisForRandomStaticOrderSingleProc(g.get(), srcActor, dstActor, NrAttempts);
        } else if (method == "min_st") {
            SDFstateSpaceSelfTimedMinimalLatencyAnalysis selftimedMinimalLatencyAnalysisAlgo;

            SDFstateSpaceSelfTimedMinimalLatencyAnalysis::analyze(
                    *g, *srcActor, *dstActor, latency, thr);
        } else {
            throw CException("Latency analysis method unknown.");
        }

        // Measure execution time
        stopTimer(&timer);

        out << "latency(" << g->getName() << ") = " << latency << std::endl;

        if (thr != -1) {
            out << "throughput(" << g->getName() << ") = " << thr << std::endl;
        }

        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "binding_aware_throughput") {

        // check that it is a discrete-time graph
        if (!g->isDiscreteTimeGraph()) {
            throw CException(
                    "Cannot use the binding_aware_throughput algorithm, because the graph has "
                    "non-integer execution times.");
        }

        std::vector<double> tileUtilization;

        SDFflowType flowType = SDFflowType::SDFflowTypeNSoC;
        if (analyze.front().value == "NSoC") {
            flowType = SDFflowType::SDFflowTypeNSoC;
        } else if (analyze.front().value == "MPFlow") {
            flowType = SDFflowType::SDFflowTypeMPFlow;
        } else {
            throw CException("Flow type '" + analyze.front().value + "'is not supported.");
        }

        // Create a platform graph
        if (settings->xmlArchGraph == nullptr) {
            throw CException("No architectureGraph given.");
        }
        auto platformGraph = std::unique_ptr<PlatformGraph>(constructPlatformGraph(settings->xmlArchGraph));

        // Set the mapping of the application onto the platform graph
        if (settings->xmlMapping == nullptr) {
            throw CException("No mapping given.");
        }
        setMappingPlatformGraph(platformGraph, g.get(), settings->xmlMapping);

        // Create a binding-aware SDFG
        auto bindingAwareSDFG = std::make_unique<BindingAwareSDFG>(g.get(), platformGraph.get(), flowType);

        // Measure execution time
        startTimer(&timer);

        // Analyze the throughput
        double thr = SDFstateSpaceBindingAwareThroughputAnalysis::analyze(*bindingAwareSDFG,
                                                                          tileUtilization);

        // Measure execution time
        stopTimer(&timer);

        out << "thr(" << g->getName() << ") = " << thr << std::endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "static_periodic_schedule") {
        SDFstateSpaceStaticPeriodicScheduler::schedule(g.get());
    } else if (analyze.front().key == "static_periodic_schedule_chao") {

        auto h = TimedSDFgraph::dynamicCast(transformSDFtoHSDF(*g));
        SDFstateSpaceStaticPeriodicSchedulerChao::schedule(h.get());

    } else if (analyze.front().key == "mpthroughput") {

        auto G = std::make_unique<GraphDecoration::Graph>(*g);
        MaxPlusAnalysis::Exploration mpe;
        mpe.G = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.OG = std::move(G);


        if (!analyze.front().value.empty()) {
            CStrings options;

            // Extract options supplied with latency switch
            stringtok(options, analyze.front().value, "=");
            if (options.empty()) {
                throw CException("Empty set of arguments provided");
            }
            if (options.front() != "maxNumberOfStates") {
                throw CException("Unexpected argument provided: " + options.front());
            }
            options.pop_front();
            mpe.limitTheMaxNumberOfStates();
            uint64_t N = std::stol(options.front());
            mpe.setMaxNumberOfStates(N);

            out << "The maximum number of states will be limited to " << N << "\n";
        }

        // Measure execution time
        startTimer(&timer);

        double thr = mpe.explore();

        // Measure execution time
        stopTimer(&timer);

        if (mpe.maxNumberOfStatesWasReached()) {
            out << "The maximum number of states was reached before finding a recurrent state.\n";
        } else {
            out << "thr(" << g->getName() << ") = " << thr << std::endl;
        }

        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "throughput-mpeigenvalue") {

        auto G = std::make_unique<GraphDecoration::Graph>(*g);
        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);

        // Measure execution time
        startTimer(&timer);

        auto M = std::unique_ptr<Matrix>(mpe.convertToMaxPlusMatrix());

        double thr = 1.0 / M->mp_eigenvalue();

        // Measure execution time
        stopTimer(&timer);

        out << "thr(" << g->getName() << ") = " << thr << std::endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;

    } else if (analyze.front().key == "mpdelay") {

        auto G = std::make_unique<GraphDecoration::Graph>(*g);
        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);

        CNode *sdfPropertiesNode = CGetChildNode(settings->xmlAppGraph, "sdfProperties");
        if (sdfPropertiesNode == nullptr) {
            throw CException("Invalid xml file - missing 'sdfProperties' node");
        }
        CNode *graphPropertiesNode = CGetChildNode(sdfPropertiesNode, "graphProperties");
        if (graphPropertiesNode == nullptr) {
            throw CException("Invalid xml file - missing 'graphProperties' node");
        }

        auto gamma_s_v = std::make_unique<std::vector<MPTime>>();

        CNode *mpschedule_ststNode = CGetChildNode(graphPropertiesNode, "mpschedule_steadystate");
        if (mpschedule_ststNode == nullptr) {
            throw CException("Invalid xml file - missing 'mpschedule_steadystate' node");
        }

        CNode *channelNode = CGetChildNode(mpschedule_ststNode, "channel");
        while (channelNode != nullptr) {
            CNode *tokenNode = CGetChildNode(channelNode, "token");
            while (tokenNode != nullptr) {
                CString v = CGetNodeContent(tokenNode);
                gamma_s_v->emplace_back(static_cast<CDouble>(v));
                tokenNode = CNextNode(tokenNode, "token");
            }
            channelNode = CNextNode(channelNode, "channel");
        }

        CNode *mpdelay_periodNode = CGetChildNode(graphPropertiesNode, "mpperiod");
        if (mpdelay_periodNode == nullptr) {
            throw CException("Invalid xml file - missing 'mpperiod' node");
        }
        CString v = CGetNodeContent(mpdelay_periodNode);
        auto period = MPTime(static_cast<CDouble>(v));

        auto gamma_s = std::make_unique<MaxPlus::Vector>(gamma_s_v.get());

        std::unique_ptr<std::vector<MPTime>> gamma0_v = nullptr;

        CNode *mpschedule_initNode = CGetChildNode(graphPropertiesNode, "mpschedule_initial");
        while (mpschedule_initNode != nullptr) {
            gamma0_v = std::make_unique<std::vector<MPTime>>();

            channelNode = CGetChildNode(mpschedule_initNode, "channel");
            while (channelNode != nullptr) {
                CNode *tokenNode = CGetChildNode(channelNode, "token");
                while (tokenNode != nullptr) {
                    CString v = CGetNodeContent(tokenNode);
                    gamma0_v->emplace_back(static_cast<CDouble>(v));
                    tokenNode = CNextNode(tokenNode, "token");
                }
                channelNode = CNextNode(channelNode, "channel");
            }

            auto gamma0 = std::make_unique<MaxPlus::Vector>(gamma0_v.get());

            // Measure execution time
            startTimer(&timer);
            MPTime d = mpe.exploreDelayForSchedule(gamma0.get(), gamma_s.get(), period);

            // Measure execution time
            stopTimer(&timer);

            out << "delay: " << static_cast<CDouble>(d) << std::endl;

            out << "analysis time: ";
            printTimer(out, &timer);
            out << std::endl;

            mpschedule_initNode = CNextNode(mpschedule_initNode, "mpschedule_initial");
        }
    } else if (analyze.front().key == "mpconverthsdf") {

        auto G = std::make_unique<GraphDecoration::Graph>(*g);
        MaxPlusAnalysis::Exploration mpe;

        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);
        mpe.g = g.get();

        // Measure execution time
        startTimer(&timer);
        std::unique_ptr<TimedSDFgraph> h = mpe.convertToHSDFByMP();

        // Measure execution time
        stopTimer(&timer);
        outputSDFasXML(*h, out);

    } else if (analyze.front().key == "mpeigen") {
        auto G = std::make_unique<GraphDecoration::Graph>(*g);
        MaxPlusAnalysis::Exploration mpe;
        mpe.G = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.OG = std::move(G);

        // Measure execution time
        startTimer(&timer);

        std::unique_ptr<MaxPlus::Vector> eigenvector = nullptr;
        MPTime eigenvalue;

        mpe.exploreEigen(&eigenvector, &eigenvalue);

        // Measure execution time
        stopTimer(&timer);

        out << "eigenvalue: " << MPString(eigenvalue) << std::endl;

        for (unsigned int i = 0; i < eigenvector->getSize(); i++) {
            out << MPString(eigenvector->get(i)) << " ; ";
        }
        out << std::endl;
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;

    } else {
        throw CException("Unknown analysis algorithm.");
    }
}

/**
 * analyzeSDFG ()
 * The function analyzes the SDF graph.
 */
void analyzeSDFG(std::ostream &out) {
    // Find sdf graph in XML structure
    CNode *sdfNode = CGetChildNode(settings->xmlAppGraph, "sdf");
    if (sdfNode == nullptr) {
        throw CException("Invalid xml file - missing 'sdf' node");
    }
    CNode *sdfPropertiesNode = CGetChildNode(settings->xmlAppGraph, "sdfProperties");

    // Construction SDF graph model
    auto sdfGraph = std::make_unique<TimedSDFgraph>();
    sdfGraph->construct(sdfNode, sdfPropertiesNode);

    // The actual analysis...
    analyzeSDFG(sdfGraph, settings->arguments, out);
}

} // namespace

} // namespace SDF
/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    int exit_status = 0;
    std::ofstream out;

    settings = std::make_unique<Settings>();

    try {
        // Initialize the program
        initSettings(argc, argv);

        // Set output stream
        if (!settings->outputFile.empty()) {
            out.open(settings->outputFile.c_str());
        } else {
            (static_cast<std::ostream &>(out)).rdbuf(std::cout.rdbuf());
        }

        // Perform requested actions
        analyzeSDFG(out);
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
