/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3print.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 23, 2007
 *
 *  Function        :   (C)SDFG print functionality
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *
 * $Id: sdf3print.cc,v 1.5.2.1 2009-03-20 20:35:30 sander Exp $
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

#include "sdf3print.h"
#include "maxplus/algebra/mpmatrix.h"
#include "sdf/analysis/maxplus/mpexplore.h"
#include "sdf/sdf.h" // IWYU pragma: export - indirectly includes
#include <memory>
#include <utility>

using namespace SDF;
using namespace MaxPlus;
using namespace MaxPlusAnalysis;

constexpr uint64_t DefaultStackSz = 1000;
constexpr uint64_t DefaultHashSz = 10000;
constexpr uint64_t DefaultDepStackSz = 1000;


namespace SDF {

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

    // xml doc to hold on to
    std::unique_ptr<CDoc> appGraphDoc;
    std::unique_ptr<CDoc> archGraphDoc;
    std::unique_ptr<CDoc> mappingDoc;
    std::unique_ptr<CDoc> usageDoc;
    std::unique_ptr<CDoc> messageSetDoc;

    // Output file
    CString outputFile;

    // Iterations
    uint iterations{};

    // Switch argument(s) given to analysis algorithm
    CPairs arguments;

    // Application graph
    CNode *xmlAppGraph{};

    // Architecture graph
    CNode *xmlArchGraph{};

    // Mapping
    CNode *xmlMapping{};

    // Usage
    CNode *xmlUsage{};

    // Interconnect messages
    CNode *xmlMessagesSet{};
};

