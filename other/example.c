
/* small example that shows how to use the core sound generating events.
this example depends on gnuplot to be installed.
when running it, it should display a gnuplot window with a noisy sine wave.
see exe/run-example for how to compile and run with gcc */

/* the sc version of this file defines macros which are only available in sc.
the macros are used as optional helpers to simplify common tasks where c syntax alone offers no good alternative */
#include <stdio.h>
#include <sph-sp.h>
status_t example_event(sp_sample_t duration, sp_event_t* _out) {
  status_declare;
  sp_declare_event(_result);
  sp_declare_event(_event);
  sp_sample_t* amod;
  sp_declare_event_4(g, s1, n1, n2);
  sp_declare_sine_config(s1_config);
  sp_declare_noise_config(n1_config);
  sp_declare_cheap_noise_config(n2_config);
  free_on_error_init(1);
  status_require((sp_group_new(0, 10, (&g))));
  free_on_error((&g), (g.free));
  status_require((sp_event_memory_init(g, 1)));
  sp_path_t _t1;
  // sp-path
  sp_path_segment_t _t2[2];
  _t2[0] = sp_path_line((duration / 2), (1.0));
  _t2[1] = sp_path_line(duration, 0);
  spline_path_set((&_t1), _t2, 2);
  status_require((sp_path_samples_new(_t1, duration, (&amod))));
  sp_event_memory_add(g, (&amod));
  // sp-sine
  s1_config.amod = amod;
  s1_config.frq = 3;
  ((s1_config.channel_config)[1]).use = 1;
  ((s1_config.channel_config)[1]).amp = 0.1;
  status_require((sp_wave_event(0, duration, s1_config, (&s1))));
  sp_group_add(g, s1);
  // sp-noise
  n1_config.amod = amod;
  n1_config.cutl = 0.2;
  ((n1_config.channel_config)[0]).use = 1;
  ((n1_config.channel_config)[0]).delay = 30000;
  ((n1_config.channel_config)[0]).amp = 0.2;
  status_require((sp_noise_event(0, duration, n1_config, (&n1))));
  sp_group_add(g, n1);
  // sp-cheap-noise
  n2_config.amod = amod;
  n2_config.cut = 0.5;
  ((n2_config.channel_config)[1]).use = 1;
  ((n2_config.channel_config)[1]).amp = 0.001;
  status_require((sp_cheap_noise_event(0, duration, n2_config, (&n2))));
  sp_group_add(g, n2);
  _result = g;
  *_out = _result;
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
int main() {
  status_declare;
  sp_declare_event(event);
  sp_declare_events(events, 1);
  sp_initialize(1, 2, 48000);
  status_require((example_event((sp_rate * 2), (&event))));
  sp_events_add(events, event);
  status_require((sp_render_quick(events, 1)));
exit:
  return ((status.id));
}
