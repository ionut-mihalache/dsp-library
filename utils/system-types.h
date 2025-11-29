#ifndef __SYSTEM_TYPES_H
#define __SYSTEM_TYPES_H

#ifdef linux
#include <unistd.h>
#include <sys/uio.h>

typedef int aqua_file_handle;
typedef loff_t aqua_object_size;
typedef mode_t aqua_mode_t;
typedef pid_t aqua_pid_t;
typedef int aqua_prot_t;
typedef size_t aqua_size_t;
#endif

#ifdef _WIN32
#include <windows.h>

typedef HANDLE aqua_file_handle;
typedef DWORD aqua_object_size;
typedef unsigned int aqua_mode_t;
typedef DWORD aqua_pid_t;
typedef DWORD aqua_prot_t;
typedef SIZE_T aqua_size_t;
#endif

#endif // __SYSTEM_TYPES_H