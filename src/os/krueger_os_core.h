#ifndef KRUEGER_OS_CORE_H
#define KRUEGER_OS_CORE_H

//////////////
// NOTE: Files

typedef u32 Os_File_Flags;
enum {
  OS_FILE_READ        = (1<<0),
  OS_FILE_WRITE       = (1<<1),
  OS_FILE_EXECUTE     = (1<<2),
  OS_FILE_SHARE_READ  = (1<<3),
  OS_FILE_SHARE_WRITE = (1<<4),
};

typedef u32 Os_File_Iter_Flags;
enum {
  OS_FILE_ITER_SKIP_FILE      = (1<<0),
  OS_FILE_ITER_SKIP_DIRECTORY = (1<<1),
  OS_FILE_ITER_SKIP_HIDDEN    = (1<<2),
  OS_FILE_ITER_DONE           = (1<<31),
};

typedef struct Os_File_Iter Os_File_Iter;
struct Os_File_Iter {
  u32 flags;
  void *ptr;
};

typedef struct Os_File_Info Os_File_Info;
struct Os_File_Info {
  String8 name;
  File_Properties props;
};

///////////////
// NOTE: Handle

typedef struct Os_Handle Os_Handle;
struct Os_Handle {
  uxx ptr[1];
};

#define OS_HANDLE_NULL ((Os_Handle){0})

///////////////////////////////////////////
// NOTE: Handle (Helpers, Implemented Once)

internal b32 os_handle_match(Os_Handle a, Os_Handle b);
internal b32 os_handle_is_valid(Os_Handle handle);

////////////////////////////////////////////////
// NOTE: File System (Helpers, Implemented Once)

internal String8 os_data_from_file_path(Arena *arena, String8 file_path);
internal b32 os_write_data_to_file_path(String8 path, String8 data);

//////////////////////////////////////////////////////////
// NOTE: Main Initialization/Shutdown (Implemented Per-OS)

internal void os_core_init(void);
internal void os_core_shutdown(void);

/////////////////////////////////////////
// NOTE: System Info (Implemented Per-OS)

internal Date_Time os_get_date_time(void);
internal String8 os_get_exec_file_path(Arena *arena);

//////////////////////////////////////////
// NOTE: System Abort (Implemented Per-OS)

internal void os_abort(s32 exit_code);

///////////////////////////////////////////////
// NOTE: Memory Allocation (Implemented Per-OS)

internal void *os_reserve(uxx size);
internal b32 os_commit(void *ptr, uxx size);
internal void os_decommit(void *ptr, uxx size);
internal void os_release(void *ptr, uxx size);

//////////////////////////////////
// NOTE: Time (Implemented Per-OS)

internal u64 os_get_time_us(void);
internal void os_sleep_ms(u32 ms);

/////////////////////////////////////////
// NOTE: File System (Implemented Per-OS)

internal Os_Handle os_file_open(String8 path, Os_File_Flags flags);
internal void os_file_close(Os_Handle file);
internal u32 os_file_read(Os_Handle file, void *buffer, u64 size);
internal u32 os_file_write(Os_Handle file, void *buffer, u64 size);
internal u64 os_file_get_size(Os_Handle file);
internal File_Properties os_file_get_properties(Os_Handle file);
internal b32 os_copy_file_path(String8 dst, String8 src);

internal Os_File_Iter *os_file_iter_begin(Arena *arena, String8 path, Os_File_Iter_Flags flags);
internal b32 os_file_iter_next(Arena *arena, Os_File_Iter *iter, Os_File_Info *info_out);
internal void os_file_iter_end(Os_File_Iter *iter);

//////////////////////////////////////////////////////////
// NOTE: Dinamically-Loaded Libraries (Implemented Per-OS)

internal Os_Handle os_library_open(String8 path);
internal void os_library_close(Os_Handle lib);
internal void *os_library_load_proc(Os_Handle lib, String8 name);

/////////////////////////////////////////
// NOTE: Entry Point (Implemented Per-OS)

#if BUILD_ENTRY_POINT
internal void entry_point(int argc, char **argv);
#endif

#endif // KRUEGER_OS_CORE_H
