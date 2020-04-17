#include <sph-sp.h>
#define song_duration 9600

status_t wvl_path_new(sp_count_t duration, sp_count_t** out)
{
  status_declare;
  sp_sample_t* wvl_float;
  status_require((sp_samples_new(duration, (&wvl_float))));
  status_require((sp_counts_new(duration, out)));
  spline_path_new_get_4(wvl_float,
    duration,
    (spline_path_move(0, 600)),
    (spline_path_line(24000, 1800)),
    (spline_path_line(duration, 2400)),
    (spline_path_constant()));
  sp_counts_from_samples(wvl_float, duration, (*out));
exit:
  return (status);
}

status_t wvl2_path_new(sp_count_t duration, sp_count_t** out)
{
  status_declare;
  sp_sample_t* wvl_float;
  status_require((sp_samples_new(duration, (&wvl_float))));
  status_require((sp_counts_new(duration, out)));
  spline_path_new_get_4(wvl_float,
    duration,
    (spline_path_move(0, 20)),
    (spline_path_line(24000, 10)),
    (spline_path_line(duration, 40)),
    (spline_path_constant()));
  sp_counts_from_samples(wvl_float, duration, (*out));
exit:
  return (status);
}

status_t amp_path_new(sp_count_t duration, sp_sample_t** out)
{
  status_declare;
  status_require((sp_samples_new(duration, out)));
  status_id_require((spline_path_new_get_4((*out),
    duration,
    (spline_path_move(0, (0.3))),
    (spline_path_line(24000, (0.1))),
    (spline_path_line(duration, 0)),
    (spline_path_constant()))));
exit:
  return (status);
}

status_t constant_path_new(sp_sample_t value, sp_count_t duration, sp_sample_t** out)
{
  status_declare;
  status_require((sp_samples_new(duration, out)));
  status_id_require((spline_path_new_get_2(
    (*out), duration, (spline_path_move(0, value)), (spline_path_constant()))));
exit:
  return (status);
}

int main()
{
  status_declare;
  sp_block_t out;
  sp_count_t* wvl;
  sp_count_t* wvl2;
  sp_sample_t* cut_l;
  sp_sample_t* cut_h;
  sp_sample_t* trn_l;
  sp_sample_t* trn_h;
  sp_sample_t* amp1;
  sp_sample_t* amp2;
  sp_sample_t* amp[sp_channel_limit];
  sp_synth_partial_t prt;
  sp_count_t events_size;
  sp_event_t e;
  sp_event_t events[10];
  sp_synth_partial_t config[10];
  sp_count_t* state;
  events_size = 0;
  sp_initialise(0);
  status_require((wvl_path_new(song_duration, (&wvl))));
  status_require((wvl2_path_new(song_duration, (&wvl2))));
  status_require((amp_path_new(song_duration, (&amp1))));
  status_require((constant_path_new((0.01), song_duration, (&cut_l))));
  status_require((constant_path_new((0.1), song_duration, (&cut_h))));
  status_require((constant_path_new((0.07), song_duration, (&trn_l))));
  status_require((constant_path_new((0.5), song_duration, (&amp2))));
  trn_h = trn_l;
  config[0] = sp_synth_partial_1(0, song_duration, 0, amp1, wvl, 0);
  config[1] = sp_synth_partial_1(0, song_duration, 1, amp2, wvl2, 0);
  status_require((sp_synth_event(0, song_duration, 1, 2, config, events)));
  events_size = 1;
  amp[0] = cut_h;
  status_require((sp_noise_event(0,
    song_duration,
    amp,
    cut_l,
    cut_h,
    trn_l,
    trn_h,
    0,
    0,
    sp_default_random_state,
    (events_size + events))));
  events_size = (1 + events_size);
  sp_seq_events_prepare(events, events_size);
  status_require((sp_block_new(1, song_duration, (&out))));
  sp_seq(0, song_duration, out, 0, events, events_size);
  sp_plot_samples((*(out.samples)), song_duration);
  sp_events_free(events, events_size);
exit:
  return ((status.id));
}
