// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __AQUA_SYSTEM_VALUES_H_
#define __AQUA_SYSTEM_VALUES_H_

enum {
    AQUA_MEM_PROT_READ = 1u << 0,
    AQUA_MEM_PROT_WRITE = 1u << 1,
    AQUA_MEM_PROT_EXEC = 1u << 2
};

enum {
    AQUA_MEM_SHARED = 1u << 0,
    AQUA_MEM_PRIVATE = 1u << 1,
    AQUA_MEM_ANONYMOUS = 1u << 2
};

enum {
    AQUA_FILE_PERM_READ = 1u << 0,
    AQUA_FILE_PERM_WRITE = 1u << 1,
    AQUA_FILE_PERM_RDWR = 1u << 2
};

enum {
    AQUA_FILE_MODE_USER_READ = 1u << 0,
    AQUA_FILE_MODE_USER_WRITE = 1u << 1,
    AQUA_FILE_MODE_GROUP_READ = 1u << 2,
    AQUA_FILE_MODE_GROUP_WRITE = 1u << 3,
    AQUA_FILE_MODE_OTHER_READ = 1u << 4,
    AQUA_FILE_MODE_OTHER_WRITE = 1u << 5
};

#endif // __AQUA_SYSTEM_VALUES_H_
