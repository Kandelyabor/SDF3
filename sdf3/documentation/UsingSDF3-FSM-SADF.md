# Using SDF3 - FSM-SADF domain

The FSM-SADF domain is supported with the following executables, described separately below.

- [`sdf3analyze-fsmsadf`](#analyze)
- [`sdf3flow-fsmsadf`](#flow)
- [`sdf3generate-fsmsadf`](#generate)
- [`sdf3transform-fsmsadf`](#transform)
- [`sdf3inputconvert-fsmsadf`](#input-convert)
- [`sdf3outputconvert-fsmsadf`](#output-convert)

## Analyze

- `sdf3analyze-fsmsadf`

``` text
SDF3 sdf3analyze-fsmsadf (version 1.0)

Usage: sdf3analyze-fsmsadf [--settings <file>] [--graph <file> --algo <algorithm>] [--output <file>] [--seq <sequence> --traceFiles <file.txt,file.etf>]
   --settings  <file>  settings for the analysis (default: sdf3.opt)
   --graph  <file>     input SADF graph
   --algo <algorithm>  analyze the graph with requested algorithm:
       criticalscenariosequence
       extractscenariographs
       isolatescenarios
       mpeigen
       throughput (computes conservative throughput approximation, deprecated)
       throughput-mpautomaton
       throughput-regex
       latency-regex
       throughput-statespace (deprecated)
       trace
       vectortrace
       schedule-check
       buffersize-maxplus
   --output <file>     output file (default: stdout)
For use with 'trace' algorithm only:
   --seq  <sequence>     comma-separated sequence of scenarios
   --traceFormat  <format>     <format> is either TNO or XML
   --traceFiles  <file.txt,file.etf>     output files for trace in TNO format
   --traceFile  <out.xml>     output files for trace in XML format
For use with 'latency-regex' algorithm only:
   --period  <number>     latency is computed relative to the period

Settings not found.
```

### criticalscenariosequence

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo criticalscenariosequence
```

``` sh
Critical scenario sequence:
sl
analysis time: 187ms
```

### extractscenariographs

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo extractscenariographs
```

Outputs to the console information about the scenarios and the graph and sdf models of the individual scenarios.

### isolatescenarios

Creates an equivalent FSM-SADF graph with a separate scenario graph for each scenario.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo isolatescenarios
```

Outputs an FSM-SADF model.

### mpeigen

Returns pairs of eigenvalues and eigenvectors for each of the scenarios of the FSM-SADF.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo mpeigen
```

``` sh
eigenvalue: 5.83e+06
-7.46636e+06 ; -6.63992e+06 ; -2.38779e+06 ; -2.34489e+06 ; 0 ; -6.63992e+06 ; -4.25393e+06 ; -4.23248e+06 ;
eigenvalue: 5.83e+06
-9.43472e+06 ; -8.24734e+06 ; -2.9799e+06 ; -2.93701e+06 ; 0 ; -8.24734e+06 ; -4.84604e+06 ; -4.82459e+06 ;
eigenvalue: 5.83e+06
-9.08995e+06 ; -8.37737e+06 ; -2.94727e+06 ; -2.90437e+06 ; 0 ; -8.37737e+06 ; -5.75598e+06 ; -5.73453e+06 ; -4.81341e+06 ; -4.79196e+06 ;
eigenvalue: 5.83e+06
-9.27858e+06 ; -8.40348e+06 ; -4.17568e+06 ; -2.93701e+06 ; 0 ; -8.40348e+06 ; -6.04182e+06 ; -4.82459e+06 ;
eigenvalue: 5.83e+06
-8.66157e+06 ; -7.78647e+06 ; -2.32e+06 ; -3.55867e+06 ; 0 ; -7.78647e+06 ; -4.20759e+06 ; -5.42481e+06 ;
```

### throughput

Computes a conservative throughput approximation, **the analysis is deprecated in favor of exact throughput analysis with `throughput-mpautomaton`**

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo throughput
```

``` sh
Throughput (scenario graph):       1.28232e-07
analysis time: 78ms
Throughput (reference schedule):   1.28232e-07
analysis time: 62ms
Throughput (scenario transitions): 1.60793e-07
analysis time: 31ms
```

### throughput-mpautomaton

Max-plus-automaton based exact worst-case throughput analysis.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo throughput-mpautomaton
```

``` sh
Throughput (max-plus automaton):          1.71527e-07
analysis time: 296ms
```

### throughput-statespace

Max-plus-state-space-based throughput analysis method (deprecated in favor of the max-plus-automaton-based method).

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo throughput-statespace
```

``` sh
Note that the state-space based throughput computation is deprecated in favor of throughput computation with max-plus automaton (throughput-mpautomaton).
Throughput (state-space):          1.71527e-07
analysis time: 234ms
```

### throughput-regex

Throughput analysis for FSM-SADF graphs where the scenario sequence is specified as a regular expression.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/regexExample.xml --algo throughput-regex
```

``` sh
Throughput (max-plus automaton):          0.25
analysis time: 15ms
```

### latency-regex

Latency analysis with respect to a provided period for FSM-SADF graphs where the scenario sequence is specified as a regular expression.
Latency $L$ for period $\mu$ on token $x$ means that $L\in{}R$ is the smallest such that instance $k$ of token $x$ is produced no later than $L+k\cdot{}\mu{}$.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/regexExample.xml --algo latency-regex --period 5.0
```

``` text
The latency vector is : 3 4 -1 4 with respect to initial tokens w  x  y  z
analysis time: 15ms
```

### trace

Generate a trace file for the specified scenario sequence in [Eclipse Trace4CPS](https://eclipse.dev/trace4cps/) or [cmtrace](https://github.com/Model-Based-Design-Lab/cmtrace) format.

For the Trace4CPS format:

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo trace --traceFormat Trace4CPS --traceFiles trace.txt,trace.etf --seq ss,m
```

The output is produced in the specified files `trace.txt` and `trace.etf`.

For the cmtrace format:

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo trace --traceFormat XML --traceFile trace.xml --seq ss,m
```

The output is produced in the specified file `trace.xml`.

### vectortrace

Generate a vector-trace file of the sequence of max-plus state-vectors for the specified scenario sequence in  [cmtrace](https://github.com/Model-Based-Design-Lab/cmtrace) format.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --algo vectortrace --traceFormat XML --traceFile trace.xml --seq ss,m
```

The output is produced in the specified file `trace.xml`.

### schedule-check

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/bag-1.xml --algo schedule-check --seq scenario1,scenario2,scenario2,scenario1
```

``` text
[INFO   ] Failed opening 'sdf3.opt', fallback on command-line settings.
The schedule check passed.
analysis time: 31ms
```

### buffersize-maxplus

Compute buffer sizes vs throughput tradeoff using the max-plus based analysis method.

``` sh
./sdf3analyze-fsmsadf --graph ../examples/fsmsadf/simplefsmsadf.xml --algo buffersize-maxplus
```

``` xml
<?xml version='1.0' encoding='UTF-8'?>
<sdf3 type='sdf' version='1.0'
     xmlns:xsi='https://www.w3.org/2001/XMLSchema-instance'
      xsi:noNamespaceSchemaLocation='https://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>
    <storageThroughputTradeOffs>
        <distributionsSet thr='0.2' sz='1'>
            <distribution>
                <ch name='a_b' sz='1'/>
            </distribution>
        </distributionsSet>
        <distributionsSet thr='0.333333' sz='2'>
            <distribution>
                <ch name='a_b' sz='2'/>
            </distribution>
        </distributionsSet>
    </storageThroughputTradeOffs>
</sdf3>
```

## Flow

`sdf3flow-fsmsadf`

Run from the directory `sdf3/sdf3/fsmsadf/testbench/flow-test`.

``` sh
../../../output/bin/sdf3flow-fsmsadf --settings ./sdf3-fsmsadf-flow-test.opt
```

## Generate

- `sdf3generate-fsmsadf`

Usage example:

``` sh
./sdf3generate-fsmsadf --settings ../examples/fsmsadf-flow/sdf3-fsmsadf-flow-test.opt
```

Generates xml input files from readable dataflow specification. Requires python and requires xmllint to be installed.

## Transform

``` sh
./sdf3transform-fsmsadf
```

``` text
[INFO   ] Failed opening 'sdf3.opt', fallback on command-line settings.
SDF3 sdf3transform-fsmsadf (version 1.0)
Usage: sdf3transform-fsmsadf [--settings <file>] [--graph <file> --transform <list>]
 [--scenario <name>] [--output <file>]
   --settings  <file>  settings for the graph generator (default: sdf3.opt)
   --graph  <file>     input SDF graph
   --transform <list>  comma separated list with graph    transformations:
       wc_sdfg
       model_autoconc(<max>)
       to_sdfg
       create_detector
       to_binding_aware
       merge_scenarios
   --scenario <name>   select a scenario
   --output <file>     output file (default: stdout)
Required settings not found.
```

### wc_sdfg

Transform an FSM-SADF graph into an SDF graph that is a worst-case abstraction of the SADF graph.

``` sh
./sdf3transform-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --transform wc_sdfg
```

### model_autoconc(\<max\>)

Generate a transformed FSM-SADF model in which the auto-concurrency of actors is limited.

``` sh
./sdf3transform-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --transform 'model_autoconc(1)'
```

### to_sdfg

Generate an SDF graph that represents the selected scenario of the FSM-SADF.

``` sh
./sdf3transform-fsmsadf --graph ../examples/fsmsadf/mp3decoder.xml --transform to_sdfg --scenario m
```

### create_detector

Add a detector scenario.

``` sh
./sdf3transform-fsmsadf --graph ../examples/fsmsadf/fsmsadf-susan.xml --transform create_detector
```

**TODO: more documentation needed.**

### to_binding_aware

``` sh
./sdf3transform-fsmsadf --graph ../examples/fsmsadf/fsmsadf-flow-test-mapped.xml --transform to_binding_aware
```


### merge_scenarios

``` sh
./sdf3transform-fsmsadf --graph ../examples/fsmsadf/bag-13.xml --transform merge_scenarios
```

``` text
[INFO   ] Failed opening 'sdf3.opt', fallback on command-line settings.
Error: switch_out1_dmaProduceWrite_0 not found in detector scenario.
```

**TODO: find an example on which it works as intended.**

## Input Convert

**TODO: add documentation about this tool**

``` sh
./sdf3inputconvert-fsmsadf --settings ../examples/fsmsadf-input-output-convert/sdf3-input-conversion-compsoc.opt
```

``` text
[INFO   ] Parsing application file: ../examples/fsmsadf-input-convert/app_test0.xml
[INFO   ] Reading unit conversion values: time unit: 1ns, data unit: 128bytes
[INFO   ] Create application graph for application: example-app
[INFO   ] Parsing architecture file: ../examples/fsmsadf-input-convert/arch_buffdim.xml
[INFO   ] Create architecture graph
[WARNING] Using hardcoded values for TDMA
[WARNING] Using hardcoded values for TDMA
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[WARNING] Using hardcoded value for latency: 1200 timeUnits
[INFO   ] Write output document to file: 'fsmsadf-input-convert-compsoc.xml'
```

## Output Convert

``` sh
./sdf3outputconvert-fsmsadf --settings ../examples/fsmsadf-input-output-convert/sdf3-output-convert-compsoc-1scen.opt
```

``` text
Convert output files
[INFO   ] Parsing application file..
[INFO   ] Reading unit conversion values: time unit: 20ns, data unit: 1bytes
[INFO   ] Parsing architecture file..
[INFO   ] We found a clock: NoC_clock (50 MHz)
[INFO   ] Creating communication file: '../examples/fsmsadf-input-output-convert/fsmsadf-output-convert-compsoc-1scen-communication.xml'
[INFO   ] Using mapping: 21
Connection between same tilefbd10
[INFO   ] Saving communication file: '../examples/fsmsadf-input-output-convert/fsmsadf-output-convert-compsoc-1scen-communication.xml'
[INFO   ] Creating mapping file: ../examples/fsmsadf-input-output-convert/fsmsadf-output-convert-compsoc-1scen-mapping.xml
[INFO   ] Using mapping 21
[INFO   ] Using default clock for tile: mb0
[INFO   ] Using default clock for tile: mb1
[INFO   ] slots = 1
[INFO   ] slots = 1
[INFO   ] Saving mapping file: ../examples/fsmsadf-input-output-convert/fsmsadf-output-convert-compsoc-1scen-mapping.xml
```
