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
 *  Function        :   (C)SDFG analysis algorithms
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *
 * $Id: sdf3analysis.cc,v 1.2.2.1 2009-03-26 15:58:13 mgeilen Exp $
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

#include "csdf/base/timed/graph.h"
#include "csdf/csdf.h" // IWYU pragma: keep

#include "sdf/sdf.h" // IWYU pragma: keep
#include <memory>

namespace CSDF {

using namespace SDF;

using CPair = struct CPair {
    CString key;
    CString value;
};

using CPairs = std::list<CPair>;
using CPairsIter = CPairs::iterator;

/**
 * Settings
 * Struct to store program settings.
 */
using Settings = struct Settings {
    // Input file with graph
    CString graphFile;

    // xml doc to hold on to
    std::unique_ptr<CDoc> graphDoc;

    // Output file
    CString outputFile;

    // Switch argument(s) given to analysis algorithm
    CPairs arguments;

    // Application graph
    CNode *xmlAppGraph{};
};

namespace{

/**
 * settings
 * Program settings.
 */
Settings settings; // NOLINT(*global-variables,cert-err58-cpp)

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
    out << "   --graph  <file>     input CSDF graph" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
    out << "   --algo <algorithm>  analyze the graph with requested algorithm:";
    out << std::endl;
    out << "       consistency" << std::endl;
    out << "       repetition_vector" << std::endl;
    out << "       repetition_vector_sum" << std::endl;
    out << "       throughput" << std::endl;
    out << "       buffersize" << std::endl;
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
            while (level != 0 && !arguments.empty()) {
                if (arguments.operator[](static_cast<int>(ePos)) == ')') {
                    level--;
                } else if (arguments.operator[](static_cast<int>(ePos)) == '(') {
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
            settings.graphFile = argv[arg];
        } else if (argv[arg] == CString("--output") && arg + 1 < argc) {
            arg++;
            settings.outputFile = argv[arg];
        } else if (argv[arg] == CString("--algo") && arg + 1 < argc) {
            arg++;
            settings.arguments = parseSwitchArgument(argv[arg]);
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
    CNode *appGraphNode = nullptr;
    CNode *sdf3Node = nullptr;

    // Open file
    appGraphDoc = CParseFile(file);
    if (appGraphDoc == nullptr || appGraphDoc->getXmlDoc() == nullptr) {
        throw CException("Failed loading application from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    sdf3Node = appGraphDoc->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get application graph node
    appGraphNode = CGetChildNode(sdf3Node, "applicationGraph");
    if (appGraphNode == nullptr) {
        throw CException("No application graph in '" + file + "'.");
    }

    return appGraphNode;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void initSettings(int argc, char **argv) {
    // Parse the command line
    parseCommandLine(argc, argv);

    // Check required settings
    if (settings.graphFile.empty() || settings.arguments.empty()) {
        helpMessage(std::cerr);
        throw CException("");
    }

    // Load application graph
    settings.xmlAppGraph = loadApplicationGraphFromFile(settings.graphFile, settings.graphDoc, MODULE);
}

/**
 * analyzeCSDFG ()
 * The function analyzes the CSDF graph.
 */
void analyzeCSDFG(const TimedCSDFgraph& g, CPairs &analyze, std::ostream &out) {
    CTimer timer;

    // Measure execution time
    startTimer(&timer);

    // Run analysis
    if (analyze.front().key == "consistency") {
        if (!g.isConsistent()) {
            out << "Graph is not consistent." << std::endl;
        } else {
            out << "Graph is consistent." << std::endl;
        }
    } else if (analyze.front().key == "repetition_vector") {
        RepetitionVector repetitionVector;
        repetitionVector = g.getRepetitionVector();

        out << "Repetition vector:" << std::endl;
        for (uint i = 0; i < repetitionVector.size(); i++) {
            out << " [" << g.getActor(i)->getName() << "] = ";
            out << repetitionVector[i] << std::endl;
        }
    } else if (analyze.front().key == "repetition_vector_sum") {
        uint sum = 0;

        RepetitionVector repetitionVector;
        repetitionVector = g.getRepetitionVector();

        for (int i : repetitionVector) {
            sum += i;
        }

        out << "Repetition vector sum: " << sum << std::endl;
    } else if (analyze.front().key == "throughput") {
        double thr = 0.0;

        thr = CSDF::CSDFstateSpaceThroughputAnalysis::analyze(g);
        out << "thr(" << g.getName() << ") = " << thr << std::endl;
    } else if (analyze.front().key == "buffersize") {
        std::unique_ptr<StorageDistributionSet> minStorageDistributions = nullptr;
        CSDFstateSpaceBufferAnalysis bufferAlgo;

        // Explore trade-off space
        minStorageDistributions = bufferAlgo.analyze(g);

        out << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
        out << "<sdf3 type='sdf' version='1.0'" << std::endl;
        out << "     xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance'";
        out << std::endl;
        out << "     "
               "xsi:noNamespaceSchemaLocation='https://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd'>"
            << std::endl;
        out << "    <storageThroughputTradeOffs>" << std::endl;

        for (StorageDistributionSet *p = minStorageDistributions.get(); p != nullptr; p = p->next.get()) {
            out << "        <distributionsSet thr='";
            out << p->thr;
            out << "' sz='" << p->sz << "'>" << std::endl;
            for (StorageDistribution *d = p->distributions.get(); d != nullptr; d = d->next.get()) {
                out << "            <distribution>" << std::endl;
                for (uint c = 0; c < g.nrChannels(); c++) {
                    out << "                <ch name='";
                    out << g.getConstChannel(c)->getName();
                    out << "' sz='" << d->sp[c];
                    out << "'/>" << std::endl;
                }
                out << "            </distribution>" << std::endl;
            }
            out << "        </distributionsSet>" << std::endl;
        }
        out << "    </storageThroughputTradeOffs>" << std::endl;
        out << "</sdf3>" << std::endl;
    } else {
        throw CException("Unknown analysis algorithm.");
    }

    // Measure execution time
    stopTimer(&timer);
    out << "analysis time: ";
    printTimer(out, &timer);
    out << std::endl;
}

/**
 * analyzeCSDFG ()
 * The function analyzes the CSDF graph.
 */
void analyzeCSDFG(std::ostream &out) {
    CNode *csdfNode = nullptr;
    CNode *csdfPropertiesNode = nullptr;
    std::unique_ptr<TimedCSDFgraph> csdfGraph = nullptr;

    // Find csdf graph in XML structure
    csdfNode = CGetChildNode(settings.xmlAppGraph, "csdf");
    if (csdfNode == nullptr) {
        throw CException("Invalid xml file - missing 'csdf' node");
    }
    csdfPropertiesNode = CGetChildNode(settings.xmlAppGraph, "csdfProperties");

    // Construction CSDF graph model
    csdfGraph = constructTimedCSDFgraph(csdfNode, csdfPropertiesNode);

    // The actual analysis...
    analyzeCSDFG(*csdfGraph, settings.arguments, out);

}

} // namespace

} // namespace CSDF
/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    int exit_status = 0;
    std::ofstream out;

    try {
        // Initialize the program
        CSDF::initSettings(argc, argv);

        // Set output stream
        if (!CSDF::settings.outputFile.empty()) {
            out.open(CSDF::settings.outputFile.c_str());
        } else {
            (static_cast<std::ostream &>(out)).rdbuf(std::cout.rdbuf());
        }

        // Perform requested actions
        CSDF::analyzeCSDFG(out);
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
