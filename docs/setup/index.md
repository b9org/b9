---
layout: page
title: Base9 Setup
section: setup
---

## Getting Set-up 

#### Set-up on Ubuntu:

[Ubuntu Set-up Instructions](./ubuntu.md)

#### Set-up on OSX:

[OSX Set-up Instructions](./osx.md)

#### Clone the repository and get submodules
```
git clone --recursive https://github.com/b9org/b9.git
cd Base9
git submodule update --init
```

## Directory Structure of Base9

Learn more about our [Directory Structure](./DirectoryStructure.md)


## Build Base9
```
mkdir build && cd build
npm install esprima
cmake –GNinja –DCMAKE_BUILD_TYPE=Debug ..
ninja
```

## Run Base9

#### Run Hello World!

```sh
./b9run/b9run ./test/hello.b9mod
```

#### Run a Base9 Benchmark (outdated)

Command structure:

```sh
b9run [-function <function>] <module> [<arg>...]
```

b9run is the main executable that runs the Base9 modules. In the following example, we are running the factorial function with an input of 20 by passing the factorial.b9mod shared object file to the virtual machine: 

```sh
./b9run/b9run -function factorial test/factorial.b9mod 20
```


## Test Base9 

You can run the full Base9 test suit by running the following command from the build directory:

```sh
ninja test
```
