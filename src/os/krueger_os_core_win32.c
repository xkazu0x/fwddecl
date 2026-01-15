#ifndef KRUEGER_OS_CORE_WIN32_C
#define KRUEGER_OS_CORE_WIN32_C

////////////////////////
// NOTE: Win32 Functions

internal void
_win32_date_time_from_system_time(Date_Time *out, SYSTEMTIME *in) {
  out->year   = in->wYear;
  out->month  = in->wMonth;
  out->day    = in->wDay;
  out->hour   = in->wHour;
  out->min    = in->wMinute;
  out->sec    = in->wSecond;
  out->msec   = in->wMilliseconds;
}

internal void
_win32_dense_time_from_file_time(Dense_Time *out, FILETIME *in) {
  SYSTEMTIME system_time;
  FileTimeToSystemTime(in, &system_time);
  Date_Time date_time;
  _win32_date_time_from_system_time(&date_time, &system_time);
  *out = dense_time_from_date_time(date_time);
}

internal File_Property_Flags
_win32_file_property_flags_from_dwFileAttributes(DWORD dwFileAttributes) {
  File_Property_Flags result = 0;
  if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    result |= FILE_PROPERTY_IS_DIRECTORY;
  }
  return(result);
}

///////////////////////////////////////////
// NOTE: Init/Shutdown (Implemented Per-OS)

internal void
os_core_init(void) {
  LARGE_INTEGER large_integer;
  QueryPerformanceFrequency(&large_integer);
  _win32_us_res = large_integer.QuadPart;
  timeBeginPeriod(1);
}

internal void
os_core_shutdown(void) {
  timeEndPeriod(1);
}

/////////////////////////////////////////
// NOTE: System Info (Implemented Per-OS)

internal Date_Time
os_get_date_time(void) {
  SYSTEMTIME system;
  GetSystemTime(&system);
  Date_Time result = {
    .year  = system.wYear,
    .month = system.wMonth,
    .day   = system.wDay,
    .hour  = system.wHour,
    .min   = system.wMinute,
    .sec   = system.wSecond,
    .msec  = system.wMilliseconds,
  };
  return(result);
}

internal String8
os_get_exec_file_path(Arena *arena) {
  Temp scratch = scratch_begin(&arena, 1);
  u16 *tmp = push_array(scratch.arena, u16, MAX_PATH);
  DWORD len = GetModuleFileNameW(0, tmp, MAX_PATH);
  u16 *str = push_array(scratch.arena, u16, len + 1);
  len = GetModuleFileNameW(0, str, len + 1);
  String8 result = str8_from_str16(arena, str16(str, len));
  scratch_end(scratch);
  return(result);
}

//////////////////////////////////////////
// NOTE: System Abort (Implemented Per-OS)

internal void
os_abort(s32 exit_code) {
  ExitProcess(exit_code);
}

///////////////////////////////////////////////
// NOTE: Memory Allocation (Implemented Per-OS)

internal void *
os_reserve(uxx size) {
  void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
  return(result);
}

internal b32
os_commit(void *ptr, uxx size) {
  b32 result = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
  return(result);
}

