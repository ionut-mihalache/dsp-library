// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Created by ionut on 29.06.2024.
//

#ifndef DSP_HASHMAP_H
#define DSP_HASHMAP_H

#include <pthread.h>
#include <stdint.h>

#include "macros.h"

#define HM_MAX_SIZE 8192
#define HM_UNBLOCKED 0
#define HM_BLOCKED 1
#define HM_NOT_OCCUPIED 0
#define HM_OCCUPIED 1

#define CHOOSE_HASHMAP_MACRO(p_1, p_2, p_3, p_4, p_Macro, ...)                 \
    CHOOSE_MACRO4(p_1, p_2, p_3, p_4, p_Macro, __VA_ARGS__)
#define CHOOSE_HM_INIT_MACRO(p_1, p_2, p_3, p_4, p_Macro, ...)                 \
    CHOOSE_MACRO4(p_1, p_2, p_3, p_4, p_Macro, __VA_ARGS__)

#define HM_ELEMENT(p_Name, p_EType)                                            \
    struct {                                                                   \
        uint8_t m_HasData : 1;                                                 \
        p_EType m_Value;                                                       \
    } p_Name

#define S_HASHMAP_COMMONS(p_KType, p_EType, p_MaxSize)                         \
    uint8_t m_Block : 1;                                                       \
    pthread_spinlock_t m_Lock;                                                 \
    size_t m_MaxSize;                                                          \
    size_t m_CurrSize;                                                         \
    uint64_t (*m_HashFn)(p_KType);                                             \
    uint8_t                                                                    \
        m_Occ[p_MaxSize]; /* Used for checking if position at index is free */ \
    struct {                                                                   \
        p_KType m_Key;                                                         \
        p_EType m_El;                                                          \
    } m_Data[p_MaxSize]

#define HASHMAP2(p_Name, p_KType, p_EType)                                     \
    struct {                                                                   \
        S_HASHMAP_COMMONS(p_KType, p_EType, HM_MAX_SIZE);                      \
    } p_Name

#define HASHMAP3(p_Name, p_KType, p_EType, p_MaxSize)                          \
    struct {                                                                   \
        S_HASHMAP_COMMONS(p_KType, p_EType, p_MaxSize);                        \
    } p_Name

