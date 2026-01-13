#ifndef KRUEGER_PLATFORM_AUDIO_H
#define KRUEGER_PLATFORM_AUDIO_H

//////////////
// NOTE: Types

#define PLATFORM_AUDIO_CALLBACK(x) void x(s16 *buffer, u32 num_frames, void *user_data)
typedef PLATFORM_AUDIO_CALLBACK(Platform_Audio_Callback);

typedef struct Platform_Audio_Desc Platform_Audio_Desc;
struct Platform_Audio_Desc {
  u32 sample_rate;
  u32 num_channels;
  Platform_Audio_Callback *callback;
  void *user_data;
};

////////////////
// NOTE: Globals

global const u32 _audio_default_sample_rate = 44100;
global const u16 _audio_default_num_channels = 1;
global Platform_Audio_Desc _platform_audio_desc;

//////////////////////////////////
// NOTE: Helpers, Implemented Once

internal void platform_audio_setup(Platform_Audio_Desc *desc);
internal u32 platform_audio_get_sample_rate(void);
internal u32 platform_audio_get_num_channels(void);

/////////////////////////////////
// NOTE: Implemented Per-Platform

internal void platform_audio_init(void);
internal void platform_audio_shutdown(void);

#endif // KRUEGER_PLATFORM_AUDIO_H
