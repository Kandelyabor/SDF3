#!/usr/bin/env python

import os
import sys
import argparse
import re
import xml.etree.ElementTree as ET
import subprocess

class Actor(object):
    def __init__(self, name = 'unset', number = 0):
        self._fifos_prod_ = []
        self._fifos_cons_ = []
        self.name         = name
        self.number       = number
        self.type         = name

    def addConsFifo(self, x):
        self._fifos_cons_.append(x)

    def addProdFifo(self, x):
        self._fifos_prod_.append(x)

    def xmlPorts(self, fifoList, type, actor):
        for i, f in enumerate(fifoList):
            rate = f.dstRate
            if type == "out":
                rate = f.srcRate
            port = ET.Element('port', attrib = {'name': self.portName(f, i), 'type': type,'rate': str(rate)})
            actor.append(port)

    def xmlActor(self):
        actor = ET.Element('actor', attrib = {'name': self.name, 'type': self.type})

        self.xmlPorts(self._fifos_prod_, 'out', actor)
        self.xmlPorts(self._fifos_cons_, 'in', actor)
        return actor

    def xmlArguments(self, fifoList, argumentMapping):
        for i, f in enumerate(fifoList):
            arg = ET.Element('argument', attrib = {'port': self.portName(f, i), 'number': str(i)})
            argumentMapping.append(arg)

    def xmlProperties(self):
        actorProperties = ET.Element('actorProperties', attrib = {'actor': self.name})
        processor       = ET.Element('processor', attrib = {'type': 'microblaze', 'default': 'true'})
        implementation  = ET.Element('implementation')
        executionTime  = ET.Element('executionTime', attrib = {'time' : '10000'})
        imem  = ET.Element('memory', attrib = {'name' : '.code', 'size' : '4000', 'type': 'instr'})
        dmem  = ET.Element('memory', attrib = {'name' : '.data', 'size' : '4000', 'type': 'data'})
        function        = ET.Element('function',  attrib = {'symbol': '%s_func' % (self.name)})

        implementation.append(function)
        processor.append(implementation)
        processor.append(executionTime)
        processor.append(imem)
        processor.append(dmem)
        actorProperties.append(processor)
        return actorProperties

    def portName(self, fifo, portId):
        return "%s_%d" % (fifo.name, portId)

    def prodPortName(self, fifo):
        return self.portName(fifo, self._fifos_prod_.index(fifo))

    def consPortName(self, fifo):
        return self.portName(fifo, self._fifos_cons_.index(fifo))

class Fifo(object):
    def __init__(self, id, producer, consumer):
        self.producer       = producer
        self.consumer       = consumer
        self.name           = 'f%s%s%d' % (producer.name, consumer.name, id)
        self.initial_token  = []
        self.tokenSize      = 4
        self.srcRate        = 1
        self.dstRate        = 1
        producer.addProdFifo(self)
        consumer.addConsFifo(self)

    def xmlChannel(self):
        channel = ET.Element('channel', attrib = {
            'name'    : self.name,
            'srcActor': self.producer.name,
            'srcPort' : self.producer.prodPortName(self),
            'dstActor': self.consumer.name,
            'dstPort' : self.consumer.consPortName(self),
        })
        if self.initial_token:
            tokenString = ''
            channelName = ''
            tokenNr = 0
            for token in self.initial_token:
              tokenString += token
              channelName += token
              tokenString += ','
              tokenNr += 1
            channel.attrib['initialTokens'] = '%d' % (tokenNr-1)
            tokenString = tokenString[:-2]
            channel.attrib['persistentTokenNames'] = '%s' %(tokenString)
        channel.attrib['name'] = '%s' %(self.name)
        return channel

    def xmlChannelProperties(self):
        channelProperties = ET.Element('channelProperties', attrib = {'channel': self.name})
        tokenSize = ET.Element('tokenSize', attrib = {'sz': '%d' % self.tokenSize})
        channelProperties.append(tokenSize)
        return channelProperties

    def xmlTransition(self):
        scenarioTransition = ET.Element('transition', attrib = {'destination': 'q%d' % self.consumer.number})
        return scenarioTransition

class Unit(object):
    def __init__(self, key = 'unset', value = 'unset'):
        self.key         = key
        self.value       = value

class Graph(object):
    def __init__(self):
        self.actors = {}
        self.fifos  = []

    def getActor(self, name):
        """ Returns actor with the specified name. Creates a new actor if the actor wasn't defined yet """
        if name not in self.actors:
            self.actors[name] = Actor(name, len(self.actors)+1)
        return self.actors[name]

    def fifoConnection(self, id, a0, a1):
        """ Create a fifo connection between two actors """
        a0 = self.getActor(a0)
        a1 = self.getActor(a1)
        fifo = Fifo(id, a0, a1)
        self.fifos.append(fifo)
        return fifo

