/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 21, 2009
 *
 *  Function        :   FSM-based SADF transformation functionality
 *
 *  History         :
 *      21-07-09    :   Initial version.
 *
 * $Id: sdf3transform.cc,v 1.1.2.3 2010-05-11 03:27:08 mgeilen Exp $
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
#include "fsmsadf/fsmsadf.h" // IWYU pragma: keep
#include <memory>

using namespace FSMSADF;
using namespace SDF;

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void ToolTransform::initSettings(const CString & /*modulename*/, int argc, char **argv) {
    this->setSettings(std::make_unique<TransformSettings>());

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
        throw CException("Required settings not found.");
    }
}

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void ToolTransform::helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << "Usage: " << TOOL << " [--settings <file>] [--graph <file> --transform <list>]"
        << std::endl;
    out << " [--scenario <name>] [--output <file>]" << std::endl;
    out << "   --settings  <file>  settings for the graph generator (default: sdf3.opt)"
        << std::endl;
    out << "   --graph  <file>     input SDF graph" << std::endl;
    out << "   --transform <list>  comma separated list with graph";
    out << "    transformations:" << std::endl;
    out << "       wc_sdfg" << std::endl;
    out << "       model_autoconc(<max>)" << std::endl;
    out << "       to_sdfg" << std::endl;
    out << "       create_detector" << std::endl;
    out << "       to_binding_aware" << std::endl;
    out << "       merge_scenarios" << std::endl;
    out << "   --scenario <name>   select a scenario" << std::endl;
    out << "   --output <file>     output file (default: stdout)" << std::endl;
}

bool ToolTransform::parseCommand(int argc, char **argv, int *arg) {
    if (argv[*arg] == CString("--settings") && (*arg) + 1 < argc) {
        (*arg)++;
        this->transformSettings().settingsFile = argv[*arg];
    } else if (argv[*arg] == CString("--graph") && (*arg) + 1 < argc) {
        (*arg)++;
        this->transformSettings().graphFile = argv[*arg];
    } else if (argv[*arg] == CString("--output") && (*arg) + 1 < argc) {
        (*arg)++;
        this->transformSettings().outputFile = argv[*arg];
    } else if (argv[*arg] == CString("--scenario") && (*arg) + 1 < argc) {
        (*arg)++;
        this->transformSettings().scenario = argv[*arg];
    } else if (argv[*arg] == CString("--transform") && (*arg) + 1 < argc) {
        (*arg)++;
        this->transformSettings().arguments = this->parseSwitchArgument(argv[*arg]);
    } else {
        return false;
    }
    return true;
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void ToolTransform::setDefaults() { this->transformSettings().settingsFile = "sdf3.opt"; }

/**
 * parseSettingsFile ()
 * The function parses all settings from the file.
 */
void ToolTransform::parseSettingsFile(const CString &module, const CString &type) {
    CNode *sdf3Node = nullptr;
    CNode *settingsNode = nullptr;
    CString name;
    CString file;

    // Open settings file and get root node
    std::unique_ptr<CDoc> settingsDoc = CParseFile(this->transformSettings().settingsFile);
    sdf3Node = settingsDoc->getRootNode();
    if (sdf3Node == nullptr) {
        // turn off unnecessary notification
        // logInfo("Failed opening '" + this->transformSettings().settingsFile
        //         + "', fallback on command-line settings.");
        return;
    }

    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + this->transformSettings().settingsFile
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
        logInfo("File '" + this->transformSettings().settingsFile
                + "' contains no "
                  " settings of type '"
                + type + "', fallback on command-line settings .");
        return;
    }

    // Get the graph file
    if (!CHasChildNode(settingsNode, "graph")) {
        throw CException("File '" + this->transformSettings().settingsFile + "' contains no graphFile.");
    }
    CNode *graphNode = CGetChildNode(settingsNode, "graph");
    this->transformSettings().graphFile = CGetAttribute(graphNode, "file");

    // Get the transformation(s)
    if (!CHasChildNode(settingsNode, "transform")) {
        throw CException("File '" + this->transformSettings().settingsFile + "' contains no transforms.");
    }

    CNode *transformNode = CGetChildNode(settingsNode, "transform");
    if (!CHasChildNode(transformNode, "type")) {
        throw CException("No transforms specified.");
    }

    CPair p;
    p.key = "";
    p.value = "";
    for (CNode *entryNode = CGetChildNode(transformNode, "type"); entryNode != nullptr;
         entryNode = CNextNode(entryNode, "type")) {
        if (!CHasAttribute(entryNode, "name")) {
            throw CException("Transform entry has no transform name.");
        }
        p.key = CGetAttribute(entryNode, "name");
        this->transformSettings().arguments.push_back(p);
    }

    // Get the scenario
    if (CHasChildNode(settingsNode, "scenario")) {
        CNode *outputNode = CGetChildNode(settingsNode, "scenario");
        this->transformSettings().scenario = CGetAttribute(outputNode, "name");
    }

    // Get the output file
    if (CHasChildNode(settingsNode, "output")) {
        CNode *outputNode = CGetChildNode(settingsNode, "output");
        this->transformSettings().outputFile = CGetAttribute(outputNode, "file");
    }
}

