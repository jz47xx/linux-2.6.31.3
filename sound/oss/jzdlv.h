/* header file for dlv */
void write_codec_file(int addr, int val);
int read_codec_file(int addr);
void printk_codec_files(void);
int write_codec_file_bit(int addr, int bitval, int mask_bit);
void set_audio_data_replay(void);
void unset_audio_data_replay(void);
void set_record_mic_input_audio_without_playback(void);
void unset_record_mic_input_audio_without_playback(void);
void set_record_line_input_audio_without_playback(void);
void unset_record_line_input_audio_without_playback(void);
void set_playback_line_input_audio_direct_only(void);
void unset_playback_line_input_audio_direct_only(void);
void set_record_mic_input_audio_with_direct_playback(void);
void unset_record_mic_input_audio_with_direct_playback(void);
void set_record_playing_audio_mixed_with_mic_input_audio(void);
void unset_record_playing_audio_mixed_with_mic_input_audio(void);
void set_record_mic_input_audio_with_audio_data_replay(void);
void unset_record_mic_input_audio_with_audio_data_replay(void);
void set_record_line_input_audio_with_audio_data_replay(void);
void unset_record_line_input_audio_with_audio_data_replay(void);
