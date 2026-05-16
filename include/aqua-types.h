// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __AQUA_TYPES_H_
#define __AQUA_TYPES_H_

#include <stdint.h>

typedef void aqua_void_t;
typedef void *aqua_void_ptr_t;

typedef int aqua_int_t;
typedef long aqua_long_t;
typedef long long aqua_llong_t;

typedef int8_t aqua_i8_t;
typedef uint8_t aqua_u8_t;
typedef int16_t aqua_i16_t;
typedef uint16_t aqua_u16_t;
typedef int32_t aqua_i32_t;
typedef uint32_t aqua_u32_t;
typedef int64_t aqua_i64_t;
typedef uint64_t aqua_u64_t;

typedef aqua_u8_t aqua_mem_prot_t;
typedef aqua_u8_t aqua_mem_flags_t;
typedef aqua_u8_t aqua_file_flags_t;
typedef aqua_u32_t aqua_file_mode_t;
typedef aqua_u16_t aqua_mem_perm_t;
typedef aqua_u8_t aqua_bool_t;

typedef int aqua_file_handle_t;

#endif // __AQUA_TYPES_H_
