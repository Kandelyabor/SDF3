/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2005
 *
 *  Function        :   Free SDF graphs
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: sdf3generate.cc,v 1.4 2008-10-16 14:44:29 sander Exp $
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

#include "sdf3generate.h"
#include "sdf/sdf.h" // IWYU pragma: keep
using namespace SDF;
namespace SDF {

/**
 * Settings
 * Struct to store program settings.
 */
using Settings = struct Settings {
    // settings file
    CString settingsFile;

    // output file
    CString outputFile;

    // Graph
    bool stronglyConnected{};
    bool acyclic{};
    bool multigraph{};

    // Actors
    uint nrActors{};
    double avgInDegree{};
    double varInDegree{};
    double minInDegree{};
    double maxInDegree{};
    double avgOutDegree{};
    double varOutDegree{};
    double minOutDegree{};
    double maxOutDegree{};

    // Rate
    double avgRate{};
    double varRate{};
    double minRate{};
    double maxRate{};
    uint repetitionVectorSum{};

    // Execution time
    bool execTime{};
    uint nrProcTypes{};
    double mapChance{};
    double avgExecTime{};
    double varExecTime{};
    double minExecTime{};
    double maxExecTime{};

    // State size
    bool stateSize{};
    double avgStateSize{};
    double varStateSize{};
    double minStateSize{};
    double maxStateSize{};

    // Token size
    bool tokenSize{};
    double avgTokenSize{};
    double varTokenSize{};
    double minTokenSize{};
    double maxTokenSize{};

    // Buffer size
    bool bufferSize{};

    // Bandwidth requirement
    bool bandwidthRequirement{};
    double avgBandwidth{};
    double varBandwidth{};
    double minBandwidth{};
    double maxBandwidth{};

    // Latency requirement
    bool latencyRequirement{};
    double avgLatency{};
    double varLatency{};
    double minLatency{};
    double maxLatency{};

    // Throughput constraint
    bool throughputConstraint{};
    uint autoConcurrencyDegree{};
    double throughputScaleFactor{};

    // Initial tokens
    double initialTokenProp{};
    double tokenFreeProp{};
    bool tokenFreeCycle{};

    // MCM
    bool integerMCM{};
};

namespace {


/**
 * settings
 * Program settings.
 */

Settings settings; // NOLINT(*const-global-variables,cert-err58-cpp)


/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << std::endl;
    out << "Usage: " << TOOL << " [--settings <file> --output <file>]" << std::endl;
    out << "   --settings  <file>  settings for the graph generator (default: ";
    out << "sdf3.opt)" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
}

/**
 * parseCommandLine ()
 * The function parses the command line arguments and add info to the
 * supplied settings structure.
 */
void parseCommandLine(int argc, char **argv) {
    int arg = 1;

    while (arg < argc) {
        if (argv[arg] == CString("--output") && arg + 1 < argc) {
            arg++;
            settings.outputFile = argv[arg];
        } else if (argv[arg] == CString("--settings") && arg + 1 < argc) {
            arg++;
            settings.settingsFile = argv[arg];
        } else {
            helpMessage(std::cerr);
            throw CException("");
        }

        // Next argument
        arg++;
    }
}

/**
 * parseSettingsFile ()
 * The function parses all settings from the file.
 */
void parseSettingsFile(const CString &module, const CString &type) {

    // Open settings file and get root node
    std::unique_ptr<CDoc> settingsDoc = CParseFile(settings.settingsFile);
    CNode *sdf3Node = nullptr;
    if (settingsDoc != nullptr && settingsDoc->getXmlDoc()!=nullptr) {
        sdf3Node = settingsDoc->getRootNode();
    }
    if (sdf3Node == nullptr) {
        throw CException("Failed opening '" + settings.settingsFile + "'.");
    }

    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + settings.settingsFile + "' is not of type '"
                         + module + "'.");
    }

    // Get the settings element of the tool
    CNode *settingsNode = nullptr;
    for (settingsNode = CGetChildNode(sdf3Node, "settings"); settingsNode != nullptr;
         settingsNode = CNextNode(settingsNode, "settings")) {
        if (CGetAttribute(settingsNode, "type") == type) {
            break;
        }
    }

    // Found the correct settings element?
    if (settingsNode == nullptr) {
        throw CException("File '" + settings.settingsFile
                         + "' contains no "
                           " settings of type '"
                         + type + "'.");
    }

    // Graph
    CNode *graphNode = CGetChildNode(settingsNode, "graph");
    if (graphNode == nullptr) {
        throw CException("No graph element in settings.");
    }

    CNode *actorsNode = CGetChildNode(graphNode, "actors");
    if (actorsNode == nullptr) {
        throw CException("Missing actors element in settings.");
    }
    CNode *degreeNode = CGetChildNode(graphNode, "degree");
    if (degreeNode == nullptr) {
        throw CException("Missing degree element in settings.");
    }
    CNode *rateNode = CGetChildNode(graphNode, "rate");
    if (rateNode == nullptr) {
        throw CException("Missing rate element in settings.");
    }

    settings.nrActors = static_cast<uint>(CGetAttribute(actorsNode, "nr"));
    settings.avgInDegree = static_cast<double>(CGetAttribute(degreeNode, "avg"));
    settings.varInDegree = static_cast<double>(CGetAttribute(degreeNode, "var"));
    settings.minInDegree = static_cast<double>(CGetAttribute(degreeNode, "min"));
    settings.maxInDegree = static_cast<double>(CGetAttribute(degreeNode, "max"));
    settings.avgOutDegree = static_cast<double>(CGetAttribute(degreeNode, "avg"));
    settings.varOutDegree = static_cast<double>(CGetAttribute(degreeNode, "var"));
    settings.minOutDegree = static_cast<double>(CGetAttribute(degreeNode, "min"));
    settings.maxOutDegree = static_cast<double>(CGetAttribute(degreeNode, "max"));
    settings.avgRate = static_cast<double>(CGetAttribute(rateNode, "avg"));
    settings.varRate = static_cast<double>(CGetAttribute(rateNode, "var"));
    settings.minRate = static_cast<double>(CGetAttribute(rateNode, "min"));
    settings.maxRate = static_cast<double>(CGetAttribute(rateNode, "max"));

    // Sum of the repetition vector constrained?
    if (CHasAttribute(rateNode, "repetitionVectorSum")) {
        settings.repetitionVectorSum =
                static_cast<uint>(CGetAttribute(rateNode, "repetitionVectorSum"));
    }

    // Graph structure
    if (CHasChildNode(graphNode, "structure")) {
        CNode *structureNode = CGetChildNode(graphNode, "structure");

        if (CHasAttribute(structureNode, "stronglyConnected")) {
            if (CGetAttribute(structureNode, "stronglyConnected") == CString("true")) {
                settings.stronglyConnected = true;
            }
        }

        if (CHasAttribute(structureNode, "acyclic")) {
            if (CGetAttribute(structureNode, "acyclic") == CString("true")) {
                settings.acyclic = true;
            }
        }

        if (CHasAttribute(structureNode, "multigraph")) {
            if (CGetAttribute(structureNode, "multigraph") == CString("false")) {
                settings.multigraph = false;
            }
        }
    }

    // Initial tokens
    if (CHasChildNode(graphNode, "initialTokens")) {
        CNode *initialTokensNode = CGetChildNode(graphNode, "initialTokens");

        if (CHasAttribute(initialTokensNode, "prop")) {
            settings.initialTokenProp = static_cast<double>(CGetAttribute(initialTokensNode, "prop"));
        }

        if (CHasAttribute(initialTokensNode, "tokenFreeProp")) {
            settings.tokenFreeProp = static_cast<double>(CGetAttribute(initialTokensNode, "tokenFreeProp"));
        }

        if (CHasAttribute(initialTokensNode, "tokenFreeCycle")) {
            if (CGetAttribute(initialTokensNode, "tokenFreeCycle") == CString("true")) {
                settings.tokenFreeCycle = true;
            }
        }
    }

    // Graph properties
    if (CHasChildNode(settingsNode, "graphProperties")) {
        CNode *graphPropertiesNode = CGetChildNode(settingsNode, "graphProperties");

        if (CHasChildNode(graphPropertiesNode, "procs")
            && CHasChildNode(graphPropertiesNode, "execTime")) {
            CNode *procsNode = CGetChildNode(graphPropertiesNode, "procs");
            CNode *execTimeNode = CGetChildNode(graphPropertiesNode, "execTime");

            settings.execTime = true;
            settings.nrProcTypes = static_cast<uint>(CGetAttribute(procsNode, "nrTypes"));
            settings.mapChance = static_cast<double>(CGetAttribute(procsNode, "mapChance"));
            settings.avgExecTime = static_cast<double>(CGetAttribute(execTimeNode, "avg"));
            settings.varExecTime = static_cast<double>(CGetAttribute(execTimeNode, "var"));
            settings.minExecTime = static_cast<double>(CGetAttribute(execTimeNode, "min"));
            settings.maxExecTime = static_cast<double>(CGetAttribute(execTimeNode, "max"));
        }

        if (CHasChildNode(graphPropertiesNode, "stateSize")) {
            CNode *stateSizeNode = CGetChildNode(graphPropertiesNode, "stateSize");

            settings.stateSize = true;
            settings.avgStateSize = static_cast<double>(CGetAttribute(stateSizeNode, "avg"));
            settings.varStateSize = static_cast<double>(CGetAttribute(stateSizeNode, "var"));
            settings.minStateSize = static_cast<double>(CGetAttribute(stateSizeNode, "min"));
            settings.maxStateSize = static_cast<double>(CGetAttribute(stateSizeNode, "max"));
        }

        if (CHasChildNode(graphPropertiesNode, "tokenSize")) {
            CNode *tokenSizeNode = CGetChildNode(graphPropertiesNode, "tokenSize");

            settings.tokenSize = true;
            settings.avgTokenSize = static_cast<double>(CGetAttribute(tokenSizeNode, "avg"));
            settings.varTokenSize = static_cast<double>(CGetAttribute(tokenSizeNode, "var"));
            settings.minTokenSize = static_cast<double>(CGetAttribute(tokenSizeNode, "min"));
            settings.maxTokenSize = static_cast<double>(CGetAttribute(tokenSizeNode, "max"));
        }

        if (CHasChildNode(graphPropertiesNode, "bufferSize")) {
            settings.bufferSize = true;
        }

        if (CHasChildNode(graphPropertiesNode, "bandwidthRequirement")) {
            CNode *bandwidthNode = CGetChildNode(graphPropertiesNode, "bandwidthRequirement");

            settings.bandwidthRequirement = true;
            settings.avgBandwidth = static_cast<double>(CGetAttribute(bandwidthNode, "avg"));
            settings.varBandwidth = static_cast<double>(CGetAttribute(bandwidthNode, "var"));
            settings.minBandwidth = static_cast<double>(CGetAttribute(bandwidthNode, "min"));
            settings.maxBandwidth = static_cast<double>(CGetAttribute(bandwidthNode, "max"));
        }

        if (CHasChildNode(graphPropertiesNode, "latencyRequirement")) {
            CNode *latencyNode = CGetChildNode(graphPropertiesNode, "latencyRequirement");

            settings.latencyRequirement = true;
            settings.avgLatency = static_cast<double>(CGetAttribute(latencyNode, "avg"));
            settings.varLatency = static_cast<double>(CGetAttribute(latencyNode, "var"));
            settings.minLatency = static_cast<double>(CGetAttribute(latencyNode, "min"));
            settings.maxLatency = static_cast<double>(CGetAttribute(latencyNode, "max"));
        }

        if (CHasChildNode(graphPropertiesNode, "throughputConstraint")) {
            CNode *throughputNode = CGetChildNode(graphPropertiesNode, "throughputConstraint");

            settings.throughputConstraint = true;

            if (CHasAttribute(throughputNode, "autoConcurrencyDegree")) {
                settings.autoConcurrencyDegree =
                        static_cast<uint>(CGetAttribute(throughputNode, "autoConcurrencyDegree"));
            }
            if (CHasAttribute(throughputNode, "scaleFactor")) {
                settings.throughputScaleFactor =
                        static_cast<double>(CGetAttribute(throughputNode, "scaleFactor"));
            }
        }

        if (CHasChildNode(graphPropertiesNode, "integerMCM")) {
            settings.integerMCM = true;
        }
    }
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void setDefaults() {
    settings.settingsFile = "sdf3.opt";
    settings.stronglyConnected = false;
    settings.acyclic = false;
    settings.multigraph = true;
    settings.nrActors = 0;
    settings.avgInDegree = 0;
    settings.varInDegree = 0;
    settings.minInDegree = 0;
    settings.maxInDegree = 0;
    settings.avgOutDegree = 0;
    settings.varOutDegree = 0;
    settings.minOutDegree = 0;
    settings.maxOutDegree = 0;
    settings.avgRate = 0;
    settings.varRate = 0;
    settings.minRate = 0;
    settings.maxRate = 0;
    settings.repetitionVectorSum = 0;
    settings.execTime = false;
    settings.nrProcTypes = 0;
    settings.mapChance = 0;
    settings.avgExecTime = 0;
    settings.varExecTime = 0;
    settings.minExecTime = 0;
    settings.maxExecTime = 0;
    settings.stateSize = false;
    settings.avgStateSize = 0;
    settings.varStateSize = 0;
    settings.minStateSize = 0;
    settings.maxStateSize = 0;
    settings.tokenSize = false;
    settings.avgTokenSize = 0;
    settings.varTokenSize = 0;
    settings.minTokenSize = 0;
    settings.maxTokenSize = 0;
    settings.bufferSize = false;
    settings.bandwidthRequirement = false;
    settings.avgBandwidth = 0;
    settings.varBandwidth = 0;
    settings.minBandwidth = 0;
    settings.maxBandwidth = 0;
    settings.latencyRequirement = false;
    settings.avgLatency = 0;
    settings.varLatency = 0;
    settings.minLatency = 0;
    settings.maxLatency = 0;
    settings.throughputConstraint = false;
    settings.autoConcurrencyDegree = 0;
    settings.throughputScaleFactor = 1;
    constexpr double OneTenth = 0.1;
    settings.initialTokenProp = OneTenth; // NOLINT(*magic-numbers)
    settings.tokenFreeProp = OneTenth;
    settings.tokenFreeCycle = false;
    settings.integerMCM = false;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void initSettings(int argc, char **argv) {
    // Defaults
    setDefaults();

    // Parse the command line
    parseCommandLine(argc, argv);

    // Parse settings
    parseSettingsFile(MODULE, SETTINGS_TYPE);
}

/**
 * generateRandomSDFG ()
 * Generate a random SDF graph which is connected, consistent and deadlock-free.
 */
void generateRandomSDFG(std::ostream &out) {
    std::unique_ptr<TimedSDFgraph> sdfGraph = generateSDFgraph(settings.nrActors,
                                               settings.avgInDegree,
                                               settings.varInDegree,
                                               settings.minInDegree,
                                               settings.maxInDegree,
                                               settings.avgOutDegree,
                                               settings.varOutDegree,
                                               settings.minOutDegree,
                                               settings.maxOutDegree,
                                               settings.avgRate,
                                               settings.varRate,
                                               settings.minRate,
                                               settings.maxRate,
                                               settings.acyclic,
                                               settings.stronglyConnected,
                                               settings.initialTokenProp,
                                               settings.tokenFreeProp,
                                               settings.tokenFreeCycle,
                                               settings.repetitionVectorSum,
                                               settings.multigraph);

    generateSDFgraphProperties(*sdfGraph,
                               settings.execTime,
                               settings.nrProcTypes,
                               settings.mapChance,
                               settings.avgExecTime,
                               settings.varExecTime,
                               settings.minExecTime,
                               settings.maxExecTime,
                               settings.stateSize,
                               settings.avgStateSize,
                               settings.varStateSize,
                               settings.minStateSize,
                               settings.maxStateSize,
                               settings.tokenSize,
                               settings.avgTokenSize,
                               settings.varTokenSize,
                               settings.minTokenSize,
                               settings.maxTokenSize,
                               settings.throughputConstraint,
                               settings.autoConcurrencyDegree,
                               settings.throughputScaleFactor,
                               settings.bandwidthRequirement,
                               settings.avgBandwidth,
                               settings.varBandwidth,
                               settings.minBandwidth,
                               settings.maxBandwidth,
                               settings.bufferSize,
                               settings.latencyRequirement,
                               settings.avgLatency,
                               settings.varLatency,
                               settings.minLatency,
                               settings.maxLatency,
                               settings.integerMCM);

    outputSDFasXML(*sdfGraph, out);
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

    try {
        // Initialize the program
        initSettings(argc, argv);

        // Set output stream
        if (!settings.outputFile.empty()) {
            out.open(settings.outputFile.c_str());
        } else {
            (static_cast<std::ostream &>(out)).rdbuf(std::cout.rdbuf());
        }

        // Generate random graph
        generateRandomSDFG(out);
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
