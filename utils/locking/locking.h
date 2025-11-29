#ifndef AQUA_LOCKING_H
#define AQUA_LOCKING_H

#if defined(__linux__)
#include <pthread.h>

typedef pthread_mutex_t aqua_mutex_t;
typedef pthread_cond_t aqua_cond_t;
typedef pthread_spinlock_t aqua_spinlock_t;

#elif defined(_WIN32)
#include <Windows.h>

typedef HANDLE aqua_mutex_t;
typedef HANDLE aqua_cond_t;

// For _WIN32 the spinlock defaults to mutex because there is not shared
// spinlock yet (CRITICAL_SECTION works only inside the same process)
typedef HANDLE aqua_spinlock_t;

#else
#error "Platform not supported by AQUA"
#endif

#endif // AQUA_LOCKING_H
