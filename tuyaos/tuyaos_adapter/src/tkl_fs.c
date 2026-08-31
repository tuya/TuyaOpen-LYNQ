#include <stdlib.h>
#include <string.h>
#include "tkl_fs.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_fs_api.h"
#include "vlog.h"
#include "lfs_port.h"

#undef LOGD
#define LOGD(fmt, ...)

#ifndef EXT_FLASH_MOUNT_PATH
#define EXT_FLASH_MOUNT_PATH "/ext"
#endif

typedef struct
{
    UINT32 file_flag;
    lfs_file_t* lfs_ptr;
} ty_fs;

typedef struct
{
    lfs_dir_t        dir;
    struct lfs_info  info;
    bool             is_ext_fs_flag;
} TKL_DIR_CTX_S;

static int is_ext_path(const char* path)
{
    if (path && strncmp(path, EXT_FLASH_MOUNT_PATH, strlen(EXT_FLASH_MOUNT_PATH)) == 0) {
        return 1;
    }
    return 0;
}

static const char* strip_path(const char* path)
{
    if (is_ext_path(path)) {
        return path;  // ext flash: LFS stores full path including /ext/ prefix
    }
    const char* f = strrchr(path, '/');
    if (NULL == f)
        return path;
    else
        return f + 1;
}

static fs_storage_type_t get_storage_type(const char* path)
{
    return is_ext_path(path) ? FS_TYPE_EXTERNAL : FS_TYPE_INTERNAL;
}

/**
* @brief Make directory
*
* @param[in] path: path of directory
*
* @note This API is used for making a directory
*
* @return 0 on success. Others on failed
*/
INT_T tkl_fs_mkdir(CONST CHAR_T* path)
{
    if(!is_ext_path(path)) {
        return OPRT_OK;
    }

    LOGD("mkdir %s", path);
    INT_T ret = ol_fs_mkdir(path, FS_TYPE_EXTERNAL);
    if(ret) {
        LOGE("mkidr %s failed %d",path, ret);
    }
    return ret;
}

/**
* @brief Remove directory
*
* @param[in] path: path of directory
*
* @note This API is used for removing a directory23
*
* @return 0 on success. Others on failed
*/
INT_T tkl_fs_remove(CONST CHAR_T* path)
{
    if (NULL == path) {
        LOGE("tkl_fs_remove, path null");
        return -1;
    }

    const char* file = strip_path(path);
    LOGD("tkl_fs_remove: %s, file: %s", path, file);
    return ol_fs_remove(file, get_storage_type(path));
}

/**
* @brief Get file mode
*
* @param[in] path: path of directory
* @param[out] mode: bit attibute of directory
*
* @note This API is used for getting file mode.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_fs_mode(CONST CHAR_T* path, UINT_T* mode)
{
    LOGD("tkl_fs_mode not support: %s", path);
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief Check whether the file or directory exists
*
* @param[in] path: path of directory
* @param[out] is_exist: the file or directory exists or not
*
* @note This API is used to check whether the file or directory exists.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_fs_is_exist(CONST CHAR_T* path, BOOL_T* is_exist)
{
    OLFILE fp = NULL;

    if (NULL == path) {
        LOGE("tkl_fs_is_exist path null");
        return OPRT_INVALID_PARM;
    }

    const char* file = strip_path(path);
    fp = ol_fs_open(file, "r", get_storage_type(path));
    if (fp) {
        ol_fs_close(fp);
        *is_exist = TRUE;
    } else {
        *is_exist = FALSE;
    }
    LOGD("tkl_fs_is_exist, path: %s, file: %s, exist: %d", path, file, *is_exist);
    return OPRT_OK;
}

/**
* @brief File rename
*
* @param[in] path_old: old path of directory
* @param[in] path_new: new path of directory
*
* @note This API is used to rename the file.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_fs_rename(CONST CHAR_T* path_old, CONST CHAR_T* path_new)
{
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief Open directory
*
* @param[in] path: path of directory
* @param[out] dir: handle of directory
*
* @note This API is used to open a directory
*
* @return 0 on success. Others on failed
*/
INT_T tkl_dir_open(CONST CHAR_T* path, TUYA_DIR* dir)
{
    if(dir == NULL) {
        return OPRT_INVALID_PARM;
    }
    *dir = NULL;

    TKL_DIR_CTX_S* ctx = (TKL_DIR_CTX_S*)tkl_system_malloc(sizeof(TKL_DIR_CTX_S));
    if(ctx == NULL) {
        LOGE("tkl_dir_open malloc failed, path=%s", path);
        return OPRT_MALLOC_FAILED;
    }
    memset(ctx, 0, sizeof(TKL_DIR_CTX_S));

    if(!is_ext_path(path)) {
        ctx->is_ext_fs_flag = false;
        *dir = (TUYA_DIR)ctx;
        return OPRT_OK;
    }

    ctx->is_ext_fs_flag = true;
    INT_T ret = (INT_T)ol_fs_ex_opendir(&ctx->dir, path);
    if(ret != 0) {
        LOGE("tkl_dir_open opendir %s failed, ret=%d", path, ret);
        tkl_system_free(ctx);
        return ret;
    }
    *dir = (TUYA_DIR)ctx;
    LOGD("dir %s open succ",path);
    return ret;
}

