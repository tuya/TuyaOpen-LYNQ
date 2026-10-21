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
#include "gpio.h"
#include "pad.h"

#define SPI0_FLASH_JEDEC_TEST   1

#if SPI0_FLASH_JEDEC_TEST
/* SPI0 CS = PAD23 = GPIO instance 0, index 8 */
#define SPI0_CS_GPIO_INSTANCE   0
#define SPI0_CS_GPIO_INDEX      8
#define SPI0_CS_PAD_NUM         23

static void spi0_cs_init(void)
{
    PadConfig_t cfg;
    PAD_getDefaultConfig(&cfg);
    cfg.mux = PAD_MUX_ALT0;  /* GPIO function */
    PAD_setPinConfig(SPI0_CS_PAD_NUM, &cfg);

    GpioPinConfig_t gpioConfig;
    gpioConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioConfig.misc.initOutput = 1;  /* CS high (inactive) */
    GPIO_pinConfig(SPI0_CS_GPIO_INSTANCE, SPI0_CS_GPIO_INDEX, &gpioConfig);
}

static void spi0_cs_low(void)
{
    GPIO_pinWrite(SPI0_CS_GPIO_INSTANCE, 1 << SPI0_CS_GPIO_INDEX, 0);
}

static void spi0_cs_high(void)
{
    GPIO_pinWrite(SPI0_CS_GPIO_INSTANCE, 1 << SPI0_CS_GPIO_INDEX, 1 << SPI0_CS_GPIO_INDEX);
}
#endif

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

#if SPI0_FLASH_JEDEC_TEST
    unsigned char cmd[4] = {0x9F, 0x00, 0x00, 0x00};
    unsigned char buf[4] = {0};

    OL_LOG_PRINTF("spi_demo: read flash JEDEC ID via SPI0");

    /* Init CS as GPIO output high */
    spi0_cs_init();

    config.arg = 1000000U;
    config.cb_event = NULL;
    config.control = (ARM_SPI_MODE_MASTER | \
                      ARM_SPI_CPOL0_CPHA0 | \
                      ARM_SPI_DATA_BITS(8) | \
                      ARM_SPI_MSB_LSB | \
                      ARM_SPI_SS_MASTER_SW);

    ret = ol_spi_init(OL_SPI_INDEX_0, &config);
    OL_LOG_PRINTF("ol_spi_init(SPI0) ret = %d", ret);
    if(0 != ret)
    {
        OL_LOG_PRINTF("spi0 init fail, return");
        return;
    }

    /* CS low -> transfer -> CS high */
    spi0_cs_low();
    ret = ol_spi_write_read(OL_SPI_INDEX_0, cmd, buf, 4);
    spi0_cs_high();

    OL_LOG_PRINTF("ol_spi_write_read ret = %d", ret);
    OL_LOG_PRINTF("JEDEC ID: %02X %02X %02X", buf[1], buf[2], buf[3]);

    ret = ol_spi_deinit(OL_SPI_INDEX_0);
    OL_LOG_PRINTF("ol_spi_deinit ret = %d", ret);

#else
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
#endif
}

