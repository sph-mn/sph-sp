/** accumulate an integer phase and reset it after cycles.
  float value phases would be inexact and therefore harder to reset */
sp_time_t sp_phase_96(sp_time_t current, sp_time_t change) {
  sp_time_t result;
  result = (current + change);
  return (((96000 <= result) ? (result % 96000) : result));
}
/** accumulate an integer phase with change given as a float value.
  change must be a positive value and is rounded to the next larger integer */
sp_time_t sp_phase_96_float(sp_time_t current, double change) { return ((sp_phase_96(current, (sp_cheap_ceiling_positive(change))))); }
/** contains the initial phase offsets per partial and channel
  as a flat array. should be freed with free */
status_t sp_synth_state_new(sp_time_t channel_count, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_time_t** out_state) {
  status_declare;
  sp_time_t i;
  sp_time_t channel_i;
  status_require((sph_helper_calloc((channel_count * config_len * sizeof(sp_time_t)), out_state)));
  for (i = 0; (i < config_len); i = (1 + i)) {
    for (channel_i = 0; (channel_i < channel_count); channel_i = (1 + channel_i)) {
      (*out_state)[(channel_i + (channel_count * i))] = ((config[i]).phs)[channel_i];
    };
  };
exit:
  status_return;
}
/** create sines that start and end at specific times and can optionally modulate the frequency of others.
  sp-synth output is summed into out.
  amplitude and wavelength can be controlled by arrays separately for each partial and channel.
  modulators can be modulated themselves in chains. state has to be allocated by the caller with sp-synth-state-new.
  modulator amplitude is relative to carrier amplitude.
  paths are relative to the start of partials.
  # requirements
  * modulators must come after carriers in config
  * config-len must not change between calls with the same state
  * all amplitude/wavelength arrays must be of sufficient size and set for all channels
  * sp-initialise must have been called once before using sp-synth
  # algorithm
  * read config from last to first element
  * write modulator output to temporary buffers that are indexed by carrier id
  * apply modulator output from the buffers and sum to output for final carriers
  * each partial has integer phases that are reset in cycles and kept in state between calls */
status_t sp_synth(sp_block_t out, sp_time_t start, sp_time_t duration, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_time_t* phases) {
  status_declare;
  sp_sample_t amp;
  sp_sample_t* carrier;
  sp_time_t channel_i;
  sp_time_t end;
  sp_time_t i;
  sp_sample_t modulated_wvl;
  sp_sample_t* modulation_index[sp_synth_partial_limit][sp_channel_limit];
  sp_sample_t* modulation;
  sp_time_t phs;
  sp_time_t prt_duration;
  sp_synth_count_t prt_i;
  sp_time_t prt_offset_right;
  sp_time_t prt_offset;
  sp_synth_partial_t prt;
  sp_time_t prt_start;
  sp_time_t wvl;
  /* modulation blocks (channel array + data. at least one would be carrier) */
  memreg_init(((config_len - 1) * out.channels));
  memset(modulation_index, 0, (sizeof(modulation_index)));
  end = (start + duration);
  prt_i = config_len;
  while (prt_i) {
    /* sequencing of partials */
    prt_i = (prt_i - 1);
    prt = config[prt_i];
    if (end < prt.start) {
      break;
    };
    if (prt.end <= start) {
      continue;
    };
    /* offsets relative to the currently generated block */
    prt_start = ((prt.start < start) ? (start - prt.start) : 0);
    prt_offset = ((prt.start > start) ? (prt.start - start) : 0);
    prt_offset_right = ((prt.end > end) ? 0 : (end - prt.end));
    prt_duration = (duration - prt_offset - prt_offset_right);
    if (prt.modifies) {
      for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
        /* get modulator output buffer */
        carrier = modulation_index[(prt.modifies - 1)][channel_i];
        if (!carrier) {
          status_require((sph_helper_calloc((duration * sizeof(sp_sample_t)), (&carrier))));
          memreg_add(carrier);
        };
        phs = phases[(channel_i + (out.channels * prt_i))];
        modulation = modulation_index[prt_i][channel_i];
        /* calculate sample and sum with carrier */
        for (i = 0; (i < prt_duration); i = (1 + i)) {
          amp = (prt.amp)[channel_i][(prt_start + i)];
          carrier[(prt_offset + i)] = (carrier[(prt_offset + i)] + (amp * sp_sine_96(phs)));
          wvl = (prt.wvl)[channel_i][(prt_start + i)];
          modulated_wvl = (modulation ? (wvl + (wvl * modulation[(prt_offset + i)])) : wvl);
          phs = sp_phase_96_float(phs, (48000 / modulated_wvl));
        };
        /* save modulated output */
        phases[(channel_i + (out.channels * prt_i))] = phs;
        modulation_index[(prt.modifies - 1)][channel_i] = carrier;
      };
    } else {
      for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
        /* is carrier. sums into main output */
        phs = phases[(channel_i + (out.channels * prt_i))];
        modulation = modulation_index[prt_i][channel_i];
        for (i = 0; (i < prt_duration); i = (1 + i)) {
          amp = (prt.amp)[channel_i][(prt_start + i)];
          wvl = (prt.wvl)[channel_i][(prt_start + i)];
          modulated_wvl = (modulation ? (wvl + (wvl * modulation[(prt_offset + i)])) : wvl);
          phs = sp_phase_96_float(phs, (48000 / modulated_wvl));
          ((out.samples)[channel_i])[(prt_offset + i)] = (((out.samples)[channel_i])[(prt_offset + i)] + (amp * sp_sine_96(phs)));
        };
        phases[(channel_i + (out.channels * prt_i))] = phs;
      };
    };
  };
