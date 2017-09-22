FROM ubuntu

RUN apt-get update && apt-get install -y \
 build-essential \
 cmake \
 git \
 nano \
 nodejs \
 npm \
 vim
RUN ln -s /usr/bin/nodejs /usr/bin/node

RUN mkdir /tutorial

WORKDIR /tutorial
RUN git clone --branch next --recursive https://github.com/youngar/Base9.git

WORKDIR /tutorial/Base9
RUN  npm install
RUN mkdir build

WORKDIR /tutorial/Base9/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN make -j8

ADD go /go
CMD  /bin/bash /go

