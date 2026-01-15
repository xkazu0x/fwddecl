#ifndef KRUEGER_OS_CORE_C
#define KRUEGER_OS_CORE_C

///////////////////////////////////////////
// NOTE: Handle (Helpers, Implemented Once)

internal b32
os_handle_match(Os_Handle a, Os_Handle b) {
  b32 result = (a.ptr[0] == b.ptr[0]);
  return(result);
}

internal b32
os_handle_is_valid(Os_Handle handle) {
  b32 result = !os_handle_match(handle, OS_HANDLE_NULL);
  return(result);
}

////////////////////////////////////////////////
// NOTE: File System (Helpers, Implemented Once)

internal String8
os_data_from_file_path(Arena *arena, String8 file_path) {
  String8 result = {0};
  Os_Handle file = os_file_open(file_path, OS_FILE_READ | OS_FILE_SHARE_READ);
  if (os_handle_is_valid(file)) {
    result.len = os_file_get_size(file);
    result.str = arena_push(arena, result.len);
    os_file_read(file, result.str, result.len);
    os_file_close(file);
  }
  return(result);
}

internal b32
os_write_data_to_file_path(String8 path, String8 data) {
  b32 result = false;
  Os_Handle file = os_file_open(path, OS_FILE_WRITE);
  if (os_handle_is_valid(file)) {
    u32 write_size = os_file_write(file, data.str, data.len);
    result = (write_size == data.len);
    os_file_close(file);
  }
  return(result);
}

#endif // KRUEGER_OS_CORE_C
