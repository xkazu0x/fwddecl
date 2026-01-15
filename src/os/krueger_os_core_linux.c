#ifndef KRUEGER_OS_CORE_LINUX_C
#define KRUEGER_OS_CORE_LINUX_C

///////////////////////////////////////////
// NOTE: Init/Shutdown (Implemented Per-OS)

internal void
os_core_init(void) {
}

internal void
os_core_shutdown(void) {
}

/////////////////////////////////////////
// NOTE: System Info (Implemented Per-OS)

internal Date_Time
os_get_date_time(void) {
  Date_Time result = {0};
  return(result);
}

internal String8
os_get_exec_file_path(Arena *arena) {
  Temp scratch = scratch_begin(&arena, 1);
  u8 *tmp = push_array(scratch.arena, u8, PATH_MAX);
  ssize_t len = readlink("/proc/self/exe", (char *)tmp, PATH_MAX);
  u8 *str = push_array(arena, u8, len + 1);
  mem_copy(str, tmp, len);
  str[len] = 0;
  String8 result = str8(str, len);
  scratch_end(scratch);
  return(result);
}

//////////////////////////////////////////
// NOTE: System Abort (Implemented Per-OS)

internal void
os_abort(s32 exit_code) {
  exit(exit_code);
}

///////////////////////////////////////////////
// NOTE: Memory Allocation (Implemented Per-OS)

internal void *
os_reserve(uxx size) {
  void *result = mmap(0, size, PROT_NONE , MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (result == MAP_FAILED) result = 0;
  return(result);
}

internal b32
os_commit(void *ptr, uxx size) {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
  return(true);
}

internal void
os_decommit(void *ptr, uxx size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

internal void
os_release(void *ptr, uxx size) {
  munmap(ptr, size);
}

//////////////////////////////////
// NOTE: Time (Implemented Per-OS)

internal u64
os_get_time_us(void) {
  struct timespec clock;
  clock_gettime(CLOCK_MONOTONIC, &clock);
  u64 result = clock.tv_sec*million(1) + clock.tv_nsec/thousand(1); 
  return(result);
}

internal void
os_sleep_ms(u32 ms) {
  usleep(ms*thousand(1));
}

/////////////////////////////////////////
// NOTE: File System (Implemented Per-OS)

internal OS_Handle
os_file_open(String8 path, OS_File_Flags flags) {
  OS_Handle result = {0};
  Temp scratch = scratch_begin(0, 0);
  path = str8_copy(scratch.arena, path);
  int linux_flags = 0;
  if ((flags & OS_FILE_READ) && (flags & OS_FILE_WRITE)) {
    linux_flags = O_RDWR;
  } else if(flags & OS_FILE_WRITE) {
    linux_flags = O_WRONLY;
  } else if(flags & OS_FILE_READ) {
    linux_flags = O_RDONLY;
  }
  if(flags & OS_FILE_WRITE) {
    linux_flags |= O_CREAT;
  }
  int fd = open((char *)path.str, linux_flags, S_IRUSR | S_IWUSR);
  if (fd != -1) {
    result.ptr[0] = fd;
  }
  scratch_end(scratch);
  return(result);
}

internal void
os_file_close(OS_Handle file) {
  int fd = (int)file.ptr[0];
  close(fd);
}

internal u32
os_file_read(OS_Handle file, void *buffer, u64 size) {
  int fd = (int)file.ptr[0];
  u64 read_size = read(fd, buffer, size);
  return(read_size);
}

internal u32
os_file_write(OS_Handle file, void *buffer, u64 size) {
  int fd = (int)file.ptr[0];
  u64 write_size = write(fd, buffer, size);
  return(write_size);
}

internal u64
os_file_get_size(OS_Handle file) {
  u64 result = 0;
  int fd = (int)file.ptr[0];
  struct stat st;
  if (fstat(fd, &st) == 0) {
    result = st.st_size;
  }
  return(result);
}

internal File_Properties
os_file_get_properties(Os_Handle file) {
  File_Properties result = {0};
  return(result);
}

internal b32
os_copy_file_path(String8 dst, String8 src) {
  b32 result = false;
  OS_Handle src_h = os_file_open(src, OS_FILE_READ);
  OS_Handle dst_h = os_file_open(dst, OS_FILE_WRITE);
  if (!os_handle_match(src_h, OS_HANDLE_NULL) &&
      !os_handle_match(dst_h, OS_HANDLE_NULL)) {
    int src_fd = (int)src_h.ptr[0];
    int dst_fd = (int)dst_h.ptr[0];
    off_t sendfile_off = 0;
    u64 size = os_get_file_size(src_h);
    u32 write_size = sendfile(dst_fd, src_fd, &sendfile_off, size);
    if (write_size == size) {
      result = true;
    }
    os_file_close(src_h);
    os_file_close(dst_h);
  }
  return(result);
}

internal OS_File_Iter *
os_file_iter_begin(Arena *arena, String8 path, OS_File_Iter_Flags flags) {
  // TODO:
  OS_File_Iter *result = 0;
  return(result);
}

internal b32
os_file_iter_next(Arena *arena, OS_File_Iter *iter, OS_File_Info *info_out) {
  // TODO:
  b32 result = false;
  return(result);
}

internal void
os_file_iter_end(OS_File_Iter *iter) {
  // TODO:
}

//////////////////////////////////////////////////////////
// NOTE: Dinamically-Loaded Libraries (Implemented Per-OS)

internal OS_Handle
os_library_open(String8 path) {
  OS_Handle result = {0};
  Temp scratch = scratch_begin(0, 0);
  path = str8_copy(scratch.arena, path);
  result.ptr[0] = (uxx)dlopen((char *)path.str, RTLD_LAZY | RTLD_LOCAL);
  scratch_end(scratch);
  return(result);
}

internal void
os_library_close(OS_Handle lib) {
  void *so = (void *)lib.ptr[0];
  dlclose(so);
}

internal void *
os_library_load_proc(OS_Handle lib, String8 name) {
  Temp scratch = scratch_begin(0, 0);
  name = str8_copy(scratch.arena, name);
  void *so = (void *)lib.ptr[0];
  void *result = (void *)dlsym(so, (char *)name.str);
  scratch_end(scratch);
  return(result);
}

/////////////////////////////////////////
// NOTE: Entry Point (Implemented Per-OS)

#if BUILD_ENTRY_POINT
int
main(int argc, char **argv) {
  base_entry_point(argc, argv);
  return(0);
}
#endif

#endif // KRUEGER_OS_CORE_LINUX_C
