/**
 * @file tkl_spi.c
 * @brief SPI adapter for L511C-Y7PVM (SPI0: PAD23-CS, PAD24-MOSI, PAD25-MISO, PAD26-SCLK)
 * @version 0.1
 * @date 2026-05-29
 */

#include "tuya_cloud_types.h"

#define ENABLE_SPI 1
#define ENABLE_SPI_TEST 0
#define USE_BSP_SPI_DIRECT 0

#if defined(ENABLE_SPI) && (ENABLE_SPI == 1)

#include <string.h>
#include "tkl_spi.h"
#include "tkl_output.h"
#include "tkl_mutex.h"
#include "tkl_semaphore.h"
#include "tkl_memory.h"
#include "tkl_system.h"

#include "Driver_SPI.h"
#include "Driver_Common.h"
#include "cmsis_os2.h"
#include "ol_spi_api.h"

#define LOGD(fmt, args...) tkl_log_output("[tkl_spi][DBG/%d] " fmt "\n", __LINE__, ##args)
#define LOGE(fmt, args...) tkl_log_output("[tkl_spi][ERR/%d] " fmt "\n", __LINE__, ##args)

#define SPI_DEV_NUM 2
#define SPI_ONCE_SEND_LEN  (4 * 1024)      //DMA 单次发送最大4k

extern ARM_DRIVER_SPI Driver_SPI0;
extern ARM_DRIVER_SPI Driver_SPI1;

static ARM_DRIVER_SPI *spi_drv[] = {&Driver_SPI0, &Driver_SPI1};

typedef struct {
    bool init_flag;
    bool is_irq_enable;
    bool dma_mode;
    ol_spi_index_num port_id;
    TUYA_SPI_IRQ_CB cb;
    TKL_MUTEX_HANDLE mutex;
    uint8_t         *temp_buffer;
    volatile bool   transfer_done;
} spi_dev_cfg_t;

static spi_dev_cfg_t spi_dev[SPI_DEV_NUM];

static void spi0_event_cb(uint32_t event)
{
    if (spi_dev[0].init_flag && (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)) {
        // LOGD("spi0_event_cb, event=%d", event);
        spi_dev[0].transfer_done = true;
    }
}

static void spi1_event_cb(uint32_t event)
{
    if (spi_dev[1].init_flag && (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)) {
        // LOGD("spi1_event_cb, event=%d", event);
        spi_dev[1].transfer_done = true;
    }
}

OPERATE_RET tkl_spi_init(TUYA_SPI_NUM_E port, CONST TUYA_SPI_BASE_CFG_T *cfg)
{
    if (port >= SPI_DEV_NUM || cfg == NULL) {
        return OPRT_INVALID_PARM;
    }
#if defined(ENABLE_EXT_FLASH) && (ENABLE_EXT_FLASH == 1)
    if (port == EXT_FLASH_SPI_INDEX) {
        LOGE("SPI%d is reserved for ext flash", EXT_FLASH_SPI_INDEX);
        return OPRT_NOT_SUPPORTED;
    }
#endif
    if (cfg->role != TUYA_SPI_ROLE_MASTER) {
        return OPRT_NOT_SUPPORTED;
    }
    if (spi_dev[port].init_flag) {
        return OPRT_OK;
    }

    memset(&spi_dev[port], 0, sizeof(spi_dev_cfg_t));

    uint32_t control = ARM_SPI_MODE_MASTER;

    switch (cfg->mode) {
        case TUYA_SPI_MODE0: control |= ARM_SPI_CPOL0_CPHA0; break;
        case TUYA_SPI_MODE1: control |= ARM_SPI_CPOL0_CPHA1; break;
        case TUYA_SPI_MODE2: control |= ARM_SPI_CPOL1_CPHA0; break;
        case TUYA_SPI_MODE3: control |= ARM_SPI_CPOL1_CPHA1; break;
        default: control |= ARM_SPI_CPOL0_CPHA0; break;
    }

    control |= ARM_SPI_DATA_BITS(cfg->databits == TUYA_SPI_DATA_BIT16 ? 16 : 8);
    control |= (cfg->bitorder == TUYA_SPI_ORDER_LSB2MSB) ? ARM_SPI_LSB_MSB : ARM_SPI_MSB_LSB;
    control |= ARM_SPI_SS_MASTER_SW;

    ol_spi_config_t spi_cfg = {
        .control = control,
        .arg = cfg->freq_hz,
        .cb_event = (port == TUYA_SPI_NUM_0) ? spi0_event_cb : spi1_event_cb,
    };

    ol_spi_index_num id = (port == TUYA_SPI_NUM_0) ? OL_SPI_INDEX_0 : OL_SPI_INDEX_1;
    int ret = ol_spi_init(id, &spi_cfg);
    if (ret != 0) {
        LOGE("ol_spi_init(%d) fail, ret=%d", id, ret);
        return OPRT_COM_ERROR;
    }

    tkl_mutex_create_init(&spi_dev[port].mutex);
    spi_dev[port].dma_mode = true;  // SPI0 and SPI1 both use DMA mode
    spi_dev[port].init_flag = true;
    spi_dev[port].port_id = id;
    spi_dev[port].temp_buffer = (uint8_t *)tkl_system_malloc(SPI_ONCE_SEND_LEN);

    LOGD("spi%d init OK, freq=%d, mode=%d", port, cfg->freq_hz, cfg->mode);
    return OPRT_OK;
}

