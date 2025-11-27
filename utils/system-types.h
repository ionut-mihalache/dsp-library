#ifndef __SYSTEM_TYPES_H
#define __SYSTEM_TYPES_H

#include <Windows.h>

#ifdef linux
typedef int FILE_HANDLE;
#endif

#ifdef _WIN32
typedef HANDLE FILE_HANDLE
#endif

#endif // __SYSTEM_TYPES_H