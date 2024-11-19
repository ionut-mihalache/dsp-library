#include "dsp.h"
#include "dsp-client.h"
#include "protocol.h"

#include "log/log.h"

#include <sys/ipc.h>
#include <sys/shm.h>

static char *ptr = NULL;

void connect() {
    key_t key;
    int shmId;

    fprintf(stdout, "Connecting...");
    key = ftok(SHMEM_PATH, 123);
    assert(key > 0);

    shmId = shmget(key, 4096, 0600 | IPC_CREAT);
    assert(shmId > 0);

    ptr = shmat(shmId, NULL, 0);
    assert(ptr != NULL);

    // int shmfd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, 0600);
    // assert(shmfd >= 0);
    // ftruncate(shmfd, 4096);
    // fprintf(stdout, "Shared object ID: %d\n", shmfd);
    // ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    // assert(ptr != NULL);
    // close(shmfd);
    fprintf(stdout, "Connected.\n");
}
