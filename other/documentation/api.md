# functions
~~~
sp_block_to_file :: sp_block_t:block uint8_t*:path sp_time_t:rate -> status_t
sp_block_copy :: sp_block_t:a sp_block_t:b -> void
sp_block_free :: sp_block_t*:a -> void
sp_block_new :: sp_channel_count_t:channel_count sp_time_t:sample_count sp_block_t*:out_block -> status_t
sp_block_with_offset :: sp_block_t:a sp_time_t:offset -> sp_block_t
sp_block_zero :: sp_block_t:a -> void
sp_compositions_max :: sp_size_t:sum -> sp_size_t
sp_compositions_max :: sp_time_t:sum -> sp_time_t
sp_convolution_filter :: sp_sample_t*:in sp_time_t:in_len sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_convolution_filter_state_free :: sp_convolution_filter_state_t*:state -> void
sp_convolution_filter_state_set :: sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state -> status_t
sp_convolve :: sp_sample_t*:a sp_time_t:a_len sp_sample_t*:b sp_time_t:b_len sp_time_t:result_carryover_len sp_sample_t*:result_carryover sp_sample_t*:result_samples -> void
sp_convolve_one :: sp_sample_t*:a sp_time_t:a_len sp_sample_t*:b sp_time_t:b_len sp_sample_t*:result_samples -> void
sp_deinitialize :: -> void
sp_envelope_scale :: sp_time_t**:out sp_time_t:length sp_path_point_count_t:point_count sp_sample_t:y_scalar sp_sample_t*:x sp_sample_t*:y sp_sample_t*:c -> status_t
sp_envelope_scale_curve3 :: sp_time_t**:out sp_time_t:length sp_sample_t:y_scalar sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:c1 sp_sample_t:c2 -> status_t
sp_envelope_scale_curve4 :: sp_time_t**:out sp_time_t:length sp_sample_t:y_scalar sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 -> status_t
sp_envelope_scale_curve5 :: sp_time_t**:out sp_time_t:length sp_sample_t:y_scalar sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:y5 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 sp_sample_t:c4 -> status_t
sp_envelope_scale3 :: sp_time_t**:out sp_time_t:length sp_sample_t:y_scalar sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 -> status_t
sp_envelope_scale4 :: sp_time_t**:out sp_time_t:length sp_sample_t:y_scalar sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 -> status_t
sp_envelope_scale5 :: sp_time_t**:out sp_time_t:length sp_sample_t:y_scalar sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:y5 -> status_t
sp_envelope_zero :: sp_sample_t**:out sp_time_t:length sp_path_point_count_t:point_count sp_sample_t*:x sp_sample_t*:y sp_sample_t*:c -> status_t
sp_envelope_zero_curve3 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:c1 sp_sample_t:c2 -> status_t
sp_envelope_zero_curve4 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 -> status_t
sp_envelope_zero_curve5 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 sp_sample_t:c4 -> status_t
sp_envelope_zero3 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:y1 -> status_t
sp_envelope_zero4 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 -> status_t
sp_envelope_zero5 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 -> status_t
sp_event_list_add :: sp_event_list_t**:a sp_event_t:event -> status_t
sp_event_list_display :: sp_event_list_t*:a -> void
sp_event_list_free :: sp_event_list_t**:events -> void
sp_event_list_remove :: sp_event_list_t**:a sp_event_list_t*:element -> void
sp_event_list_reverse :: sp_event_list_t**:a -> void
sp_event_list_validate :: sp_event_list_t*:a -> void
sp_event_memory_add_with_handler :: sp_event_t*:event void*:address sp_memory_free_t:handler -> status_t
sp_event_memory_ensure :: sp_event_t*:a sp_time_t:additional_size -> status_t
sp_event_memory_fixed_add_with_handler :: sp_event_t*:event void*:address sp_memory_free_t:handler -> void
sp_event_memory_free :: sp_event_t*:event -> void
sp_event_schedule :: sp_event_t:event sp_time_t:onset sp_time_t:duration void*:config -> sp_event_t
sp_fft :: sp_time_t:input_len double*:input_or_output_real double*:input_or_output_imag -> int
sp_ffti :: sp_time_t:input_len double*:input_or_output_real double*:input_or_output_imag -> int
sp_file_close_read :: sp_file_t:file -> void
sp_file_close_write :: sp_file_t*:file -> void
sp_file_open_read :: uint8_t*:path sp_file_t*:file -> status_t
sp_file_open_write :: uint8_t*:path sp_channel_count_t:channel_count sp_time_t:sample_rate sp_file_t*:file -> status_t
sp_file_read :: sp_file_t:file sp_time_t:sample_count sp_sample_t**:samples -> status_t
sp_file_write :: sp_file_t*:file sp_sample_t**:samples sp_time_t:sample_count -> status_t
sp_group_add :: sp_event_t*:a sp_event_t:event -> status_t
sp_group_append :: sp_event_t*:a sp_event_t:event -> status_t
sp_group_event_f :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> void
sp_group_event_free :: sp_event_t*:a -> void
sp_group_event_parallel_f :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> void
sp_group_free :: sp_event_t*:group -> void
sp_group_generate :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:a -> status_t
sp_group_generate_parallel :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:a -> status_t
sp_group_prepare :: sp_event_t*:event -> status_t
sp_group_prepare_parallel :: sp_event_t*:a -> status_t
sp_initialize :: uint16_t:cpu_count sp_channel_count_t:channel_count sp_time_t:rate -> status_t
sp_map_event_config_new_n :: sp_time_t:count sp_map_event_config_t**:out -> status_t
sp_map_event_free :: sp_event_t*:event -> void
sp_map_event_generate :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> status_t
sp_map_event_isolated_generate :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> status_t
sp_map_event_prepare :: sp_event_t*:event -> status_t
sp_modulo_match :: size_t:index size_t*:divisors size_t:divisor_count -> size_t
sp_moving_average :: sp_sample_t*:in sp_time_t:in_size sp_sample_t*:prev sp_sample_t*:next sp_time_t:radius sp_sample_t*:out -> void
sp_noise_event_config_defaults :: -> sp_noise_event_config_t
sp_noise_event_config_new_n :: sp_time_t:count sp_noise_event_config_t**:out -> status_t
sp_noise_event_free :: sp_event_t*:event -> void
sp_noise_event_generate :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> status_t
sp_noise_event_prepare :: sp_event_t*:event -> status_t
sp_normal_random :: sp_time_t:min sp_time_t:max -> sp_time_t
sp_null_ir :: sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_pan_to_amp :: sp_sample_t:value sp_channel_count_t:channel -> sp_sample_t
sp_pan_to_amp :: sp_sample_t:value sp_channel_count_t:channel -> sp_sample_t
sp_passthrough_ir :: sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_path_samples :: sp_sample_t**:out sp_time_t:length sp_path_point_count_t:point_count sp_sample_t*:x sp_sample_t*:y sp_sample_t*:c -> status_t
sp_path_samples_curve2 :: sp_sample_t**:out sp_time_t:length sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:c1 -> status_t
sp_path_samples_curve3 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:c1 sp_sample_t:c2 -> status_t
sp_path_samples_curve4 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 -> status_t
sp_path_samples_curve5 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:y5 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 sp_sample_t:c4 -> status_t
sp_path_samples2 :: sp_sample_t**:out sp_time_t:length sp_sample_t:y1 sp_sample_t:y2 -> status_t
sp_path_samples3 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 -> status_t
sp_path_samples4 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 -> status_t
sp_path_samples5 :: sp_sample_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:y5 -> status_t
sp_path_times :: sp_time_t**:out sp_time_t:length sp_path_point_count_t:point_count sp_sample_t*:x sp_sample_t*:y sp_sample_t*:c -> status_t
sp_path_times_curve2 :: sp_time_t**:out sp_time_t:length sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:c1 -> status_t
sp_path_times_curve3 :: sp_time_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:c1 sp_sample_t:c2 -> status_t
sp_path_times_curve4 :: sp_time_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 -> status_t
sp_path_times_curve5 :: sp_time_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:y5 sp_sample_t:c1 sp_sample_t:c2 sp_sample_t:c3 sp_sample_t:c4 -> status_t
sp_path_times2 :: sp_time_t**:out sp_time_t:length sp_sample_t:y1 sp_sample_t:y2 -> status_t
sp_path_times3 :: sp_time_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 -> status_t
sp_path_times4 :: sp_time_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 -> status_t
sp_path_times5 :: sp_time_t**:out sp_time_t:length sp_sample_t:x1 sp_sample_t:x2 sp_sample_t:x3 sp_sample_t:y1 sp_sample_t:y2 sp_sample_t:y3 sp_sample_t:y4 sp_sample_t:y5 -> status_t
sp_permutations_max :: sp_size_t:set_size sp_size_t:selection_size -> sp_size_t
sp_permutations_max :: sp_time_t:set_size sp_time_t:selection_size -> sp_time_t
sp_phase :: sp_time_t:current sp_time_t:change sp_time_t:cycle -> sp_time_t
sp_phase_float :: sp_time_t:current double:change sp_time_t:cycle -> sp_time_t
sp_plot_samples :: sp_sample_t*:a sp_time_t:a_size -> void
sp_plot_samples_to_file :: sp_sample_t*:a sp_time_t:a_size uint8_t*:path -> void
sp_plot_samples_file :: uint8_t*:path uint8_t:use_steps -> void
sp_plot_spectrum :: sp_sample_t*:a sp_time_t:a_size -> void
sp_plot_spectrum_to_file :: sp_sample_t*:a sp_time_t:a_size uint8_t*:path -> void
sp_plot_spectrum_file :: uint8_t*:path -> void
sp_plot_times :: sp_time_t*:a sp_time_t:a_size -> void
sp_plot_times_to_file :: sp_time_t*:a sp_time_t:a_size uint8_t*:path -> void
sp_random_state_new :: sp_time_t:seed -> sp_random_state_t
sp_render_config :: sp_channel_count_t:channel_count sp_time_t:rate sp_time_t:block_size sp_bool_t:display_progress -> sp_render_config_t
sp_render_file :: sp_event_t:event uint8_t*:path -> status_t
sp_render_plot :: sp_event_t:event -> status_t
sp_render_range_block :: sp_event_t:event sp_time_t:start sp_time_t:end sp_render_config_t:config sp_block_t*:out -> status_t
sp_render_range_file :: sp_event_t:event sp_time_t:start sp_time_t:end sp_render_config_t:config uint8_t*:path -> status_t
sp_sample_max :: sp_sample_t:a sp_sample_t:b -> sp_sample_t
sp_sample_min :: sp_sample_t:a sp_sample_t:b -> sp_sample_t
sp_sample_random_discrete_bounded :: sp_time_t*:cudist sp_time_t:cudist_size sp_sample_t:range -> sp_sample_t
sp_samples_to_times :: sp_sample_t*:in sp_size_t:count sp_time_t*:out -> void
sp_samples_to_times_replace :: sp_sample_t*:in sp_size_t:count sp_time_t**:out -> status_t
sp_samples_to_units :: sp_sample_t*:in_out sp_size_t:count -> void
sp_samples_beta_distribution :: sp_sample_t:base sp_sample_t:alpha sp_sample_t:beta_param sp_time_t:count sp_sample_t*:out -> void
sp_samples_binary_mask :: sp_sample_t:base uint8_t*:pattern sp_time_t:pattern_len sp_time_t:count sp_sample_t*:out -> void
sp_samples_blend :: sp_sample_t*:a sp_sample_t*:b sp_sample_t:fraction sp_time_t:size sp_sample_t*:out -> void
sp_samples_display :: sp_sample_t*:in sp_size_t:count -> void
sp_samples_divisions :: sp_sample_t:start sp_sample_t:divisor sp_time_t:count sp_sample_t*:out -> void
sp_samples_gaussian :: sp_sample_t:base sp_sample_t:centre sp_sample_t:width sp_sample_t:count sp_sample_t*:out -> void
sp_samples_geometric :: sp_sample_t:base sp_sample_t:ratio sp_time_t:count sp_sample_t*:out -> void
sp_samples_limit_abs :: sp_sample_t*:in sp_time_t:count sp_sample_t:limit sp_sample_t*:out -> void
sp_samples_logarithmic :: sp_sample_t:base sp_sample_t:scale sp_time_t:count sp_sample_t*:out -> void
sp_samples_scale_to_times :: sp_sample_t*:a sp_time_t:size sp_time_t:max sp_time_t*:out -> void
sp_samples_scale_sum :: sp_sample_t*:in sp_size_t:count sp_sample_t:target_y sp_sample_t*:out -> void
sp_samples_scale_y :: sp_sample_t*:in sp_time_t:count sp_sample_t:target_y -> void
sp_samples_segment_steps :: sp_sample_t:base sp_sample_t*:levels sp_time_t:segments sp_time_t:count sp_sample_t*:out -> void
sp_samples_set_gain :: sp_sample_t*:in_out sp_size_t:count sp_sample_t:amp -> void
sp_samples_set_gain :: sp_sample_t*:in_out sp_size_t:count sp_sample_t:amp -> void
sp_samples_set_unity_gain :: sp_sample_t*:in_out sp_sample_t*:reference sp_size_t:count -> void
sp_scale_canonical :: sp_scale_t:scale sp_time_t:divisions -> static inline sp_scale_t
sp_scale_divisions :: sp_scale_t:scale -> static inline sp_time_t
sp_scale_first_index :: sp_scale_t:scale -> static inline sp_time_t
sp_scale_make :: sp_time_t*:pitch_classes sp_time_t:count sp_time_t:divisions -> static inline sp_scale_t
sp_scale_mask :: sp_time_t:divisions -> static inline sp_scale_t
sp_scale_rotate :: sp_scale_t:scale sp_time_t:steps sp_time_t:divisions -> static inline sp_scale_t
sp_seq :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_list_t**:events -> status_t
sp_seq_parallel :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_list_t**:events -> status_t
sp_sequence_max :: sp_time_t:size sp_time_t:min_size -> sp_time_t
sp_set_sequence_max :: sp_size_t:set_size sp_size_t:selection_size -> sp_size_t
sp_set_sequence_max :: sp_time_t:set_size sp_time_t:selection_size -> sp_time_t
sp_shuffle :: function_pointer void void* sp_size_t sp_size_t:swap void*:in sp_size_t:count -> void
sp_sinc :: sp_sample_t:a -> sp_sample_t
sp_sine :: sp_time_t:size sp_sample_t:amp sp_sample_t*:amod sp_time_t:frq sp_time_t*:fmod sp_time_t*:phs_state sp_sample_t*:out -> void
sp_sine_lfo :: sp_time_t:size sp_sample_t:amp sp_sample_t*:amod sp_time_t:frq sp_time_t*:fmod sp_time_t*:phs_state sp_sample_t*:out -> void
sp_sine_period :: sp_time_t:size sp_sample_t*:out -> void
sp_spectral_inversion_ir :: sp_sample_t*:a sp_time_t:a_len -> void
sp_spectral_reversal_ir :: sp_sample_t*:a sp_time_t:a_len -> void
sp_square :: sp_time_t:t sp_time_t:size -> sp_sample_t
sp_stat_repetition_all_max :: sp_time_t:size -> sp_time_t
sp_stat_repetition_max :: sp_time_t:size sp_time_t:width -> sp_time_t
sp_stat_samples_center :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_deviation :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_inharmonicity :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_kurtosis :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_mean :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_median :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_range :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_skewness :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_center :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_deviation :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_inharmonicity :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_kurtosis :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_mean :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_median :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_range :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_skewness :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_unique_all_max :: sp_time_t:size -> sp_time_t
sp_stat_unique_max :: sp_time_t:size sp_time_t:width -> sp_time_t
sp_status_description :: status_t:a -> uint8_t*
sp_status_name :: status_t:a -> uint8_t*
sp_time_deharmonize :: sp_time_t:a sp_time_t:base sp_sample_t:amount -> sp_time_t
sp_time_expt :: sp_time_t:base sp_time_t:exp -> sp_time_t
sp_time_factorial :: sp_time_t:a -> sp_time_t
sp_time_harmonize :: sp_time_t:a sp_time_t:base sp_sample_t:amount -> sp_time_t
sp_time_harmonize :: sp_time_t:a sp_time_t:base sp_sample_t:amount -> sp_time_t
sp_time_max :: sp_time_t:a sp_time_t:b -> sp_time_t
sp_time_min :: sp_time_t:a sp_time_t:b -> sp_time_t
sp_time_random_discrete :: sp_time_t*:cudist sp_time_t:cudist_size -> sp_time_t
sp_time_random_discrete_bounded :: sp_time_t*:cudist sp_time_t:cudist_size sp_time_t:range -> sp_time_t
sp_time_sum :: sp_time_t*:in sp_time_t:size -> sp_time_t
sp_times_to_samples :: sp_time_t*:in sp_size_t:count sp_sample_t*:out -> void
sp_times_to_samples_replace :: sp_time_t*:in sp_size_t:count sp_sample_t**:out -> status_t
sp_times_bits_to_times :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_blend :: sp_time_t*:a sp_time_t*:b sp_sample_t:fraction sp_time_t:size sp_time_t*:out -> void
sp_times_compositions :: sp_time_t:sum sp_time_t***:out sp_time_t*:out_size sp_time_t**:out_sizes -> status_t
sp_times_constant :: sp_size_t:count sp_time_t:value sp_time_t**:out -> status_t
sp_times_contains :: sp_time_t*:in sp_size_t:count sp_time_t:value -> uint8_t
sp_times_display :: sp_time_t*:in sp_size_t:count -> void
sp_times_extract_in_range :: sp_time_t*:a sp_time_t:size sp_time_t:min sp_time_t:max sp_time_t*:out sp_time_t*:out_size -> void
sp_times_geometric :: sp_time_t:base sp_time_t:ratio sp_time_t:count sp_time_t*:out -> void
sp_times_gt_indices :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out sp_time_t*:out_size -> void
sp_times_insert_space :: sp_time_t*:in sp_time_t:size sp_time_t:index sp_time_t:count sp_time_t*:out -> void
sp_times_limit :: sp_time_t*:a sp_time_t:count sp_time_t:n sp_time_t*:out -> void
sp_times_logarithmic :: sp_time_t:base sp_sample_t:scale sp_time_t:count sp_time_t*:out -> void
sp_times_make_seamless_left :: sp_time_t*:a sp_time_t:a_count sp_time_t*:b sp_time_t:b_count sp_time_t*:out -> void
sp_times_make_seamless_right :: sp_time_t*:a sp_time_t:a_count sp_time_t*:b sp_time_t:b_count sp_time_t*:out -> void
sp_times_mask :: sp_time_t*:a sp_time_t*:b sp_sample_t*:coefficients sp_time_t:size sp_time_t*:out -> void
sp_times_permutations :: sp_time_t:size sp_time_t*:set sp_time_t:set_size sp_time_t***:out sp_time_t*:out_size -> status_t
sp_times_random_binary :: sp_time_t:size sp_time_t*:out -> status_t
sp_times_random_discrete :: sp_time_t*:cudist sp_time_t:cudist_size sp_time_t:count sp_time_t*:out -> void
sp_times_random_discrete_unique :: sp_time_t*:cudist sp_time_t:cudist_size sp_time_t:size sp_time_t*:out -> void
sp_times_remove :: sp_time_t*:in sp_time_t:size sp_time_t:index sp_time_t:count sp_time_t*:out -> void
sp_times_scale :: sp_time_t*:in sp_size_t:count sp_time_t:factor sp_time_t*:out -> status_t
sp_times_scale_sum :: sp_time_t*:in sp_size_t:count sp_time_t:sum sp_time_t*:out -> void
sp_times_scale_y :: sp_time_t*:in sp_size_t:count sp_time_t:target_y sp_time_t*:out -> void
sp_times_select :: sp_time_t*:in sp_time_t*:indices sp_time_t:count sp_time_t*:out -> void
sp_times_select_random :: sp_time_t*:a sp_time_t:size sp_time_t*:out sp_time_t*:out_size -> void
sp_times_sequence_increment :: sp_time_t*:in sp_size_t:size sp_size_t:set_size -> void
sp_times_sequences :: sp_time_t:base sp_time_t:digits sp_time_t:size sp_time_t*:out -> void
sp_times_shuffle_swap :: void*:a sp_size_t:i1 sp_size_t:i2 -> void
sp_times_subdivide_difference :: sp_time_t*:a sp_time_t:size sp_time_t:index sp_time_t:count sp_time_t*:out -> void
sp_times_sum :: sp_time_t*:a sp_time_t:size -> sp_time_t
sp_triangle :: sp_time_t:t sp_time_t:a sp_time_t:b -> sp_sample_t
sp_u64_from_array :: uint8_t*:a sp_time_t:count -> uint64_t
sp_wave :: sp_time_t:size sp_sample_t*:wvf sp_time_t:wvf_size sp_sample_t:amp sp_sample_t*:amod sp_time_t:frq sp_time_t*:fmod sp_time_t*:phs_state sp_sample_t*:out -> void
sp_wave_event_config_defaults :: -> sp_wave_event_config_t
sp_wave_event_config_new_n :: sp_time_t:count sp_wave_event_config_t**:out -> status_t
sp_wave_event_generate :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> status_t
sp_wave_event_prepare :: sp_event_t*:event -> status_t
sp_window_blackman :: sp_sample_t:a sp_time_t:width -> sp_sample_t
sp_windowed_sinc_bp_br :: sp_sample_t*:in sp_time_t:in_len sp_sample_t:cutoff_l sp_sample_t:cutoff_h sp_sample_t:transition_l sp_sample_t:transition_h sp_bool_t:is_reject sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_windowed_sinc_bp_br_ir :: sp_sample_t:cutoff_l sp_sample_t:cutoff_h sp_sample_t:transition_l sp_sample_t:transition_h sp_bool_t:is_reject sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_bp_br_ir_f :: void*:arguments sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_ir :: sp_sample_t:cutoff sp_sample_t:transition sp_time_t*:result_len sp_sample_t**:result_ir -> status_t
sp_windowed_sinc_lp_hp :: sp_sample_t*:in sp_time_t:in_len sp_sample_t:cutoff sp_sample_t:transition sp_bool_t:is_high_pass sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_windowed_sinc_lp_hp_ir :: sp_sample_t:cutoff sp_sample_t:transition sp_bool_t:is_high_pass sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir :: sp_sample_t:cutoff sp_sample_t:transition sp_bool_t:is_high_pass sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir_f :: void*:arguments sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir_length :: sp_sample_t:transition -> sp_time_t
sp_sample_round_to_multiple :: sp_sample_t:a sp_sample_t:base -> sp_sample_t
sp_sample_sort_less :: void*:a ssize_t:b ssize_t:c -> uint8_t
sp_sample_sort_swap :: void*:a ssize_t:b ssize_t:c -> void
sp_samples_absolute_max :: value_t*:in sp_size_t:count -> sp_sample_t
sp_samples_add :: value_t*:in_out sp_size_t:count sp_sample_t:value -> void
sp_samples_add_samples :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_samples_additions :: sp_sample_t:start sp_sample_t:summand sp_sample_t:count value_t*:out -> void
sp_samples_and_samples :: value_t*:a value_t*:b sp_size_t:count sp_sample_t:limit value_t*:out -> void
sp_samples_array_free :: value_t**:in sp_size_t:count -> void
sp_samples_bessel :: sp_sample_t:base sp_sample_t:count value_t*:out -> void
sp_samples_clustered :: sp_sample_t:center sp_sample_t:spread sp_sample_t:count value_t*:out -> void
sp_samples_copy :: value_t*:in sp_size_t:count value_t*:out -> void
sp_samples_cumulative :: sp_sample_t:base value_t*:deltas sp_time_t:count value_t*:out -> void
sp_samples_cusum :: value_t*:in sp_sample_t:count value_t*:out -> void
sp_samples_decumulative :: sp_sample_t:base value_t*:deltas sp_time_t:count value_t*:out -> void
sp_samples_divide :: value_t*:in_out sp_size_t:count sp_sample_t:value -> void
sp_samples_divide_samples :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_samples_duplicate :: value_t*:a sp_size_t:count value_t**:out -> status_t
sp_samples_equal :: value_t*:in sp_size_t:count sp_sample_t:value -> sp_bool_t
sp_samples_even_harmonics :: sp_sample_t:base sp_time_t:count value_t*:out -> void
sp_samples_exponential :: sp_sample_t:base sp_sample_t:k sp_sample_t:count value_t*:out -> void
sp_samples_fixed_sets :: sp_sample_t:base value_t*:ratios sp_sample_t:len value_t*:out -> void
sp_samples_linear :: sp_sample_t:base sp_sample_t:k sp_sample_t:count value_t*:out -> void
sp_samples_logistic :: sp_sample_t:base sp_sample_t:k sp_sample_t:count value_t*:out -> void
sp_samples_max :: value_t*:in sp_size_t:count -> sp_sample_t
sp_samples_min :: value_t*:in sp_size_t:count -> sp_sample_t
sp_samples_modular_series :: sp_sample_t:base sp_time_t:mod sp_sample_t:delta sp_time_t:count value_t*:out -> void
sp_samples_multiplications :: sp_sample_t:base sp_time_t:count value_t*:out -> void
sp_samples_multiply :: value_t*:in_out sp_size_t:count sp_sample_t:value -> void
sp_samples_multiply_samples :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_samples_new :: sp_size_t:count value_t**:out -> status_t
sp_samples_nth_harmonics :: sp_sample_t:base sp_sample_t:k sp_time_t:count value_t*:out -> void
sp_samples_odd_harmonics :: sp_sample_t:base sp_time_t:count value_t*:out -> void
sp_samples_or_samples :: value_t*:a value_t*:b sp_size_t:count sp_sample_t:limit value_t*:out -> void
sp_samples_power :: sp_sample_t:base sp_sample_t:p sp_sample_t:count value_t*:out -> void
sp_samples_prime_indexed :: sp_sample_t:base sp_time_t:count value_t*:out -> void
sp_samples_range :: value_t*:in sp_size_t:start sp_size_t:end value_t*:out -> void
sp_samples_reverse :: value_t*:in sp_size_t:count value_t*:out -> void
sp_samples_set :: value_t*:in_out sp_size_t:count sp_sample_t:value -> void
sp_samples_set_samples :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_samples_set_samples_left :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_samples_shuffle :: value_t*:in sp_size_t:count -> void
sp_samples_sort_ascending :: value_t*:a sp_size_t:count -> void
sp_samples_square :: value_t*:in sp_size_t:count -> void
sp_samples_subtract :: value_t*:in_out sp_size_t:count sp_sample_t:value -> void
sp_samples_subtract_samples :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_samples_swap :: sp_time_t*:in_out sp_ssize_t:index_1 sp_ssize_t:index_2 -> void
sp_samples_xor_samples :: value_t*:a value_t*:b sp_size_t:count sp_sample_t:limit value_t*:out -> void
sp_time_round_to_multiple :: sp_time_t:a sp_time_t:base -> sp_time_t
sp_time_sort_less :: void*:a ssize_t:b ssize_t:c -> uint8_t
sp_time_sort_swap :: void*:a ssize_t:b ssize_t:c -> void
sp_times_absolute_max :: value_t*:in sp_size_t:count -> sp_time_t
sp_times_add :: value_t*:in_out sp_size_t:count sp_time_t:value -> void
sp_times_add_times :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_times_additions :: sp_time_t:start sp_time_t:summand sp_time_t:count value_t*:out -> void
sp_times_and_times :: value_t*:a value_t*:b sp_size_t:count sp_time_t:limit value_t*:out -> void
sp_times_array_free :: value_t**:in sp_size_t:count -> void
sp_times_bessel :: sp_time_t:base sp_time_t:count value_t*:out -> void
sp_times_clustered :: sp_time_t:center sp_time_t:spread sp_time_t:count value_t*:out -> void
sp_times_copy :: value_t*:in sp_size_t:count value_t*:out -> void
sp_times_cumulative :: sp_time_t:base value_t*:deltas sp_time_t:count value_t*:out -> void
sp_times_cusum :: value_t*:in sp_time_t:count value_t*:out -> void
sp_times_decumulative :: sp_time_t:base value_t*:deltas sp_time_t:count value_t*:out -> void
sp_times_divide :: value_t*:in_out sp_size_t:count sp_time_t:value -> void
sp_times_divide_times :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_times_duplicate :: value_t*:a sp_size_t:count value_t**:out -> status_t
sp_times_equal :: value_t*:in sp_size_t:count sp_time_t:value -> sp_bool_t
sp_times_even_harmonics :: sp_time_t:base sp_time_t:count value_t*:out -> void
sp_times_exponential :: sp_time_t:base sp_time_t:k sp_time_t:count value_t*:out -> void
sp_times_fixed_sets :: sp_time_t:base value_t*:ratios sp_time_t:len value_t*:out -> void
sp_times_linear :: sp_time_t:base sp_time_t:k sp_time_t:count value_t*:out -> void
sp_times_logistic :: sp_time_t:base sp_time_t:k sp_time_t:count value_t*:out -> void
sp_times_max :: value_t*:in sp_size_t:count -> sp_time_t
sp_times_min :: value_t*:in sp_size_t:count -> sp_time_t
sp_times_modular_series :: sp_time_t:base sp_time_t:mod sp_sample_t:delta sp_time_t:count value_t*:out -> void
sp_times_multiplications :: sp_time_t:base sp_time_t:count value_t*:out -> void
sp_times_multiply :: value_t*:in_out sp_size_t:count sp_time_t:value -> void
sp_times_multiply_times :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_times_new :: sp_size_t:count value_t**:out -> status_t
sp_times_nth_harmonics :: sp_time_t:base sp_time_t:k sp_time_t:count value_t*:out -> void
sp_times_odd_harmonics :: sp_time_t:base sp_time_t:count value_t*:out -> void
sp_times_or_times :: value_t*:a value_t*:b sp_size_t:count sp_time_t:limit value_t*:out -> void
sp_times_power :: sp_time_t:base sp_time_t:p sp_time_t:count value_t*:out -> void
sp_times_prime_indexed :: sp_time_t:base sp_time_t:count value_t*:out -> void
sp_times_range :: value_t*:in sp_size_t:start sp_size_t:end value_t*:out -> void
sp_times_reverse :: value_t*:in sp_size_t:count value_t*:out -> void
sp_times_set :: value_t*:in_out sp_size_t:count sp_time_t:value -> void
sp_times_set_times :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_times_set_times_left :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_times_shuffle :: value_t*:in sp_size_t:count -> void
sp_times_sort_ascending :: value_t*:a sp_size_t:count -> void
sp_times_square :: value_t*:in sp_size_t:count -> void
sp_times_subtract :: value_t*:in_out sp_size_t:count sp_time_t:value -> void
sp_times_subtract_times :: value_t*:in_out sp_size_t:count value_t*:in -> void
sp_times_swap :: sp_time_t*:in_out sp_ssize_t:index_1 sp_ssize_t:index_2 -> void
sp_times_xor_times :: value_t*:a value_t*:b sp_size_t:count sp_time_t:limit value_t*:out -> void
~~~

