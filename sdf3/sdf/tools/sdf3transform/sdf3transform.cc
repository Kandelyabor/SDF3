/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 23, 2007
 *
 *  Function        :   SDFG transformation functionality
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *      18-02-17    :   Included retiming functionality.
 *
 * $Id: sdf3transform.cc,v 1.2.2.2 2010-04-25 01:21:17 mgeilen Exp $
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

#include "sdf3transform.h"
#include "sdf/sdf.h" // IWYU pragma: keep
#include <memory>

using namespace SDF;
namespace SDF {

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

    // Switch argument(s) given to algorithm
    CPairs arguments;

    // Application graph
    CNode *xmlAppGraph{};
};

namespace{



/**
 * settings
 * Program settings.
 */
Settings settings; // NOLINT(*non-const-global-variables,cert-err58-cpp)

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << std::endl;
    out << "Usage: " << TOOL << " --graph <file> --transform <list>";
    out << " [--output <file>]" << std::endl;
    out << "   --graph  <file>     input SDF graph" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
    out << "   --transform <list>  comma separated list with graph";
    out << " transformations:" << std::endl;
    out << "       to_hsdf" << std::endl;
    out << "       unfold(<count>)" << std::endl;
    out << "       model_buffersize" << std::endl;
    out << "       model_autoconc(<max>)" << std::endl;
    out << "       select_buffersize" << std::endl;
    out << "       move_state" << std::endl;
    out << "       capacity_constrained(<mcm>)" << std::endl;
    out << "       retiming(<dip>) (without dip it will compute optimal retiming)" << std::endl;
    // out << "       to_fsmsadf" << std::endl;
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

