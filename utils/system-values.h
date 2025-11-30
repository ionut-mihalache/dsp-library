#ifndef AQUA_SYSTEM_VALUES_H
#define AQUA_SYSTEM_VALUES_H

#if defined(__linux__)
#include <sys/mman.h>

#define AQUA_PROT_READ PROT_READ
#define AQUA_PROT_WRITE PROT_WRITE

#define AQUA_S_IRUSR S_IRUSR
#define AQUA_S_IWUSR S_IWUSR
#define AQUA_S_IRGRP S_IRGRP
#define AQUA_S_IWGRP S_IWGRP
#define AQUA_S_IROTH S_IROTH
#define AQUA_S_IWOTH S_IWOTH

#elif defined(_WIN32)
#include <Windows.h>

#define AQUA_PROT_READ FILE_MAP_READ
#define AQUA_PROT_WRITE FILE_MAP_WRITE

#define AQUA_S_IRUSR 0x01
#define AQUA_S_IWUSR 0x02
#define AQUA_S_IRGRP 0x04
#define AQUA_S_IWGRP 0x08
#define AQUA_S_IROTH 0x10
#define AQUA_S_IWOTH 0x20

#else
#error "Platform not supported by AQUA"
#endif

#endif // AQUA_SYSTEM_VALUES_H
