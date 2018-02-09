# Welcome to Base9!

[![Build Status](https://api.travis-ci.org/b9org/b9.svg?branch=master)](https://travis-ci.org/b9org/b9)

[![Coverage Status](https://coveralls.io/repos/github/b9org/b9/badge.svg?branch=master)](https://coveralls.io/github/b9org/b9?branch=master)

Base9 is a miniature virtual machine and programming language! We’re using it to show people how to use [Eclipse OMR](https://github.com/eclipse/omr) to build their own programming languages. The Base9 language is a simple subset of JavaScript. 


## Requirements

To get started with Base9, you'll need:

`git, build-essential, nodejs, npm, esprima, cmake, ninja` 

Click [here](https://github.com/arianneb/Base9/blob/documentation/doc/README.md) for detailed instructions on getting set-up with Base9


## Clone the repository and get the submodules 
```
git clone --recursive https://github.com/youngar/Base9.git
cd Base9
git submodule update --init
```

## Build Base9
```
mkdir build && cd build
cmake –GNinja –DCMAKE_BUILD_TYPE=Debug ..
ninja
```

## Run Hello World!
On Ubuntu:

`./b9run/b9run ./test/libhellod.so`

On OSX:

`./b9run/b9run ./test/libhellod.dylib"`


## Test Base9

You can run the full Base9 test suit by running either:

`ninja test`

or 

`ctest -V -R run_interpreter_test`
