// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Created by ionut on 29.06.2024.
//

#ifndef DSP_MACROS_H
#define DSP_MACROS_H

#include <errno.h>
#include <stdlib.h>

#define CHOOSE_MACRO(p_Macro, ...) p_Macro
#define CHOOSE_MACRO1(p_1, p_Macro, ...) CHOOSE_MACRO(p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO2(p_1, p_2, p_Macro, ...)                                  \
    CHOOSE_MACRO1(p_2, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO3(p_1, p_2, p_3, p_Macro, ...)                             \
    CHOOSE_MACRO2(p_2, p_3, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO4(p_1, p_2, p_3, p_4, p_Macro, ...)                        \
    CHOOSE_MACRO3(p_2, p_3, p_4, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO5(p_1, p_2, p_3, p_4, p_5, p_Macro, ...)                   \
    CHOOSE_MACRO4(p_2, p_3, p_4, p_5, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO6(p_1, p_2, p_3, p_4, p_5, p_6, p_Macro, ...)              \
    CHOOSE_MACRO5(p_2, p_3, p_4, p_5, p_6, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO7(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_Macro, ...)         \
    CHOOSE_MACRO6(p_2, p_3, p_4, p_5, p_6, p_7, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO8(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_Macro, ...)    \
    CHOOSE_MACRO7(p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO9(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_Macro,    \
                      ...)                                                     \
    CHOOSE_MACRO8(p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO10(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_10,      \
                       p_Macro, ...)                                           \
    CHOOSE_MACRO9(p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_10, p_Macro,       \
                  __VA_ARGS__)

#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "%s (%d): %s - %s\n", __FILE__, __LINE__,          \
                    call_description, strerror(errno));                        \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

#define max(a, b)                                                              \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a >= _b ? _a : _b;                                                    \
    })

#define min(a, b)                                                              \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a <= _b ? _a : _b;                                                    \
    })

#define QPUSH(p_Queue, p_QMaxSize, p_Code)                                     \
    do {                                                                       \
        Sync.mutexLock((p_Queue)->m_Metadata.m_Lock);                          \
        while (*(p_Queue)->m_Metadata.m_Size == (p_QMaxSize)) {                \
            Sync.condWait((p_Queue)->m_Metadata.m_EmptyCond,                   \
                          (p_Queue)->m_Metadata.m_Lock);                       \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PushIdxPtr) =                                \
            ((*(p_Queue)->m_Metadata.m_PushIdxPtr) + 1) % (p_QMaxSize);        \
        (*(p_Queue)->m_Metadata.m_Size)++;                                     \
                                                                               \
        Sync.mutexUnlock((p_Queue)->m_Metadata.m_Lock);                        \
                                                                               \
        Sync.condBroadcast((p_Queue)->m_Metadata.m_FullCond);                  \
    } while (0)

#define QPOP(p_Queue, p_QMaxSize, p_Code)                                      \
    do {                                                                       \
        Sync.mutexLock((p_Queue)->m_Metadata.m_Lock);                          \
        while (*(p_Queue)->m_Metadata.m_Size == 0) {                           \
            Sync.condWait((p_Queue)->m_Metadata.m_FullCond,                    \
                          (p_Queue)->m_Metadata.m_Lock);                       \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PopIdxPtr) =                                 \
            ((*(p_Queue)->m_Metadata.m_PopIdxPtr) + 1) % (p_QMaxSize);         \
        (*(p_Queue)->m_Metadata.m_Size)--;                                     \
                                                                               \
        Sync.mutexUnlock((p_Queue)->m_Metadata.m_Lock);                        \
                                                                               \
        Sync.condBroadcast((p_Queue)->m_Metadata.m_EmptyCond);                 \
    } while (0)

#endif // DSP_MACROS_H
