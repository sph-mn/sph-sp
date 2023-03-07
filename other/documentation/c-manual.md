# c manual
## general
* sph-sp compiles to a shared object library and comes with a header file of declarations for usage in c
* usage with other programming languages is possible by creating bindings, either by writing a c wrapper extension for the other language or using a foreign function interface
* some identifiers have prefixes that define a topic or namespace they belong to. for example, function names prefixed with sp_time_ work on data of type sp_time_t

## units
* times are in number of samples
  * the macros (rt nominator denominator) and (rts nominator denominator) calculate sample rate invariant times based on the sp_rate global variable or _sp_rate preprocessor variable respectively
  * rt(3, 1): 3 seconds
  * rt(1, 4): 1/4 second
* sample rate factor
  * the filter implementations take cutoff and transition value as a decimal
  * 0.5 is the highest possible frequency
  * 0.0 is the lowest possible frequency
* wave frequencies are in hertz
  * 1 hertz is one full period (zero, up, down, zero) in one second

## error handling
* function local goto with an exit label
* integer error number and library name in a struct status_t
* uses [sph-sc-lib status](https://github.com/sph-mn/sph-sc-lib#status), which is included with sph-sp.h
* useful bindings:
  * status_declare: declares a variable with name `status` of type `status_t`
  * status_is_failure: true if status.id is zero
  * status_require(expression): evaluates `status = expression` and jumps to exit on error
  * srq(expression): alias of status_require
  * status_return: same as `return status`

example function layout:

~~~
status_t example() {
  status_declare;
  srq(custom_example());
  exit:
    if (status_is_failure) custom_cleanup;
    status_return;
}
~~~

# fundamental data types
## sp_sample_t
* quasi continuous values in floating point format
* the c data type is configurable and for example double or float
* about floating point values in general:
  * not necessarily equal when compared by `=`. values can be approximately equal in a small range
  * can store integers without rounding errors (perhaps up to 2**52), but decimal values are subject to errors from rounding necessitated by the storage format and the limited available size
  * summation, especially of values with large differences, introduces accumulating rounding errors. large types like 64 bit float and greater reduce the error amount
* prefix: sp_sample

## sp_time_t
* discrete values as integers
* the c data type is configurable and for example uint32_t or uint64_t
* used for counts of samples and other integer values
* at 96000 samples per second, an unsigned 32 bit type can count samples for 44739 seconds or about 12 hours. an unsigned 64 bit type can count samples for 192153584101141 seconds. the former might be sufficient and save memory
* can store only positive values
* subtracting to lower than zero usually subtracts from the largest possible value, addition beyond the maximum value restarts from zero
* prefix: sp_time

## sp_block_t
* sample arrays for multiple channels in one object
* prefix: sp_block
* useful bindings:
  * `status_t sp_block_new(sp_channel_count_t channel_count, sp_time_t sample_count, sp_block_t* out_block)`
  * `status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_time_t rate)`
  * `void sp_block_zero(sp_block_t a)`: sets all values to zero
  * `void sp_block_free(sp_block_t a)`: frees sample arrays for all channels

example
~~~
sp_block_t block;
// 2 channels, 48000 samples long
srq(sp_block_new(2, 48000, &block));
~~~

# interpolated paths
* specify points and interpolation methods and get arrays for points on the path
* use cases: envelopes and other modulation paths
* inspired by svg paths
* functionality provided by [sph-sc-lib spline-path](https://github.com/sph-mn/sph-sc-lib#spline-path)
* prefix: sp_path
* each segment specifies an end point. the start point of each segment is zero or the end of the previous segment
* useful bindings:
  * sp_path_samples_new: returns an array for a path
  * sp_path_line(x, y): return a line segment
  * sp_path_move(x, y): draw to next segment from this point. can be used at the beginning to start at a specific y value, in the middle to jump up or down or to create gaps
  * sp_path_bezier(x1, y1, x2, y2, x3, y3)

example
~~~
sp_sample_t* amod;
sp_path_t amod_path;
sp_path_segment_t amod_segments[2];
amod_segments[0] = sp_path_line(20000, 1.0);
amod_segments[1] = sp_path_line(80000, 0);
spline_path_set(&amod_path, amod_segments, 2);
srq(sp_path_samples_new(amod_path, 80000, &amod));
~~~

## sp_path_curves
interface to create a path of possibly bent lines.
configuration is a struct with separate arrays for the x and y values, and an additional array for curvature values from -1.0 to 1.0.
~~~
sp_path_curves_config_declare(amod_path, 3);
amod_path.x[0] = 0;
amod_path.x[1] = 20000;
amod_path.x[2] = 80000;
amod_path.y[0] = 0;
amod_path.y[1] = 1.0;
amod_path.y[2] = 0;
amod_path.c[0] = 0.5;
amod_path.c[1] = 0;
amod_path.c[2] = 0;
srq(sp_path_curves_samples_new(amod_path, 80000, &amod));
~~~

# sample processors
## filtering
* filter state is allocated if null
* useful bindings:
  * sp_windowed_sinc_lp_hp: low-pass or high-pass filter
  * sp_windowed_sinc_bp_br: band-pass or band-reject filter
  * sp_convolution_filter: convolution with a custom impulse response
  * sp_cheap_filter: low/high/band/reject
  * sp_moving_average

example
~~~
sp_convolution_filter_state_t* state = 0;
// 0.1 cutoff, 0.08 transition band
srq(sp_windowed_sinc_lp_hp(input, duration, 0.1, 0.08, 0, &state, output));
sp_convolution_filter_state_free(state);
~~~

~~~
sp_cheap_filter_state_t state;
// 0 is_multipass
srq(sp_cheap_filter_state_new(duration, 0, &state));
// 0.2 cutoff, 1 pass, 0 q_factor
sp_cheap_filter_lp(input, duration, 0.2, 1, 0, &state, output);
sp_cheap_filter_state_free(&state);
~~~

~~~
// null previous input, null next input, 3 radius
sp_moving_average(input, duration, 0, 0, 3, output);
~~~

## other
* useful bindings
  * sp_noise: generate white noise
  * sp_convolve: discrete linear convolution
  * sp_fft: fast fourier transform (converts from sounds to loudness values for frequencies)
  * sp_ffti: inverse fast fourier transform (converts from loudness values for frequencies to sounds)

~~~
sp_noise(&sp_default_random_state, duration, output);
// carryover is a sample array and its length must be at least b_len - 1
// 48000 duration, 10 impulse response length, 9 carryover length
sp_convolve(input, 48000, impulse_response, 10, 9, carryover, output);
// fft and the inverse. calculated in place, imaginary_part can be all zeros
status_i_require(sp_fft(len, input_outupt, imaginary_part));
status_i_require(sp_ffti(len, input_output, imaginary_part));
~~~

# sequencing with sp_seq
* based on a concept of events that occur to affect output in a certain period
* sp_seq receives an output block, one or more events, and a start and end time to generate for
* events and event subtrees can be processed in parallel on an unlimited number of system cpu cores. this is a feature of sp_seq_parallel, which has the exact same function type signature as sp_seq
* sp_seq can be called nested inside events and therefore events can be composed in various ways
* sp_seq manages the list of active events and calls event functions to prepare, generate blocks, and eventually free events
* it works roughly like this: sp_seq(start, end, output_block, events) calls any active event.f(relative_start, relative_end, output_block_for_result)

## events
* sp_event_t variables must be declared with sp_declare_event or initialized with sp_event_reset unless they are overwritten with other events that have been declared in such a way
* event.prepare, event.generate and event.free are custom functions
* event.config and event.data is for custom data that will be available to the functions during the duration of the event
* event.prepare is called before the event is to be rendered and, together with groups, allows to build a nested composition of event where needed resources will be allocated on demand
* event.generate receives part of the block passed to sp_seq and is supposed to sum into. that means, events are supposed to add to a given output block instead of overwriting it
* the core sound events (wave-event, noise-event, etc) accept channel configuration for muting channels, amplitude differences, and delays between channels

## custom events
* define a struct for use as the event configuration if needed
* define the event function event.f that will generate samples
* define an event.free function to free the custom event.config or event.data when the event has finished
* set .start and .end times as needed

## registering memory with events
* events can track pointers with handler functions that will be called with the pointer on event.free
* the use case for this is to free memory, or deinitialize other resources, when the event ends
* custom free functions need to call sp_event_memory_free(event) if memory has been registered

~~~
status_t sp_event_memory_add(sp_event_t*, void*));
status_t sp_event_memory_add_with_handler(sp_event_t*, void*, void(void*)));
~~~