class Fsm(object):
    def __init__(self):
        self.initialName = 'unset'
        self.initialId = 'unset'
        self.detector = 'unset'
        self.graphs = []

    def addFsm(self, x, name, detect):
        self.graphs.append(x)
        self.initialName = name
        self.detector = detect

    def setFsmId(self, Id):
        self.initialId = Id

class Application(object):
    def __init__(self, x):
      self.name = 'smap'
      self.dftype = 'unset'
      self.graphs = []
      self.names = []
      self.fsm = x
      self.throughput = 'unset'
      self.units = []

    def addGraph(self, x, name):
        self.graphs.append(x)
        self.names.append(name)

    def setName(self, x):
        self.name = x

    def setType(self, x):
        self.dftype = x

    def setThroughput(self, value):
        self.throughput = value

    def addUnit(self, key, value):
        unit = Unit(key, value)
        self.units.append(unit)

    def xml(self, outFile):
        applicationGraph = ET.Element('applicationGraph', attrib = {'name': self.name, 'type': self.dftype})

        if self.dftype == 'sdf':
          csdfgraph = ET.Element('csdfgraph', attrib = {'name': self.names[0]})
          csdfProperties = ET.Element('csdfProperties')

          for key, actor in self.graphs[0].actors.items():
              csdfgraph.append(actor.xmlActor())
              csdfProperties.append(actor.xmlProperties())
          for fifo in self.graphs[0].fifos:
              csdfgraph.append(fifo.xmlChannel())
              csdfProperties.append(fifo.xmlChannelProperties())

          applicationGraph.append(csdfgraph)
          applicationGraph.append(csdfProperties)

        elif self.dftype == 'fsmsadf':
          fsmsadf = ET.Element('fsmsadf')
          applicationGraph.append(fsmsadf)
          fsmsadfProperties = ET.Element('fsmsadfProperties')

          graphProperties = ET.Element('graphProperties')
          timeConstraints  = ET.Element('timeConstraints')
          throughput       = ET.Element('throughput')
          throughput.text = self.throughput
          timeConstraints.append(throughput)
          graphProperties.append(timeConstraints)
          fsmsadfProperties.append(graphProperties)

          graphUnits = ET.Element('units')
          for i, unit in enumerate(self.units):
            newUnit = ET.Element(unit.key)
            newUnit.text = unit.value
            graphUnits.append(newUnit)

          fsmsadfProperties.append(graphUnits)

          scenarios = ET.Element('scenarios')
          for i, graph in enumerate(self.graphs):
            scenariograph = ET.Element('scenariograph', attrib = {'name': self.names[i], 'type': 'scenariograph'})
            scenarioProperties = ET.Element('scenario', attrib = {'name': self.names[i], 'graph': self.names[i], 'reward': '1'})
            for key, actor in graph.actors.items():
                scenariograph.append(actor.xmlActor())
                scenarioProperties.append(actor.xmlProperties())
            for fifo in graph.fifos:
                scenariograph.append(fifo.xmlChannel())
                scenarioProperties.append(fifo.xmlChannelProperties())

            fsmsadf.append(scenariograph)
            scenarios.append(scenarioProperties)

          fsmsadfProperties.append(scenarios)
          applicationGraph.append(fsmsadfProperties)

          fsmGraph = ET.Element('fsm')
          for key, actor in self.fsm.graphs[0].actors.items():
            scenarioTransition = ET.Element('state', attrib = {'name': 'q%d' % actor.number, 'scenario': actor.name})
            if self.fsm.initialName == actor.name:
                self.fsm.setFsmId('q%d' % actor.number)
            for fifo in self.fsm.graphs[0].fifos:
              if fifo.producer.name == actor.name:
                scenarioTransition.append(fifo.xmlTransition())
            fsmGraph.append(scenarioTransition)
          fsmGraph.attrib = {'initialstate': self.fsm.initialId, 'detector': self.fsm.detector}
          applicationGraph.append(fsmGraph)

        sdf3 = ET.Element('sdf3', attrib = {
            'type': self.dftype,
        })
        sdf3.append(applicationGraph)

        tree = ET.ElementTree(element = sdf3)
        tree.write(outFile, xml_declaration=True)

