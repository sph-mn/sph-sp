# sph-sp
basic utilities for digital sound processing as a shared library and c code.
reference implementation for learning and, with some extra testing, practical usage.
prefers higher precision to faster calculation.

# features
* generic input/output port for sound data for alsa and au format files
* 32 bit float samples
* convolution function
* windowed sinc filter
* moving average filter
* processors work for segments from continous streams
* avoids and compensates for rounding errors
* sample format customisable to some extent

# dependencies
* run-time
  * alsa
  * libc
* quick build
  * gcc and shell for the provided compile script
* development build
  * sph-sc

# installation
```
./exe/compile-c
./exe/install
```

# usage
```
#include <sph-sp.c>
```