bool ToolTransform::checkRequiredSettings() {
    if (this->transformSettings().graphFile.empty()) {
        return false;
    }
    if (this->transformSettings().arguments.empty()) {
        return false;
    };

    return true;
}

/**
 * postInitSettings ()
 *
 */
void ToolTransform::postInitSettings() {
    // Parse settings
    this->parseSettingsFile(MODULE, SETTINGS_TYPE);
}

/**
 * loadGraphsFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
void ToolTransform::loadGraphsFromFile(const CString &file,
                                       const CString &module,
                                       CNode **appGraphNode,
                                       CNode **archGraphNode,
                                       std::list<CNode *> *mappingsNodes,
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

    // Get mapping graph nodes
    for (CNode *mappingNode = CGetChildNode(sdf3Node, "mapping"); mappingNode != nullptr;
         mappingNode = CNextNode(mappingNode, "mapping")) {
        mappingsNodes->push_back(mappingNode);
    }
}

void ToolTransform::cleanUp() {
    Tool::cleanUp();
    // cleanup the xml document
    if (this->xmlAppGraphDoc != nullptr) {
        this->xmlAppGraphDoc.reset();
    }
    // cleanup other xml resources
    xmlCleanup();
}

void ToolTransform::loadGraphs() {
    FSMSADF::ToolTransform::loadGraphsFromFile(this->transformSettings().graphFile,
                             MODULE,
                             &(this->xmlAppGraph),
                             &(this->xmlArchGraph),
                             &(this->xmlMappings),
                             &(this->xmlAppGraphDoc));
}

/**
 * transformGraph ()
 * The function transforms the SDF graph and the resulting (H)SDF graph
 * is outputted in XML format to the out stream.
 */
