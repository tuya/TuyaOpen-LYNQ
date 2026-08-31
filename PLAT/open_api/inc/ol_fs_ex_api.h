#ifndef __OL_FS_EX_API__
#define __OL_FS_EX_API__


#include "ol_fs_api.h"


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to mount SPI extend flash file system.
 * PARAMETERS
 *      NONE
 * RETURN VALUES
 *      =0 :       init success
 *      <0 :       error
 *
 *****************************************************************************/
extern int ol_fs_ex_mount(void);


/********************************************
*for file_open <mode>
*       "rb"        read only
*       "rb+"       read and write
*       "wb"        create,write,truncate
*       "wb+"       create,write,read,truncate
*       "ab"        create,write,append
*       "ab+"       create,write,read,append
*********************************************/
/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to open a file from the QSPI flash with the mode.
 * PARAMETERS
 *      fileName [IN]:    The file name of the file you want to open
 *      mode     [IN]:    how to open the file
 * RETURN VALUES
 *      OLFILE
 *
 *****************************************************************************/
extern OLFILE ol_fs_ex_open(const char * path, const char * mode);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to close a file handle from the QSPI flash.
 * PARAMETERS
 *      fp[IN]:    a file handle give by ol_fs_ex_open
 * RETURN VALUES
 *      =0 :       close file success
 *      <0 :       error
 *
 *****************************************************************************/
extern int ol_fs_ex_close(OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to read data from a opened file in the QSPI flash.
 * PARAMETERS
 *      buf [OUT]:    a pointer to read buffer
 *      size [IN]:    number of bytes to read
 *      fp   [IN]:    a file handle give by ol_fs_ex_open
 * RETURN VALUES
 *      >=0 :        concrete length of read data
 *      <0  :        error
 *
 *****************************************************************************/
extern int ol_fs_ex_read(void *buf, UINT32 size, OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to write data into a file which can be written in the QSPI flash.
 * PARAMETERS
 *      buf  [IN]:    a pointer to write buffer
 *      size [IN]:    number of bytes to write
 *      fp   [IN]:    a file handle give by ol_fs_ex_open
 * RETURN VALUES
 *      >=0 :         exact length of write data
 *      <0  :         error
 *
 *****************************************************************************/
extern int ol_fs_ex_write(void *buf, UINT32 size, OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get the number of bytes that the current position of the file pointer is
 *      offset from the beginning of the file in the QSPI flash.
 * PARAMETERS
 *      fp [IN]:     a file handle give by ol_fs_ex_open
 * RETURN VALUES
 *      >=0 :        the number of byte
 *      <0  :        error
 *
 *****************************************************************************/
extern int ol_fs_ex_tell(OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to repositions the offset of the open file in the QSPI flash.
 * PARAMETERS
 *      fp       [IN]:    a file handle give by ol_fs_ex_open
 *      offset   [IN]:    number of bytes to move the file pointer
 *      seekType [IN]:    The file pointer reference position.
 * RETURN VALUES
 *      =0 :              file seek success
 *      <0 :              error
 *
 *****************************************************************************/
extern int ol_fs_ex_seek(OLFILE fp, INT32 offset, UINT8 seekType);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to get the file size in the QSPI flash.
 * PARAMETERS
 *      fp [IN]:    a file handle give by ol_fs_ex_open
 * RETURN VALUES
 *      >=0 :       concrete file size
 *      <0  :       error
 *
 *****************************************************************************/
extern int ol_fs_ex_size(OLFILE fp);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to remove a file.
 * PARAMETERS
 *      fileName [IN]:  a pointer to file name
 * RETURN VALUES
 *      =0 :            remove file success
 *      <0 :            error
 *
 *****************************************************************************/
extern int ol_fs_ex_remove(const char * fileName);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to unmount extend flash file system.
 * PARAMETERS
 *      NONE
 * RETURN VALUES
 *      =0 :       init success
 *      <0 :       error
 *
 *****************************************************************************/
extern void ol_fs_ex_unmount(void);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to create a directory.
 * PARAMETERS
 *      path   [IN]:    Directory to be created
 * RETURN VALUES
 *      =0 :        create directory success
 *      <0 :        error num define in enum lfs_error
 *
 *****************************************************************************/
extern int ol_fs_ex_mkdir(char* path);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is to open directory.
 * PARAMETERS
 *      hDir   [IN]:    a directory handle give by ol_fs_opendir
 *      path   [IN]:    directory to be opened
 * RETURN VALUES
 *      Return directory handle
 *
 *****************************************************************************/
extern void* ol_fs_ex_opendir(lfs_dir_t *dir, const char *path);


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
extern int ol_fs_ex_readdir(lfs_dir_t *dir, struct lfs_info *info);


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
extern int ol_fs_ex_closedir(void* hDir);

#endif
