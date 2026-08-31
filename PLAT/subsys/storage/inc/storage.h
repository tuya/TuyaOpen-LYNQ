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
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "../fatfs/source/ff.h"
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

#ifndef FEATURE_SUBSYS_FATFS_ENABLE
typedef void DIR;
#endif
struct dirent
{
    char d_name[128];
    unsigned char d_type;
};

struct statvfs
{
    unsigned long  f_bsize;    /* Filesystem block size */
    unsigned long  f_frsize;   /* Fragment size */
    fsblkcnt_t     f_blocks;   /* Size of fs in f_frsize units */
    fsblkcnt_t     f_bfree;    /* Number of free blocks */
    fsblkcnt_t     f_bavail;   /* Number of free blocks for unprivileged users */
    fsfilcnt_t     f_files;    /* Number of inodes */
    fsfilcnt_t     f_ffree;    /* Number of free inodes */
    fsfilcnt_t     f_favail;   /* Number of free inodes for unprivileged users */
    unsigned long  f_fsid;     /* Filesystem ID */
    unsigned long  f_flag;     /* Mount flags */
    unsigned long  f_namemax;  /* Maximum filename length */
};


void    subStorageInit(void);
bool    pathPrefixIsValid(char *path);
bool    pathExists(const char *pathname);
int32_t getFsTotalSize(uint8_t partation);
int32_t getFsFreeSize(uint8_t partation);

int     statvfs(const char *path, struct statvfs *buf);

int     remove(const char *pathname);
int     rename(const char *oldpath, const char *newpath);
int     stat(const char *pathname, struct stat *buf);

FILE   *file_fopen(const char *pathname, const char *mode);
int     file_fclose(FILE *stream);
size_t  file_fread(void *ptr, size_t size, size_t nitems, FILE *stream);
size_t  file_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int     file_fseek(FILE *stream, long offset, int whence);
long    file_ftell(FILE *stream);
void    file_rewind(FILE *stream);
int     file_fstat(int fildes, struct stat *buf);
int     file_truncate(int fildes, off_t length);
char   *file_fgets(char *s, int n, FILE *stream);
int     file_fputs(const char *s, FILE *stream);

int            mkdir(const char *pathname, mode_t mode);
DIR           *opendir(const char *);
int            closedir(DIR *);
struct dirent *readdir(DIR *);


#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_STORAGE_H */