# functions
~~~
sp_block_to_file :: sp_block_t:block uint8_t*:path sp_time_t:rate -> status_t
sp_block_copy :: sp_block_t:a sp_block_t:b -> void
sp_block_free :: sp_block_t*:a -> void
sp_block_new :: sp_channel_count_t:channel_count sp_time_t:sample_count sp_block_t*:out_block -> status_t
sp_block_with_offset :: sp_block_t:a sp_time_t:offset -> sp_block_t
sp_block_zero :: sp_block_t:a -> void
sp_channel_config :: sp_bool_t:mute sp_time_t:delay sp_time_t:phs sp_sample_t:amp sp_sample_t*:amod -> sp_channel_config_t
sp_cheap_filter :: sp_state_variable_filter_t:type sp_sample_t*:in sp_time_t:in_size sp_sample_t:cutoff sp_time_t:passes sp_sample_t:q_factor sp_cheap_filter_state_t*:state sp_sample_t*:out -> void
sp_cheap_filter_state_free :: sp_cheap_filter_state_t*:a -> void
sp_cheap_filter_state_new :: sp_time_t:max_size sp_bool_t:is_multipass sp_cheap_filter_state_t*:out_state -> status_t
sp_cheap_noise_event_config_new :: sp_cheap_noise_event_config_t**:out -> status_t
sp_cheap_noise_event_free :: -> void
sp_cheap_noise_event_prepare :: sp_event_t*:event -> status_t
sp_compositions_max :: sp_time_t:sum -> sp_time_t
sp_convolution_filter :: sp_sample_t*:in sp_time_t:in_len sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_convolution_filter_state_free :: sp_convolution_filter_state_t*:state -> void
sp_convolution_filter_state_set :: sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state -> status_t
sp_convolve :: sp_sample_t*:a sp_time_t:a_len sp_sample_t*:b sp_time_t:b_len sp_time_t:result_carryover_len sp_sample_t*:result_carryover sp_sample_t*:result_samples -> void
sp_convolve_one :: sp_sample_t*:a sp_time_t:a_len sp_sample_t*:b sp_time_t:b_len sp_sample_t*:result_samples -> void
sp_event_list_add :: sp_event_list_t**:a sp_event_t:event -> status_t
sp_event_list_display :: sp_event_list_t*:a -> void
sp_event_list_free :: sp_event_list_t**:events -> void
sp_event_list_remove_element :: sp_event_list_t**:a sp_event_list_t*:element -> void
sp_event_list_reverse :: sp_event_list_t**:a -> void
sp_event_list_validate :: sp_event_list_t*:a -> void
sp_event_memory_add :: sp_event_t*:event void*:address sp_memory_free_t:handler -> void
sp_event_memory_free :: sp_event_t*:event -> void
sp_event_memory_init :: sp_event_t*:a sp_time_t:additional_size -> status_t
sp_fft :: sp_time_t:input_len double*:input_or_output_real double*:input_or_output_imag -> int
sp_ffti :: sp_time_t:input_len double*:input_or_output_real double*:input_or_output_imag -> int
sp_file_close :: sp_file_t:a -> status_t
sp_file_open :: uint8_t*:path int:mode sp_channel_count_t:channel_count sp_time_t:sample_rate sp_file_t*:result_file -> status_t
sp_file_position :: sp_file_t*:file sp_time_t*:result_position -> status_t
sp_file_position_set :: sp_file_t*:file sp_time_t:sample_offset -> status_t
sp_file_read :: sp_file_t*:file sp_time_t:sample_count sp_sample_t**:result_block sp_time_t*:result_sample_count -> status_t
sp_file_write :: sp_file_t*:file sp_sample_t**:block sp_time_t:sample_count sp_time_t*:result_sample_count -> status_t
sp_filter :: sp_sample_t*:in sp_time_t:in_size sp_sample_t:cutoff_l sp_sample_t:cutoff_h sp_sample_t:transition_l sp_sample_t:transition_h sp_bool_t:is_reject sp_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_group_add :: sp_event_t*:a sp_event_t:event -> status_t
sp_group_add_set :: sp_event_t*:group sp_time_t:start sp_time_t:duration sp_sample_t:volume void*:config sp_event_t:event -> status_t
sp_group_append :: sp_event_t*:a sp_event_t:event -> status_t
sp_group_append_set :: sp_event_t*:group sp_sample_t:volume void*:config sp_event_t:event -> status_t
sp_group_event_f :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> void
sp_group_event_free :: sp_event_t*:a -> void
sp_group_event_parallel_f :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:event -> void
sp_group_free :: -> void
sp_group_prepare :: sp_event_t*:event -> status_t
sp_group_prepare_parallel :: sp_event_t*:a -> status_t
sp_initialize :: uint16_t:cpu_count sp_channel_count_t:channels sp_time_t:rate -> status_t
sp_map_event_config_new :: sp_map_event_config_t**:out -> status_t
sp_map_event_free :: -> void
sp_map_event_prepare :: sp_event_t*:event -> status_t
sp_moving_average :: sp_sample_t*:in sp_time_t:in_size sp_sample_t*:prev sp_sample_t*:next sp_time_t:radius sp_sample_t*:out -> void
sp_noise_event_config_new :: sp_noise_event_config_t**:out -> status_t
sp_noise_event_free :: -> void
sp_noise_event_prepare :: sp_event_t*:event -> status_t
sp_null_ir :: sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_passthrough_ir :: sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_path_derivation :: sp_path_t:path sp_sample_t**:x_changes sp_sample_t**:y_changes sp_time_t:index sp_path_t*:out -> status_t
sp_path_derivations_normalized :: sp_path_t:base sp_time_t:count sp_sample_t**:x_changes sp_sample_t**:y_changes sp_path_t**:out -> status_t
sp_path_multiply :: sp_path_t:path sp_sample_t:x_factor sp_sample_t:y_factor -> void
sp_path_samples_1 :: sp_sample_t**:out sp_time_t:size sp_path_segment_t:s1 -> status_t
sp_path_samples_2 :: sp_sample_t**:out sp_time_t:size sp_path_segment_t:s1 sp_path_segment_t:s2 -> status_t
sp_path_samples_3 :: sp_sample_t**:out sp_time_t:size sp_path_segment_t:s1 sp_path_segment_t:s2 sp_path_segment_t:s3 -> status_t
sp_path_samples_4 :: sp_sample_t**:out sp_time_t:size sp_path_segment_t:s1 sp_path_segment_t:s2 sp_path_segment_t:s3 sp_path_segment_t:s4 -> status_t
sp_path_samples_derivation :: sp_path_t:path sp_sample_t**:x_changes sp_sample_t**:y_changes sp_time_t:index sp_sample_t**:out sp_path_time_t*:out_size -> status_t
sp_path_samples_derivations_normalized :: sp_path_t:path sp_time_t:count sp_sample_t**:x_changes sp_sample_t**:y_changes sp_sample_t***:out sp_time_t**:out_sizes -> status_t
sp_path_samples_new :: sp_path_t:path sp_path_time_t:size sp_sample_t**:out -> status_t
sp_path_times_1 :: sp_time_t**:out sp_time_t:size sp_path_segment_t:s1 -> status_t
sp_path_times_2 :: sp_time_t**:out sp_time_t:size sp_path_segment_t:s1 sp_path_segment_t:s2 -> status_t
sp_path_times_3 :: sp_time_t**:out sp_time_t:size sp_path_segment_t:s1 sp_path_segment_t:s2 sp_path_segment_t:s3 -> status_t
sp_path_times_4 :: sp_time_t**:out sp_time_t:size sp_path_segment_t:s1 sp_path_segment_t:s2 sp_path_segment_t:s3 sp_path_segment_t:s4 -> status_t
sp_path_times_derivation :: sp_path_t:path sp_sample_t**:x_changes sp_sample_t**:y_changes sp_time_t:index sp_time_t**:out sp_path_time_t*:out_size -> status_t
sp_path_times_new :: sp_path_t:path sp_path_time_t:size sp_time_t**:out -> status_t
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
sp_render_block :: sp_event_t:event sp_time_t:start sp_time_t:end sp_render_config_t:config sp_block_t*:out -> status_t
sp_render_config :: sp_channel_count_t:channels sp_time_t:rate sp_time_t:block_size -> sp_render_config_t
sp_render_file :: sp_event_t:event sp_time_t:start sp_time_t:end sp_render_config_t:config uint8_t*:path -> status_t
sp_render_quick :: sp_event_t:event uint8_t:file_or_plot -> status_t
sp_sample_random_custom :: sp_random_state_t*:state sp_time_t*:cudist sp_time_t:cudist_size sp_sample_t:range -> sp_sample_t
sp_samples_to_times :: sp_sample_t*:in sp_time_t:in_size sp_time_t*:out -> void
sp_samples_absolute_max :: sp_sample_t*:in sp_time_t:in_size -> sp_sample_t
sp_samples_add :: sp_sample_t*:a sp_time_t:size sp_sample_t*:b sp_sample_t*:out -> void
sp_samples_add_1 :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_additions :: sp_sample_t:start sp_sample_t:summand sp_time_t:count sp_sample_t*:out -> void
sp_samples_and :: sp_sample_t*:a sp_sample_t*:b sp_time_t:size sp_sample_t:limit sp_sample_t*:out -> void
sp_samples_array_free :: sp_sample_t**:a sp_time_t:size -> void
sp_samples_blend :: sp_sample_t*:a sp_sample_t*:b sp_sample_t:fraction sp_time_t:size sp_sample_t*:out -> void
sp_samples_copy :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> void
sp_samples_differences :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> void
sp_samples_display :: sp_sample_t*:a sp_time_t:size -> void
sp_samples_divide :: sp_sample_t*:a sp_time_t:size sp_sample_t*:b sp_sample_t*:out -> void
sp_samples_divide_1 :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_divisions :: sp_sample_t:start sp_sample_t:n sp_time_t:count sp_sample_t*:out -> void
sp_samples_duplicate :: sp_sample_t*:a sp_time_t:size sp_sample_t**:out -> status_t
sp_samples_equal_1 :: sp_sample_t*:a sp_time_t:size sp_sample_t:n -> uint8_t
sp_samples_every_equal :: sp_sample_t*:a sp_time_t:size sp_sample_t:n -> uint8_t
sp_samples_limit_abs :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_multiply :: sp_sample_t*:a sp_time_t:size sp_sample_t*:b sp_sample_t*:out -> void
sp_samples_multiply_1 :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_new :: sp_time_t:size sp_sample_t**:out -> status_t
sp_samples_or :: sp_sample_t*:a sp_sample_t*:b sp_time_t:size sp_sample_t:limit sp_sample_t*:out -> void
sp_samples_reverse :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> void
sp_samples_scale_to_times :: sp_sample_t*:a sp_time_t:size sp_time_t:max sp_time_t*:out -> void
sp_samples_scale_sum :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_scale_y :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_set_1 :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_set_unity_gain :: sp_sample_t*:in sp_time_t:in_size sp_sample_t*:out -> void
sp_samples_smooth :: sp_sample_t*:a sp_time_t:size sp_time_t:radius sp_sample_t*:out -> status_t
sp_samples_sort_less :: void*:a ssize_t:b ssize_t:c -> uint8_t
sp_samples_sort_swap :: void*:a ssize_t:b ssize_t:c -> void
sp_samples_square :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> void
sp_samples_subtract :: sp_sample_t*:a sp_time_t:size sp_sample_t*:b sp_sample_t*:out -> void
sp_samples_subtract_1 :: sp_sample_t*:a sp_time_t:size sp_sample_t:n sp_sample_t*:out -> void
sp_samples_xor :: sp_sample_t*:a sp_sample_t*:b sp_time_t:size sp_sample_t:limit sp_sample_t*:out -> void
sp_seq :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_list_t**:events -> status_t
sp_seq_parallel :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_list_t**:events -> status_t
sp_sequence_max :: sp_time_t:size sp_time_t:min_size -> sp_time_t
sp_sequence_set_hash :: sp_sequence_set_key_t:a sp_time_t:memory_size -> uint64_t
sp_set_sequence_max :: sp_time_t:set_size sp_time_t:selection_size -> sp_time_t
sp_shuffle :: sp_random_state_t*:state function_pointer void void* size_t size_t:swap void*:a size_t:size -> void
sp_sin_lq :: sp_sample_t:a -> sp_sample_t
sp_sinc :: sp_sample_t:a -> sp_sample_t
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
sp_stat_samples_repetition_all :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_samples_skewness :: sp_sample_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_center :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_deviation :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_inharmonicity :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_kurtosis :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_mean :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_median :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_range :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_repetition :: sp_time_t*:a sp_time_t:size sp_time_t:width sp_sample_t*:out -> uint8_t
sp_stat_times_repetition :: sp_time_t*:a sp_time_t:size sp_time_t:width sp_sample_t*:out -> uint8_t
sp_stat_times_repetition_all :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_times_skewness :: sp_time_t*:a sp_time_t:size sp_sample_t*:out -> uint8_t
sp_stat_unique_all_max :: sp_time_t:size -> sp_time_t
sp_stat_unique_max :: sp_time_t:size sp_time_t:width -> sp_time_t
sp_state_variable_filter_all :: sp_sample_t*:out sp_sample_t*:in sp_sample_t:in_count sp_sample_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_bp :: sp_sample_t*:out sp_sample_t*:in sp_sample_t:in_count sp_sample_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_br :: sp_sample_t*:out sp_sample_t*:in sp_sample_t:in_count sp_sample_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_hp :: sp_sample_t*:out sp_sample_t*:in sp_sample_t:in_count sp_sample_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_lp :: sp_sample_t*:out sp_sample_t*:in sp_sample_t:in_count sp_sample_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_peak :: sp_sample_t*:out sp_sample_t*:in sp_sample_t:in_count sp_sample_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_status_description :: status_t:a -> uint8_t*
sp_status_name :: status_t:a -> uint8_t*
sp_time_expt :: sp_time_t:base sp_time_t:exp -> sp_time_t
sp_time_factorial :: sp_time_t:a -> sp_time_t
sp_time_random_custom :: sp_random_state_t*:state sp_time_t*:cudist sp_time_t:cudist_size sp_time_t:range -> sp_time_t
sp_time_random_discrete :: sp_random_state_t*:state sp_time_t*:cudist sp_time_t:cudist_size -> sp_time_t
sp_time_round_to_multiple :: sp_time_t:a sp_time_t:base -> sp_time_t
sp_times_add :: sp_time_t*:a sp_time_t:size sp_time_t*:b sp_time_t*:out -> void
sp_times_add_1 :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out -> void
sp_times_additions :: sp_time_t:start sp_time_t:summand sp_time_t:count sp_time_t*:out -> void
sp_times_and :: sp_time_t*:a sp_time_t*:b sp_time_t:size sp_time_t:limit sp_time_t*:out -> void
sp_times_array_free :: sp_time_t**:a sp_time_t:size -> void
sp_times_bits_to_times :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_blend :: sp_time_t*:a sp_time_t*:b sp_sample_t:fraction sp_time_t:size sp_time_t*:out -> void
sp_times_compositions :: sp_time_t:sum sp_time_t***:out sp_time_t*:out_size sp_time_t**:out_sizes -> status_t
sp_times_constant :: sp_time_t:a sp_time_t:size sp_time_t:value sp_time_t**:out -> status_t
sp_times_contains :: sp_time_t*:a sp_time_t:size sp_time_t:b -> uint8_t
sp_times_copy :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_counted_sequences :: sp_sequence_hashtable_t:known sp_time_t:limit sp_times_counted_sequences_t*:out sp_time_t*:out_size -> void
sp_times_counted_sequences_hash :: sp_time_t*:a sp_time_t:size sp_time_t:width sp_sequence_hashtable_t:out -> void
sp_times_counted_sequences_sort_greater :: void*:a ssize_t:b ssize_t:c -> uint8_t
sp_times_counted_sequences_sort_less :: void*:a ssize_t:b ssize_t:c -> uint8_t
sp_times_counted_sequences_sort_swap :: void*:a ssize_t:b ssize_t:c -> void
sp_times_cusum :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_deduplicate :: sp_time_t*:a sp_time_t:size sp_time_t*:out sp_time_t*:out_size -> status_t
sp_times_differences :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_display :: sp_time_t*:a sp_time_t:size -> void
sp_times_divide :: sp_time_t*:a sp_time_t:size sp_time_t*:b sp_time_t*:out -> void
sp_times_divide_1 :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out -> void
sp_times_duplicate :: sp_time_t:a sp_time_t:size sp_time_t**:out -> status_t
sp_times_equal_1 :: sp_time_t*:a sp_time_t:size sp_time_t:n -> uint8_t
sp_times_gt_indices :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out sp_time_t*:out_size -> void
sp_times_insert_space :: sp_time_t*:in sp_time_t:size sp_time_t:index sp_time_t:count sp_time_t*:out -> void
sp_times_limit :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out -> void
sp_times_mask :: sp_time_t*:a sp_time_t*:b sp_sample_t*:coefficients sp_time_t:size sp_time_t*:out -> void
sp_times_multiplications :: sp_time_t:start sp_time_t:factor sp_time_t:count sp_time_t*:out -> void
sp_times_multiply :: sp_time_t*:a sp_time_t:size sp_time_t*:b sp_time_t*:out -> void
sp_times_multiply_1 :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out -> void
sp_times_new :: sp_time_t:size sp_time_t**:out -> status_t
sp_times_or :: sp_time_t*:a sp_time_t*:b sp_time_t:size sp_time_t:limit sp_time_t*:out -> void
sp_times_permutations :: sp_time_t:size sp_time_t*:set sp_time_t:set_size sp_time_t***:out sp_time_t*:out_size -> status_t
sp_times_random_binary :: sp_random_state_t*:state sp_time_t:size sp_time_t*:out -> status_t
sp_times_random_discrete :: sp_random_state_t*:state sp_time_t*:cudist sp_time_t:cudist_size sp_time_t:count sp_time_t*:out -> void
sp_times_random_discrete_unique :: sp_random_state_t*:state sp_time_t*:cudist sp_time_t:cudist_size sp_time_t:size sp_time_t*:out -> void
sp_times_range :: sp_time_t:start sp_time_t:end sp_time_t*:out -> void
sp_times_remove :: sp_time_t*:in sp_time_t:size sp_time_t:index sp_time_t:count sp_time_t*:out -> void
sp_times_reverse :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_scale :: sp_time_t*:a sp_time_t:a_size sp_time_t:factor sp_time_t*:out -> status_t
sp_times_select :: sp_time_t*:a sp_time_t*:indices sp_time_t:size sp_time_t*:out -> void
sp_times_select_random :: sp_random_state_t*:state sp_time_t*:a sp_time_t:size sp_time_t*:out sp_time_t*:out_size -> void
sp_times_sequence_increment :: sp_time_t*:a sp_time_t:size sp_time_t:set_size -> void
sp_times_sequences :: sp_time_t:base sp_time_t:digits sp_time_t:size sp_time_t*:out -> void
sp_times_set_1 :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out -> void
sp_times_shuffle :: sp_random_state_t*:state sp_time_t*:a sp_time_t:size -> void
sp_times_shuffle_swap :: void*:a size_t:i1 size_t:i2 -> void
sp_times_sort_less :: void*:a ssize_t:b ssize_t:c -> uint8_t
sp_times_sort_swap :: void*:a ssize_t:b ssize_t:c -> void
sp_times_square :: sp_time_t*:a sp_time_t:size sp_time_t*:out -> void
sp_times_subdivide :: sp_time_t*:a sp_time_t:size sp_time_t:index sp_time_t:count sp_time_t*:out -> void
sp_times_subtract :: sp_time_t*:a sp_time_t:size sp_time_t*:b sp_time_t*:out -> void
sp_times_subtract_1 :: sp_time_t*:a sp_time_t:size sp_time_t:n sp_time_t*:out -> void
sp_times_swap :: sp_time_t*:a ssize_t:i1 ssize_t:i2 -> void
sp_times_xor :: sp_time_t*:a sp_time_t*:b sp_time_t:size sp_time_t:limit sp_time_t*:out -> void
sp_triangle :: sp_time_t:t sp_time_t:a sp_time_t:b -> sp_sample_t
sp_u64_from_array :: uint8_t*:a sp_time_t:size -> uint64_t
sp_wave_event_config_new :: sp_wave_event_config_t**:out -> status_t
sp_wave_event_free :: -> void
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
~~~

