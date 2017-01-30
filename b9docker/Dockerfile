FROM ubuntu

RUN apt-get update
RUN apt-get install -y clang-3.8
RUN apt-get install -y build-essential
RUN apt-get install -y git
RUN apt-get install -y nodejs
RUN apt-get install -y npm
RUN apt-get install -y vim nano
RUN cp /usr/bin/js /usr/bin/node
RUN chmod +x /usr/bin/node

RUN mkdir /tutorial
RUN cd /tutorial 
RUN cd /tutorial && git clone https://github.com/jduimovich/omr.git
RUN cd /tutorial && git clone  https://github.com/youngar/Base9.git

RUN cd /tutorial/Base9 && npm install esprima
RUN cd /tutorial/Base9 && make bench

ADD go go
CMD  /bin/bash go

