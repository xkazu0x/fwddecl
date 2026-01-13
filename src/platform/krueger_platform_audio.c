#ifndef KRUEGER_PLATFORM_AUDIO_C
#define KRUEGER_PLATFORM_AUDIO_C

//////////////////////////////////
// NOTE: Helpers, Implemented Once

internal
PLATFORM_AUDIO_CALLBACK(_platform_audio_cb_stub) {
}

internal void
platform_audio_setup(Platform_Audio_Desc *desc) {
  _platform_audio_desc.sample_rate = _audio_default_sample_rate;
  _platform_audio_desc.num_channels = _audio_default_num_channels;
  _platform_audio_desc.callback = _platform_audio_cb_stub;
  if (desc) {
    if (desc->sample_rate) _platform_audio_desc.sample_rate = desc->sample_rate;
    if (desc->num_channels) _platform_audio_desc.num_channels = desc->num_channels;
    if (desc->callback) _platform_audio_desc.callback = desc->callback;
    if (desc->user_data) _platform_audio_desc.user_data = desc->user_data;
  }
}

internal u32
platform_audio_get_sample_rate(void) {
  return(_platform_audio_desc.sample_rate);
}

internal u32
platform_audio_get_num_channels(void) {
  return(_platform_audio_desc.num_channels);
}

#endif // KRUEGER_PLATFORM_AUDIO_C