# macros
~~~
distributions_template_h(value_t, value_type_name, type_name)
error_memory_add(address)
error_memory_add2(address, handler)
error_memory_free
error_memory_init(register_size)
f128
f64
local_memory_add(address)
local_memory_add2(address, handler)
local_memory_free
local_memory_init(register_size)
sp_block_declare(a)
sp_block_plot_1(a)
sp_bool_t
sp_calloc_type(count, type, pointer_address)
sp_channel_count_limit
sp_channel_count_t
sp_cheap_ceiling_positive(a)
sp_cheap_floor_positive(a)
sp_cheap_round_positive(a)
sp_d
sp_declare_block(id)
sp_declare_event(id)
sp_declare_event_list(id)
sp_default_resolution
sp_define_event(name, _prepare, duration)
sp_define_samples(id, value)
sp_define_samples_new_srq(id, count)
sp_define_times(id, value)
sp_define_times_srq(id, count)
sp_duration(n, d)
sp_event_alloc_srq(event_pointer, allocator, size, pointer_address)
sp_event_config_get(a, type)
sp_event_duration(a)
sp_event_envelope_scale3_srq(event_pointer, out, ...)
sp_event_envelope_scale4_srq(event_pointer, out, ...)
sp_event_envelope_scale5_srq(event_pointer, out, ...)
sp_event_envelope_scale_curve3_srq(event_pointer, out, ...)
sp_event_envelope_scale_curve4_srq(event_pointer, out, ...)
sp_event_envelope_scale_curve5_srq(event_pointer, out, ...)
sp_event_envelope_scale_srq(event_pointer, out, ...)
sp_event_envelope_zero3_srq(event_pointer, out, ...)
sp_event_envelope_zero4_srq(event_pointer, out, ...)
sp_event_envelope_zero5_srq(event_pointer, out, ...)
sp_event_envelope_zero_curve3_srq(event_pointer, out, ...)
sp_event_envelope_zero_curve4_srq(event_pointer, out, ...)
sp_event_envelope_zero_curve5_srq(event_pointer, out, ...)
sp_event_envelope_zero_srq(event_pointer, out, ...)
sp_event_free(a)
sp_event_malloc_srq(event_pointer, size, pointer_address)
sp_event_malloc_type_n_srq(event_pointer, count, type, pointer_address)
sp_event_malloc_type_srq(event_pointer, type, pointer_address)
sp_event_memory_add(event, address)
sp_event_memory_array_add
sp_event_memory_fixed_add(event, address)
sp_event_path_samples3_srq(event_pointer, out, ...)
sp_event_path_samples4_srq(event_pointer, out, ...)
sp_event_path_samples5_srq(event_pointer, out, ...)
sp_event_path_samples_curve3_srq(event_pointer, out, ...)
sp_event_path_samples_curve4_srq(event_pointer, out, ...)
sp_event_path_samples_curve5_srq(event_pointer, out, ...)
sp_event_path_samples_srq(event_pointer, out, ...)
sp_event_path_times3_srq(event_pointer, out, ...)
sp_event_path_times4_srq(event_pointer, out, ...)
sp_event_path_times5_srq(event_pointer, out, ...)
sp_event_path_times_curve3_srq(event_pointer, out, ...)
sp_event_path_times_curve4_srq(event_pointer, out, ...)
sp_event_path_times_curve5_srq(event_pointer, out, ...)
sp_event_path_times_srq(event_pointer, out, ...)
sp_event_prepare_optional_srq(a)
sp_event_prepare_srq(a)
sp_event_reset(x)
sp_event_samples_srq(event_pointer, size, pointer_address)
sp_event_times_srq(event_pointer, size, pointer_address)
sp_event_units_srq(event_pointer, size, pointer_address)
sp_exp
sp_factor_to_hz(x)
sp_filter_passes_limit
sp_filter_passes_t
sp_filter_state_free
sp_filter_state_t
sp_frq_t
sp_group_event(event_pointer)
sp_group_event_list(event)
sp_group_parallel_event(event_pointer)
sp_hz_to_factor(x)
sp_hz_to_rad(a)
sp_hz_to_samples(x)
sp_inline_abs(a)
sp_inline_absolute_difference(a, b)
sp_inline_default(a, b)
sp_inline_limit(x, min_value, max_value)
sp_inline_max(a, b)
sp_inline_min(a, b)
sp_inline_mod(a, b)
sp_inline_no_underflow_subtract(a, b)
sp_inline_no_zero_divide(a, b)
sp_inline_optional(a, b)
sp_local_alloc_srq(allocator, size, pointer_address)
sp_local_samples_srq(size, pointer_address)
sp_local_times_srq(size, pointer_address)
sp_local_units_srq(size, pointer_address)
sp_malloc_type(count, type, pointer_address)
sp_malloc_type_srq(count, type, pointer_address)
sp_map_event(event_pointer, _config)
sp_map_event_config_new(out)
sp_max_frq
sp_memory_error
sp_noise
sp_noise_event(event_pointer, _config)
sp_noise_event_config_new(out)
sp_optional_array_get(array, fixed, index)
sp_path_point_count_limit
sp_path_point_count_t
sp_pow
sp_rad_to_hz(a)
sp_random_seed
sp_random_state_t
sp_rate_duration(n, d)
sp_realloc_type(count, type, pointer_address)
sp_render_block_seconds
sp_s_group_libc
sp_s_group_sp
sp_s_group_sph
sp_s_id_eof
sp_s_id_file_eof
sp_s_id_file_not_implemented
sp_s_id_file_read
sp_s_id_file_write
sp_s_id_input_type
sp_s_id_invalid_argument
sp_s_id_memory
sp_s_id_not_implemented
sp_s_id_undefined
sp_sample_interpolate_linear(a, b, t)
sp_sample_nearly_equal
sp_sample_random
sp_sample_random_primitive
sp_sample_t
sp_sample_to_time
sp_sample_to_unit(a)
sp_samples_nearly_equal
sp_samples_random(size, out)
sp_samples_random_bounded(range, size, out)
sp_samples_random_bounded_primitive
sp_samples_random_primitive
sp_samples_sum
sp_samples_to_hz(x)
sp_samples_zero(a, size)
sp_scale_t
sp_seq_events_prepare
sp_size_t
sp_ssize_t
sp_status_set(_id)
sp_status_set_goto(id)
sp_stime_t
sp_subtract(a, b)
sp_time_interpolate_linear(a, b, t)
sp_time_odd_p(a)
sp_time_random
sp_time_random_bounded(range)
sp_time_random_bounded_primitive
sp_time_random_primitive
sp_time_t
sp_time_to_sample(x)
sp_times_random(size, out)
sp_times_random_bounded(range, size, out)
sp_times_random_bounded_primitive
sp_times_random_primitive
sp_times_zero(a, size)
sp_unit_random
sp_unit_random_primitive
sp_unit_t
sp_units_random(size, out)
sp_units_random_bounded(range, size, out)
sp_units_random_primitive
sp_wave_event(event_pointer, _config)
sp_wave_event_config_new(out)
spline_path_value_t
spline_path_value_t
sprd
srq
~~~

