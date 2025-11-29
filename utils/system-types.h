#ifndef AQUA_SYSTEM_TYPES_H
#define AQUA_SYSTEM_TYPES_H

#if defined(__linux__)
#include <sys/user.h>
#include <unistd.h>
#include <sys/uio.h>

typedef int aqua_file_handle;
typedef loff_t aqua_object_size;
typedef mode_t aqua_mode_t;
typedef pid_t aqua_pid_t;
typedef int aqua_prot_t;
typedef size_t aqua_size_t;

#elif defined(_WIN32)
#include <windows.h>

typedef HANDLE aqua_file_handle;
typedef DWORD aqua_object_size;
typedef SECURITY_ATTRIBUTES *aqua_mode_t;
typedef DWORD aqua_pid_t;
typedef DWORD aqua_prot_t;
typedef SIZE_T aqua_size_t;

#else
#error "Platform not supported by AQUA"
#endif

#endif // AQUA_SYSTEM_TYPES_H
