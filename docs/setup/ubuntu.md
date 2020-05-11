---
title: Ubuntu Setup
section: setup
---

Depending on your user privileges, you may have to precede some of the
commands in the steps below with `sudo`.

## Requirements

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

## Clone the repository and get the submodules

```sh
git clone --recursive https://github.com/b9org/b9.git
```

## Upgrade node.js

If you are running Ubuntu 16.04 and older, your nodejs may be older than version 4.5, in
which case you will need to upgrade it. You can install newer nodejs binaries from 
[NodeSource](https://nodesource.com/)'s PPA using:

```sh
curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash - \
&& sudo apt-get install -y nodejs
```

Check out [this](https://nodejs.org/en/download/package-manager) guide for more information 
on obtaining node.js packages.

You may run `nodejs -v` to verify that your nodejs version is 4.5.0 or above.

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

Ensure you're in the build directory and run:

```sh
./b9run/b9run ./test/hello.b9mod
```

## Test b9

You can run the full b9 test suite with:

```sh
ninja test
```
