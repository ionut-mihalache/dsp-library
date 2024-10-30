#include "dsp.h"

// static int value = 0;
static char *ptr = NULL;

void hello() {
    fprintf(stdout, "Hello world!\n");
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

void install() {
    fprintf(stdout, "ptr before mmap: %p\n", ptr);
    int shmfd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, 0600);
    assert(shmfd >= 0);
    ftruncate(shmfd, 4096);
    fprintf(stdout, "Shared object ID: %d\n", shmfd);
    ptr = mmap(NULL, 4096, PROT_WRITE | PROT_READ, MAP_SHARED, shmfd, 0);
    fprintf(stdout, "ptr after mmap: %p\n", ptr);
    close(shmfd);
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