    while (arg < argc) {
        // Configuration file
        if (argv[arg] == CString("--graph") && arg + 1 < argc) {
            arg++;
            settings.graphFile = argv[arg];
        } else if (argv[arg] == CString("--output") && arg + 1 < argc) {
            arg++;
            settings.outputFile = argv[arg];
        } else if (argv[arg] == CString("--transform") && arg + 1 < argc) {
            arg++;
            settings.arguments = parseSwitchArgument(argv[arg]);
        } else {
            helpMessage(std::cerr);
            throw CException("");
        }

        // Next argument
        arg++;
    }
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
 * transformGraph ()
 * The function transforms the SDF graph and the resulting (H)SDF graph
 * is outputted in XML format to the out stream.
 */
void transformGraph(TimedSDFgraph *g, CPairs &transforms, std::ostream &out) {
    SDFcomponent component = SDFcomponent(g->getParent(), g->getId());
    std::unique_ptr<TimedSDFgraph> h_in = TimedSDFgraph::dynamicCast(g->clone(component));
    std::unique_ptr<TimedSDFgraph> h_out = nullptr;

    for (const auto &[tKey, tValue] : transforms) {

        // Perform transformation
        if (tKey == "to_hsdf") {
            h_out = TimedSDFgraph::dynamicCast(transformSDFtoHSDF(*h_in));
        } else if (tKey == "unfold") {
            if (tValue.empty()) {
                throw CException("Missing unfolding factor.");
            }

            h_out = TimedSDFgraph::dynamicCast(unfoldHSDF(*h_in, static_cast<uint>(tValue)));
        } else if (tKey == "model_buffersize") {
            h_out = modelBufferSizeInSDFgraph(*h_in);
        } else if (tKey == "model_autoconc") {
            if (tValue.empty()) {
                throw CException("Missing maximum degree of auto-concurrency.");
            }

            h_out = TimedSDFgraph::dynamicCast(
                    modelAutoConcurrencyInSDFgraph(*h_in, static_cast<uint>(tValue)));
        } else if (tKey == "select_buffersize") {
            StorageDistributionSet* storageDistributions = nullptr;
            StorageDistributionSet* thrPoint = nullptr;
            SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;
            uint nrThrPoints = 0;
            uint nrDistPoints = 0;
            uint select = 0;
            StorageDistribution* dist = nullptr;

            // Load storage distributions from XML file
            storageDistributions = bufferAnalysisAlgo.analyze(*h_in);

            // Print list of all throughput points
            for (StorageDistributionSet* d = storageDistributions; d != nullptr; d = d->next.get()) {
                std::cout << "Throughput " << nrThrPoints << ": " << d->thr << std::endl;
                nrThrPoints++;
            }

            // Select throughput point
            if (nrThrPoints == 1) {
                select = 0;
            } else {
                select = nrThrPoints;
                while (select >= nrThrPoints) {
                    std::cout << "Select storage distribution (throughput): ";
                    std::cin >> select;
                }
                std::cout << std::endl;
            }

            // Find throughput point
            nrThrPoints = 0;
            for (thrPoint = storageDistributions; thrPoint != nullptr; thrPoint = thrPoint->next.get()) {
                if (nrThrPoints == select) {
                    break;
                }
                nrThrPoints++;
            }

            // Print list of all storage distributions with selected throughput
            for (StorageDistribution* d = thrPoint->distributions.get(); d != nullptr; d = d->next.get()) {
                std::cout << "Distribution " << nrDistPoints << ": " << std::endl;
                for (uint c = 0; c < h_in->nrChannels(); c++) {
                    std::cout << "    " << d->sp[c] << std::endl;
                }
                nrDistPoints++;
            }

            // Select distribution point
            if (nrDistPoints == 1) {
                select = 0;
            } else {
                select = nrDistPoints;
                while (select >= nrDistPoints) {
                    std::cout << "Select storage distribution: ";
                    std::cin >> select;
                }
                std::cout << std::endl;
            }

            // Find distribution point
            nrDistPoints = 0;
            for (dist = thrPoint->distributions.get(); dist != nullptr; dist = dist->next.get()) {
                if (select == nrDistPoints) {
                    break;
                }
                nrDistPoints++;
            }

            // Create a copy of the graph and set buffer sizes of the channels
            SDFcomponent component(h_in->getParent(), h_in->getId(), h_in->getName());
            h_out = TimedSDFgraph::dynamicCast(h_in->clone(component));

            for (const auto& iterCh : h_out->getChannels()) {
                auto* ch = dynamic_cast<TimedSDFchannel*>(iterCh.get());
                TimedSDFchannel::BufferSize buffersize;

                buffersize.sz = static_cast<int>(dist->sp[ch->getId()]);
                buffersize.mem = 0;
                buffersize.src = 0;
                buffersize.dst = 0;
                ch->setBufferSize(buffersize);
            }
        } else if (tKey == "move_state") {
            // Create a copy of the graph and set buffer sizes of the channels
            SDFcomponent component(h_in->getParent(), h_in->getId(), h_in->getName());
            h_out = TimedSDFgraph::dynamicCast(h_in->clone(component));

            for (const auto& iter : h_out->getActors()) {
                auto* a = dynamic_cast<TimedSDFactor*>(iter.get());
                TimedSDFchannel::BufferSize buffersize;
                uint state = UINT_MAX;

                buffersize.sz = 1;
                buffersize.mem = 1;
                buffersize.src = 1;
                buffersize.dst = 1;

                // Find the minimum state size of all processors
                for (const auto& p : a->getProcessors()) {
                    if (p->stateSize < state) {
                        state = static_cast<uint>(p->stateSize);
                    }
                }

                // Create self-edge channel with state size token
                TimedSDFchannel* c = h_out->createChannel(a, 1, a, 1, 1);
                c->setTokenSize(static_cast<int>(state));
                c->setInitialTokens(1);
                c->setBufferSize(buffersize);
                c->setMinBandwidth(0);
                c->setMinLatency(0);

                // reduce the state size of all processors
                for (const auto& p : a->getProcessors()) {
                    p->stateSize = p->stateSize - state;
                }
            }
        } else if (tKey == "capacity_constrained") {
            if (tValue.empty()) {
                throw CException("Missing MCM.");
            }

            h_out = modelCapacityConstrainedBuffer(*h_in, static_cast<uint>(tValue));
        } else if (tKey == "retiming") {
            if (tValue.empty()) {
                h_out = retime_SAnew(*h_in);
            } else {
                h_out = retime_SAnew(*h_in, static_cast<uint>(tValue));
            }
        }
        else {
            throw CException("Unknown transformation.");
        }

        // Output graph becomes next input
        h_in = std::move(h_out);
    }

    // Output resulting SDF graph
    if (h_in != nullptr) {
        outputSDFasXML(*h_in, out);
    }
}

/**
 * transformGraph ()
 * The function transforms the SDF graph.
 */
void transformGraph(std::ostream &out) {

    // Find sdf graph in XML structure
    CNode *sdfNode = CGetChildNode(settings.xmlAppGraph, "sdf");
    if (sdfNode == nullptr) {
        throw CException("Invalid xml file - missing 'sdf' node");
    }
    CNode *sdfPropertiesNode = CGetChildNode(settings.xmlAppGraph, "sdfProperties");

    // Construction SDF graph model
    auto sdfGraph = std::make_unique<TimedSDFgraph>();
    sdfGraph->construct(sdfNode, sdfPropertiesNode);

    // The actual transformation...
    transformGraph(sdfGraph.get(), settings.arguments, out);
}

} // anonymous namespace


} // namespace SDF
/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    int exit_status = 0;
    std::ofstream out;

    try {
        // Initialize the program
        initSettings(argc, argv);

        // Set output stream
        if (!settings.outputFile.empty()) {
            out.open(settings.outputFile.c_str());
        } else {
            (static_cast<std::ostream &>(out)).rdbuf(std::cout.rdbuf());
}

        // Perform requested actions
        transformGraph(out);
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
