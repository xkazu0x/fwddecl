#ifndef KRUEGER_PLATFORM_CORE_C
#define KRUEGER_PLATFORM_CORE_C

///////////////////////////////////
// NOTE: Helpers (Implemented Once)

internal b32
platform_handle_match(Platform_Handle a, Platform_Handle b) {
  b32 result = (a.ptr[0] == b.ptr[0]);
  return(result);
}

internal b32
platform_handle_is_valid(Platform_Handle handle) {
  b32 result = !platform_handle_match(handle, PLATFORM_HANDLE_NULL);
  return(result);
}

internal String8
platform_data_from_file_path(Arena *arena, String8 file_path) {
  String8 result = {0};
  Platform_Handle file = platform_file_open(file_path, PLATFORM_FILE_READ | PLATFORM_FILE_SHARE_READ);
  if (platform_handle_is_valid(file)) {
    result.len = platform_get_file_size(file);
    result.str = arena_push(arena, result.len);
    platform_file_read(file, result.str, result.len);
    platform_file_close(file);
  }
  return(result);
}

#endif // KRUEGER_PLATFORM_CORE_C
