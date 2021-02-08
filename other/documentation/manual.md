# sph-sp manual
*draft*

# general
* sph-sp compiles to a shared object library and includes a header file with declarations for programming with c
* usage with other programming languages is possible by creating bindings, either by writing a c wrapper extension for the other language or using a foreign function interface
* some identifiers have prefixes that define a topic or namespace they belong to. for example, function names prefixed with sp_time_ work on data of type sp_time_t

# error handling
* uses [sph-sc-lib status](https://github.com/sph-mn/sph-sc-lib#status), which is included with sph-sp.h. many functions return status_t objects
* prefix: sp_s

# fundamental data types
## sp_sample_t
* quasi continuous values in floating point format
* the c data type is configurable and for example double or float
* about floating point values in general
  * not necessarily equal with =, they can be approximately equal in a small range
  * can store integers without rounding errors (perhaps up to 2**52), but decimal values are subject to errors from rounding necessitated by the limited available size
  * summation, especially of values with large differences, introduces accumulating rounding errors. large types like 64 bit float and greater, reduce the error range
* prefix: sp_sample

## sp_time_t
* discrete values as integers
* the c data type is configurable and for example uint32_t or uint64_t
* used for counts of samples and other integer values
* at 96000 samples per second, 32 bit types can count samples for 44739 seconds or about 12 hours. 64 bit types can count samples for 192153584101141 seconds
* can store only positive values
* subtracting to lower than zero usually subtracts from the largest possible value, addition beyond the maximum value restarts from zero
* prefix: sp_time

## sp_block_t
* sample arrays for multiple channels in one object
* prefix: sp_block

important functions:
~~~
sp_block_new(sp_channel_count_t channel_count, sp_time_t sample_count, sp_block_t* out_block);
status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_time_t rate);
void sp_block_zero(sp_block_t a);
void sp_block_free(sp_block_t a);
~~~

# sample processors
* filtering
* sp_wave
* sp_noise
* sp_cheap_noise
* sp_convolution
* sp_moving_average
* custom blockwise seamless convolution processors

# interpolated paths
* specify two dimensional points with interpolation methods and get arrays for sampled paths between them
* use cases: envelopes and other modulation paths
* inspired by svg paths
* prefix: sp_path

# sequencing with sp_seq
* sequencing with sp_seq is based on the concept of events that occur to affect output in a specific period
* sp_seq receives an output block, one or more events, and a start and end time to generate for
* sp_seq can be called to generate blocks of output with arbitrary start/end time and an arbitrary number of events
* events can be processed in parallel on an arbitrary number of system cpu cores
* sp_seq can be called nested in events and events can be combined in various ways
* sp_seq manages the list of active events and calling event functions requesting output for the relevant offsets and duration, as well as cleanup for past events
* the interface is roughly like this: sp_seq(output_block, events) calls active event.f(valid_start, valid_end, output_block_at_offset)

## events
* sp_event_t variables must be declared with sp_declare_event or initialized with sp_event_set_null unless they are overwritten with other events
* event.generate, event.free and event.prepare are custom functions
* event.state is for custom data that will be available to the functions during the duration of the event
* event.generate by default receive part of the block passed to sp_seq, where they are supposed to sum to, that means, they add to the output block instead of overwriting it
* event parameters can be modulated by the same data by sharing data from a parent context
* event.prepare is called before the event is to be rendered and, with groups, allows to build a nested composition where resources are allocated on demand
* the standard events (wave-event, noise-event, etc) accept channel configuration for muting channels, differing amplitudes and delays between channels

## custom events
* define a struct for use as the event state if needed
* define the event function (event.f) that will generate samples
* define an event.free function to free the custom state when the event has finished
* set .start and .end times as needed

## registering memory with events
* events can receive an arbitrary number of pointers with handler functions that will be called with the pointer on event.free. the default handler is "free" for malloc/calloc allocated memory
* the use case for this is to free memory or deinitialize other resource handles from when the event was prepared
* this could be handled manually using the event state, but this feature makes it generic

### usage
* custom free functions need to call sp_event_memory_free(event) if memory is registered

## group-event
* events can be bundled into a single event. this, for example, could represent one sound of an instrument with many sub-events, or a whole portion of a song, a riff, bar or similar
* the group end time automatically extends to fit the events that are added to it
* groups with nested events/groups can combine the partials of a digital instrument

~~~
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_group_size_t memory_size, sp_event_t* out);
sp_group_memory_add(a, pointer);
sp_group_add(a, event);
~~~

## wave-event
* generate sines or other waves

~~~
sp_wave_state
sp_sine_state
sp_sine_state_lfo
~~~

## noise-event
* generate noise optionally filtered by a single windowed-sinc low/high/band-pass filter
* the modulation interval, the smallest interval in which filter parameter changes over time are applied, is configurable. this has substantial performance implications, as updating a windowed sinc filter for every sample for example would be extremely processing intensive

## cheap-noise-event
* like noise-event but filters less processing intensive using a state-variable filter
* the filter frequency cutoff will as a tradeoff be less steep and a larger range of frequencies will be attenuated until the desired attenuation is reached

## map-event
* post process event output with custom functions
* events usually receive an output block shared by all events. with the isolate option, sub events receive a dedicated block and multiple map-events can post-process specifically the output of one event before being summed with the output of all events
* use case: filter chains, any other post processing of event output

## rendering events to files or arrays
* to file, to plot, to array
* parallel rendering

# utility features
* array helpers
  * generating a list of harmonics
  * generating all sequences of a set
  * shuffle
  * scaling values to fit under a limit
  * scaling values to match a sum
  * counting and analysing subsequences
  * converting intervals to absolute times
* plotting
  * samples
  * times
  * frequency spectrum
* random number generation
  * custom probability functions
* statistics
* the rt macro for sample rate invariant times
* measuring a frequency spectrum

# composing
## how to play instruments
~~~
times: (time ...)
parameters: (struct/variable/any ...)
map times, parameters as x, y
  trigger(x, y) -> event
~~~

* post-processing
* generating events from a list of start times with parameters

# how to
## how to do phase modulation
slight changes of frequency over a short period of time by using the fmod frequency modulation feature of sp-wave-event.

## how could live input data be incorporated
for example with a long running event that on generate reads user input.
user input might have to be read in a separate thread and buffered for the time between one block and the next.
