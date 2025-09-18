/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   December 6, 2009
 *
 *  Function        :   Output FSM-based SADF in HTML format
 *
 *  History         :
 *      06-12-09    :   Initial version.
 *
 * $Id: html.cc,v 1.4 2010-02-15 08:23:07 sander Exp $
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

#include "html.h"
#include <memory>
#include <sys/stat.h>

// For VS, the mkdir function is defined in direct.h
#ifdef _MSC_VER
#include <direct.h>
#endif

#include "fsmsadf/resource_allocation/flow/compsoc_platform/binding_aware_graph/binding_aware_graph.h"

namespace FSMSADF {

/**
 * OutputHTML()
 * Constructor.
 */
OutputHTML::OutputHTML() { dirname = "html"; }

/**
 * ~OutputHTML()
 * Destructor.
 */
OutputHTML::~OutputHTML() = default;

/**
 * outputAsHTML()
 * Output the flow as a set of HTML files.
 */
void OutputHTML::outputAsHTML(SDF3FlowBase *flow) {
    // Application graph
    printApplicationGraph(flow);

    // Binding aware graphs
    printBindingAwareGraphs(flow);

    // Platform graph
    printPlatformGraph(flow);

    // Platform bindings
    printPlatformBindings(flow);
}

/**
 * printApplicationGraph()
 * Print application graph.
 */
void OutputHTML::printApplicationGraph(SDF3FlowBase *flow) {
    std::ofstream oFile;

    // Get the application graph
    Graph* applicationGraph = flow->getApplicationGraph();

    // Create an output file for the application graph
    createOutputFileApplicationGraph(applicationGraph, oFile);

    // Print the file header
    printHeader(oFile, flow);
    oFile << "<h2>Application graph: " << applicationGraph->getName();
    oFile << "</h2>" << std::endl;

    // Print the FSM
    printFSM(applicationGraph->getFSM(), oFile);

    // Print list of scenarios
    oFile << "<p>Scenarios: ";
    for (const auto& s : applicationGraph->getScenarios()) {
        oFile << "<a href='" << getScenarioURL(*s) << "'>" << s->getName();
        oFile << "</a> ";
    }

    // Print each scenario
    for (const auto& s : applicationGraph->getScenarios()) {
        oFile << "<hr/>" << std::endl;
        printScenario(*s, getScenarioGraphPicURL(*s), oFile);
    }

    // Print the file footer
    printFooter(oFile, flow);
}

/**
 * printBindingAwareGraphs()
 * Print the binding aware graphs.
 */
void OutputHTML::printBindingAwareGraphs(SDF3FlowBase *flow) {
    PlatformBindings* platformBindings = flow->getPlatformBindings();

    for (const auto& platformBinding : *platformBindings) {
        if (platformBinding->getName() != "initial") {
            std::ofstream oFile;
            auto bag = std::make_unique<BindingAwareGraphCompSoCPlatform>(platformBinding.get());

            bag->constructBindingAwareGraph(true);

            // Create an output file for the application graph
            createOutputFileBindingAwareGraph(platformBinding.get(), oFile);

            // Print the file header
            printHeader(oFile, flow);
            oFile << "<h2>Binding aware graph: " << platformBinding->getName();
            oFile << "</h2>" << std::endl;

            // Print the FSM
            printFSM(bag->getFSM(), oFile);

            // Print list of scenarios
            oFile << "<p>Scenarios: ";
            for (const auto& s : bag->getScenarios()) {
                oFile << "<a href='" << getScenarioURL(*s) << "'>" << s->getName();
                oFile << "</a> ";
            }

            // Print each scenario
            for (const auto& s : bag->getScenarios()) {
                oFile << "<hr/>" << std::endl;

                CString name = getScenarioBAGPicURL(*s, platformBinding.get());
                printScenario(*s, name, oFile);
            }

            // Print the file footer
            printFooter(oFile, flow);
        }
    }
}

/**
 * printPlatformGraph()
 * Print platform graph.
 */
void OutputHTML::printPlatformGraph(SDF3FlowBase *flow) {
    std::ofstream oFile;
    CString map;

    // Get the platform graph
    PlatformGraph* pg = flow->getPlatformGraph();

    // Create an output file for the platform graph
    createOutputFilePlatformGraph(pg, oFile);

    // Print the file header
    printHeader(oFile, flow);
    oFile << "<h2>Platform graph: " << pg->getName() << "</h2>" << std::endl;

    // Insert picture of platform graph
    convertPlatformGraphToPNG(pg, getPlatformGraphPicURL(pg), map);
    oFile << "<p class='center'><img src='";
    oFile << getPlatformGraphPicURL(pg);
    oFile << "' usemap='#platform_graph_" << pg->getName().replace("-", "_");
    oFile << "'/></p>" << std::endl;
    oFile << std::endl;
    oFile << map << std::endl;

    // Print list of all tiles
    oFile << "<p>Tiles: ";
    for (const auto& t : pg->getTiles()) {
        oFile << "<a href='" << getTileURL(t.get()) << "'>";
        oFile << t->getName() << "</a>" << std::endl;
    }
    oFile << "</p>" << std::endl;

    // Print list of all connections
    oFile << "<p>Connections: ";
    for (const auto& c : pg->getConnections()) {
        oFile << "<a href='" << getConnectionURL(c.get()) << "'>";
        oFile << c->getName() << "</a>" << std::endl;
    }
    oFile << "</p>" << std::endl;

    // Tile properties
    for (const auto& t : pg->getTiles()) {
        oFile << "<a name='" << getTileURL(t.get(), true) << "'></a>" << std::endl;
        oFile << "<h4>Tile properties: " << t->getName() << "</h4>" << std::endl;

        // Processors
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Processor</th>" << std::endl;
        oFile << "<th>Type</th>" << std::endl;
        oFile << "<th>Arbitration type</th>" << std::endl;
        oFile << "<th>Wheelsize</th>" << std::endl;
        oFile << "<th>Context switch overhead</th>" << std::endl;
        oFile << "<th>Preemption?</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        for (const auto& p : t->getProcessors()) {
            oFile << "<tr>" << std::endl;
            oFile << "<td><a name='" << getProcessorURL(p.get(), true) << "'></a>";
            oFile << p->getName() << "</td>" << std::endl;
            oFile << "<td>" << p->getType() << "</td>" << std::endl;
            oFile << "<td>" << p->getArbitrationType() << "</td>" << std::endl;
            oFile << "<td>" << p->getWheelsize() << "</td>" << std::endl;
            oFile << "<td>" << p->getContextSwitchOverhead() << "</td>" << std::endl;
            if (p->hasSupportForPreemption()) {
                oFile << "<td>true</td>" << std::endl;
            } else {
                oFile << "<td>false</td>" << std::endl;
            }
            oFile << "</tr>" << std::endl;
        }
        oFile << "</table>" << std::endl;
        oFile << "<br/>" << std::endl;

        // Memories
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Memory</th>" << std::endl;
        oFile << "<th>Size</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        for (const auto& m : t->getMemories()) {
            oFile << "<tr>" << std::endl;
            oFile << "<td><a name='" << getMemoryURL(m.get(), true) << "'></a>";
            oFile << m->getName() << "</td>" << std::endl;
            oFile << "<td>" << m->getSize() << "</td>" << std::endl;
            oFile << "</tr>" << std::endl;
        }
        oFile << "</table>" << std::endl;
        oFile << "<br/>" << std::endl;

