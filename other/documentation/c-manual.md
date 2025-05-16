# c manual

see also [example.c](../examples/example.c).

## general
- sph-sp compiles to a shared object library and comes with a header file of declarations for usage in c
- usage with other programming languages is possible by creating bindings, either by writing a c wrapper extension for the other language or using a foreign function interface
- some identifiers have prefixes that define a topic or namespace they belong to. for example, function names prefixed with sp_time_ work on data of type sp_time_t

## units
- times are in number of samples
  - the macros sp_duration(n, d) and sp_rate_duration(n, d) calculate sample rate invariant times based on the global variable sp_rate
  - sample rate factor
    - filter implementations take cutoff and transition as a decimal
    - 0.5 is the highest possible frequency
    - 0.0 is the lowest possible frequency
- wave frequencies are in hertz
  - 1 hertz is one full period (zero, up, down, zero) in one second

## error handling
- function local goto with an exit label
- integer error number and library name in a struct status_t
- uses [sph-sc-lib status](https://github.com/sph-mn/sph-sc-lib#status), which is included with sph-sp.h
- useful bindings:
  - status_declare: declares a variable with name "status" of type "status_t"
  - status_is_failure: true if status.id is zero
  - status_require(expression): evaluates "status = expression" and jumps to exit on error
  - srq(expression): alias of status_require
  - status_return: same as "return status"

example function layout:
```c
status_t example() {
  status_declare;
  srq(custom_example());
  exit:
    if (status_is_failure) custom_cleanup;
    status_return;
}
```

# fundamental data types

## sp_sample_t
- quasi continuous values in floating point format
- the c data type is configurable and for example double or float
- floating point values can have rounding errors and summation imprecision
- prefix: sp_sample

## sp_time_t
- discrete values as integers
- can be uint32_t or uint64_t
- used for counts of samples and other integer values
- prefix: sp_time

## sp_block_t
- sample arrays for multiple channels in one object
- prefix: sp_block
- useful bindings:
  - "status_t sp_block_new(sp_channel_count_t channel_count, sp_time_t sample_count, sp_block_t* out_block)"
  - "status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_time_t rate)"
  - "void sp_block_zero(sp_block_t a)": sets all values to zero
  - "void sp_block_free(sp_block_t a)": frees sample arrays for all channels

example
```c
sp_block_t block;
// 2 channels, 48000 samples long
srq(sp_block_new(2, 48000, &block));
```

# interpolated paths
- specify points and interpolation methods and get arrays for points on the path
- use cases: envelopes and other modulation paths
- functionality is provided by calls like sp_path_samples or sp_envelope_scale
- "sp_path_samples(...)" calculates a sample array from points
- "sp_envelope_zero(...)" or "sp_envelope_scale(...)" provide alternative ways of defining envelopes

example
```c
sp_sample_t* amod;
// allocate and define arrays x, y, c etc. for interpolation
// generate an array of length 80000
srq(sp_path_samples(&amod, 80000, point_count, x, y, c));
```

# sample processors

## filtering
- filter state is allocated if null
- useful bindings:
  - sp_windowed_sinc_lp_hp: low-pass or high-pass filter
  - sp_windowed_sinc_bp_br: band-pass or band-reject filter
  - sp_convolution_filter: convolution with a custom impulse response
  - sp_cheap_filter: low/high/band/reject
  - sp_moving_average

example
```c
sp_convolution_filter_state_t* state = 0;
// 0.1 cutoff, 0.08 transition band
srq(sp_windowed_sinc_lp_hp(input, duration, 0.1, 0.08, 0, &state, output));
sp_convolution_filter_state_free(state);
```

```c
sp_cheap_filter_state_t state;
// 0 is_multipass
srq(sp_cheap_filter_state_new(duration, 0, &state));
// 0.2 cutoff, 1 pass, 0 q_factor
sp_cheap_filter_lp(input, duration, 0.2, 1, 0, &state, output);
sp_cheap_filter_state_free(&state);
```

```c
// null previous input, null next input, 3 radius
sp_moving_average(input, duration, 0, 0, 3, output);
```

## other
- useful bindings:
  - sp_noise: generate white noise
  - sp_convolve: discrete linear convolution
  - sp_fft / sp_ffti: fast fourier transform and inverse

```c
sp_noise(&sp_default_random_state, duration, output);
// 48000 duration, 10 impulse response length, 9 carryover length
sp_convolve(input, 48000, impulse_response, 10, 9, carryover, output);
status_i_require(sp_fft(len, input_output, imaginary_part));
status_i_require(sp_ffti(len, input_output, imaginary_part));
```

# sequencing with sp_seq
- based on a concept of events that occur to affect output in a certain period
- sp_seq receives an output block, one or more events, and a start and end time to generate for
- events and event subtrees can be processed in parallel with sp_seq_parallel
- events can be nested in groups
- sp_seq manages the list of active events and calls event functions to prepare, generate blocks, and eventually free events

## events
- must be declared with sp_declare_event or reset with sp_event_reset
- event.prepare is called before the event is rendered
- event.generate receives part of the block passed to sp_seq and should sum into it
- wave-event and noise-event accept channel configuration (amp, freq, etc.)

## custom events
- define a config struct if needed
- define event.f, event.free, etc.
- register memory to be freed at event end with sp_event_memory_add or sp_event_memory_add_with_handler

## calculated event durations
- events whose prepare function defines their duration have to be prepared before use
- if event.end is zero, it can be auto-prepared when added to groups or passed to rendering functions

## group-event
- bundles events into one event
- sp_group_add automatically extends the group end time

```c
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_group_size_t memory_size, sp_event_t* out);
status_t sp_group_add(sp_event_t* a, sp_event_t event);
status_t sp_group_append(sp_event_t* a, sp_event_t event);
```

## wave-event
- generates sines or other waves
- default waveform is sp_sine_table
- for low frequency sines there is sp_sine_table_lfo

## noise-event
- generates white noise, optionally filtered by a single windowed-sinc low/high/band-pass filter
- cheap-noise-event uses a state-variable filter

## map-event
- post-processes event output with custom functions
- can isolate sub-event output to a dedicated block and then sum into the main output

# how to
## how could live input data be incorporated
a long running event can read user input in its generate call. data might need buffering between blocks.

## how to do phase modulation
slight changes of frequency over a short period of time can be done via fmod with sp_wave_event.

# caveats
## stack allocating event config
config structs like sp_wave_event_config_t should remain valid while the event is used. if not prepared immediately, they should be heap allocated and tracked with sp_event_memory_add.

## not resetting event variables when reusing them
local sp_event_t variables should be reset with sp_event_reset before reuse.

# troubleshooting
## floating point exception
- has sp_initialize() been called once?

## double free or corruption
- if sp_event_memory_fixed_add was used, ensure the memory register size was large enough

## segmentation fault
- arrays shorter than event duration (e.g. amod, fmod) can cause out-of-bounds access
```