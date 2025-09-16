/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3analyze.cc
 *
 *  Author          :   Sander Stuijk (s.stuijk@tue.nl)
 *
 *  Date            :   21 April 2009
 *
 *  Function        :   FSM-based SADF Graph Analysis Functionality
 *
 *  History         :
 *      21-04-09    :   Initial version.
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
#include "base/base.h" // IWYU pragma: keep
#include "fsmsadf/fsmsadf.h" // IWYU pragma: keep
#include "sdf/sdf.h" // IWYU pragma: keep
#include <cstdlib>
#include <memory>


namespace FSMSADF {

using namespace MPExplore;
using namespace SDF;


/**
 * initSettings ()
 * The function initializes the program settings.
 */
void ToolAnalyze::initSettings(const CString & /*modulename*/, int argc, char **argv) {
    this->setSettings(std::make_unique<AnalyzeSettings>());

    // Bypass Tool::initSettings because we want different behaviour:
    // use opt if available, but overwrite settings from command-line

    // Defaults
    this->setDefaults();

    // Parse the command line
    Tool::parseCommandLine(argc, argv);

    // any post processing based on the settings
    this->postInitSettings();

    // Check required settings
    if (!this->checkRequiredSettings()) {
        helpMessage(std::cerr);
        throw CException("Settings not found.");
    }
}

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void ToolAnalyze::helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << std::endl;
    out << "Usage: " << TOOL << " [--settings <file>] [--graph <file> --algo <algorithm>]";
    out << " [--output <file>] [--seq <sequence> --traceFiles <file.txt,file.etf>]" << std::endl;
    out << "   --settings  <file>  settings for the analysis (default: sdf3.opt)" << std::endl;
    out << "   --graph  <file>     input SADF graph" << std::endl;
    out << "   --algo <algorithm>  analyze the graph with requested algorithm: " << std::endl;
    out << "       criticalscenariosequence" << std::endl;
    out << "       extractscenariographs" << std::endl;
    out << "       isolatescenarios" << std::endl;
    out << "       mpeigen" << std::endl;
    out << "       throughput (computes conservative throughput approximation, deprecated)"
        << std::endl;
    out << "       throughput-mpautomaton" << std::endl;
    out << "       throughput-regex" << std::endl;
    out << "       latency-regex" << std::endl;
    out << "       throughput-statespace (deprecated)" << std::endl;
    out << "       trace" << std::endl;
    out << "       vectortrace" << std::endl;
    out << "       schedule-check" << std::endl;
    out << "       buffersize-maxplus" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
    out << "For use with 'trace' algorithm only:" << std::endl;
    out << "   --seq  <sequence>     comma-separated sequence of scenarios" << std::endl;
    out << "   --traceFormat  <format>     <format> is either Trace4CPS or XML" << std::endl;
    out << "   --traceFiles  <file.txt,file.etf>     output files for trace in Trace4CPS format"
        << std::endl;
    out << "   --traceFile  <out.xml>     output files for trace in XML format" << std::endl;
    out << "For use with 'latency-regex' algorithm only:" << std::endl;
    out << "   --period  <number>     latency is computed relative to the period" << std::endl;
    out << std::endl;
}