internal void
os_decommit(void *ptr, uxx size) {
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

internal void
os_release(void *ptr, uxx size) {
  VirtualFree(ptr, 0, MEM_RELEASE);
}

//////////////////////////////////
// NOTE: Time (Implemented Per-OS)

internal u64
os_get_time_us(void) {
  LARGE_INTEGER large_integer;
  QueryPerformanceCounter(&large_integer);
  u64 result = large_integer.QuadPart*million(1)/_win32_us_res;
  return(result);
}

internal void
os_sleep_ms(u32 ms) {
  Sleep(ms);
}

/////////////////////////////////////////
// NOTE: File System (Implemented Per-OS)

internal Os_Handle
os_file_open(String8 path, Os_File_Flags flags) {
  Os_Handle result = {0};
  Temp scratch = scratch_begin(0, 0);
  String16 path16 = str16_from_str8(scratch.arena, path);
  DWORD desired_access = 0;
  DWORD share_mode = 0;
  DWORD creation_disposition = OPEN_EXISTING;
  if (flags & OS_FILE_READ)        desired_access |= GENERIC_READ;
  if (flags & OS_FILE_WRITE)       desired_access |= GENERIC_WRITE;
  if (flags & OS_FILE_EXECUTE)     desired_access |= GENERIC_EXECUTE;
  if (flags & OS_FILE_SHARE_READ)  share_mode |= FILE_SHARE_READ;
  if (flags & OS_FILE_SHARE_WRITE) share_mode |= FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  if (flags & OS_FILE_WRITE)       creation_disposition = CREATE_ALWAYS;
  HANDLE handle = CreateFileW(path16.str, desired_access, share_mode, 0, creation_disposition, FILE_ATTRIBUTE_NORMAL, 0);
  if (handle != INVALID_HANDLE_VALUE) {
    result.ptr[0] = (uxx)handle; 
  }
  scratch_end(scratch);
  return(result);
}

internal void
os_file_close(Os_Handle file) {
  HANDLE handle = (HANDLE)file.ptr[0];
  CloseHandle(handle);
}

internal u32
os_file_read(Os_Handle file, void *buffer, u64 size) {
  u32 read_size = 0;
  HANDLE handle = (HANDLE)file.ptr[0];
  ReadFile(handle, buffer, (DWORD)size, (DWORD *)&read_size, 0);
  return(read_size);
}

internal u32
os_file_write(Os_Handle file, void *buffer, u64 size) {
  u32 write_size = 0;
  HANDLE handle = (HANDLE)file.ptr[0];
  WriteFile(handle, buffer, (DWORD)size, (DWORD *)&write_size, 0);
  return(write_size);
}

internal u64
os_file_get_size(Os_Handle file) {
  u64 result = 0;
  HANDLE handle = (HANDLE)file.ptr[0];
  GetFileSizeEx(handle, (LARGE_INTEGER *)&result);
  return(result);
}

internal File_Properties
os_file_get_properties(Os_Handle file) {
  File_Properties result = {0};
  if (os_handle_is_valid(file)) {
    HANDLE handle = (HANDLE)file.ptr[0];
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(handle, &info)) {
      result.size = compose_64bit(info.nFileSizeHigh, info.nFileSizeLow);
      _win32_dense_time_from_file_time(&result.created, &info.ftCreationTime);
      _win32_dense_time_from_file_time(&result.modified, &info.ftLastWriteTime);
      result.flags = _win32_file_property_flags_from_dwFileAttributes(info.dwFileAttributes);
    }
  }
  return(result);
}

internal b32
os_copy_file_path(String8 dst, String8 src) {
  Temp scratch = scratch_begin(0, 0);
  String16 dst16 = str16_from_str8(scratch.arena, dst);
  String16 src16 = str16_from_str8(scratch.arena, src);
  b32 result = CopyFileW(src16.str, dst16.str, 0);
  scratch_end(scratch);
  return(result);
}

internal Os_File_Iter *
os_file_iter_begin(Arena *arena, String8 path, Os_File_Iter_Flags flags) {
  Temp scratch = scratch_begin(&arena, 1);
  String8 path_with_wildcard = str8_cat(scratch.arena, path, str8_lit("\\*"));
  String16 path16 = str16_from_str8(scratch.arena, path_with_wildcard);
  Os_File_Iter *result = push_struct(arena, Os_File_Iter);
  result->flags = flags;
  result->ptr = push_struct(arena, _Win32_File_Iter);
  _Win32_File_Iter *win32_iter = (_Win32_File_Iter *)result->ptr;
  win32_iter->handle = FindFirstFileExW(path16.str, FindExInfoBasic, &win32_iter->find_data, FindExSearchNameMatch, 0, FIND_FIRST_EX_LARGE_FETCH);
  scratch_end(scratch);
  return(result);
}

