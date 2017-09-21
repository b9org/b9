FROM ubuntu

RUN apt-get update
RUN apt-get install -y clang-3.8
RUN apt-get install -y build-essential
RUN apt-get install -y git
RUN apt-get install -y nodejs
RUN apt-get install -y npm
RUN apt-get install -y vim nano
RUN apt-get install -y cmake
RUN cp /usr/bin/js /usr/bin/node
RUN chmod +x /usr/bin/node

RUN mkdir /tutorial
RUN cd /tutorial && git clone --branch next --recursive https://github.com/youngar/Base9.git

RUN cd /tutorial/Base9 && npm install
RUN cd /tutorial/Base9 && mkdir build
RUN cd /tutorial/Base9/build && cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN cd /tutorial/Base9/build && make -j8

ADD go go
CMD  /bin/bash go

