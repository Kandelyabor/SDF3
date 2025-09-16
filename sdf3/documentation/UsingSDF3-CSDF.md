# Using SDF3 - CSDF domain

The CSDF domain is supported with the following executables, described separately below.

- [`sdf3analyze-csdf`](#analyze)
- [`sdf3generate-csdf`](#generate)
- [`sdf3print-csdf`](#print)
- [`sdf3transform-csdf`](#transform)

## Analyze

- `sdf3analyze-csdf`

``` text
SDF3 sdf3analyze-csdf (version 1.0)

Usage: sdf3analyze-csdf --graph <file> --algo <algorithm> [--output <file>]
   --graph  <file>     input CSDF graph
   --output <file>     output file (default: stdout)
   --algo <algorithm>  analyze the graph with requested algorithm:
       consistency
       repetition_vector
       repetition_vector_sum
       throughput
       buffersize
```

### consistency

``` sh
./sdf3analyze-csdf --graph ../examples/csdf/simple_example.xml --algo consistency
```

``` sh
Graph is consistent.
analysis time: 0ms
```

### repetition_vector

Report the repetition vector of the graph, i.e., the smallest non-empty set of actor firings that brings the graph back to its initial state.

``` sh
./sdf3analyze-csdf --graph ../examples/csdf/simple_example.xml --algo repetition_vector
```

``` sh
Repetition vector:
 [a] = 6
 [b] = 4
 [c] = 2
analysis time: 0ms
```

### repetition_vector_sum

Report the sum of the repetition vector of the graph.

``` sh
./sdf3analyze-csdf --graph ../examples/csdf/simple_example.xml --algo repetition_vector_sum
```

``` sh
Repetition vector sum: 12
analysis time: 0ms
```

### throughput

Compute throughput of the graph by constructing the state-space of the operational behavior.
This method requires that the graph uses integer execution times.

``` sh
./sdf3analyze-csdf --graph ../examples/csdf/simple_example.xml --algo throughput
```

``` sh
thr(example) = 0.0833333
analysis time: 0ms
```

### buffersize

Determine the Pareto-optimal trade-offs between throughput and buffer size and report as an xml file.

``` sh
./sdf3analyze-csdf --graph ../examples/csdf/simple_example.xml --algo buffersize
```

## Generate

- `sdf3generate-csdf`

**`sdf3generate` does not seem to work. It generates an SDF graph.**

Usage example:

``` sh
./sdf3generate-csdf --settings ../examples/csdf-generate/sdf3.opt
```

## Print

``` sh
./sdf3print-csdf
```

``` text
SDF3 sdf3print-csdf (version 1.0)

Usage: sdf3print-csdf --graph <file> --format <type> [--output <file>]
   --graph  <file>     input SDF graph
   --output <file>     output file (default: stdout)
   --format <type>     output format:
       txt
       buffy
```

### txt

The following will generate a representation of the graph in a human readable textual format.

``` sh
./sdf3print-csdf --graph ../examples/csdf/simple_example.xml --format txt
```

### buffy

The following will generate C code for the 'buffy' buffer sizing method.
It will produce the entire trade-off space between throughput and buffer sizes.

``` sh
./sdf3print-csdf --graph ../examples/csdf/simple_example.xml --format buffy
```

## Transform

`sdf3transform-csdf` is used to transform CSDF graphs into different types of graphs. Its usage is as follows.

``` text
SDF3 sdf3transform-csdf (version 1.0)

Usage: sdf3transform-csdf --graph <file> --transform <list> [--output <file>]
   --graph  <file>     input CSDF graph
   --output <file>     output file (default: stdout)
   --transform <list>  comma separated list with graph transformations:
       to_wc-fsmsadf
```

### to_wc-fsmsadf

Transform a SDF graph to an FSM-SADF graph.

``` sh
./sdf3transform-csdf --graph ../examples/csdf/simple_example.xml --transform to_wc-fsmsadf
```