# macros
~~~
f64
free_event_on_error(event_address)
free_event_on_exit(event_address)
free_on_error(address, handler)
free_on_error1(address)
free_on_error_free
free_on_error_init(register_size)
free_on_exit(address, handler)
free_on_exit1(address)
free_on_exit_free
free_on_exit_init(register_size)
rt(n, d)
rts(n, d)
sp_abs(a)
sp_absolute_difference(a, b)
sp_block_declare(a)
sp_block_plot_1(a)
sp_bool_t
sp_calloc_type(count, type, pointer_address)
sp_channel_count_t
sp_channel_limit
sp_cheap_ceiling_positive(a)
sp_cheap_filter_bp(...)
sp_cheap_filter_br(...)
sp_cheap_filter_hp(...)
sp_cheap_filter_lp(...)
sp_cheap_filter_passes_limit
sp_cheap_floor_positive(a)
sp_cheap_round_positive(a)
sp_declare_cheap_filter_state(name)
sp_declare_cheap_noise_config(name)
sp_declare_event(id)
sp_declare_event_2(id1, id2)
sp_declare_event_3(id1, id2, id3)
sp_declare_event_4(id1, id2, id3, id4)
sp_declare_event_list(id)
sp_declare_group(id)
sp_declare_noise_config(name)
sp_declare_sine_config(name)
sp_declare_sine_config_lfo(name)
sp_default_random_seed
sp_define_event(name, _prepare, duration)
sp_event_duration(a)
sp_event_duration_set(a, duration)
sp_event_free(a)
sp_event_memory_add1(event, address)
sp_event_memory_add1_2(a, data1, data2)
sp_event_memory_add1_3(a, data1, data2, data3)
sp_event_memory_malloc(event, count, type, pointer_address)
sp_event_move(a, start)
sp_event_pointer_free(a)
sp_file_bit_input
sp_file_bit_output
sp_file_bit_position
sp_file_declare(a)
sp_file_format
sp_file_mode_read
sp_file_mode_read_write
sp_file_mode_write
sp_filter_state_free
sp_filter_state_t
sp_group_event_list(event)
sp_group_size_t
sp_malloc_type(count, type, pointer_address)
sp_max(a, b)
sp_memory_add
sp_min(a, b)
sp_no_underflow_subtract(a, b)
sp_no_zero_divide(a, b)
sp_noise
sp_path_bezier
sp_path_constant
sp_path_end
sp_path_free
sp_path_get
sp_path_i_bezier
sp_path_i_constant
sp_path_i_line
sp_path_i_move
sp_path_i_path
sp_path_line
sp_path_move
sp_path_path
sp_path_point_t
sp_path_prepare_segments
sp_path_samples_constant(out, size, value)
sp_path_segment_count_t
sp_path_segment_t
sp_path_set
sp_path_size
sp_path_t
sp_path_time_t
sp_path_times_constant(out, size, value)
sp_path_value_t
sp_random_state_t
sp_realloc_type(count, type, pointer_address)
sp_s_group_libc
sp_s_group_sndfile
sp_s_group_sp
sp_s_group_sph
sp_s_id_eof
sp_s_id_file_channel_mismatch
sp_s_id_file_closed
sp_s_id_file_encoding
sp_s_id_file_header
sp_s_id_file_incompatible
sp_s_id_file_incomplete
sp_s_id_file_position
sp_s_id_file_type
sp_s_id_input_type
sp_s_id_invalid_argument
sp_s_id_memory
sp_s_id_not_implemented
sp_s_id_undefined
sp_sample_array_nearly_equal
sp_sample_interpolate_linear(a, b, t)
sp_sample_nearly_equal
sp_sample_random
sp_sample_t
sp_sample_t
sp_samples_random
sp_samples_random_bounded
sp_samples_sum
sp_samples_zero(a, size)
sp_seq_events_prepare
sp_sequence_set_equal(a, b)
sp_sf_read
sp_sf_write
sp_sine_config_t
sp_status_set(id)
sp_time_half_t
sp_time_interpolate_linear(a, b, t)
sp_time_random
sp_time_random_bounded
sp_time_t
sp_times_random
sp_times_random_bounded
sp_times_zero(a, size)
spline_path_time_t
spline_path_value_t
srq
~~~