OPERATE_RET tkl_spi_deinit(TUYA_SPI_NUM_E port)
{
    if (port >= SPI_DEV_NUM) {
        return OPRT_INVALID_PARM;
    }
    if (!spi_dev[port].init_flag) {
        return OPRT_OK;
    }

    ol_spi_deinit(spi_dev[port].port_id);
    if (spi_dev[port].mutex) {
        tkl_mutex_release(spi_dev[port].mutex);
    }

    if (spi_dev[port].temp_buffer) {
        tkl_system_free(spi_dev[port].temp_buffer);
        spi_dev[port].temp_buffer = NULL;
    }

    spi_dev[port].init_flag = false;
    return OPRT_OK;
}

OPERATE_RET tkl_spi_transfer(TUYA_SPI_NUM_E port, VOID_T* send_buf, VOID_T* receive_buf, UINT32_T length)
{
    if (port > SPI_DEV_NUM || length == 0 || (send_buf == NULL && receive_buf == NULL)) { 
        return OPRT_INVALID_PARM;
    }
    
    if(!spi_dev[port].init_flag)
        return OPRT_COM_ERROR;

    void *s_buf = send_buf;
    void *r_buf = receive_buf;

    if(s_buf == NULL) {
        s_buf = spi_dev[port].temp_buffer;
    }
    if(r_buf == NULL) {
        r_buf = spi_dev[port].temp_buffer;
    }

    tkl_mutex_lock(spi_dev[port].mutex);
    spi_drv[port]->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
    int total_send_len = 0;
    int total_rcv_len = 0;
    int total_xfer_len = 0;
    OPERATE_RET ret = OPRT_OK;
    while(total_xfer_len < length) {
        spi_dev[port].transfer_done = false;
        int xfer_len = length - total_xfer_len > SPI_ONCE_SEND_LEN ? SPI_ONCE_SEND_LEN : length - total_xfer_len;
        ret = ol_spi_write_read(spi_dev[port].port_id, (unsigned char *)s_buf+total_send_len, (unsigned char *)r_buf+total_rcv_len, xfer_len);
        if(ret == 0) {
            ARM_SPI_STATUS status = spi_drv[port]->GetStatus();
            if (status.busy) {
                SYS_TICK_T start_tick = tkl_system_get_tick_count();
                while(!spi_dev[port].transfer_done) {
                    if (tkl_system_get_tick_count() - start_tick > 1000) {
                        LOGE("spi_transfer timeout, port=%d, xfer_len=%d", port, xfer_len);
                        ret = OPRT_COM_ERROR;
                        break;
                    }
                }
                if (ret != 0) break;
            }
        } else {
            LOGE("spi_transfer fail, port=%d, len=%d, ret=%d", port, length, ret);
            break;
        }
        total_xfer_len += xfer_len;
        if(send_buf)
            total_send_len += xfer_len;
        if(receive_buf)
            total_rcv_len += xfer_len;
    }
    spi_drv[port]->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
    tkl_mutex_unlock(spi_dev[port].mutex);

    if(ret == OPRT_OK && spi_dev[port].is_irq_enable && (spi_dev[port].cb != NULL) && send_buf) {
        spi_dev[port].cb(port, TUYA_SPI_EVENT_TX_COMPLETE);
    }

    // LOGD("spi_transfer %d %d %d", port, length, ret);
    return ret;
}

OPERATE_RET tkl_spi_send(TUYA_SPI_NUM_E port, VOID_T *data, UINT32_T size)
{
    return tkl_spi_transfer(port, data, NULL, size);
}