# variables
~~~
sp_channel_count_t sp_channel_count
sp_event_t sp_null_event
sp_random_state_t sp_random_state
sp_sample_t* sp_sine_table
sp_sample_t* sp_sine_table_lfo
sp_time_t sp_rate
sp_time_t sp_sine_lfo_factor
uint32_t sp_cpu_count
~~~

# types
~~~
sp_convolution_filter_ir_f_t: void* sp_sample_t** sp_time_t* -> status_t
sp_event_block_generate_t: sp_time_t sp_time_t sp_time_t sp_block_t sp_event_t* -> status_t
sp_event_generate_t: sp_time_t sp_time_t sp_block_t sp_event_t* -> status_t
sp_event_prepare_t: sp_event_t* -> status_t
sp_map_generate_t: sp_time_t sp_time_t sp_block_t sp_block_t void* -> status_t
sp_memory_free_t: void* -> void
sp_stat_samples_f_t: sp_sample_t* sp_time_t sp_sample_t* -> uint8_t
sp_stat_times_f_t: sp_time_t* sp_time_t sp_sample_t* -> uint8_t
sp_state_variable_filter_t: sp_sample_t* sp_sample_t* sp_sample_t sp_sample_t sp_time_t sp_sample_t* -> void
sp_block_t: struct
  channel_count: sp_channel_count_t
  size: sp_time_t
  samples: array sp_sample_t* sp_channel_count_limit
