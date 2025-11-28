#ifndef __SYSTEM_VALUES_H
#define __SYSTEM_VALUES_H

#ifdef linux
#include <sys/mman.h>

#define AQUA_PROT_READ PROT_READ
#define AQUA_PROT_WRITE PROT_WRITE
#endif

#ifdef _WIN32
#include <windows.h>

#define AQUA_PROT_READ FILE_MAP_READ
#define AQUA_PROT_WRITE FILE_MAP_WRITE
#endif

#endif // __SYSTEM_VALUES_H