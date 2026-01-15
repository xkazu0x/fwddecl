#ifndef KRUEGER_OS_GFX_H
#define KRUEGER_OS_GFX_H

#include "krueger_keycode.h"

//////////////
// NOTE: Types

typedef struct Os_Graphics_Info Os_Graphics_Info;
struct Os_Graphics_Info {
  f32 refresh_rate;
};

typedef enum Os_Event_Type {
  OS_EVENT_WINDOW_CLOSE,
  OS_EVENT_KEY_PRESS,
  OS_EVENT_KEY_RELEASE,
} Os_Event_Type;

typedef struct Os_Event Os_Event;
struct Os_Event {
  Os_Event *next;
  Os_Event_Type type;
  Os_Handle window;
  Keycode keycode;
};

typedef struct Os_Event_List Os_Event_List;
struct Os_Event_List {
  Os_Event *first;
  Os_Event *last;
  u32 count;
};

////////////////////////////////////////////////////
// NOTE: Event Functions (Helpers, Implemented Once)

internal Os_Event *os_event_list_push(Arena *arena, Os_Event_List *list, Os_Event_Type type);

/////////////////////////////////////////////////
// NOTE: Main Initialization (Implemented Per-OS)

internal void os_graphics_init(void);

//////////////////////////////////////////////////
// NOTE: Graphics System Info (Implemented Per-OS)

internal Os_Graphics_Info os_get_graphics_info(void);

/////////////////////////////////////
// NOTE: Windows (Implemented Per-OS)

internal Os_Handle os_window_open(String8 name, s32 width, s32 height);
internal void os_window_close(Os_Handle handle);
internal void os_window_show(Os_Handle handle);
internal void os_window_blit(Os_Handle handle, u32 *buffer, s32 buffer_w, s32 buffer_h);
internal b32 os_window_is_fullscreen(Os_Handle handle);
internal void os_window_set_fullscreen(Os_Handle handle, b32 fullscreen);
internal Rect2 os_window_get_client_rect(Os_Handle handle);

////////////////////////////////////
// NOTE: Events (Implemented Per-OS)

internal Os_Event_List os_get_event_list(Arena *arena);

#endif // KRUEGER_OS_GFX_H
