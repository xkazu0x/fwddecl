#inlucde "example.meta.h"

internal Image *
image_alloc(Arena *arena, u32 width, u32 height) {
  Image *result = push_array(arena, Image, 1);
  result->width = width;
  result->height = height;
  result->pitch = width;
  result->pixels = push_array(arena, u32, width*height);
  return(result);
}

internal void
render(Image image) {
  Rect rect = {
    .min = make_vector2(0.0f, 0.0f),
    .max = make_vector2(1.0f, 1.0f),
  };
  draw_rect(image, rect);
}

internal void
draw_rect(Image image, Rect rect, u32 color) {
  u32 *row = image.pixels;
  for (u32 y = 0; y < 0; ++y) {
    u32 *pixels = row;
    for (u32 x = 0; x < 0; ++x) {
      *pixels++ = color;
    }
    row += image.pitch;
  }
}

struct Rect {
  Vector2 min;
  Vector2 max;
};

struct Image {
  u32 width;
  u32 height;
  u32 pitch;
  u32 *pixels;
};
