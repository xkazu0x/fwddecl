#ifndef KRUEGER_OS_C
#define KRUEGER_OS_C

#include "krueger_os_core.c"
#if OS_WINDOWS
#include "krueger_os_core_win32.c"
#elif OS_LINUX
#include "krueger_os_core_linux.c"
#else
#error core not implemented for the current os.
#endif

#if OS_FEATURE_GFX
#include "krueger_os_gfx.c"
#if OS_WINDOWS
#include "krueger_os_gfx_win32.c"
#elif OS_LINUX
#include "krueger_os_gfx_linux.c"
#else
#error gfx not implemented for the current os.
#endif
#endif

#if OS_FEATURE_AUDIO
#include "krueger_os_audio.c"
#if OS_WINDOWS
#include "krueger_os_audio_win32.c"
#elif OS_LINUX
#include "krueger_os_audio_linux.c"
#else
#error audio not implemented for the current os.
#endif
#endif

#endif // KRUEGER_OS_C
