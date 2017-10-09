[![Build Status](https://api.travis-ci.org/youngar/Base9.svg?branch=master)](https://travis-ci.org/youngar/Base9)

# Building Base9

## Grab the source

clone this repository using git:

```sh
git clone https://github.com/youngar/Base9
cd Base9
# Download sub-modules: googletest and omr
git submodule update --init
```

## Building

### Set up the build directory

Create a build directory and configure the build. Base9 and OMR use Cmake.
We suggest using the ninja generator.

```sh
# Inside the Base9 directory
mkdir build && cd build
cmake -G Ninja ..
```

### Build it!

```sh
# Inside the build directory
ninja -j2
```

## Testing

To run the base9 tests, run:
```sh
# Inside the build directory
ctest -V
```

## Benchmarking

To run the benchmark, use:
```
ninja bench
```
