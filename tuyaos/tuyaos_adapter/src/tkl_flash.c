/**
 * @file tkl_ext_flash.c
 * @brief External SPI Flash adapter for L511C-Y7PVM (Winbond 25Q128JWSQ via SPI0)
 * @version 0.1
 * @date 2026-05-29
 */

#include "tuya_cloud_types.h"

#if defined(ENABLE_FLASH) && (ENABLE_FLASH == 1)

#include <string.h>
#include "tkl_flash.h"
#include "ol_spi_api.h"
#include "ex_storage.h"
#include "cmsis_os2.h"
#include "pkg_718pm_mapdef.h"
#include "vlog.h"

static bool ext_flash_inited = false;

OPERATE_RET tkl_flash_init(void)
{
    exStorageInitDev(EXT_FLASH_SPI_INDEX, EF_IMG_LFS_SIZE);
    int ret = exStorageInit();
    if (ret != 0) {
        LOGE("exStorageInit fail, ret=%d", ret);
        return OPRT_COM_ERROR;
    }

    LOGD("ext flash init success, size=%d bytes", EF_IMG_LFS_SIZE);
    ext_flash_inited = true;
    return OPRT_OK;
}

OPERATE_RET tkl_flash_deinit(void)
{
    if (!ext_flash_inited) {
        return OPRT_OK;
    }

    exStorageDeinit();
    ext_flash_inited = false;
    return OPRT_OK;
}

