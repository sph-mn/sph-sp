
/* small example that shows how to use the core sound generators with sc macros.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example for how to compile and run with gcc */

#include <sph-sp.h>
#define _sp_rate 48000
status_t d7_hh_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  srq((sp_event_memory_init(_event, 10)));
  // sp-path*
  sp_sample_t* amod;
  sp_path_t amod_path;
  sp_path_segment_t amod_segments[2];
  amod_segments[0] = sp_path_line((rt(1, 20)), (_event->volume));
  amod_segments[1] = sp_path_line(_duration, 0);
  spline_path_set((&amod_path), amod_segments, 2);
  status_require((sp_path_samples_new(amod_path, _duration, (&amod))));
  sp_event_memory_add1(_event, amod);
  // sp-noise-config*
  sp_noise_event_config_t* n1c;
  status_require((sp_noise_event_config_new((&n1c))));
  n1c->amod = amod;
  sp_event_memory_add1(_event, n1c);
  // sp-noise*
  _event->data = n1c;
  _event->prepare = sp_noise_event_prepare;
  if (_event->prepare) {
    srq(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(d7_hh, d7_hh_prepare, 0);
status_t d7_hh_r1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  _event->prepare = sp_group_prepare;
  srq((sp_event_memory_init(_event, 10)));
  sp_time_t tempo = rt(1, 3);
  sp_time_t times_length = 6;
  sp_time_t* times;
  status_require((sp_times_new(times_length, (&times))));
  sp_event_memory_add1(_event, times);
  times[0] = 1;
  times[1] = 1;
  times[2] = 4;
  times[3] = 4;
  times[4] = 4;
  times[5] = 1;
  sp_times_multiply_1(times, times_length, tempo, times);
  sp_times_cusum(times, times_length, times);
  for (sp_time_t i = 0; (i < times_length); i += 1) {
    status_require((sp_group_add_set(_event, (times[i]), (rt(1, 6)), (_event->volume), ((void*)(0)), d7_hh)));
  };
  if (_event->prepare) {
    srq(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(d7_hh_r1, d7_hh_r1_prepare, 0);
/* use one cpu core and two output channels and add one riff to the song at specified offset, duration and volume */
status_t main() {
  status_declare;
  sp_declare_group(_song);
  sp_event_t* _event = &_song;
  sp_initialize(1, 2, _sp_rate);
  status_require((sp_group_add_set(_event, 0, (rt(4, 1)), (0.5), ((void*)(0)), d7_hh_r1)));
  status_require((sp_render_quick((*_event), 1)));
exit:
  status_return;
}
