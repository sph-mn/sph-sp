
/* small example that shows how to use the core sound generators and sc macros.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a series of bursts of noise.
see exe/run-example for how to compile and run with gcc */
#include <sph-sp.h>
#define _rate 48000
status_t noise_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  srq((sp_event_memory_init(_event, 2)));
  // sp-path*
  sp_sample_t* amod;
  sp_path_t amod_path;
  sp_path_segment_t amod_segments[1];
  amod_segments[0] = sp_path_line(_duration, (_event->volume));
  spline_path_set((&amod_path), amod_segments, 1);
  status_require((sp_path_samples_new(amod_path, _duration, (&amod))));
  sp_event_memory_add1(_event, amod);
  sp_event_memory_add1(_event, amod);
  // sp-noise-config*
  sp_noise_event_config_t* config;
  status_require((sp_noise_event_config_new((&config))));
  sp_event_memory_add1(_event, config);
  // sp-noise*
  config->amod = amod;
  config->amp = 1;
  config->cuth = 0.5;
  _event->data = config;
  _event->prepare = sp_noise_event_prepare;
  if (_event->prepare) {
    srq(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(noise, noise_prepare, 0);
status_t riff_prepare(sp_event_t* _event) {
  status_declare;
  sp_time_t _duration = (_event->end - _event->start);
  _event->prepare = sp_group_prepare;
  sp_time_t times[3] = { 0, (1 * sp_rate), (1.5 * sp_rate) };
  sp_time_t durations[3] = { (0.5 * sp_rate), (0.15 * sp_rate), (0.35 * sp_rate) };
  sp_sample_t volumes[3] = { 1.0, 0.5, 0.75 };
  for (sp_time_t i = 0; (i < 3); i += 1) {
    srq((sp_group_add_set(_event, (times[i]), (durations[i]), (volumes[i]), 0, noise)));
  };
  if (_event->prepare) {
    srq(((_event->prepare)(_event)));
  };
exit:
  status_return;
}
sp_define_event(riff, riff_prepare, (2 * _rate));
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
