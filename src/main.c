
#include <lfs.h>
#include <stdio.h>
#include "extraction.h"
#include "lfs_bind.h"
#include "context.h"


//#define USE_DOKANY = 0



#ifdef USE_DOKANY
#include <dokan/dokan.h>
#include "dolkany_viewer.h"
#endif

bool enable_browsing = true;





int __cdecl wmain(ULONG argc, PWCHAR argv[])
{
    ULONG debug_options = 0;
    ULONG positional_arg = 0;
    LPCWSTR mount_point = NULL;
    LPCWSTR media = NULL;
    uint32_t unit_size = 512;
    uint32_t block_size = 8192;
    uint8_t format = 0;
    const char* folder = "extracted_folder";
    bool args_entered;


    for (ULONG i = 1; i < argc; i++)
    {
        PWCHAR* arg = argv[i];

        if (wcscmp(arg, L"-d") == 0)
        {
        }
        else if (wcscmp(arg, L"--block-size") == 0)
        {
            block_size = _wtoi(argv[++i]);
        }
        else if (wcscmp(arg, L"--unit-size") == 0)
        {
            unit_size = _wtoi(argv[++i]);
        }
        else if (wcscmp(arg, L"--format") == 0)
        {
            format = 1;
        }
        else
        {
            switch (positional_arg++)
            {
                case 0:
                {
                    // mount point
                    mount_point = (LPCWSTR)arg;
                    break;
                }
                case 1:
                {
                    // media
                    media = (LPCWSTR)arg;
                    break;
                }
            }
        }
    }
    
    #ifdef USE_DOKANY
    args_entered = mount_point == NULL || media == NULL;
    #else
    args_entered = mount_point == NULL;
    #endif
    if (args_entered)
    {
        fprintf(stderr, "Usage: <mount point> <media>\n");
        fprintf(stderr, "   e.g. F: PhysicalDrive2\n");
        printf("no input found, using default MotionSenseDevice....\n");
        PWCHAR mount_string = L"Y:";
        wchar_t media_string[10] = L"G:";
        char DeviceStrings[MAX_PATH];
        char CurrentDevice[MAX_PATH];
        
        GetLogicalDriveStringsA(MAX_PATH, DeviceStrings);
        for (int i = 0; i < 60; i += 4) {
            if (DeviceStrings[i] != (char)0) {
                memcpy(CurrentDevice, &DeviceStrings[i], 3);
                CurrentDevice[3] = '\0';
                printf("drive found: %s ", CurrentDevice);
                int DriveType = GetDriveTypeA(CurrentDevice);
                printf("drive type: %i \n", DriveType);
                //GetDiskFreeSpaceA()
                if (DriveType == 2) {
                    printf("found drive! \n");
                    CurrentDevice[2] = '\0';
                    size_t newsize = strlen(CurrentDevice) + 1;

                    mbstowcs(media_string, CurrentDevice, 1);
                    
                }
            }
        }
        
        
        mount_point = (LPCWSTR)mount_string;
        media = (LPCWSTR)media_string;
        block_size = 4096;
        unit_size = 512;
        
    }

    char media_path[255];
    sprintf(media_path, "\\\\.\\%ls", media);

    HANDLE media_handle =
        CreateFile(media_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (media_handle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Cannot open media file: %ld\n", GetLastError());
        return 2;
    }

    LARGE_INTEGER media_file_size;
    GetFileSizeEx(media_handle, &media_file_size);

    DISK_GEOMETRY disk_geometry;

    DWORD bytes_returned;
    if (DeviceIoControl(media_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &disk_geometry, sizeof(disk_geometry), &bytes_returned, NULL) == 0)
    {
        fprintf(stderr, "Cannot manage media file\n");
        return 3;
    }

    if (disk_geometry.BytesPerSector != unit_size)
    {
        fprintf(stderr, "Media secor side does not match %ld: \n", disk_geometry.BytesPerSector);
        return 4;
    }

    GET_LENGTH_INFORMATION length_information;

    if (DeviceIoControl(media_handle, IOCTL_DISK_GET_LENGTH_INFO , NULL, 0, &length_information, sizeof(length_information), &bytes_returned, NULL) == 0)
    {
        fprintf(stderr, "Get media file size\n");
        return 5;
    }

    uint32_t block_count = length_information.Length.QuadPart / block_size;

    lfs_t lfs;

    fs_context.lfs = &lfs;
    fs_context.media_handle = media_handle;
    fs_context.unit_size = unit_size;

    struct lfs_config lfs_config = {
        // context
        &fs_context,
        // read
        lfs_bind_read,
        // prog
        lfs_bind_prog,
        // erase
        lfs_bind_erase,
        // sync
        lfs_bind_sync,
        // read_size
        unit_size,
        // prog_size
        unit_size,
        // block_size
        block_size,
        // block_count
        block_count,
        // block_cycles
        1000,
        // cache_size
        unit_size,
        // lookahead_size
        128,
        // read_buffer
        NULL,
        // prog_buffer
        NULL,
        // lookahead_buffer
        NULL,
        // name_max
        0,
        // file_max
        0,
        // attr_max
        0
    };

    if (format)
    {
        int lfs_format_result = lfs_format(&lfs, &lfs_config);
        if (lfs_format_result != LFS_ERR_OK)
        {
            fprintf(stderr, "Failed to format littlefs\n");
            return lfs_format_result;
        }
    }
    printf("mounting lfs...\n");
    int lfs_mount_result = lfs_mount(&lfs, &lfs_config);
    if (lfs_mount_result != LFS_ERR_OK)
    {
        fprintf(stderr, "Failed to mount littlefs %d\n", lfs_mount_result);
        return lfs_mount_result;
    }
    printf("file system mounted!\n");
    
    
    copy_all_files3(&lfs, folder);
    //file_create_test(&lfs);



    #if USE_DOKANY
    
    if (enable_browsing) {
        dokany_activate(unit_size, block_size, debug_options, mount_point);
    }
    #endif
    
    printf("unmounting...\n");
    lfs_unmount(&lfs);
    printf("completed file copying!\n");
    Sleep(8000);
	
}