bool ToolAnalyze::parseCommand(int argc, char **argv, int *arg) {
    if (argv[*arg] == CString("--settings") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().settingsFile = argv[*arg];
    } else if (argv[*arg] == CString("--graph") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().graphFile = argv[*arg];
    } else if (argv[*arg] == CString("--output") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().outputFile = argv[*arg];
    } else if ((argv[*arg] == CString("--traceFiles") && (*arg) + 1 < argc) || ((argv[*arg] == CString("--traceFile") && (*arg) + 1 < argc))) {
        (*arg)++;
        this->analyzeSettings().traceFiles = argv[*arg];
    } else if (argv[*arg] == CString("--seq") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().seq = argv[*arg];
    } else if (argv[*arg] == CString("--traceFormat") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().traceFormat = argv[*arg];
    } else if (argv[*arg] == CString("--algo") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().arguments = this->parseSwitchArgument(argv[*arg]);
    } else if (argv[*arg] == CString("--period") && (*arg) + 1 < argc) {
        (*arg)++;
        this->analyzeSettings().period = argv[*arg];
    } else {
        return false;
    }
    return true;
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void ToolAnalyze::setDefaults() { this->analyzeSettings().settingsFile = "sdf3.opt"; }

/**
 * parseSettingsFile ()
 * The function parses all settings from the file.
 */
void ToolAnalyze::parseSettingsFile(const CString &module, const CString &type) {
    CNode *sdf3Node = nullptr;
    CNode *settingsNode = nullptr;
    CString name;
    CString file;

    // Open settings file and get root node
    std::unique_ptr<CDoc> settingsDoc = CParseFile(this->analyzeSettings().settingsFile);
    sdf3Node = settingsDoc->getRootNode();
    if (sdf3Node == nullptr) {
        // turn off unnecessary notification
        // logInfo("Failed opening '" + this->analyzeSettings().settingsFile
        //         + "', fallback on command-line settings.");
        return;
    }

    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + this->analyzeSettings().settingsFile
                         + "' is not of type '" + module + "'.");
    }

    // Get the settings element of the tool
    for (settingsNode = CGetChildNode(sdf3Node, "settings"); settingsNode != nullptr;
         settingsNode = CNextNode(settingsNode, "settings")) {
        if (CGetAttribute(settingsNode, "type") == type) {
            break;
        }
    }

    // Found the correct settings element?
    if (settingsNode == nullptr) {
        logInfo("File '" + this->analyzeSettings().settingsFile
                + "' contains no "
                  " settings of type '"
                + type + "', fallback on command-line settings.");
        return;
    }

    // Get the graph file
    if (!CHasChildNode(settingsNode, "graph")) {
        throw CException("File '" + this->analyzeSettings().settingsFile + "' contains no graphFile.");
    }
    CNode *graphNode = CGetChildNode(settingsNode, "graph");
    this->analyzeSettings().graphFile = CGetAttribute(graphNode, "file");

    // Get the algorithm
    if (!CHasChildNode(settingsNode, "algo")) {
        throw CException("File '" + this->analyzeSettings().settingsFile + "' contains no algorithm.");
    }
    CNode *algoNode = CGetChildNode(settingsNode, "algo");
    CPair p;
    p.key = CGetAttribute(algoNode, "name");
    p.value = "";
    this->analyzeSettings().arguments.push_back(p);

    // Get the scenario sequence
    if (CHasChildNode(settingsNode, "seq")) {
        CNode *sequenceNode = CGetChildNode(settingsNode, "seq");
        this->analyzeSettings().seq = "";

        if (!CHasChildNode(sequenceNode, "entry")) {
            throw CException("Sequence has no entries.");
        }

        for (CNode *entryNode = CGetChildNode(sequenceNode, "entry"); entryNode != nullptr;
             entryNode = CNextNode(entryNode, "entry")) {
            if (!CHasAttribute(entryNode, "name")) {
                throw CException("Sequence entry has no scenario name.");
            }
            this->analyzeSettings().seq = this->analyzeSettings().seq + CGetAttribute(entryNode, "name") + ",";
        }
    }

    this->analyzeSettings().traceFiles = "";

    // Get the trace format
    if (CHasChildNode(settingsNode, "traceFormat")) {
        CNode *txtNode = CGetChildNode(settingsNode, "traceFormat");
        this->analyzeSettings().traceFormat = CGetAttribute(txtNode, "name").toUpper();
    }

    // Get the trace txt file
    if (CHasChildNode(settingsNode, "traceTxt")) {
        CNode *txtNode = CGetChildNode(settingsNode, "traceTxt");
        this->analyzeSettings().traceFiles = CGetAttribute(txtNode, "file") + ",";
    }

    // Get the trace etf file
    if (CHasChildNode(settingsNode, "traceEtf")) {
        CNode *etfNode = CGetChildNode(settingsNode, "traceEtf");
        this->analyzeSettings().traceFiles += CGetAttribute(etfNode, "file");
    }

    // Get the trace xml file
    if (CHasChildNode(settingsNode, "traceFile")) {
        CNode *txtNode = CGetChildNode(settingsNode, "traceFile");
        this->analyzeSettings().traceFiles = CGetAttribute(txtNode, "file");
    }

    // Get the output file
    if (CHasChildNode(settingsNode, "output")) {
        CNode *outputNode = CGetChildNode(settingsNode, "output");
        this->analyzeSettings().outputFile = CGetAttribute(outputNode, "file");
    }
}

bool ToolAnalyze::checkRequiredSettings() {
    if (this->analyzeSettings().graphFile.empty()) {
        return false;
    }
    if (this->analyzeSettings().arguments.empty()) {
        return false;
    };

    return true;
}

/**
 * postInitSettings ()
 *
 */