# variables
~~~
sp_channel_count_t sp_channels
sp_random_state_t sp_default_random_state
sp_sample_t* sp_sine_table
sp_sample_t* sp_sine_table_lfo
sp_sequence_set_key_t sp_sequence_set_null
sp_time_t sp_rate
sp_time_t sp_sine_lfo_factor
status_t(*)(sp_event_t*) sp_event_prepare_t
uint32_t sp_cpu_count
~~~

# types
~~~
sp_convolution_filter_ir_f_t: void* sp_sample_t** sp_time_t* -> status_t
sp_event_generate_t: sp_time_t sp_time_t sp_block_t sp_event_t* -> status_t
sp_map_generate_t: sp_time_t sp_time_t sp_block_t sp_block_t void* -> status_t
sp_memory_free_t: void* -> void
sp_stat_samples_f_t: sp_sample_t* sp_time_t sp_sample_t* -> uint8_t
sp_stat_times_f_t: sp_time_t* sp_time_t sp_sample_t* -> uint8_t
sp_state_variable_filter_t: sp_sample_t* sp_sample_t* sp_sample_t sp_sample_t sp_time_t sp_sample_t* -> void
sp_block_t: struct
  channels: sp_channel_count_t
  size: sp_time_t
  samples: array sp_sample_t* sp_channel_limit
