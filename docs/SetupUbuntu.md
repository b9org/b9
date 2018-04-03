---
layout: default
title: Ubuntu Setup
---

## Setting up Base9 on Ubuntu

Depending on your user privileges, you may have to precede some of the
commands in the steps below with `sudo`.
### 1. Requirements

To get started with b9 using the Ninja build system, you'll need the following:

* `git` 
* `build-essential`
* `nodejs` **(Minimum version 4.5.0)**
* `npm`
* `cmake` **(Minimum version 3.2.0)**
* `ninja`

You can install the packages above using the following command:

```sh
apt-get update && apt-get install git build-essential nodejs npm cmake
```

### 2. Clone the repository and get the submodules

```sh
git clone --recursive https://github.com/b9org/b9.git
```

### 3. Create link for nodejs v4.5+

Building b9 requires nodejs version 4.5+. The default nodejs versions
in Ubuntu 16.04 and older do not meet this requirement, so you may have to upgrade your nodejs. There are several ways to do it, and [this
is one way](https://nodejs.org/en/download/package-manager/#debian-and-ubuntu-based-linux-distributions) we can recommend.

Assuming you now have nodejs v4.5+, create a link using the command below:
```sh
ln -s $NODEJS_BIN /usr/bin/node
```
Variable `NODEJS_BIN` must be the location of where your nodejs version 4.5+ executable is located. Here is an example of how this may look like:

```sh
ln -s /usr/local/bin/nodejs /usr/bin/node
```

You may verify that /usr/bin/node is indeed a link to version 4.5+ by running:
```sh
/usr/bin/node -v
```

### 4. Install Esprima
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

Ensure you're in the build directory and run:

```sh
./b9run/b9run ./test/hello.b9mod
```

### 6. Test b9

You can run the full b9 test suite with:

```sh
ninja test
```
