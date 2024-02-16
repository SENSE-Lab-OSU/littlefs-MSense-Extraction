#include "extraction.h"
#include <windows.h>
#include <lfs.h>
#include <stdio.h>





// Define the read/write buffer size
#define BUFFER_SIZE 20056

// Define the source and destination file system structures
lfs_t src_fs;
lfs_t dst_fs;


bool sort_data = true;

// A function that copies all files from one file system to another
int copy_all_files(lfs_t* src, lfs_t* dst) {
    // Create a buffer to store the file data
    char buffer[BUFFER_SIZE];

    // Create a directory structure to iterate over the files
    lfs_dir_t dir;

    // Open the root directory of the source file system
    int err = lfs_dir_open(src, &dir, "/");
    if (err) {
        printf("Failed to open source directory\n");
        return err;
    }

    // Loop over all the files in the source directory
    while (true) {
        // Read an entry from the directory
        struct lfs_info info;
        err = lfs_dir_read(src, &dir, &info);
        if (err < 0) {
            printf("Failed to read source directory\n");
            return err;
        }

        // If the end of the directory is reached, break the loop
        if (err == 0) {
            break;
        }

        // If the entry is a file, copy it to the destination file system
        if (info.type == LFS_TYPE_REG) {
            // Open the file in the source file system
            lfs_file_t src_file;
            err = lfs_file_open(src, &src_file, info.name, LFS_O_RDONLY);
            if (err) {
                printf("Failed to open source file %s\n", info.name);
                return err;
            }

            // Open the file in the destination file system
            lfs_file_t dst_file;
            err = lfs_file_open(dst, &dst_file, info.name, LFS_O_WRONLY | LFS_O_CREAT);
            if (err) {
                printf("Failed to open destination file %s\n", info.name);
                return err;
            }

            // Loop over the file data and copy it to the buffer
            while (true) {
                // Read a chunk of data from the source file
                lfs_ssize_t nread = lfs_file_read(src, &src_file, buffer, BUFFER_SIZE);
                if (nread < 0) {
                    printf("Failed to read source file %s\n", info.name);
                    return nread;
                }

                // If the end of the file is reached, break the loop
                if (nread == 0) {
                    break;
                }

                // Write the chunk of data to the destination file
                lfs_ssize_t nwrite = lfs_file_write(dst, &dst_file, buffer, nread);
                if (nwrite < 0) {
                    printf("Failed to write destination file %s\n", info.name);
                    return nwrite;
                }
            }

            // Close both files
            err = lfs_file_close(src, &src_file);
            if (err) {
                printf("Failed to close source file %s\n", info.name);
                return err;
            }

            err = lfs_file_close(dst, &dst_file);
            if (err) {
                printf("Failed to close destination file %s\n", info.name);
                return err;
            }

            // Print a message to indicate the file is copied
            printf("Copied file %s\n", info.name);
        }
    }

    // Close the source directory
    err = lfs_dir_close(src, &dir);
    if (err) {
        printf("Failed to close source directory\n");
        return err;
    }

    // Return success
    return 0;
}



