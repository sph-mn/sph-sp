# negative frequencies
* negative frequencies are possible when sph-sp is compiled with sp_time_t set to a signed type, either integer or floating point
* frequency modulation commonly uses negative frequencies to specify phase inverted frequencies

# how to do frequency modulation synthesis
* option
  * compile sph-sp with sp_time_t set to a signed int type, for example int32_t. negative frequencies are then possible
* option
  * create a sp_sample_t array for the modulator
  * create a sp_time_t array for the fmod parameter with the absolute values from the modulator (negative frequencies mapped to positive frequencies)
  * create a sp_time_t array for the pmod parameter with any modulator index with a negative frequency is set to
  * use these created fmod and pmod
  option
    create a custom event that uses the output of sp-wave-event internally
    writes to empty blocks and uses the block output to create fmod and pmod for next operator
    option
      use settings in generate function directly to avoid block allocation
    configuration
      sp-wave-event-config-t
      modulates
    pro
      supports all wave event settings for all operators
    con
      memory intensive
  option
    use sp-sample-t for frequency and phase

# how to add a channel delay
* allocate the longest amod/fmod/pmod and pass earlier or later pointers to the channel-config

# channel configuration
per channel configuration for events, with defaults taken from the first channel

## purpose
* trivial configuration of simulated human hearing localization effects without having to create groups and multiple events per channel
* it is assumed that these effects are so common that they should be part of the core event configuration

## semantics
* the first channel is the root channel, other channel are either copies of the first (if use is false) or overrides if values are set (if use is true)
* only pointer values are optional and can be set to null, other values have to be set
* field channel-count allows to target only the number of needed channels
* field channel allows to choose the target channel
* channels can not share more than the first channels state. there is no nested hierarchy of channel config inheritance
* all unselected channels are ignored. these are the channels that fall outside the range of channel-count or are not selected by the channel configuration in the range of channel-count

## implementation
* preprocessor variable for number of maximum channel-count
* this way everything will be allocated with the configuration type
* a high number of maximum channels that is rarely used wastes space
* increasing the number of channels that can be simultanously generated with a single sound-event requires recompilation. however, multiple events for any number (limited by sp_channel_count_t) of simultaneous channels is possible

## alternatives
* separate events per channel
  * pro
    * reduced code complexity
    * channels can be parallel-processed (not that useful because there is usually more than one event anyway)
  * con
    * more allocations (event state/configuration (phs for waves, buffers and filter-state for noise), event-list) and nested sp-seq calls

# merge noise-event and cheap-noise-event
* they could use the same configuration type, allocators, and so on
* only few parameters differ, although cheap-noise works a bit differently
* some parameters can not be normalized, like "passes"
  * the filter state type is different
  * the decision basis is the number of fields that can only be used for one type and make the use of the other type less (space) efficient
* pro
  * same interface makes using these types together, or exchanging them, easier
* con
  * separate, optimized type definitions are always more efficient and even quite simple to manage

# sound-event
a single generic event for waves and noise

conclusion
* for the most efficient wave and noise events, they must exist as separate events with their own minimal configuration type
* the interface for these events can be normalized and make it easier for a function like sound-event to prepare them either using a stack allocated combined configuration or similar
* in any case, if there is a specific issue to solve, the abstraction should be made on the simple cases. as of yet, it is unclear what exactly the content of this abstraction would be, and if the result (switching between noise/wave easily, setting amp/amod/frq/fmod only once) is worth it

goals
* only one function or macro prepares either
* one variable decides if wave or noise
* shared and normalized parameters like frq and amp that do not have to be repeated in branches when allocating for either noise or wave

* pro
  * separate events for wave, noise, and cheap-noise do not have to exist. although similar prepare functions will
    * no seperate -defaults/-config-new-n/-config-new bindings
  * convenient to configure the parameters amp/amod/frq/fmod for any event once
  * select wave or noise by boolean
  * normalized interface and frq and fmod do not have to be converted, amp and amod only set once. however, the amp for noise probably does have to be adjusted because of gain loss.
  * effect of sounds devolving into noise by partial is easy
* con
  * the simplest wave-event partial will still allocate noise-event config memory
  * the combined struct brings considerable complexity just for not having to allocate separate configuration types. such a structure is usually a sign for an inefficient abstraction

