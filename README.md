# Welcome to Base9!

[![Build Status](https://api.travis-ci.org/b9org/b9.svg?branch=master)](https://travis-ci.org/b9org/b9)
[![Coverage Status](https://coveralls.io/repos/github/b9org/b9/badge.svg?branch=master)](https://coveralls.io/github/b9org/b9?branch=master)

b9 is an educational language runtime and programming language! We’re using it to show people how to use
[Eclipse OMR] to build their own language runtime with a Just in Time (JIT) Compiler! The b9 language, b9porcelain, is a simple subset of JavaScript.

[Eclipse OMR]: https://github.com/eclipse/omr

This page contains basic instructions on building b9 in an Ubuntu 16.04 environment.
You may copy each of the blocks of commands and paste them in the terminal.
Depending on your user privileges, you may need to precede some of commands with
sudo in steps 1 and 3. For more detailed instructions, check out our 
[set-up page](./docs/SetupBase9.md).

### 1. Requirements

To get started with b9 on Ubuntu 16.04, you'll need the following:

* `git` 
* `build-essential`
* `nodejs`
* `npm` 
* `cmake` 
* `ninja`

To install the applications above, run the following command
```plaintext
apt-get update && apt-get install git build-essential nodejs npm cmake ninja
```

### 2. Clone the repository and get the submodules

```
git clone --recursive https://github.com/b9org/b9.git \
&& cd b9 \
&& git submodule update --init \
&& cd ..
```

### 3. Upgrade and create link for nodejs and install esprima

```plaintext
npm install -g n \
&& n latest \
&& ln -s /usr/local/bin/node /usr/bin/node \
&& cd b9 \
&& npm install esprima \
&& cd ..
```

### 4. Build b9

```
mkdir build \
&& cd build \
&& cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ../b9 \
&& ninja
```

### 5. Run Hello World!

Ensure you're in the build directory and run:

`./b9run/b9run ./test/hello.b9mod`

### 6. Test b9

You can run the full b9 test suite with:

`ninja test`

## Build your own Language Runtime

If you'd like to build your own language runtime, you can follow our [Developer Journey] to see the steps we took in building b9.

[Developer Journey]: ./docs/DeveloperJourney.md
