#include "commons.h"

#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>

#if defined(__linux__)
#include <errno.h>
#include <sys/mman.h>
#endif

#if defined(_WIN32)
#pragma comment(lib, "Advapi32.lib")

#include <Windows.h>
#include <AclAPI.h>
#endif

#include "dsp.h"
#include "macros.h"
#include "system-values.h"

#if defined(_WIN32)
static inline aqua_void_t s_ModeToPerms(aqua_mode_t *ownerAccess,
                                        aqua_mode_t *groupAccess,
                                        aqua_mode_t *otherAccess,
                                        aqua_mode_t mode);
#endif

aqua_file_handle createShmObject(const char *p_Name, int p_Oflag,
                                 aqua_mode_t p_Mode, aqua_object_size_t p_Size,
                                 uint8_t p_Unlink) {
    aqua_file_handle handle;

#if defined(__linux__)
    int rc;
    uint8_t shouldTruncate = true;

    int oldMask = umask(0);

    if (p_Unlink) {
        shm_unlink(p_Name); // TODO: This should not happen all the time.
    }

    handle = shm_open(p_Name, O_CREAT | O_EXCL | p_Oflag, p_Mode);
    if (handle < 0) {
        if (errno == EEXIST) {
            shouldTruncate = false;
            handle = shm_open(p_Name, p_Oflag, p_Mode);
            goto end;
            // DIE(handle < 0, "Could not open shared memory object");
        } else {
            // DIE(handle < 0, "Could not open shared memory object");
            goto end;
        }
    }

    if (!shouldTruncate) {
        goto end;
    }

    // Prepare space to allow installation for SERVICES_NUMBER services at most
    // We need a bit map for fast iteration
    // Get the number of bytes for the bit map
    // The information that we need is an array of pointers to the information
    // that we need
    rc = ftruncate(handle, p_Size);
    DIE(rc != 0, "Could not truncate shared memory object");

    umask(oldMask);

    DIE(handle < 0, "Could not create shared memory object");
end:
#endif

#if defined(_WIN32)
    // TODO: This is just to remove unused. Should be removed for good
    // EXPLICIT_ACCESS ea[3]; // for owner, group and others
    // aqua_mode_t ownerAccess, groupAccess, otherAccess;
    // PACL acl;
    // SECURITY_DESCRIPTOR *sd;
    // SECURITY_ATTRIBUTES sa;

    // s_ModeToPerms(&ownerAccess, &groupAccess, &otherAccess, p_Mode);

    // ZeroMemory(ea, sizeof(ea));

    // ea[0].grfAccessPermissions = ownerAccess;
    // ea[0].grfAccessMode = GRANT_ACCESS;
    // ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    // ea[0].Trustee.ptstrName = "OWNER RIGHTS";

    // ea[1].grfAccessPermissions = groupAccess;
    // ea[1].grfAccessMode = GRANT_ACCESS;
    // ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    // ea[1].Trustee.ptstrName = "Authenticated Users";

    // ea[2].grfAccessPermissions = otherAccess;
    // ea[2].grfAccessMode = GRANT_ACCESS;
    // ea[2].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    // ea[2].Trustee.ptstrName = (LPSTR)WinWorldSid;

    // acl = NULL;
    // SetEntriesInAcl(3, ea, NULL, &acl);

    // sd =
    //     (SECURITY_DESCRIPTOR *)LocalAlloc(LPTR,
    //     SECURITY_DESCRIPTOR_MIN_LENGTH);
    // InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    // SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE);

    // sa.nLength = sizeof(sa);
    // sa.lpSecurityDescriptor = sd;
    // sa.bInheritHandle = FALSE;

    handle = CreateFileMapping(INVALID_HANDLE_VALUE, // use paging file
                               NULL,                 // default security
                               PAGE_READWRITE,       // read/write access
                               0,      // maximum object size (high-order DWORD)
                               p_Size, // maximum object size (low-order DWORD)
                               p_Name); // name of mapping object
    DIE(handle == NULL, "Could not create shared memory object");

    // Just hide unused warnings when using WIN32 API
    (void)p_Unlink;
    (void)p_Oflag;
    (void)p_Mode;
#endif

    return handle;
}