overhead is a branch in sp-sound-event-prepare deciding which sub prepare function to use

## how to define the configuration type
how to avoid having to allocate different configuration types for wave or noise

note that there are only few shared fields, mostly amp/amod/frq/fmod

* option
  * nested-unions
  * union fields for value groups that differ between subtypes
  * channel-config and type-channel-config have to be handled separately but it is very similar to having only one type
  * pro
    * values can and must be set only when used
    * more space efficient than flat-merged
    * extra code for accessing the union fields
  * structure
    * general-config
    * shared-channel-config
    * type-config
    * type-channel-config
  * alternative structure
    * general-config
    * type-config
    * type-channel-config (can not set shared-channel-config once and then type specific config)
* option
  * flat-merged
  * all possible config values concatenated into one struct
  * pro
    * values can always be set and are only used when needed
    * needs about 70% more space compared to nested-unions
    * less code needed for use and implementation
* option
  * generic parameters in a separate struct type
  * set separately and copied into the specific type
  * types
    * generic-channel-config-t (amp amod frq fmod channel use)
    * noise-t (channel-count generic-channel-config channel-config state)
    * wave-t (channel-count generic-channel-config channel-config state)
* option
  * fully separate config types
  * con
    * necessary pre-prediction of what partials will be noise and which will not for exact bulk allocation of event configuration
    * increases the number of allocations, either for per event allocation or bulk allocation, but only if mixed event types are used

### implementation example comparison
~~~
sp-sound-event-wave-channel-config-t (type (struct (phs sp-time-t) (pmod sp-time-t*)))
sp-sound-event-wave-config-t
(type
  (struct
    (wvf-size sp-time-t)
    (wvf sp-sample-t*)
    (channel-count sp-channel-count-t)
    (channel-config (array sp-sound-event-wave-channel-config-t sp-channel-count-limit))))
sp-sound-event-noise-channel-config-t
(type
  (struct
    (filter-state void*)
    (is-reject uint8-t)
    (passes sp-time-half-t)
    (trnl sp-sample-t)
    (trnh sp-sample-t)
    (wdt sp-time-t)
    (wmod sp-time-t*)))
sp-sound-event-noise-config-t
(type
  (struct
    (resolution sp-time-t)
    (random-state sp-random-state-t)
    (temp (array sp-sample-t* 3))
    (channel-count sp-channel-count-t)
    (channel-config (array sp-sound-event-noise-channel-config-t sp-channel-count-limit))))
sp-sound-event-type-config-t
(type (union (wave sound-event-wave-config-t) (noise sound-event-noise-config-t)))
sp-sound-event-channel-config-t
(type
  (struct
    (amod sp-sample-t*)
    (amp sp-sample-t)
    (channel sp-channel-count-t)
    (fmod sp-time-t*)
    (frq sp-time-t)
    (use sp-bool-t)))
sp-sound-event-config-t
(type
  (struct
    (type uint8-t)
    (type-config sp-sound-event-type-config-t)
    (channel-count sp-channel-count-t)
    (channel-config sp-sound-event-channel-config-t sp-channel-count-limit)))
~~~

flat-merged
~~~
sp-sound-event-channel-config-t
(type
  (struct
    (amod sp-sample-t*)
    (amp sp-sample-t)
    (channel sp-channel-count-t)
    (fmod sp-time-t*)
    (frq sp-time-t)
    (use sp-bool-t)
    (filter-state void*)
    (is-reject uint8-t)
    (passes sp-time-half-t)
    (trnl sp-sample-t)
    (trnh sp-sample-t)
    (wdt sp-time-t)
    (wmod sp-time-t*)))
sp-sound-event-config-t
(type
  (struct
    (type uint8-t)
    (resolution sp-time-t)
    (random-state sp-random-state-t)
    (temp (array sp-sample-t* 3))
    (wvf-size sp-time-t)
    (wvf sp-sample-t*)
    (channel-count sp-channel-count-t)
    (channel-config sp-sound-event-channel-config-t sp-channel-count-limit)))
~~~

# what parameters of core events should possibly be time variable
* in traditional software pretty much everything can be time dependent
* allowing time dependence is a performance consideration because in sph-sp it needs at least one extra null pointer check per generated sample (per channel)

