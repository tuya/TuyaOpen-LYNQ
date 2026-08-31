/**
 * @file tkl_storage.c
 * @brief Storage adapter for L511C-Y7PVM - mount/unmount external flash LFS
 * @version 0.1
 * @date 2026-05-29
 */

#include "tuya_cloud_types.h"
#include "tkl_storage.h"
#include "tkl_output.h"
#include "ol_spi_api.h"
#include "ol_fs_api.h"
#include "ex_storage.h"
#include "pkg_718pm_mapdef.h"
#include "vlog.h"

#if defined(ENABLE_STORAGE) && ENABLE_STORAGE == 1 

static bool storage_mounted = false;

OPERATE_RET tkl_storage_get_internal_dir(CHAR_T path[MAX_PATH_LEN])
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_df(CHAR_T *mount_point, TKL_STORAGE_DF_T *df)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_get_device_info(CHAR_T *dev_name, TKL_STORAGE_DEVICE_INFO_T *list)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_get_device_list(TKL_STORAGE_DEVICE_INFO_T *list, INT_T num)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_get_part_list(CHAR_T *dev_name, TKL_STORAGE_PART_INFO_T *list, INT_T num)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_make_part(CHAR_T *part_name, CHAR_T *part_type,
    CHAR_T *fs_type, BOOL_T isadd, UINT_T part_index, LONG_T start, LONG_T end)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_mkfs(CHAR_T *fs_type, CHAR_T *fs_dev, CHAR_T *parm,
    TKL_STORAGE_PROGRESS_CB cb, VOID *user_ctx)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief set ext flash mount info
 *
 * @param mount_path point to mount path
 * @param size ext flash size, MB
 * @return OPERATE_RET 0 on success. A negative error code on error.
 */
OPERATE_RET __attribute__((weak)) tkl_storage_get_mount_info(char **mount_path, uint16_t *size)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_mount(CHAR_T *source, CHAR_T *target, CHAR_T *fs_type, ULONG_T flags, VOID *data)
{
    LOGD("tkl_storage_mount called");
    if (storage_mounted) {
        LOGD("already mounted");
        return OPRT_OK;
    }

    uint32_t ext_flash_size = 32;
    char* ext_flash_mount_path = EXT_FLASH_MOUNT_PATH;

    tkl_storage_get_mount_info(&ext_flash_mount_path, &ext_flash_size);

    LOGD("exStorageInitDev SPI%d, size=0x%x", EXT_FLASH_SPI_INDEX, ext_flash_size*1024*1024);
    exStorageInitDev(EXT_FLASH_SPI_INDEX, ext_flash_size*1024*1024);
    int ret = exStorageInit();
    if (ret != 0) {
        LOGE("exStorageInit fail, ret=%d", ret);
        return OPRT_COM_ERROR;
    }
    LOGD("exStorageInit OK");

    ret = ol_fs_mount(FS_TYPE_EXTERNAL);
    if (ret != 0) {
        LOGE("ol_fs_mount(EXTERNAL) fail, ret=%d", ret);
        exStorageDeinit();
        return OPRT_COM_ERROR;
    }

    storage_mounted = true;
    extern void tkl_ext_fs_list(const char *dirpath);
    tkl_ext_fs_list(EXT_FLASH_MOUNT_PATH);
    LOGD("ext flash LFS mounted OK, size=%dMB", ext_flash_size);
    return OPRT_OK;
}

OPERATE_RET tkl_storage_umount(CHAR_T *target, INT_T flags)
{
    if (!storage_mounted) {
        return OPRT_OK;
    }

    ol_fs_unmount(FS_TYPE_EXTERNAL);
    exStorageDeinit();
    storage_mounted = false;
    LOGD("ext flash unmounted");
    return OPRT_OK;
}

OPERATE_RET tkl_storage_loop_event(TKL_STORAGE_EVENT_CB event_cb, VOID *user_ctx)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_fsck(CHAR_T *blk_dev, CHAR_T *fs_type, CHAR_T *parm, INT_T flags,
    TKL_STORAGE_PROGRESS_CB cb, VOID *user_ctx)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_ioctl(CHAR_T *dev_name, ULONG_T request, VOID *args)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_storage_check_badblocks(CHAR_T *dev_name, TKL_STORAGE_BADBLK_CB cb, VOID *user_ctx)
{
    return OPRT_OK;
}

#endif