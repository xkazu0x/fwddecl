#define OS_FEATURE_GFX 1

#define OGL_MAJOR_VER 3
#define OGL_MINOR_VER 0

#include "base/krueger_base.h"
#include "os/krueger_os.h"
#include "opengl/krueger_opengl.h"

#include "base/krueger_base.c"
#include "os/krueger_os.c"
#include "opengl/krueger_opengl.c"

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

  Os_Handle window = os_window_open(str8_lit("example"), window_w, window_h);
  Image image = image_alloc(render_w, render_h);

  ogl_window_equip(window);
  ogl_window_select(window);

  os_window_show(window);

  for (b32 quit = false; !quit;) {
    Temp scratch = scratch_begin(0, 0);
    Os_Event_List event_list = os_get_event_list(scratch.arena);
    for (Os_Event *event = event_list.first; event != 0; event = event->next) {
      switch (event->type) {
        case OS_EVENT_WINDOW_CLOSE: {
          quit = true;
        } break;
      }
    }

    draw_example(image);
    render_image_to_window(window, image);

    scratch_end(scratch);
  }
}

internal void
render_image_to_window(Os_Handle window, Image image) {
  Rect2 client_rect = os_window_get_client_rect(window);
  Vector2 client_size = vector2_sub(client_rect.max, client_rect.min);

  glViewport(0, 0, (s32)client_size.x, (s32)client_size.y);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, image.pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_2D);

  f32 display_w = image.width*(client_size.y/image.height);
  f32 display_h = image.height*(client_size.x/image.width);

  if (client_size.x >= display_w) {
    display_h = client_size.y;
  } else if (client_size.y >= display_h) {
    display_w = client_size.x;
  }

  f32 x = display_w/client_size.x;
  f32 y = display_h/client_size.y;

  glBegin(GL_TRIANGLES); {
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-x, y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x, -y);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-x, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x, -y);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-x, -y);
  } glEnd();

  ogl_window_swap(window);
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
  result.pixels = os_reserve(image_size);
  os_commit(result.pixels, image_size);
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