OPERATE_RET tkl_flash_read(UINT32_T addr, UCHAR_T *dst, UINT32_T size)
{
    if (!ext_flash_inited) {
        if(tkl_flash_init())
            return OPRT_COM_ERROR;
    }
    if (NULL == dst || 0 == size) {
        return OPRT_INVALID_PARM;
    }

    int ret = exStorageRead(0, addr, dst, size);
    if (ret != 0) {
        LOGE("read fail, addr=0x%x, size=%d, ret=%d", addr, size, ret);
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_flash_write(UINT32_T addr, CONST UCHAR_T *src, UINT32_T size)
{
    if (!ext_flash_inited) {
        if(tkl_flash_init())
            return OPRT_COM_ERROR;
    }
    if (NULL == src || 0 == size) {
        return OPRT_INVALID_PARM;
    }

    int ret = exStorageWrite(0, addr, (uint8_t *)src, size);
    if (ret != 0) {
        LOGE("write fail, addr=0x%x, size=%d, ret=%d", addr, size, ret);
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_flash_erase(UINT32_T addr, UINT32_T size)
{
    if (!ext_flash_inited) {
        if(tkl_flash_init())
            return OPRT_COM_ERROR;
    }

    int ret = exStorageClear(0, addr, size);
    if (ret != 0) {
        LOGE("erase fail, addr=0x%x, size=%d, ret=%d", addr, size, ret);
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_flash_lock(UINT32_T addr, UINT32_T size)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_flash_unlock(UINT32_T addr, UINT32_T size)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_flash_get_one_type_info(TUYA_FLASH_TYPE_E type, TUYA_FLASH_BASE_INFO_T* info)
{
    return OPRT_NOT_SUPPORTED;
}

/*============================================================================
 * Test function - uses tkl interfaces to verify ext flash read/write/erase
 *============================================================================*/

#define ENABLE_EXT_FLASH_TEST 0
#if defined(ENABLE_EXT_FLASH_TEST) && (ENABLE_EXT_FLASH_TEST == 1)

#include "tkl_fs.h"
#include "tkl_storage.h"
#include "ol_fs_api.h"

#define TEST_ADDR       0x0000
#define TEST_SIZE       256

void tkl_ext_flash_test(void)
{
    OPERATE_RET ret;
    UCHAR_T write_buf[TEST_SIZE];
    UCHAR_T read_buf[TEST_SIZE];
    int i;
    int pass = 1;

    LOGD("===== tkl_ext_flash Test Start =====");

    /* ---- Part 1: Raw flash read/write/erase via tkl_ext_flash ---- */
    LOGD("---- Part 1: Raw Flash R/W/E ----");
    LOGD("[2] tkl_flash_erase addr=0x%x, size=0x1000...", TEST_ADDR);
    ret = tkl_flash_erase(TEST_ADDR, 0x1000);
    if (ret != OPRT_OK) {
        LOGE("[2] erase FAIL, ret=%d", ret);
        goto part1_exit;
    }
    LOGD("[2] erase OK");

    LOGD("[3] read after erase, verify 0xFF...");
    memset(read_buf, 0, sizeof(read_buf));
    ret = tkl_flash_read(TEST_ADDR, read_buf, TEST_SIZE);
    if (ret != OPRT_OK) {
        LOGE("[3] read FAIL, ret=%d", ret);
        goto part1_exit;
    }
    pass = 1;
    for (i = 0; i < TEST_SIZE; i++) {
        if (read_buf[i] != 0xFF) {
            LOGE("[3] FAIL: byte[%d]=0x%x, expect 0xFF", i, read_buf[i]);
            pass = 0;
            break;
        }
    }
    if (pass) LOGD("[3] *** ERASE VERIFY OK ***");

    for (i = 0; i < TEST_SIZE; i++) write_buf[i] = (UCHAR_T)(i & 0xFF);
    LOGD("[4] tkl_flash_write %d bytes...", TEST_SIZE);
    ret = tkl_flash_write(TEST_ADDR, write_buf, TEST_SIZE);
    if (ret != OPRT_OK) {
        LOGE("[4] write FAIL, ret=%d", ret);
        goto part1_exit;
    }
    LOGD("[4] write OK");

    LOGD("[5] tkl_flash_read and verify...");
    memset(read_buf, 0, sizeof(read_buf));
    ret = tkl_flash_read(TEST_ADDR, read_buf, TEST_SIZE);
    if (ret != OPRT_OK) {
        LOGE("[5] read FAIL, ret=%d", ret);
        goto part1_exit;
    }
    pass = 1;
    for (i = 0; i < TEST_SIZE; i++) {
        if (read_buf[i] != write_buf[i]) {
            LOGE("[5] FAIL: byte[%d]=0x%x, expect 0x%x", i, read_buf[i], write_buf[i]);
            pass = 0;
            break;
        }
    }
    if (pass) LOGD("[5] *** RAW R/W VERIFY OK ***");

part1_exit:
    tkl_flash_deinit();
    LOGD("---- Part 1 Complete ----");

    osDelay(1000);

    /* ---- Part 2: File system via tkl_fs (path with /ext/) ---- */
    LOGD("---- Part 2: FS via tkl_fopen(\"/ext/...\") ----");

    // Mount external flash via tkl_storage
    ret = tkl_storage_mount(NULL, NULL, NULL, 0, NULL);
    if (ret != OPRT_OK) {
        LOGE("[FS] tkl_storage_mount fail, ret=%d", ret);
        goto done;
    }
    LOGD("[FS] ext flash mounted OK");

    // Test file operations through tkl_fs with "/ext/" prefix
    char test_data[] = "TuyaOS ExFlash FS test 20260529!";
    char rbuf[128] = {0};

    LOGD("[FS.1] tkl_fopen /ext/test.txt for write...");
    TUYA_FILE fp = tkl_fopen("/ext/test.txt", "wb+");
    if (!fp) {
        LOGE("[FS.1] tkl_fopen FAIL");
        goto fs_cleanup;
    }
    LOGD("[FS.1] open OK");

    LOGD("[FS.2] tkl_fwrite %d bytes...", (int)strlen(test_data));
    int wlen = tkl_fwrite(test_data, strlen(test_data), fp);
    LOGD("[FS.2] tkl_fwrite ret=%d", wlen);

    LOGD("[FS.3] tkl_fseek to 0...");
    tkl_fseek(fp, 0, 0);

    LOGD("[FS.4] tkl_fread...");
    int rlen = tkl_fread(rbuf, sizeof(rbuf) - 1, fp);
    LOGD("[FS.4] tkl_fread ret=%d, data='%s'", rlen, rbuf);

    if (rlen > 0 && strncmp(test_data, rbuf, strlen(test_data)) == 0) {
        LOGD("[FS.5] *** FS READ/WRITE VERIFY OK ***");
    } else {
        LOGE("[FS.5] *** FS VERIFY FAIL ***");
    }

    LOGD("[FS.6] tkl_fclose...");
    tkl_fclose(fp);

    // Verify persistence
    LOGD("[FS.7] Re-open /ext/test.txt for read...");
    fp = tkl_fopen("/ext/test.txt", "rb");
    if (fp) {
        memset(rbuf, 0, sizeof(rbuf));
        rlen = tkl_fread(rbuf, sizeof(rbuf) - 1, fp);
        LOGD("[FS.7] re-read ret=%d, data='%s'", rlen, rbuf);
        if (rlen > 0 && strncmp(test_data, rbuf, strlen(test_data)) == 0) {
            LOGD("[FS.7] *** PERSISTENCE OK ***");
        } else {
            LOGE("[FS.7] *** PERSISTENCE FAIL ***");
        }
        tkl_fclose(fp);
    } else {
        LOGE("[FS.7] re-open FAIL");
    }

    // Check existence and remove
    BOOL_T exist = FALSE;
    tkl_fs_is_exist("/ext/test.txt", &exist);
    LOGD("[FS.8] is_exist='/ext/test.txt' => %d", exist);

    LOGD("[FS.9] tkl_fs_remove /ext/test.txt...");
    tkl_fs_remove("/ext/test.txt");
    exist = TRUE;
    tkl_fs_is_exist("/ext/test.txt", &exist);
    LOGD("[FS.9] after remove, exist=%d", exist);

fs_cleanup:
    tkl_storage_umount(NULL, 0);
    LOGD("---- Part 2 Complete ----");

done:
    LOGD("===== tkl_ext_flash Test ALL Complete =====");

    while (1) {
        osDelay(5000);
    }
}

#endif /* ENABLE_EXT_FLASH_TEST */

#endif /* ENABLE_EXT_FLASH */