        // Network interfaces
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Network interface</th>" << std::endl;
        oFile << "<th>#incoming</th>" << std::endl;
        oFile << "<th>Bandwidth in</th>" << std::endl;
        oFile << "<th>#outgoing</th>" << std::endl;
        oFile << "<th>Bandwidth out</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        for (const auto& n : t->getNetworkInterfaces()) {
            oFile << "<tr>" << std::endl;
            oFile << "<td><a name='" << getNetworkInterfaceURL(n.get(), true) << "'></a>";
            oFile << n->getName() << "</td>" << std::endl;
            oFile << "<td>" << n->getMaxNrInConnections() << "</td>" << std::endl;
            oFile << "<td>" << n->getInBandwidth() << "</td>" << std::endl;
            oFile << "<td>" << n->getMaxNrOutConnections() << "</td>" << std::endl;
            oFile << "<td>" << n->getOutBandwidth() << "</td>" << std::endl;
            oFile << "</tr>" << std::endl;
        }
        oFile << "</table>" << std::endl;
        oFile << "<br/>" << std::endl;
    }

    // Connection properties
    oFile << "<h4>Connection properties</h4>" << std::endl;
    oFile << "<table>" << std::endl;
    oFile << "<tr>" << std::endl;
    oFile << "<th>Connection</th>" << std::endl;
    oFile << "<th>Src tile</th>" << std::endl;
    oFile << "<th>Src ni</th>" << std::endl;
    oFile << "<th>Dst tile</th>" << std::endl;
    oFile << "<th>Dst ni</th>" << std::endl;
    oFile << "<th>Latency</th>" << std::endl;
    oFile << "</tr>" << std::endl;
    for (const auto& c : pg->getConnections()) {
        NetworkInterface *src = c->getSrcNetworkInterface();
        NetworkInterface *dst = c->getDstNetworkInterface();

        oFile << "<tr>" << std::endl;
        oFile << "<td><a name='" << getConnectionURL(c.get(), true) << "'></a>";
        oFile << c->getName() << "</td>" << std::endl;
        oFile << "<td><a href='" << getTileURL(src->getTile()) << "'>";
        oFile << src->getTile()->getName() << "</a></td>" << std::endl;
        oFile << "<td><a href='" << getNetworkInterfaceURL(src) << "'>";
        oFile << src->getName() << "</a></td>" << std::endl;
        oFile << "<td><a href='" << getTileURL(dst->getTile()) << "'>";
        oFile << dst->getTile()->getName() << "</a></td>" << std::endl;
        oFile << "<td><a href='" << getNetworkInterfaceURL(dst) << "'>";
        oFile << dst->getName() << "</a></td>" << std::endl;
        oFile << "<td>" << c->getLatency() << "</td>" << std::endl;
        oFile << "</tr>" << std::endl;
    }
    oFile << "</table>" << std::endl;

    // Print the file footer
    printFooter(oFile, flow);
}

/**
 * printPlatformBindings()
 * Print all platform bindings.
 */
void OutputHTML::printPlatformBindings(SDF3FlowBase *flow) {
    PlatformBindings* platformBindings = flow->getPlatformBindings();
    PlatformGraph* pg = flow->getPlatformGraph();
    Graph* ag = flow->getApplicationGraph();
    std::ofstream oFile;

    // Compute repetition vector of each scenario
    std::map<const Scenario *, RepetitionVector> repetitionVectors;
    for (const auto& s : ag->getScenarios()) {
        repetitionVectors[s.get()] = s->computeRepetitionVector();
    }

    // Create an output file for the platform bindings overview
    createOutputFilePlatformBindings(oFile);

    // Print the file header
    printHeader(oFile, flow);
    oFile << "<h2>Platform bindings</h2>" << std::endl;

    oFile << "<p>The values printed in the table are the Pareto quantities ";
    oFile << "that are used at the end of the flow to determine Pareto ";
    oFile << "dominance. Please note that intermediate steps in the flow may ";
    oFile << "use different Pareto quantities to determine Pareto ";
    oFile << "dominance.</p>" << std::endl;

    // Print a list with all platform bindings
    oFile << "<table>" << std::endl;
    oFile << "<tr>" << std::endl;
    oFile << "<th>Binding</th>" << std::endl;
    for (const auto& t : pg->getTiles()) {
        for (const auto& p : t->getProcessors()) {
            oFile << "<th><a href='" << getTileURL(t.get()) << "'>";
            oFile << t->getName() << "</a>.<a href='" << getProcessorURL(p.get());
            oFile << "'>" << p->getName() << "</a></th>" << std::endl;
        }
    }
    for (const auto& t : pg->getTiles()) {
        for (const auto& m : t->getMemories()) {
            oFile << "<th><a href='" << getTileURL(t.get()) << "'>";
            oFile << t->getName() << "</a>.<a href='" << getMemoryURL(m.get());
            oFile << "'>" << m->getName() << "</a></th>" << std::endl;
        }
    }
    for (const auto& t : pg->getTiles()) {
        for (const auto& n : t->getNetworkInterfaces()) {
            oFile << "<th><a href='" << getTileURL(t.get()) << "'>";
            oFile << t->getName() << "</a>.<a href='";
            oFile << getNetworkInterfaceURL(n.get());
            oFile << "'>" << n->getName() << "</a></th>" << std::endl;
        }
    }
    oFile << "</tr>" << std::endl;
    for (const auto& pb : *platformBindings) {
        // Pareto quantities
        GraphBindingBase gb = GraphBindingBase(pb.get());
        gb.computeParetoQuantities(repetitionVectors, true);
        const GraphBindingBase::ParetoQuantities pq = gb.getParetoQuantities();

        oFile << "<tr>" << std::endl;
        oFile << "<td><a href='" << getPlatformBindingURL(pb.get()) << "'>";
        oFile << pb->getName() << "</a></td>" << std::endl;

        // Processors
        for (const auto& t : pg->getTiles()) {
            for (const auto& p : t->getProcessors()) {
                auto ppq = pq.processingLoad.find(p.get());
                // todo: provide nicer solution for Windows/Nix compatibility
                if (std::isnan(ppq->second)) {
                    oFile << "<td>0</td>" << std::endl;
                } else {
                    oFile << "<td>" << ppq->second;
                    oFile << "</td>" << std::endl;
                }
            }
        }

        // Memories
        for (const auto& t : pg->getTiles()) {
            for (const auto& m : t->getMemories()) {
                oFile << "<td>" << pq.memoryLoad.find(m.get())->second << "</td>";
                oFile << std::endl;
            }
        }

        // Communication load
        for (const auto& t : pg->getTiles()) {
            for (const auto& n : t->getNetworkInterfaces()) {
                oFile << "<td>" << pq.communicationLoad.find(n.get())->second;
                oFile << "</td>" << std::endl;
            }
        }
        oFile << "</tr>" << std::endl;
    }
    oFile << "</table>" << std::endl;

    // Print the file footer
    printFooter(oFile, flow);

    // Print the individual platform bindings
    for (const auto& i : *platformBindings) {
        printPlatformBinding(i.get(), flow);
    }
}

/**
 * printPlatformBinding()
 * Print platform binding.
 */