void ToolAnalyze::postInitSettings() {
    // Parse settings
    this->parseSettingsFile(MODULE, SETTINGS_TYPE);
}

/**
 * loadApplicationGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
void ToolAnalyze::loadGraphsFromFile(const CString &file,
                                     const CString &module,
                                     CNode **appGraphNode,
                                     CNode **archGraphNode,
                                     CNode **mappingGraphNode,
                                     std::unique_ptr<CDoc> *appGraphDoc) {
    CNode *sdf3Node = nullptr;

    // Open file
    *appGraphDoc = CParseFile(file);
    if (*appGraphDoc == nullptr || (*appGraphDoc)->getXmlDoc()==nullptr) {
        throw CException("Failed loading SADF Graph from '" + file + "'.");
    }

    // Locate the sdf3 root element and check module type
    sdf3Node = (*appGraphDoc)->getRootNode();
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + file
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get application graph node
    *appGraphNode = CGetChildNode(sdf3Node, "applicationGraph");
    if (*appGraphNode == nullptr) {
        throw CException("No applicationGraph node in '" + file + "'.");
    }

    // Get architecture graph node
    *archGraphNode = CGetChildNode(sdf3Node, "architectureGraph");

    // Get mapping graph node
    *mappingGraphNode = CGetChildNode(sdf3Node, "mapping");
}

void ToolAnalyze::cleanUp() {
    Tool::cleanUp();
    // cleanup the xml document
    if (this->xmlAppGraphDoc != nullptr) {
        this->xmlAppGraphDoc.reset();
    }
    // cleanup other xml resources
    xmlCleanup();
}

/**
 * AnalyzeGraph()
 * The function that calls the actual analysis functions
 */