// A function that copies all files from a LittleFS file system to a Windows folder
int copy_all_files2(lfs_t* src, char* dst_path) {
    // Create a buffer to store the file data
    char buffer[BUFFER_SIZE];

    // Create a directory structure to iterate over the files
    lfs_dir_t dir;

    // Open the root directory of the source file system
    int err = lfs_dir_open(src, &dir, "/");
    if (err) {
        printf("Failed to open source directory\n");
        return err;
    }

    // Loop over all the files in the source directory
    while (true) {
        // Read an entry from the directory
        struct lfs_info info;
        err = lfs_dir_read(src, &dir, &info);
        if (err < 0) {
            printf("Failed to read source directory\n");
            return err;
        }

        // If the end of the directory is reached, break the loop
        if (err == 0) {
            break;
        }

        // If the entry is a file, copy it to the destination folder
        if (info.type == LFS_TYPE_REG) {
            // Open the file in the source file system
            lfs_file_t src_file;
            err = lfs_file_open(src, &src_file, info.name, LFS_O_RDONLY);
            if (err) {
                printf("Failed to open source file %s\n", info.name);
                return err;
            }

            // Construct the full name of the destination file
            char dst_file[MAX_PATH];
            sprintf(dst_file, "%s\\%s", dst_path, info.name);

            // Open the file in the destination folder
            HANDLE dst_handle = CreateFile(dst_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (dst_handle == INVALID_HANDLE_VALUE) {
                printf("Failed to open destination file %s\n", dst_file);
                printf("Error code: %d\n", GetLastError());
                return -1;
            }

            // Loop over the file data and copy it to the buffer
            while (true) {
                // Read a chunk of data from the source file
                lfs_ssize_t nread = lfs_file_read(src, &src_file, buffer, BUFFER_SIZE);
                if (nread < 0) {
                    printf("Failed to read source file %s\n", info.name);
                    return nread;
                }

                // If the end of the file is reached, break the loop
                if (nread == 0) {
                    break;
                }

                // Write the chunk of data to the destination file
                DWORD nwrite;
                BOOL success = WriteFile(dst_handle, buffer, nread, &nwrite, NULL);
                
                if (!success || nwrite != nread) {
                    printf("Failed to write destination file %s\n", dst_file);
                    printf("Error code: %d\n", GetLastError());
                    return -1;
                }
            }

            // Close both files
            err = lfs_file_close(src, &src_file);
            if (err) {
                printf("Failed to close source file %s\n", info.name);
                return err;
            }

            bool success = CloseHandle(dst_handle);
            if (!success) {
                printf("Failed to close destination file %s\n", dst_file);
                printf("Error code: %d\n", GetLastError());
                return -1;
            }

            // Print a message to indicate the file is copied
            printf("Copied file %s\n", info.name);
        }
    }

    // Close the source directory
    err = lfs_dir_close(src, &dir);
    if (err) {
        printf("Failed to close source directory\n");
        return err;
    }

    // Return success
    return 0;
}



// A function that copies all files from a LittleFS file system to a folder
int copy_all_files3(lfs_t* src, char* dst_path) {
    // Create a buffer to store the file data
    char buffer[BUFFER_SIZE];

    // Create a directory structure to iterate over the files
    lfs_dir_t dir;
   
    // Open the root directory of the source file system
    int err = lfs_dir_open(src, &dir, "/");
    if (err) {
        printf("Failed to open source directory\n");
        return err;
    }

    CreateDirectory(dst_path, NULL);
    // Loop over all the files in the source directory
    while (true) {
        // Read an entry from the directory
        struct lfs_info info;
        err = lfs_dir_read(src, &dir, &info);
        if (err < 0) {
            printf("Failed to read source directory\n");
            return err;
        }

        // If the end of the directory is reached, break the loop
        if (err == 0) {
            break;
        }

        // If the entry is a file, copy it to the destination folder
        if (info.type == LFS_TYPE_REG) {
            // Open the file in the source file system
            lfs_file_t src_file;
            err = lfs_file_open(src, &src_file, info.name, LFS_O_RDONLY);
            if (err) {
                printf("Failed to open source file %s\n", info.name);
                return err;
            }

            // Construct the full name of the destination file
            char dst_file[MAX_PATH];
            sprintf(dst_file, "%s/%s", dst_path, info.name);

            // Open the file in the destination folder
            FILE* dst_fp = fopen(dst_file, "wb");
            if (dst_fp == NULL) {
                printf("Failed to open destination file %s\n", dst_file);
                printf("Error code: %d\n", errno);
                return -1;
            }
            // Loop over the file data and copy it to the buffer
            while (true) {
                // Read a chunk of data from the source file
                lfs_ssize_t nread = lfs_file_read(src, &src_file, buffer, BUFFER_SIZE);
                if (nread < 0) {
                    printf("Failed to read source file %s\n", info.name);
                    return nread;
                }

                // If the end of the file is reached, break the loop
                if (nread == 0) {
                    break;
                }

                // Write the chunk of data to the destination file
                size_t nwrite = fwrite(buffer, 1, nread, dst_fp);
                if (nwrite != nread) {
                    printf("Failed to write destination file %s\n", dst_file);
                    printf("Error code: %d\n", errno);
                    return -1;
                }
            }

            // Close both files
            err = lfs_file_close(src, &src_file);
            if (err) {
                printf("Failed to close source file %s\n", info.name);
                return err;
            }

            int success = fclose(dst_fp);
            if (success != 0) {
                printf("Failed to close destination file %s\n", dst_file);
                printf("Error code: %d\n", errno);
                return -1;
            }

            // Print a message to indicate the file is copied
            printf("Copied file %s\n", info.name);
        }
    }

    // Close the source directory
    err = lfs_dir_close(src, &dir);
    if (err) {
        printf("Failed to close source directory\n");
        return err;
    }

    // Return success
    return 0;
}



void file_create_test(lfs_t* lfs) {

    lfs_file_t test_file;
    const char* path = "/lfs";
    lfs_file_open(&lfs, &test_file, path, LFS_O_CREAT | LFS_O_RDWR);
    char a = 'a';
    lfs_file_write(&lfs, &test_file, &a, sizeof(a));
    lfs_file_close(&lfs, &test_file);
    printf("done writing test file!");
}
