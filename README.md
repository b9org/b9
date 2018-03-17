# Welcome to Base9!

[![Build Status](https://api.travis-ci.org/b9org/b9.svg?branch=master)](https://travis-ci.org/b9org/b9)
[![Coverage Status](https://coveralls.io/repos/github/b9org/b9/badge.svg?branch=master)](https://coveralls.io/github/b9org/b9?branch=master)

Base9 is an educational language runtime and programming language! We’re using it to show people how to use
[Eclipse OMR] to build their own language runtime with a Just in Time (JIT) Compiler! The Base9 language, b9porcelain, is a simple subset of JavaScript.

[Eclipse OMR]: https://github.com/eclipse/omr

## Requirements

To get started with Base9, you'll need:

`git, build-essential, nodejs, npm, esprima, cmake, ninja` 

For detailed instructions on setting up Base9, check out our [set-up page](./docs/SetupBase9.md).


## Clone the repository and get the submodules

```
git clone --recursive https://github.com/b9org/b9.git
cd Base9
git submodule update --init
```

## Build Base9

```
mkdir build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

## Run Hello World!

Ensure you're in the build directory and run:

`./b9run/b9run ./test/hello.b9mod`

## Test Base9

You can run the full Base9 test suite with:

`ninja test`

## Build your own Language Runtime

If you'd like to build your own language runtime, you can follow our [Developer Journey] to see the steps we took in building Base9.

[Developer Journey]: ./docs/DeveloperJourney.md
