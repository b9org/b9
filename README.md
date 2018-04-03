# Welcome to b9!

[![Build Status](https://api.travis-ci.org/b9org/b9.svg?branch=master)](https://travis-ci.org/b9org/b9)
[![Coverage Status](https://coveralls.io/repos/github/b9org/b9/badge.svg?branch=master)](https://coveralls.io/github/b9org/b9?branch=master)

b9 is an educational language runtime and programming language! We’re using it to show people how to use [Eclipse OMR] to build their own language runtime with a Just in Time (JIT) Compiler! The b9 language, b9porcelain, is a simple subset of JavaScript.

[Eclipse OMR]: https://github.com/eclipse/omr

# Getting started

This page contains some basic instructions to get you started. For more detailed instructions, go to:
* [Ubuntu set-up page](./docs/SetupUbuntu.md).
* [OSX set-up page](./docs/SetupOSX.md).

### 1. Requirements

To get started with b9 using the Ninja build system, you'll need the following:

* `git` 
* `build-essential`
* `nodejs` **(Minimum version 4.5.0)**
* `npm`
* `esprima`
* `cmake` **(Minimum version 3.2.0)**
* `ninja`

### 2. Clone the repository and get the submodules

```sh
git clone --recursive https://github.com/b9org/b9.git
```

### 3. Install Esprima

```sh
cd b9 \
&& npm install esprima \
&& cd ..
```

### 4. Build b9

```sh
mkdir build \
&& cd build \
&& cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ../b9 \
&& ninja
```

### 5. Run Hello World!

In the `build` directory, run:

```sh
./b9run/b9run ./test/hello.b9mod
```

### 6. Test b9

You can run the full b9 test suite with:

```sh
ninja test
```

## Build your own Language Runtime

If you'd like to build your own language runtime, you can follow our [Developer Journey] to see the steps we took in building b9.

[Developer Journey]: ./docs/DeveloperJourney.md
