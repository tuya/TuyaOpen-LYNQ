#include "st77922_func.h"
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "RTE_Device.h"
#include "api_i2c.h"
#include "cmsis_os2.h"
#include "gpio.h"
#include "pad.h"

static TpDevSt77922_t st77922_dev = {0};
const TsFunc_t st77922_func;
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

static int st77922_report(void) { return TRUE; }

static int st77922_supend(void) { return TRUE; }

static int st77922_resum(void) { return TRUE; }

static int st77922_update_fw(uint8_t *bin_addr, uint16_t len) { return TRUE; }

static int st77922_set_workmode(int mode, uint8_t enable) { return TRUE; }
static uint32_t st77922_check_esd(void) { return TRUE; }

static int st77922_prox_handle(uint8_t cmd) { return TRUE; }

static int st77922_get_dbg_data(uint8_t *buf, uint16_t len) { return TRUE; }

static int st77922_get_test_result(uint8_t *buf, uint16_t len) { return 0; }

static void st77922_rst(void)
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

static int st77922_scan(uint32_t usrId, int16_t *pos)
{
    uint8_t read_data[18] = {0xFF};
    uint8_t wbuf[4] = {0};
    uint32_t rlen = 0;
    int ret = 0;
    uint8_t stat = 0;
    wbuf[0] = 0x00;
    wbuf[1] = 0x10;
    ret = api_i2c_write(st77922_dev.i2c_id, wbuf, 2, TRUE);
    if(ret != 0)
    {
        EPAT_LOG(st77922_scan_wr_failed, P_ERROR, "st77922_scan write failed");
        return 0;
    }
    rlen = 18;
    ret = api_i2c_read(st77922_dev.i2c_id, read_data, rlen, false);
    if(ret != 0)
    {
        EPAT_LOG(st77922_scan_rd_failed, P_ERROR, "st77922_scan read failed");
        return 0;
    }
    int16_t Xpos = 0;
    int16_t Ypos = 0;
    if(read_data[0] & 0x08)
    {
        for(uint8_t i = 0; i < 2; i++)
        {
            uint8_t *p = &read_data[4 + i * 7];
            if(*p & 0x80)
            {
                stat |= 0x01;
                Xpos = (uint16_t)(((uint16_t)(*p & 0x3F) << 8) |
                                  ((uint16_t) * (p + 1) & 0xFF));
                Ypos = (uint16_t)(((uint16_t)(*(p + 2) & 0x3F) << 8) |
                                  ((uint16_t) * (p + 3) & 0xFF));
                if(pos != NULL)
                {
                    if(Xpos <= 480) pos[i * 2 + 0] = Xpos;
                    if(Ypos <= 480) pos[i * 2 + 1] = Ypos;
                }
            }
        }
        EPAT_LOG(tpScan_st77922, P_INFO, "[%d]addr=0x%x, 0x%x", osKernelGetTickCount(),
                 ST77922_ADDR, read_data[0]);
    }
    // EPAT_LOG(tp_scan_st77922, P_INFO,
    // "0x%x,0x%x,0x%x,x%d,y%d",usrId,tpDev.tpUsrId,tpDev.i2cUsrId,pos[0],pos[1]);
    return stat;
}

static int st77922_i2c_init(TpDevSt77922_t *ts_data, uint8_t port_id,
                            uint32_t speed, void (*tp_bus_cb)(uint32_t event))
{
    api_i2c_create(port_id, NULL, &ts_data->i2c_id);
    int ret = api_i2c_open(ts_data->i2c_id, NULL, 1000);
    if(ret != 0)
    {
        EPAT_LOG(st77922_i2c_init, P_ERROR, "st77922_i2c_init open failed");
    }
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SPEED, &speed);
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_ISR_CB, tp_bus_cb);
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                        &ts_data->slave_addr);
    return ret;
}

TpDevSt77922_t *st77922_dev_get(void)
{
    if(st77922_dev.hyn_fuc_used == NULL)
    {
        memset(&st77922_dev, 0, sizeof(TpDevSt77922_t));
        st77922_dev.hyn_fuc_used = &st77922_func;
    }

    return &st77922_dev;
}

static void st77922_set_i2c_addr(TpDevSt77922_t *ts_data, uint8_t addr)
{
    ts_data->slave_addr = addr;
    api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                  &ts_data->slave_addr);
}

static int st77922_init(TpDevSt77922_t *ts_data)
{
    st77922_set_i2c_addr(ts_data, ST77922_ADDR);
    st77922_rst();
    osDelay(50);
    return 0;
}

int st77922_dev_init(uint8_t port_id, uint32_t speed,
                     void (*tp_bus_cb_func)(uint32_t event))
{
    int ret = 0;
    TpDevSt77922_t *dev = st77922_dev_get();
#ifdef TP_RST_PAD_INDEX
    dev->hyn_fuc_used->tp_reset();
#endif
    ret = st77922_i2c_init(dev, port_id, speed, tp_bus_cb_func);
    if(ret != 0)
    {
        return ret;
    }
    ret = st77922_init(dev);
    if(ret != 0)
    {
        EPAT_LOG(api_tp_create_failed, P_ERROR, "api_tp_create failed");
        return ret;
    }
    return ret;
}

const TsFunc_t st77922_func = {.tp_reset = st77922_rst,
                               .tp_report = st77922_report,
                               .tp_supend = st77922_supend,
                               .tp_resum = st77922_resum,
                               .tp_updata_fw = st77922_update_fw,
                               .tp_set_workmode = st77922_set_workmode,
                               .tp_check_esd = st77922_check_esd,
                               .tp_prox_handle = st77922_prox_handle,
                               .tp_get_dbg_data = st77922_get_dbg_data,
                               .tp_get_test_result = st77922_get_test_result,
                               .tp_scan = st77922_scan};