void OutputHTML::printPlatformBinding(PlatformBinding *pb, SDF3FlowBase *flow) {
    std::ofstream oFile;
    CString map;

    // Application graph
    Graph *ag = pb->getApplicationGraph();
    PlatformGraph *pg = pb->getPlatformGraph();

    // Create an output file for the platform binding
    createOutputFilePlatformBinding(pb, oFile);

    // Print the file header
    printHeader(oFile, flow);
    oFile << "<h2>Platform bindings: " << pb->getName() << "</h2>" << std::endl;

    // General information
    oFile << "<p>Application graph: <a href='";
    oFile << getApplicationGraphURL(ag) << "'>";
    oFile << ag->getName() << "</a></p>" << std::endl;
    oFile << "<p>Platform graph: <a href='";
    oFile << getPlatformGraphURL(pg) << "'>";
    oFile << pg->getName() << "</a></p>" << std::endl;
    oFile << "<p>Scenarios: ";
    for (const auto& s : ag->getScenarios()) {
        oFile << "<a href='" << getScenarioPlatformBindingURL(*s, pb);
        oFile << "'>" << s->getName() << "</a> ";
    }
    oFile << "</p>" << std::endl;

    // Resource usage
    oFile << "<h4>Resource usage</h4>" << std::endl;

    // Print resource usage of each processor
    oFile << "<table>" << std::endl;
    oFile << "<tr>" << std::endl;
    oFile << "<th>Processor</th>" << std::endl;
    oFile << "<th>Wheel usage</th>" << std::endl;
    oFile << "</tr>" << std::endl;
    for (const auto& tb : pb->getTileBindings()) {
        Tile *t = tb->getTile();

        // Processor bindings
        for (const auto& pcb : tb->getProcessorBindings()) {
            Processor *p = pcb->getProcessor();

            oFile << "<tr>" << std::endl;
            oFile << "<td><a href='" << getTileURL(t) << "'>";
            oFile << t->getName() << "</a>.<a href='";
            oFile << getProcessorURL(p) << "'>" << p->getName();
            oFile << "</a></td>" << std::endl;
            oFile << "<td>" << pcb->getWheelsizeUsedForGraph();
            oFile << " (";
            oFile << (100.0 * pcb->getWheelsizeUsedForGraph() / p->getWheelsize());
            oFile << "%)"
                  << "</td>" << std::endl;
            oFile << "</tr>" << std::endl;
        }
    }
    oFile << "</table>" << std::endl;
    oFile << "<br/>" << std::endl;

    // Print resource usage of each memory
    oFile << "<table>" << std::endl;
    oFile << "<tr>" << std::endl;
    oFile << "<th>Memory</th>" << std::endl;
    oFile << "<th>Usage</th>" << std::endl;
    oFile << "</tr>" << std::endl;

    // Memory bindings
    /// @todo reimplement this with new code!
    /*
    for (TileBindings::iterator j = pb->getTileBindings().begin();
         j != pb->getTileBindings().end(); j++)
    {
        TileBinding *tb = *j;
        Tile *t = tb->getTile();

         for (MemoryBindings::iterator k = tb->getMemoryBindings().begin();
                k != tb->getMemoryBindings().end(); k++)
        {
            MemoryBinding *mb = *k;
            Memory *m = mb->getMemory();

            oFile << "<tr>" << std::endl;
            oFile << "<td><a href='" << getTileURL(t) << "'>";
            oFile << t->getName() << "</a>.<a href='";
            oFile << getMemoryURL(m) << "'>" << m->getName();
            oFile << "</a></td>" << std::endl;
            oFile << "<td>" << mb->getMemorySizeUsedForGraph();
            oFile << " (";
            oFile << (100.0 * mb->getMemorySizeUsedForGraph() / m->getSize());
            oFile << "%)" << "</td>" << std::endl;
            oFile << "</tr>" << std::endl;
        }
    }
    * */
    oFile << "</table>" << std::endl;
    oFile << "<br/>" << std::endl;

    // Print resource usage of each network interface
    oFile << "<table>" << std::endl;
    oFile << "<tr>" << std::endl;
    oFile << "<th>Network interface</th>" << std::endl;
    oFile << "<th>#Incoming</th>" << std::endl;
    oFile << "<th>Bandwidth in</th>" << std::endl;
    oFile << "<th>#Outgoing</th>" << std::endl;
    oFile << "<th>Bandwidth out</th>" << std::endl;
    oFile << "</tr>" << std::endl;
    for (const auto& tb : pb->getTileBindings()) {
        Tile *t = tb->getTile();

        // Network interface bindings
        for (auto& nb : tb->getNetworkInterfaceBindings()) {
            NetworkInterface *n = nb->getNetworkInterface();

            oFile << "<tr>" << std::endl;
            oFile << "<td><a href='" << getTileURL(t) << "'>";
            oFile << t->getName() << "</a>.<a href='";
            oFile << getNetworkInterfaceURL(n) << "'>" << n->getName();
            oFile << "</a></td>" << std::endl;
            oFile << "<td>" << nb->getNrInConnectionsUsedForGraph();
            oFile << " (";
            oFile << (100.0 * nb->getNrInConnectionsUsedForGraph() / n->getMaxNrInConnections());
            oFile << "%)"
                  << "</td>" << std::endl;
            oFile << "<td>" << nb->getInBandwidthUsedForGraph();
            oFile << " (";
            oFile << (100.0 * nb->getInBandwidthUsedForGraph() / n->getInBandwidth());
            oFile << "%)"
                  << "</td>" << std::endl;
            oFile << "<td>" << nb->getNrOutConnectionsUsedForGraph();
            oFile << " (";
            oFile << (100.0 * nb->getNrOutConnectionsUsedForGraph() / n->getMaxNrOutConnections());
            oFile << "%)"
                  << "</td>" << std::endl;
            oFile << "<td>" << nb->getOutBandwidthUsedForGraph();
            oFile << " (";
            oFile << (100.0 * nb->getOutBandwidthUsedForGraph() / n->getOutBandwidth());
            oFile << "%)"
                  << "</td>" << std::endl;
            oFile << "</tr>" << std::endl;
        }
    }
    oFile << "</table>" << std::endl;
    oFile << "<br/>" << std::endl;

    // Scenarios
    for (const auto& s : ag->getScenarios()) {
        ScenarioGraph *sg = s->getScenarioGraph();
        bool hasBinding = false;

        // Scenario
        oFile << "<hr/>" << std::endl;
        oFile << "<a name='" << getScenarioPlatformBindingURL(*s, pb);
        oFile << "'></a>" << std::endl;
        oFile << "<h4>Scenario: " << s->getName() << "</h4>" << std::endl;

        // Insert picture of scenario graph to platform graph binding
        convertPlatformBindingToPNG(*s, pb, getPlatformBindingPicURL(*s, pb), map);
        oFile << "<p class='center'><img src='";
        oFile << getPlatformBindingPicURL(*s, pb);
        oFile << "' usemap='#platform_binding_" << s->getName() << "_";
        oFile << pb->getName().replace("-", "_");
        oFile << "'/></p>" << std::endl;
        oFile << std::endl;
        oFile << map << std::endl;

        // Print resource usage of each processor
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Processor</th>" << std::endl;
        oFile << "<th>Wheel usage</th>" << std::endl;
        oFile << "<th>Schedule</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        for (const auto& tb : pb->getTileBindings()) {
            Tile *t = tb->getTile();

            // Processor bindings
            for (const auto& pcb : tb->getProcessorBindings()) {
                Processor *p = pcb->getProcessor();

                oFile << "<tr>" << std::endl;
                oFile << "<td><a href='" << getTileURL(t) << "'>";
                oFile << t->getName() << "</a>.<a href='";
                oFile << getProcessorURL(p) << "'>" << p->getName();
                oFile << "</a></td>" << std::endl;
                oFile << "<td>" << pcb->getAllocatedWheelsize(*s);
                oFile << " (";
                oFile << (100.0 * pcb->getAllocatedWheelsize(*s) / p->getWheelsize());
                oFile << "%)"
                      << "</td>" << std::endl;
                oFile << "<td>";
                if (pcb->hasStaticOrderSchedule(*s)) {
                    StaticOrderSchedule &so = pcb->getStaticOrderSchedule(*s);
                    for (uint l = 0; l < so.size(); l++) {
                        ActorRef a = so[l].actor;

                        if (l == so.getStartPeriodicSchedule()) {
                            oFile << " (";
                        }

                        oFile << " <a href='" << getActorURL(*s, a) << "'>";
                        oFile << a->getName() << "</a>";
                    }
                    if (!so.empty()) {
                        oFile << " )";
                    }
                }
                oFile << "</td>" << std::endl;
                oFile << "</tr>" << std::endl;
            }
        }
        oFile << "</table>" << std::endl;
        oFile << "<br/>" << std::endl;

        // Print resource usage of each memory
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Memory</th>" << std::endl;
        oFile << "<th>Usage</th>" << std::endl;
        oFile << "<th>Actors</th>" << std::endl;
        oFile << "<th>Channels</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        // Memory bindings
        /*
        for (TileBindings::iterator j = pb->getTileBindings().begin();
             j != pb->getTileBindings().end(); j++)
        {
            TileBinding *tb = *j;
            Tile *t = tb->getTile();

            for (MemoryBindings::iterator k = tb->getMemoryBindings().begin();
                    k != tb->getMemoryBindings().end(); k++)
            {
                MemoryBinding *mb = *k;
                Memory *m = mb->getMemory();

                oFile << "<tr>" << std::endl;
                oFile << "<td><a href='" << getTileURL(t) << "'>";
                oFile << t->getName() << "</a>.<a href='";
                oFile << getMemoryURL(m) << "'>" << m->getName();
                oFile << "</a></td>" << std::endl;
                oFile << "<td>" << mb->getAllocatedMemorySize(s);
                oFile << " (";
                oFile << (100.0 * mb->getAllocatedMemorySize(s) / m->getSize());
                oFile << "%)" << "</td>" << std::endl;
                oFile << "<td>";
                for (Actors::iterator l = sg->getActors().begin();
                        l != sg->getActors().end(); l++)
                {
                    Actor *a = *l;

                    // Actor bound to memory in scenario?
                    if (mb->hasActorBinding(s, a))
                    {
                        oFile << "<a href='" << getActorURL(s, a);
                        oFile << "'>" << a->getName() << "</a> " << std::endl;
                    }
                }

                oFile << "</td>" << std::endl;
                oFile << "<td>";
                for (Channels::iterator l = sg->getChannels().begin();
                        l != sg->getChannels().end(); l++)
                {
                    Channel *c = *l;

                    // Channel bound to memory in scenario?
                    if (mb->hasChannelBinding(s, c))
                    {
                        oFile << "<a href='" << getChannelURL(s, c);
                        oFile << "'>" << c->getName() << "</a> " << std::endl;
                    }
                }
                oFile << "</td>" << std::endl;
                oFile << "</tr>" << std::endl;
            }
        }
        * */
        oFile << "</table>" << std::endl;
        oFile << "<br/>" << std::endl;

        // Print resource usage of each network interface
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Network interface</th>" << std::endl;
        oFile << "<th>#Incoming</th>" << std::endl;
        oFile << "<th>Bandwidth in</th>" << std::endl;
        oFile << "<th>Channels in</th>" << std::endl;
        oFile << "<th>#Outgoing</th>" << std::endl;
        oFile << "<th>Bandwidth out</th>" << std::endl;
        oFile << "<th>Channels out</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        for (const auto& tb : pb->getTileBindings()) {
            Tile *t = tb->getTile();

            // Network interface bindings
            for (auto& nb : tb->getNetworkInterfaceBindings()) {
                NetworkInterface *n = nb->getNetworkInterface();

                oFile << "<tr>" << std::endl;
                oFile << "<td><a href='" << getTileURL(t) << "'>";
                oFile << t->getName() << "</a>.<a href='";
                oFile << getNetworkInterfaceURL(n) << "'>" << n->getName();
                oFile << "</a></td>" << std::endl;
                oFile << "<td>" << nb->getAllocatedNrInConnections(*s);
                oFile << " (";
                oFile << (100.0 * nb->getAllocatedNrInConnections(*s) / n->getMaxNrInConnections());
                oFile << "%)"
                      << "</td>" << std::endl;
                oFile << "<td>" << nb->getAllocatedBandwidthInConnections(*s);
                oFile << " (";
                oFile << (100.0 * nb->getAllocatedBandwidthInConnections(*s) / n->getInBandwidth());
                oFile << "%)"
                      << "</td>" << std::endl;
                oFile << "<td>";
                for (const auto& c : sg->getChannels()) {
                    // Channel bound to network interface in scenario?
                    if (nb->hasInChannelBinding(*s, c.get())) {
                        oFile << "<a href='" << getChannelURL(*s, c.get());
                        oFile << "'>" << c->getName() << "</a> " << std::endl;
                    }
                }
                oFile << "</td>" << std::endl;
                oFile << "<td>" << nb->getAllocatedNrOutConnections(*s);
                oFile << " (";
                oFile << (100.0 * nb->getAllocatedNrOutConnections(*s)
                          / n->getMaxNrOutConnections());
                oFile << "%)"
                      << "</td>" << std::endl;
                oFile << "<td>" << nb->getAllocatedBandwidthOutConnections(*s);
                oFile << " (";
                oFile << (100.0 * nb->getAllocatedBandwidthOutConnections(*s)
                          / n->getOutBandwidth());
                oFile << "%)"
                      << "</td>" << std::endl;
                oFile << "<td>";
                for (const auto& c : sg->getChannels()) {
                    // Channel bound to network interface in scenario?
                    if (nb->hasOutChannelBinding(*s, c.get())) {
                        oFile << "<a href='" << getChannelURL(*s, c.get());
                        oFile << "'>" << c->getName() << "</a> " << std::endl;
                    }
                }
                oFile << "</td>" << std::endl;
                oFile << "</tr>" << std::endl;
            }
        }
        oFile << "</table>" << std::endl;
        oFile << "<br/>" << std::endl;

        // Print channel to connection bindings
        oFile << "<table>" << std::endl;
        oFile << "<tr>" << std::endl;
        oFile << "<th>Connection</th>" << std::endl;
        oFile << "<th>Channel</th>" << std::endl;
        oFile << "</tr>" << std::endl;
        for (const auto& cb : pb->getConnectionBindings()) {
            // Iterate over all channels
            for (const auto& c : sg->getChannels()) {
                // Channel bound to this connection in this scenario?
                if (cb->hasChannelBinding(*s, c.get())) {
                    oFile << "<tr>" << std::endl;
                    oFile << "<td><a href='";
                    oFile << getConnectionURL(cb->getConnection());
                    oFile << "'>" << cb->getConnection()->getName();
                    oFile << "</a></td>" << std::endl;
                    oFile << "<td><a href='" << getChannelURL(*s, c.get());
                    oFile << "'>" << c->getName() << "</a></td>" << std::endl;
                    oFile << "</tr>" << std::endl;

                    // Found a binding
                    hasBinding = true;
                }
            }
        }
        if (!hasBinding) {
            // Print message that no binding exists
            oFile << "<tr>" << std::endl;
            oFile << "<td>(no binding)</td>" << std::endl;
            oFile << "<td>(no binding)</td>" << std::endl;
            oFile << "</tr>" << std::endl;
        }
        oFile << "</table>" << std::endl;
    }

    // Print the file footer
    printFooter(oFile, flow);
}

