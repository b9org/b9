[![Build Status](https://api.travis-ci.org/youngar/Base9.svg?branch=master)](https://travis-ci.org/eclipse/omr)

Cloning
-------
To quickly clone the source code, use:
```
git clone --recursive --shallow-submodule --depth=1 https://github.com/youngar/Base9
```

Building
--------
You may not need to disable warnings as errors to build.
```
cd omr && ./configure --disable-warnings-as-errors
make -C omr/jitbuilder
make
```