#define S__INIT_HASHMAP_DEFAULT(p_Name, p_HashFn, p_MaxSize)                   \
    do {                                                                       \
        uint32_t hmSize = sizeof(p_Name.m_Data) / sizeof(p_Name.m_Data[0]);    \
                                                                               \
        if (hmSize < p_MaxSize) {                                              \
            fprintf(stderr,                                                    \
                    "[%s!line %d]: Cannot init hashmap because of a mismatch " \
                    "of size. ",                                               \
                    #p_Name, __LINE__);                                        \
            fprintf(stderr, "Got %u. Expecting a maximum value of %u.\n",      \
                    p_MaxSize, hmSize);                                        \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        p_Name.m_Block = HM_UNBLOCKED;                                         \
        p_Name.m_CurrSize = 0;                                                 \
        p_Name.m_HashFn = p_HashFn;                                            \
                                                                               \
        pthread_spin_init(&p_Name.m_Lock, PTHREAD_PROCESS_PRIVATE);            \
    } while (0)

#define INIT_HASHMAP_DEFAULT(p_Name, p_HashFn)                                 \
    do {                                                                       \
        S__INIT_HASHMAP_DEFAULT(p_Name, p_HashFn, HM_MAX_SIZE);                \
        p_Name.m_MaxSize = HM_MAX_SIZE;                                        \
    } while (0)

#define INIT_HASHMAP_NON_BLOCK(p_Name, p_HashFn, p_MaxSize)                    \
    do {                                                                       \
        S__INIT_HASHMAP_DEFAULT(p_Name, p_HashFn, p_MaxSize);                  \
        p_Name.m_MaxSize = p_MaxSize;                                          \
    } while (0)

#define INIT_HASHMAP_BLOCK(p_Name, p_HashFn, p_MaxSize, p_Block)               \
    do {                                                                       \
        INIT_HASHMAP_NON_BLOCK(p_Name, p_HashFn, p_MaxSize);                   \
        p_Name.m_Block = p_Block;                                              \
    } while (0)

#define HASHMAP_PUT(p_Name, p_Key, p_El)                                       \
    do {                                                                       \
        if (p_Name.m_HashFn == NULL) {                                         \
            fprintf(stderr, "[%s]: Hash function is not initialized.\n",       \
                    #p_Name);                                                  \
            exit(1);                                                           \
        }                                                                      \
        uint64_t idx = p_Name.m_HashFn(p_Key) % p_Name.m_MaxSize;              \
                                                                               \
        pthread_spin_lock(&p_Name.m_Lock);                                     \
        if (p_Name.m_Occ[idx] == HM_OCCUPIED) {                                \
            fprintf(stderr,                                                    \
                    "Element position is occupied. Exiting program.\n");       \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        p_Name.m_CurrSize++;                                                   \
        p_Name.m_Data[idx].m_Key = p_Key;                                      \
        p_Name.m_Data[idx].m_El = p_El;                                        \
        p_Name.m_Occ[idx] = HM_OCCUPIED;                                       \
        pthread_spin_unlock(&p_Name.m_Lock);                                   \
    } while (0)

#define HASHMAP_INSERT(p_Name, p_Key, p_El)                                    \
    do {                                                                       \
        if (p_Name.m_HashFn == NULL) {                                         \
            fprintf(stderr, "[%s]: Hash function is not initialized.\n",       \
                    #p_Name);                                                  \
            exit(1);                                                           \
        }                                                                      \
        uint64_t idx = p_Name.m_HashFn(p_Key) % p_Name.m_MaxSize;              \
        while (p_Name.m_Block == HM_BLOCKED) {                                 \
            pthread_spin_lock(&p_Name.m_Lock);                                 \
            if (p_Name.m_Occ[idx] == HM_NOT_OCCUPIED) {                        \
                pthread_spin_unlock(&p_Name.m_Lock);                           \
                break;                                                         \
            }                                                                  \
            pthread_spin_unlock(&p_Name.m_Lock);                               \
        }                                                                      \
        if (p_Name.m_Occ[idx] != HM_NOT_OCCUPIED) {                            \
            fprintf(stderr,                                                    \
                    "Element position is occupied. Exiting program.\n");       \
            exit(1);                                                           \
        }                                                                      \
        pthread_spin_lock(&p_Name.m_Lock);                                     \
        p_Name.m_CurrSize++;                                                   \
        p_Name.m_Data[idx].m_Key = p_Key;                                      \
        p_Name.m_Data[idx].m_El = p_El;                                        \
        p_Name.m_Occ[idx] = HM_OCCUPIED;                                       \
        pthread_spin_unlock(&p_Name.m_Lock);                                   \
    } while (0)

#define HASHMAP_GET(p_Name, p_Key, p_Result)                                   \
    do {                                                                       \
        uint64_t idx = p_Name.m_HashFn(p_Key) % p_Name.m_MaxSize;              \
                                                                               \
        pthread_spin_lock(&p_Name.m_Lock);                                     \
        if (p_Name.m_Occ[idx] == HM_NOT_OCCUPIED) {                            \
            p_Result.m_HasData = 0;                                            \
            pthread_spin_unlock(&p_Name.m_Lock);                               \
            break;                                                             \
        }                                                                      \
                                                                               \
        p_Result.m_HasData = 1;                                                \
        p_Result.m_Value = p_Name.m_Data[idx].m_El;                            \
        pthread_spin_unlock(&p_Name.m_Lock);                                   \
    } while (0)

#define HASHMAP_REMOVE(p_Name, p_Key, p_Result)                                \
    do {                                                                       \
        uint64_t idx = p_Name.m_HashFn(p_Key) % p_Name.m_MaxSize;              \
                                                                               \
        pthread_spin_lock(&p_Name.m_Lock);                                     \
        if (p_Name.m_Occ[idx] == HM_NOT_OCCUPIED) {                            \
            p_Result.m_HasData = 0;                                            \
            pthread_spin_unlock(&p_Name.m_Lock);                               \
            break;                                                             \
        }                                                                      \
                                                                               \
        p_Result.m_HasData = 1;                                                \
        p_Result.m_Value = p_Name.m_Data[idx].m_El;                            \
        p_Name.m_Occ[idx] = HM_NOT_OCCUPIED;                                   \
        pthread_spin_unlock(&p_Name.m_Lock);                                   \
    } while (0)

#define HASHMAP(...)                                                           \
    CHOOSE_HASHMAP_MACRO(__VA_ARGS__, HASHMAP3, HASHMAP2)(__VA_ARGS__)
#define INIT_HASHMAP(...)                                                      \
    CHOOSE_HM_INIT_MACRO(__VA_ARGS__, INIT_HASHMAP_BLOCK,                      \
                         INIT_HASHMAP_NON_BLOCK, INIT_HASHMAP_DEFAULT)         \
    (__VA_ARGS__)

#endif // DSP_HASHMAP_H
