//
// Created by ionut on 01.06.2024.
//

#ifndef DSP_LOG_H
#define DSP_LOG_H

#include <stdio.h>
#include <time.h>

#ifdef __COMPILE_MODE_DEBUG__
#define LOGF(...)                                                              \
    do {                                                                       \
        time_t t = time(NULL);                                                 \
        struct tm *tm = localtime(&t);                                         \
                                                                               \
        fprintf(stdout, "[%d-%02d-%02d %02d:%02d:%02d]: ", tm->tm_year + 1900, \
                tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,          \
                tm->tm_sec);                                                   \
                                                                               \
        fprintf(stdout, __VA_ARGS__);                                          \
    } while (0)

#define ELOGF(...)                                                             \
    do {                                                                       \
        time_t t = time(NULL);                                                 \
        struct tm *tm = localtime(&t);                                         \
                                                                               \
        fprintf(stderr, "[%d-%02d-%02d %02d:%02d:%02d]: ", tm->tm_year + 1900, \
                tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,          \
                tm->tm_sec);                                                   \
                                                                               \
        fprintf(stderr, __VA_ARGS__);                                          \
    } while (0)

#define CHOOSE_LOG_MACRO(p_Log1, p_Log2, p_LogMacroName, ...) p_LogMacroName

#define LOG1(p_Log1) LOGF("%s\n", p_Log1)
#define LOG2(p_Log1, p_Log2) LOGF("%s %s\n", p_Log1, p_Log2)

#define LOG_ERR1(p_Log1) ELOGF("%s\n", p_Log1)
#define LOG_ERR2(p_Log1, p_Log2) ELOGF("%s %s\n", p_Log1, p_Log2)

#define LOG(...) CHOOSE_LOG_MACRO(__VA_ARGS__, LOG2, LOG1)(__VA_ARGS__)
#define LOG_ERR(...)                                                           \
    CHOOSE_LOG_MACRO(__VA_ARGS__, LOG_ERR2, LOG_ERR1)(__VA_ARGS__)

#else
#define LOGF(...)
#define ELOGF(...)
#define LOG(...)
#define LOG_ERR(...)
#endif

#endif // DSP_LOG_H