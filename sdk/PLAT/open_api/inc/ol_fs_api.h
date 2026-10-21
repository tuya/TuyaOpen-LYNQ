#ifndef __OL_FS_API__
#define __OL_FS_API__

#include "lfs.h"

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef void*   OLFILE;

typedef enum
{
    OL_LFS_OK = 0,
    OL_LFS_ERR_EOF = -1,
    OL_LFS_ERR_PARAM = -2,
    OL_LFS_ERR_READ = -3,
    OL_LFS_ERR_WRITE = -4,
}ol_lfs_errno;


// 存储类型枚举
typedef enum {
    FS_TYPE_INTERNAL = 0,   // 内部 Flash
    FS_TYPE_EXTERNAL = 1    // 外接 SPI Flash
} fs_storage_type_t;

/********************************************
 * file open mode:
 *   "rb"        read only
 *   "rb+"       read and write
 *   "wb"        create, write, truncate
 *   "wb+"       create, write, read, truncate
 *   "ab"        create, write, append
 *   "ab+"       create, write, read, append
 *********************************************/

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to open a file with the mode.
 * PARAMETERS
 *      fileName [IN]:    The file name of the file you want to open
 *      mode     [IN]:    how to open the file
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      OLFILE
 *
 *****************************************************************************/
extern OLFILE ol_fs_open(const char *fileName, const char* mode, fs_storage_type_t storage_type);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to close a file handle.
 * PARAMETERS
 *      fp[IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      =0 :       close file success
 *      <0 :       error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_close(OLFILE fp);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to read data from a opened file.
 * PARAMETERS
 *      buf [OUT]:    a pointer to read buffer
 *      size [IN]:    number of bytes to read
 *      fp   [IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      >=0 :        concrete length of read data
 *      <0  :        error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_read(void *buf, UINT32 size, OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to write data into a file which can be written.
 * PARAMETERS
 *      buf  [IN]:    a pointer to write buffer
 *      size [IN]:    number of bytes to write
 *      fp   [IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      >=0 :         exact length of write data
 *      <0  :         error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_write(void *buf, UINT32 size, OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get the number of bytes that the current position of the file pointer is offset from the beginning of the file.
 * PARAMETERS
 *      fp [IN]:     a file handle give by ol_fs_open
 * RETURN VALUES
 *      >=0 :        the number of byte
 *      <0  :        error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_tell(OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to repositions the offset of the open file.
 * PARAMETERS
 *      fp       [IN]:    a file handle give by ol_fs_open
 *      offset   [IN]:    number of bytes to move the file pointer
 *      seekType [IN]:    The file pointer reference position.
 * RETURN VALUES
 *      =0 :              file seek success
 *      <0 :              error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_seek(OLFILE fp, INT32 offset, UINT8 seekType);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get the file size.
 * PARAMETERS
 *      fp [IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      >=0 :       concrete file size
 *      <0  :       error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_size(OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to remove a file.
 * PARAMETERS
 *      fileName [IN]:  a pointer to file name
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      =0 :            remove file success
 *      <0 :            error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_remove(const char * fileName,fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get the total file space.
 * PARAMETERS
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      unsigned int,total file space.
 *
 *****************************************************************************/
extern unsigned int ol_fs_get_totalspacesize(fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get the free file space.
 * PARAMETERS
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      unsigned int,free file space.
 *
 *****************************************************************************/
extern unsigned int ol_fs_get_freespacesize(fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get used file space.
 * PARAMETERS
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      unsigned int,used file space.
 * RETURN MESSAGE
 *      NONE
 *
 *****************************************************************************/
extern unsigned int ol_fs_get_usedspacesize(fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to list the files.
 * PARAMETERS
 *      list_buf :  a pointer to list buffer
 *      buf_size :  the maxsize list
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      =0 :        list file success
 *      <0 :        error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_list(char * list_buf, int buf_size, fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to flush the cache to the file.
 * PARAMETERS
 *      fp   [IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      =0 :        flush file success
 *      <0 :        error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_flush(OLFILE fp);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to mount file system.
 * PARAMETERS
 *      fp   [IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      =0 :        flush file success
 *      <0 :        error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern int ol_fs_mount(fs_storage_type_t storage_type);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to unmount file system.
 * PARAMETERS
 *      fp   [IN]:    a file handle give by ol_fs_open
 * RETURN VALUES
 *      =0 :        flush file success
 *      <0 :        error num define in enum ol_lfs_errno
 *
 *****************************************************************************/
extern void ol_fs_unmount(fs_storage_type_t storage_type);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to create a directory.
 * PARAMETERS
 *      path   [IN]:    Directory to be created
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      =0 :        create directory success
 *      <0 :        error num define in enum lfs_error
 *
 *****************************************************************************/
extern int ol_fs_mkdir(const char *path, fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to open directory.
 * PARAMETERS
 *      path   [IN]:    directory to be opened
 *      storage_type [IN]: File system storage types
 * RETURN VALUES
 *      Return directory handle
 *
 *****************************************************************************/
extern void* ol_fs_opendir(const char *path, fs_storage_type_t storage_type);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to read an entry in the directory.
 * PARAMETERS
 *      hDir   [IN]:    a directory handle give by ol_fs_opendir
 *      info   [OUT]:   point to directory information structure
 * RETURN VALUES
 *      =0 :        read directory success
 *      <0 :        error num define in enum lfs_error
 *
 *****************************************************************************/
extern int ol_fs_readdir(void* hDir, struct lfs_info *info);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to close directory.
 * PARAMETERS
 *      hDir   [IN]:    a directory handle give by ol_fs_opendir
 * RETURN VALUES
 *      =0 :        close directory success
 *      <0 :        error num define in enum lfs_error
 *
 *****************************************************************************/
extern int ol_fs_closedir(void* hDir);

#endif

