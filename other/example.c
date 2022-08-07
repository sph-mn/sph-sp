
/* small example that shows how to use the core sound generators.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example or exe/run-example-sc for how to compile and run with gcc */

#include <sph-sp.h>
#define _sp_rate 48000
typedef struct {
  sp_sample_t amp;
} s1_c_t;
status_t s1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  sp_noise_event_config_t* n1_c;
  s1_c_t c = *((s1_c_t*)(_event->config));
  srq((sp_event_memory_init(_event, 2)));
  // sp-path*
  sp_sample_t* amod;
  sp_path_t amod_path;
  sp_path_segment_t amod_segments[2];
  amod_segments[0] = sp_path_line((_duration / 20), (c.amp));
  amod_segments[1] = sp_path_line(_duration, 0);
  spline_path_set((&amod_path), amod_segments, 2);
  status_require((sp_path_samples_new(amod_path, _duration, (&amod))));
  sp_event_memory_add(_event, amod);
  srq((sp_event_memory_init(_event, 1)));
  srq((sp_noise_event_config_new((&n1_c))));
  sp_event_memory_add(_event, n1_c);
  n1_c->amod = amod;
  ((n1_c->channel_config)[0]).use = 1;
  ((n1_c->channel_config)[0]).amp = (0.5 * c.amp);
  sp_noise_event(_event, n1_c);
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(s1_event, s1_prepare, 0);
/* defines a group named t1 with a default duration of (2/1 * sample_rate).
   srq (alias for status_require) checks return codes and jumps to an exit label on error */
status_t t1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  _event->prepare = sp_group_prepare;
  s1_c_t* s1_c;
  sp_time_t tempo;
  sp_time_t times_length;
  sp_time_t times[4] = { 0, 2, 4, 6 };
  sp_event_t event;
  times_length = 4;
  tempo = (rt(1, 1) / 8);
  srq((sp_event_memory_init(_event, times_length)));
  for (size_t i = 0; (i < times_length); i += 1) {
    event = s1_event;
    srq((sp_event_memory_init((&event), 1)));
    srq((sp_event_memory_init((&event), 1)));
    srq((sph_helper_malloc((sizeof(s1_c_t)), (&s1_c))));
    sp_event_memory_add((&event), s1_c);
    s1_c->amp = ((i % 2) ? 0.5 : 1.0);
    event.config = s1_c;
    event.start = (tempo * times[i]);
    event.end = (event.start + rt(1, 6));
    srq((sp_group_add(_event, event)));
  };
  if (_event->prepare) {
    status_require(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(t1_event, t1_prepare, (rts(1, 1)));
/** use one cpu core and two output channels and add one track to the song at specified offset, duration and volume */
int main() {
  status_declare;
  sp_declare_group(group);
  sp_declare_event(event);
  sp_initialize(1, 2, _sp_rate);
  event = t1_event;
  srq((sp_group_add((&group), event)));
  srq((sp_render(group, 1)));
exit:
  status_i_return;
}
