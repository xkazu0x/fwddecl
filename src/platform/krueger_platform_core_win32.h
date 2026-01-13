#ifndef KRUEGER_PLATFORM_CORE_WIN32_H
#define KRUEGER_PLATFORM_CORE_WIN32_H

/////////////////////////////
// NOTE: Includes / Libraries

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NO_MIN_MAX
#define NO_MIN_MAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm")

//////////////
// NOTE: Types

typedef struct _Win32_File_Iter _Win32_File_Iter;
struct _Win32_File_Iter {
  HANDLE handle;
  WIN32_FIND_DATAW find_data;
};

////////////////////////
// NOTE: Windows Globals

global u64 _win32_us_res;

///////////////////////////
// NOTE: Windows Functions

internal void _win32_date_time_from_system_time(Date_Time *out, SYSTEMTIME *in);
internal void _win32_dense_time_from_file_time(Dense_Time *out, FILETIME *in);
internal File_Property_Flags _win32_file_property_flags_from_dwFileAttributes(DWORD dwFileAttributes);

#endif // KRUEGER_PLATFORM_CORE_WIN32_H
