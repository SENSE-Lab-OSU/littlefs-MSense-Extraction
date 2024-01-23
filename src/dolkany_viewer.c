#include <dokan/dokan.h>
#include <stdio.h>
#include "operations.h"
#include "context.h"
#include "dolkany_viewer.h"


void Dokany_Init() {




}


void dokany_activate(uint32_t unit_size, uint32_t block_size, ULONG dolkan_debug_options, LPCWSTR mount_point) {

    DOKAN_OPTIONS dokan_options = {
        // Version
        131,
        // ThreadCount
        1,
        // Options
        dolkan_debug_options,
        // GlobalContext
        (ULONG64)&fs_context,
        // MountPoint
        mount_point,
        // UNCName
        NULL,
        // Timeout
        100,
        // AllocationUnitSize
        unit_size,
        // SectorSize
        block_size
    };

    DOKAN_OPERATIONS dokan_operations = {
        LFS_ZwCreateFile,
        LFS_Cleanup,
        LFS_CloseFile,
        LFS_ReadFile,
        LFS_WriteFile,
        LFS_FlushFileBuffers,
        LFS_GetFileInformation,
        LFS_FindFiles,
        LFS_FindFilesWithPattern,
        LFS_SetFileAttributes,
        LFS_SetFileTime,
        LFS_DeleteFile,
        LFS_DeleteDirectory,
        LFS_MoveFile,
        LFS_SetEndOfFile,
        LFS_SetAllocationSize,
        LFS_LockFile,
        LFS_UnlockFile,
        LFS_GetDiskFreeSpace,
        LFS_GetVolumeInformation,
        LFS_Mounted,
        LFS_Unmounted,
        LFS_GetFileSecurity,
        LFS_SetFileSecurity,
        LFS_FindStreams
    };

    int status = DokanMain(&dokan_options, &dokan_operations);
    switch (status) {
    case DOKAN_SUCCESS:
        fprintf(stderr, "Success\n");
        break;
    case DOKAN_ERROR:
        fprintf(stderr, "Error\n");
        break;
    case DOKAN_DRIVE_LETTER_ERROR:
        fprintf(stderr, "Bad Drive letter\n");
        break;
    case DOKAN_DRIVER_INSTALL_ERROR:
        fprintf(stderr, "Can't install driver\n");
        break;
    case DOKAN_START_ERROR:
        fprintf(stderr, "Driver something wrong\n");
        break;
    case DOKAN_MOUNT_ERROR:
        fprintf(stderr, "Can't assign a drive letter\n");
        break;
    case DOKAN_MOUNT_POINT_ERROR:
        fprintf(stderr, "Mount point error\n");
        break;
    case DOKAN_VERSION_ERROR:
        fprintf(stderr, "Version error\n");
        break;
    default:
        fprintf(stderr, "Unknown error: %d\n", status);
        break;
    }


    return status;
}


