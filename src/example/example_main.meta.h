#ifndef EXAMPLE_MAIN_META_H
#define EXAMPLE_MAIN_META_H

#ifndef fwddecl_ignore
#define fwddecl_ignore
#endif

typedef struct Image Image;

internal void render_image_to_window(Os_Handle window, Image image);
internal void draw_example(Image dst);
internal Image image_alloc(u32 width, u32 height);
internal Image image_scissor(Image image, u32 x0, u32 y0, u32 x1, u32 y1);

#endif // EXAMPLE_MAIN_META_H