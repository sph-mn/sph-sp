# start/end for events or start/duration
* duration is always dependent on start and the event can be moved in time just by updating start
* calculations are simpler with an end value. it would have to be calculated for each event check or cached in a new struct field with more code complexity. `if(end <= t) {continue;} else if(end <= t) {break;}`
