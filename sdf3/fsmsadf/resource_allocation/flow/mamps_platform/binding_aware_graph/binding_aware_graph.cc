/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      16-11-11    :   Initial version.
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

#include "binding_aware_graph.h"

namespace FSMSADF {

/**
 * BindingAwareGraph()
 * Constructor.
 */
BindingAwareGraphMampsPlatform::BindingAwareGraphMampsPlatform(const PlatformBinding *b) :
    BindingAwareGraphBase(b) {}

/**
 * ~BindingAwareGraph()
 * Destructor.
 */
BindingAwareGraphMampsPlatform::~BindingAwareGraphMampsPlatform() = default;

/**
 * Model the communication in a SDFG by
 * first creating the actors and then connecting
 * them by inserting channels in between them.
 *
 * Once the graph is created, add it to the application SDFG
 * by linking the created graph to app graph.
 *
 * The communication model implemented in this
 * method is presented in [Roel et al.] "An Automated Flow to Map
 * Throughput Constrained Applications to a MPSoC"
 *
 * @param c The channel that is mapped to the connection
 * @param bc Constraints on the channel binding
 * @param co Connection on which the channel will be mapped
 * @param s Scenario in which the mapping is performed
 *
 */
void BindingAwareGraphMampsPlatform::modelChannelToConnectionBinding(Channel *c,
                                                                     ChannelBindingConstraint *bc,
                                                                     Connection *co,
                                                                     Scenario *s) {
    // String to hold comma separated names of initial tokens.
    CString persistentTokenNames;

    // Scenario graph that will include the binding aware graph
    ScenarioGraph *sg = c->getScenarioGraph();

    // Buffer size of the channel
    BufferSize b = bc->getBufferSize();

    // The latency during the transmission through the connection 'co'
    Time latency = co->getLatency();

    // Constants used to compute overheads, buffersize and

    const int N = static_cast<int>((c->getTokenSizeOfScenario(*s) + 3) / 4);
    const int fifo_buffer_size = 32;
    const int w = 1;

    // sending and receiving overheads
    Time sending_overhead = 1000 * N; // NOLINT(*magic-numbers)
    Time receiving_overhead = 1000 * N; // NOLINT(*magic-numbers)

    // create the actor S1
    ActorRef s1 = sg->createActor();
    s1->setName(c->getName() + "_s1");
    s1->setExecutionTimeOfScenario(*s, "s1", sending_overhead);
    s1->setDefaultProcessorType("s1");

    // create the actor S2
    ActorRef s2 = sg->createActor();
    s2->setName(c->getName() + "_s2");
    s2->setExecutionTimeOfScenario(*s, "s2", 0);
    s2->setDefaultProcessorType("s2");

    // create the actor S3
    ActorRef s3 = sg->createActor();
    s3->setName(c->getName() + "_s3");
    s3->setExecutionTimeOfScenario(*s, "s3", 0);
    s3->setDefaultProcessorType("s3");

    // create the actor c1
    ActorRef c1 = sg->createActor();
    c1->setName(c->getName() + "_c1");
    c1->setExecutionTimeOfScenario(*s, "c1", 1);
    c1->setDefaultProcessorType("c1");

    // create the actor c2
    ActorRef c2 = sg->createActor();
    c2->setName(c->getName() + "_c2");
    c2->setExecutionTimeOfScenario(*s, "c2", latency - 1);
    c2->setDefaultProcessorType("c2");

    // create actor d1
    ActorRef d1 = sg->createActor();
    d1->setName(c->getName() + "_d1");
    d1->setExecutionTimeOfScenario(*s, "d1", receiving_overhead);
    d1->setDefaultProcessorType("d1");

    // create actor d2
    ActorRef d2 = sg->createActor();
    d2->setName(c->getName() + "_d2");
    d2->setExecutionTimeOfScenario(*s, "d2", 0);
    d2->setDefaultProcessorType("d2");

    // create actor d3
    ActorRef d3 = sg->createActor();
    d3->setName(c->getName() + "_d3");
    d3->setExecutionTimeOfScenario(*s, "d3", 0);
    d3->setDefaultProcessorType("d3");

    // connect the actors present on the source tile
    // by adding channels between them

    // channel src---->s1
    ChannelRef src_s1 = sg->createChannel(c->getSrcActor(), s1);
    src_s1->setName(c->getName() + "-src_s1");
    src_s1->getSrcPort()->setRateOfScenario(*s, c->getSrcPort()->getRateOfScenario(*s));
    src_s1->getDstPort()->setRateOfScenario(*s, 1);
    src_s1->setInitialAndFinalTokens(c->getInitialTokens());

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < src_s1->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += src_s1->getName();
    }
    src_s1->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Channel s1----->s1
    ChannelRef s1_s1 = sg->createChannel(s1, s1);
    s1_s1->setName(c->getName() + "-s1_s1");
    s1_s1->getSrcPort()->setRateOfScenario(*s, 1);
    s1_s1->getDstPort()->setRateOfScenario(*s, 1);
    s1_s1->setInitialAndFinalTokens(1);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < s1_s1->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += s1_s1->getName();
    }
    s1_s1->setPersistentAndFinalTokenNames(persistentTokenNames);

    // channel s1----->s2
    ChannelRef s1_s2 = sg->createChannel(s1, s2);
    s1_s2->setName(c->getName() + "-s1_s2");
    s1_s2->getSrcPort()->setRateOfScenario(*s, N);
    s1_s2->getDstPort()->setRateOfScenario(*s, 1);
    s1_s2->setInitialAndFinalTokens(0);

    // channel s2----->s3
    ChannelRef s2_s3 = sg->createChannel(s2, s3);
    s2_s3->setName(c->getName() + "-s2_s3");
    s2_s3->getSrcPort()->setRateOfScenario(*s, 1);
    s2_s3->getDstPort()->setRateOfScenario(*s, N);
    s2_s3->setInitialAndFinalTokens(0);

    // channel s3----->s1
    ChannelRef s3_s1 = sg->createChannel(s3, s1);
    s3_s1->setName(c->getName() + "-s3_s1");
    s3_s1->getSrcPort()->setRateOfScenario(*s, 1);
    s3_s1->getDstPort()->setRateOfScenario(*s, 1);
    s3_s1->setInitialAndFinalTokens(1);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < s3_s1->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += s3_s1->getName();
    }
    s3_s1->setPersistentAndFinalTokenNames(persistentTokenNames);

    // create channels to model the behaviour
    // of source buffer
    ChannelRef channel_for_src_buffer = sg->createChannel(s3, c->getSrcActor());
    channel_for_src_buffer->setName(c->getName() + "-channel_for_src_buffer");
    channel_for_src_buffer->getSrcPort()->setRateOfScenario(*s, 1);
    channel_for_src_buffer->getDstPort()->setRateOfScenario(*s,
                                                            c->getSrcPort()->getRateOfScenario(*s));

    ASSERT(b[(int)BufferLocation::Value::Src] >= c->getInitialTokens(),
           "There is not enough space in the input buffer to fit the initial tokens.");

    // cannot understand this computation from the compsoc implementation
    channel_for_src_buffer->setInitialAndFinalTokens(b[static_cast<int>(BufferLocation::Value::Src)]
                                                     - c->getInitialTokens());

    // channel_for_src_buffer->setStorageSpaceChannel(c); TODO: check whether this statement is not
    // required???

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < channel_for_src_buffer->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += channel_for_src_buffer->getName();
    }
    channel_for_src_buffer->setPersistentAndFinalTokenNames(persistentTokenNames);

    // connect the actors modeling the interconnect
    // s2----->c1
    ChannelRef s2_c1 = sg->createChannel(s2, c1);
    s2_c1->setName(c->getName() + "-s2_c1");
    s2_c1->getSrcPort()->setRateOfScenario(*s, 1);
    s2_c1->getDstPort()->setRateOfScenario(*s, 1);
    s2_c1->setInitialAndFinalTokens(0);

    // c1----->c1
    ChannelRef c1_c1 = sg->createChannel(c1, c1);
    c1_c1->setName(c->getName() + "-c1_c1");
    c1_c1->getSrcPort()->setRateOfScenario(*s, 1);
    c1_c1->getDstPort()->setRateOfScenario(*s, 1);
    c1_c1->setInitialAndFinalTokens(1);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < c1_c1->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += c1_c1->getName();
    }
    c1_c1->setPersistentAndFinalTokenNames(persistentTokenNames);

    // c1------>c2
    ChannelRef c1_c2 = sg->createChannel(c1, c2);
    c1_c2->setName(c->getName() + "-c1_c2");
    c1_c2->getSrcPort()->setRateOfScenario(*s, 1);
    c1_c2->getDstPort()->setRateOfScenario(*s, 1);
    c1_c2->setInitialAndFinalTokens(0);

    // c2---->c1
    ChannelRef c2_c1 = sg->createChannel(c2, c1);
    c2_c1->setName(c->getName() + "-c2_c1");
    c2_c1->getSrcPort()->setRateOfScenario(*s, 1);
    c2_c1->getDstPort()->setRateOfScenario(*s, 1);
    c2_c1->setInitialAndFinalTokens(w);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < c2_c1->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += c2_c1->getName();
    }
    c2_c1->setPersistentAndFinalTokenNames(persistentTokenNames);

    // c2---->d2
    ChannelRef c2_d2 = sg->createChannel(c2, d2);
    c2_d2->setName(c->getName() + "-c2_d2");
    c2_d2->getSrcPort()->setRateOfScenario(*s, 1);
    c2_d2->getDstPort()->setRateOfScenario(*s, 1);
    c2_d2->setInitialAndFinalTokens(0);

    // d2---->s2
    ChannelRef d2_s2 = sg->createChannel(d2, s2);
    d2_s2->setName(c->getName() + "-d2_s2");
    d2_s2->getSrcPort()->setRateOfScenario(*s, 1);
    d2_s2->getDstPort()->setRateOfScenario(*s, 1);
    d2_s2->setInitialAndFinalTokens(fifo_buffer_size);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < d2_s2->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += d2_s2->getName();
    }
    d2_s2->setPersistentAndFinalTokenNames(persistentTokenNames);

    // connect actors modeling the destination tile
    // d1----->d1
    ChannelRef d1_d1 = sg->createChannel(d1, d1);
    d1_d1->setName(c->getName() + "-d1_d1");
    d1_d1->getSrcPort()->setRateOfScenario(*s, 1);
    d1_d1->getDstPort()->setRateOfScenario(*s, 1);
    d1_d1->setInitialAndFinalTokens(1);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < d1_d1->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += d1_d1->getName();
    }
    d1_d1->setPersistentAndFinalTokenNames(persistentTokenNames);

    // d2----->d1
    ChannelRef d2_d1 = sg->createChannel(d2, d1);
    d2_d1->setName(c->getName() + "-d2_d1");
    d2_d1->getSrcPort()->setRateOfScenario(*s, 1);
    d2_d1->getDstPort()->setRateOfScenario(*s, N);
    d2_d1->setInitialAndFinalTokens(0);

    // d1----->d3
    ChannelRef d1_d3 = sg->createChannel(d1, d3);
    d1_d3->setName(c->getName() + "-d1_d3");
    d1_d3->getSrcPort()->setRateOfScenario(*s, 1);
    d1_d3->getDstPort()->setRateOfScenario(*s, 1);
    d1_d3->setInitialAndFinalTokens(1);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < d1_d3->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += d1_d3->getName();
    }
    d1_d3->setPersistentAndFinalTokenNames(persistentTokenNames);

    // d3----->d2
    ChannelRef d3_d2 = sg->createChannel(d3, d2);
    d3_d2->setName(c->getName() + "-d3_d2");
    d3_d2->getSrcPort()->setRateOfScenario(*s, N);
    d3_d2->getDstPort()->setRateOfScenario(*s, 1);
    d3_d2->setInitialAndFinalTokens(0);

    // d1----->dst
    ChannelRef d1_dst = sg->createChannel(d1, c->getDstActor());
    d1_dst->setName(c->getName() + "-d1_dst");
    d1_dst->getSrcPort()->setRateOfScenario(*s, 1);
    d1_dst->getDstPort()->setRateOfScenario(*s, c->getDstPort()->getRateOfScenario(*s));
    d1_dst->setInitialAndFinalTokens(0);

    // confirm this computation
    // create channel to model the destination buffer
    ChannelRef channel_for_dst_buffer = sg->createChannel(c->getDstActor(), d3);
    channel_for_dst_buffer->setName(c->getName() + "-channel_for_dst_buffer");
    channel_for_dst_buffer->getSrcPort()->setRateOfScenario(*s,
                                                            c->getDstPort()->getRateOfScenario(*s));
    channel_for_dst_buffer->getDstPort()->setRateOfScenario(*s, 1);
    channel_for_dst_buffer->setInitialAndFinalTokens(
            b[static_cast<int>(BufferLocation::Value::Dst)]);

    // not set???? channel_for_dst_buffer->setStorageSpaceChannel(c);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < channel_for_dst_buffer->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += channel_for_dst_buffer->getName();
    }
    channel_for_dst_buffer->setPersistentAndFinalTokenNames(persistentTokenNames);

    // model the FIFO channel
    ChannelRef channel_for_fifo_buffer = sg->createChannel(c->getSrcActor(), c->getDstActor());
    channel_for_fifo_buffer->setName(c->getName() + "-channel_for_fifo_buffer");
    channel_for_fifo_buffer->getSrcPort()->setRateOfScenario(*s,
                                                             c->getSrcPort()->getRateOfScenario(*s));
    channel_for_fifo_buffer->getDstPort()->setRateOfScenario(*s,
                                                             c->getDstPort()->getRateOfScenario(*s));
    channel_for_fifo_buffer->setInitialAndFinalTokens(c->getInitialTokens());
    // not modled???? channel_for_fifo_buffer->setStorageSpaceChannel(c);

    // create the persistent Token Names
    persistentTokenNames = "";
    for (uint i = 0; i < channel_for_fifo_buffer->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames += "PT-";
        persistentTokenNames += i;
        persistentTokenNames += "-";
        persistentTokenNames += channel_for_fifo_buffer->getName();
    }
    channel_for_fifo_buffer->setPersistentAndFinalTokenNames(persistentTokenNames);

    logInfo("Binding aware graph added!");
}

} // End namespace FSMSADF
