/****************************************************************************
 *
 ****************************************************************************/
#include "stdio.h"
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "bsp.h"
#include "ol_spi_api.h"
#include "ol_log.h"

static volatile bool spi_master_transfer_done;

void test_spi_master_event_cb(uint32_t event)
{
    MBTK_LOG_INFO("spi master event = %ld", event);
    if(event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        spi_master_transfer_done = true;
    }
}

void spi_demo(void)
{
    int ret = -1;
    ol_spi_config_t config = {0x0};
    unsigned char buf_r[] = "1234567890";
    unsigned char buf_w[] = "ABCDEFGHJK";

    config.arg = 200000U;
    config.cb_event = test_spi_master_event_cb;
    config.control = (ARM_SPI_MODE_MASTER | \
                      ARM_SPI_CPOL1_CPHA0 | \
                      ARM_SPI_DATA_BITS(8) | \
                      ARM_SPI_MSB_LSB | \
                      ARM_SPI_SS_MASTER_UNUSED);

    ret = ol_spi_init(OL_SPI_INDEX_1, &config);
    if(0 != ret)
    {
        OL_LOG_INFO("spi init fail ret = %d, return", ret);
        return;
    }

    spi_master_transfer_done = false;

    ret = ol_spi_write_read(OL_SPI_INDEX_1, buf_w, buf_r, sizeof(buf_r));
    OL_LOG_INFO("spi write and read ret = %d, write data = %s, read data = %s", ret, buf_w, buf_r);

    while(false == spi_master_transfer_done)
    {
        osDelay(1000);
    }

    ret = ol_spi_write(OL_SPI_INDEX_1, buf_w, sizeof(buf_w));
    OL_LOG_INFO("spi write ret = %d, data = %s", ret, buf_w);

    ret = ol_spi_read(OL_SPI_INDEX_1, buf_r, sizeof(buf_r));
    OL_LOG_INFO("spi read ret = %d, data = %s", ret, buf_r);

    ret = ol_spi_deinit(OL_SPI_INDEX_1);
    OL_LOG_INFO("spi deinit ret = %d", ret);
}