/**
* @brief Close directory
*
* @param[in] dir: handle of directory
*
* @note This API is used to close a directory
*
* @return 0 on success. Others on failed
*/
INT_T tkl_dir_close(TUYA_DIR dir)
{
    if(dir == NULL) {
        return OPRT_INVALID_PARM;
    }

    INT_T ret = OPRT_OK;
    TKL_DIR_CTX_S* ctx = (TKL_DIR_CTX_S*)dir;
    if(ctx->is_ext_fs_flag) {
        ret = (INT_T)ol_fs_ex_closedir(&ctx->dir);
        if(ret != 0) {
            LOGE("tkl_dir_close failed, ret=%d", ret);
        }
    }
    tkl_system_free(ctx);
    return ret;
}

/**
* @brief Read directory
*
* @param[in] dir: handle of directory
* @param[out] info: file information
*
* @note This API is used to read a directory.
* Read the file information of the current node, and the internal pointer points to the next node.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_dir_read(TUYA_DIR dir, TUYA_FILEINFO* info)
{
    if((dir == NULL) || (info == NULL)) {
        return OPRT_INVALID_PARM;
    }

    TKL_DIR_CTX_S* ctx = (TKL_DIR_CTX_S*)dir;
    if(!ctx->is_ext_fs_flag) {
        *info = NULL;
        return OPRT_NOT_SUPPORTED;
    }

    memset(&ctx->info, 0, sizeof(struct lfs_info));
    INT_T ret = (INT_T)ol_fs_ex_readdir(&ctx->dir, &ctx->info);
    *info = (TUYA_FILEINFO)&ctx->info;
    if(ret < 0 && ret != OL_LFS_ERR_EOF) {
        LOGE("dir read failed %d", ret);
        return ret;
    }

    return OPRT_OK;
}

/**
* @brief Get the name of the file node
*
* @param[in] info: file information
* @param[out] name: file name
*
* @note This API is used to get the name of the file node.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_dir_name(TUYA_FILEINFO info, CONST CHAR_T** name)
{
    if((info == NULL) || (name == NULL)) {                                                                                                                             
        return OPRT_INVALID_PARM;                                                                                                                                      
    }                                                                                                                                                                  
                                                                                                                                                                       
    struct lfs_info* pinfo = (struct lfs_info*)info;                                                                                                                   
    *name = (CONST CHAR_T*)pinfo->name;                                                                                                                              
    return OPRT_OK;  
}

/**
* @brief Check whether the node is a directory
*
* @param[in] info: file information
* @param[out] is_dir: is directory or not
*
* @note This API is used to check whether the node is a directory.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_dir_is_directory(TUYA_FILEINFO info, BOOL_T* is_dir)
{
    if((info == NULL) || (is_dir == NULL)) {
        return OPRT_INVALID_PARM;
    }

    struct lfs_info* pinfo = (struct lfs_info*)info;
    *is_dir = (pinfo->type == LFS_TYPE_DIR) ? TRUE : FALSE;
    return OPRT_OK;
}

/**
* @brief Check whether the node is a normal file
*
* @param[in] info: file information
* @param[out] is_regular: is normal file or not
*
* @note This API is used to check whether the node is a normal file.
*
* @return 0 on success. Others on failed
*/
INT_T tkl_dir_is_regular(TUYA_FILEINFO info, BOOL_T* is_regular)
{
    if((info == NULL) || (is_regular == NULL)) {
        return OPRT_INVALID_PARM;
    }

    struct lfs_info* pinfo = (struct lfs_info*)info;
    *is_regular = (pinfo->type == LFS_TYPE_REG) ? TRUE : FALSE;
    return OPRT_OK;
}

/**
* @brief Open file
*
* @param[in] path: path of file
* @param[in] mode: file open mode: "r","w"...
*
* @note This API is used to open a file
*
* @return the file handle, NULL means failed
*/
TUYA_FILE tkl_fopen(CONST CHAR_T* path, CONST CHAR_T* mode)
{
    if (NULL == path) {
        LOGE("tkl_fopen failed, path null");
        return NULL;
    }

    const char* file = strip_path(path);
    OLFILE fp = ol_fs_open(file, mode, get_storage_type(path));
    if (!fp) {
        LOGD("tkl_fopen failed, path: %s, file: %s, mode: %s", path, file, mode);
        return NULL;
    }

    LOGD("tkl_fopen succ, path: %s, file: %s, mode: %s", path, file, mode);
    return (TUYA_FILE)fp;
}