/**
 * createOutputFileApplicationGraph()
 * Create a file to output the application graph.
 */
void OutputHTML::createOutputFileApplicationGraph(Graph *applicationGraph, std::ofstream &out) {
    createOutputFile(getApplicationGraphURL(applicationGraph), out);
}

/**
 * createOutputFileBindingAwareGraph()
 * Create a file to output the binding aware graph.
 */
void OutputHTML::createOutputFileBindingAwareGraph(PlatformBinding *platformBinding,
                                                   std::ofstream &out) {
    createOutputFile(getBindingAwareGraphURL(platformBinding), out);
}

/**
 * createOutputFilePlatformGraph()
 * Create a file to output the platform graph.
 */
void OutputHTML::createOutputFilePlatformGraph(PlatformGraph *platformGraph, std::ofstream &out) {
    createOutputFile(getPlatformGraphURL(platformGraph), out);
}

/**
 * createOutputFilePlatformBinding()
 * Create a file to output the platform binding.
 */
void OutputHTML::createOutputFilePlatformBinding(PlatformBinding *platformBinding,
                                                 std::ofstream &out) {
    createOutputFile(getPlatformBindingURL(platformBinding), out);
}

/**
 * createOutputFilePlatformBindings()
 * Create a file to output the platform bindings overview.
 */
void OutputHTML::createOutputFilePlatformBindings(std::ofstream &out) {
    createOutputFile(getPlatformBindingsURL(), out);
}

/**
 * createOutputFile()
 * Create an output file with the supplied name.
 */
