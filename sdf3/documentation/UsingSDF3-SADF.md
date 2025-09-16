# Using SDF3 - SADF domain

The SADF domain is supported with the following executables, described separately below.

[`sdf3analyze-sadf`](#analyze)
[`sdf3convert-sadf-csdf2sadf`](#convert-csdf-to-sadf)
[`sdf3convert-sadf-sadf2csdf`](#convert-sadf-to-csdf)
[`sdf3convert-sadf-sadf2fsmsadf`](#convert-sadf-to-fsm-sadf)
[`sdf3convert-sadf-sadf2sdf`](#convert-sadf-to-sdf)
[`sdf3convert-sadf-sdf2sadf`](#convert-sdf-to-sadf)
[`sdf3print-sadf`](#print)
[`sdf3verify-sadf`](#verify)

## Analyze

``` sh
./sdf3analyze-sadf
```

``` text
SDF3 sdf3analyze-sadf (version 1.0)

Usage: sdf3analyze-sadf --graph <file> --check <property> [--output <file>]
   --graph  <file>      input SADF graph
   --output <file>      output file (default: stdout)
   --compute <property> compute requested property for graph:
       statistics
       number_of_states[(resolve_non_determinism)]
       throughput[(<process>)]
       inter_firing_latency(minimum|maximum|average|variance[,process(<process>)])
       response_delay(minimum|maximum|expected[,process(<process>)])
       deadline_miss(response|periodic,process(<process>),deadline(<value>))
       buffer_occupancy(maximum|average|variance[,channel(<channel>)])
       scenario_probabilities[(<process>)]
```

### Statistics

Collect various statistics of the SADF graph.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example1.xml --compute statistics
```

``` text
Graph Statistics:
  Number Of Kernels = 4
  Number Of Detectors = 1
  Number Of Data Channels = 6
  Number Of Control Channels = 3
Kernel Statistics:
  Average Number Of Scenarios = 1.75
  Average Number Of Profiles per Scenario = 1.42857
Detector Statistics:
  Average Number Of Scenarios = 1
  Average Number Of Markov Chain States per Scenario = 3
  Average Number Of SubScenarios = 2
  Average Number Of Profiles per SubScenario = 1
```

### number_of_states

Determine the number of states of the Markov Decision Process of the SADF graph.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example1.xml --compute number_of_states
```

``` text
SADF graph 'example1' implies a Markov Decision Process with 919 unique states.
```

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example1.xml --compute 'number_of_states(resolve_non_determinism)'
```

``` text
SADF graph 'example1' implies a Markov Decision Process with 460 unique states.
```

### Throughput

Determine the throughput of all processes or a selected process of the SADF graph.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example2.xml --compute throughput
```

``` text
---- Throughput of all processes for SADF graph 'example2' ----
Kernel 'A': 0.022053  (#States: 38, Analysis Time: 1187ms)
Kernel 'B': 0.022053  (#States: 11, Analysis Time: 31ms)
Detector 'D': 0.022053  (#States: 31, Analysis Time: 93ms)
```

### Inter-firing Latency

Determine the (minimum/maximum/average/variance of) inter-firing latency of all processes or a selected process.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example2.xml --compute 'inter_firing_latency(minimum)'
```

``` text
---- Minimum inter-firing latencies of all processes for SADF graph 'example2' ----
Kernel 'A': 0  (#States: 38, Analysis Time: 843ms)
Kernel 'B': 8  (#States: 11, Analysis Time: 15ms)
Detector 'D': 8  (#States: 31, Analysis Time: 93ms)
```

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example2.xml --compute 'inter_firing_latency(variance)'
```

``` text
---- Variance in inter-firing latencies of all processes for SADF graph 'example2' ----
Kernel 'A': 1247.41  (#States: 38, Analysis Time: 1031ms)
Kernel 'B': 282.998  (#States: 11, Analysis Time: 31ms)
Detector 'D': 419.477  (#States: 31, Analysis Time: 93ms)
```

### Response Delay

Determine the (minimum/maximum/expected) the response delay of all processes or a selected process.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example1.xml --compute 'response_delay(maximum)'
```

``` text
---- Maximum response delays of all processes for SADF graph 'example1' ----
Kernel 'A': 2.1  (#States: 2, Analysis Time: 0ms)
Kernel 'B': 6.1  (#States: 2, Analysis Time: 0ms)
Kernel 'C': 9.1  (#States: 2, Analysis Time: 0ms)
Kernel 'D': 11.6  (#States: 2, Analysis Time: 0ms)
Detector 'E': 2.1  (#States: 2, Analysis Time: 0ms)
```

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example1.xml --compute 'response_delay(expected)'
```

``` text
---- Expected response delay of all processes for SADF graph 'example1' ----
Kernel 'A': 2.05  (#States: 2, Analysis Time: 0ms)
Kernel 'B': 6.05  (#States: 2, Analysis Time: 0ms)
Kernel 'C': 9.05  (#States: 2, Analysis Time: 0ms)
Kernel 'D': 11.55  (#States: 2, Analysis Time: 0ms)
Detector 'E': 2.05  (#States: 2, Analysis Time: 0ms)
```

### Deadline Miss Probability

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example1.xml --compute 'deadline_miss(periodic,process(A),deadline(15))'
```

``` text
---- Periodic deadline miss probability of one process for SADF graph 'example1' ----
Kernel 'A': 0.571429  (#States: 22, Analysis Time: 46ms)
```

### Buffer Occupancy

Determine maximum, average or variance of buffer occupancy of the SADF graph.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example2.xml --compute 'buffer_occupancy(average)'
```

``` text
---- Average buffer occupancy of all channels for SADF graph 'example2' ----
Data channel 'A2B': 2  (#States: 36, Analysis Time: 15ms)
Data channel 'B2A': 0.75782  (#States: 79, Analysis Time: 31ms)
Data channel 'B2D': 0.850246  (#States: 67, Analysis Time: 31ms)
Control channel 'D2A': 1.40434  (#States: 72, Analysis Time: 15ms)
Control channel 'D2B': 1  (#States: 45, Analysis Time: 31ms)
```

### Scenario Probabilities

Determine the probabilities of the scenarios of all processes or a selected process.

``` sh
./sdf3analyze-sadf --graph ../examples/sadf/example2.xml --compute scenario_probabilities
```

``` text
---- (Sub)Scenario probabilities of all processes for SADF graph 'example2' ----
Kernel 'A' (Analysis Time: 0ms)
        P('S') = 0.75
        P('T') = 0.25
Kernel 'B' (Analysis Time: 0ms)
        P('S') = 0.75
        P('T') = 0.25
Detector 'D' (Analysis Time: 0ms)
        P('S') = 0.75
        P('T') = 0.25
```

## Convert CSDF to SADF

```  sh
./sdf3convert-sadf-csdf2sadf
```

``` text
SDF3 sdf3convert-csdf-sadf (version 1.0)

Usage: sdf3convert-csdf-sadf --graph <file> [--output <file>]
   --graph  <file>     input CSDF graph
   --output <file>     output file (default: stdout)
```

```  sh
./sdf3convert-sadf-csdf2sadf --graph ../examples/csdf/simple_example.xml --output ./simple_example2-sadf.xml
```

``` text
NOTE: SADF interprets auto-concurrency in the original CSDF as absent
```

## Convert SADF to CSDF

```  sh
./sdf3convert-sadf-sadf2csdf
```

``` text
SDF3 sdf3convert-sadf-csdf (version 1.0)

Usage: sdf3convert-sadf-csdf --graph <file> [--output <file>]
   --graph  <file>     input SADF graph
   --output <file>     output file (default: stdout)
```

```  sh
./sdf3convert-sadf-sadf2csdf --graph ../examples/sadf/ce_converted_csdf.xml --output ./csdf.xml
```

## Convert SADF to FSM-SADF

```  sh
./sdf3convert-sadf-sadf2fsmsadf
```

``` text
SDF3 sdf3convert-sadf-fsmsadf (version 1.0)

Usage: sdf3convert-sadf-fsmsadf --graph <file> [--output <file>]
   --graph  <file>     input SADF graph
   --output <file>     output file (default: stdout)
```

```  sh
./sdf3convert-sadf-sadf2fsmsadf --graph ../examples/sadf/ce_converted_csdf_strongly_consistent.xml
```

## Convert SADF to SDF

```  sh
./sdf3convert-sadf-sadf2sdf
```

``` text
SDF3 sdf3convert-sadf-sdf (version 1.0)

Usage: sdf3convert-sadf-sdf --graph <file> [--output <file>]
   --graph  <file>     input SADF graph
   --output <file>     output file (default: stdout)
```

```  sh
./sdf3convert-sadf-sadf2sdf --graph ../examples/sadf/mp3_converted_sdf.xml
```

## Convert SDF to SADF

```  sh
./sdf3convert-sadf-sdf2sadf
```

``` text
SDF3 sdf3convert-sdf-sadf (version 1.0)

Usage: sdf3convert-sdf-sadf --graph <file> [--type=<type>] [--output <file>]
   --graph  <file>     input SDF graph
   --type  <type>      sadf or fsmsadf (default: sadf)
   --output <file>     output file (default: stdout)
```

```  sh
./sdf3convert-sadf-sdf2sadf --graph ../examples/h263decoder.xml
```

## Print

```  sh
./sdf3print-sadf
```

``` text
SDF3 sdf3print-sadf (version 1.0)

Usage: sdf3print-sadf --graph <file> --format <type> [--output <file>]
   --graph  <file>     input SADF graph
   --output <file>     output file (default: stdout)
   --format <type>     output format:
       dot
       html[(dot(<location>))]
       php[(dot(<location>))]
       poosl[(settings(<file>),log(<file>))]
```

### Graphviz dot

```  sh
./sdf3print-sadf --graph ../examples/sadf/example1.xml --format dot
```

``` text
digraph example1 {

    K0 [ label="A" ];
    K1 [ label="B" ];
    K2 [ label="C" ];
    K3 [ label="D" ];

    D0 [ label="E", style=dotted ];

    K0 -> K1 [ label="A2B", taillabel="2" ];
    K1 -> K0 [ label="B2A(2)", headlabel="2" ];
    K1 -> K2 [ label="B2C", headlabel="1" ];
    K2 -> K3 [ label="C2D" ];
    K3 -> K1 [ label="D2B(3)", taillabel="1", headlabel="2" ];
    K0 -> D0 [ label="A2E", taillabel="1", headlabel="1" ];

    D0 -> K1 [ label="E2B", headlabel="1", style=dotted ];
    D0 -> K2 [ label="E2C", headlabel="1", style=dotted ];
    D0 -> K3 [ label="E2D", headlabel="1", style=dotted ];
}
```

### HTML

```  sh
./sdf3print-sadf --graph ../examples/sadf/example1.xml --format html
```

Only works on a Unix platform.

### PHP

```  sh
./sdf3print-sadf --graph ../examples/sadf/example1.xml --format php
```

Only work in a Unix platform.

### POOSL

Convert the SADF graph to a [POOSL](https://projects.eclipse.org/projects/modeling.poosl) model.

```  sh
./sdf3print-sadf --graph ../examples/sadf/example1.xml --format poosl
```

## Verify

```  sh
./sdf3verify-sadf
```

``` text
SDF3 sdf3verify-sadf (version 1.0)

Usage: sdf3verify-sadf --graph <file> --check <property> [--output <file>]
   --graph  <file>     input SADF graph
   --output <file>     output file (default: stdout)
   --check <property>  verify requested property for graph:
       connected_graph
       is_timed
       strong_consistency
       ergodicity
       moc
       is_hsdf
       is_sdf
       is_csdf
       simple_sdf_collection
```

### Connected Graph

``` sh
./sdf3verify-sadf --graph ../examples/sadf/example1.xml --check connected_graph
```

``` text
SADF graph 'example1' is connected.
```

### Is Timed

``` sh
./sdf3verify-sadf --graph ../examples/sadf/example1.xml --check is_timed
```

``` text
SADF graph 'example1' is timed.
```

### Strong Consistency

``` sh
./sdf3verify-sadf --graph ../examples/sadf/example1.xml --check strong_consistency
```

``` text
SADF graph 'example1' is strongly consistent.
```

### MoC

``` sh
./sdf3verify-sadf --graph ../examples/sadf/example2.xml --check moc
```

``` text
SADF graph 'example2' is really an SADF graph.
```

### Is HSDF

``` sh
./sdf3verify-sadf --graph ../examples/sadf/example2.xml --check is_hsdf
```

``` text
SADF graph 'example2' does not represent an HSDF graph.
```

### Is SDF

``` sh
./sdf3verify-sadf --graph ../examples/sadf/mp3_converted_sdf.xml --check is_sdf
```

``` text
SADF graph 'mp3_converted_sdf' represents an SDF graph.
```

### Is CSDF

``` sh
./sdf3verify-sadf --graph ../examples/sadf/mp3_converted_sdf.xml --check is_csdf
```

``` text
SADF graph 'mp3_converted_sdf' represents a CSDF graph.
```

### Simple SDF Collection

``` sh
./sdf3verify-sadf --graph ../examples/sadf/mp3_converted_sdf.xml --check simple_sdf_collection
```

``` text
SADF graph 'mp3_converted_sdf' is a strongly consistent collection of SDF graphs.
```