sp_convolution_filter_state_t: struct
  carryover: sp_sample_t*
  carryover_len: sp_time_t
  carryover_alloc_len: sp_time_t
  ir: sp_sample_t*
  ir_f: sp_convolution_filter_ir_f_t
  ir_f_arguments: void*
  ir_f_arguments_len: uint8_t
  ir_len: sp_time_t
sp_event_list_t: struct sp_event_list_struct
  previous: struct sp_event_list_struct*
  next: struct sp_event_list_struct*
  event: sp_event_t
sp_event_t: struct sp_event_t
  start: sp_time_t
  end: sp_time_t
  generate: function_pointer status_t sp_time_t sp_time_t sp_block_t struct sp_event_t*
  prepare: function_pointer status_t struct sp_event_t*
  free: function_pointer void struct sp_event_t*
  config: void*
  memory: sp_memory_t
sp_events_t: struct
  data: void*
  size: size_t
  used: size_t
  current: size_t
sp_file_t: struct
  file: FILE*
  data_size: sp_size_t
  channel_count: sp_channel_count_t
sp_map_event_config_t: struct
  config: void*
  event: sp_event_t
  map_generate: sp_map_generate_t
  isolate: sp_bool_t
sp_noise_event_channel_config_t: struct
  amp: sp_sample_t
  amod: sp_sample_t*
  channel: sp_channel_count_t
  filter_state: sp_convolution_filter_state_t*
  frq: sp_frq_t
  fmod: sp_time_t*
  wdt: sp_frq_t
  wmod: sp_time_t*
  trnl: sp_frq_t
  trnh: sp_frq_t
  use: sp_bool_t
sp_noise_event_config_t: struct
  is_reject: sp_bool_t
  random_state: sp_random_state_t
  resolution: sp_time_t
  temp: array sp_sample_t* 3
  channel_count: sp_channel_count_t
  channel_config: array sp_noise_event_channel_config_t sp_channel_count_limit
sp_path_segments_t: struct
  data: void*
  size: size_t
  used: size_t
sp_render_config_t: struct
  channel_count: sp_channel_count_t
  rate: sp_time_t
  block_size: sp_time_t
  display_progress: sp_bool_t
sp_samples_t: struct
  data: void*
  size: size_t
  used: size_t
sp_times_t: struct
  data: void*
  size: size_t
  used: size_t
sp_wave_event_channel_config_t: struct
  amod: sp_sample_t*
  amp: sp_sample_t
  channel: sp_channel_count_t
  fmod: sp_time_t*
  frq: sp_frq_t
  phs: sp_time_t
  pmod: sp_time_t*
  use: sp_bool_t
sp_wave_event_config_t: struct
  wvf: sp_sample_t*
  wvf_size: sp_time_t
  channel_count: sp_channel_count_t
  channel_config: array sp_wave_event_channel_config_t sp_channel_count_limit
~~~