void OutputHTML::createOutputFile(const CString &name, std::ofstream &out) {
    CString filename = getDirname() + "/" + name;

    // Create output directory
    createOutputDirectory();

    // Create output file
    out.open(filename);
    if (!out.is_open()) {
        throw CException("[ERROR] Failed creating '" + filename + "'");
    }
}

/**
 * createOutputDirectory()
 * Create directory for HTML output
 */
void OutputHTML::createOutputDirectory() {
#ifdef _MSC_VER
    _mkdir(dirname.c_str());
#else
    mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#endif
}

/**
 * printScenario()
 * Print the scenario in HTML format to the supplied output stream.
 */
void OutputHTML::printScenario(const Scenario& s, const CString &name, std::ostream &oFile) {
    CString map;

    // Scenario graph
    ScenarioGraph *sg = s.getScenarioGraph();

    // Section heading
    oFile << "<a name='" << getScenarioURL(s, true) << "'></a>" << std::endl;
    oFile << "<h3>Scenario: " << s.getName() << "</h3>" << std::endl;

    // Scenario graph
    oFile << "<p>Scenario graph: " << sg->getName() << std::endl;

    // Insert picture of scenario graph
    convertScenarioGraphToPNG(s, sg, name, map);
    oFile << "<p class='center'><img src='";
    oFile << name;
    oFile << "' usemap='#scenario_graph_" << s.getName() << "'/></p>" << std::endl;
    oFile << std::endl;
    oFile << map << std::endl;

    // Print list of all actors
    oFile << "<p>Actors: ";
    for (const auto& a : sg->getActors()) {
        oFile << "<a href='" << getActorURL(s, a.get()) << "'>";
        oFile << a->getName() << "</a>" << std::endl;
    }
    oFile << "</p>" << std::endl;

    // Print list of all channels
    oFile << "<p>Channels: ";
    for (const auto& c : sg->getChannels()) {
        oFile << "<a href='" << getChannelURL(s, c.get()) << "'>";
        oFile << c->getName() << "</a>" << std::endl;
    }
    oFile << "</p>" << std::endl;

    oFile << "<h4>Actor properties</h4>" << std::endl;
    oFile << "<table>" << std::endl;
    oFile << "<tr>";
    oFile << "<th>Actor</th>" << std::endl;
    oFile << "<th>Processor type</th>" << std::endl;
    oFile << "<th>Execution time</th>" << std::endl;
    oFile << "<th>State size</th>" << std::endl;
    oFile << "<th>Default?</th>" << std::endl;
    oFile << "</tr>" << std::endl;

    // Print properties of all actors
    for (const auto& a : sg->getActors()) {
        for (const auto &t : a->getProcessorTypes()) {
            oFile << "<tr>" << std::endl;
            oFile << "<td>";
            oFile << "<a name='" << getActorURL(s, a.get(), true) << "'></a>";
            oFile << a->getName() << "</td>" << std::endl;
            oFile << "<td>" << t.type << "</td>" << std::endl;
            oFile << "<td>" << a->getExecutionTimeOfScenario(s, t.type) << "</td>" << std::endl;
            /// @todo: print out memory size (instead of state size)
            oFile << "<td/>" << std::endl;
            // oFile << "<td>" << a->getStateSizeOfScenario(s, t.type) << "</td>" << std::endl;
            if (t.type == a->getDefaultProcessorType()) {
                oFile << "<td>true</td>" << std::endl;
            } else {
                oFile << "<td>false</td>" << std::endl;
            }
            oFile << "</tr>" << std::endl;
        }
    }

    oFile << "</table>" << std::endl;
    oFile << "<h4>Channel properties</h4>" << std::endl;
    oFile << "<table>" << std::endl;
    oFile << "<tr>";
    oFile << "<th>Channel</th>" << std::endl;
    oFile << "<th>Src actor</th>" << std::endl;
    oFile << "<th>Src port</th>" << std::endl;
    oFile << "<th>Src rate</th>" << std::endl;
    oFile << "<th>Dst actor</th>" << std::endl;
    oFile << "<th>Dst port</th>" << std::endl;
    oFile << "<th>Dst rate</th>" << std::endl;
    oFile << "<th>Initial tokens</th>" << std::endl;
    oFile << "<th>Token size</th>" << std::endl;
    oFile << "</tr>" << std::endl;

    // Print properties of all channels
    for (const auto& c : sg->getChannels()) {
        // Properties
        oFile << "<tr>" << std::endl;
        oFile << "<td>";
        oFile << "<a name='" << getChannelURL(s, c.get(), true) << "'></a>";
        oFile << c->getName();
        oFile << "</td>" << std::endl;
        oFile << "<td><a href='" << getActorURL(s, c->getSrcActor());
        oFile << "'>" << c->getSrcActor()->getName() << "</a></td>" << std::endl;
        oFile << "<td><a href='" << getActorURL(s, c->getSrcActor());
        oFile << "'>" << c->getSrcPort()->getName() << "</a></td>" << std::endl;
        oFile << "<td>" << c->getSrcPort()->getRateOfScenario(s) << "</td>" << std::endl;
        oFile << "<td><a href='" << getActorURL(s, c->getDstActor());
        oFile << "'>" << c->getDstActor()->getName() << "</a></td>" << std::endl;
        oFile << "<td><a href='" << getActorURL(s, c->getDstActor());
        oFile << "'>" << c->getDstPort()->getName() << "</a></td>" << std::endl;
        oFile << "<td>" << c->getDstPort()->getRateOfScenario(s) << "</td>" << std::endl;
        oFile << "<td>" << c->getInitialTokens() << "</td>" << std::endl;
        oFile << "<td>" << c->getTokenSizeOfScenario(s) << "</td>" << std::endl;
        oFile << "</tr>" << std::endl;
    }
    oFile << "</table>" << std::endl;
}

/**
 * printFSM()
 * Print the FSM to the supplied stream.
 */
void OutputHTML::printFSM(FSM *fsm, std::ostream &oFile) {
    CString map;

    oFile << "<h4>FSM</h4>" << std::endl;
    convertFSMToPNG(fsm, getFSMPicURL(), map);
    oFile << "<p class='center'><img src='" << getFSMPicURL();
    oFile << "' usemap='#fsm'/></p>" << std::endl;
    oFile << map << std::endl;
}

/**
 * printHeader()
 * Print the HTML header to the supplied output stream.
 */
void OutputHTML::printHeader(std::ostream &oFile, SDF3FlowBase *flow) {
    oFile << "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' ";
    oFile << "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>" << std::endl;
    oFile << "<html xmlns='http://www.w3.org/1999/xhtml'>" << std::endl;
    oFile << "<head>" << std::endl;
    oFile << "    <title>SDF3</title>" << std::endl;
    oFile << "    <meta http-equiv='Content-type' content='text/html;charset=iso-8859-1' />"
          << std::endl;
    oFile << "    <style>" << std::endl;
    printStylesheet(oFile);
    oFile << "    </style>" << std::endl;
    oFile << "</head>" << std::endl;
    oFile << "<body>" << std::endl;
    oFile << "<a name='top'></a>" << std::endl;
    oFile << "<div id='mainone'>" << std::endl;
    oFile << std::endl;
    oFile << "    <!-- header -->" << std::endl;
    oFile << "    <div id='header'>" << std::endl;
    oFile << "        <a href='https://www.es.ele.tue.nl/sdf3'>SDF3</a>" << std::endl;
    oFile << "    </div>" << std::endl;
    oFile << std::endl;
    oFile << "    <!-- navigation -->" << std::endl;
    oFile << "    <div id='nav-main'>" << std::endl;
    oFile << "        <ul id='nav-main-list'>" << std::endl;
    oFile << "            <li><a href='";
    oFile << getApplicationGraphURL(flow->getApplicationGraph());
    oFile << "'>Application Graph</a></li>" << std::endl;
    oFile << "            <li><a href='";
    oFile << getPlatformGraphURL(flow->getPlatformGraph());
    oFile << "'>Platform Graph</a></li>" << std::endl;
    oFile << "            <li><a href='";
    oFile << getPlatformBindingsURL();
    oFile << "'>Platform Bindings</a></li>" << std::endl;
    oFile << "        </ul>" << std::endl;
    oFile << "    </div>" << std::endl;
    oFile << std::endl;
    oFile << "    <div id='content'>";
}

