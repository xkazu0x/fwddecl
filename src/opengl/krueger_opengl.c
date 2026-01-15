#ifndef KRUEGER_OPENGL_C
#define KRUEGER_OPENGL_C

#if OS_WINDOWS
#include "krueger_opengl_win32.c"
#elif OS_LINUX
#include "krueger_opengl_linux.c"
#else
#error opengl layer not implemented for the current os.
#endif

#endif // KRUEGER_OPENGL_C
