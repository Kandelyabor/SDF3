/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3flow.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *
 * $Id: sdf3flow.cc,v 1.5 2008-05-07 11:29:38 sander Exp $
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

#include "sdf3flow.h"
#include "sdf/sdf.h" // IWYU pragma: export - indirectly includes
#include "settings.h"
#include <memory>

using namespace SDF;
namespace SDF {

namespace {


/**
 * settings
 * Program settings.
 */
Settings settings(MODULE, SETTINGS_TYPE); // NOLINT(*const-global-variables,cert-err58-cpp)

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
    out << "   --step             run flow step-by-step" << std::endl;
    out << "   --html             output result of flow in HTML" << std::endl;
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
 * The complete mapping flow. The function returns the system usage
 * after the mapping is completed. On failure, it returns nullptr.
 */
CNode *mapApplicationGraphToArchitectureGraph(CNode *xmlAppGraph,
                                              CNode *xmlArchGraph,
                                              CNode *xmlSystemUsage,
                                              std::ostream &out,
                                              bool outputAsHTML) {

    // Create a new mapping flow
    auto flow = std::make_unique<SDF3Flow>(settings.flowType, xmlAppGraph, xmlArchGraph, xmlSystemUsage);

    // Run flow step-by-step?
    if (settings.stepFlag) {
        flow->setStepMode(true);
    }

    // Tile binding and scheduling algorithm
    std::unique_ptr<TileMapping> tileMapping = nullptr;
    if (settings.tileMappingAlgo == "loadbalance") {
        tileMapping = std::make_unique<LoadBalanceBinding>(flow->getFlowType());
        (dynamic_cast<LoadBalanceBinding*>(tileMapping.get()))
                ->setConstantsTileCostFunction(settings.cnst_a,
                                               settings.cnst_b,
                                               settings.cnst_c,
                                               settings.cnst_d,
                                               settings.cnst_e,
                                               settings.cnst_f,
                                               settings.cnst_g,
                                               settings.cnst_k,
                                               settings.cnst_l,
                                               settings.cnst_m,
                                               settings.cnst_n,
                                               settings.cnst_o,
                                               settings.cnst_p,
                                               settings.cnst_q);
    } else {
        throw CException("[ERROR] Unknown tile mapping algorithm.");
    }
    flow->setTileMappingAlgo(std::move(tileMapping));

    // NoC routing and scheduling algorithm
    std::unique_ptr<NoCScheduler> nocMappingAlgo = nullptr;
    if (settings.nocMappingAlgo == "greedy") {
        nocMappingAlgo = std::make_unique<GreedyNoCScheduler>(settings.maxDetour);
    } else if (settings.nocMappingAlgo == "ripup") {
        nocMappingAlgo = std::make_unique<RipupNoCScheduler>(settings.maxDetour, settings.maxNrRipups);
    } else if (settings.nocMappingAlgo == "knowledge") {
        nocMappingAlgo = std::make_unique<KnowledgeNoCScheduler>(settings.maxDetour, settings.maxNrRipups);
    } else if (settings.nocMappingAlgo == "random") {
        nocMappingAlgo = std::make_unique<RandomNoCScheduler>(
                settings.maxDetour, settings.maxNrRipups, settings.maxNrTries);
    } else if (settings.nocMappingAlgo == "classic") {
        nocMappingAlgo = std::make_unique<ClassicNoCScheduler>(settings.maxDetour, settings.maxNrRipups);
    } else {
        throw CException("[ERROR] Unknown scheduling algorithm.");
    }
    auto nocMapping = std::make_unique<NoCMapping>(nocMappingAlgo.get(), flow->getFlowType());
    flow->setNoCMappingAlgo(std::move(nocMapping));

    // Measure execution time
    CTimer timer;
    startTimer(&timer);

    // Execute the flow
    SDF3Flow::FlowState status = flow->run();

    // Measure execution time
    stopTimer(&timer);

    std::cerr << "Execution time: ";
    printTimer(std::cerr, &timer);
    std::cerr << std::endl;

    // Output the result of the flow
    flow->outputMappingAsXML(out);

    // Clear the output buffer
    out.flush();

    // Flow failed?
    if (status != SDF3Flow::FlowState::FlowCompleted) {
        logError("Failed to complete flow.");
        return nullptr;
    }

    // Output result of flow as HTML?
    if (outputAsHTML) {
        flow->outputMappingAsHTML();
    }

    // Retrieve system usage in XML format
    CNode *xmlSDF3Node = flow->createSDF3Node();

    // Done
    return CGetChildNode(xmlSDF3Node, "systemUsage");
}


/**
 * mapApplicationGraphsToArchitectureGraph ()
 * Map all application graphs in the order specified by the settings to
 * the architecture graph.
 */
bool mapApplicationGraphsToArchitectureGraph(std::ostream &out) {
    CNode *xmlSystemUsage = settings.xmlSystemUsage;

    for (auto *xmlAppGraph : settings.xmlAppGraphs) {
        CString appName;
        std::ofstream outFile;

        // Output every mapping to a different file
        if (!settings.outputFile.empty() && settings.xmlAppGraphs.size() != 1) {
            appName = CGetAttribute(xmlAppGraph, "name");
            outFile.open(appName + "_" + settings.outputFile);
            out.rdbuf(outFile.rdbuf());
        }

        // Map application to architecture
        xmlSystemUsage = mapApplicationGraphToArchitectureGraph(
                xmlAppGraph, settings.xmlArchGraph, xmlSystemUsage, out, settings.outputAsHTML);

        // Flow failed?
        if (xmlSystemUsage == nullptr) {
            return false;
        }
    }

    return true;
}

} // anonymous namespace


} // namespace SDF
/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) {
    int exit_status = 0;

    try {
        // Initialize the program
        if (!initSettings(argc, argv)) {
            return 1;
        }

        // Run mapping flow
        if (mapApplicationGraphsToArchitectureGraph(settings.outputStream)) {
            exit_status = 0;
        } else {
            exit_status = 1;
        }
    } catch (CException &e) {
        std::cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