aqua_void_t createQ(aqua_void_t **p_QPtrRes, aqua_size_t p_Size,
                    aqua_prot_t p_Prot, aqua_file_handle p_FileHandle) {
#if defined(__linux__)
    *p_QPtrRes =
        mmap(NULL, p_Size, p_Prot, MAP_SHARED | MAP_POPULATE, p_FileHandle, 0);
    DIE(*p_QPtrRes == MAP_FAILED, "Could not map return queue memory");
#endif

#if defined(_WIN32)
    BOOL bRet;

    *p_QPtrRes = MapViewOfFile(p_FileHandle, // handle to map object
                               p_Prot, 0, 0, p_Size);
    DIE(*p_QPtrRes == NULL, "Could not map return queue memory");

    bRet = VirtualLock(*p_QPtrRes, p_Size);
    DIE(bRet == FALSE, "Could not lock memory in RAM");
#endif
}

aqua_void_t createQSimple(aqua_void_t **p_QPtrRes, aqua_size_t p_Size,
                          aqua_prot_t p_Prot, aqua_file_handle p_FileHandle) {
#if defined(__linux__)
    *p_QPtrRes = mmap(NULL, p_Size, p_Prot, MAP_SHARED, p_FileHandle, 0);
    DIE(*p_QPtrRes == MAP_FAILED, "Could not map memory");
#endif

#if defined(_WIN32)
    BOOL bRet;

    *p_QPtrRes = MapViewOfFile(p_FileHandle, // handle to map object
                               p_Prot, 0, 0, p_Size);
    DIE(*p_QPtrRes == NULL, "Could not map memory");

    bRet = VirtualLock(*p_QPtrRes, p_Size);
    DIE(bRet == FALSE, "Could not lock memory in RAM");
#endif
}

aqua_void_t triggerKernelPageInit(aqua_void_t *p_MemoryAddr, aqua_size_t p_Size,
                                  aqua_prot_t p_Prot) {
    volatile char *accessPtr = (volatile char *)p_MemoryAddr;
    aqua_size_t pageIdx;

    switch (p_Prot) {
    case AQUA_PROT_READ:
        for (aqua_size_t i = 0; i < p_Size; i++) {
            (aqua_void_t) accessPtr[i];
        }
        break;
    case AQUA_PROT_WRITE:
    case AQUA_PROT_READ | AQUA_PROT_WRITE:
        for (pageIdx = 0; pageIdx < p_Size; pageIdx += PAGE_SIZE) {
            accessPtr[pageIdx] = 0;
        }
        break;
    default:
        // In case of any other permission nothing happens for now
        break;
    }
}
#if defined(_WIN32)
static inline aqua_void_t s_ModeToPerms(aqua_mode_t *p_OwnerAccess,
                                        aqua_mode_t *p_GroupAccess,
                                        aqua_mode_t *p_OtherAccess,
                                        aqua_mode_t p_Mode) {
    if (p_Mode & AQUA_S_IRUSR) {
        (*p_OwnerAccess) |= AQUA_PROT_READ;
    }
    if (p_Mode & AQUA_S_IWUSR) {
        (*p_OwnerAccess) |= AQUA_PROT_WRITE;
    }

    if (p_Mode & AQUA_S_IRGRP) {
        (*p_GroupAccess) |= AQUA_PROT_READ;
    }
    if (p_Mode & AQUA_S_IWGRP) {
        (*p_GroupAccess) |= AQUA_PROT_WRITE;
    }

    if (p_Mode & AQUA_S_IROTH) {
        (*p_OtherAccess) |= AQUA_PROT_READ;
    }
    if (p_Mode & AQUA_S_IWOTH) {
        (*p_OtherAccess) |= AQUA_PROT_WRITE;
    }
}
#endif