# Base9 Documentation 

## Introduction to Base9

Base9 is a tiny virtual machine and programming language! We’re using it to show people how to use [Eclipse OMR](https://github.com/eclipse/omr) to build their own programming language. 

The Base9 language is a subset of JavaScript and is very limited in functionality. It currently only contains support for integers. 

For a full overview of Base9 and it's components, plus a tutorial on adding new bytecodes, see our [Supercharge a Language Runtime](https://github.com/arianneb/Base9/blob/documentation/doc/CASCON2017_SuperchargeALanguageRuntime.pdf) presentation and tutorial. 


## Base9 Architecture 

Base9 is comprised of two major components:

#### 1. Ahead of Time (AOT) Compilation Unit

The AOT compilation unit is where the Base9 source code is tranformed into bytecodes, which will be used as input to the VM at runtime. Here, the source code undergoes a 2-step compilation process which makes it consumable by the VM. 

#### 2. Virtual Machine / Runtime Unit
At runtime, the byte codes are given to the VM as input, where they will either be interpreted by Base9 or JIT compiled using OMR.

![](https://github.com/arianneb/Base9/blob/documentation/images/b9_architecture.png)


## Getting Set-up 

#### Set-up on Ubuntu:
**insert link**

#### Set-up on OSX:
**insert link**

#### Clone the repository and get submodules
```
git clone --recursive https://github.com/youngar/Base9.git
cd Base9
git submodule update --init
```


## Directory Structure of Base9

**insert description and link**


## Build Base9
```
mkdir build && cd build
npm install esprima
cmake –GNinja –DCMAKE_BUILD_TYPE=Debug ..
ninja
```

## Run Base9

#### Run Hello World!
On Ubuntu:

```./b9run/b9run ./test/libhellod.so```

On OSX:

```./b9run/b9run ./test/libhellod.dylib"```

#### Run a Base9 Benchmark

Command structure:

```b9run [-function <function>] <module> [<arg>...]```

b9run is the main executable that runs the Base9 modules. In the following example, we are running the factorial function with an input of 20 by passing the libfactoriald.so shared object file to the virtual machine: 

```./b9run/b9run -function factorial test/libfactoriald.so 20```
 


## Test Base9 

You can run the full Base9 test suit by running either:

```ninja test```

or

```ctest -V -R run_interpreter_test```
