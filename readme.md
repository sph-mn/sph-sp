# sph-sp
c code and shared library for sound synthesis and sequencing. the sound processor implementations can serve as examples.

# features
* basics
  * 32 bit float wav file output and input
  * 64 bit float sample format by default, other formats possible for many processors
  * processing of non-interleaved sample arrays with one array per channel
  * unlimited number of channels and unlimited sample rate
  * fast fourier transform (fft) and inverse fast fourier transform (ifft)
  * plotting of samples and sound spectra using gnuplot
* processors that work seamlessly on blocks of continuous data streams
  * windowed-sinc low-pass, high-pass, band-pass and band-reject filters
  * state-variable high/low/band/all-pass filter
  * moving average filter
  * convolution
* synthesis
  * paths created from interpolation between given points. can be used for amplitude evelopes and modulation
  * lookup-table oscillator for sines and other wave shapes with a stable phase, and time time-dependent frequency and amplitude changes given by arrays
  * white noise generator
* sequencing
  * event renderer for parallel block processing with custom routines
  * events for filtered noise and wave output
  * event groups that compose for instruments, riffs and songs
  * per channel configuration with optional delay
* array processing
  * utilities like array arithmetic, shuffle, permutations, compositions, statistics, and more

# code example
* see [other/example.c](other/example.c)
* or alternatively [other/example.sc](other/example.sc)

# documentation
* [c manual](other/documentation/c-manual.md)
* [sc manual](other/documentation/sc-manual.md)
* [api listing](other/documentation/api.md)

# dependencies
* linux or compatible (libc)
* gcc and shell for the provided compile script
* optional: gnuplot
* optional: [sph-sc](https://github.com/sph-mn/sph-sc), for development on sph-sp itself or when using sc

# setup
* install dependencies
* execute the following from the project directory

```
./exe/compile-c
./exe/install
```

the first argument to `exe/install` can be the destination path prefix, for example `./exe/install /tmp`.

installed files
* /usr/include/sph-sp.h
* /usr/lib/libsph-sp.so

# compile-time configuration options
the `ifndef`s at the top of `source/c-precompiled/main/sph-sp.h` can be customised before compilation. custom preprocessor variables can be set before including the header when embedding the full code, or a customized header must be used when using a shared library which has to be compiled with the same configuration.

some options that can be configured:

| name | default | description |
| --- | --- | --- |
|sp_channel_limit|2|maximum number of channels|
|sp_channel_count_t|uint8_t|data type for numbers of channels|
|sp_sample_t|double|float data type for samples (quasi continuous)|
|sp_time_t|uint32_t|integer data type for sample counts (discrete)|

# c usage
```
#include <sph-sp.h>
```
call `sp_initialize(cpu_count, default_channel_count, default_sample_rate)` once before using other features. for example `sp_initialize(1, 2, 48000)`.

compilation of programs using sph-sp with gcc
```
gcc -lsph-sp main.c
```

# license
* files under `source/c/foreign/nayuki-fft`: mit license
* rest: lgpl3+

# thanks to
* [tom roelandts](https://tomroelandts.com/) on whose information the windowed sinc filters are based on
* [mborg](https://github.com/mborgerding/kissfft) for the first fft implementation that was used
* [mhroth](https://github.com/mhroth/tinywav) for a simple example of reading/writing wav files
* [nayuki](https://www.nayuki.io/page/free-small-fft-in-multiple-languages) for the concise fft implementation that is currently used
* [steve smith's dspguide](http://www.dspguide.com/) for information about dsp theory
* [xoshiro.di.unimi.it](http://xoshiro.di.unimi.it/) for the pseudorandom number generator

# similar projects
* [clm](https://ccrma.stanford.edu/software/snd/snd/clm.html)
* [puredata](https://puredata.info/)
* csound, supercollider, cmix, cmusic, and arctic