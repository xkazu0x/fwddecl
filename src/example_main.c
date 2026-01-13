#define PLATFORM_FEATURE_GRAPHICS 1

#include "base/krueger_base.h"
#include "platform/krueger_platform.h"

#include "base/krueger_base.c"
#include "platform/krueger_platform.c"

#include "example.c"

internal void
entry_point(int argc, char **argv) {
  u32 render_w = 800;
  u32 render_h = 600;

  u32 window_w = render_w;
  u32 window_h = render_h;

  Platform_Handle window = platform_window_open(str8_lit("example"), window_w, window_h);
  platform_window_show(window);

  Image image = image_alloc(render_w, render_h);

  for (b32 quit = false; !quit;) {
    Temp scratch = scratch_begin(0, 0);
    Platform_Event_List event_list = platform_get_event_list(scratch.arena);
    for (Platform_Event *event = event_list.first; event != 0; event = event->next) {
      switch (event->type) {
        case PLATFORM_EVENT_WINDOW_CLOSE: {
          quit = true;
        } break;
      }
    }
    if (quit) break;
    draw_example(image);
    platform_window_blit(window, image.pixels, image.width, image.height);
    scratch_end(scratch);
  }
}