namespace {

/**
 * settings
 * Program settings->
 */
std::unique_ptr<Settings> settings; // NOLINT(*non-const-global-variables)

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << std::endl;
    out << "Usage: " << TOOL << " --graph <file> --format <type>";
    out << " [--output <file>]" << std::endl;
    out << "   --graph  <file>     input SDF graph" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
    out << "   --iters <float>     number of SDF graph iterations" << std::endl;
    out << "   --format <type>     output format:" << std::endl;
    out << "       hapi                                      " << std::endl;
    out << "       dot                                       " << std::endl;
    out << "       txt                                       " << std::endl;
    out << "       xmltrace                                  " << std::endl;
    out << "       tnotrace                                  " << std::endl;
    out << "       buffy(outputActor(<value>))               " << std::endl;
    out << "       selfTimedSchedule                         " << std::endl;
    out << "       buffyThroughput(outputActor(<value>))     " << std::endl;
    out << "       stateSpaceThroughput(outputActor(<value>))" << std::endl;
    out << "       maxplusmatrix                             " << std::endl;
    out << "       islinearsystemdeadlockfree                " << std::endl;
    out << "       maxpluslinearsystem                       " << std::endl;
    out << "       maxplusmatrixLatex                        " << std::endl;
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
            settings->graphFile = argv[arg];
        } else if (argv[arg] == CString("--output") && arg + 1 < argc) {
            arg++;
            settings->outputFile = argv[arg];
        } else if (argv[arg] == CString("--iters") && arg + 1 < argc) {
            arg++;
            int iter = std::stoi(argv[arg]);
            if (iter >= 0) {
                settings->iterations = static_cast<uint>(iter);
            } else {
                throw CException("Iterations must be unsigned integer");
            }
        } else if (argv[arg] == CString("--format") && arg + 1 < argc) {
            arg++;
            settings->arguments = parseSwitchArgument(argv[arg]);
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
 * loadUsageFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the resource usage.
 */
CNode *loadUsageFromFile(const CString &file, std::unique_ptr<CDoc>& usageDoc, const CString& module) {

    // Open file
    usageDoc = CParseFile(file);
    if (usageDoc == nullptr || usageDoc->getXmlDoc()==nullptr) {
        throw CException("Failed loading usage from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    CNode *sdf3Node = usageDoc->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get usage node
    CNode *usageNode = CGetChildNode(sdf3Node, "systemUsage");

    return usageNode;
}

/**
 * loadMessagesSetFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the interconnect messages.
 */
CNode *loadMessagesSetFromFile(const CString &file, std::unique_ptr<CDoc>& messagesSetDoc, const CString &module) {

    // Open file
    messagesSetDoc = CParseFile(file);
    if (messagesSetDoc == nullptr || messagesSetDoc->getXmlDoc()==nullptr) {
        throw CException("Failed loading usage from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    CNode *sdf3Node = messagesSetDoc->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get usage node
    CNode *messagesSetNode = CGetChildNode(sdf3Node, "messagesSet");

    return messagesSetNode;
}

/**
 * initSettings ()
 * The function initializes the program settings->
 */
void initSettings(int argc, char **argv)
// void initSettings()
{
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

    // Load usage
    settings->xmlUsage = loadUsageFromFile(settings->graphFile, settings->usageDoc, MODULE);

    // Interconnect messages
    settings->xmlMessagesSet = loadMessagesSetFromFile(settings->graphFile, settings->messageSetDoc, MODULE);
}

/**
 * printSDFG ()
 * The function outputs the SDF graph in the requested format.
 */
void printSDFG(const TimedSDFgraph& g, CPairs &format, std::ostream &out) { // NOLINT(*cognitive-complexity)
    if (format.front().key == "dot") {
        outputSDFasDot(g, out);
    } else if (format.front().key == "hapi") {
        outputSDFasHAPI(g);
    } else if (format.front().key == "txt") {
        g.print(out);
    } else if (format.front().key == "xmltrace" || format.front().key == "tnotrace") {
        MaxPlusAnalysis::Exploration mpe;
        mpe.g = &g;

        // Get vector sequence of self timed execution of SDF
        auto [u, s, y] = mpe.selfTimedStateSpaceSchedule(settings->iterations);

        // prepare trace of firings
        TraceOfFirings tof;

        // create firing trace
        for (uint i=0; i<settings->iterations; i++) {
            // add actor firings
            for (const auto &actor : g.getActors()) {
                auto* tActor = dynamic_cast<TimedSDFactor *> (actor.get());

                auto actorFiring = SDF::TraceOfFirings::createActorFiring();
                actorFiring->setName(tActor->getName());
                actorFiring->setDuration(MPTime(tActor->getExecutionTime()));
                actorFiring->setIteration(i);
                actorFiring->setTime(s[i]->get(tActor->getId()));
                tof.addActorFiring(actorFiring);
            }

            // add input firings
            for (const auto &input : g.getInputs()) {
                auto inputFiring = SDF::TraceOfFirings::createFiring();
                inputFiring->setName(input->getName());
                inputFiring->setTime(u[i]->get(input->getId()));
                inputFiring->setIteration(i);
                tof.addInputFiring(inputFiring);
            }

            // add output firing
            for (const auto &output : g.getOutputs()) {
                auto outputFiring = SDF::TraceOfFirings::createFiring();
                outputFiring->setName(output->getName());
                outputFiring->setTime(y[i]->get(output->getId()));
                outputFiring->setIteration(i);
                tof.addOutputFiring(outputFiring);
            }
        }

        // create dependencies
        for (uint i=0; i<settings->iterations; i++) {
            // add actor dependencies
            for (const auto &c : g.getChannels()) {
                auto *src_a = dynamic_cast<TimedSDFactor *>(c->getSrcActor());
                auto *dst_a = dynamic_cast<TimedSDFactor *>(c->getDstActor());
                auto it = c->getInitialTokens();
                auto srcETneg = src_a->getExecutionTime() < 0;
                auto dstETneg = dst_a->getExecutionTime() < 0;

                // skip dependencies between actor firings outside iteration count
                if(it + i >= settings->iterations) {
                    continue;
                }

                // Trace4CPS doesn't allow negative timed actions,
                // so we manually set dependency directions
                DependencyType depType = CendTOCstart;
                if (!srcETneg && !dstETneg) { depType = CendTOCstart; }
                else if ((!srcETneg && dstETneg)) { depType = CendTOCend; }
                else if ((srcETneg && !dstETneg)) { depType = CstartTOCstart; }
                else {depType = CstartTOCend;}

                auto dep = SDF::TraceOfFirings::createDependency();
                dep->setDType(depType);
                dep->setSrcFiring(tof.getActorFiring(src_a->getName(), i));
                dep->setDstFiring(tof.getActorFiring(dst_a->getName(), it + i));
                tof.addDependency(dep);
            }

            // add input dependencies
            for (const auto &ic : g.getInputChannels()) {
                auto *src_i = ic->getSrcInput();
                auto *dst_a = dynamic_cast<TimedSDFactor *>(ic->getDstActor());
                auto dstETneg = dst_a->getExecutionTime() < 0;

                auto dep = SDF::TraceOfFirings::createDependency();
                dep->setDType(dstETneg ? EventToCEnd : EventToCStart);
                dep->setSrcFiring(tof.getInputFiring(src_i->getName(), i));
                dep->setDstFiring(tof.getActorFiring(dst_a->getName(), i));
                tof.addDependency(dep);
            }

            // add output dependencies
            for (const auto &oc : g.getOutputChannels()) {
                auto *src_a = dynamic_cast<TimedSDFactor *>(oc->getSrcActor());
                auto *dst_o = oc->getDstOutput();
                auto srcETneg = src_a->getExecutionTime() < 0;

                auto dep = SDF::TraceOfFirings::createDependency();
                dep->setDType(srcETneg ? CstartToEvent : CendToEvent);
                dep->setSrcFiring(tof.getActorFiring(src_a->getName(), i));
                dep->setDstFiring(tof.getOutputFiring(dst_o->getName(), i));
                tof.addDependency(dep);
            }
        }

        // create xml or tno trace file
        if (format.front().key == "xmltrace") {
            tof.xmlTrace(settings->outputFile);
        } else {
            tof.tnoTrace(settings->outputFile);
        }

    } else if (format.front().key == "buffy") {
        uint64_t stackSz = DefaultStackSz;
        uint64_t hashSz = DefaultHashSz;
        uint64_t depStackSz = DefaultDepStackSz;
        CString outputActor = "";

        // Parse the value of the key
        CPairs options = parseSwitchArgument(format.front().value);
        for (const auto &[key, value] : options) {

            if (key == "outputActor") {
                outputActor = value;
            } else if (key == "stackSz") {
                stackSz = static_cast<uint>(value);
            } else if (key == "hashSz") {
                hashSz = static_cast<uint>(value);
            } else if (key == "depStackSz") {
                depStackSz = static_cast<uint>(value);
            }
        }

        // Check arguments
        if (outputActor.empty()) {
            throw CException("Missing outputActor.");
        }

        outputSDFasBuffyModel(g, out, outputActor, stackSz, hashSz, depStackSz);
    } else if (format.front().key == "buffyThroughput") {
        uint64_t stackSz = DefaultStackSz;
        uint64_t hashSz = DefaultHashSz;
        uint64_t depStackSz = DefaultDepStackSz;
        CString outputActor = "";

        // Parse the value of the key
        CPairs options = parseSwitchArgument(format.front().value);
        for (const auto &[key, value] : options) {
            if (key == "outputActor") {
                outputActor = value;
            } else if (key == "stackSz") {
                stackSz = static_cast<uint>(value);
            } else if (key == "hashSz") {
                hashSz = static_cast<uint>(value);
            } else if (key == "depStackSz") {
                depStackSz = static_cast<uint>(value);
            }
        }

        // Check arguments
        if (outputActor.empty()) {
            throw CException("Missing outputActor.");
        }

        outputSDFasBuffyModelThroughput(g, out, outputActor, stackSz, hashSz, depStackSz);
    } else if (format.front().key == "stateSpaceThroughput") {
        uint64_t stackSz = DefaultStackSz;
        uint64_t hashSz = DefaultHashSz;

        // Parse the value of the key
        CPairs options = parseSwitchArgument(format.front().value);
        for (const auto &[key, value] : options) {

            if (key == "stackSz") {
                stackSz = static_cast<uint>(value);
            } else if (key == "hashSz") {
                hashSz = static_cast<uint>(value);
            }
        }

        outputSDFasStateSpaceThroughputModel(g, out, stackSz, hashSz);
    } else if (format.front().key == "selfTimedSchedule") {
        outputSDFasSelfTimedScheduleModel(g, out);
    } else if (format.front().key == "maxplusmatrix") {
        std::pair<std::unique_ptr<GraphDecoration::Graph>, std::unique_ptr<GraphDecoration::Graph>> Graphs =
                GraphDecoration::Graph::OpenGraphConversion(g);
        MaxPlusAnalysis::Exploration mpe;

        mpe.OG = std::move(Graphs.first);
        mpe.G = std::move(Graphs.second);
        mpe.g = &g;

        auto M = std::unique_ptr<Matrix>(mpe.convertToMaxPlusMatrix());

        MPString mStr;
        M->toMatlabString(mStr);
        out << mStr << std::endl;

        CStrings statesLabels = mpe.getMatrixStateLabelling();
        CStrings inputsLabels = mpe.getMatrixInputLabelling();
        CStrings outputsLabels = mpe.getMatrixOutputLabelling();
        out << "State labels = [";
        out << CString::join(statesLabels, ", ");
        out << "]" << std::endl;
        if (!inputsLabels.empty()) {
            out << "Input labels = [";
            out << CString::join(inputsLabels, ", ");
            out << "]" << std::endl;
        }
        if (!outputsLabels.empty()) {
            out << "Output labels = [";
            out << CString::join(outputsLabels, ", ");
            out << "]" << std::endl;
        }
    } else if (format.front().key == "islinearsystemdeadlockfree") {
        MaxPlusAnalysis::Exploration mpe;
        mpe.g = &g;

        if (mpe.isLinearStateSpaceDeadlocking()) {
            out << "Linear state space deadlocks." << std::endl;
        } else {
            out << "Linear state space is deadlock free." << std::endl;
        }
    } else if (format.front().key == "maxpluslinearsystem") {
        MaxPlusAnalysis::Exploration mpe;
        mpe.g = &g;

        auto [A, B, C, D, H, J] = mpe.computeLinearStateSpaceMatrices();

        MPString mStr;
        A->toMatlabString(mStr);
        out << "State matrix A:" << std::endl << mStr << std::endl;
        B->toMatlabString(mStr);
        out << "State matrix B:" << std::endl << mStr << std::endl;
        C->toMatlabString(mStr);
        out << "State matrix C:" << std::endl << mStr << std::endl;
        D->toMatlabString(mStr);
        out << "State matrix D:" << std::endl << mStr << std::endl;
        H->toMatlabString(mStr);
        out << "State matrix H:" << std::endl << mStr << std::endl;
        J->toMatlabString(mStr);
        out << "State matrix J:" << std::endl << mStr << std::endl;

        CStrings actorLabels = mpe.getMatrixActorLabelling();
        CStrings statesLabels = mpe.getMatrixStateLabelling();
        CStrings inputsLabels = mpe.getMatrixInputLabelling();
        CStrings outputsLabels = mpe.getMatrixOutputLabelling();
        out << "Actor labels = [";
        out << CString::join(actorLabels, ", ");
        out << "]" << std::endl;
        out << "State labels = [";
        out << CString::join(statesLabels, ", ");
        out << "]" << std::endl;
        if (!inputsLabels.empty()) {
            out << "Input labels = [";
            out << CString::join(inputsLabels, ", ");
            out << "]" << std::endl;
        }
        if (!outputsLabels.empty()) {
            out << "Output labels = [";
            out << CString::join(outputsLabels, ", ");
            out << "]" << std::endl;
        }

    } else if (format.front().key == "maxplusmatrixLatex") {
        std::pair<std::unique_ptr<GraphDecoration::Graph>, std::unique_ptr<GraphDecoration::Graph>> Graphs =
                GraphDecoration::Graph::OpenGraphConversion(g);
        MaxPlusAnalysis::Exploration mpe;

        mpe.OG = std::move(Graphs.first);
        mpe.G = std::move(Graphs.second);
        mpe.g = &g;

        auto M = std::unique_ptr<MaxPlus::Matrix>(mpe.convertToMaxPlusMatrix());
        CStrings statesLabels = mpe.getMatrixStateLabelling();
        CStrings inputsLabels = mpe.getMatrixInputLabelling();
        CStrings outputsLabels = mpe.getMatrixOutputLabelling();
        CString outString = "\\begin{ bmatrix }\n";
        CStrings::iterator j;
        for (j = statesLabels.begin(); j != statesLabels.end(); j++) {
            outString += *j;
            outString += "\\\\";
            outString += "\n";
        }
        for (j = outputsLabels.begin(); j != outputsLabels.end(); j++) {
            outString += *j;
            outString += "\\\\";
            outString += "\n";
        }

        outString += "\\end{bmatrix} = \n";
        out << outString << std::endl;

        MPString mStr;
        M->toLaTeXString(mStr);
        out << mStr << std::endl;

        outString = "\\otimes \\begin{ bmatrix }\n";

        for (j = statesLabels.begin(); j != statesLabels.end(); j++) {
            outString += *j;
            outString += "\\\\";
            outString += "\n";
        }
        for (j = inputsLabels.begin(); j != inputsLabels.end(); j++) {
            outString += *j;
            outString += "\\\\";
            outString += "\n";
        }

        outString += "\\end{bmatrix} \n";
        out << outString << std::endl;

    } else {
        throw CException("Invalid output format requested.");
    }
}

/**
 * printSDFG ()
 * The function outputs the SDF graph.
 */
void printSDFG(std::ostream &out) {

    // Find sdf graph in XML structure
    CNode *sdfNode = CGetChildNode(settings->xmlAppGraph, "sdf");
    if (sdfNode == nullptr) {
        throw CException("Invalid xml file - missing 'sdf' node");
    }
    CNode *sdfPropertiesNode = CGetChildNode(settings->xmlAppGraph, "sdfProperties");

    // Construction SDF graph model
    TimedSDFgraph sdfGraph;
    sdfGraph.construct(sdfNode, sdfPropertiesNode);

    // The actual printing...
    printSDFG(sdfGraph, settings->arguments, out);

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
    settings = std::make_unique<Settings>();

    try {
        // Initialize the program
        initSettings(argc, argv);
        // initSettings();

        // Set output stream
        if (!settings->outputFile.empty()) {
            out.open(settings->outputFile.c_str());
        } else {
            (static_cast<std::ostream &>(out)).rdbuf(std::cout.rdbuf());
        }

        // Perform requested actions
        printSDFG(out);
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
