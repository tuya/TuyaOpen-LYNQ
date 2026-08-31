#include "axs15231_func.h"
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "RTE_Device.h"
#include "api_i2c.h"
#include "api_tp.h"
#include "cmsis_os2.h"
#include "gpio.h"
#include "pad.h"

#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

static TpDevAxs15231_t axs15231_dev = {0};
const TsFunc_t axs15231_func;
static int16_t last_x = 0, last_y = 0;

static int axs15231_report(void) { return TRUE; }

static int axs15231_supend(void) { return TRUE; }

static int axs15231_resum(void) { return TRUE; }

static int axs15231_update_fw(uint8_t *bin_addr, uint16_t len) { return TRUE; }

static int axs15231_set_workmode(int mode, uint8_t enable) { return TRUE; }
static uint32_t axs15231_check_esd(void) { return TRUE; }

static int axs15231_prox_handle(uint8_t cmd) { return TRUE; }

static int axs15231_get_dbg_data(uint8_t *buf, uint16_t len) { return TRUE; }

static int axs15231_get_test_result(uint8_t *buf, uint16_t len) { return 0; }

static void axs15231_rst(void)
{
#ifdef TP_RST_GPIO_INSTANCE
    PadConfig_t padConfig = {0};
    PAD_getDefaultConfig(&padConfig);
    padConfig.pullUpEnable = PAD_PULL_UP_DISABLE;
    padConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    padConfig.mux = TP_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(TP_RST_PAD_INDEX, &padConfig);

    GpioPinConfig_t gpioCfg = {0};
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(TP_RST_GPIO_INSTANCE, TP_RST_GPIO_PIN, &gpioCfg);
    GPIO_pinWrite(TP_RST_GPIO_INSTANCE, 1 << TP_RST_GPIO_PIN, 0);
    osDelay(11);
    GPIO_pinWrite(TP_RST_GPIO_INSTANCE, 1 << TP_RST_GPIO_PIN,
                  1 << TP_RST_GPIO_PIN);
#endif
}

static int axs15231_scan(uint32_t usrId, int16_t *pos)
{
    uint8_t send_cmd[11] = {0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00,
                            0x00, 0x08, 0x00, 0x00, 0x00};
    uint16_t rd_len = 8;
    uint8_t read_data[8] = {0};
    uint8_t stat = 0;
    int ret = 0;
    if(!axs15231_dev.hyn_fuc_used)
    {
        EPAT_LOG(tp_scan_ax15231_not_init, P_ERROR, "axs15231 driver not init");
        return -1;
    }
    ret = api_i2c_write(axs15231_dev.i2c_id, send_cmd, 11, TRUE);
    if(ret != 0)
    {
        EPAT_LOG(axs15231_scan_wr_failed, P_ERROR,
                 "axs15231_scan write failed");
        return 0;
    }
    ret = api_i2c_read(axs15231_dev.i2c_id, read_data, rd_len, false);
    if(ret != 0)
    {
        EPAT_LOG(axs15231_scan_rd_failed, P_ERROR, "axs15231_scan read failed");
        return 0;
    }
    stat = read_data[2] >> 4;  // 8=HOLD,4=RELEASED,0=PRESSED
    int16_t Xpos = ((read_data[2] & 0x0F) << 8) + read_data[3];
    int16_t Ypos = ((read_data[4] & 0x0F) << 8) + read_data[5];
    if(last_x != Xpos || last_y != Ypos)
    {
        last_x = Xpos;
        last_y = Ypos;
        stat |= 0x01;
        if(pos != NULL)
        {
            if(Xpos <= 320) pos[0] = Xpos;
            if(Ypos <= 480) pos[1] = Ypos;
        }
    }
    else if(stat == 0x4)
    {  // RELEASED
        last_x = 0;
        last_y = 0;
        stat = 0x0;
    }
    else
    {
        EPAT_LOG(axs15231_tp_raw, P_INFO,
                 "stat 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x", stat, read_data[0],
                 read_data[1], read_data[2], read_data[3], read_data[4],
                 read_data[5]);
    }
    return stat;
}

static int axs15231_i2c_init(TpDevAxs15231_t *ts_data, uint8_t port_id,
                             uint32_t speed, void (*tp_bus_cb)(uint32_t event))
{
    api_i2c_create(port_id, NULL, &ts_data->i2c_id);
    int ret = api_i2c_open(ts_data->i2c_id, NULL, 1000);
    if(ret != 0)
    {
        EPAT_LOG(axs15231_i2c_init, P_ERROR, "axs15231_i2c_init open failed");
    }
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SPEED, &speed);
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_ISR_CB, tp_bus_cb);
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                        &ts_data->slave_addr);
    return ret;
}

TpDevAxs15231_t *axs15231_dev_get(void)
{
    if(axs15231_dev.hyn_fuc_used == NULL)
    {
        memset(&axs15231_dev, 0, sizeof(TpDevAxs15231_t));
        axs15231_dev.hyn_fuc_used = &axs15231_func;
    }

    return &axs15231_dev;
}

static void axs15231_set_i2c_addr(TpDevAxs15231_t *ts_data, uint8_t addr)
{
    ts_data->slave_addr = addr;
    api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                  &ts_data->slave_addr);
}

static int axs15231_init(TpDevAxs15231_t *ts_data)
{
    axs15231_set_i2c_addr(ts_data, AXS_ADDR);
    axs15231_rst();
    osDelay(50);
    return 0;
}

int axs15231_dev_init(uint8_t port_id, uint32_t speed,
                      void (*tp_bus_cb_func)(uint32_t event))
{
    int ret = 0;
    TpDevAxs15231_t *dev = axs15231_dev_get();
#ifdef TP_RST_PAD_INDEX
    dev->hyn_fuc_used->tp_reset();
#endif
    ret = axs15231_i2c_init(dev, port_id, speed, tp_bus_cb_func);
    if(ret != 0)
    {
        return ret;
    }
    ret = axs15231_init(dev);
    if(ret != 0)
    {
        EPAT_LOG(api_tp_create_failed, P_ERROR, "api_tp_create failed");
        return ret;
    }
    return ret;
}

const TsFunc_t axs15231_func = {.tp_reset = axs15231_rst,
                                .tp_report = axs15231_report,
                                .tp_supend = axs15231_supend,
                                .tp_resum = axs15231_resum,
                                .tp_updata_fw = axs15231_update_fw,
                                .tp_set_workmode = axs15231_set_workmode,
                                .tp_check_esd = axs15231_check_esd,
                                .tp_prox_handle = axs15231_prox_handle,
                                .tp_get_dbg_data = axs15231_get_dbg_data,
                                .tp_get_test_result = axs15231_get_test_result,
                                .tp_scan = axs15231_scan};
