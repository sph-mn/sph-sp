
/* small example that shows how to use the core sound generating events.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a noisy sine wave.
see exe/run-example for how to compile and run with gcc */

/* the sc version of this file defines macros which are only available in sc.
the macros are used as optional helpers to simplify common tasks where c syntax alone offers no good alternative */
#include <stdio.h>
#include <sph-sp.h>

/** heap allocates a noise_event_config struct and sets some defaults */
status_t sp_noise_event_config_new(sp_noise_event_config_t** out) {
  status_declare;
  sp_noise_event_config_t* result;
  sp_channel_config_t channel_config = { 0 };
  status_require((sp_malloc_type(1, sp_noise_event_config_t, (&result))));
  (*result).channels = sp_channels;
  (*result).trnl = 0.1;
  (*result).trnh = 0.1;
  (*result).cutl = 0.0;
  (*result).cuth = 0.5;
  (*result).amp = 1;
  (*result).amod = 0;
  (*result).cutl_mod = 0;
  (*result).cuth_mod = 0;
  (*result).resolution = 96;
  (*result).is_reject = 0;
  for (sp_time_t i = 0; (i < sp_channel_limit); i += 1) {
    (result->channel_config)[i] = channel_config;
  };
  *out = result;
exit:
  status_return;
}
status_t example_event(sp_event_t* _event) {
  status_declare;
  printf("-- example-event-prepare\n");
  sp_sample_t* amod;
  sp_time_t duration;
  sp_noise_event_config_t* n1_config;
  sp_declare_event_3(s1, n1, n2);
  sp_declare_sine_config(s1_config);
  sp_declare_sine_config(s2_config);
  sp_declare_cheap_noise_config(n2_config);
  free_on_error_init(2);
  free_on_error((_event->free), _event);
  status_require((sp_event_memory_init(_event, 1)));
  status_require((sp_noise_event_config_new((&n1_config))));
  free_on_error1(n1_config);
  duration = (_event->end - _event->start);
  _event->free = sp_group_free;
  sp_path_t _t1;
  // sp-path
  sp_path_segment_t _t2[2];
  _t2[0] = sp_path_line((duration / 2), (1.0));
  _t2[1] = sp_path_line(duration, 0);
  spline_path_set((&_t1), _t2, 2);
  status_require((sp_path_samples_new(_t1, duration, (&amod))));
  sp_event_memory_add1(_event, (&amod));
  /* (sp-cheap-noise* n2 0 duration n2-config (amod amod cut 0.5) (1 use 1 amp 0.001))
(status-require (sp-group-add _event n2))
(sp-sine* s1 0 duration s1-config (amod amod frq 8) (1 use 1 amp 0.1))
(status-require (sp-group-add _event s1)) */

  // sp-noise*
  (*n1_config).amod = amod;
  (*n1_config).cutl = 0.2;
  (*n1_config).cuth = 0.5;
  (((*n1_config).channel_config)[0]).use = 1;
  (((*n1_config).channel_config)[0]).delay = 30000;
  (((*n1_config).channel_config)[0]).amp = 0.5;
  n1.start = 0;
  n1.end = duration;
  n1.data = &(*n1_config);
  n1.prepare = sp_noise_event_prepare;
  status_require((sp_group_add(_event, n1)));
  status_require((sp_group_prepare(_event)));
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
int main() {
  status_declare;
  sp_declare_event(event);
  sp_declare_event_list(events);
  sp_initialize(1, 2, 48000);
  event.start = 0;
  event.end = (sp_rate * 2);
  event.prepare = example_event;
  status_require((sp_event_list_add((&events), event)));
  status_require((sp_render_quick(events, 1)));
exit:
  return ((status.id));
}
