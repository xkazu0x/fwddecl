#ifndef KRUEGER_PLATFORM_AUDIO_LINUX_C
#define KRUEGER_PLATFORM_AUDIO_LINUX_C

////////////////////////
// NOTE: Linux Functions

internal void *
_linux_alsa_thread_proc(void *param) {
  for (;;) {
    int write_res = snd_pcm_writei(_lnx_audio_state.device, _lnx_audio_state.buffer, _lnx_audio_state.num_frames);
    if (write_res < 0) {
      snd_pcm_prepare(_lnx_audio_state.device);
    } else {
      _platform_audio_desc.callback((s16 *)_lnx_audio_state.buffer, _lnx_audio_state.num_frames, _platform_audio_desc.user_data);
    }
  }
  return(0);
}

/////////////////////////////////
// NOTE: Implemented Per-Platform

internal void
platform_audio_init(void) {
  u32 rate = _platform_audio_desc.sample_rate;
  int dir = 0;

  snd_pcm_open(&_lnx_audio_state.device, "default", SND_PCM_STREAM_PLAYBACK, 0);

  snd_pcm_hw_params_t *hw_params;
  snd_pcm_hw_params_alloca(&hw_params);
  snd_pcm_hw_params_any(_lnx_audio_state.device, hw_params);
  snd_pcm_hw_params_set_access(_lnx_audio_state.device, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(_lnx_audio_state.device, hw_params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(_lnx_audio_state.device, hw_params, _platform_audio_desc.num_channels);
  snd_pcm_hw_params_set_rate_near(_lnx_audio_state.device, hw_params, &rate, &dir);
  snd_pcm_hw_params(_lnx_audio_state.device, hw_params);

  _platform_audio_desc.sample_rate = rate;
  _lnx_audio_arena = arena_alloc();
  _lnx_audio_state.num_frames = 2048;
  u32 bytes_per_frame = _platform_audio_desc.num_channels*sizeof(s16);
  u32 buffer_byte_size = _lnx_audio_state.num_frames*bytes_per_frame;
  _lnx_audio_state.buffer = push_array(_lnx_audio_arena, u8, buffer_byte_size);

  pthread_t thread;
  pthread_create(&thread, 0, _linux_alsa_thread_proc, 0);
}

internal void
platform_audio_shutdown(void) {
  pthread_join(_lnx_audio_state.thread, 0);
  snd_pcm_drain(_lnx_audio_state.device);
  snd_pcm_close(_lnx_audio_state.device);
  arena_release(_lnx_audio_arena);
}

#endif // KRUEGER_PLATFORM_AUDIO_LINUX_C
