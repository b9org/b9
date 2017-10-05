# Docker with the development requirements of starting a dev environment
FROM ubuntu as base9-dev
RUN apt-get update && apt-get install -y \
 build-essential \
 clang-3.8 \
 cmake \
 git \
 nano \
 nodejs \
 npm \
 vim
RUN ln -s /usr/bin/nodejs /usr/bin/node


