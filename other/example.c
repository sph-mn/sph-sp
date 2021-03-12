
/* small example that shows how to use the core sound generators and sc macros.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example for how to compile and run with gcc */

/* the sc version of this file defines macros which are only available in sc.
the macros are used as optional helpers to simplify common tasks where c syntax alone offers no good alternative */
#include <sph-sp.h>
#define _rate 48000
status_t noise_trigger(sp_event_t* _event) {
  status_declare;
  sp_sample_t* amod;
  sp_time_t duration;
  sp_noise_event_config_t* config;
  duration = (_event->end - _event->start);
  srq((sp_event_memory_init(_event, 2)));
  srq((sp_noise_event_config_new((&config))));
  sp_event_memory_add1(_event, config);
  sp_path_t _t1;
  // sp-path
  sp_path_segment_t _t2[1];
  _t2[0] = sp_path_line(duration, (_event->volume));
  spline_path_set((&_t1), _t2, 1);
  status_require((sp_path_samples_new(_t1, duration, (&amod))));
  sp_event_memory_add1(_event, amod);
  // sp-noise*
  config->amod = amod;
  config->amp = 1;
  config->cuth = 0.5;
  _event->data = config;
  _event->prepare = sp_noise_event_prepare;
  srq(((_event->prepare)(_event)));
exit:
  status_return;
}
sp_define_trigger_event(noise, noise_trigger, 0);
status_t riff_trigger(sp_event_t* _event) {
  status_declare;
  sp_time_t times[3] = { 0, (1 * sp_rate), (1.5 * sp_rate) };
  sp_time_t durations[3] = { (0.5 * sp_rate), (0.15 * sp_rate), (0.35 * sp_rate) };
  sp_sample_t volumes[3] = { 1.0, 0.5, 0.75 };
  for (sp_time_t i = 0; (i < 3); i += 1) {
    srq((sp_group_add_set(_event, (times[i]), (durations[i]), (volumes[i]), 0, noise)));
  };
  srq((sp_group_prepare(_event)));
exit:
  status_return;
}
sp_define_trigger_event(riff, riff_trigger, (2 * _rate));
int main() {
  status_declare;
  sp_declare_group(song);
  srq((sp_initialize(1, 2, _rate)));
  srq((sp_group_append((&song), riff)));
  srq((sp_group_append((&song), riff)));
  srq((sp_group_append((&song), riff)));
  srq((sp_render_quick(song, 1)));
exit:
  return ((status.id));
}
