---
layout: default
title: OSX Setup
---

## Setting up Base9 on OSX

### 1. Requirements

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

### 2. Clone the repository and get the submodules

```sh
cd ~ \
&& git clone --recursive https://github.com/b9org/b9.git
```

### 3. Install Esprima
```sh
cd b9 \
&& npm install esprima
```
### 4. Build b9

```sh
mkdir build \
&& cd build \
&& cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. \
&& ninja
```

### 5. Run Hello World!

Ensure you're still in the `build` directory and run:

```sh
./b9run/b9run ./test/hello.b9mod
```

### 6. Test b9

You can run the full b9 test suite with:

```sh
ninja test
```
