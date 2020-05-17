/* creates few seconds long wav file with a synthesised bassdrum.
an example for using the synthesiser and sequence output using event groups.
can be compiled with: gcc -lsph-sp example.c -o example.exe */
#include <stdio.h>
#include <sph-sp.h>
status_t
g1_new(sp_time_t start, sp_time_t rate, sp_event_t* out)
{
  status_declare;
  sp_event_t b1;
  sp_time_t b1_duration;
  sp_synth_partial_t b1_partials[1];
  sp_sample_t* b1_p1_amp;
  sp_time_t* b1_p1_wvl;
  sp_group_declare(g);
  status_require((sp_group_new(0, 2, 4, (&g))));
  b1_duration = rt(1, 2);
  status_require((sp_path_samples_3((&b1_p1_amp),
    b1_duration,
    (sp_path_line((rt(1, 8)), 1)),
    (sp_path_line((rt(3, 8)), 0)),
    (sp_path_line(b1_duration, 0)))));
  sp_group_memory_add(g, b1_p1_amp);
  status_require((sp_path_times_3((&b1_p1_wvl),
    b1_duration,
    (sp_path_line((rt(1, 8)), (rt(1, 128)))),
    (sp_path_line((rt(2, 8)), (rt(1, 64)))),
    (sp_path_line(b1_duration, (rt(1, 16)))))));
  sp_group_memory_add(g, b1_p1_wvl);
  b1_partials[0] = sp_synth_partial_2(
    0, b1_duration, 0, b1_p1_amp, b1_p1_amp, b1_p1_wvl, b1_p1_wvl, 0, 0);
  status_require((sp_synth_event(0, b1_duration, 2, 1, b1_partials, (&b1))));
  sp_group_add(g, b1);
  sp_group_prepare(g);
  *out = g;
exit:
  if (status_is_failure) {
    sp_group_free(g);
  };
  status_return;
}
status_t
song_new(sp_time_t rate, sp_event_t* out)
{
  status_declare;
  sp_event_t e;
  sp_group_declare(song);
  status_require((sp_group_new(0, 2, 0, (&song))));
  status_require((g1_new(0, rate, (&e))));
  sp_group_append((&song), e);
  status_require((g1_new(0, rate, (&e))));
  sp_group_append((&song), e);
  sp_group_prepare(song);
  *out = song;
exit:
  return (status);
}
int
main()
{
  status_declare;
  sp_initialise(1);
  sp_event_t song;
  sp_time_t rate;
  declare_render_config(render_config);
  rate = render_config.rate;
  status_require((song_new(rate, (&song))));
  status_require(
    (sp_render_file(song, 0, (rt(3, 1)), render_config, ("/tmp/song.wav"))));
  (song.free)((&song));
exit:
  printf("status: %d\n", (status.id));
  return ((status.id));
}
