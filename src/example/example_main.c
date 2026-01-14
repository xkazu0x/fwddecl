#define PLATFORM_FEATURE_GRAPHICS 1

#include "base/krueger_base.h"
#include "platform/krueger_platform.h"

#include "base/krueger_base.c"
#include "platform/krueger_platform.c"

#include "example_main.meta.h"

struct Image {
  u32 width;
  u32 height;
  u32 pitch;
  u32 *pixels;
};

fwddecl_ignore internal void
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

internal void *
test_ptr(void *ptr, void* ptr2) {
  return(0);
}

internal void
draw_example(Image dst) {
  local u32 tick = 0;

  u32 x0 = dst.width/8;
  u32 y0 = dst.height/6;
  u32 x1 = dst.width - x0;
  u32 y1 = dst.height - y0;

  Image image = image_scissor(dst, x0, y0, x1, y1);
  u32 *row = image.pixels;

  for (u32 y = 0; y < image.height; ++y) {
    u32 *pixels = row;
    for (u32 x = 0; x < image.width; ++x) {
      u32 g = (((y - tick)%256) << 8);
      u32 b = (((x + tick)%256) << 0);
      *pixels++ = g | b;
    }
    row += image.pitch;
  }

  ++tick;
}

internal Image
image_alloc(u32 width, u32 height) {
  uxx image_size = width*height*sizeof(u32);
  Image result = {0};
  result.width = width;
  result.height = height;
  result.pitch = width;
  result.pixels = platform_reserve(image_size);
  platform_commit(result.pixels, image_size);
  return(result);
}

internal Image
image_scissor(Image image, u32 x0, u32 y0, u32 x1, u32 y1) {
  Image result = {0};
  result.width = x1 - x0;
  result.height = y1 - y0;
  result.pitch = image.pitch;
  result.pixels = image.pixels + y0*image.width + x0;
  return(result);
}
