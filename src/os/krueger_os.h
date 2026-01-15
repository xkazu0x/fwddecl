#ifndef KRUEGER_OS_H
#define KRUEGER_OS_H

#if !defined(OS_FEATURE_GFX)
#define OS_FEATURE_GFX 0
#endif
#if !defined(OS_FEATURE_AUDIO)
#define OS_FEATURE_AUDIO 0
#endif

#include "krueger_os_core.h"
#if OS_WINDOWS
#include "krueger_os_core_win32.h"
#elif OS_LINUX
#include "krueger_os_core_linux.h"
#else
#error core not implemented for the current os.
#endif

#if OS_FEATURE_GFX
#include "krueger_os_gfx.h"
#if OS_WINDOWS
#include "krueger_os_gfx_win32.h"
#elif OS_LINUX
#include "krueger_os_gfx_linux.h"
#else
#error gfx not implemented for the current os.
#endif
#endif

#if OS_FEATURE_AUDIO
#include "krueger_os_audio.h"
#if OS_WINDOWS
#include "krueger_os_audio_win32.h"
#elif OS_LINUX
#include "krueger_os_audio_linux.h"
#else
#error audio not implemented for the current os.
#endif
#endif

#endif // KRUEGER_OS_H
