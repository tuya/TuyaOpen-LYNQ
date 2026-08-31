/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "ol_fs_api.h"
#include "ol_spi_api.h"
#include "lfs.h"

#define LFS_FILENAME        "fs_test.txt"
#define DIR_TEST_NAME       "/dir_test"
#define TEST_EXFLASH_SIZE   (0x80000)  // 512KB，根据实际调整

#define FS_DEMO_USE_EXTERNAL  1

void test_change_file(fs_storage_type_t type)
{
    OLFILE fp = NULL;
    int ret = -1;
    char buff[64] = {0};
    int filesize = 0;

    fp = ol_fs_open(LFS_FILENAME, "ab+",type);
    if(fp == NULL)
    {
        OL_LOG_PRINTF("ol_fs_open fail");
        return;
    }

    filesize = ol_fs_size(fp);
    OL_LOG_INFO("ol_fs_size = %d", filesize);

    ret = ol_fs_read(buff, sizeof(buff), fp);
    OL_LOG_INFO("ol_fs_read size = %d, buf = %s", ret, buff);

    ol_fs_seek(fp, 5, SEEK_SET);
    ret = ol_fs_tell(fp);
    OL_LOG_INFO("ol_fs_tell offset = %d", ret);

    memset(buff, 0, sizeof(buff));
    ret = ol_fs_read(buff, sizeof(buff), fp);
    OL_LOG_INFO("ol_fs_read size = %d, buf = %s", ret, buff);

    ret = ol_fs_tell(fp);
    OL_LOG_INFO("ol_fs_tell offset = %d", ret);

    ret = ol_fs_close(fp);
}


void test_create_file(fs_storage_type_t type)
{
    OSAFILE fp = NULL;
    int ret = -1;
    int i = 0;

    fp = ol_fs_open(LFS_FILENAME, "wb+",type);
    if(fp == NULL)
    {
        OL_LOG_PRINTF("ol_fs_open fail");
        return;
    }

    ret = ol_fs_write((void*)"0123456789", strlen("0123456789"), fp);
    OL_LOG_INFO("ol_fs_write ret = %d", ret);

    ret = ol_fs_flush(fp);
    OL_LOG_INFO("ol_fs_flush ret = %d", ret);

    ret = ol_fs_write((void*)"abcdefghij", strlen("abcdefghij"), fp);
    OL_LOG_INFO("ol_fs_write ret = %d", ret);

    while(i++ < 64)
    {
        ol_fs_write(&i, sizeof(i), fp);
    }

    ret = ol_fs_close(fp);
    OL_LOG_INFO("ol_fs_close ret = %d", ret);
}

// 测试目录创建与 stat
static void test_dir_create(fs_storage_type_t type)
{
    int ret;
    OLFILE hDir;
    struct lfs_info info;

    // 尝试打开目录，若失败则创建
    hDir = ol_fs_opendir(DIR_TEST_NAME, type);
    if (hDir == NULL) {
        OL_LOG_INFO("dir not exist, creating: %s", DIR_TEST_NAME);
        ret = ol_fs_mkdir(DIR_TEST_NAME, type);
        if (ret < 0) {
            OL_LOG_ERROR("mkdir fail, ret=%d", ret);
            return;
        }
        hDir = ol_fs_opendir(DIR_TEST_NAME, type);
        if (hDir == NULL) {
            OL_LOG_ERROR("open newly created dir fail");
            return;
        }
    }
    ol_fs_closedir(hDir);

    // 使用 stat 获取目录信息
    memset(&info, 0, sizeof(info));
    ret = ol_fs_stat(DIR_TEST_NAME, &info, type);
    OL_LOG_INFO("stat dir: ret=%d, type=%d, name=%s", ret, info.type, info.name);

    // 对文件 stat
    memset(&info, 0, sizeof(info));
    ret = ol_fs_stat(LFS_FILENAME, &info, type);
    OL_LOG_INFO("stat file: ret=%d, type=%d, name=%s, size=%d", ret, info.type, info.name, info.size);
}

// 测试遍历根目录
static void test_dir_list(fs_storage_type_t type)
{
    OLFILE hDir;
    struct lfs_info info;
    int ret;

    hDir = ol_fs_opendir("/", type);
    if (hDir == NULL) {
        OL_LOG_ERROR("opendir / fail");
        return;
    }

    OL_LOG_INFO("=== Directory listing of / ===");
    while ((ret = ol_fs_readdir(hDir, &info)) > 0) {
        if (info.type == LFS_TYPE_REG) {
            OL_LOG_INFO("[FILE] %s (%u bytes)", info.name, info.size);
        } else if (info.type == LFS_TYPE_DIR) {
            OL_LOG_INFO("[DIR]  %s", info.name);
        }
    }
    if (ret < 0) {
        OL_LOG_ERROR("readdir error: %d", ret);
    }
    ol_fs_closedir(hDir);
}

// 主演示函数
void fs_demo(void)
{
    fs_storage_type_t type = FS_TYPE_EXTERNAL;  // 使用外部 Flash，也可改为 FS_TYPE_INTERNAL
    int t_size, u_size, f_size;
    char list_buf[1024] = {0};
    char *token;

    // 1. 初始化外部 SPI Flash 设备（仅外部 Flash 需要）
#ifdef FS_DEMO_USE_EXTERNAL
    exStorageInitDev(OL_SPI_INDEX_1, TEST_EXFLASH_SIZE);
    if (exStorageInit() != 0) {
        OL_LOG_ERROR("exStorageInit failed");
        return;
    }
#endif

    // 2. 挂载文件系统
    if (ol_fs_mount(type) != 0) {
        OL_LOG_ERROR("mount failed");
        return;
    }
    OL_LOG_INFO("Filesystem mounted");

    // 3. 查询空间
    t_size = ol_fs_get_totalspacesize(type);
    u_size = ol_fs_get_usedspacesize(type);
    f_size = ol_fs_get_freespacesize(type);
    OL_LOG_INFO("Total=%d, Used=%d, Free=%d", t_size, u_size, f_size);

    // 4. 文件操作
    test_create_file(type);
    test_change_file(type);

    // 5. 列出文件（使用 ol_fs_list 接口）
    ol_fs_list(list_buf, sizeof(list_buf), type);
    token = strtok(list_buf, "\r\n");
    OL_LOG_INFO("=== ol_fs_list output ===");
    while (token != NULL) {
        OL_LOG_INFO("%s", token);
        token = strtok(NULL, "\n");
    }

    // 6. 目录操作
    test_dir_create(type);
    test_dir_list(type);

    // 7. 再次查询空间（确认占用）
    u_size = ol_fs_get_usedspacesize(type);
    f_size = ol_fs_get_freespacesize(type);
    OL_LOG_INFO("After operations: Used=%d, Free=%d", u_size, f_size);

    // 8. 清理：删除文件和目录
    ol_fs_remove(LFS_FILENAME, type);
    ol_fs_remove(DIR_TEST_NAME, type);
    osDelay(1000);

    // 9. 最终空间查询
    u_size = ol_fs_get_usedspacesize(type);
    f_size = ol_fs_get_freespacesize(type);
    OL_LOG_INFO("After cleanup: Used=%d, Free=%d", u_size, f_size);

    // 10. 卸载
    ol_fs_unmount(type);
    OL_LOG_INFO("Filesystem unmounted");
}

