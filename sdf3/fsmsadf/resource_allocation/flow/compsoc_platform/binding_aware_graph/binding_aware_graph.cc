/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 15, 2011
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      15-04-11    :   Initial version.
 *
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
#include "fsmsadf/transform/dsm_encode/dsm_encode.h"
#include <memory>

namespace FSMSADF {
/**
 * BindingAwareGraph()
 * Constructor.
 */
BindingAwareGraphCompSoCPlatform::BindingAwareGraphCompSoCPlatform(const PlatformBinding *pb) :
    BindingAwareGraphBase(pb) {}

/**
 * ~BindingAwareGraph()
 * Destructor.
 */
BindingAwareGraphCompSoCPlatform::~BindingAwareGraphCompSoCPlatform() = default;

/**
 * hasNonZeroNonWcActors()
 * Helper function that decides if there are worst-case actors left with an
 * execution time larger than zero.
 */
bool BindingAwareGraphCompSoCPlatform::hasNonZeroNonWcActors(ScenarioGraph *sg,
                                                             Scenario *s,
                                                             ActorRefs wcActors) {
    for (const auto& a : sg->getActors()) {
        Processor *pa = this->getBindingAwarePropertiesOfScenarioGraph(sg).getProcessorMappedToActor(a.get());
        if ((std::find(wcActors.begin(), wcActors.end(), a.get()) == wcActors.end())
            && (a->getExecutionTimeOfScenario(*s, pa->getType()) != 0)) {
            return true;
        }
    }
    return false;
}

/**
 * Custom initialization of the worst-case actors.
 * Instead of assuming the worst-case hit on every actor, we add the ET of each actor
 * that does not have an incoming channel from another tile to a actor in front of it.
 * This does not change the analysis outcome, and is less negative that the default.
 */
void BindingAwareGraphCompSoCPlatform::initWorstCaseActors() {
    for (const auto& sg : getScenarioGraphs()) {
        // unique since a graph with isolated scenario is used).
        Scenario *s = getScenarioOfScenarioGraph(*sg);

        ActorRefs wcActors;

        for (const auto& a : sg->getActors()) {
            if (a->isDetectorActor()) {
                wcActors.push_back(a.get());
            }

            for (const auto& p : a->getPorts()) {
                if (p->getType() == Port::PortType::In) {
                    ActorRef srcActor = sg->getActor(p->getChannel()->getSrcActor()->getName());
                    // Actor * dstActor = sg->getActor( p->getChannel()->getDstActor()->getName() );

                    ProcessorBinding* srcPb =
                            this->getPlatformBinding()->getProcessorBindingOfActorNameInScenario(
                                    *s, srcActor->getName());
                    if (srcPb == nullptr) {
                        throw CException("could not find srcPb");
                    }
                    Processor *srcProc = srcPb->getProcessor();

                    ProcessorBinding* dstPb =
                            this->getPlatformBinding()->getProcessorBindingOfActorNameInScenario(*s,
                                                                                      a->getName());
                    if (dstPb == nullptr) {
                        throw CException("could not find dstPb");
                    }
                    Processor *dstProc = dstPb->getProcessor();
                    if (srcProc != dstProc) {
                        wcActors.push_back(a.get());
                        break;
                    }
                }
            }
        }

        while (hasNonZeroNonWcActors(sg.get(), s, wcActors)) {
            for (const auto& a : sg->getActors()) {
                Processor *pa = this->getBindingAwarePropertiesOfScenarioGraph(sg.get()).getProcessorMappedToActor(a.get());

                if ((std::find(wcActors.begin(), wcActors.end(), a.get()) == wcActors.end())
                    && (a->getExecutionTimeOfScenario(*s, pa->getType()) != 0)) {
                    Time ownET = a->getExecutionTimeOfScenario(*s, pa->getType());
                    for (auto iter = a->getPorts().begin();
                         iter != a->getPorts().end();
                         iter++) {
                        Port* p = iter->get();
                        if (p->getType() == Port::PortType::In) {
                            Actor* srcActor =
                                    sg->getActor(p->getChannel()->getSrcActor()->getName());
                            Processor *ps =
                                    this->getBindingAwarePropertiesOfScenarioGraph(sg.get()).getProcessorMappedToActor(srcActor);

                            Time srcET = srcActor->getExecutionTimeOfScenario(*s, ps->getType());
                            srcActor->setExecutionTimeOfScenario(*s, ps->getType(), srcET + ownET);

                            a->setExecutionTimeOfScenario(*s, pa->getType(), 0);
                            break;
                        }
                    }
                }
            }
        }
    }
}

/**
 * Model the binding of an actor to a tile inside the binding-aware graph.
 * @param scenario s
 * @param actor a
 * @param processor p
 */
void BindingAwareGraphCompSoCPlatform::modelActorToTileBinding(Actor *a,
                                                               Processor *p,
                                                               Scenario *s) {
    // Scenario graph
    ScenarioGraph *sg = a->getScenarioGraph();

    Time execTime = a->getExecutionTimeOfScenario(*s, p->getType());

    // Compute WCRT when using TDMA
    Time slotSize = p->getContextSwitchOverhead() + p->getSlotLength();
    Time tdmaTimeSlice = this->getBindingAwarePropertiesOfScenarioGraph(sg).getTimesliceMappedToProcessors(p);
    Time effectiveTimeSlice =
            tdmaTimeSlice - ((tdmaTimeSlice / slotSize) * p->getContextSwitchOverhead());

    Time waitingTime = p->getWheelsize() - tdmaTimeSlice;
    Time wcrt = 0;

    if (tdmaTimeSlice == 0 && execTime != 0) {
        wcrt = TIME_MAX;
    } else {
        while (execTime > 0) {
            wcrt = wcrt + waitingTime;

            if (execTime < effectiveTimeSlice) {
                wcrt = wcrt + execTime;
                execTime = 0;
            } else {
                wcrt = wcrt + tdmaTimeSlice;
                execTime = execTime - effectiveTimeSlice;
            }
        }
    }

    // Set the execution time of actor a on processor 'wcrt' as its worst-case
    // response time
    a->setExecutionTimeOfScenario(*s, "wcrt", wcrt);
    a->setDefaultProcessorType("wcrt");

    // Add self-loop with one token
    ChannelRef c = a->getScenarioGraph()->createChannel(a, a);
    c->setName(a->getName() + "-selfedge");
    c->getSrcPort()->setRateOfScenario(*s, 1);
    c->getDstPort()->setRateOfScenario(*s, 1);
    c->setInitialAndFinalTokens(1);
    c->setPersistentAndFinalTokenNames(CString("PT-") + c->getName());
}

/**
 * The function models a channel mapped to a connection.
 *
 */
void BindingAwareGraphCompSoCPlatform::modelChannelToConnectionBinding(
        Channel *c, ChannelBindingConstraint *cbc, Connection *co, Scenario *s) {
    if (cbc->hasBufferLocationConstraint()
        && (cbc->getBufferLocation() == BufferLocation::Value::Mem)) {
        insertSharedMemoryConnectionModel(c, cbc, co, s);
    } else {
        insertTileToTileConnectionModel(c, cbc, co, s);
    }
}

/**
 * The function models a channel mapped to a connection, with the buffer
    and the read/write counters mapped in the tiles memories.
 *
 */
void BindingAwareGraphCompSoCPlatform::insertTileToTileConnectionModel(
        Channel *c, ChannelBindingConstraint *cbc, Connection *co, Scenario *s) {
    // Scenario graph
    ScenarioGraph *sg = c->getScenarioGraph();

    // Buffer size of channel c
    BufferSize bufferSize = cbc->getBufferSize();

    // Src and dst actors
    Actor *src = c->getSrcActor();
    Actor *dst = c->getDstActor();

    // Rates
    Rate srcRate = c->getSrcPort()->getRateOfScenario(*s);
    Rate dstRate = c->getDstPort()->getRateOfScenario(*s);

    // Processor to which destination actor of channel c is bound
    Processor *srcProc = this->getBindingAwarePropertiesOfScenarioGraph(sg).getProcessorMappedToActor(src);
    Processor *dstProc = this->getBindingAwarePropertiesOfScenarioGraph(sg).getProcessorMappedToActor(dst);

    // Execution times of the CompSoC model actors:
    Time dmaLatency = 15; // NOLINT(*magic-numbers)
    Time nocLatency = co->getLatency();

    // CoMik latency based on thesis Nelson2014 section 2.3.7
    Time srcTableLength = srcProc->getWheelsize();
    Time srcService = this->getBindingAwarePropertiesOfScenarioGraph(sg).getTimesliceMappedToProcessors(srcProc)
                      - srcProc->getContextSwitchOverhead();
    Time srcWcrt = srcTableLength - srcService + 1;
    Time srcComikRateInv = srcTableLength / srcService;
    Time srcComikLatency = srcWcrt - srcComikRateInv;

    Time dstTableLength = dstProc->getWheelsize();
    Time dstService = this->getBindingAwarePropertiesOfScenarioGraph(sg).getTimesliceMappedToProcessors(dstProc)
                      - dstProc->getContextSwitchOverhead();
    Time dstWcrt = dstTableLength - dstService + 1;
    Time dstComikRateInv = dstTableLength / dstService;
    Time dstComikLatency = dstWcrt - dstComikRateInv;

    // From producer to consumer:

    // Create an actor to model the dmaProduceWrite latency
    ActorRef dmaProdWrite = sg->createActor();
    dmaProdWrite->setName(src->getName() + "_dmaProduceWrite_" + CString(c->getSrcPort()->getId()));
    dmaProdWrite->setExecutionTimeOfScenario(*s, "dma-" + srcProc->getName(), dmaLatency);
    dmaProdWrite->setDefaultProcessorType("dma-" + srcProc->getName());

    // Add channel from source to dmaProduceWrite actor
    ChannelRef src_dmaProdWrite = sg->createChannel(src, dmaProdWrite);
    src_dmaProdWrite->setName(c->getName() + "-" + src->getName() + "-dmaProduceWrite");
    src_dmaProdWrite->getSrcPort()->setRateOfScenario(*s, srcRate);
    src_dmaProdWrite->getDstPort()->setRateOfScenario(*s, srcRate);
    src_dmaProdWrite->setInitialAndFinalTokens(0);

    // Create an actor to model the Comik latency at the producer side
    ActorRef comikProd = sg->createActor();
    comikProd->setName(c->getName() + "_comikProduce");
    comikProd->setExecutionTimeOfScenario(*s, "comik", srcComikLatency);
    comikProd->setDefaultProcessorType("comik");

    // Add channel from comikProduce to src actor
    ChannelRef comikProd_src = sg->createChannel(comikProd, src); // NOLINT(*suspicious-call-argument)
    comikProd_src->setName(c->getName() + "-comikProduce-" + src->getName());
    comikProd_src->getSrcPort()->setRateOfScenario(*s, srcRate);
    comikProd_src->getDstPort()->setRateOfScenario(*s, srcRate);
    comikProd_src->setInitialAndFinalTokens(0);

    // Add channel from dmaProduceWrite to comikProduce actor
    ChannelRef dmaProdWrite_comikProd = sg->createChannel(dmaProdWrite, comikProd);
    dmaProdWrite_comikProd->setName(c->getName() + "-dmaProduceWrite-comikProduce");
    dmaProdWrite_comikProd->getSrcPort()->setRateOfScenario(*s, srcRate);
    dmaProdWrite_comikProd->getDstPort()->setRateOfScenario(*s, srcRate);

    // Initial tokens reflect storage space
    if (bufferSize[static_cast<int>(BufferLocation::Value::Src)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    dmaProdWrite_comikProd->setInitialAndFinalTokens(bufferSize[static_cast<int>(BufferLocation::Value::Src)]);
    sg->reduceInitialTokens(*s, *dmaProdWrite_comikProd);

    CString persistentTokenNames = "";
    for (uint i = 0; i < dmaProdWrite_comikProd->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + dmaProdWrite_comikProd->getName();
    }
    dmaProdWrite_comikProd->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Create an actor to model the nocProduce latency
    ActorRef nocProd = sg->createActor();
    nocProd->setName(c->getName() + "_nocProduce");
    nocProd->setExecutionTimeOfScenario(*s, "noc", nocLatency);
    nocProd->setDefaultProcessorType("noc");

    // Add channel from dmaProdWriteuce to nocProduce actor
    ChannelRef dmaProdWrite_nocProd = sg->createChannel(dmaProdWrite, nocProd);
    dmaProdWrite_nocProd->setName(c->getName() + "-dmaProduceWrite-nocProduce");
    dmaProdWrite_nocProd->getSrcPort()->setRateOfScenario(*s, srcRate);
    dmaProdWrite_nocProd->getDstPort()->setRateOfScenario(*s, srcRate);
    dmaProdWrite_nocProd->setInitialAndFinalTokens(0);
    sg->reduceInitialTokens(*s, *dmaProdWrite_nocProd);

    // Create an actor to model the CoMik latency at the consumer side
    ActorRef comikCons = sg->createActor();
    comikCons->setName(c->getName() + "_comikConsume");
    comikCons->setExecutionTimeOfScenario(*s, "comik", dstComikLatency);
    comikCons->setDefaultProcessorType("comik");

    // Add channel from nocProduce to comikConsume actor
    ChannelRef nocProd_comikCons = sg->createChannel(nocProd, comikCons);
    nocProd_comikCons->setName(c->getName() + "-nocProduce-comikConsume");
    nocProd_comikCons->getSrcPort()->setRateOfScenario(*s, srcRate);
    nocProd_comikCons->getDstPort()->setRateOfScenario(*s, dstRate);
    // If there are any initial tokens on original channels, add them here
    nocProd_comikCons->setInitialAndFinalTokens(c->getInitialTokens());
    if (c->getInitialTokens() != 0U) {
        nocProd_comikCons->setPersistentAndFinalTokenNames(c->getPersistentTokenNames());
    }

    // Add channel from comikConsume to dst actor
    ChannelRef comik_prodDst = sg->createChannel(comikCons, dst);
    comik_prodDst->setName(c->getName() + "-comikConsume-" + dst->getName());
    comik_prodDst->getSrcPort()->setRateOfScenario(*s, dstRate);
    comik_prodDst->getDstPort()->setRateOfScenario(*s, dstRate);
    comik_prodDst->setInitialAndFinalTokens(0);

    // From consumer to producer:

    // Create an actor to model the dmaConsumeWrite latency
    ActorRef dmaConsWrite = sg->createActor();
    dmaConsWrite->setName(dst->getName() + "_dmaConsumeWrite_" + CString(c->getDstPort()->getId()));
    dmaConsWrite->setExecutionTimeOfScenario(*s, "dma-" + dstProc->getName(), dmaLatency);
    dmaConsWrite->setDefaultProcessorType("dma-" + dstProc->getName());

    // Add channel from dst to dmaConsumeWrite actor
    ChannelRef dst_dmaConsWrite = sg->createChannel(dst, dmaConsWrite); // NOLINT(*suspicious-call-argument)
    dst_dmaConsWrite->setName(c->getName() + "-" + dst->getName() + "-dmaConsumeWrite");
    dst_dmaConsWrite->getSrcPort()->setRateOfScenario(*s, dstRate);
    dst_dmaConsWrite->getDstPort()->setRateOfScenario(*s, dstRate);
    dst_dmaConsWrite->setInitialAndFinalTokens(0);

    // Create an actor to model the nocConsume latency
    ActorRef nocCons = sg->createActor();
    nocCons->setName(c->getName() + "_nocConsume");
    nocCons->setExecutionTimeOfScenario(*s, "noc", nocLatency);
    nocCons->setDefaultProcessorType("noc");

    // Add channel from dmaConsumeWrite to nocConsume actor
    ChannelRef dmaConsWrite_nocCons = sg->createChannel(dmaConsWrite, nocCons);
    dmaConsWrite_nocCons->setName(c->getName() + "-dmaConsumeWrite-nocConsume");
    dmaConsWrite_nocCons->getSrcPort()->setRateOfScenario(*s, dstRate);
    dmaConsWrite_nocCons->getDstPort()->setRateOfScenario(*s, dstRate);
    dmaConsWrite_nocCons->setInitialAndFinalTokens(0);

    // Add channel from nocCons to Comik Producer actor to model memory space
    ChannelRef nocCons_comikProd = sg->createChannel(nocCons, comikProd);
    nocCons_comikProd->setName(c->getName() + "-nocConsume-comikProduce");
    nocCons_comikProd->getSrcPort()->setRateOfScenario(*s, dstRate);
    nocCons_comikProd->getDstPort()->setRateOfScenario(*s, srcRate);

    // Initial tokens reflect storage space
    if (bufferSize[static_cast<int>(BufferLocation::Value::Mem)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    nocCons_comikProd->setInitialAndFinalTokens(
            bufferSize[static_cast<int>(BufferLocation::Value::Mem)] - c->getInitialTokens());
    sg->reduceInitialTokens(*s, *nocCons_comikProd);

    persistentTokenNames = "";
    for (uint i = 0; i < nocCons_comikProd->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + nocCons_comikProd->getName();
    }
    nocCons_comikProd->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Set repetition vectors of new actors
    RepetitionVector repetitionVector = s->computeRepetitionVector();
    dmaProdWrite->setRepetitionsOfScenario(*s, repetitionVector[dmaProdWrite->getId()]);
    comikProd->setRepetitionsOfScenario(*s, repetitionVector[comikProd->getId()]);
    nocProd->setRepetitionsOfScenario(*s, repetitionVector[nocProd->getId()]);
    comikCons->setRepetitionsOfScenario(*s, repetitionVector[comikCons->getId()]);
    dmaConsWrite->setRepetitionsOfScenario(*s, repetitionVector[dmaConsWrite->getId()]);
    nocCons->setRepetitionsOfScenario(*s, repetitionVector[nocCons->getId()]);
}

/**
 * The function models a channel mapped to a connection, with the buffer
    and read/write counters mapped in the shared memory.
 *
 */
void BindingAwareGraphCompSoCPlatform::insertSharedMemoryConnectionModel(
        Channel *c, ChannelBindingConstraint *cbc, Connection *co, Scenario *s) {
    // Scenario graph
    ScenarioGraph *sg = c->getScenarioGraph();

    // Buffer size of channel c
    BufferSize bufferSize = cbc->getBufferSize();

    // Src and dst actors
    Actor *src = c->getSrcActor();
    Actor *dst = c->getDstActor();

    // Rates
    Rate srcRate = c->getSrcPort()->getRateOfScenario(*s);
    Rate dstRate = c->getDstPort()->getRateOfScenario(*s);

    // Processor to which destination actor of channel c is bound
    Processor *srcProc = this->getBindingAwarePropertiesOfScenarioGraph(sg).getProcessorMappedToActor(src);
    Processor *dstProc = this->getBindingAwarePropertiesOfScenarioGraph(sg).getProcessorMappedToActor(dst);

    // Execution times of the CompSoC model actors:
    Time dmaLatency = 15; // NOLINT(*magic-numbers)
    Time nocLatency = co->getLatency();

    // CoMik latency based on thesis Nelson2014 section 2.3.7
    Time srcTableLength = srcProc->getWheelsize();
    Time srcService = this->getBindingAwarePropertiesOfScenarioGraph(sg).getTimesliceMappedToProcessors(srcProc)
                      - srcProc->getContextSwitchOverhead();
    Time srcWcrt = srcTableLength - srcService + 1;
    Time srcComikRateInv = srcTableLength / srcService;
    Time srcComikLatency = srcWcrt - srcComikRateInv;

    Time dstTableLength = dstProc->getWheelsize();
    Time dstService = this->getBindingAwarePropertiesOfScenarioGraph(sg).getTimesliceMappedToProcessors(dstProc)
                      - dstProc->getContextSwitchOverhead();
    Time dstWcrt = dstTableLength - dstService + 1;
    Time dstComikRateInv = dstTableLength / dstService;
    Time dstComikLatency = dstWcrt - dstComikRateInv;

    // From producer to consumer:

    // Create an actor to model the dmaProduceWrite latency
    ActorRef dmaProdWrite = sg->createActor();
    dmaProdWrite->setName(src->getName() + "_dmaProduceWrite_" + CString(c->getSrcPort()->getId()));
    dmaProdWrite->setExecutionTimeOfScenario(*s, "dma-" + srcProc->getName(), dmaLatency);
    dmaProdWrite->setDefaultProcessorType("dma-" + srcProc->getName());

    // Add channel from source to dmaProduceWrite actor
    ChannelRef src_dmaProdWrite = sg->createChannel(src, dmaProdWrite);
    src_dmaProdWrite->setName(c->getName() + "-" + src->getName() + "-dmaProduceWrite");
    src_dmaProdWrite->getSrcPort()->setRateOfScenario(*s, srcRate);
    src_dmaProdWrite->getDstPort()->setRateOfScenario(*s, srcRate);
    src_dmaProdWrite->setInitialAndFinalTokens(0);

    // Create an actor to model the Comik latency at the producer side
    ActorRef comikProd = sg->createActor();
    comikProd->setName(c->getName() + "_comikProduce");
    comikProd->setExecutionTimeOfScenario(*s, "comik", srcComikLatency);
    comikProd->setDefaultProcessorType("comik");

    // Add channel from comikProduce to src actor
    ChannelRef comikProd_src = sg->createChannel(comikProd, src); // NOLINT(*suspicious-call-argument)
    comikProd_src->setName(c->getName() + "-comikProduce-" + src->getName());
    comikProd_src->getSrcPort()->setRateOfScenario(*s, srcRate);
    comikProd_src->getDstPort()->setRateOfScenario(*s, srcRate);
    comikProd_src->setInitialAndFinalTokens(0);

    // Add channel from dmaProduceWrite to comikProduce actor
    ChannelRef dmaProdWrite_comikProd = sg->createChannel(dmaProdWrite, comikProd);
    dmaProdWrite_comikProd->setName(c->getName() + "-dmaProduceWrite-comikProduce");
    dmaProdWrite_comikProd->getSrcPort()->setRateOfScenario(*s, srcRate);
    dmaProdWrite_comikProd->getDstPort()->setRateOfScenario(*s, srcRate);
    // Initial tokens reflect storage space
    if (bufferSize[static_cast<int>(BufferLocation::Value::Src)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    dmaProdWrite_comikProd->setInitialAndFinalTokens(
            bufferSize[static_cast<int>(BufferLocation::Value::Src)]);
    sg->reduceInitialTokens(*s, *dmaProdWrite_comikProd);
    // Set token names
    CString persistentTokenNames = "";
    for (uint i = 0; i < dmaProdWrite_comikProd->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }
        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + dmaProdWrite_comikProd->getName();
    }
    dmaProdWrite_comikProd->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Create an actor to model the nocProduce latency
    ActorRef nocProd = sg->createActor();
    nocProd->setName(c->getName() + "_nocProduce");
    nocProd->setExecutionTimeOfScenario(*s, "noc", nocLatency);
    nocProd->setDefaultProcessorType("noc");

    // Add channel from dmaProduceWrite to nocProduce actor
    ChannelRef dmaProdWrite_nocProd = sg->createChannel(dmaProdWrite, nocProd);
    dmaProdWrite_nocProd->setName(c->getName() + "-dmaProduceWrite-nocProduce");
    dmaProdWrite_nocProd->getSrcPort()->setRateOfScenario(*s, srcRate);
    dmaProdWrite_nocProd->getDstPort()->setRateOfScenario(*s, srcRate);
    dmaProdWrite_nocProd->setInitialAndFinalTokens(0);

    // Create an actor to model the dmaConsumeRead
    ActorRef dmaConsRead = sg->createActor();
    dmaConsRead->setName(c->getName() + "_dmaConsumeRead_" + CString(c->getDstPort()->getId()));
    dmaConsRead->setExecutionTimeOfScenario(*s, "dma-" + dstProc->getName(), dmaLatency);
    dmaConsRead->setDefaultProcessorType("dma-" + dstProc->getName());

    // Add channel from nocProduce to dmaConsumeRead actor
    ChannelRef nocProd_dmaConsRead = sg->createChannel(nocProd, dmaConsRead);
    nocProd_dmaConsRead->setName(c->getName() + "-nocProduce-dmaConsumeRead");
    nocProd_dmaConsRead->getSrcPort()->setRateOfScenario(*s, srcRate);
    nocProd_dmaConsRead->getDstPort()->setRateOfScenario(*s, dstRate);
    // If there are any initial tokens on original channels, add them here
    nocProd_dmaConsRead->setInitialAndFinalTokens(c->getInitialTokens());
    if (c->getInitialTokens() != 0U) {
        nocProd_dmaConsRead->setPersistentAndFinalTokenNames(c->getPersistentTokenNames());
    }

    // Create an actor to model the CoMik latency at the consumer side
    ActorRef comikCons = sg->createActor();
    comikCons->setName(c->getName() + "_comikConsume");
    comikCons->setExecutionTimeOfScenario(*s, "comik", dstComikLatency);
    comikCons->setDefaultProcessorType("comik");

    // Add channel from dmaConsumeRead to comikConsume actor
    ChannelRef dmaConsRead_comikCons = sg->createChannel(dmaConsRead, comikCons);
    dmaConsRead_comikCons->setName(c->getName() + "-dmaConsumeRead-comikConsume");
    dmaConsRead_comikCons->getSrcPort()->setRateOfScenario(*s, dstRate);
    dmaConsRead_comikCons->getDstPort()->setRateOfScenario(*s, dstRate);
    dmaConsRead_comikCons->setInitialAndFinalTokens(0);

    // Add channel from comikConsume to dst actor
    ChannelRef comik_prodDst = sg->createChannel(comikCons, dst);
    comik_prodDst->setName(c->getName() + "-comikConsume-" + dst->getName());
    comik_prodDst->getSrcPort()->setRateOfScenario(*s, dstRate);
    comik_prodDst->getDstPort()->setRateOfScenario(*s, dstRate);
    comik_prodDst->setInitialAndFinalTokens(0);

    // From consumer to producer:

    // Add channel from dst to dmaConsumeRead actor
    ChannelRef dst_dmaConsRead = sg->createChannel(dst, dmaConsRead); // NOLINT(*suspicious-call-argument)
    dst_dmaConsRead->setName(c->getName() + "-" + dst->getName() + "-dmaConsumeRead");
    dst_dmaConsRead->getSrcPort()->setRateOfScenario(*s, dstRate);
    dst_dmaConsRead->getDstPort()->setRateOfScenario(*s, dstRate);
    // Initial tokens reflect storage space
    if (bufferSize[static_cast<int>(BufferLocation::Value::Dst)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    dst_dmaConsRead->setInitialAndFinalTokens(
            bufferSize[static_cast<int>(BufferLocation::Value::Dst)]);
    sg->reduceInitialTokens(*s, *dst_dmaConsRead);
    // Set token names
    persistentTokenNames = "";
    for (uint i = 0; i < dst_dmaConsRead->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }
        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + dst_dmaConsRead->getName();
    }
    dst_dmaConsRead->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Create an actor to model the dmaConsumeWrite latency
    ActorRef dmaConsWrite = sg->createActor();
    dmaConsWrite->setName(dst->getName() + "_dmaConsumeWrite_" + CString(c->getDstPort()->getId()));
    dmaConsWrite->setExecutionTimeOfScenario(*s, "dma-" + dstProc->getName(), dmaLatency);
    dmaConsWrite->setDefaultProcessorType("dma-" + dstProc->getName());

    // Add channel from dst to dmaConsumeWrite actor
    ChannelRef dst_dmaConsWrite = sg->createChannel(dst, dmaConsWrite); // NOLINT(*suspicious-call-argument)
    dst_dmaConsWrite->setName(c->getName() + "-" + dst->getName() + "-dmaConsumeWrite");
    dst_dmaConsWrite->getSrcPort()->setRateOfScenario(*s, dstRate);
    dst_dmaConsWrite->getDstPort()->setRateOfScenario(*s, dstRate);
    dst_dmaConsWrite->setInitialAndFinalTokens(0);

    // Create an actor to model the nocConsume latency
    ActorRef nocCons = sg->createActor();
    nocCons->setName(c->getName() + "_nocConsume");
    nocCons->setExecutionTimeOfScenario(*s, "noc", nocLatency);
    nocCons->setDefaultProcessorType("noc");

    // Add channel from dmaConsumeWrite to nocConsume actor
    ChannelRef dmaConsWrite_nocCons = sg->createChannel(dmaConsWrite, nocCons);
    dmaConsWrite_nocCons->setName(c->getName() + "-dmaConsumeWrite-nocConsume");
    dmaConsWrite_nocCons->getSrcPort()->setRateOfScenario(*s, dstRate);
    dmaConsWrite_nocCons->getDstPort()->setRateOfScenario(*s, dstRate);
    dmaConsWrite_nocCons->setInitialAndFinalTokens(0);

    // Create an actor to model the dmaProduceRead latency
    ActorRef dmaProdRead = sg->createActor();
    dmaProdRead->setName(dst->getName() + "_dmaProduceRead_" + CString(c->getDstPort()->getId()));
    dmaProdRead->setExecutionTimeOfScenario(*s, "dma-" + dstProc->getName(), dmaLatency);
    dmaProdRead->setDefaultProcessorType("dma-" + dstProc->getName());

    // Add channel from nocCons to dmaProduceRead actor to model memory space
    ChannelRef nocCons_dmaProdRead = sg->createChannel(nocCons, comikProd);
    nocCons_dmaProdRead->setName(c->getName() + "-nocConsume-dmaProduceRead");
    nocCons_dmaProdRead->getSrcPort()->setRateOfScenario(*s, dstRate);
    nocCons_dmaProdRead->getDstPort()->setRateOfScenario(*s, srcRate);
    // Initial tokens reflect storage space
    if (bufferSize[static_cast<int>(BufferLocation::Value::Mem)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    nocCons_dmaProdRead->setInitialAndFinalTokens(
            bufferSize[static_cast<int>(BufferLocation::Value::Mem)] - c->getInitialTokens());
    sg->reduceInitialTokens(*s, *nocCons_dmaProdRead);
    // Set token names
    persistentTokenNames = "";
    for (uint i = 0; i < nocCons_dmaProdRead->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }
        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + nocCons_dmaProdRead->getName();
    }
    nocCons_dmaProdRead->setPersistentAndFinalTokenNames(persistentTokenNames);

    // Add channel from dmaProduceRead to Comik Producer actor to model memory space
    ChannelRef dmaProdRead_comikProd = sg->createChannel(dmaProdRead, comikProd);
    dmaProdRead_comikProd->setName(c->getName() + "-dmaProduceRead -comikProduce");
    dmaProdRead_comikProd->getSrcPort()->setRateOfScenario(*s, srcRate);
    dmaProdRead_comikProd->getDstPort()->setRateOfScenario(*s, srcRate);

    // Set repetition vectors of new actors
    RepetitionVector repetitionVector = s->computeRepetitionVector();
    dmaProdWrite->setRepetitionsOfScenario(*s, repetitionVector[dmaProdWrite->getId()]);
    comikProd->setRepetitionsOfScenario(*s, repetitionVector[comikProd->getId()]);
    nocProd->setRepetitionsOfScenario(*s, repetitionVector[nocProd->getId()]);
    dmaConsRead->setRepetitionsOfScenario(*s, repetitionVector[dmaConsRead->getId()]);
    comikCons->setRepetitionsOfScenario(*s, repetitionVector[comikCons->getId()]);
    dmaConsWrite->setRepetitionsOfScenario(*s, repetitionVector[dmaConsWrite->getId()]);
    nocCons->setRepetitionsOfScenario(*s, repetitionVector[nocCons->getId()]);
    dmaProdRead->setRepetitionsOfScenario(*s, repetitionVector[dmaProdRead->getId()]);
}

void BindingAwareGraphCompSoCPlatform::modelChannelToTileBinding(Channel *c,
                                                                 ChannelBindingConstraint *bc,
                                                                 Scenario *s) {
    if (c->getSrcActor() == c->getDstActor()) {
        return;
    }

    // Scenario graph
    ScenarioGraph *sg = c->getScenarioGraph();

    CString persistentTokenNames;

    BufferSize b = bc->getBufferSize();

    // Add channel from dst to src actor to model memory space
    ChannelRef bufferCapacity =
            c->getScenarioGraph()->createChannel(c->getDstActor(), c->getSrcActor());
    bufferCapacity->setName(c->getName() + "-buffer-capacity");
    bufferCapacity->getSrcPort()->setRateOfScenario(*s, c->getDstPort()->getRateOfScenario(*s));
    bufferCapacity->getDstPort()->setRateOfScenario(*s, c->getSrcPort()->getRateOfScenario(*s));

    // Initial tokens reflect storage space
    if (b[static_cast<int>(BufferLocation::Value::Mem)] < c->getInitialTokens()) {
        throw CException("Insufficient buffer space to store initial tokens.");
    }
    bufferCapacity->setInitialAndFinalTokens(b[static_cast<int>(BufferLocation::Value::Mem)]
                                             - c->getInitialTokens());
    sg->reduceInitialTokens(*s, *bufferCapacity);

    persistentTokenNames = "";
    for (uint i = 0; i < bufferCapacity->getInitialTokens(); i++) {
        if (!persistentTokenNames.empty()) {
            persistentTokenNames += ",";
        }

        persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
        persistentTokenNames = persistentTokenNames + CString("-");
        persistentTokenNames = persistentTokenNames + bufferCapacity->getName();
    }
    bufferCapacity->setPersistentAndFinalTokenNames(persistentTokenNames);
}

/**
 * Model the static-order schedules inside the binding-aware graph
 * one for the original graph ("scheduling"), and one for the actors modelling the
 * DMA transactions of each actor.
 */
void BindingAwareGraphCompSoCPlatform::modelStaticOrderScheduleOnProcessor(Scenario *s) {
    ScenarioGraph *oldSg = s->getScenarioGraph();
    std::unique_ptr<ScenarioGraph> newSg =
            oldSg->clone(GraphComponent(oldSg->getParent(), oldSg->getId(), oldSg->getName()));

    const PlatformBinding *b = getPlatformBinding();
    ScenarioRef sApp = b->getApplicationGraph()->getScenarioRef(s->getName());

    // Iterate over the tile bindings
    for (const auto& tb : b->getConstTileBindings()) {

        // Iterate over the processor bindings
        for (const auto& pb : tb->getProcessorBindings()) {

            StaticOrderSchedule sos = pb->getStaticOrderSchedule(*sApp);
            // If not detector, do not schedule switch and select actor
            if (newSg->getName() != "detector") {
                removeSwitchesAndSelects(&sos);
            }

            // Only encode schedule if one exists for this tile
            if (!sos.empty()) {
                // Encode regular schedule
                newSg = encodeSchedule(s, pb->getProcessor()->getName(), "scheduling", newSg.get(), sos);

                // Encode schedule of the DMA related actors for this processor
                std::unique_ptr<StaticOrderSchedule> dmaSchedule = extractDmaSchedule(oldSg, sos);
                if (dmaSchedule && !dmaSchedule->empty()) {
                    newSg = encodeSchedule(
                            s, pb->getProcessor()->getName(), "dmaSched", newSg.get(), *dmaSchedule);
                }
            }
        }
    }
    this->replaceScenarioGraph(oldSg, std::move(newSg));
}

/**
 * This function imposes an (simple) ordering of all DMA transactions belonging to an
 * actor, split in two sets for consuming and producing ports.
 * Such an ordering of DMA transactions of one actor is imposed by the POSE operating
 * system.
 */
void BindingAwareGraphCompSoCPlatform::interModelOrdering(Scenario *s, ScenarioGraph *sg) {
    // Iterate over all actors in the graph
    for (const auto& a: sg->getActors()) {
        std::list<CString> dmaActorNames;
        dmaActorNames.emplace_back("_dmaConsumeWrite_");
        dmaActorNames.emplace_back("_dmaProduceWrite_");
        dmaActorNames.emplace_back("_dmaConsumeRead_");
        dmaActorNames.emplace_back("_dmaProduceRead_");

        enforceDmaOrder(s, sg, a.get(), dmaActorNames);
    }
}

/**
 * This function iterates over the ports of an actor and enforces a simple order
 * between all ports connected to an actor with name "base+portId".
 * A back-edge from the last to the first actor is inserted if more than
 * one channel was inserted.
 */
void BindingAwareGraphCompSoCPlatform::enforceDmaOrder(Scenario *s,
                                                       ScenarioGraph *sg,
                                                       Actor *a,
                                                       std::list<CString>& dmaActorNames) {
    ActorRef firstActor = nullptr;
    ActorRef previousActor = nullptr;
    uint previousPortId = 0;

    // Iterate over the ports
    for (const auto& dmaActorName: dmaActorNames) {
        for (const auto& p: a->getPorts()) {
            CString actorName = dmaActorName + CString(p->getId());
            ;

            // and check if the corresponding DMA actor exists
            if (sg->hasActor(actorName)) {
                if ((previousActor != nullptr)) {
                    // Add channel from previous to current dma actor
                    ChannelRef dmaOrder = sg->createChannel(previousActor, sg->getActor(actorName));
                    dmaOrder->setName(actorName + "-" + CString(previousPortId));
                    dmaOrder->getSrcPort()->setRateOfScenario(*s, 1);
                    dmaOrder->getDstPort()->setRateOfScenario(*s, 1);
                    dmaOrder->setInitialAndFinalTokens(0);
                } else {
                    firstActor = sg->getActor(actorName);
                }
                previousActor = sg->getActor(actorName);
                previousPortId = p->getId();
            }
        }
    }

    // Insert back-edge if more than one ordering actor was inserted
    if ((firstActor != nullptr) && (firstActor != previousActor)) {
        // Add channel from last to first dmaConsumeWrite or dmaProduceWrite actor
        ChannelRef dmaOrder = sg->createChannel(previousActor, firstActor);
        dmaOrder->setName(firstActor->getName() + "_backEdge");
        dmaOrder->getSrcPort()->setRateOfScenario(*s, 1);
        dmaOrder->getDstPort()->setRateOfScenario(*s, 1);
        dmaOrder->setInitialAndFinalTokens(1);
        dmaOrder->setPersistentAndFinalTokenNames(CString("PT-") + dmaOrder->getName());
    }
}

/*
 * Return a schedule from which all actors starting with "switch_"
 * or "select_" are removed.
 */
void BindingAwareGraphCompSoCPlatform::removeSwitchesAndSelects(StaticOrderSchedule *sos) {
    auto i = sos->begin();
    while (i != sos->end()) {
        const Actor* a = i->actor;
        if ((a->getName().compare(0, 7, "switch_") == 0) // NOLINT(*magic-numbers)
            || (a->getName().compare(0, 7, "select_") == 0)) { // NOLINT(*magic-numbers)
            i = sos->erase(i);
        } else {
            i++;
        }
    }
}

/**
 * Compose the static order periodic schedule of actors modelling the DMA.
 * The schedule follows the original SOS, but now all actors modelling the
 * DMA consume and produce operations on each port replace the original actors.
 * Note that first all consumption operations must be finished,
 * then the original actor can fire, and then all production operations are performed.
 */
std::unique_ptr<StaticOrderSchedule> BindingAwareGraphCompSoCPlatform::extractDmaSchedule(ScenarioGraph *sg,
                                                                          StaticOrderSchedule& sos) {
    auto dmaSchedule = std::make_unique<StaticOrderSchedule>();

    std::list<CString> dmaActorNames;
    dmaActorNames.emplace_back("_dmaConsumeWrite_");
    dmaActorNames.emplace_back("_dmaProduceWrite_");
    dmaActorNames.emplace_back("_dmaConsumeRead_");
    dmaActorNames.emplace_back("_dmaProduceRead_");

    // For each actor in the original SOS
    for (const auto& i: sos) {
        const Actor* a = i.actor;

        // iterate over the ports
        for (const auto& p: a->getPorts()) {
            for (const auto& dmaActorName: dmaActorNames) {

                // and check if the corresponding DMA consume actor exists
                if (sg->hasActor(dmaActorName)) {
                    // Add dma actor to schedule
                    dmaSchedule->appendActor(sg->getActor(dmaActorName));
                }
            }
        }
    }
    return dmaSchedule;
}

} // End namespace FSMSADF
