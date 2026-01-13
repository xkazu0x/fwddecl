struct V2S32 {
  s32 x, y;
};

internal s32
add(s32 a, s32 b) {
  s32 result = a + b;
  return(result);
}

internal V2S32
v2s32_add(V2S32 a, V2S32 b) {
  V2S32 result = {
    .x = a.x + b.x,
    .y = a.y + b.y,
  };
  return(result);
}
