#ifndef KRUEGER_OS_GFX_LINUX_H
#define KRUEGER_OS_GFX_LINUX_H

/////////////////////////////
// NOTE: Includes / Libraries

#include <X11/Xlib.h>
#include <X11/Xutil.h>

//////////////
// NOTE: Types

typedef struct _Linux_Window _Linux_Window;
struct _Linux_Window {
  _Linux_Window *next;
  _Linux_Window *prev;
  Window xwnd;
};

typedef struct _Linux_Graphics_State _Linux_Graphics_State;
struct _Linux_Graphics_State {
  Arena *arena;
  Display *display;
  Atom wm_delete_window;
  Os_Graphics_Info info;
  _Linux_Window *first_window;
  _Linux_Window *last_window;
  _Linux_Window *first_free_window;
};

////////////////
// NOTE: Globals

global _Linux_Graphics_State *_lnx_gfx_state;

//////////////////
// NOTE: Functions

internal Os_Handle _linux_handle_from_window(_Linux_Window *window);
internal _Linux_Window *_linux_window_from_handle(Os_Handle handle);
internal _Linux_Window *_linux_window_from_xwnd(Window xwnd);

internal _Linux_Window *_linux_window_alloc(void);
internal void _linux_window_release(_Linux_Window *window);

internal Keycode _linux_keycode_from_keysym(KeySym keysym);

#endif // KRUEGER_OS_GFX_LINUX_H
