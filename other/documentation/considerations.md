# what parameters of core events should be possible to modulate
in traditional software pretty much everything can be time dependent.
allowing modulation is a performance consideration because it needs one extra check per generated sample (per channel).
but some things are deemed to be a too subtle effect and modulation is currently not supported for thees (ex: transition length of filters), or are intended to be realised with second events (ex: fmod per channel) or custom events if needed.

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

# event retirement
* groups should retire events as soon as possible
* option
  * check after seq
  * find old events
  * con
    * loop is called for every render block
    * duplicates part of the seq loop (search for start)

# delayed event initialisation
* only generate event dependencies when event is needed
* to reduce memory usage
* option - selected
  * to be implemented if needed as array of event initialisers and arguments in group
  * initialiser function and arguments arrays
  * groups can filter and call before seq
  * con
    * only groups will support initialisation
  * pro
    * less space needed for other event types
    * not every event will be checked
* option
  * initialiser function for events
  * seq would have to call them
  * pro
    * supported for all events
  * con
    * seq overhead

# group parallelisation
* option: replaced group function - selected
  * least overhead
* option: group attribute
* option: render argument

# start/end for events or start/duration
* duration is always dependent on start and the event can be moved in time just by updating start
* calculations are simpler with an end value. it would have to be calculated for each event check or cached in a new struct field with more code complexity. `if(end <= t) {continue;} else if(end <= t) {break;}`

# should seq return the event offset
* selected: no, extra declaration/count/return effort is even easier handled if needed in caller

# notes
* event start/end times are event relative
* events must never be called for start/end outside their range
* out is positioned at start with index 0 so that seq and event.f behave the same when nested