/**
* @brief Close file
*
* @param[in] file: file handle
*
* @note This API is used to close a file
*
* @return 0 on success. EOF on failed
*/
INT_T tkl_fclose(TUYA_FILE file)
{
    int ret = ol_fs_close((OLFILE)file);
    return (0 == ret) ? OPRT_OK : OPRT_COM_ERROR;
}

/**
* @brief Read file
*
* @param[in] buf: buffer for reading file
* @param[in] bytes: buffer size
* @param[in] file: file handle
*
* @note This API is used to read a file
*
* @return the bytes read from file
*/
INT_T tkl_fread(VOID_T* buf, INT_T bytes, TUYA_FILE file)
{
    return ol_fs_read(buf, (UINT32_T)bytes, (OLFILE)file);
}

/**
* @brief write file
*
* @param[in] buf: buffer for writing file
* @param[in] bytes: buffer size
* @param[in] file: file handle
*
* @note This API is used to write a file
*
* @return the bytes write to file
*/
INT_T tkl_fwrite(VOID_T* buf, INT_T bytes, TUYA_FILE file)
{
    int ret = 0;
    int writelen = 0;
    int len = 0;
    while (writelen < bytes) {
        len = bytes - writelen;
        len = len > 16384 ? 16384 : len;
        ret = ol_fs_write(((char *)buf)+writelen, (UINT32_T)len, (OLFILE)file);
        if (ret != len) {
            break;
        }
        ol_fs_flush((OLFILE)file);
        writelen += len;
    }
    return writelen;
}

/**
* @brief write buffer to flash
*
* @param[in] fd: file fd
*
* @note This API is used to write buffer to flash
*
* @return 0 on success. others on failed
*/
INT_T tkl_fsync(INT_T fd)
{
    TUYA_FILE file = (TUYA_FILE)fd;
    return tkl_fflush(file);
}

/**
* @brief Read string from file
*
* @param[in] buf: buffer for reading file
* @param[in] len: buffer size
* @param[in] file: file handle
*
* @note This API is used to read string from file
*
* @return the content get from file, NULL means failed
*/
CHAR_T* tkl_fgets(CHAR_T* buf, INT_T len, TUYA_FILE file)
{
    LOGD("tkl_fgets not support");
    return NULL;
}

/**
* @brief Check wheather to reach the end fo the file
*
* @param[in] file: file handle
*
* @note This API is used to check wheather to reach the end fo the file
*
* @return 0 on not eof, others on eof
*/
INT_T tkl_feof(TUYA_FILE file)
{
    if (ol_fs_tell((OLFILE)file) == ol_fs_size((OLFILE)file))
        return 0;
    return -1;
}

/**
* @brief Seek to the offset position of the file
*
* @param[in] file: file handle
* @param[in] offs: offset
* @param[in] whence: seek start point mode
*
* @note This API is used to seek to the offset position of the file.
*
* @return 0 on success, others on failed
*/
INT_T tkl_fseek(TUYA_FILE file, INT64_T offs, INT_T whence)
{
    return ol_fs_seek((OLFILE)file, (INT32_T)offs, whence);
}

/**
* @brief Get current position of file
*
* @param[in] file: file handle
*
* @note This API is used to get current position of file.
*
* @return the current offset of the file
*/
INT64_T tkl_ftell(TUYA_FILE file)
{
    return ol_fs_tell((OLFILE)file);
}

/**
* @brief Get file size
*
* @param[in] filepath file path + file name
*
* @note This API is used to get the size of file.
*
* @return the sizeof of file
*/
INT_T tkl_fgetsize(CONST CHAR_T* filepath)
{
    TUYA_FILE fp = tkl_fopen(filepath, "r");
    if (!fp) {
        LOGE("tkl_fgetsize failed: %s", filepath);
        return 0;
    }
    int size = ol_fs_size(fp);
    tkl_fclose(fp);
    LOGD("tkl_fgetsize succ: %s", filepath);
    return size;
}

/**
* @brief Judge if the file can be access
*
* @param[in] filepath file path + file name
*
* @param[in] mode access mode
*
* @note This API is used to access one file.
*
* @return 0 success,-1 failed
*/
INT_T tkl_faccess(CONST CHAR_T* filepath, INT_T mode)
{
    BOOL_T exist;
    INT_T ret = tkl_fs_is_exist(filepath, &exist);
    LOGD("tkl_faccess: %s, ret/%d, exist/%d", filepath, ret, exist);
    if (ret || FALSE == exist) {
        return -1;
    } else {
        return 0;
    }
}