internal b32
os_file_iter_next(Arena *arena, Os_File_Iter *iter, Os_File_Info *info_out) {
  b32 result = false;
  Os_File_Iter_Flags flags = iter->flags;
  _Win32_File_Iter *win32_iter = (_Win32_File_Iter *)iter->ptr;
  if (!(flags & OS_FILE_ITER_DONE) && win32_iter->handle != INVALID_HANDLE_VALUE) {
    do {
      b32 is_usable = true;

      WCHAR *file_name = win32_iter->find_data.cFileName;
      DWORD attributes = win32_iter->find_data.dwFileAttributes;

      if (file_name[0] == '.') {
        if (flags & OS_FILE_ITER_SKIP_HIDDEN) {
          is_usable = false;
        } else if (file_name[1] == 0) {
          is_usable = false;
        } else if (file_name[1] == '.' && file_name[2] == 0) {
          is_usable = false;
        }
      }

      if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (flags & OS_FILE_ITER_SKIP_DIRECTORY) {
          is_usable = false;
        }
      } else {
        if (flags & OS_FILE_ITER_SKIP_FILE) {
          is_usable = false;
        }
      }

      if (is_usable) {
        info_out->name = str8_from_str16(arena, str16_cstr(file_name));
        info_out->props.size = compose_64bit(win32_iter->find_data.nFileSizeHigh, win32_iter->find_data.nFileSizeLow);
        _win32_dense_time_from_file_time(&info_out->props.created, &win32_iter->find_data.ftCreationTime);
        _win32_dense_time_from_file_time(&info_out->props.modified, &win32_iter->find_data.ftLastWriteTime);
        info_out->props.flags = _win32_file_property_flags_from_dwFileAttributes(attributes);
        if (!FindNextFileW(win32_iter->handle, &win32_iter->find_data)) {
          iter->flags |= OS_FILE_ITER_DONE;
        }
        result = true;
        break;
      }
    } while (FindNextFileW(win32_iter->handle, &win32_iter->find_data));
  }
  if (!result) {
    iter->flags |= OS_FILE_ITER_DONE;
  }
  return(result);
}

internal void
os_file_iter_end(Os_File_Iter *iter) {
  _Win32_File_Iter *win32_iter = (_Win32_File_Iter *)iter->ptr;
  FindClose(win32_iter->handle);
}

//////////////////////////////////////////////////////////
// NOTE: Dinamically-Loaded Libraries (Implemented Per-OS)

internal Os_Handle
os_library_open(String8 path) {
  Os_Handle result = {0};
  Temp scratch = scratch_begin(0, 0);
  String16 path16 = str16_from_str8(scratch.arena, path);
  HMODULE module = LoadLibraryW(path16.str);
  result.ptr[0] = (uxx)module;
  scratch_end(scratch);
  return(result);
}

internal void
os_library_close(Os_Handle lib) {
  HMODULE module = (HMODULE)lib.ptr[0];
  FreeLibrary(module);
}

internal void *
os_library_load_proc(Os_Handle lib, String8 name) {
  Temp scratch = scratch_begin(0, 0);
  HMODULE module = (HMODULE)lib.ptr[0];
  name = str8_copy(scratch.arena, name);
  void *result = (void *)GetProcAddress(module, (LPCSTR)name.str);
  scratch_end(scratch);
  return(result);
}

/////////////////////////////////////////
// NOTE: Entry Point (Implemented Per-OS)

#if BUILD_ENTRY_POINT
internal void
_win32_entry_point_caller(int argc, WCHAR **wargv) {
  // NOTE: convert arguments from UTF-16 to UTF-8
  Arena *args_arena = arena_alloc(.res_size = MB(1));
  char **argv = push_array(args_arena, char *, argc);
  for (int i = 0; i < argc; i += 1) {
    String16 arg16 = str16_cstr((u16 *)wargv[i]);
    String8 arg8 = str8_from_str16(args_arena, arg16);
    argv[i] = (char *)arg8.str;
  }

  // NOTE: call base entry point
  base_entry_point(argc, argv);
}
#if BUILD_CONSOLE_INTERFACE
int
wmain(int argc, WCHAR **wargv) {
  _win32_entry_point_caller(argc, wargv);
  return(0);
}
#else
int
wWinMain(HINSTANCE instance, HINSTANCE prev_instance, LPWSTR cmd_line, int cmd_show) {
  _win32_entry_point_caller(__argc, __wargv);
  return(0);
}
#endif
#endif

#endif // KRUEGER_OS_CORE_WIN32_C
