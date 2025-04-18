#include <sph-sp.h>
#define _sp_rate 48000

// Basic example: a single sine wave with constant amplitude.
// Demonstrates minimal event construction and plotting.
status_t simple_event_plot() {
  status_declare;
  sp_sample_t* amod;
  sp_time_t duration = _sp_rate;
  sp_event_t event = sp_null_event;

  // Allocate constant amplitude array
  srq((sp_samples_new(duration, (&amod))));
  sp_samples_set(amod, duration, 1);
  srq((sp_event_memory_add((&event), amod)));

  // Allocate and configure sine wave event
  sp_wave_event_config_t* wave_event_config;
  srq((sp_wave_event_config_new((&wave_event_config))));
  srq((sp_event_memory_add((&event), wave_event_config)));
  wave_event_config->channel_config->amod = amod;
  wave_event_config->channel_config->frq = 10;

  event.start = 0;
  event.end = duration;

  // Render to gnuplot
  sp_wave_event((&event), wave_event_config);
  srq((sp_render_plot(event)));
exit:
  status_return;
}

// Custom configuration for a two-channel wave event
typedef struct {
  sp_sample_t amp;
} s1_c_t;

// Defines an event named s1-event using a user-defined prepare function.
// This event emits sound on two channels with envelope shaping.
status_t s1_prepare(sp_event_t* _event) {
  status_declare;
  _event->prepare = 0;
  sp_time_t _duration = (_event->end - _event->start);

  // Access user configuration (amp field)
  s1_c_t c = *((s1_c_t*)(_event->config));

  // Create interpolated amplitude envelope from silence to c.amp
  sp_sample_t* amod;
  sp_event_envelope_zero3_srq(_event, (&amod), _duration, (0.1), (c.amp));

  // Allocate and configure wave event with two channels
  sp_wave_event_config_t* wec;
  srq((sp_wave_event_config_new((&wec))));
  srq((sp_event_memory_add(_event, wec)));
  (wec->channel_config)->amod = amod;
  (wec->channel_config)->frq = 300;

  // Second channel with lower amplitude
  (1 + wec->channel_config)->use = 1;
  (1 + wec->channel_config)->amp = (0.5 * c.amp);

  sp_wave_event(_event, wec);
  sp_event_prepare_optional_srq((*_event));
exit:
  status_return;
}

sp_define_event(s1_event, s1_prepare, 0);

// Defines an event named t1-event, which is a group of multiple s1-events.
// Groups are used to sequence and compose multiple sound events over time.
status_t t1_prepare(sp_event_t* _event) {
  status_declare;
  _event->prepare = 0;

  sp_event_t event;
  s1_c_t* s1_c;
  sp_time_t tempo;
  sp_time_t times_length;
  sp_time_t times[8] = { 0, 2, 4, 6, 8, 12, 14, 16 };

  sp_event_reset(event);
  times_length = 8;

  // One beat is 1/8 of a second; tempo defines spacing between events
  tempo = (sp_duration(1, 1) / 8);

  // Create a group event (container for sub-events)
  sp_group_event(_event);

  for (sp_size_t i = 0; (i < times_length); i += 1) {
    event = s1_event;

    // Allocate config for sub-event
    sp_event_malloc_type_srq((&event), s1_c_t, (&s1_c));
    s1_c->amp = ((i % 2) ? 0.25 : 0.9);
    event.config = s1_c;

    // Start times are offset to begin after one second
    event.start = (_sp_rate + (tempo * times[i]));
    event.end = (event.start + sp_duration(1, 6));

    srq((sp_group_add(_event, event)));
  }

  // Prepares all sub-events
  sp_event_prepare_optional_srq((*_event));
exit:
  status_return;
}

sp_define_event(t1_event, t1_prepare, (sp_duration(3, 1)));

int main() {
  status_declare;

  // Use one CPU core and two output channels
  sp_initialize(1, 2, _sp_rate);

  // Render and plot the composite group event
  srq((sp_render_file(t1_event, ("/tmp/sp-example.wav"))));
  srq((sp_render_plot(t1_event)));
exit:
  sp_deinitialize();
  status_i_return;
}