## calculated event durations
* events whose prepare function defines their duration have to be prepared before being used with sp_seq. see also the binding sp_event_prepare
* for events where end is zero are automatically copied and prepared when added to groups or when passed to sp_render_file or sp_render_plot. this feature eases usage of group events of variable length because no fixed times have to be known and specified when composing them

## group-event
* events can be bundled into a single event. this, for example, could represent one sound of an instrument with many sub-events, or a whole portion of a song, a riff, bar or similar
* sp_group_add automatically extends the group end time for events that are added to it
* groups with nested events/groups can, for example, combine the partials of a digital instrument

~~~
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_group_size_t memory_size, sp_event_t* out);
status_t sp_group_add(sp_event_t* a, sp_event_t event);
status_t sp_group_append(sp_event_t* a, sp_event_t event);
~~~

## wave-event
* generate sines or other waves
* sp_sine_table is used as the default waveform
* for low frequency sines there is the wave table sp_sine_table_lfo of length sp_rate times sp_sine_lfo_factor

## noise-event
* generate white noise, optionally filtered by a single windowed-sinc low/high/band-pass filter
* the modulation interval, the smallest interval in which filter parameter changes over time are applied, is configurable. this has substantial performance implications, as updating a windowed sinc filter for every sample would be extremely processing intensive

