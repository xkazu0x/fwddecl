#ifndef KRUEGER_PLATFORM_AUDIO_LINUX_H
#define KRUEGER_PLATFORM_AUDIO_LINUX_H

/////////////////////////////
// NOTE: Includes / Libraries

#include <pthread.h>
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

//////////////
// NOTE: Types

typedef struct _Linux_Audio_State _Linux_Audio_State;
struct _Linux_Audio_State {
  pthread_t thread;
  snd_pcm_t *device;
  u32 num_frames;
  void *buffer;
};

//////////////////////
// NOTE: Linux Globals

global _Linux_Audio_State _lnx_audio_state;
global Arena *_lnx_audio_arena;

////////////////////////
// NOTE: Linux Functions

internal void *_linux_alsa_thread_proc(void *param);

#endif // KRUEGER_PLATFORM_AUDIO_LINUX_H
