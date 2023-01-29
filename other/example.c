
/* small example that shows how to use the core sound generators.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example or exe/run-example-sc for how to compile and run with gcc */

#include <sph-sp.h>
#define _sp_rate 48000

/* custom type for event configuration */
typedef struct {
  sp_sample_t amp;
} s1_c_t;
status_t s1_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  s1_c_t c;
  sp_sound_event_config_t* se_c;
  sp_sample_t* amod;
  c = *((s1_c_t*)(_event->config));
  sp_event_memory_ensure(_event, 2);
  /* envelope */
  sp_path_curves_config_declare(amod_path, 3);
  (amod_path.x)[0] = 0;
  (amod_path.x)[1] = (_duration / 20);
  (amod_path.x)[2] = _duration;
  (amod_path.y)[0] = 0;
  (amod_path.y)[1] = c.amp;
  (amod_path.y)[2] = 0;
  srq((sp_path_curves_samples_new(amod_path, _duration, (&amod))));
  sp_event_memory_add(_event, amod);
  /* sound event */
  srq((sp_sound_event_config_new((&se_c))));
  sp_event_memory_add(_event, se_c);
  se_c->amod = amod;
  se_c->noise = 1;
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
  /* defines a group named t1 with a default duration of (1/1 * sample_rate).
       srq (alias for status_require) checks return codes and jumps to a label named 'exit' on error */
  s1_c_t* s1_c;
  sp_time_t tempo;
  sp_time_t times_length;
  sp_time_t times[4] = { 0, 2, 4, 6 };
  sp_event_t event;
  times_length = 4;
  tempo = (rt(1, 1) / 8);
  sp_event_memory_ensure(_event, times_length);
  for (size_t i = 0; (i < times_length); i += 1) {
    event = s1_event;
    sp_event_malloc_type((&event), s1_c_t, (&s1_c));
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