# moving average filter
* use cases: smoothing of paths, smoothing of input control streams
* not as useful: frequency filtering
* options
  * take previous/next data block
  * mirror data before/after start/end
  * take offset argument to control where to take previous data from
  * centered
    * good for smoothing finite, ready, paths in a single array
  * right aligned - has a starting delay. as does centered
    * does only need left portion for incremental processing
    * introduces shift. might be possible to remove afterwards
* option
  * centered for smoothing, mirrors after start/end for finite data
  * right aligned for incremental filtering. takes previous input for overlap portion

# group parallelization
* option: replaced group generate function - selected
  * least overhead
* option: event or group config attribute
* option: render argument

# event start/end vs start/duration
start/end
* selected because sp_seq needs end frequently. `if(end <= t) {continue;} else if(end <= t) {break;}`
* easy to check when event ends without having to calculate start + end
* duration has to be calculated with end - start where needed
* theoretically possible that end is set to a smaller value than start
* to shift event in time start and end have to be modified

start/duration
* to shift event in time only start has to be modified

# should seq return the event offset
* selected: no, extra declaration/count/return effort is even easier handled if needed in caller

# notes
* event.generate start/end times are event relative
* event.generate must never be called outside the event range
* out is positioned with index 0 at the current event output start. this ensures that seq and event.generate behave the same when nested
* events should be freed as soon as possible
* noise: non-dependent values and no apparent repetition
* wave: dependent values (values depend on previous value) with repetition
  * "movement marked by the regulated succession of strong and weak elements, or of opposite or different conditions"
* "a pulse must decay to silence before the next occurs if it is to be really distinct. for this reason, the fast-transient sounds of percussion instruments lend themselves to the definition of rhythm"
* perfect square waves create aliasing because they contain frequencies that can not be represented

# patterns
* allocate configuration for multiple events in one array and free with a group that contains the events
  * avoids allocations

