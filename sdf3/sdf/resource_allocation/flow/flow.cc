/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.cc
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
 * $Id: flow.cc,v 1.4 2008-03-20 16:16:18 sander Exp $
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

#include "flow.h"
#include "sdf/output/html/html.h"
#include "sdf/output/xml/xml.h"
#include "sdf/resource_allocation/mpsoc_arch/xml.h"
#include <memory>

namespace SDF {
/**
 * SDF3Flow ()
 * Constructor.
 */
SDF3Flow::SDF3Flow(SDFflowType type,
                   CNode *xmlAppGraph,
                   CNode *xmlArchGraph,
                   CNode *xmlSystemUsage) :
    flowType(type),
    // Run flow in one go
    stepMode(false),
    xmlAppGraph(xmlAppGraph),
    xmlArchGraph(xmlArchGraph),
    xmlSystemUsage(xmlSystemUsage),
    // Application graph
    appGraph(createAppGraph(xmlAppGraph)),
    // Platform graph
    platformGraph(createPlatformGraph(xmlArchGraph)),
    // Tile binding and scheduling algorithm
    tileMapping(nullptr),
    // NoC routing and scheduling algorithm
    nocMapping(nullptr),
    // Design flow is in its initial state
    stateOfFlow(FlowState::FlowStart),
    // No storage space distributions found
    minStorageDistributions(nullptr),
    selectedStorageDistributionSet(nullptr),
    selectedStorageDistribution(nullptr) {

    setSystemUsagePlatformGraph(platformGraph.get(), xmlSystemUsage);
}

/**
 * ~SDF3Flow ()
 * Destructor.
 */
SDF3Flow::~SDF3Flow() = default;

/**
 * createAppGraph ()
 * The function creates an timed SDFG according to the XML specification.
 */
std::unique_ptr<TimedSDFgraph> SDF3Flow::createAppGraph(CNode *xmlAppGraph) {

    // Select nodes in the XML structure
    CNode *sdfNode = CGetChildNode(xmlAppGraph, "sdf");
    CNode *sdfPropertiesNode = CGetChildNode(xmlAppGraph, "sdfProperties");

    // Construct the graph
    auto g = std::make_unique<TimedSDFgraph>();
    g->construct(sdfNode, sdfPropertiesNode);

    return g;
}

/**
 * createPlatformGraph ()
 * The function creates a platform graph according to the XML specification.
 */
std::unique_ptr<PlatformGraph> SDF3Flow::createPlatformGraph(CNode *xmlPlatformGraph) {
    return constructPlatformGraph(xmlPlatformGraph);
}

/**
 * setSystemUsagePlatformGraph ()
 * The function sets the usage of the platform graph as specified in the
 * XML specs.
 */
void SDF3Flow::setSystemUsagePlatformGraph(PlatformGraph* g, CNode *systemUsage) {
    setUsagePlatformGraph(g, systemUsage);
}

/**
 * checkInputDesignFlow ()
 * The function checks that all required inputs are present and it initializes
 * the used data-structures.
 */
void SDF3Flow::checkInputDesignFlow() {
    // Output current state of the flow
    logInfo("Check input design flow.");

    if (tileMapping == nullptr) {
        throw CException("[ERROR] No tile mapping algorithm specified.");
    }

    if (nocMapping == nullptr) {
        throw CException("[ERROR] No NoC mapping algorithm specified.");
    }

    // Link application and platform to tile mapping algorithm
    tileMapping->setAppGraph(getAppGraph());
    tileMapping->setArchGraph(getPlatformGraph());

    // Link application, platform, interconnect and usage to NoC mapping algo
    nocMapping->init(getAppGraph(), getPlatformGraph(), xmlArchGraph, xmlSystemUsage);

    // Advance to next state in the flow
    setNextStateOfFlow(FlowState::FlowModelNonLocalMemory);
}

/**
 * run ()
 * Execute the design flow. The function returns the last state reached by
 * the design flow. Its value is equal to 'FlowCompleted' when all phases
 * have been executed successfully or else it is 'FlowFailed' which indicates
 * that a step of the flow could not be completed successfully.
 */
SDF3Flow::FlowState SDF3Flow::run() {
    CTimer timer;

    do {
        // Measure execution time
        startTimer(&timer);

        switch (getStateOfFlow()) {
        case FlowState::FlowStart:
            checkInputDesignFlow();
            break;

        case FlowState::FlowModelNonLocalMemory:
            modelNonLocalMemoryAccesses();
            break;

        case FlowState::FlowComputeStorageDist:
            computeStorageDistributions();
            break;

        case FlowState::FlowSelectStorageDist:
            selectStorageDistribution();
            break;

        case FlowState::FlowEstimateStorageDist:
            estimateStorageConstraints();
            break;

        case FlowState::FlowEstimateLatencyConstraint:
            estimateLatencyConstraints();
            break;

        case FlowState::FlowEstimateBandwidthConstraint:
            estimateBandwidthConstraints();
            break;

        case FlowState::FlowBindSDFGtoTile:
            bindSDFGtoTiles();
            break;

        case FlowState::FlowStaticOrderScheduleTiles:
            constructStaticOrderScheduleTiles();
            break;

        case FlowState::FlowAllocateTDMAtimeSlices:
            allocateTDMAtimeSlices();
            break;

        case FlowState::FlowOptimizeStorageSpaceAllocations:
            optimizeStorageSpaceAllocations();
            break;

        case FlowState::FlowExtractCommunicationConstraints:
            extractCommunicationConstraints();
            break;

        case FlowState::FlowScheduleCommunication:
            scheduleCommunication();
            break;

        case FlowState::FlowUpdateBandwidthAllocations:
            updateBandwidthAllocations();
            break;

        default:
            break;
        }

        // Measure execution time
        stopTimer(&timer);

        std::cout << "[INFO   ] Step took: ";
        printTimer(std::cout, &timer);
        std::cout << std::endl;

        if (getStepMode()) {
            handleUserInteraction();
        }

    } while (getStateOfFlow() != FlowState::FlowCompleted
             && getStateOfFlow() != FlowState::FlowFailed);

    return getStateOfFlow();
}

/**
 * handleUserInteraction ()
 * The function request the user for the next action to perform. Possible
 * actions include continuing with the next step of the flow, completing the
 * remaining flow or printing the current result of the flow to the terminal.
 */
void SDF3Flow::handleUserInteraction() {
    bool done = false;
    do {
        // Print message
        std::cout << "Command: (n)ext step / (c)ontinue flow / print (s)tate / print (h)tml? ";

        // First character determines command
        char cmd = static_cast<char>(std::cin.get());

        // Command is end-of-line (default action: step)
        if (cmd == '\n') {
            cmd = 'n';
        } else {
            // Remove remaining characters on current line from the input
            char c = 0;
            do {
                c = static_cast<char>(std::cin.get());
            } while (c != '\n');
        }

        switch (cmd) {
        case 'n':
            done = true;
            break;

        case 'c':
            setStepMode(false);
            done = true;
            break;

        case 's':
            outputMappingAsXML(std::cout);
            break;

        case 'h':
            outputMappingAsHTML();
            break;

        default:
            break;
        }
    } while (!done);
}

/**
 * createStorageDistributionsNode ()
 * The function returns an XML node that describes the storage-space throughput
 * trade-off points.
 */
CNode *SDF3Flow::createStorageDistributionsNode(const StorageDistributionSet& distributions) {
    const StorageDistributionSet* set = &distributions;
    StorageDistribution* d = nullptr;

    // Create new node for trade-off space
    CNode *storageNode = CNewNode("storageThroughputTradeOffs");

    while (set != nullptr) {
        // Create node for all distributions with same size
        CNode *distributionsNode = CAddNode(storageNode, "distributionsSet");
        CAddAttribute(distributionsNode, "sz", CString(set->sz));
        CAddAttribute(distributionsNode, "thr", CString(set->thr));

        // Select first distribution in set
        d = set->distributions.get();

        while (d != nullptr) {
            // Create node for specific distribution
            CNode *dNode = CAddNode(distributionsNode, "distribution");

            for (uint i = 0; i < appGraph->nrChannels(); i++) {
                CNode *chNode = CAddNode(dNode, "ch");
                CAddAttribute(chNode, "name", appGraph->getChannel(i)->getName());
                CAddAttribute(chNode, "sz", CString(d->sp[i]));
            }

            // Next distribution
            d = d->next.get();
        }

        // Next distribution set
        set = set->next.get();
    }

    return storageNode;
}

/**
 * createSDF3Node ()
 * The function returns an sdf3 node that describes the current status of
 * the mapping flow.
 */
CNode *SDF3Flow::createSDF3Node() {
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node,
                  "xsi:noNamespaceSchemaLocation",
                  "https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // Application graph node
    CAddNode(sdf3Node, createApplicationGraphNode(*appGraph));

    // Architecture graph node
    CNode *archGraphNode = CAddNode(sdf3Node, createPlatformGraphNode(platformGraph.get()));
    if (nocMapping != nullptr) {
        CAddNode(archGraphNode, nocMapping->createNetworkNode());
}

    // Mapping node (to platform graph and to interconnect)
    CNode *mappingNode = CAddNode(sdf3Node, createMappingNode(platformGraph.get(), appGraph.get()));
    if (nocMapping != nullptr) {
        CAddNode(mappingNode, nocMapping->createMappingNode());
}

    // System usage node
    CNode *systemUsageNode = CAddNode(sdf3Node, createSystemUsageNode(platformGraph.get()));
    if (nocMapping != nullptr) {
        nocMapping->addUsageNode(systemUsageNode);
}

    // Storage-space / throughput trade-off points
    CAddNode(sdf3Node, createStorageDistributionsNode(*minStorageDistributions));

    // Messages node
    if (nocMapping != nullptr) {
        CAddNode(sdf3Node, nocMapping->getMessages());
}

    return sdf3Node;
}

/**
 * outputMappingAsXML ()
 * The function outputs an sdf3 node to the supplied stream.
 */
void SDF3Flow::outputMappingAsXML(std::ostream &out) {
    // SDF mapping node
    CNode *sdf3Node = createSDF3Node();

    // Create document and save it
    std::unique_ptr<CDoc> doc = CNewDoc(sdf3Node);
    doc->saveFile(out);
}

/**
 * outputMappingAsHTML ()
 * The function outputs the mapping to a set of HTML pages.
 */
void SDF3Flow::outputMappingAsHTML() {
    SDFconvertToHTML sdfConvertToHTML;

    // Associate the graphs with the converter
    sdfConvertToHTML.setSDFgraph(getAppGraph());
    sdfConvertToHTML.setPlatformGraph(getPlatformGraph());
    sdfConvertToHTML.setSetOfNoCScheduleProblems(nocMapping->getSetOfNoCScheduleProblems());

    // Convert...
    sdfConvertToHTML.convert(getAppGraph()->getName() + "_");
}

} // namespace SDF