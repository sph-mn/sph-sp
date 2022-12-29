# sph-sp
c code and shared library for sound synthesis and sequencing. the sound processor implementations can serve as examples.

# features
* basics
  * 32 bit float wav file output
  * 64 bit float sample format by default, other formats possible for many processors
  * processing of non-interleaved sample arrays with one array per channel. number of channels and sample rate can be custom set
  * fast fourier transform (fft) and inverse fast fourier transform (ifft)
  * plotting of samples and sound spectra using gnuplot
* processors that work seamlessly on blocks of continuous data streams
  * windowed-sinc low-pass, high-pass, band-pass and band-reject filters
  * state-variable high/low/band/all-pass filter
  * moving average filter
  * convolution
* synthesis
  * paths created from interpolation between given points. can be used for amplitude, wavelength and other controls
  * lookup-table oscillator for sines and other wave shapes with a stable phase, arrays for time-dependent frequency and amplitude changes
  * sine/triangle/square/sawtooth-wave and noise generator
* sequencing
  * event renderer for parallel block processing with custom routines
  * events for filtered noise and wave output
  * event groups that compose for riffs and songs
  * per channel configuration with optional delay
* array processing
  * utilities like array arithmetic, shuffle, permutations, compositions and statistics such as median, deviation, skewness and more

# code example
sph-sp is written in [sc](https://github.com/sph-mn/sph-sc), which maps directly to c, and c is fully supported. except for the fact that sc supports scheme style macros and when using sc these can be used to simplify usage.

* see [other/example.sc](other/example.sc) for how it currently looks using sc
* see [other/example.c](other/example.c) for the c version. there are several things c can not express as succinctly as sc, mostly related to literals and variable number of arguments

# documentation
* [c manual](other/documentation/c-manual.md)
* [sc manual](other/documentation/sc-manual.md)
* [api listing](other/documentation/api.md)

otherwise see the comments above function definitions in the code.

# dependencies
* run-time
  * libc
  * libsndfile
  * linux or compatible
  * optional: gnuplot
* quick build
  * gcc and shell for the provided compile script
* development build
  * [sph-sc](https://github.com/sph-mn/sph-sc)

# setup
* install dependencies
* then in the project directory execute

```
./exe/compile-c
./exe/install
```

first argument to `exe/install` can be the destination path prefix, for example `./exe/install /tmp`.

installed files
* /usr/include/sph-sp.h
* /usr/lib/libsph-sp.so

# compile-time configuration options
the `ifndef`s at the top of `source/c-precompiled/main/sph-sp.h` can be customised before compilation. custom preprocessor variables can be set before including the header when embedding the full code, or a customized header must be used when using a shared library which has to be compiled with the same configuration.

some options that can be configured:

| name | default | description |
| --- | --- | --- |
|sp_channel_limit|2|maximum number of channels|
|sp_channels_t|uint8_t|data type for numbers of channels|
|sp_file_format|(SF_FORMAT_WAV \| SF_FORMAT_DOUBLE)|file format to use for output. a combination of soundfile constants for file format and sample format, for example (SF_FORMAT_AU \| SF_FORMAT_DOUBLE). output format conversion is done automatically as necessary|
|sp_sample_t|double|float data type for samples (quasi continuous)|
|sp_time_t|uint32_t|integer data type for sample counts (discrete)|

# c usage
```
#include <sph-sp.h>
```
call `sp_initialize(cpu_count, default_channel_count, default_sample_rate)` once somewhere. for example `sp_initialize(1, 2, 48000)`.

compilation with gcc
```
gcc -lsph-sp main.c
```

# license
* files under `source/c/foreign/nayuki-fft`: mit license
* rest: lgpl3+

# possible enhancements
* split header into a core and an extra header

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