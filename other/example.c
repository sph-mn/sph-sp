
/* small example that shows how to use the core sound generators.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example or exe/run-example-sc for how to compile and run with gcc */

#include <sph-sp.h>
#define _sp_rate 48000
status_t s1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  sp_event_config_load(config, sp_default_event_config_t, _event);
  status_require((sp_event_memory_init(_event, 10)));
  // sp-path*
  sp_sample_t* amod;
  sp_path_t amod_path;
  sp_path_segment_t amod_segments[2];
  amod_segments[0] = sp_path_line((rt(1, 20)), (config.amp));
  amod_segments[1] = sp_path_line(_duration, 0);
  spline_path_set((&amod_path), amod_segments, 2);
  status_require((sp_path_samples_new(amod_path, _duration, (&amod))));
  sp_event_memory_add(_event, amod);
  sp_noise_event_config_t* n1c;
  status_require((sp_noise_event_config_new((&n1c))));
  n1c->amod = amod;
  ((n1c->channel_config)[0]).use = 1;
  ((n1c->channel_config)[0]).amp = (0.5 * config.amp);
  sp_event_memory_add(_event, n1c);
  _event->data = n1c;
  _event->prepare = sp_noise_event_prepare;
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(s1, s1_prepare, 0);
status_t r1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  _event->prepare = sp_group_prepare;
  status_require((sp_event_memory_init(_event, 10)));
  sp_time_t tempo = (rt(1, 1) / 8);
  sp_sample_t volume = 1;
  // sp-array-values*
  sp_time_t times_length = 4;
  sp_time_t* times;
  status_require((sp_times_new(times_length, (&times))));
  sp_event_memory_add(_event, times);
  times[0] = 0;
  times[1] = 2;
  times[2] = 4;
  times[3] = 6;
  for (sp_time_t i = 0; (i < times_length); i += 1) {
    /* the default config can be used to pass values for amplitude, frequency and panning */
    volume = ((i % 2) ? 0.5 : 1.0);
    sp_default_event_config_new(volume, 0, 0, ((sp_default_event_config_t**)(&(s1.config))));
    sp_event_memory_add(_event, (s1.config));
    status_require((sp_group_add_set(_event, (tempo * times[i]), (rt(1, 6)), s1)));
  };
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(r1, r1_prepare, (rts(1, 1)));
/* use one cpu core and two output channels and add one riff to the song at specified offset, duration and volume */
status_t main() {
  status_declare;
  sp_declare_group(_song);
  sp_event_t* _event = &_song;
  sp_initialize(1, 2, _sp_rate);
  status_require((sp_group_add_set(_event, 0, 0, r1)));
  status_require((sp_render_quick((*_event), 1)));
exit:
  status_return;
}