# general philosophy
* configuration driven, no post-processing
* [design by contract](https://en.wikipedia.org/wiki/Design_by_contract)
* no function-domain compatibility assertions to make higher-level abstractions more efficient. the higher-level abstractions could make the assertions if needed
* only use hierarchy for sharing configuration and state. parents share with derivating children. there is no sharing between children, no post-processing, no post analyis, no child analysis

# meter
* a raster for periodic events to fall on
* not necessarily sounded but expected by the listener
* a frequency of possible recurrence
* traditional meter notation incorporates beat count. 4/8 is the same as 2/4
* irrational meters have bars whose start/end never meet again

# processors sum into output without floating point error correction
* option - selected
  * use a large float format that keeps errors smaller
* option
  * sum all events into their own buffers
  * later sum with error correction

# normalizing gain after filtering
filters lower the volume of the sound

* option
  * find a multiplier that matches the current filters reduction of amplitude
* option
  * normalize to amp for each block
  * this is not seamless at block boundaries as different blocks will still have different averages and volumes

# event configuration data structure
* option - selected
  * pre-defined structs
  * most efficient because offsets are calculated at compile time
* option
  * collection of name-value associations, for example a hash table
  * parents would not know what names are possible

# why not use a float type for all number types
* pro
  * all integers up to 2**52 exactly representable, and comparable
  * f64 is enough for time
  * dont have to implement array functions twice
* con
  * integers are used to make exact modulo possible for phase reset
  * sample-t and time-t are already configurable and could be set to the same type if needed
  * operations on float values may be much slower
  * bit operations are not possible
  * precision errors when summing times or limiting phases with modulo
  * possible overhead from needed rounding

# are filter banks needed in sp
* no, because that is post processing
* and its biggest benefit would be multiple band passes which is not better than separate filtered noise streams

# number types
unit: 0..1
sample: -1..1

# onset representations
* as factors multiplied by tempo
* as number of samples

## fixed
* 1 4 8 10
* easier to make changes while keeping total duration
* alignments to totals or intermediate points are easier
* pro
  * insert is easier
  * changes do not necessarily affect other elements
  * can be merged easily, even with different list lengths
* con
  * interval relation not obvious

## cumulative
intervals

* 3 1 3 2 3
* patterns on intervals are apparent
* permutations still valid
* calculating total length needs sum
* any insert or deletion needs ajdustment of surrounding intervals to keep total duration
* linear increase of delay more visible
  * 1 1.1 1.2 1.3
  * vs absolute
  * 1 2.1 3.3 4.6
* the first element has special meaning because it does not follow any other element
  * if it starts at the first beat this interval must be 0 even if all others are 1

# caveats
* not passing an amod where it is required
* setting frequency to 0
* event variable used multiple times without resetting already freed event memory
* adding the same pointer multiple times to event memory
* sharing resources with other events yet freeing them too early
* if sp_time_t is an unsigned type then core events do not support negative frequencies

# on scheduling events when adding them to groups
possible parameters
* onsets
  * multiplied by meter
* durations
* per onset instrument variables
* meter
* shifts
  * can be added to onsets
  * could be proportional to duration or spacing but that does not make much sense
* intervals
* spacing
  * pause after duration
  * meter and onset is more useful because adding to varying duration would lead to hard to predict onsets
  * adds to total duration

## how to add series to groups
* single event addition function preferred to series addition function
* implementation should support unmetered sounds (meter = 1)
* can be used in a loop to add a series, even with data from arrays
* handled by function, sp-event-schedule or similar
  * onset
  * duration
  * event.config
  * copying event
* handled by caller
  * loops for adding multiple items, otherwise passing event.config requires additional array of void pointers
  * meter, easily multiplied with the onset
  * shifts, easily added to onset
  * intervals, converted to onsets if needed

# candidates for new event fields
* amp, frq, pan
* name: for better error tracing and better render progress reporting

# naming: count vs length vs size vs duration
* example: channel-size has a different meaning from channel-count
* use size exclusively for bytes?
* or use size always with unit from context

# dynamic or fixed size array for event memory
* option
  * dynamic array
  * pro
    * helpers that wrap event memory addition do not need extra consideration when used
    * needs only one function for adding memory
    * users do not have to predict the final count
    * no optimization burden on the user
  * con
    * every addition requires two checks, is already allocated and is size sufficient
    * every addition requires a status check because the resize can fail
    * memory waste depending on grow factor, often about double from what is needed
* otion
  * fixed length array where user gives the length that will be required
  * pro
    * good for optimized core implementations
    * sph-sp already has a high memory requirement, every bit saved counts
      * although the space requirement comes mostly from the high resolution modulation arrays, not from event memory
  * con
    * users have to predict memory use. miscalculations lead to non-obvious crashes
    * loops and function calls that add memory are hard to predict
    * need is unclear, when is optimizing space usage this way relevant
* option
  * list
  * allocation for every addition
  * space waste through structure requirements
* option
  * vlist
  * con
    * space performance similar to dynamic array
    * addition performance in between dynamic/fixed
    * no implementation ready

# should functions that use random take a random state as an argument
using a global default random state makes calls simpler.
but then it can not be easily used deterministically by guaranteeing the value of the default random-state.
it is useful for users to just call (sp-time-random) and get a random number, using the default state.

* option - selected
  * always provide alternatve versions that take a random state
* option
  * modify the global random state for deterministic results
  * must be thread local to be usable in multiple threads
* option
  * always pass a random state argument

# how to customize core events
for example for custom wave formulas in generate

* option - selected
  * no customization, require a completely new custom event with custom config

# human sound localization effects
* interaural time difference
  * delayed time variant properties (amod, fmod, pmod, wmod)
* interaural intensity difference
  * differing amod per channel (channel-config.amod)
* spectral differences
  * different amods per partial and channel, differing noise filter settings per channel

# music theory
* musical scale
  * set of frequencies
  * [musical scales](https://web.mst.edu/~kosbar/ee3430/ff/fourier/notes_scales.html)
  * [a study of musical scales](https://ianring.com/musictheory/scales/)
* pitch ratios
  * unison 1:1
  * perfect octave 2:1
  * perfect fourth 4:3
  * perfect fifth 3:2
  * 1:1 (unison), 2:1 (octave), 5:3 (major sixth), 3:2 (perfect fifth), 4:3 (perfect fourth), 5:4 (major third), 6:5 (minor third)
