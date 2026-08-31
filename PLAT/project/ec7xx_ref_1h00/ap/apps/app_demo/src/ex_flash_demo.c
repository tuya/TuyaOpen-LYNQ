/****************************************************************************
 *
 ****************************************************************************/
#include "stdio.h"
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "mem_map.h"
#include "ex_storage.h"
#include "ol_spi_api.h"
#include "ol_fs_ex_api.h"
#include "ol_log.h"


#define TEST_EXFLASH_LOAD_SIZE          (0x80000)//512KByte
#define TEST_RW_BUFF_SIZE               (1024)


void test_block_page(void)
{
    int ret = 0;
    int offset = 0;
    int count = 0;
    uint8_t wbuf[TEST_RW_BUFF_SIZE] = {0};
    uint8_t rbuf[TEST_RW_BUFF_SIZE] = {0};

    //test write/read flash memory begin area(0 ~ 5)
    for(count = 0; count < 5; count++)
    {
        offset = count*TEST_RW_BUFF_SIZE;
        memset(wbuf, count+1, sizeof(wbuf));
        memset(rbuf, 0, sizeof(rbuf));

        ret = exStorageWrite(0, offset, wbuf, sizeof(wbuf));
        OL_LOG_PRINTF("exStorageWrite count[%d] ret = %d", count, ret);

        osDelay(1000);

        ret = exStorageRead(0, offset, rbuf, sizeof(rbuf));
        OL_LOG_PRINTF("exStorageRead count[%d] ret = %d, r0=%d...r512=%d...r1023=%d", count, ret, rbuf[0],rbuf[512],rbuf[1023]);

        osDelay(1000);
    }

    //test write/read flash memory end area(2045 ~ 2047)
    //out of bounds will fail
    for(count=2045; count<2050; count++)
    {
        offset = count*TEST_RW_BUFF_SIZE;
        memset(wbuf, count+1, sizeof(wbuf));
        memset(rbuf, 0, sizeof(rbuf));

        ret = exStorageWrite(0, offset, wbuf, sizeof(wbuf));
        OL_LOG_PRINTF("exStorageWrite count[%d] ret = %d", count, ret);

        osDelay(1000);

        ret = exStorageRead(0, offset, rbuf, sizeof(rbuf));
        OL_LOG_PRINTF("exStorageRead count[%d] ret = %d, r0=%d...r512=%d...r1023=%d", count, ret, rbuf[0],rbuf[512],rbuf[1023]);

        osDelay(1000);
    }
}


void test_file_system(void)
{
    OSAFILE fp = NULL;
    int ret = -1;
    int size = 0;
    char buff[1024] = {0};

    ret = ol_fs_ex_mount();
    if(ret != 0)
    {
        OL_LOG_PRINTF("init mount fail, %d", ret);
        return;
    }

    fp = ol_fs_ex_open("ex_fs_test.txt", "ab+");
    if(fp == NULL)
    {
        OL_LOG_PRINTF("ol_fs_ex_open file fail");
        return;
    }

    ret = ol_fs_ex_write((void*)"0123456789", strlen("0123456789"), fp);
    OL_LOG_INFO("ol_fs_write ret = %d", ret);

    ret = ol_fs_ex_write((void*)"abcdefghij", strlen("abcdefghij"), fp);
    OL_LOG_INFO("ol_fs_write ret = %d", ret);

    size = ol_fs_ex_size(fp);
    OL_LOG_INFO("ol_fs_size = %d", size);

    ol_fs_ex_seek(fp, 5, SEEK_SET);

    ret = ol_fs_ex_read(buff, sizeof(buff), fp);
    OL_LOG_INFO("ol_fs_read size = %d, buf = %s", ret, buff);

    ret = ol_fs_ex_close(fp);
    OL_LOG_INFO("ol_fs_close ret = %d", ret);

    ol_fs_ex_unmount();
}

void ex_flash_demo(void)
{
    int ret = 0;

    exStorageInitDev(OL_SPI_INDEX_0, TEST_EXFLASH_LOAD_SIZE);
    ret = exStorageInit();
    if(EXSTO_OK != ret)
    {
        OL_LOG_PRINTF("exStorageInit error = %d", ret);
        return;
    }

    OL_LOG_PRINTF("exStorageInit Success !");

    //Erase all Flash
    exStorageClear(0, 0, TEST_EXFLASH_LOAD_SIZE);

    if(1)
    {
        test_file_system();
    }
    else
    {
        test_block_page();
    }

    ret = exStorageDeinit();
    OL_LOG_PRINTF("exStorageDeinit ret = %d", ret);
}

