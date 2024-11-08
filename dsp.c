#include "dsp.h"

#include "protocol.h"
#include "dsp-service.h"
#include "dsp-client.h"

#include "log/log.h"

// static int value = 0;
static char *ptr = NULL;
static struct InstallSharedData *ishdata = NULL;

void hello() {
    fprintf(stdout, "Shared library: Hello world!\n");
}

void increment() {
    if (ptr == NULL) {
        fprintf(stderr, "Operation could not be performed!\n");
        return;
    }

    // fprintf(stdout, "ptr after mmap: %p\n", ptr);
    // fprintf(stdout, "Before increment: %d\n", *(int *)ptr);
    fprintf(stdout, "Calling %s\n", __func__);
    (*(int *)ptr)++;
    fprintf(stdout, "After increment: %d\n", *(int *)ptr);
    fprintf(stdout, "Return %s\n", __func__);
}

int getValue() {
    // fprintf(stdout, "Shared ptr is %p\n", (int *)ptr);
    return *(int *)ptr;
}

// void install() {
//     fprintf(stdout, "ptr before mmap: %p\n", ptr);
//     int shmfd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, 0600);
//     assert(shmfd >= 0);
//     ftruncate(shmfd, 4096);
//     fprintf(stdout, "Shared object ID: %d\n", shmfd);
//     ptr = mmap(NULL, 4096, PROT_WRITE | PROT_READ, MAP_SHARED, shmfd, 0);
//     fprintf(stdout, "ptr after mmap: %p\n", ptr);
//     close(shmfd);
// }

void initService() {
    int rc;

    int ishdfd = shm_open(INSTALL_SHD, O_CREAT | O_EXCL | O_RDWR, 600);
    if (ishdfd != EEXIST) {
        assert(ishdfd >= 0);
    }

    if (ishdfd == EEXIST) {
        return;
    }

    ishdata = mmap(0, sizeof(struct InstallSharedData), PROT_READ | PROT_WRITE, MAP_SHARED, ishdfd, 0);
    assert(ishdata != MAP_FAILED);

    // pthread_spin_init(&ishdata->m_InstallMZoneLk);
    // rc = pthread_mutex_init(&ishdata->m_InstallMZoneMx, PTHREAD_PROCESS_SHARED);
    // assert(rc == 0);

    rc = pthread_spin_init(&ishdata->m_InstallMZoneLk, PTHREAD_PROCESS_SHARED);
    assert(rc == 0);
}

void install() {
    int rc;
    int ishmfd = shm_open(INSTALL_MZONE, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (ishmfd != EEXIST) {
        assert(ishmfd >= 0);
    }

    if (ishmfd == EEXIST) {
        goto find_free_zone;
    }

    // Prepare space to allow installation for SERVICES_NUMBER services at most
    // We need a bit map for fast iteration
    // Get the number of bytes for the bit map
    // The information that we need is an array of pointers to the information that we need
    uint8_t bytesnr = SERVICES_NUMBER >> 3;
    rc = ftruncate(ishmfd, bytesnr + (SERVICES_NUMBER * sizeof(char *)));
    assert(rc == 0);

find_free_zone:
    char *imzone = mmap(0, bytesnr + (SERVICES_NUMBER * sizeof(char *)), PROT_READ | PROT_WRITE, MAP_SHARED, ishmfd, 0);
    assert(imzone != MAP_FAILED);
}

void uninstall() {
    if (ptr) {
        munmap(ptr, 8192);
    }
}

void connect() {
    fprintf(stdout, "Connecting...\n");
    int shmfd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, 0600);
    assert(shmfd >= 0);
    ftruncate(shmfd, 4096);
    fprintf(stdout, "Shared object ID: %d\n", shmfd);
    ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(ptr != NULL);
    close(shmfd);
    fprintf(stdout, "Connected.\n");
}
