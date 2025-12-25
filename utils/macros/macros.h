//
// Created by ionut on 29.06.2024.
//

#ifndef AQUA_DSP_MACROS_H
#define AQUA_DSP_MACROS_H

#include <stdlib.h>
#include <stdio.h>

#ifdef linux
#include <errno.h>
#include <string.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

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

#if defined(__linux__)
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "%s (%d): %s - %s\n", __FILE__, __LINE__,          \
                    call_description, strerror(errno));                        \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)
#endif

#if defined(_WIN32)
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        LPVOID errorStr;                                                       \
        DWORD errNumber = GetLastError();                                      \
        FormatMessage(                                                         \
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |      \
                FORMAT_MESSAGE_IGNORE_INSERTS,                                 \
            NULL, errNumber, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),        \
            (LPTSTR) & errorStr, 0, NULL);                                     \
                                                                               \
        if (assertion) {                                                       \
            fprintf(stderr, "%s (%d): %s - %s\n", __FILE__, __LINE__,          \
                    call_description, (char *)errorStr);                       \
            LocalFree(errorStr);                                               \
            ExitProcess(EXIT_FAILURE);                                         \
        }                                                                      \
    } while (0)
#endif

#if defined(__linux__)
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
#endif

#ifdef linux
#define QPUSH(p_Queue, p_QMaxSize, p_Code)                                     \
    do {                                                                       \
        pthread_mutex_lock((p_Queue)->m_Metadata.m_Lock);                      \
        while (*(p_Queue)->m_Metadata.m_Size == (p_QMaxSize)) {                \
            pthread_cond_wait((p_Queue)->m_Metadata.m_EmptyCond,               \
                              (p_Queue)->m_Metadata.m_Lock);                   \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PushIdxPtr) =                                \
            ((*(p_Queue)->m_Metadata.m_PushIdxPtr) + 1) % (p_QMaxSize);        \
        (*(p_Queue)->m_Metadata.m_Size)++;                                     \
                                                                               \
        pthread_mutex_unlock((p_Queue)->m_Metadata.m_Lock);                    \
                                                                               \
        pthread_cond_broadcast((p_Queue)->m_Metadata.m_FullCond);              \
    } while (0)

#define QPOP(p_Queue, p_QMaxSize, p_Code)                                      \
    do {                                                                       \
        pthread_mutex_lock((p_Queue)->m_Metadata.m_Lock);                      \
        while (*(p_Queue)->m_Metadata.m_Size == 0) {                           \
            pthread_cond_wait((p_Queue)->m_Metadata.m_FullCond,                \
                              (p_Queue)->m_Metadata.m_Lock);                   \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PopIdxPtr) =                                 \
            ((*(p_Queue)->m_Metadata.m_PopIdxPtr) + 1) % (p_QMaxSize);         \
        (*(p_Queue)->m_Metadata.m_Size)--;                                     \
                                                                               \
        pthread_mutex_unlock((p_Queue)->m_Metadata.m_Lock);                    \
                                                                               \
        pthread_cond_broadcast((p_Queue)->m_Metadata.m_EmptyCond);             \
    } while (0)
#endif

#ifdef _WIN32
#define QPUSH(p_Queue, p_QMaxSize, p_Code)                                     \
    do {                                                                       \
        WaitForSingleObject((p_Queue)->m_Metadata.m_Lock, INFINITE);           \
        while (*(p_Queue)->m_Metadata.m_Size == (p_QMaxSize)) {                \
            ReleaseMutex((p_Queue)->m_Metadata.m_Lock);                       \
            WaitForSingleObject((p_Queue)->m_Metadata.m_EmptyCond, INFINITE);  \
            WaitForSingleObject((p_Queue)->m_Metadata.m_Lock, INFINITE);       \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PushIdxPtr) =                                \
            ((*(p_Queue)->m_Metadata.m_PushIdxPtr) + 1) % (p_QMaxSize);        \
        (*(p_Queue)->m_Metadata.m_Size)++;                                     \
                                                                               \
        ReleaseMutex((p_Queue)->m_Metadata.m_Lock);                            \
                                                                               \
        SetEvent((p_Queue)->m_Metadata.m_FullCond);                            \
    } while (0)

#define QPOP(p_Queue, p_QMaxSize, p_Code)                                      \
    do {                                                                       \
        WaitForSingleObject((p_Queue)->m_Metadata.m_Lock, INFINITE);           \
        while (*(p_Queue)->m_Metadata.m_Size == 0) {                           \
            ReleaseMutex((p_Queue)->m_Metadata.m_Lock);                        \
            WaitForSingleObject((p_Queue)->m_Metadata.m_FullCond, INFINITE);   \
            WaitForSingleObject((p_Queue)->m_Metadata.m_Lock, INFINITE);       \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PopIdxPtr) =                                 \
            ((*(p_Queue)->m_Metadata.m_PopIdxPtr) + 1) % (p_QMaxSize);         \
        (*(p_Queue)->m_Metadata.m_Size)--;                                     \
                                                                               \
        ReleaseMutex((p_Queue)->m_Metadata.m_Lock);                            \
                                                                               \
        SetEvent((p_Queue)->m_Metadata.m_EmptyCond);                           \
    } while (0)
#endif

#endif // AQUA_DSP_MACROS_H