sp_channel_config_t: struct
  use: sp_bool_t
  mute: sp_bool_t
  delay: sp_time_t
  phs: sp_time_t
  amp: sp_sample_t
  amod: sp_sample_t*
sp_cheap_filter_state_t: struct
  in_temp: sp_sample_t*
  out_temp: sp_sample_t*
  svf_state: array sp_sample_t * 2 sp_cheap_filter_passes_limit
sp_cheap_noise_event_config_t: struct
  amp: sp_sample_t
  amod: sp_sample_t*
  cut: sp_sample_t
  cut_mod: sp_sample_t*
  q_factor: sp_sample_t
  passes: sp_time_t
  type: sp_state_variable_filter_t
  random_state: sp_random_state_t*
  resolution: sp_time_t
  channels: sp_channel_count_t
  channel_config: array sp_channel_config_t sp_channel_limit
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
  data: void*
  memory: sp_memory_t
  volume: sp_sample_t
sp_events_t: struct
  data: void*
  size: size_t
  used: size_t
  current: size_t
sp_file_t: struct
  flags: uint8_t
  sample_rate: sp_time_t
  channel_count: sp_channel_count_t
  data: void*
sp_map_event_config_t: struct
  event: sp_event_t
  map_generate: sp_map_generate_t
  state: void*
  isolate: sp_bool_t