def parseDot(f):
    a = Application(Fsm())
    # parse lines
    name = re.compile(r"[\s]*name[\s]*=[\s]*(?P<n0>[-\w]+)[\s]*")
    dftype = re.compile(r"[\s]*dftype[\s]*=[\s]*(?P<d0>[-\w]+)[\s]*")
    throughput = re.compile(r"[\s]*throughput[\s]*=[\s]*(?P<t0>[-\w]+)[\s]*")
    units = re.compile(r"[\s]*(?P<u0>[-\w]+)[\s]*=[\s]*(?P<u1>[-\w\ ]+)[\s]*")
    digraph = re.compile(r"[\s]*digraph[\s]*(?P<g0>[-\w]+)[\s]*")
    edge = re.compile(r"[\s]*(?P<a0>[-\w]+)[\s]*->[\s]*(?P<a1>[-\w]+)[\s]*")
    fsm = re.compile(r"[\s]*fsm[\s]*\[initial=(?P<i0>[-\w]+)[\s]*detector=(?P<i1>[-\w]+)[\s]*\][\s]*")
    transition = re.compile(r"[\s]*(?P<s0>[-\w]+)[\s]*->[\s]*(?P<s1>[-\w]+)[\s]*")
    closure = re.compile(r"[\s]*}\s]")
  
    inGraph = 0
    inFsm = 0
    for i, line in enumerate(f):
        nameMatch = re.match(name, line)
        dftypeMatch = re.match(dftype, line)
        throughputMatch = re.match(throughput, line)
        unitsMatch = re.match(units, line)
        digraphMatch = re.match(digraph, line)
        fsmMatch = re.match(fsm, line)
        closureMatch = re.match(closure, line)
        if nameMatch:
          nameName = nameMatch.groupdict()
          a.setName(nameName['n0'])
        elif dftypeMatch:
          dftypeName = dftypeMatch.groupdict()
          a.setType(dftypeName['d0'])
        elif throughputMatch:
          throughputName = throughputMatch.groupdict()
          a.setThroughput(throughputName['t0'])
        elif unitsMatch:
          unitsName = unitsMatch.groupdict()
          a.addUnit(unitsName['u0'], unitsName['u1'])
        elif digraphMatch:
          graphName = digraphMatch.groupdict()
          a.addGraph(Graph(), graphName['g0'])
          inGraph = 1
        elif fsmMatch:
          initialScenario = fsmMatch.groupdict()
          a.fsm.addFsm(Graph(), initialScenario['i0'], initialScenario['i1'])
          inFsm = 1
        elif closureMatch:
          inGraph = 0
          inFsm = 0
        elif inFsm == 1:
          transitionMatch = re.match(transition, line)
          if not transitionMatch:
            continue
          scenarioNames = transitionMatch.groupdict()
          fifo = a.fsm.graphs[0].fifoConnection(i+1, scenarioNames['s0'], scenarioNames['s1'])
        elif inGraph == 1:
          edgeMatch = re.match(edge, line)
          if not edgeMatch:
            continue
          actorNames = edgeMatch.groupdict()

          fifo = a.graphs[len(a.graphs)-1].fifoConnection(i+1, actorNames['a0'], actorNames['a1'])

          arguments = re.findall(r"\[(.*)\]", line)
          if len(arguments) == 1:
              for item in arguments[0].split():
                  options = re.findall(r"[ \t]*(.*)[ \t]*=[ \t]*(.*)[ \t]*", item)
                  if len(options) == 1 and options[0][0] == 'initialTokens':
                      tokens = re.findall(r"[ \t]*(\w*),?[ \t]*", options[0][1])
                      for token in tokens:
                        fifo.initial_token.append(token)
                  elif len(options) == 1 and options[0][0] == 'dstRate':
                      fifo.dstRate = int(options[0][1])
                  elif len(options) == 1 and options[0][0] == 'srcRate':
                      fifo.srcRate = int(options[0][1])
                  elif len(options) == 1 and options[0][0] == 'tokenSize':
                      fifo.tokenSize = int(options[0][1])
                  elif len(options) == 1 and options[0][0] == 'channelName':
                      fifo.name = options[0][1]
    return a

def parseArguments():
    parser = argparse.ArgumentParser(description="""Convert a simple SDF description into CompSOC xml""")
    parser.add_argument('--input',  dest='input',  default=sys.stdin,  type=argparse.FileType('r'), help='Input file')
    try:
        # python > 2.7 distinguishes strings and binary data.
        # We need the underlying binary stdout buffer for the xml tree writer.
        if sys.stdout.buffer != None:
            stdoutBinary = sys.stdout.buffer
    except:
        stdoutBinary = sys.stdout

    parser.add_argument('--output', dest='output', default=stdoutBinary, type=argparse.FileType('wb'), help='Output file')
    return parser.parse_args()

def main():
    args = parseArguments()
    a = parseDot(args.input)
    a.xml(args.output)

if __name__ == '__main__':
    main()
