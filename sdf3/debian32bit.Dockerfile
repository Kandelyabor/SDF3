# build with: 
# docker build . -t sdf3
# docker create sdf3
# check the container name and use it in the following:
# docker cp CONTAINER:/sdf3bin/ <host path>

FROM i386/debian

RUN apt-get update
RUN apt-get install --force-yes -y libssl-dev wget build-essential git
RUN wget https://github.com/Kitware/CMake/releases/download/v3.24.2/cmake-3.24.2.tar.gz
RUN tar -zxvf cmake-3.24.2.tar.gz
WORKDIR /cmake-3.24.2
RUN ./bootstrap
RUN make
RUN make install
RUN cmake --version

WORKDIR /usr/
COPY . .

RUN cmake -DCODE_COVERAGE=OFF .
RUN make
RUN ctest

# make release executables
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN make
RUN mkdir /sdf3bin
RUN cp -R ./build/bin/* /sdf3bin/

VOLUME /sdf3bin

# keep running for debugging
# ENTRYPOINT ["tail", "-f", "/dev/null"]