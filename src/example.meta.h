#ifndef EXAMPLE_META_H
#define EXAMPLE_META_H

typedef struct Image Image;

internal void *test_ptr(void *ptr, void *ptr2);
internal void draw_example(Image dst);
internal Image image_alloc(u32 width, u32 height);
internal Image image_scissor(Image image, u32 x0, u32 y0, u32 x1, u32 y1);

#endif // EXAMPLE_META_H