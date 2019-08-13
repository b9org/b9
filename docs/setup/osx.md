---
title: OSX Setup
---

## Requirements

If you do not already have the OSX command line tools installed, then you may do so by running the following command in the terminal.
```sh
sudo xcode-select --install
```

To get started with b9 using the Ninja build system, you'll need the following:

* `git` 
* `node`
* `npm`
* `cmake`
* `ninja`

To install the above packages, you may use [homebrew](https://brew.sh/) and run the following command in the terminal:

```sh
brew install git node npm cmake ninja
```

## Clone the repository and get the submodules

```sh
git clone --recursive https://github.com/b9org/b9.git
```

## Install Esprima

```sh
cd b9 \
&& npm install esprima
```

## Build b9

```sh
mkdir build \
&& cd build \
&& cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. \
&& ninja
```

## Run Hello World!

Ensure you're still in the `build` directory and run:

```sh
./b9run/b9run ./test/hello.b9mod
```

## Test b9

You can run the full b9 test suite with:

```sh
ninja test
```
