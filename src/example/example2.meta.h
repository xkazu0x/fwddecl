#ifndef EXAMPLE2_META_H
#define EXAMPLE2_META_H

#ifndef fwddecl_ignore
#define fwddecl_ignore
#endif

typedef struct V2S32 V2S32;
typedef struct Image Image;

internal s32 add(s32 a, s32 b);
internal V2S32 v2s32_add(V2S32 a, V2S32 b);
internal void render_image_to_window(Os_Handle window, Image image);
internal void draw_example(Image dst);
internal Image image_alloc(u32 width, u32 height);
internal Image image_scissor(Image image, u32 x0, u32 y0, u32 x1, u32 y1);

#endif // EXAMPLE2_META_H