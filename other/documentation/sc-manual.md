# sc manual
sc extends the [c api](c-manual.md) with new syntax that can save a lot of boilerplate code.

using the sc api requires the use of [sc](https://github.com/sph-mn/sph-sc) to compile your sc code.
compiling sc to c can be as simple as calling `sc input.sc output.c`. to compile a directory structure with .sc files to a directory structure with .c files, the following can be be used: `find -type f -name "*.sc" -exec sc --parents '{}' "target_dir" \;`.

example
~~~
(sc-include-once "/usr/share/sph-sp/sc-macros")
(sp-init* 48000)

(sp-define-event* d7-hh
  (sp-event-memory* 10)
  (sp-path-samples* amod ((rt 1 20) _event:volume) (_duration 0))
  (sp-noise-config* n1c)
  (sp-noise-config-new* n1c _event (amod amod))
  (sp-noise* _event n1c))

(sp-define-group* d7-hh-r1
  (sp-event-memory* 10)
  (sp-time* tempo (rt 1 3))
  (sp-intervals* times tempo 0 1 1 4 4 4 1)
  (for-each-index i times-length
    (sp-group-add* _event (array-get times i) (rt 1 6) _event:volume 0 d7-hh)))

(sp-define-song* 1 2 (sp-group-add* _event 0 (rt 2 1) 0.5 0 d7-hh-r1) (sp-render-plot*))
~~~

## general
* many macros expect to be used inside sp-define-event* or sp-define-group*
* sc-macro names end with * to avoid name conflicts with plain c functions

## sp-time*, sp-sample*
define sp_time_t or sp_sample_t variables

example
~~~
(sp-time* a)
(sp-time* a 1)
(sp-time* a 1 b 2 c 3)
(sp-sample* a)
(sp-sample* a 1.1)
(sp-sample* a 1.1 b 2.2 c 3.3)
~~~

## sp-times*, sp-samples*
* define sp_time_t or sp_sample_t arrays
* heap allocated and automatically freed with the event

forms
~~~
(sp-samples* name value ...)
~~~

examples
~~~
(sp-samples* a)
(sp-samples* a 2 1 1)
~~~

## sp-define-event*
* define a single event as a global variable
* can be used as (sp-define-event name body ...) or (sp-define-event (name default-duration) body ...)
* implicitly defined variables
  * _event: pointer to the result event, usually with start, end and volume set
  * _duration: difference between _event:end and _event:start
* finally calls _event:prepare if not null
* arguments and types are implicit
* status-declare is implicit
* exit label with status-return is optional
* free-on-error-free/free-on-exit-free is added automatically if free-on-error feature use found

## sp-define-group*, sp-define-group-parallel*
like sp-define-event* but automatically sets the group prepare function.

sp-define-group-parallel* distributes the events of the group over multiple cpu cores, which can drastically speed up processing if there are multiple events (or event trees) that need a lot of processing.
it might be possible to use it nested but there should only be a single parallel group to keep overhead low.

## sp-wave-config* , sp-noise-config*, sp-cheap-noise-config*
* declare configuration variables for (sine-) waves or filtered noise

forms
... means zero or multiple.

~~~
(sp-wave-config* name ...)
~~~

## sp-wave-config-new*, sp-noise-config-new*, sp-cheap-noise-config-new*
* sp-wave-config* configures a sine wave by default
* config-key/value sets fields of sp_wave_event_config_t or the other corresponding structs
* channel-config-key/value sets fields of sp_channel_config_t

~~~
(sp-wave-config-new* name (config-key/value ...) channel-config ...)
(sp-noise-config-new* name (config-key/value ...) channel-config ...)
(sp-cheap-noise-config-new* name (config-key/value ...) channel-config ...)
~~~

channel-config
~~~
(channel-index channel-config-key/value ...)
~~~

example
~~~
(sp-wave-config-new* s1c (amod amod frq 300) (1 mute #t))
~~~

## sp-wave*, sp-noise*, sp-cheap-noise*
setup an event to be a wave, noise or cheap-noise event.

example
~~~
(sp-wave* event s1c)
~~~

* second channel disabled
* amod is an array of samples for amplitude over time, possibly created with sp-path-samples*

## sp-group-add*, sp-group-append*
forms
~~~
(sp-group-add* start duration volume data event)
(sp-group-append* volume data event)
~~~

## sp-init*
includes sph-sp.h and defines the _sp_rate preprocessor variable

forms
~~~
(sp-init* rate)
~~~

## sp-define-song*
defines a main function, calls sp_initialize and allows for events to be added to a main song event that can be rendered with sp-render-file* or sp-render-plot*

forms
~~~
(sp-define-song* parallelization channels body ...)
~~~

## sp-intervals*
* like sp-samples* but values are multiplied by tempo and cumulative, so that they become event start times relative to zero.
* also defines a new variable {name}_length

forms
~~~
(sp-intervals* name tempo values ...)
~~~

(sp-intervals* name 20 1 2 3 3 2)

becomes

(20 60 120 180 220)
