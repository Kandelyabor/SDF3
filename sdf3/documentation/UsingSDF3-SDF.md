# Using SDF3 - SDF domain

The SDF domain is supported with the following executables, described separately below.

- [`sdf3analyze-sdf`](#analyze)
- [`sdf3flow-sdf`](#flow)
- [`sdf3generate-sdf`](#generate)
- [`sdf3print-sdf`](#print)
- [`sdf3transform-sdf`](#transform)

## Analyze

- `sdf3analyze-sdf`

``` text
Usage: sdf3analyze-sdf --graph <file> --algo <algorithm> [--output <file>]
   --graph  <file>     input SDF graph
   --output <file>     output file (default: stdout)
   --algo <algorithm>  analyze the graph with requested algorithm:
       consistency
       deadlock
       simple_cycles
       acyclic_graph
       connected_graph
       repetition_vector
       repetition_vector_sum
       strongly_connected_components
       mcm[(cycle,dasdan,karp,howard,yto,yto-mcr)]
       mpthroughput
       mpdelay
       mpeigen
       mpconverthsdf
       is_hsdf
       statistics
       throughput
       throughput-mpeigenvalue
       buffersize
       buffersize_ning_gao
       buffersize_capacity_constrained
       latency(method,srcActor,dstActor)
       binding_aware_throughput([NSoC,MPFlow])
       static_periodic_schedule
       static_periodic_schedule_chao
       k_periodic
```

### Algorithms

### consistency

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo consistency
```

``` sh
Graph is consistent.
```

### deadlock

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo deadlock
```

``` sh
Graph is deadlock free.
```

### simple_cycles

Report all simple cycles in the dataflow graph structure.

**Note that the algorithm may in fact not always report all simple cycles due to a known bug.**

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo simple_cycles
```

``` sh
Simple cycles:
Cycle: vld
Cycle: iq
Cycle: mc
```

### acyclic_graph

Report if the graph is acyclic (contains no cycles).

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo acyclic_graph
```

``` sh
Graph is acyclic.
```

### connected_graph

Report if the graph is connected.

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo connected_graph
```

``` sh
Graph is a connected graph.
```

### repetition_vector

Report the repetition vector of the graph, i.e., the smallest non-empty set of actor firings that brings the graph back to its initial state.

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo repetition_vector
```

``` sh
Repetition vector:
 [vld] = 1
 [iq] = 594
 [idct] = 594
 [mc] = 1
```

### repetition_vector_sum

Report the sum of the repetition vector of the graph.

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo repetition_vector_sum
```

``` sh
Repetition vector sum: 1190
```

### strongly_connected_components

Report the strongly connected components of the graph.

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo strongly_connected_components
```

``` sh
Strongly connected components:
Component: vld
Component: iq
Component: idct
Component: mc
```

### mcm[(cycle,dasdan,karp,howard,yto,yto-mcr)]

Determine the *maximum cycle mean* (mcm) of the (HSDF equivalent of the) graph.
The mcm is defined as the maximum over all cycles in the graph of the sum of the firing durations of the actors on the cycle divided by the total number of tokens on the channels of the cycle. Note that although in literature it is often called the maximum cycle mean, it is technically a *maximum cycle ratio*, i.e., the maximum ratio of execution time over number of tokens. The mcm or mcr is the reciprocal of the maximum throughput of the graph.

Six methods to compute the mcm are supported.

- cycle, the default method use if no method is explicitly specified. the mcm is determined by explicit enumeration of the simple cycles of the graph. **Note that due to a bug in the simple cycle enumeration it may not always report the correct result.**

Note that some of the methods require the graph to be strongly connected.

- dasdan
- karp
- howard
- yto
- yto-mcr

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo mcm
```

``` sh
MCM(h263decoder) = 332046
analysis time: 1390ms
```

### throughput

Compute throughput of the graph by constructing the state-space of the operational behavior.
This method requires that the graph uses integer execution times.

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo throughput
```

``` sh
thr(h263decoder) = 3.01163e-06
analysis time: 0ms
```

### mpthroughput

Compute throughput of the graph by constructing the state-space of (normalized) max-plus state vectors until a recurrent state is reached.

Note that some graphs may have an infinite state-space (diverging state vectors), in which case the algorithm may not terminate.
Optionally a maximum number of states can be provided as in the example below.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo 'mpthroughput(maxNumberOfStates=1000)'
```

``` sh
thr(satellite) = 0.00094697
analysis time: 15ms
```

### throughput-mpeigenvalue

Compute throughput of the graph by determining the state matrix of the graph and computing its largest eigenvalue.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo throughput-mpeigenvalue
```

``` sh
thr(satellite) = 0.00094697
analysis time: 515ms
```

### k_periodic

Compute throughput using the K-Periodic scheduling technique of Bodin, Munier-Kordon and de Dinechin.

> B. Bodin, A. Munier-Kordon and B. D. de Dinechin, "K-Periodic schedules for evaluating the maximum throughput of a Synchronous Dataflow graph," 2012 International Conference on Embedded Computer Systems (SAMOS), Samos, Greece, 2012, pp. 152-159, doi: 10.1109/SAMOS.2012.6404169.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo k_periodic
```

``` text
thr(satellite) = 0.00094697
analysis time: 15ms
```

### mpdelay

Determine the transient delay of the graph with respect to a periodic steady-state schedule specified in the graph's xml file.

``` sh
./sdf3analyze-sdf --graph ../examples/modem-mpdelay.xml --algo mpdelay
```

``` text
delay: -2
analysis time: 0ms
```

### mpeigen

Compute an eigenvalue and eigenvector of the graph from a cycle in the state-space of (normalized) max-plus state vectors.

Note that some graphs may have an infinite state-space (diverging state vectors), in which case the algorithm may not terminate.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo mpeigen
```

``` sh
eigenvalue: 1056
-258 ; -257 ; -256 ; -258 ; -257 ; -256 ; -255 ; -254 ; -253 ; -243 ; -255 ; -254 ; -253 ; -243 ; -242 ; -241 ; -241 ; -242 ; -242 ; -241 ; -240 ; 0 ;
analysis time: 15ms
```

### mpconverthsdf

Convert the SDF graph to an equivalent HSDF graph through its max-plus matrix, as described in:

> Marc Geilen. 2009. Reduction techniques for synchronous dataflow graphs. In Proceedings of the 46th Annual Design Automation Conference (DAC '09). Association for Computing Machinery, New York, NY, USA, 911–916. <https://doi.org/10.1145/1629911.1630146>

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo mpconverthsdf --output ./out.xml
```

### is_hsdf

Test if the graph is an HSDF graph.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo is_hsdf
```

``` sh
Graph is not a HSDF graph.
```

### statistics

Report statistics of the graph.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo statistics
```

``` text
#actors:          22
#channels:        48
#actors (HSDF):   4515
#channels (HSDF): 11619
```

### buffersize

Determine the Pareto-optimal trade-offs between throughput and buffer size and report as an xml file.

``` sh
./sdf3analyze-sdf --graph ../examples/satellite.xml --algo buffersize --output ./out.xml
```

### buffersize_ning_gao

Determine the minimum buffer size of an HSDF graph using the method of Ning and Gao.

> Qi Ning and Guang R. Gao. 1993. A novel framework of register allocation for software pipelining. In Proceedings of the 20th ACM SIGPLAN-SIGACT symposium on Principles of programming languages (POPL '93). Association for Computing Machinery, New York, NY, USA, 29–42. <https://doi.org/10.1145/158511.158519>

``` sh
./sdf3analyze-sdf --graph ../examples/simplehsdf.xml --algo buffersize_ning_gao
```

``` text
b(a) = 2
b(b) = 2
s(a) = 0
s(b) = 3
size =   4
period = 4
```

### buffersize_capacity_constrained

Generate buffer sizing throughput tradeoff space for capacity-constrained model.

``` sh
./sdf3analyze-sdf --graph ../examples/hsdf.xml --algo buffersize_capacity_constrained --output out.xml
```

### latency(method,srcActor,dstActor)

Determine the latency from `srcActor` to `dstActor`.
Method is one of the following: `st`, `sp`, `min`, `ro`, `min_st`

Based on the following paper:

> A. H. Ghamarian, S. Stuijk, T. Basten, M. C. W. Geilen and B. D. Theelen, "Latency Minimization for Synchronous Data Flow Graphs," 10th Euromicro Conference on Digital System Design Architectures, Methods and Tools (DSD 2007), Lubeck, Germany, 2007, pp. 189-196, doi: 10.1109/DSD.2007.4341468.

``` sh
./sdf3analyze-sdf --graph ../examples/h263decoder.xml --algo "latency(ro,vld,mc)"
```

``` text
latency(h263decoder) = 639218
analysis time: 187ms
```

### binding_aware_throughput([NSoC,MPFlow])

Requires a model with an architecture graph description typically produced by the flow.

**TODO: find example with non-zero throughput**

``` sh
./sdf3analyze-sdf --graph ../examples/sdf-flow/h263decoder_result.xml --algo 'binding_aware_throughput(MPFlow)'
```

``` text
thr(h263decoder) = 0
analysis time: 0ms
```

### static_periodic_schedule

Generate a static periodic schedule for a strongly connected graph.

``` sh
./sdf3analyze-sdf --graph ../examples/stronglyconnected.xml --algo static_periodic_schedule
```

``` text
period: 23
(fork1,0): 47
(biq,0): 48
(bi,0): 48
(add,0): 49
(ac,0): 50
(fork2,0): 51
(fork2,1): 52
(conj,0): 53
(mul1,0): 66
(in,0): 47
...
(filt,15): 63
(hil,0): 56
(hil,1): 64
(eq,0): 65
(mul2,0): 68
(deci,0): 67
(deco,0): 68
(out,0): 69
thr: 1/23
```

### static_periodic_schedule_chao

``` sh
./sdf3analyze-sdf --graph ../examples/stronglyconnected.xml --algo static_periodic_schedule_chao
```

``` text
period: 23
(fork1_0,0): 0
(biq_0,0): 1
(bi_0,0): 1
(add_0,0): 2
(ac_0,0): 3
(fork2_0,0): 4
(fork2_1,0): 5
(conj_0,0): 6
(mul1_0,0): 19
(in_0,0): 0
...
(filt_15,0): 16
(hil_0,0): 9
(hil_1,0): 17
(eq_0,0): 18
(mul2_0,0): 21
(deci_0,0): 20
(deco_0,0): 21
(out_0,0): 22
thr: 1/23
```

## Flow

`sdf3flow-sdf` implements the mapping and scheduling flow for SDF graphs as described in the following thesis.

> Predictable Mapping of Streaming Applications on Multiprocessors
S. Stuijk
Ph.D. Thesis, submitted to Faculty of Electrical Engineering, Eindhoven University of Technology, The Netherlands, 2007

From the directory `output/examples/sdf-flow` in the build directory, run the following command to executed the mapping and scheduling flow.

``` sh
../../bin/sdf3flow-sdf --settings ./sdf3.opt --html --output result.xml
```

## Generate

- `sdf3generate-sdf`

Usage example:

``` sh
./sdf3generate-sdf --settings ../examples/sdf-generate/sdf3.opt
```

SDF3 contains a generator to create random SDFGs with attributes (e.g., execution times, storage sizes, throughput constraint). The generated graphs are deadlock-free and consistent. The user specifies the following additional properties of the graph:

- weakly or strongly connected
- cyclic or acyclic
- rates, degree of actors and execution times within minimum and maximum bounds and with user-specified average and variance

When using the command-line tool sdf3generate-sdf, these settings are specified in a settings file. An example of such a file is shown below.

``` xml
<sdf3 type='sdf' version='1.0'
    xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>
    <settings type='generate'>
        <graph>
            <actors nr='10'/>
            <degree avg='2' var='1' min='1' max='5'/>
            <rate avg='2' var='1' min='1' max='5' repetitionVectorSum='10'/>
            <initialTokens prop='0'/>
            <structure stronglyConnected='false' acyclic='true' multigraph='true'/>
        </graph>
        <graphProperties>
            <procs nrTypes='3' mapChance='0.25'/>
            <execTime avg='10' var='0' min='10' max='10'/>
            <stateSize avg='1' var='1' min='1' max='1'/>
            <tokenSize avg='1' var='1' min='1' max='1'/>
            <bufferSize/>
            <bandwidthRequirement avg='2' var='0' min='1' max='4'/>
            <latencyRequirement avg='2' var='0' min='1' max='4'/>
            <throughputConstraint autoConcurrencyDegree='1' scaleFactor='0.1'/>
            <integerMCM/>
        </graphProperties>
    </settings>
</sdf3>
```

With the above settings, an acyclic multigraph SDFG is generated of which the sum of the repetition vector is equal to 10. The rates on the ports are typically between 1 and 5. However, the generator may use a rate outside this range when this is required to make the graph consistent. The actors, channels and the graph itself are assigned several properties (e.g. execution time, buffer size and a throughput constraint). The bounds for these values are specified in the graphProperties section of the settings. The element integerMCM ensures that the maximum cycle mean of the graph (when every actor is executed on its default processor) is integer valued.

A precise description of the XML format that is used in the settings file can be found in the [SDF3 XML file format reference guide](https://sstuijk.estue.nl/tools/sdf3/manuals/xml/sdf/).

## Print

``` sh
./sdf3print-sdf
```

``` text
SDF3 sdf3print-sdf (version 1.0)

Usage: sdf3print-sdf --graph <file> --format <type> [--output <file>]
   --graph  <file>     input SDF graph
   --output <file>     output file (default: stdout)
   --format <type>     output format:
       hapi
       dot
       txt
       buffy(outputActor(<value>))
       selfTimedSchedule
       buffyThroughput(outputActor(<value>))
       stateSpaceThroughput(outputActor(<value>))
       maxplusmatrix
       maxplusmatrixLatex
```

### HAPI

The following will generate source files for the HAPI simulation framework.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format hapi
```

### Graphviz Dot

The following will generate a representation of the graph in the [Graphviz](https://graphviz.org/) dot format.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format dot
```

### txt

The following will generate a representation of the graph in a human readable textual format.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format txt
```

### buffy(outputActor(\<value>))

The following will generate C code for the 'buffy' buffer sizing method using the selected output actors to determine which states to store.
It will produce the entire trade-off space between throughput and buffer sizes.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format "buffy(outputActor(w))"
```

### buffyThroughput(outputActor(\<value>))

The following will generate C code for the 'buffy' buffer sizing method using the selected output actors to determine which states to store.
This version only determines the minimum buffer sizes to achieve the maximal throughput.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format 'buffyThroughput(outputActor(w))'
```

### selfTimedSchedule

The following will generate C code that determines the self-timed schedule of the graph.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format selfTimedSchedule
```

### stateSpaceThroughput(outputActor(\<value>))

Generate C code for state-space-based throughput analyzer.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format 'stateSpaceThroughput(outputActor(w))'
```

### maxplusmatrix

Generate a textual representation of the max-plus state matrix of the graph.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format maxplusmatrix
```

``` text
[
1056 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1057 264 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1058 265 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1056 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1057 264 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1058 265 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1059 266 25 -Inf -Inf -Inf 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1060 267 26 -Inf -Inf -Inf 25 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1061 268 27 -Inf -Inf -Inf 26 25 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1071 278 244 -Inf -Inf -Inf 243 242 241 240 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1059 266 25 -Inf -Inf -Inf -Inf 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1060 267 26 -Inf -Inf -Inf -Inf 25 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1061 268 27 -Inf -Inf -Inf -Inf 26 25 24 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1071 278 244 -Inf -Inf -Inf -Inf 243 242 241 240 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1072 279 245 1072 279 245 244 243 242 241 244 243 242 241 240 -Inf -Inf -Inf -Inf -Inf -Inf -Inf ;
1073 280 246 1073 280 246 245 244 243 242 245 244 243 242 241 1 -Inf -Inf -Inf -Inf -Inf -Inf ;
1073 280 246 1073 280 246 245 244 243 242 245 244 243 242 241 -Inf 1 -Inf -Inf -Inf -Inf -Inf ;
-Inf -Inf -Inf 1072 279 245 -Inf -Inf -Inf -Inf 244 243 242 241 -Inf -Inf -Inf 240 -Inf -Inf -Inf -Inf ;
1072 279 245 -Inf -Inf -Inf 244 243 242 241 -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf 240 -Inf -Inf -Inf ;
1073 280 246 1073 280 246 245 244 243 242 245 244 243 242 -Inf -Inf -Inf 241 241 240 -Inf -Inf ;
1074 281 247 1074 281 247 246 245 244 243 246 245 244 243 -Inf -Inf -Inf 242 242 241 1 -Inf ;
1314 521 487 1314 521 487 486 485 484 483 486 485 484 483 481 241 241 482 482 481 241 240
]

State labels = [_ch27, _ch28, _ch29, _ch30, _ch31, _ch32, _ch33, _ch34, _ch35, _ch36, _ch37, _ch38, _ch39, _ch40, _ch41, _ch42, _ch43, _ch44, _ch45, _ch46, _ch47, _ch48]
```

### maxplusmatrixLatex

Generate a LaTeX code representation of the max-plus state matrix of the graph.

``` sh
./sdf3print-sdf --graph ../examples/satellite.xml --format maxplusmatrixLatex
```

``` text
\begin{ bmatrix }
_ch27\\
_ch28\\
...
_ch48\\
\end{bmatrix} =

\begin{bmatrix}
1056 &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} \\
1057 &264 &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} \\
1058 &265 &24 &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} &-\infty{} \\
...
1314 &521 &487 &1314 &521 &487 &486 &485 &484 &483 &486 &485 &484 &483 &481 &241 &241 &482 &482 &481 &241 &240 \\
\end{bmatrix}

\otimes \begin{ bmatrix }
_ch27\\
_ch28\\
_ch29\\
...
_ch48\\
\end{bmatrix}
```

## Transform

`sdf3transform-sdf` is used to transform SDF graphs into different types of graphs. Its usage is as follows.

``` text
SDF3 sdf3transform-sdf (version 1.0)

Usage: sdf3transform-sdf --graph <file> --transform <list> [--output <file>]
   --graph  <file>     input SDF graph
   --output <file>     output file (default: stdout)
   --transform <list>  comma separated list with graph transformations:
       to_hsdf
       unfold(<count>)
       model_buffersize
       model_autoconc(<max>)
       select_buffersize
       move_state
       capacity_constrained(<mcm>)
       retiming(<dip>) (without dip it will compute optimal retiming)
```

### to_hsdf

Transform a SDF graph to a HSDF graph.

``` sh
./sdf3transform-sdf --graph ../examples/satellite.xml --transform to_hsdf
```

### unfold(\<count>)

Unfold a HSDF graph for a given number of times.

``` sh
./sdf3transform-sdf --graph ../examples/hsdf.xml --transform 'unfold(3)'
```

### model_buffersize

Model the buffer sizes as channels in the opposite direction in the graph.

``` sh
./sdf3transform-sdf --graph ../examples/satellite.xml --transform model_buffersize
```

### model_autoconc(\<max>)

Model limits to auto-concurrency of actors by adding self-edges in the graph.

``` sh
./sdf3transform-sdf --graph ../examples/satellite.xml --transform 'model_autoconc(1)'
```

### select_buffersize

Allows selection from the command line of one of the buffer size options and outputs a graph with the selected buffer sizes.

``` sh
./sdf3transform-sdf --graph ../examples/satellite.xml --transform select_buffersize
```

### move_state

Model the internal state of an actor as a self-edge.

``` sh
./sdf3transform-sdf --graph ../examples/satellite.xml --transform move_state
```

### capacity_constrained(\<mcm>)

For an HSDF graph, create a new timed HSDF graph in which channel sizes are modelled through explicit channels (all buffer sizes are set to unbounded) and the throughput is limited by a cycle mean of the given `mcm`.

``` sh
./sdf3transform-sdf --graph ../examples/hsdf.xml --transform 'capacity_constrained(5)'
```

### retiming(\<dip>)

Perform retiming on an HSDF graph (without dip it will compute optimal retiming).

``` sh
./sdf3transform-sdf --graph ../examples/hsdf.xml --transform 'retiming(7)'
```
