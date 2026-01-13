#ifndef KRUEGER_PLATFORM_CORE_H
#define KRUEGER_PLATFORM_CORE_H

//////////////
// NOTE: Files

typedef u32 Platform_File_Flags;
enum {
  PLATFORM_FILE_READ        = (1<<0),
  PLATFORM_FILE_WRITE       = (1<<1),
  PLATFORM_FILE_EXECUTE     = (1<<2),
  PLATFORM_FILE_SHARE_READ  = (1<<3),
  PLATFORM_FILE_SHARE_WRITE = (1<<4),
};

typedef u32 Platform_File_Iter_Flags;
enum {
  PLATFORM_FILE_ITER_SKIP_FILE      = (1<<0),
  PLATFORM_FILE_ITER_SKIP_DIRECTORY = (1<<1),
  PLATFORM_FILE_ITER_SKIP_HIDDEN    = (1<<2),
  PLATFORM_FILE_ITER_DONE           = (1<<31),
};

typedef struct Platform_File_Iter Platform_File_Iter;
struct Platform_File_Iter {
  u32 flags;
  void *ptr;
};

typedef struct Platform_File_Info Platform_File_Info;
struct Platform_File_Info {
  String8 name;
  File_Properties props;
};

///////////////
// NOTE: Handle

typedef struct Platform_Handle Platform_Handle;
struct Platform_Handle {
  uxx ptr[1];
};

#define PLATFORM_HANDLE_NULL ((Platform_Handle){0})

///////////////////////////////////
// NOTE: Helpers (Implemented Once)

internal b32 platform_handle_match(Platform_Handle a, Platform_Handle b);
internal b32 platform_handle_is_valid(Platform_Handle handle);
internal String8 platform_data_from_file_path(Arena *arena, String8 file_path);

/////////////////////////////////////////////////
// NOTE: Init/Shutdown (Implemented Per-Platform)

internal void platform_core_init(void);
internal void platform_core_shutdown(void);

internal Date_Time platform_get_date_time(void);
internal String8 platform_get_exec_file_path(Arena *arena);

internal void platform_abort(s32 exit_code);

/////////////////////////////////////////////////////
// NOTE: Memory Allocation (Implemented Per-Platform)

internal void *platform_reserve(uxx size);
internal b32 platform_commit(void *ptr, uxx size);
internal void platform_decommit(void *ptr, uxx size);
internal void platform_release(void *ptr, uxx size);

////////////////////////////////////////
// NOTE: Time (Implemented Per-Platform)

internal u64 platform_get_time_us(void);
internal void platform_sleep_ms(u32 ms);

///////////////////////////////////////////////
// NOTE: File System (Implemented Per-Platform)

internal Platform_Handle platform_file_open(String8 path, Platform_File_Flags flags);
internal void platform_file_close(Platform_Handle file);
internal u32 platform_file_read(Platform_Handle file, void *buffer, u64 size);
internal u32 platform_file_write(Platform_Handle file, void *buffer, u64 size);
internal u64 platform_get_file_size(Platform_Handle file);
internal File_Properties platform_get_file_properties(Platform_Handle file);
internal b32 platform_copy_file_path(String8 dst, String8 src);

internal Platform_File_Iter *platform_file_iter_begin(Arena *arena, String8 path, Platform_File_Iter_Flags flags);
internal b32 platform_file_iter_next(Arena *arena, Platform_File_Iter *iter, Platform_File_Info *info_out);
internal void platform_file_iter_end(Platform_File_Iter *iter);

////////////////////////////////////////////////////////////////
// NOTE: Dinamically-Loaded Libraries (Implemented Per-Platform)

internal Platform_Handle platform_library_open(String8 path);
internal void *platform_library_load_proc(Platform_Handle lib, String8 name);
internal void platform_library_close(Platform_Handle lib);

///////////////////////////////////////////////
// NOTE: Entry Point (Implemented Per-Platform)

#if BUILD_ENTRY_POINT
internal void entry_point(int argc, char **argv);
#endif

#endif // KRUEGER_PLATFORM_CORE_H
