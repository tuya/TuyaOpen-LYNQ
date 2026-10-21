/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "ol_fs_api.h"


#define LFS_FILENAME        "fs_test.txt"

void test_change_file(void)
{
    OLFILE fp = NULL;
    int ret = -1;
    char buff[64] = {0};
    int filesize = 0;

    fp = ol_fs_open(LFS_FILENAME, "ab+", FS_TYPE_INTERNAL);
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


void test_create_file(void)
{
    OSAFILE fp = NULL;
    int ret = -1;
    int i = 0;

    fp = ol_fs_open(LFS_FILENAME, "wb+", FS_TYPE_INTERNAL);
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

void fs_demo(void)
{
    int t_size = 0;
    int u_size = 0;
    int f_size = 0;
    char list_buf[512] = {0};
    char *token = NULL;

    t_size = ol_fs_get_totalspacesize(FS_TYPE_INTERNAL);
    u_size = ol_fs_get_usedspacesize(FS_TYPE_INTERNAL);
    f_size = ol_fs_get_freespacesize(FS_TYPE_INTERNAL);
    OL_LOG_INFO("check spacesize: total=%d(Byte), used=%d(Byte) free=%d(Byte)",t_size, u_size, f_size);

    test_create_file();
    test_change_file();
    ol_fs_list(list_buf, sizeof(list_buf), FS_TYPE_INTERNAL);
    token = strtok(list_buf, "\r\n");
    while (token != NULL) 
    {
        OL_LOG_INFO("ol_fs_list: %s", token);
        token = strtok(NULL, "\n");
    }

    t_size = ol_fs_get_totalspacesize(FS_TYPE_INTERNAL);
    u_size = ol_fs_get_usedspacesize(FS_TYPE_INTERNAL);
    f_size = ol_fs_get_freespacesize(FS_TYPE_INTERNAL);
    OL_LOG_INFO("check spacesize: total=%d(Byte), used=%d(Byte) free=%d(Byte)",t_size, u_size, f_size);

    ol_fs_remove(LFS_FILENAME, FS_TYPE_INTERNAL);
    osDelay(1000);

    t_size = ol_fs_get_totalspacesize(FS_TYPE_INTERNAL);
    u_size = ol_fs_get_usedspacesize(FS_TYPE_INTERNAL);
    f_size = ol_fs_get_freespacesize(FS_TYPE_INTERNAL);
    OL_LOG_INFO("check spacesize: total=%d(Byte), used=%d(Byte) free=%d(Byte)",t_size, u_size, f_size);
}

