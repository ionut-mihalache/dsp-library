// SPDX-License-Identifier: LGPL-2.1-or-later

#include <pthread.h>
#include <semaphore.h>

#include "aqua-sync.h"
#include "aqua-types.h"
#include "platform.h"

_Static_assert(sizeof(pthread_mutex_t) <= AQUA_MUTEX_MEM_SIZE,
               "AQUA_MUTEX_MEM_SIZE too small");

_Static_assert(sizeof(pthread_cond_t) <= AQUA_COND_MEM_SIZE,
               "AQUA_COND_MEM_SIZE too small");

_Static_assert(sizeof(sem_t) <= AQUA_SEM_MEM_SIZE,
               "AQUA_SEM_MEM_SIZE too small");

static aqua_void_t createMutex(aqua_mutex_t *p_Mutex, const char *p_Name) {
    (void)p_Name;

    pthread_mutex_t *mutex = (pthread_mutex_t *)p_Mutex->memory;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(mutex, &attr);

    pthread_mutexattr_destroy(&attr);
}

static aqua_void_t createCond(aqua_cond_t *p_Cond, const char *p_Name) {
    (void)p_Name;

    pthread_cond_t *cond = (pthread_cond_t *)p_Cond->memory;

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);

    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(cond, &condAttr);

    pthread_condattr_destroy(&condAttr);
}

static aqua_void_t createSemaphore(aqua_sem_t *p_Sem, const char *p_Name) {
    (void)p_Sem;
    (void)p_Name;
}

static aqua_void_t destroyMutex(aqua_mutex_t *p_Mutex) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)p_Mutex->memory;

    pthread_mutex_destroy(mutex);
}

static aqua_void_t destroyCond(aqua_cond_t *p_Cond) {
    pthread_cond_t *cond = (pthread_cond_t *)p_Cond->memory;

    pthread_cond_destroy(cond);
}

static aqua_void_t destroySemaphore(aqua_sem_t *p_Sem) {
    sem_t *sem = (sem_t *)p_Sem->memory;

    sem_destroy(sem);
}

static aqua_void_t mutexLock(aqua_mutex_t *p_Mutex) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)p_Mutex->memory;

    pthread_mutex_lock(mutex);
}

static aqua_void_t mutexUnlock(aqua_mutex_t *p_Mutex) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)p_Mutex->memory;

    pthread_mutex_unlock(mutex);
}

static aqua_void_t condWait(aqua_cond_t *p_Cond, aqua_mutex_t *p_Mutex) {
    pthread_cond_t *cond = (pthread_cond_t *)p_Cond->memory;
    pthread_mutex_t *mutex = (pthread_mutex_t *)p_Mutex->memory;

    pthread_cond_wait(cond, mutex);
}

static aqua_void_t condBroadcast(aqua_cond_t *p_Cond) {
    pthread_cond_t *cond = (pthread_cond_t *)p_Cond->memory;

    pthread_cond_broadcast(cond);
}

struct AQUA_Sync Sync = {
    .createMutex = createMutex,
    .createCond = createCond,
    .createSemaphore = createSemaphore,
    .destroyMutex = destroyMutex,
    .destroyCond = destroyCond,
    .destroySemaphore = destroySemaphore,

    .mutexLock = mutexLock,
    .mutexUnlock = mutexUnlock,

    .condWait = condWait,
    .condBroadcast = condBroadcast,
};
