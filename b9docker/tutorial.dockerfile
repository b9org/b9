# Docker with some instructions for the tutorial
from aryoung/base9-dev as aryoung/base9-tutorial

RUN mkdir /tutorial
WORKDIR /tutorial
RUN git clone --branch next --recursive https://github.com/youngar/Base9.git

WORKDIR /tutorial/Base9
RUN npm install
RUN mkdir build

WORKDIR /tutorial/Base9/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN make -j8
ADD go go
CMD  /bin/bash go