/**
 * printFooter()
 * Print the HTML footer to the supplied output stream.
 */
void OutputHTML::printFooter(std::ostream &oFile, SDF3FlowBase * /*flow*/) {
    oFile << "    </div>" << std::endl;
    oFile << "</div>" << std::endl;
    oFile << "</body>" << std::endl;
    oFile << "</html>" << std::endl;
}

/**
 * printStylesheet ()
 * Write the style sheet to the supplied output stream.
 */
void OutputHTML::printStylesheet(std::ostream &oFile) {
    oFile << "        #header a {" << std::endl;
    oFile << "            text-decoration: none;" << std::endl;
    oFile << "            color: #6889b5;" << std::endl;
    oFile << "            font-size: 20pt;" << std::endl;
    oFile << "            font-weight: bold;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #nav-main {" << std::endl;
    oFile << "            background-color: #4f6889;" << std::endl;
    oFile << "            width: 100%;" << std::endl;
    oFile << "            font-weight: bold;" << std::endl;
    oFile << "            font-family: 'Luxi Sans', verdana, helvetica, arial, sans-serif;"
          << std::endl;
    oFile << "            font-size: x-small;" << std::endl;
    oFile << "            margin-top: 5px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #nav-main-list {" << std::endl;
    oFile << "            height: 20px; /* size of navigation bar */" << std::endl;
    oFile << "            list-style: none;" << std::endl;
    oFile << "            margin: 0px;" << std::endl;
    oFile << "            padding: 0px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #nav-main-list li {" << std::endl;
    oFile << "            float: left;" << std::endl;
    oFile << "            display: inline;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #nav-main-list a {" << std::endl;
    oFile << "            color: #fff;" << std::endl;
    oFile << "            text-decoration: none;" << std::endl;
    oFile << "            vertical-align: middle;" << std::endl;
    oFile << "            padding-left: 10px;" << std::endl;
    oFile << "            padding-right: 10px;" << std::endl;
    oFile << "            line-height: 20px;" << std::endl;
    oFile << "            display: block;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "         " << std::endl;
    oFile << "        #nav-main-list a:hover {" << std::endl;
    oFile << "            background-color: #7ca3d6;" << std::endl;
    oFile << "        } " << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #nav-main-list li strong{" << std::endl;
    oFile << "            background-color: #6889b5;" << std::endl;
    oFile << "            display: block;" << std::endl;
    oFile << "        } " << std::endl;
    oFile << "        " << std::endl;
    oFile << "        * html .nav-corner-br {" << std::endl;
    oFile << "          margin-top: -12px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        * html .nav-corner-bl {" << std::endl;
    oFile << "          margin-top: -12px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        body {" << std::endl;
    oFile << "          margin: 0;" << std::endl;
    oFile << "          padding: 0;" << std::endl;
    oFile << "          text-align: center;" << std::endl;
    oFile << "          font-family: 'Luxi Sans', 'Bitstream Vera Sans', ";
    oFile << "'Lucida Grande', 'Trebuchet MS', helvetica, verdana, arial, sans-serif;" << std::endl;
    oFile << "          font-size: small;" << std::endl;
    oFile << "          color: #333;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        a {" << std::endl;
    oFile << "          text-decoration: none;" << std::endl;
    oFile << "          color: #333;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        a:hover {" << std::endl;
    oFile << "          text-decoration: underline;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #mainone {" << std::endl;
    oFile << "            min-width: 720px;" << std::endl;
    oFile << "            padding-left: 30px;" << std::endl;
    oFile << "            padding-right: 30px;" << std::endl;
    oFile << "            margin-left: auto;" << std::endl;
    oFile << "            margin-right: auto;" << std::endl;
    oFile << "            text-align: left;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #site-left {" << std::endl;
    oFile << "            width: 120px;" << std::endl;
    oFile << "            float: left;" << std::endl;
    oFile << "            margin-right: 20px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #site-middle-two {" << std::endl;
    oFile << "            margin-left: 140px;  /*width of site-left block */" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #content {" << std::endl;
    oFile << "            margin: 10px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        img {" << std::endl;
    oFile << "            border: 0px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.center {" << std::endl;
    oFile << "            text-align: center;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        .command {" << std::endl;
    oFile << "            font-family: 'Courier';" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.command {" << std::endl;
    oFile << "            margin-top: 10px;" << std::endl;
    oFile << "            margin-bottom: 10px;" << std::endl;
    oFile << "            white-space: pre;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.example {" << std::endl;
    oFile << "          display: block;" << std::endl;
    oFile << "          padding: 10px;" << std::endl;
    oFile << "          border: 1px solid #6666cc;" << std::endl;
    oFile << "          color: #000;" << std::endl;
    oFile << "          overflow: auto;" << std::endl;
    oFile << "          margin: 1em 2em;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        code.screen, pre.screen {" << std::endl;
    oFile << "            font-family: 'Courier';" << std::endl;
    oFile << "          display: block;" << std::endl;
    oFile << "          padding: 10px;" << std::endl;
    oFile << "          border: 1px solid #bbb;" << std::endl;
    oFile << "          background-color: #eee;" << std::endl;
    oFile << "          color: #000;" << std::endl;
    oFile << "          overflow: auto;" << std::endl;
    oFile << "          margin: 1em 2em;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        pre {" << std::endl;
    oFile << "            font-size: 10pt;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        img.right {" << std::endl;
    oFile << "            display: block;" << std::endl;
    oFile << "            float: right;" << std::endl;
    oFile << "            margin: 10px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        img.left {" << std::endl;
    oFile << "            display: block;" << std::endl;
    oFile << "            float: left;" << std::endl;
    oFile << "            margin: 10px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        p.right {" << std::endl;
    oFile << "            text-align: right;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.caution {" << std::endl;
    oFile << "          display: block;" << std::endl;
    oFile << "          padding: 10px;" << std::endl;
    oFile << "          border: 1px solid #6666cc;" << std::endl;
    oFile << "          background-color: #ccccff;" << std::endl;
    oFile << "          color: #000;" << std::endl;
    oFile << "          overflow: auto;" << std::endl;
    oFile << "          margin: 1em 2em;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.info {" << std::endl;
    oFile << "          display: block;" << std::endl;
    oFile << "          padding: 10px;" << std::endl;
    oFile << "          border: 1px solid #6666cc;" << std::endl;
    oFile << "          background-color: #ccccff;" << std::endl;
    oFile << "          color: #000;" << std::endl;
    oFile << "          overflow: auto;" << std::endl;
    oFile << "          margin: 1em 2em;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        th {" << std::endl;
    oFile << "            color: #4f6889;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        .header {" << std::endl;
    oFile << "            color: #4f6889;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        ul.separate li {" << std::endl;
    oFile << "            margin-bottom: 10px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        ol.separate li {" << std::endl;
    oFile << "            margin-bottom: 10px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        li.obsolete {" << std::endl;
    oFile << "            color: #aaa;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.tabs {" << std::endl;
    oFile << "            width: 100%;" << std::endl;
    oFile << "            height: 20px; /* size of navigation bar */" << std::endl;
    oFile << "            list-style: none;" << std::endl;
    oFile << "            margin: 0px;" << std::endl;
    oFile << "            padding: 0px;" << std::endl;
    oFile << "            " << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.tabs ul {" << std::endl;
    oFile << "            margin: 0px;" << std::endl;
    oFile << "            padding: 0px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.tabs li {" << std::endl;
    oFile << "            float: left;" << std::endl;
    oFile << "            display: inline;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        div.tabs a {" << std::endl;
    oFile << "            color: #6f6f6f;" << std::endl;
    oFile << "            text-decoration: none;" << std::endl;
    oFile << "            padding-left: 0px;" << std::endl;
    oFile << "            padding-right: 10px;" << std::endl;
    oFile << "            line-height: 20px;" << std::endl;
    oFile << "            display: block;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "         " << std::endl;
    oFile << "        div.tabs a:hover {" << std::endl;
    oFile << "            text-decoration: underline;" << std::endl;
    oFile << "        } " << std::endl;
    oFile << "        " << std::endl;
    oFile << "        #current{" << std::endl;
    oFile << "            font-weight: bold;" << std::endl;
    oFile << "            display: block;" << std::endl;
    oFile << "        } " << std::endl;
    oFile << "        " << std::endl;
    oFile << "        .center{" << std::endl;
    oFile << "            text-align: center;" << std::endl;
    oFile << "        } " << std::endl;
    oFile << "        td {" << std::endl;
    oFile << "            padding-right: 30px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        th {" << std::endl;
    oFile << "            padding-right: 30px;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        .footer {" << std::endl;
    oFile << "          font-size: x-small;" << std::endl;
    oFile << "            color: #777;" << std::endl;
    oFile << "            padding-top: 50px;" << std::endl;
    oFile << "            padding-bottom: 30px;" << std::endl;
    oFile << "            clear: both;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        a.footer {" << std::endl;
    oFile << "            text-decoration: none;" << std::endl;
    oFile << "            color: #666699;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        a.footer:visited {" << std::endl;
    oFile << "            color: #666699;" << std::endl;
    oFile << "        }" << std::endl;
    oFile << "        " << std::endl;
    oFile << "        a.footer:hover {" << std::endl;
    oFile << "            text-decoration: underline;" << std::endl;
    oFile << "            color: #666699;" << std::endl;
    oFile << "        }" << std::endl;
}

/**
 * getApplicationGraphURL()
 * Get an URL for the application graph.
 */
CString OutputHTML::getApplicationGraphURL(Graph *applicationGraph) {
    return ("application-graph-" + applicationGraph->getName() + ".html");
}

/**
 * getBindingAwareGraphURL()
 * Get an URL for the binding aware graph.
 */
CString OutputHTML::getBindingAwareGraphURL(PlatformBinding *platformBinding) {
    return ("binding-aware-graph-" + platformBinding->getName() + ".html");
}

/**
 * getScenarioURL()
 * Get an URL for the scenario.
 */
CString OutputHTML::getScenarioURL(const Scenario& s, bool onlyName) {
    if (onlyName) {
        return "sc-" + s.getName();
    }

    return getApplicationGraphURL(s.getGraph()) + "#sc-" + s.getName();
}

/**
 * getActorURL()
 * Get an URL for the actor.
 */
CString OutputHTML::getActorURL(const Scenario& s, ActorRef a, bool onlyName) {
    return getScenarioURL(s, onlyName) + "-ac-" + a->getName();
}

/**
 * getChannelURL()
 * Get an URL for the channel.
 */
CString OutputHTML::getChannelURL(const Scenario& s, Channel *c, bool onlyName) {
    return getScenarioURL(s, onlyName) + "-ch-" + c->getName();
}

/**
 * getPlatformGraphURL()
 * Get an URL for the platform graph.
 */
CString OutputHTML::getPlatformGraphURL(PlatformGraph *platformGraph) {
    return ("platform-graph-" + platformGraph->getName() + ".html");
}

/**
 * getTileURL()
 * Get an URL for the tile.
 */
CString OutputHTML::getTileURL(Tile *t, bool onlyName) {
    if (onlyName) {
        return "tile-" + t->getName();
    }

    return getPlatformGraphURL(dynamic_cast<PlatformGraph *>(t->getParent())) + "#tile-"
           + t->getName();
}

/**
 * getNetworkInterfaceURL()
 * Get an URL for the network interface.
 */
CString OutputHTML::getNetworkInterfaceURL(NetworkInterface *n, bool onlyName) {
    return getTileURL(n->getTile(), onlyName) + "-ni-" + n->getName();
}

/**
 * getProcessorURL()
 * Get an URL for the processor.
 */
CString OutputHTML::getProcessorURL(Processor *p, bool onlyName) {
    return getTileURL(p->getTile(), onlyName) + "-proc-" + p->getName();
}

/**
 * getMemoryURL()
 * Get an URL for the memory.
 */
CString OutputHTML::getMemoryURL(Memory *m, bool onlyName) {
    return getTileURL(m->getTile(), onlyName) + "-mem-" + m->getName();
}

/**
 * getConnectionURL()
 * Get an URL for the connection.
 */
CString OutputHTML::getConnectionURL(Connection *c, bool onlyName) {
    if (onlyName) {
        return "connection-" + c->getName();
    }

    return getPlatformGraphURL(c->getGraph()) + "#connection-" + c->getName();
}

/**
 * getPlatformBindingURL()
 * Get an URL for the platform binding.
 */
CString OutputHTML::getPlatformBindingURL(PlatformBinding *platformBinding) {
    return ("platform-binding-" + platformBinding->getName() + ".html");
}

/**
 * getPlatformBindingsURL()
 * Get an URL for the platform bindings.
 */
CString OutputHTML::getPlatformBindingsURL() { return {"platform-bindings.html"}; }

/**
 * getScenarioPlatformBindingURL()
 * Get an URL for the platform binding of scenario s.
 */
CString OutputHTML::getScenarioPlatformBindingURL(const Scenario& s, PlatformBinding *pb, bool onlyName) {
    if (onlyName) {
        return CString("platform-binding-") + pb->getName() + "-" + s.getName();
    }

    return getPlatformBindingURL(pb) + "#platform-binding-" + pb->getName() + "-" + s.getName();
}

/**
 * getScenarioGraphPicURL()
 * Get an URL for the scenario graph picture.
 */
CString OutputHTML::getScenarioGraphPicURL(const Scenario& s) {
    return ("scenario-graph-" + s.getName() + ".png");
}

/**
 * getScenarioBAGPicURL()
 * Get an URL for the scenario graph picture.
 */
CString OutputHTML::getScenarioBAGPicURL(const Scenario& s, PlatformBinding *platformBinding) {
    return ("scenario-bag-" + s.getName() + "-" + platformBinding->getName() + ".png");
}

/**
 * getPlatformGraphPicURL()
 * Get an URL for the platform graph picture.
 */
CString OutputHTML::getPlatformGraphPicURL(PlatformGraph *pg) {
    return ("platform-graph-" + pg->getName() + ".png");
}

/**
 * getFSMPicURL()
 * Get an URL for the FSM picture.
 */
CString OutputHTML::getFSMPicURL() { return {"fsm.png"}; }

/**
 * getPlatformBindingPicURL()
 * Get an URL for the platform binding picture.
 */
CString OutputHTML::getPlatformBindingPicURL(const Scenario& s, PlatformBinding *pb) {
    return ("platform-binding-" + s.getName() + "-" + pb->getName() + ".png");
}

/**
 * convertGraphToPNG ()
 * Convert the graph using dot to a PNG image. The resulting picture is stored
 * in a file with the supplied filename. The function also creates an image
 * map in XHTML format. This image map is returned through the map variable.
 * The function returns true on success. Otherwise it returns false.
 */
bool OutputHTML::convertGraphToPNG(CString &graph, CString &filename, CString &map) const {
    CString tmpFilename;
    std::ofstream tmpFile;
    std::ifstream mapFile;

    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);

    // Store graph in file
    tmpFile << graph;

    // Close the temporary file
    tmpFile.close();

    // Run dot to convert the picture
    CString cmd = "dot -Tpng -o " + getDirname() + "/" + filename + " " + tmpFilename;
    if (system(cmd.c_str()) != 0) { // NOLINT(cert-env33-c,concurrency-mt-unsafe)
        return false;
    }

    // Run dot to convert the picture
    if (system(("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename).c_str()) != 0) { // NOLINT(cert-env33-c,concurrency-mt-unsafe)
        return false;
    }

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");

    // Clear map
    map = "";

    // Read map from file
    while (!mapFile.eof()) {
        char c = 0;
        mapFile.get(c);

        if (!mapFile.eof()) {
            map += c;
        }
    }

    // Close the map
    mapFile.close();

    // Remove temporary file
    remove(tmpFilename.c_str()); // NOLINT(cert-err33-c)
    remove((tmpFilename + ".map").c_str()); // NOLINT(cert-err33-c)

    // Done
    return true;
}

/**
 * convertScenarioGraphToPNG()
 * The function converts the supplied scenario graph to a PNG image and store
 * it in the supplied filename. An XHTML image map is returned through the map
 * variable.
 */
void OutputHTML::convertScenarioGraphToPNG(const Scenario& s,
                                           ScenarioGraph *sg,
                                           CString filename,
                                           CString &map) {
    CString graph;

    // Header
    graph = "digraph scenario_graph_" + s.getName().replace("-", "_") + " {\n";
    graph += "    size=\"70,100\";\n";

    // Actors
    for (const auto& a: sg->getActors()) {
        graph += "    " + a->getName().replace("-", "_");
        graph += " [ label=\"" + a->getName();
        graph += "\", href=\"" + getActorURL(s, a.get()) + "\" ];\n";
    }

    // Channels
    for (const auto& c: sg->getChannels()) {
        Actor *srcA = c->getSrcActor();
        Actor *dstA = c->getDstActor();

        graph += "    " + srcA->getName().replace("-", "_");
        graph += " -> " + dstA->getName().replace("-", "_");
        graph += " [ label=\"" + c->getName() + "(";
        graph += CString(c->getInitialTokens()) + ")\"";
        graph += " headlabel=\"" + CString(c->getDstPort()->getRateOfScenario(s)) + "\"";
        graph += " taillabel=\"" + CString(c->getSrcPort()->getRateOfScenario(s)) + "\"";
        graph += " href=\"" + getChannelURL(s, c.get());
        graph += "\" ];\n";
    }

    // Footer
    graph += "}\n";

    // Convert graph to PNG
    convertGraphToPNG(graph, filename, map);
}

/**
 * convertFSMToPNG()
 * The function converts the supplied FSM to a PNG image and stores
 * it in the supplied filename. An XHTML image map is returned through the map
 * variable.
 */
void OutputHTML::convertFSMToPNG(FSM *fsm, CString filename, CString &map) {
    CString graph;

    // Header
    graph = "digraph fsm {\n";
    graph += "    size=\"7,10\";\n";

    // States
    for (const auto& s: fsm->getStates()) {
        graph += "    " + s->getName().replace("-", "_");
        graph += " [ label=\"" + s->getName() + "(";
        graph += s->getScenario()->getName() + ")";
        graph += "\", href=\"" + getScenarioURL(*(s->getScenario())) + "\" ";

        if (s.get() == fsm->getInitialState()) {
            graph += " shape=\"doublecircle\" ";
        } else {
            graph += " shape=\"circle\" ";
}

        graph += "];\n";
    }

    // Transitions
    for (const auto& s: fsm->getStates()) {
        for (const auto& t: s->getTransitions()) {
            graph += "    " + s->getName().replace("-", "_");
            graph += " -> " + t->getDstState()->getName().replace("-", "_");
            graph += " [ label=\"" + t->getName() + "\" ];\n";
        }
    }

    // Footer
    graph += "}\n";

    // Convert graph to PNG
    convertGraphToPNG(graph, filename, map);
}

/**
 * convertPlatformGraphToPNG()
 * The function converts the supplied platform graph to a PNG image and store
 * it in the supplied filename. An XHTML image map is returned through the map
 * variable.
 */
void OutputHTML::convertPlatformGraphToPNG(PlatformGraph *pg, CString filename, CString &map) {
    CString graph;

    // Header
    graph = "digraph platform_graph_" + pg->getName().replace("-", "_") + " {\n";
    graph += "    size=\"7,10\";\n";

    // Tiles
    for (const auto& t: pg->getTiles()) {
        graph += "    " + t->getName().replace("-", "_");
        graph += " [ label=\"" + t->getName();
        graph += "\", href=\"" + getTileURL(t.get()) + "\" ";
        graph += " shape=\"box\" ];\n";
    }

    // Connections
    for (const auto& c: pg->getConnections()) {
        Tile *src = c->getSrcNetworkInterface()->getTile();
        Tile *dst = c->getDstNetworkInterface()->getTile();

        graph += "    " + src->getName().replace("-", "_");
        graph += " -> " + dst->getName().replace("-", "_");
        graph += " [ label=\"" + c->getName() + "\"";
        graph += " href=\"" + getConnectionURL(c.get()) + "\" ];\n";
    }

    // Footer
    graph += "}\n";

    // Convert graph to PNG
    convertGraphToPNG(graph, filename, map);
}

/**
 * convertPlatformBindingToPNG()
 * The function converts the supplied platform binding to a PNG image and store
 * it in the supplied filename. An XHTML image map is returned through the map
 * variable.
 */
void OutputHTML::convertPlatformBindingToPNG(const Scenario& s,
                                             PlatformBinding *pb,
                                             CString filename,
                                             CString &map) {
    CString graph;

    // Scenario graph and platform graph
    ScenarioGraph *sg = s.getScenarioGraph();
    PlatformGraph *pg = pb->getPlatformGraph();

    // Header
    graph = "digraph platform_binding_" + s.getName() + "_";
    graph += pb->getName().replace("-", "_");
    graph += +" {\n";
    graph += "    size=\"7,10\";\n";

    // Actors
    for (const auto& a: sg->getActors()) {
        graph += "    " + a->getName().replace("-", "_");
        graph += " [ label=\"" + a->getName();
        graph += "\", href=\"" + getActorURL(s, a.get()) + "\" ];\n";
    }

    // Channels
    for (const auto& c: sg->getChannels()) {
        Actor *srcA = c->getSrcActor();
        Actor *dstA = c->getDstActor();

        graph += "    " + srcA->getName().replace("-", "_");
        graph += " -> " + dstA->getName().replace("-", "_");
        graph += " [ label=\"" + c->getName() + "(";
        graph += CString(c->getInitialTokens()) + ")\"";
        graph += " href=\"" + getChannelURL(s, c.get());
        graph += "\" ];\n";
    }

    // Tiles
    for (const auto& t: pg->getTiles()) {
        graph += "    " + t->getName().replace("-", "_");
        graph += " [ label=\"" + t->getName();
        graph += "\", href=\"" + getTileURL(t.get()) + "\" ";
        graph += " shape=\"box\" ];\n";
    }

    // Connections
    for (const auto& c: pg->getConnections()) {
        Tile *src = c->getSrcNetworkInterface()->getTile();
        Tile *dst = c->getDstNetworkInterface()->getTile();

        graph += "    " + src->getName().replace("-", "_");
        graph += " -> " + dst->getName().replace("-", "_");
        graph += " [ label=\"" + c->getName() + "\"";
        graph += " href=\"" + getConnectionURL(c.get()) + "\" ];\n";
    }

    // Bindings from actors to tiles
    for (const auto& tb: pb->getTileBindings()) {
        Tile *t = tb->getTile();

        // Iterate over all processor bindings inside tile
        for (const auto& pcb: tb->getProcessorBindings()) {
            // Iterate over all actors
            for (const auto& a: sg->getActors()) {
                // Actor bound to processor in scenario?
                if (pcb->hasActorBinding(s, a.get())) {
                    graph += "    " + a->getName().replace("-", "_");
                    graph += " -> " + t->getName().replace("-", "_");
                    graph += " [ style=\"dashed\" ];\n";
                }
            }
        }
    }

    // Footer
    graph += "}\n";

    // Convert graph to PNG
    convertGraphToPNG(graph, filename, map);
}

} // End namespace FSMSADF
