# sph-sp
c code and shared library for sound synthesis and sequencing. the sound processor implementations can also serve as examples.

# features
* basics
  * processing of non-interleaved sample arrays with one array per channel
  * unlimited number of channels and unlimited sample rate
  * 64 bit float sample format by default, other formats possible for many processors
  * 32 bit float wav file output and input
  * plotting of samples and sound spectra using gnuplot
  * fast fourier transform (fft) and inverse fast fourier transform (ifft)
* processors that work seamlessly on blocks of continuous data streams
  * windowed-sinc low-pass, high-pass, band-pass and band-reject filters
  * convolution
  * moving average filter
* synthesis
  * linear and bezier interpolation between points for envelopes and other control data
  * lookup-table oscillator for sinusoids and other wave shapes with a stable phase and time-dependent frequency and amplitude changes provided by arrays
  * white noise generator
* sequencing
  * event renderer for parallel block processing with custom routines
  * events for sines and filtered noise
  * composable event groups for instruments, riffs and songs
  * per channel configuration and channel delay
* array processing
  * arithmetic, combinations, statistics, and more

# code example
see [other/examples/example.c](other/examples/example.c).

alternative c syntax: [wisp-sc](other/examples/example.scw), [sc](other/examples/example.sc)

# sound example
these files came about during testing in development: [1](https://sph.mn/auditive_visual/sound/2022/2022-12-10.15814.flac), [2](https://sph.mn/auditive_visual/sound/2022/2022-12-10.07345.flac), [3](https://sph.mn/auditive_visual/sound/2022/2022-12-10.09439.flac), [4](https://sph.mn/auditive_visual/sound/2022/2021-04-12.05370.flac) (one stream of non-repeating patterns per channel).

# documentation
* [c manual](other/documentation/c-manual.md)
* [sc manual](other/documentation/sc-manual.md)
* [api listing](other/documentation/api.md)

# dependencies
* linux or compatible (libc)
* gcc and shell for the provided compile script
* optional: gnuplot
* optional: [sph-sc](https://github.com/sph-mn/sph-sc), for development on sph-sp itself or for using the sc macros

# setup
* install dependencies as needed
* execute the following from the project directory

```
./exe/compile-c
./exe/install
```

the first argument to `exe/install` can be the destination path prefix, for example `./exe/install /tmp`.

installed files
* /usr/include/sph-sp.h
* /usr/include/sph-sp/*
* /usr/lib/libsph-sp.so
* /usr/share/sph-sp/sc-macros.sc

# compile-time configuration options
the `ifndef`s at the top of `source/c-precompiled/sph-sp/sph-sp.h` can be customised before compilation. custom preprocessor variables can be set before including the header when embedding the full code, or a customized header must be used when using a shared library that has been compiled with the same configuration.

some options that can be configured:

| name | default | description |
| --- | --- | --- |
|sp_channel_count_t|uint8_t|data type for numbers of channels|
|sp_channel_limit|2|maximum number of channels|
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