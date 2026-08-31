#ifndef __LFS_EX_H__
#define __LFS_EX_H__

#include "lfs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint32_t total_block;
    uint32_t block_used;
    uint32_t block_size;
} littlefs_status_t;

// Initialize
int LITTLEFS_Init(void);

// Deinit
void LITTLEFS_Deinit(void);

// Wrapper of lfs_format
int LITTLEFS_Format(void);

// Wrapper of lfs_stat
int LITTLEFS_Stat(const char *path, struct lfs_info *info);

// Wrapper of lfs_remove
int LITTLEFS_Remove(const char *path);

// Wrapper of lfs_file_open
int LITTLEFS_FileOpen( lfs_file_t *file, const char *path, int flags);

// Wrapper of lfs_file_close
int LITTLEFS_FileClose(lfs_file_t *file);

// Wrapper of lfs_file_read
lfs_ssize_t LITTLEFS_FileRead(lfs_file_t *file, void *buffer, lfs_size_t size);

// Wrapper of lfs_file_write
lfs_ssize_t LITTLEFS_FileWrite(lfs_file_t *file, const void *buffer, lfs_size_t size);

// Wrapper of lfs_file_seek
lfs_soff_t LITTLEFS_FileSeek(lfs_file_t *file, lfs_soff_t off, int whence);

// Wrapper of lfs_file_truncate
int LITTLEFS_FileTruncate(lfs_file_t *file, lfs_off_t size);

// Wrapper of lfs_file_tell
lfs_soff_t LITTLEFS_FileTell(lfs_file_t *file);

// Wrapper of lfs_file_rewind
int LITTLEFS_FileRewind(lfs_file_t *file);

// Wrapper of lfs_file_size
lfs_soff_t LITTLEFS_FileSize(lfs_file_t *file);

// Wrapper of lfs_mkdir
int LITTLEFS_Mkdir(const char *path);

// Wrapper of lfs_dir_open
int LITTLEFS_DirOpen(lfs_dir_t *dir, const char *path);

// Wrapper of lfs_dir_close
int LITTLEFS_DirClose(lfs_dir_t *dir);

// Wrapper of lfs_dir_read
int LITTLEFS_DirRead(lfs_dir_t *dir, struct lfs_info *info);


// Get fs status
int LITTLEFS_Statfs(littlefs_status_t *status);

// Wrapper of lfs_file_sync
int LITTLEFS_FileSync(lfs_file_t *file);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
