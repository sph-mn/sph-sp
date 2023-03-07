
/* small example that uses the core sound generators.
this example depends on gnuplot to be installed.
it should open a gnuplot window with a series of bursts of noise.
see exe/run-example or exe/run-example-sc for how to compile and run with gcc */
#include <sph-sp.h>
#define _sp_rate 48000

/** example demonstrating fundamental event processing.
   plots the samples of a 10hz sine wave */
status_t simple_event_plot() {
  status_declare;
  sp_sample_t* amod;
  sp_time_t duration = _sp_rate;
  sp_event_t event = sp_event_null;
  /* allocate array for loudness over time */
  srq((sp_samples_new(duration, (&amod))));
  sp_samples_set(amod, duration, 1);
  srq((sp_event_memory_add((&event), amod)));
  /* allocate sound_event_config, set options and finish event preparation */
  sp_sound_event_config_t* sound_event_config;
  srq((sp_sound_event_config_new((&sound_event_config))));
  srq((sp_event_memory_add((&event), sound_event_config)));
  sound_event_config->amod = amod;
  sound_event_config->frq = 10;
  sound_event_config->noise = 0;
  event.prepare = sp_sound_event_prepare;
  event.config = sound_event_config;
  event.start = 0;
  event.end = duration;
  srq((sp_render_plot(event)));
exit:
  status_return;
}

/* demonstration of the use of event groups, paths, and custom types for event configuration */
typedef struct {
  sp_sample_t amp;
} s1_c_t;
status_t s1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  /* defines a global event variable (s1-event, sound-1) using an additionally defined event-prepare function with the following content. */

  /* custom configuration passed via the event object */
  s1_c_t c = *((s1_c_t*)(_event->config));
  /* envelope from an interpolated path */
  sp_sample_t* amod;
  sp_path_curves_config_declare(amod_path, 3);
  (amod_path.x)[0] = 0;
  (amod_path.x)[1] = (_duration / 10);
  (amod_path.x)[2] = _duration;
  (amod_path.y)[0] = 0;
  (amod_path.y)[1] = c.amp;
  (amod_path.y)[2] = 0;
  srq((sp_path_curves_samples_new(amod_path, _duration, (&amod))));
  srq((sp_event_memory_add(_event, amod)));
  /* sound event configuration */
  sp_sound_event_config_t* se_c;
  srq((sp_sound_event_config_new((&se_c))));
  srq((sp_event_memory_add(_event, se_c)));
  se_c->amod = amod;
  se_c->frq = 300;
  ((se_c->channel_config)[0]).use = 1;
  ((se_c->channel_config)[0]).amp = (0.5 * c.amp);
  sp_sound_event(_event, se_c);
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(s1_event, s1_prepare, 0);
status_t t1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  _event->prepare = sp_group_prepare;
  /* defines a group named t1 (track 1) with a default duration of 3/1 * _sp_rate.
       srq (alias for status_require) checks return error codes and jumps to a label named 'exit' on error */
  sp_event_t event;
  s1_c_t* s1_c;
  sp_time_t tempo;
  sp_time_t times_length;
  sp_time_t times[8] = { 0, 2, 4, 6, 8, 12, 14, 16 };
  sp_event_reset(event);
  times_length = 8;
  tempo = (rt(1, 1) / 8);
  for (sp_size_t i = 0; (i < times_length); i += 1) {
    event = s1_event;
    sp_event_malloc_type((&event), s1_c_t, (&s1_c));
    s1_c->amp = ((i % 2) ? 0.25 : 1.0);
    event.config = s1_c;
    event.start = (_sp_rate + (tempo * times[i]));
    event.end = (event.start + rt(1, 6));
    srq((sp_group_add(_event, event)));
  };
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(t1_event, t1_prepare, (rts(3, 1)));
int main() {
  status_declare;
  /* use one cpu core and two output channels */
  sp_initialize(1, 2, _sp_rate);
  srq((sp_render_file(t1_event, ("/tmp/sp-example.wav"))));
exit:
  status_i_return;
}