std::unique_ptr<Graph> ToolTransform::transformGraph(Graph *ag,
                                     FSMSADF::PlatformGraph *pg,
                                     PlatformBindings *pbs,
                                     CPairs &transforms,
                                     std::ostream &out) {
    std::unique_ptr<Graph> h_in = ag->clone(GraphComponent(ag->getParent(), ag->getId(), ag->getName()));
    std::unique_ptr<Graph> h_out = nullptr;

    for (auto iter = transforms.begin(); h_in != nullptr && iter != transforms.end(); iter++) {
        CPair &transform = *iter;
        std::unique_ptr<TimedSDFgraph> sdfg = nullptr;

        // Perform transformation
        if (transform.key == "wc_sdfg") {
            h_out = transformToWorstCaseGraph(*h_in);
        } else if (transform.key == "to_sdfg") {
            sdfg = FSMSADF::Graph::extractSDFgraph(*(h_in->getScenarioRef(this->transformSettings().scenario)));
            outputSDFasXML(*sdfg, out);
            h_out = nullptr;
        } else if (transform.key == "model_autoconc") {
            if (transform.value.empty()) {
                throw CException("Missing maximum degree of auto-concurrency.");
            }

            h_out = modelAutoConcurrencyInGraph(*h_in, static_cast<int>(transform.value));
        } else if (transform.key == "create_detector") {
            h_out = h_in->clone(*h_in);
            createDetectorScenario(
                    h_out->getFSM(), h_out->getScenarios(), h_out->getScenarioGraphs());
        } else if (transform.key == "to_binding_aware") {
            h_out = toBindingAware(h_in.get(), pg, pbs);
        } else if (transform.key == "merge_scenarios") {
            if (pg == nullptr) {
                throw CException("No architecture graph specified for merging scenarios.");
            }
            h_out = mergeAllScenarios(pg, pbs);
        }

        else {
            throw CException("Unknown transformation.");
        }

        // Output graph becomes next input
        h_in = std::move(h_out);
    }

    return h_in;
}

/**
 * transformGraph ()
 * The function transforms the graph.
 */
void ToolTransform::transformGraph(std::ostream &out) {
    std::unique_ptr<Graph> ag = nullptr;
    std::unique_ptr<Graph> gTransformed = nullptr;
    CNode *applicationNode = nullptr;
    CNode *architectureNode = nullptr;
    std::list<CNode *> mappingNodes;

    // Find sdf graph in XML structure
    applicationNode = this->xmlAppGraph;

    // Construction graph model
    ag = std::make_unique<Graph>();
    ag->constructFromXML(applicationNode);

    architectureNode = this->xmlArchGraph;
    mappingNodes = xmlMappings;

    std::unique_ptr<PlatformGraph> pg = nullptr;
    auto pbs = std::make_unique<PlatformBindings>();

    if ((architectureNode != nullptr) && (!xmlMappings.empty())) {
        pg = std::make_unique<PlatformGraph>(GraphComponent(nullptr, 0));
        pg->constructFromXML(architectureNode);

        for (auto *xmlMapping : mappingNodes) {
            // Create a new platform binding
            auto pb = std::make_unique<PlatformBinding>(GraphComponent(nullptr, 0),
                                           pg.get(),
                                           ag.get(),
                                           BindingMethod(BindingMethod::Method::MaxOfScenarios));

            // Load mapping
            pb->constructFromXML(xmlMapping);

            // Add to platform bindings
            pbs->push_back(std::move(pb));
        }
    }

    // The actual transformation...
    gTransformed = transformGraph(ag.get(), pg.get(), pbs.get(), this->transformSettings().arguments, out);

    // Graph returned
    if (gTransformed != nullptr) {
        // Create XML
        CNode *sdf3Node = CNewNode("sdf3");
        CAddAttribute(sdf3Node, "version", "1.0");
        CAddAttribute(sdf3Node, "type", "fsmsadf");
        CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
        CAddAttribute(sdf3Node, "xmlns", "uri:sdf3");
        CAddAttribute(sdf3Node,
                      "xsi:noNamespaceSchemaLocation",
                      "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

        // Application graph node
        CNode *appNode = CAddNode(sdf3Node, "applicationGraph");

        // SDF node
        gTransformed->convertToXML(appNode);

        if (architectureNode != nullptr) {
            pg->convertToXML(CAddNode(sdf3Node, "architectureGraph"));
        }

        for (const auto& pb : *pbs) {
            CNode *mappingNode = CAddNode(sdf3Node, "mapping");
            pb->convertToXML(mappingNode);
        }

        // Create document and save it
        std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
        doc->saveFile(out, 1);

    }
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    FSMSADF::ToolTransform T;

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

        // Perform requested actions
        T.transformGraph(out);
        T.cleanUp();
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
