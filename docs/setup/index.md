---
layout: page
title: Base9 Setup
section: setup
---

This page contains some basic instructions to get you started. For more detailed instructions, go to:

* [Ubuntu set-up page](./docs/setup/ubuntu.md).
* [OSX set-up page](./docs/setup/osx.md).

## Requirements

To get started with base9 using the Ninja build system, you'll need the following:

* `git` 
* `build-essential`
* `nodejs` **(Minimum version 4.5.0)**
* `npm`
* `esprima`
* `cmake` **(Minimum version 3.2.0)**
* `ninja`

## Clone the repository and get the submodules

```sh
git clone --recursive https://github.com/b9org/b9.git
```

## Install Esprima

```sh
cd b9 \
&& npm install esprima
```

## Build base9

```sh
mkdir build \
&& cd build \
&& cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. \
&& ninja
```

## Run Hello World!

In the `build` directory, run:

```sh
./b9run/b9run ./test/hello.b9mod
```

## Test base9

You can run the full base9 test suite with:

```sh
ninja test
```