void ToolAnalyze::analyzeGraph(std::ostream &out) { // NOLINT(*cognitive-complexity)
    CNode *applicationNode = nullptr;
    CTimer timer;

    // Find sdf graph in XML structure
    applicationNode = this->xmlAppGraph;

    // Construction SDF graph model
    auto fsmsadfGraph = std::make_unique<Graph>();
    fsmsadfGraph->constructFromXML(applicationNode);

    CPairs &analyze = this->analyzeSettings().arguments;

    // Select the correct algorithm
    if (analyze.front().key == "extractscenariographs") {
        // Print the graph which has just been loaded from file
        fsmsadfGraph->print(std::cout);

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
        fsmsadfGraph->convertToXML(appNode);

        // Create document and save it
        std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
        doc->saveFile(out, 1);

        const Scenarios& scenarios = fsmsadfGraph->getScenarios();
        for (const auto& s : scenarios) {
            std::unique_ptr<TimedSDFgraph> g = FSMSADF::Graph::extractSDFgraph(*s);
            outputSDFasXML(*g, out);
        }
    } else if (analyze.front().key == "throughput") {
        Throughput thr = 0;

        // Measure execution time
        startTimer(&timer);

        thr = throughputAnalysisGraph(fsmsadfGraph.get());
        std::cout << "Throughput (scenario graph):       " << thr << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;

        // Measure execution time
        startTimer(&timer);

        thr = throughputAnalysisUsingRefSchedule(*fsmsadfGraph);
        std::cout << "Throughput (reference schedule):   " << thr << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;

        // Measure execution time
        startTimer(&timer);

        thr = throughputAnalysisWithScenarioTransitions(*fsmsadfGraph);
        std::cout << "Throughput (scenario transitions): " << thr << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "isolatescenarios") {
        auto newFSMsadfGraph = std::unique_ptr<Graph>(fsmsadfGraph->clone(GraphComponent(nullptr, 0)));

        // Create a cloned copy of the graph
        newFSMsadfGraph->isolateScenarios();

        // Create XML
        CNode *sdf3Node = CNewNode("sdf3");
        CAddAttribute(sdf3Node, "version", "1.0");
        CAddAttribute(sdf3Node, "type", "fsmsadf");
        CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
        CAddAttribute(sdf3Node,
                      "xsi:noNamespaceSchemaLocation",
                      "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

        // Application graph node
        newFSMsadfGraph->convertToXML(CAddNode(sdf3Node, "applicationGraph"));

        // Create document and save it
        std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
        doc->saveFile(out);

    } else if (analyze.front().key == "mpeigen") {
        std::map<Scenario *, std::unique_ptr<MaxPlus::Vector>> eigenvector;
        std::map<Scenario *, MPTime> period;
        std::map<FSMtransition *, MPTime> delay;
        std::map<Scenario *, std::unique_ptr<MPExplore::Exploration>> mpe;

        if (!fsmsadfGraph->isStronglyConsistent()) {
            std::cout << "The graph is not strongly consistent. Cannot compute eigenvalues and "
                         "eigenvectors of the scenarios.";
        } else {

            // The set of scenarios...
            const Scenarios& scenarios = fsmsadfGraph->getScenarios();

            // Create Max-Plus exploration object for each scenario
            for (const auto& s : scenarios) {
                mpe[s.get()] = std::make_unique<MPExplore::Exploration>();
                mpe[s.get()]->G = std::make_unique<SGraph>(s->getScenarioGraph(), s.get());
            }

            // Compute eigenvector and period for each scenario
            for (const auto& s : scenarios) {
                std::unique_ptr<MaxPlus::Vector> v_s = nullptr;
                MPTime T_s;

                // Compute eigenvector and period of this scenario
                mpe[s.get()]->exploreEigen(&v_s, &T_s);

                // Store results
                eigenvector[s.get()] = std::make_unique<MaxPlus::Vector>(*v_s);
                period[s.get()] = T_s;

                // Output result
                std::cout << "eigenvalue: " << MPString(T_s) << std::endl;
                for (uint x = 0; x < v_s->getSize(); x++) {
                    std::cout << MPString(v_s->get(x)) << " ; ";
                }
                std::cout << std::endl;
            }
        }
    } else if (analyze.front().key == "throughput-statespace") {
        Throughput thr = 0;

        std::cout << "Note that the state-space based throughput computation is deprecated ";
        std::cout << "in favor of throughput computation with max-plus automaton ";
        std::cout << "(throughput-mpautomaton)." << std::endl;

        // Measure execution time
        startTimer(&timer);

        thr = throughputAnalysisGraphUsingStateSpace(*fsmsadfGraph);
        std::cout << "Throughput (state-space):          " << thr << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "throughput-mpautomaton") {
        Throughput thr = 0;

        // Measure execution time
        startTimer(&timer);

        thr = throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent(*fsmsadfGraph);
        std::cout << "Throughput (max-plus automaton):          " << thr << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "throughput-regex") {
        Throughput thr = 0;

        // Measure execution time
        startTimer(&timer);

        thr = throughputAnalysisGraphUsingMaxPlusAutomatonRegex(*fsmsadfGraph);
        out << "Throughput (max-plus automaton):          " << thr << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "latency-regex") {
        // Measure execution time
        startTimer(&timer);

        // Get the period
        if (this->analyzeSettings().period.empty()) {
            throw CException("No period specified.");
        }
        auto mu = MPTime(std::strtod(this->analyzeSettings().period.c_str(), nullptr));
        if (static_cast<CDouble>(mu) <= 0.0) {
            throw CException("Latency is computed relative to a positive period which is given as "
                             "an argument e.g. --period 0.43");
        }

        latencyAnalysisGraphUsingRegex(fsmsadfGraph.get(), mu, out);

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "buffersize-maxplus") {

        // Measure execution time
        // startTimer(&timer);

        BufferSizingScenarioUsingMaxPlus b = BufferSizingScenarioUsingMaxPlus(fsmsadfGraph.get());
        std::unique_ptr<StorageDistributionMPSet> minStorageDistributions = b.analyze(2);

        // Measure execution time
        // stopTimer(&timer);
        // out << "analysis time: ";
        // printTimer(out, &timer);
        // out << std::endl;
        // out << "max throughput = " << b.maxThroughput << std::endl;
        // out << "number of throughput calls = " <<
        // BufferSizingScenarioUsingMaxPlus::nrThroughputCalls << std::endl; output distributions
        out << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
        out << "<sdf3 type='sdf' version='1.0'" << std::endl;
        out << "     xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance'" << std::endl;
        out << "      "
               "xsi:noNamespaceSchemaLocation='https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>"
            << std::endl;
        out << "    <storageThroughputTradeOffs>" << std::endl;
        uint nrDistributions = 0;
        for (StorageDistributionMPSet *p = minStorageDistributions.get(); p != nullptr;
             p = p->next.get(), nrDistributions++) {
            out << "        <distributionsSet thr='";
            out << p->thr;
            out << "' sz='" << p->sz << "'>" << std::endl;
            for (StorageDistributionMP *d = p->distributions.get(); d != nullptr; d = d->next.get()) {
                out << "            <distribution>" << std::endl;
                for (const auto& b : b.fsmsadfBuffers.getBuffers()) {
                    out << "                <ch name='";
                    out << b->bufferChannel->getName();
                    out << "' sz='" << d->sp[b->id];
                    out << "'/>" << std::endl;
                }
                out << "            </distribution>" << std::endl;
            }
            out << "        </distributionsSet>" << std::endl;
        }
        out << "    </storageThroughputTradeOffs>" << std::endl;
        out << "</sdf3>" << std::endl;
    }

    else if (analyze.front().key == "criticalscenariosequence") {
        // Measure execution time
        startTimer(&timer);
        std::unique_ptr<ScenarioConstRefList> ls = criticalScenarioSequenceWeaklyConsistent(*fsmsadfGraph);

        std::cout << "Critical scenario sequence:" << std::endl;

        ScenarioConstRefList::const_iterator i = ls->begin();
        for (; i != ls->end();) {
            const auto *s = *i;
            std::cout << s->getName();
            if (++i != ls->end()) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;

        // Measure execution time
        stopTimer(&timer);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "trace") {
        TraceFormat f = Trace4CPS;
        CString &tfs = this->analyzeSettings().traceFormat;
        if (tfs.empty()) {
            // default to Trace4CPS format
            f = Trace4CPS;
        } else {
            if (tfs == "Trace4CPS") {
                f = Trace4CPS;
            } else if (tfs == "XML") {
                f = XML;
            } else {
                throw CException("Wrong trace format specification. "
                                 "Use --traceFormat with Trace4CPS or XML to specify the output format.");
            }
        }

        // Check for the validity of the entered filenames
        CStrings filenames = this->analyzeSettings().traceFiles.split(',');
        if (f == Trace4CPS) {
            if (filenames.size() != 2) {
                throw CException("Trace output files not specified or not valid. "
                                 "Use --traceFiles to specify two output files separated with a "
                                 "comma, e.g.: '--traceFiles a.txt,b.etf'");
            }
        }
        if (f == XML) {
            if (filenames.size() != 1) {
                throw CException("Trace output files not specified or not valid. "
                                 "Use --traceFiles to specify the xml output file, e.g.: "
                                 "'--traceFile trace.xml'");
            }
        }

        // Check for the validity of the entered scenario names
        if (this->analyzeSettings().seq.empty()) {
            throw CException("No scenario sequence specified. "
                             "Use --seq to specify a comma separated  sequence of scenarios, e.g.: "
                             "'--seq scen2,scen1'");
        }
        CStrings scenarioSequence = this->analyzeSettings().seq.split(',');
        CStrings uniqueScenarioSequence = scenarioSequence;
        uniqueScenarioSequence.unique();
        const Scenarios& scenarios = fsmsadfGraph->getScenarios();
        for (const auto &i : uniqueScenarioSequence) {
            bool sequencenamevalidity = false;
            for (const auto& sc : scenarios) {
                if (i == sc->getName()) {
                    sequencenamevalidity = true;
                }
            }
            if (!sequencenamevalidity) {
                throw CException("The entered scenario names do not match the ones in used in the "
                                 "xml file.\nSeparate the scenarios with a comma.");
            }
        }

        // check if the sequence follows the FSM
        if (!fsmsadfGraph->getFSM()->acceptsScenarioSequence(scenarioSequence)) {
            throw CException("The entered scenario sequence does not comply with the FSM");
        }

        // Measure execution time
        startTimer(&timer);

        traceGenerationWeaklyConsistent(*fsmsadfGraph, filenames, scenarioSequence, f);

        // Measure execution time
        stopTimer(&timer);
        std::cout << "Trace saved to '" << filenames.back() << "'" << std::endl;
        if (f == Trace4CPS) {
            std::cout << "Configuration saved to '" << filenames.front() << "'" << std::endl;
        }
        std::cout << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "vectortrace") {
        TraceFormat f = Trace4CPS;
        CString &tfs = this->analyzeSettings().traceFormat;
        if (tfs.empty()) {
            // default to Trace4CPS format
            f = Trace4CPS;
        } else {
            if (tfs == "Trace4CPS") {
                f = Trace4CPS;
            } else if (tfs == "XML") {
                f = XML;
            } else {
                throw CException("Wrong trace format specification. "
                                 "Use --traceFormat with Trace4CPS or XML to specify the output format.");
            }
        }

        // Check for the validity of the entered filenames
        CStrings filenames = this->analyzeSettings().traceFiles.split(',');
        if (f == Trace4CPS) {
            if (filenames.size() != 2) {
                throw CException("Trace output files not specified or not valid. "
                                 "Use --traceFiles to specify two output files separated with a "
                                 "comma, e.g.: '--traceFiles a.txt,b.etf'");
            }
        }
        if (f == XML) {
            if (filenames.size() != 1) {
                throw CException("Trace output files not specified or not valid. "
                                 "Use --traceFiles to specify the xml output file, e.g.: "
                                 "'--traceFile trace.xml'");
            }
        }

        // Check for the validity of the entered scenario names
        if (this->analyzeSettings().seq.empty()) {
            throw CException("No scenario sequence specified. "
                             "Use --seq to specify a comma separated  sequence of scenarios, e.g.: "
                             "'--seq scen2,scen1'");
        }
        CStrings scenarioSequence = this->analyzeSettings().seq.split(',');
        CStrings uniqueScenarioSequence = scenarioSequence;
        uniqueScenarioSequence.unique();
        const Scenarios& scenarios = fsmsadfGraph->getScenarios();
        for (const auto &i : uniqueScenarioSequence) {
            bool sequencenamevalidity = false;
            for (const auto& sc : scenarios) {
                if (i == sc->getName()) {
                    sequencenamevalidity = true;
                }
            }
            if (!sequencenamevalidity) {
                throw CException("The entered scenario names do not match the ones in used in the "
                                 "xml file.\nSeparate the scenarios with a comma.");
            }
        }

        // check if the sequence follows the FSM
        if (!fsmsadfGraph->getFSM()->acceptsScenarioSequence(scenarioSequence)) {
            throw CException("The entered scenario sequence does not comply with the FSM");
        }

        // Measure execution time
        startTimer(&timer);

        traceOfVectorsGenerationWeaklyConsistent(*fsmsadfGraph, filenames, scenarioSequence, f);

        // Measure execution time
        stopTimer(&timer);
        std::cout << "Trace saved to '" << filenames.back() << "'" << std::endl;
        if (f == Trace4CPS) {
            std::cout << "Configuration saved to '" << filenames.front() << "'" << std::endl;
        }
        std::cout << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else if (analyze.front().key == "schedule-check") {
        // Check for the validity of the entered scenario names
        if (this->analyzeSettings().seq.empty()) {
            throw CException("No scenario sequence specified. Use --seq to specify comma separated "
                             "scenario sequence.");
        }
        CStrings scenarioSequence = this->analyzeSettings().seq.split(',');
        CStrings uniqueScenarioSequence = scenarioSequence;
        uniqueScenarioSequence.unique();
        const Scenarios& scenarios = fsmsadfGraph->getScenarios();

        for (const auto &i : uniqueScenarioSequence) {
            bool sequencenamevalidity = false;
            for (const auto& j : scenarios) {
                if (i == j->getName()) {
                    sequencenamevalidity = true;
                }
            }
            if (!sequencenamevalidity) {
                throw CException("The entered scenario names do not match the ones in used in the "
                                 "xml file. Separate the scenarios with a comma.");
            }
        }
        // Measure execution time
        startTimer(&timer);

        scheduleCheckUsingStateSpace(
                *fsmsadfGraph, scenarioSequence, this->xmlArchGraph, this->xmlMappingGraph);

        // scheduleCheckUsingStateSpace throws a general exception if the check fails
        // if we get here, the check passed
        std::cout << "The schedule check passed.\n";


        // Measure execution time
        stopTimer(&timer);
        std::cout << "analysis time: ";
        printTimer(out, &timer);
        out << std::endl;
    } else {
        throw CException("Unknown algorithm.");
    }
}

void ToolAnalyze::loadGraphs() {
    FSMSADF::ToolAnalyze::loadGraphsFromFile(this->analyzeSettings().graphFile,
                             MODULE,
                             &(this->xmlAppGraph),
                             &(this->xmlArchGraph),
                             &(this->xmlMappingGraph),
                             &(this->xmlAppGraphDoc));
}

} // namespace FSMSADF

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    FSMSADF::ToolAnalyze T;

    int exit_status = 0;
    std::ofstream out;

    try {
        // Initialize the program
        T.initSettings(MODULE, argc, argv);

        // Load application graph
        T.loadGraphs();

        // Set output stream
        if (!T.settings().outputFile.empty()) {
            out.open(T.settings().outputFile.c_str());
        } else {
            (static_cast<std::ostream &>(out)).rdbuf(std::cout.rdbuf());
        }

        // Perform the actual function
        T.analyzeGraph(out);
        T.cleanUp();
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}