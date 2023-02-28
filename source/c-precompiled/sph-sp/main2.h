typedef struct {
  sp_channel_count_t channel_count;
  sp_time_t rate;
  sp_time_t block_size;
} sp_render_config_t;
sp_render_config_t sp_render_config(sp_channel_count_t channel_count, sp_time_t rate, sp_time_t block_size);
status_t sp_render_file(sp_event_t event, sp_time_t start, sp_time_t end, sp_render_config_t config, uint8_t* path);
status_t sp_render_block(sp_event_t event, sp_time_t start, sp_time_t end, sp_render_config_t config, sp_block_t* out);
status_t sp_render(sp_event_t event, uint8_t file_or_plot);