## cheap-noise-event
* like noise-event but filters less processing intensive using a state-variable filter
* compared to noise-event, the filter frequency cutoff will be less steep

## map-event
* post process event output with custom functions
* events usually receive an output block shared by all events. but with the isolate option, sub events receive a dedicated block. then multiple map-events can post-process specifically the output of one event before it is summed with the output of all events
* use case: filter chains, any other post processing of event output

## sound-event
* uniform interface to noise-event and wave-event

* sound_event_config_t.noise is 0 for a sine wave, 1 for windowed sinc filtered noise, 2 for state-variable filtered noise
* sound_event_config_t.frq is the frequency, or low frequency cutoff for filtered noise, in hertz
* .wdt is the bandwidth for noise, with .frq being the lowest frequency. wmod is a modulation array (wdt(t)) for .wdt

## rendering events to files or arrays
* to file, to plot, to array

example
~~~
sp_block_t block;
sp_render_config_t rc = sp_render_config(sp_channels, sp_rate, sp_rate);
// 0 start, 48000 duration. block memory is allocated
srq(sp_render_block(event, 0, 48000, rc, &block));
~~~

## parallelization
* is done by setting the generate function of a group to sp_group_prepare_parallel
* events are distributed over cores. cpu core threads take individual events from a queue when they need new tasks. there is currently no processing of future events outside the currently rendered block

# overview of some included utility features
* array helpers
  * generating a list of harmonics
  * generating all permutations or sequences of a set
  * shuffle
  * scaling values to fit under a limit
  * scaling values to match a sum
  * counting and analyzing subsequences
  * converting intervals to absolute times
* plotting samples, times or frequency spectra
* random number generation with custom probability functions
* statistics

# how to
## how could live input data be incorporated
for example with a long running event that on generate reads user input.
user input might have to be read in a separate thread and buffered for the time between one block and the next.

## how to do phase modulation
slight changes of frequency over a short period of time by using the fmod frequency modulation feature of sp-wave-event.

# caveats
## stack allocating event config
event config like sp_wave_event_config_t and similar needs to be available when the event prepare function is called.
unless the event is directly prepared in a sub function, it should be heap allocated, for example with sp_wave_event_config_new, and tracked with sp_event_memory_add.

## not resetting event variables when reusing them
local sp_event_t variables should be reset with sp_event_reset(event) or with by overwriting it with an already reset event.
otherwise tracked event memory might be duplicated and lead to memory errors when the event is added to a group, for example.

# troubleshooting
issues, possible causes and solutions.

## floating point exception
* has sp_initialize() been called once to initialize sph-sp? it must be called once for some features to work

## double free or corruption
* if sp_event_memory_fixed_add has been used, the memory for registering memory to be freed may not have been initialized large enough. call sp_event_memory_ensure with a large enough number

## segmentation fault
possible causes
* amod or fmod arrays shorter than event duration