exit:
  memreg_free;
  status_return;
}
#define sp_synth_partial_set_channel(prt, channel, amp_array, wvl_array, phs_array) \
  (prt.amp)[channel] = amp_array; \
  (prt.wvl)[channel] = wvl_array; \
  (prt.phs)[channel] = phs_array
/** setup a synth partial with one channel */
sp_synth_partial_t sp_synth_partial_1(sp_time_t start, sp_time_t end, sp_synth_count_t modifies, sp_sample_t* amp, sp_time_t* wvl, sp_time_t phs) {
  sp_synth_partial_t prt;
  prt.start = start;
  prt.end = end;
  prt.modifies = modifies;
  sp_synth_partial_set_channel(prt, 0, amp, wvl, phs);
  return (prt);
}
/** setup a synth partial with two channels */
sp_synth_partial_t sp_synth_partial_2(sp_time_t start, sp_time_t end, sp_synth_count_t modifies, sp_sample_t* amp1, sp_sample_t* amp2, sp_time_t* wvl1, sp_time_t* wvl2, sp_time_t phs1, sp_time_t phs2) {
  sp_synth_partial_t prt;
  prt.start = start;
  prt.end = end;
  prt.modifies = modifies;
  sp_synth_partial_set_channel(prt, 0, amp1, wvl1, phs1);
  sp_synth_partial_set_channel(prt, 1, amp2, wvl2, phs2);
  return (prt);
}
/** return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0 */
sp_sample_t sp_triangle(sp_time_t t, sp_time_t a, sp_time_t b) {
  sp_time_t remainder;
  remainder = (t % (a + b));
  return (((remainder < a) ? (remainder * (1 / ((sp_sample_t)(a)))) : ((((sp_sample_t)(b)) - (remainder - ((sp_sample_t)(a)))) * (1 / ((sp_sample_t)(b))))));
}
sp_sample_t sp_triangle_96(sp_time_t t) { return ((sp_triangle(t, 48000, 48000))); }
sp_sample_t sp_square_96(sp_time_t t) { return (((((2 * t) % (2 * 96000)) < 96000) ? -1 : 1)); }
/** writes a sine wave of size into out. can be used to create lookup tables */
status_t sp_sine_table_new(sp_sample_t** out, sp_time_t size) {
  status_declare;
  sp_time_t i;
  sp_sample_t* out_array;
  status_require((sph_helper_malloc((size * sizeof(sp_sample_t*)), (&out_array))));
  for (i = 0; (i < size); i = (1 + i)) {
    out_array[i] = sin((i * (M_PI / (size / 2))));
  };
  *out = out_array;
exit:
  status_return;
}
