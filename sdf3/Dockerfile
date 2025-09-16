# build with:
# docker build . -t sdf3
# docker create sdf3
# check the container name and use it in the following:
# docker cp CONTAINER:/sdf3bin/ <host path>
# docker cp CONTAINER:/sdf3doc/ <host path>
# docker cp CONTAINER:/sdf3test/ <host path>
# docker cp CONTAINER:/sdf3coverage/ <host path>

FROM ubuntu:24.04

# the following lines are to avoid the interactive time zone question during install
ENV TZ=Europe/Amsterdam
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get install --force-yes -y libxml2-dev libxml2-utils python3 lcov build-essential make pkg-config git wget lsb-release software-properties-common
RUN apt-get install --force-yes -y doxygen graphviz

RUN wget --no-check-certificate -vO - https://apt.kitware.com/keys/kitware-archive-latest.asc > ./key.txt
RUN cat ./key.txt | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com --recv-keys 6AF7F09730B3F0A4
RUN apt update
RUN apt install -y cmake


WORKDIR /usr/
COPY . .

# make release executables
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN make
RUN mkdir /sdf3bin
RUN cp -R ./output/bin/* /sdf3bin/

# execute tests
RUN cmake -DCODE_COVERAGE=ON .
RUN make

# run test and continue also if test fails
RUN ctest || :
RUN mkdir /sdf3test
RUN cp -R ./output/test/xhtml/* /sdf3test/
RUN cp -R ./output/test/results/* /sdf3test/

# get coverage information
RUN make sdf3coverage
RUN mkdir /sdf3coverage
RUN cp -R ./output/coverage/* /sdf3coverage/

# make documentation
RUN make doc
RUN mkdir /sdf3doc
RUN cp -R ./output/doc/html/* /sdf3doc/


VOLUME /sdf3bin
VOLUME /sdf3doc
VOLUME /sdf3test
VOLUME /sdf3coverage

# keep running for debugging
# ENTRYPOINT ["tail", "-f", "/dev/null"]N