/**
* @brief read the next character from stream
*
* @param[in] file char stream
*
* @note This API is used to get one char from stream.
*
* @return as an unsigned char cast to a int ,or EOF on end of file or error
*/
INT_T tkl_fgetc(TUYA_FILE file)
{
    unsigned char ch;
    ssize_t ret = tkl_fread(&ch, 1, file);
    if (ret == 1) {
        return ch;
    } else {
        return EOF;
    }
}

/**
* @brief flush the IO read/write stream
*
* @param[in] file char stream
*
* @note This API is used to flush the IO read/write stream.
*
* @return 0 success,-1 failed
*/
INT_T tkl_fflush(TUYA_FILE file)
{
    return ol_fs_flush((OLFILE)file);
}

/**
* @brief get the file fd
*
* @param[in] file char stream
*
* @note This API is used to get the file fd.
*
* @return the file fd
*/
INT_T tkl_fileno(TUYA_FILE file)
{
    int fd = (int)file;
    LOGD("tkl_fileno, %d", fd);
    return fd;
}

/**
* @brief truncate one file according to the length
*
* @param[in] fd file description
*
* @param[in] length the length want to truncate
*
* @note This API is used to truncate one file.
*
* @return 0 success,-1 failed
*/
INT_T tkl_ftruncate(INT_T fd, UINT64_T length)
{
    TUYA_FILE fp = (TUYA_FILE)fd;
    ty_fs* hd = (ty_fs*)fp;
    lfs_file_t* lfs_ptr = (lfs_file_t*)hd->lfs_ptr;
    LOGE("tkl_ftruncate name = %s",lfs_ptr->name);
    return LFS_fileTruncate(lfs_ptr, length);
}

void tkl_ext_fs_list(const char *dirpath)
{
    int ret = 0;
    lfs_dir_t dir;
    struct lfs_info info;
    char path[64];

    memset(&dir, 0, sizeof(lfs_dir_t));
    ret=ol_fs_ex_opendir(&dir, dirpath);
    if (ret != 0) {
        LOGE("opendir %s failed, ret=%d", dirpath, ret);
    } else {
        LOGD("opendir %s success", dirpath);
        while (1) {
            memset(&info, 0, sizeof(struct lfs_info));
            ret = ol_fs_ex_readdir(&dir, &info);
            if (ret < 0) {
                LOGE("readdir error, ret=%d", ret);
                break;
            } else if (ret == 0) {
                break;  // no more entries
            }

            if(strcmp(info.name, ".") == 0 ||  strcmp(info.name, "..") == 0)
                continue;

            // 打印文件名和大小
            if (info.type == LFS_TYPE_REG) {
                LOGI("file: %s, size=%d", info.name, info.size);
            } else if (info.type == LFS_TYPE_DIR) {
                memset(path, 0, sizeof(path));
                snprintf(path, sizeof(path), "%s/%s", dirpath, info.name);

                LOGI("path %s %s", path, info.name);
                tkl_ext_fs_list(path);
            }
        }
        ol_fs_ex_closedir(&dir);
        LOGD("closedir");
    }
}

#if 0  // tkl 版本接口
void tkl_ext_fs_list_v2(const char *dirpath)
{
    TUYA_DIR dir = NULL;
    TUYA_FILEINFO info = NULL;
    CONST CHAR_T *name = NULL;
    BOOL_T is_dir = FALSE;
    BOOL_T is_reg = FALSE;
    CHAR_T path[64];

    if (tkl_dir_open(dirpath, &dir) != OPRT_OK || dir == NULL) {
        LOGE("opendir %s failed", dirpath);
        return;
    }
    LOGD("opendir %s success", dirpath);

    while (tkl_dir_read(dir, &info) == OPRT_OK) {
        name = NULL;
        if (tkl_dir_name(info, &name) != OPRT_OK || name == NULL || name[0] == '\0') {
            break;  // no more entries
        }

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        is_dir = FALSE;
        is_reg = FALSE;
        tkl_dir_is_directory(info, &is_dir);
        tkl_dir_is_regular(info, &is_reg);

        if (is_reg) {
            snprintf(path, sizeof(path), "%s/%s", dirpath, name);
            LOGI("file: %s, size=%d", name, tkl_fgetsize(path));
        } else if (is_dir) {
            snprintf(path, sizeof(path), "%s/%s", dirpath, name);
            LOGI("path %s %s", path, name);
            tkl_ext_fs_list_v2(path);
        }
    }

    tkl_dir_close(dir);
    LOGD("closedir");
}
#endif