#ifndef KRUEGER_OS_AUDIO_C
#define KRUEGER_OS_AUDIO_C

////////////////////////////////////
// NOTE: (Helpers, Implemented Once)

internal
OS_AUDIO_CALLBACK(_os_audio_cb_stub) {
}

internal void
os_audio_setup(Os_Audio_Desc *desc) {
  if (desc) {
    _os_audio_desc.sample_rate = null_def(desc->sample_rate, _os_audio_default_sample_rate);
    _os_audio_desc.num_channels = null_def(desc->num_channels, _os_audio_default_num_channels);
    _os_audio_desc.callback = null_def(desc->callback, _os_audio_cb_stub);
    _os_audio_desc.user_data = desc->user_data;
  }
}

internal u32
os_audio_get_sample_rate(void) {
  return(_os_audio_desc.sample_rate);
}

internal u32
os_audio_get_num_channels(void) {
  return(_os_audio_desc.num_channels);
}

#endif // KRUEGER_OS_AUDIO_C
