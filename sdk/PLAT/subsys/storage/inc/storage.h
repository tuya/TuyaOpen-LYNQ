/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    storage.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_STORAGE_H
#define  SUBSYS_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef FEATURE_SUBSYS_LFSEX_ENABLE
#include "lfsex_port.h"
#else
#include "lfs_port.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
#include "sdcard.h"
#endif
#include "iniparse.h"


typedef enum
{
    FLASH_PARTITION_C = 0,      /* Internal Flash (LFS) */
    FLASH_PARTITION_D = 1,      /* External Flash (LFS) */
    FLASH_PARTITION_E = 2,      /* sdcard (FATFS) */
    FLASH_PARTITION_R = 10,     /* ROM or RAM */
    FLASH_PARTITION_X = 20,     /* External Flash */
} FlashPartitionT;

typedef struct
{
    uint8_t  partition;
    void    *fd;
} FileHeadT;

typedef struct
{
    char     prefix[4];
    uint32_t address;
    uint32_t size;
} VirtualFileT;


void        subStorageInit(void);
bool        pathPrefixIsValid(char *path);
int32_t     fsDirOpen(lfs_dir_t *dir, char *path);
int32_t     fsDirClose(lfs_dir_t *dir);
lfs_ssize_t fsDirRead(lfs_dir_t *dir, struct lfs_info *info);
int32_t     fsStatFs(lfs_status_t *status);

int     file_remove(const char *pathname);
FILE   *file_fopen(const char *pathname, const char *mode);
int     file_fclose(FILE *stream);
size_t  file_fread(void *ptr, size_t size, size_t nitems, FILE *stream);
size_t  file_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int     file_fseek(FILE *stream, long offset, int whence);
long    file_ftell(FILE *stream);
void    file_rewind(FILE *stream);
int     file_fstat(int fildes, struct stat *buf);
int     file_truncate(int fildes, off_t length);

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_STORAGE_H */