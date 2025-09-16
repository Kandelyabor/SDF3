/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3flow.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   sdf3flow
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *
 * $Id: sdf3flow.cc,v 1.1.2.3 2010-08-01 17:26:43 mgeilen Exp $
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

#include "base/exception/exception.h"
#include "base/log/log.h"
#include "base/time/time.h"
#include "base/xml/xml.h"
#include "fsmsadf/fsmsadf.h"
#include "fsmsadf/resource_allocation/flow/base_platform/base/flow.h"
#include "fsmsadf/resource_allocation/flow/compsoc_platform/base/flow.h"
#include "fsmsadf/resource_allocation/flow/mamps_platform/base/flow.h"
#include "fsmsadf/resource_allocation/flow/virtual_platform/base/flow.h"
#include "sdf3flow.h"
#include "settings.h"
#include <fstream>
#include <iostream>
#include <memory>

using namespace FSMSADF;

namespace FSMSADF {

namespace {

/**
 * settings
 * Program settings.
 */
FlowSettings settings(MODULE, SETTINGS_TYPE); // NOLINT(*global-variables,cert-err58-cpp)

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(std::ostream &out) {
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << std::endl;
    out << std::endl;
    out << "Usage: " << TOOL << " [--settings <file> --output <file>";
    out << " --step]";
    out << std::endl;
    out << "   --settings <file>  settings for algorithms (default: sdf3.opt)";
    out << std::endl;
    out << "   --output <file>    output file (default:stdout)" << std::endl;
    out << "   --html             output mapping in HTML format" << std::endl;
    out << "   --step             run flow step-by-step" << std::endl;
    out << std::endl;
}


/**
 * initSettings ()
 * The function initializes the program settings.
 */
bool initSettings(int argc, char **argv) {

    // Initialize settings
    settings.init(argc, argv);

    // Help message needed?
    if (settings.helpFlag) {
        helpMessage(std::cerr);
        return false;
    }

    // Initialize output stream
    settings.initOutputStream();

    // Done
    return true;
}

/**
 * mapApplicationGraphToArchitectureGraph ()
 * The complete mapping flow. The function returns a pointer to an sdf3node
 * when a mapping has been found. Otherwise the function returns a nullptr pointer.
 */
CNode *mapApplicationGraphToArchitectureGraph(CNode *sdf3Node, bool outputHTML) {
    SDF3FlowBase::FlowState status = SDF3FlowBase::FlowState::FlowStart;
    CNode *newSDF3Node = nullptr;
    std::unique_ptr<SDF3FlowBase> flow = nullptr;
    CTimer timer;

    // Create a new mapping flow
    if (settings.platformType == PlatformTypeVirtual) {
        flow = std::make_unique<SDF3FlowVirtualPlatform>();
    } else if (settings.platformType == PlatformTypeCompSoC) {
        flow = std::make_unique<SDF3FlowCompSoCPlatform>();
    } else if (settings.platformType == PlatformTypeMAMPS) {
        flow = std::make_unique<SDF3FlowMampsPlatform>();
    } else {
        throw CException("Unsupported platform type.");
    }

    // Run flow step-by-step?
    if (settings.stepFlag) {
        flow->setStepMode(true);
    }

    // Create flow from sdf3 node
    flow->constructFromXML(sdf3Node);

    // Set maximum number of platform bindings created in tile binding algorithm
    flow->setMaxNrBindingsTileBindingAlgo(settings.maxNrBindingsTileBindingAlgo);

    // Measure execution time
    startTimer(&timer);

    // Execute the flow
    status = flow->run();

    // Measure execution time
    stopTimer(&timer);

    std::cerr << "Execution time: ";
    printTimer(std::cerr, &timer);
    std::cerr << std::endl;

    // Create a new SDF3 node
    newSDF3Node = CNewNode("sdf3");
    flow->convertToXML(newSDF3Node);

    // Output mapping in HTML?
    if (outputHTML) {
        flow->convertToHTML("html");
    }

    // Flow failed?
    if (status != SDF3FlowBase::FlowState::FlowCompleted) {
        logError("Failed to complete flow.");
        return nullptr;
    }

    // Done
    return newSDF3Node;
}

/**
 * mappingFlow()
 * Perform the mapping flow on all application graphs.
 */
bool mappingFlow(std::ostream &out) { // NOLINT(*cognitive-complexity)
    CNode *archGraph = nullptr;
    CNode *resourceUsage = nullptr;
    CNode *appGraph = nullptr;
    CNode *mapping = nullptr;
    CNode *archSdfNode = nullptr;
    CNode *appSdfNode = nullptr;
    CNode *sdf3node = nullptr;
    std::unique_ptr<CDoc> archDoc = nullptr;
    std::unique_ptr<CDoc> appDoc = nullptr;
    std::unique_ptr<CDoc> sdf3doc = nullptr;
    std::ofstream outFile;

    // Open architecture file
    archDoc = CParseFile(settings.architectureGraph);
    if (archDoc == nullptr || archDoc->getXmlDoc()==nullptr) {
        throw CException("Failed opening '" + settings.architectureGraph + "'.");
    }

    // Locate the sdf3 root element and check module type
    archSdfNode = archDoc->getRootNode();
    if (CGetAttribute(archSdfNode, "type") != settings.module) {
        throw CException("Root element in file '" + settings.architectureGraph
                         + "' is not of type '" + settings.module + "'.");
    }

    // Architecture graph
    if (!CHasChildNode(archSdfNode, "architectureGraph")) {
        throw CException("No architecture graph found.");
    }
    archGraph = CGetChildNode(archSdfNode, "architectureGraph");

    // Resource usage
    resourceUsage = nullptr;

    // Iterate over the application graphs
    for (const auto &file : settings.applicationGraphs) {

        // Open application file
        appDoc = CParseFile(file);
        if (appDoc == nullptr || appDoc->getXmlDoc()==nullptr) {
            throw CException("Failed opening '" + file + "'.");
        }

        // Locate the sdf3 root element and check module type
        appSdfNode = appDoc->getRootNode();
        if (CGetAttribute(appSdfNode, "type") != settings.module) {
            throw CException("Root element in file '" + file + "' is not of type '"
                             + settings.module + "'.");
        }

        // Application graph
        if (!CHasChildNode(appSdfNode, "applicationGraph")) {
            throw CException("No application graph found.");
        }
        appGraph = CGetChildNode(appSdfNode, "applicationGraph");

        // Mapping related to this application?
        mapping = nullptr;
        for (CNode *n = CGetChildNode(appSdfNode, "mapping"); n != nullptr;
             n = CNextNode(n, "mapping")) {
            // Is this the initial mapping?
            if (CHasAttribute(n, "name") && CGetAttribute(n, "name") == "initial") {
                // Does the mapping specify an application graph?
                if (!CHasAttribute(n, "appGraph")) {
                    throw CException("No application graph specified "
                                     "with the default mapping.");
                }

                // Is the application graph of this mapping the application
                // graph which is going to be mapped
                if (CGetAttribute(n, "appGraph") == CGetAttribute(appGraph, "name")) {
                    mapping = n;
                    break;
                }
            }
        }
        // No mapping specified? Create an empty mapping
        if (mapping == nullptr) {
            mapping = CNewNode("mapping");
            CAddAttribute(mapping, "name", "initial");
        }

        // Resource usage specified?
        if (resourceUsage != nullptr) {
            // Does the mapping already specify some resource usage?
            if (CHasChildNode(mapping, "resourceUsage")) {
                // Remove the existing resource usage
                CRemoveNode(CGetChildNode(mapping, "resourceUsage"));
                std::cerr << "[Warning] ignoring resource usage of application";
                std::cerr << " mapping" << std::endl;
            }

            // Add resource usage to the mapping
            CAddNode(mapping, resourceUsage);
        }

        // Create a new SDF3 node
        sdf3node = CNewNode("sdf3");

        // Add elements to the SDF3 node
        CAddNode(sdf3node, CCopyNode(appGraph));
        CAddNode(sdf3node, CCopyNode(archGraph));
        CAddNode(sdf3node, CCopyNode(mapping));

        // Map the application to the architecture
        sdf3node = mapApplicationGraphToArchitectureGraph(sdf3node, settings.outputHTML);

        // Failed to find a mapping?
        if (sdf3node == nullptr) {
            return false;
        }

        // Update the resource usage
        resourceUsage = CGetChildNode(CGetChildNode(sdf3node, "mapping"), "resourceUsage");

        // Output every mapping to a different file
        if (!settings.outputFile.empty() && settings.applicationGraphs.size() != 1) {
            outFile.open(file + "_" + settings.outputFile);
            out.rdbuf(outFile.rdbuf());
        }

        // Output the result of the flow
        sdf3doc = CNewDoc(sdf3node);
        if (!sdf3doc->validate()) {
            logWarning("Failed to validate the output file.");
        }
        sdf3doc->saveFile(out, 1);
    }

    return true;
}

} // namespace


// End namespace FSMSADF
} // namespace FSMSADF

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    int exit_status = 0;
    try {
        try {
            // Initialize the program
            if (!initSettings(argc, argv)) {
                return 1;
            }

            // Run mapping flow
            if (mappingFlow(settings.outputStream)) {
                exit_status = 0;
            } else {
                exit_status = 1;
            }
        } catch (CException &e) {
            std::cerr << e;
            exit_status = 1;
        }
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
        exit_status = 1;
    }
    return exit_status;
}
