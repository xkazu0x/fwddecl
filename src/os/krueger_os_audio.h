#ifndef KRUEGER_OS_AUDIO_H
#define KRUEGER_OS_AUDIO_H

//////////////
// NOTE: Types

#define OS_AUDIO_CALLBACK(x) void x(s16 *buffer, u32 num_frames, void *user_data)
typedef OS_AUDIO_CALLBACK(Os_Audio_Callback);

typedef struct Os_Audio_Desc Os_Audio_Desc;
struct Os_Audio_Desc {
  u32 sample_rate;
  u32 num_channels;
  Os_Audio_Callback *callback;
  void *user_data;
};

////////////////
// NOTE: Globals

global const u32 _os_audio_default_sample_rate = 44100;
global const u16 _os_audio_default_num_channels = 1;
global Os_Audio_Desc _os_audio_desc;

////////////////////////////////////
// NOTE: (Helpers, Implemented Once)

internal OS_AUDIO_CALLBACK(_os_audio_cb_stub);

internal void os_audio_setup(Os_Audio_Desc *desc);
internal u32 os_audio_get_sample_rate(void);
internal u32 os_audio_get_num_channels(void);

//////////////////////////////////////////////////////////
// NOTE: Main Initialization/Shutdown (Implemented Per-OS)

internal void os_audio_init(void);
internal void os_audio_shutdown(void);

#endif // KRUEGER_OS_AUDIO_H
