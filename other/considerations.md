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
* option
  * initialiser function for events
  * seq would have to call them
  * pro
    * supported for all events
  * con
    * seq overhead
* option
  * initialiser function and arguments arrays
  * groups can filter and call before seq
  * con
    * only groups will support initialisation
  * pro
    * less space needed for other event types
    * not every event will be checked

# group parallelisation
* option: replaced group function - selected
  * least overhead
* option: group attribute
* option: render argument

# start/end for events or start/duration
* duration is always dependent on start and the event can be moved in time just by updating start
* calculations are simpler with an end value. it would have to be calculated for each event check or cached in a new struct field with more code complexity. `if(end <= t) {continue;} else if(end <= t) {break;}`
