/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 9, 2012
 *
 *  Function        :   CSDFG transformation functionality
 *
 *  History         :
 *      09-08-12    :   Initial version.
 *
 * $Id:  $
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
#include "csdf/base/timed/graph.h"
#include "csdf/csdf.h" // IWYU pragma: keep
#include "csdf/transform/to_wc_fsm_sadf/csdftowcfsmsadf.h"
#include <memory>


using namespace CSDF;
namespace CSDF {

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

namespace {

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
    out << "Usage: " << TOOL << " --graph <file> --transform <list>";
    out << " [--output <file>]" << std::endl;
    out << "   --graph  <file>     input CSDF graph" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
    out << "   --transform <list>  comma separated list with graph";
    out << " transformations:" << std::endl;
    out << "       to_wc-fsmsadf" << std::endl;
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
 * transformGraph ()
 * The function transforms the SDF graph and the resulting (H)SDF graph
 * is outputted in XML format to the out stream.
 * TODO(MG) tool is still based on a sequence of transformations, but this is not compatible with
 * transformations to different graph types.
 */
void transformGraph(const TimedCSDFgraph &g, CPairs &transforms, std::ostream &out) {
    CSDFcomponent component = CSDFcomponent(g.getParent(), g.getId());
    std::unique_ptr<CSDFgraph> h_in = g.clone(component);
    auto &th_in = dynamic_cast<TimedCSDFgraph &>(*h_in);
    std::unique_ptr<FSMSADF::Graph> h_out = nullptr;

    for (auto &transform : transforms) {
        // Perform transformation
        if (transform.key == "to_wc-fsmsadf") {
            h_out = convertCSDFGtoWCFSMSADFG(th_in);
        } else {
            throw CException("Unknown transformation.");
        }
    }

    // Output resulting SDF graph
    if (h_out != nullptr) {
        // Create XML
        CNode *sdf3Node = CNewNode("sdf3");
        CAddAttribute(sdf3Node, "version", "1.0");
        CAddAttribute(sdf3Node, "type", "fsmsadf");
        CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
        CAddAttribute(sdf3Node,
                      "xsi:noNamespaceSchemaLocation",
                      "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

        // Application graph node
        CNode *appNode = CAddNode(sdf3Node, "applicationGraph");

        // SDF node
        h_out->convertToXML(appNode);

        // Create document and save it
        std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
        doc->saveFile(out);
    }
}

/**
 * transformGraph ()
 * The function transforms the SDF graph.
 */
void transformGraph(std::ostream &out) {
    CNode *csdfNode = nullptr;
    CNode *csdfPropertiesNode = nullptr;

    // Find sdf graph in XML structure
    csdfNode = CGetChildNode(settings.xmlAppGraph, "csdf");
    if (csdfNode == nullptr) {
        throw CException("Invalid xml file - missing 'csdf' node");
    }
    csdfPropertiesNode = CGetChildNode(settings.xmlAppGraph, "csdfProperties");

    // Construction CSDF graph model
    auto csdfGraph = constructTimedCSDFgraph(csdfNode, csdfPropertiesNode);

    // The actual transformation...
    transformGraph(*csdfGraph, settings.arguments, out);
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