sp_map_event_state_t: struct
  event: sp_event_t
  map_generate: sp_map_generate_t
  state: void*
sp_noise_event_config_t: struct
  amp: sp_sample_t
  amod: sp_sample_t*
  cutl: sp_sample_t
  cuth: sp_sample_t
  trnl: sp_sample_t
  trnh: sp_sample_t
  cutl_mod: sp_sample_t*
  cuth_mod: sp_sample_t*
  resolution: sp_time_t
  is_reject: uint8_t
  channels: sp_channel_count_t
  channel_config: array sp_channel_config_t sp_channel_limit
sp_path_segments_t: struct
  data: void*
  size: size_t
  used: size_t
sp_render_config_t: struct
  channels: sp_channel_count_t
  rate: sp_time_t
  block_size: sp_time_t
sp_samples_t: struct
  data: void*
  size: size_t
  used: size_t
sp_sequence_set_key_t: struct
  size: sp_time_t
  data: uint8_t*
sp_times_counted_sequences_t: struct
  count: sp_time_t
  sequence: sp_time_t*
sp_times_t: struct
  data: void*
  size: size_t
  used: size_t
sp_wave_event_config_t: struct
  wvf: sp_sample_t*
  wvf_size: sp_time_t
  phs: sp_time_t
  frq: sp_time_t
  fmod: sp_time_t*
  amp: sp_sample_t
  amod: sp_sample_t*
  channels: sp_channel_count_t
  channel_config: array sp_channel_config_t sp_channel_limit
sp_wave_event_state_t: struct
  wvf: sp_sample_t*
  wvf_size: sp_time_t
  phs: sp_time_t
  frq: sp_time_t
  fmod: sp_time_t*
  amp: sp_sample_t
  amod: sp_sample_t*
  channel: sp_channel_count_t
~~~