OPERATE_RET tkl_spi_recv(TUYA_SPI_NUM_E port, VOID_T *data, UINT32_T size)
{
    if (port >= SPI_DEV_NUM || data == NULL || size == 0) {
        return OPRT_INVALID_PARM;
    }
    if (!spi_dev[port].init_flag) {
        return OPRT_COM_ERROR;
    }

    tkl_mutex_lock(spi_dev[port].mutex);
    // spi_cs_low(port);
    int ret = ol_spi_read(spi_dev[port].port_id, (unsigned char *)data, size);
    // spi_cs_high(port);
    tkl_mutex_unlock(spi_dev[port].mutex);

    if (ret != 0) {
        LOGE("spi_recv fail, port=%d, size=%d, ret=%d", port, size, ret);
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_spi_transfer_with_length(TUYA_SPI_NUM_E port, VOID_T* send_buf, UINT32_T send_len, VOID_T* receive_buf, UINT32_T receive_len)
{
    if (send_len != receive_len) {
        return OPRT_INVALID_PARM;
    }
    return tkl_spi_transfer(port, send_buf, receive_buf, send_len);
}

OPERATE_RET tkl_spi_abort_transfer(TUYA_SPI_NUM_E port)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_get_status(TUYA_SPI_NUM_E port, TUYA_SPI_STATUS_T *status)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_irq_init(TUYA_SPI_NUM_E port, TUYA_SPI_IRQ_CB cb)
{
    if (port >= SPI_DEV_NUM || cb == NULL) return OPRT_INVALID_PARM;
    if (!spi_dev[port].init_flag) return OPRT_COM_ERROR;
    spi_dev[port].cb = cb;
    return OPRT_OK;
}

OPERATE_RET tkl_spi_irq_enable(TUYA_SPI_NUM_E port)
{
    if (port >= SPI_DEV_NUM) return OPRT_INVALID_PARM;
    if (!spi_dev[port].init_flag) return OPRT_COM_ERROR;
    spi_dev[port].is_irq_enable = true;
    return OPRT_OK;
}

OPERATE_RET tkl_spi_irq_disable(TUYA_SPI_NUM_E port)
{
    if (port >= SPI_DEV_NUM) return OPRT_INVALID_PARM;
    if (!spi_dev[port].init_flag) return OPRT_COM_ERROR;
    spi_dev[port].is_irq_enable = false;
    return OPRT_OK;
}

INT32_T tkl_spi_get_data_count(TUYA_SPI_NUM_E port)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_ioctl(TUYA_SPI_NUM_E port, UINT32_T cmd, VOID *args)
{
    return OPRT_NOT_SUPPORTED;
}

UINT32_T tkl_spi_get_max_dma_data_length(VOID_T)
{
    return (200 * 1024);
}

/*============================================================================
 * Test: read JEDEC ID from external flash via tkl_spi on SPI0
 * Winbond 25Q128: JEDEC ID = 0xEF 0x40 0x18
 *============================================================================*/
#if defined(ENABLE_SPI_TEST) && (ENABLE_SPI_TEST == 1)

void tkl_spi_test(void)
{
    OPERATE_RET ret;
    uint8_t cmd[4] = {0x9F, 0x00, 0x00, 0x00};  // JEDEC Read ID command
    uint8_t buf[4] = {0};

    osDelay(3000);
    LOGD("===== tkl_spi Test Start =====");

    // Init SPI0: Mode0, 8bit, MSB first, 1MHz
    TUYA_SPI_BASE_CFG_T cfg = {
        .role = TUYA_SPI_ROLE_MASTER,
        .mode = TUYA_SPI_MODE0,
        .databits = TUYA_SPI_DATA_BIT8,
        .bitorder = TUYA_SPI_ORDER_MSB2LSB,
        .freq_hz = 1000000,
    };

    LOGD("[1] tkl_spi_init(SPI0)...");
    ret = tkl_spi_init(TUYA_SPI_NUM_0, &cfg);
    if (ret != OPRT_OK) {
        LOGE("[1] init FAIL, ret=%d", ret);
        return;
    }
    LOGD("[1] init OK");

    // Read JEDEC ID via transfer (full-duplex)
    LOGD("[2] tkl_spi_transfer: send 0x9F + 3 dummy...");
    ret = tkl_spi_transfer(TUYA_SPI_NUM_0, cmd, buf, 4);
    if (ret != OPRT_OK) {
        LOGE("[2] transfer FAIL, ret=%d", ret);
        goto exit;
    }
    LOGD("[2] JEDEC ID (transfer): %02X %02X %02X", buf[1], buf[2], buf[3]);

    // Verify: Winbond 25Q128 = EF 40 18
    if (buf[1] == 0xEF && buf[2] == 0x40 && buf[3] == 0x18) {
        LOGD("[2] *** JEDEC ID MATCH: Winbond 25Q128JWSQ ***");
    } else if (buf[1] != 0x00 && buf[1] != 0xFF) {
        LOGD("[2] Got valid ID (non-Winbond): %02X %02X %02X", buf[1], buf[2], buf[3]);
    } else {
        LOGE("[2] *** ID INVALID - check wiring ***");
    }

    // Also test send+recv separately
    LOGD("[3] tkl_spi_send + tkl_spi_recv...");
    uint8_t cmd_byte = 0x9F;
    uint8_t id_buf[3] = {0};
    ret = tkl_spi_send(TUYA_SPI_NUM_0, &cmd_byte, 1);
    if (ret != OPRT_OK) {
        LOGE("[3] send FAIL, ret=%d", ret);
        goto exit;
    }
    ret = tkl_spi_recv(TUYA_SPI_NUM_0, id_buf, 3);
    if (ret != OPRT_OK) {
        LOGE("[3] recv FAIL, ret=%d", ret);
        goto exit;
    }
    LOGD("[3] JEDEC ID (send+recv): %02X %02X %02X", id_buf[0], id_buf[1], id_buf[2]);

exit:
    LOGD("[4] tkl_spi_deinit...");
    tkl_spi_deinit(TUYA_SPI_NUM_0);
    LOGD("===== tkl_spi Test Complete =====");
}

#endif /* ENABLE_SPI_TEST */

#endif /* ENABLE_SPI */
