# SDF3 - Synchronous Data Flow For Free

## What is SDF3

SDF3 is a library and tool to perform all sorts of operations on
dataflow graphs. You can find all features included in the tool
at <https://www.es.ele.tue.nl/sdf3>

------------------------------------------------------------------

## How do I install it

Building SDF3 on a typical Linux platform should be easy. It uses [cmake](https://cmake.org/) (version >=3.22.0) to build:

``` bash
cd <the directory containing sdf3>
mkdir build
cd build
cmake ..
make
```

The resulting binary executables can be found in the directory
`output/bin` inside the `build` directory.
There are also static libraries that can be used with your own programs in `output/lib`.

To build SDF3 using docker, see [below](buildingwithdocker).

------------------------------------------------------------------

## Building with Docker

Linux executables can also be built by running the provided Ubuntu [Docker](https://www.docker.com/) container.

Create a docker image from the provided `Dockerfile`

``` bash
docker build . -t sdf3
```

Wait for the construction of the image to complete.
It builds release executables, test results and test coverage results in mounted volumes in the image.
The results can be copied from the image to the host by creating a container from the image and copying the volumes to the host as needed.

``` bash
docker cp CONTAINER:/sdf3bin/ <host path>
docker cp CONTAINER:/sdf3test/ <host path>
docker cp CONTAINER:/sdf3coverage/ <host path>
```

------------------------------------------------------------------

## Is there any documentation for SDF3

See [UsingSDF3.md](./UsingSDF3.md)

Further documentation on the models and algorithms that are implemented in
SDF3 can be found at <https://www.es.ele.tue.nl/sdf3>. This website
provides also a command-line reference guide and documentation of
the API.

Documentation of the source code can be generated automatically
using doxygen. This can be done using the following commands:

``` bash
cd <the directory containing sdf3>
make doc
```

The resulting HTML documentation can be viewed with a browser from
the following location `<the directory containing sdf3>/build/doc/html/`.

------------------------------------------------------------------

## What license is used for SDF3

SDF3 is dual licensed. You can choose between the GNU GPL and the SDF3 Proprietary License. The GNU GPL gives you the right to use, modify and redistribute SDF3 under certain conditions. The GNU GPL license is distributed with the software, see the file [LICENSE](LICENSE). It is also
available at <http://www.gnu.org/licenses/gpl.txt>. Using SDF3 under the terms of the GNU GPL is free of charge.

The SDF3 Proprietary License gives you the right to use the source code for making your own commercial software. It allows you full protection of your work made with SDF3. The exact terms of this license will be established through a contract between you and Eindhoven University of Technology. For more information on the SDF3 Proprietary License, please contact us

------------------------------------------------------------------

## AUTHOR CONTACT INFORMATION

- Sander Stuijk (<s.stuijk@tue.nl>)
- Marc Geilen (<m.c.w.geilen@tue.nl>)

------------------------------------------------------------------

## Contributors

- Sander Stuijk
- Marc Geilen
- Bart Theelen
- Martijn Koedam
- Peter Poplavko
- Amir Ghamarian
- Reinier van Kampenhout
- Hadi Alizadeh Ara
- Morteza Damavandpeyma
- Hossein Elahi
- Umar Waqas
- Xue-Yang Zhu
- Yang Yang
- Anouk Westhoff
- Joep van Wanrooij
