
/* small example that shows how to use the core sound generators.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example or exe/run-example-sc for how to compile and run with gcc */

#include <sph-sp.h>
#define _sp_rate 48000
typedef struct {
  sp_sample_t amp;
} s1_config_t;
status_t s1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  s1_config_t config = *((s1_config_t*)(_event->config));
  status_require((sp_event_memory_init(_event, 2)));
  // sp-path*
  sp_sample_t* amod;
  sp_path_t amod_path;
  sp_path_segment_t amod_segments[2];
  amod_segments[0] = sp_path_line((_duration / 20), (config.amp));
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
  _event->config = n1c;
  _event->prepare = sp_noise_event_prepare;
  printf("not prepared\n");
  (_event->prepare)(_event);
  printf("prepared\n");
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(s1_event, s1_prepare, 0);
/* defines a group named r1 with a default duration of (1/1 * sample_rate). srq (status-require) checks return codes */
status_t r1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  _event->prepare = sp_group_prepare;
  sp_declare_event(event);
  s1_config_t* s1_config;
  sp_time_t tempo;
  sp_time_t times_length;
  sp_time_t times[4] = { 0, 2, 4, 6 };
  times_length = 4;
  tempo = (rt(1, 1) / 8);
  status_require((sp_event_memory_init(_event, times_length)));
  for (size_t i = 0; (i < times_length); i += 1) {
    event = s1_event;
    srq((sp_event_memory_init((&event), 1)));
    srq((sp_malloc_type(1, s1_config_t, (&s1_config))));
    sp_event_memory_add((&event), s1_config);
    s1_config->amp = ((i % 2) ? 0.5 : 1.0);
    event.config = s1_config;
    event.start = (tempo * times[i]);
    event.end = (event.start + rt(1, 6));
    status_require((sp_group_add(_event, event)));
  };
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(r1_event, r1_prepare, (rts(1, 1)));
/* use one cpu core and two output channels and add one riff to the song at specified offset, duration and volume */
status_t main() {
  status_declare;
  sp_declare_group(_song);
  sp_event_t* _event = &_song;
  sp_initialize(1, 1, _sp_rate);
  status_require((sp_group_add(_event, r1_event)));
  status_require((sp_render_quick((*_event), 1)));
exit:
  status_return;
}
