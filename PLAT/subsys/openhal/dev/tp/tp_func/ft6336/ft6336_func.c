#include "ft6336_func.h"
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "RTE_Device.h"
#include "api_i2c.h"
#include "cmsis_os2.h"
#include "gpio.h"
#include "pad.h"

static TpDevFt6336_t ft6336_dev = {0};
const TsFunc_t ft6336_func;
uint8_t ft6336_i2c_port = 0;
static I2cSpeed_e ft6336_i2c_speed = OPEN_I2C_SPEED_400KHZ;
static void (*ft6336_bus_cb)(uint32_t event) = NULL;
static void ft6336_set_i2c_addr(TpDevFt6336_t *ts_data, uint8_t addr);
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

static int ft6336_report(void) { return TRUE; }

static int ft6336_supend(void) { 
    uint8_t wbuf[4] = {0};
    int ret = 0;
    wbuf[0] = FT6336_OPERATE_MODE;
    wbuf[1] = 0x00;
    ret = api_i2c_write(ft6336_dev.i2c_id, wbuf, 2, TRUE);
    if(ret != 0)
    {
        EPAT_LOG(ft6336_supend_failed, P_ERROR, "ft6336_supend write failed");
        return 0;
    }
    return TRUE; 
}

static int ft6336_resum(void) { 
    uint8_t wbuf[4] = {0};
    int ret = 0;
    wbuf[0] = FT6336_OPERATE_MODE;
    wbuf[1] = 0x01;
    ret = api_i2c_write(ft6336_dev.i2c_id, wbuf, 2, TRUE);
    if(ret != 0)
    {
        EPAT_LOG(ft6336_resum_failed, P_ERROR, "ft6336_supend write failed");
        return 0;
    }
    return TRUE; 
}

static int ft6336_update_fw(uint8_t *bin_addr, uint16_t len) { return TRUE; }

static int ft6336_set_workmode(int mode, uint8_t enable) { return TRUE; }
static uint32_t ft6336_check_esd(void) { return TRUE; }

static int ft6336_prox_handle(uint8_t cmd) { return TRUE; }

static int ft6336_get_dbg_data(uint8_t *buf, uint16_t len) { return TRUE; }

static int ft6336_get_test_result(uint8_t *buf, uint16_t len) { return 0; }

static void ft6336_rst(void)
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

static int ft6336_scan(uint32_t usrId, int16_t *pos)
{
    int fingerNum = 0;
    uint8_t temp[4] = {0};
    uint8_t wbuf[4] = {0};
    uint32_t rlen = 0;
    int ret = 0;
    wbuf[0] = FT6336_GET_FINGERNUM;
    ret = api_i2c_open(ft6336_dev.i2c_id, NULL, 1000);
    if(ret != 0)
    {
        //为了规避其他外设由于复用I2C端口，导致I2C端口被关闭，如果打开失败则重新创建I2C。
        api_i2c_create(ft6336_i2c_port, NULL, &ft6336_dev.i2c_id);
        if(ft6336_dev.i2c_id != 0)
        {
            ret = api_i2c_open(ft6336_dev.i2c_id, NULL, 1000);
            if(ret != 0)
            {
                EPAT_LOG(ft6336_scan_failed2, P_ERROR,
                         "ft6336_scan open failed");
                return ret;
            }
        }
        else
        {
            EPAT_LOG(ft6336_scan_failed3, P_ERROR,
                     "ft6336_scan create i2c failed");
            return ret;
        }
    }
    ret = api_i2c_ioctl(ft6336_dev.i2c_id, OPEN_I2C_IOCTL_SPEED, &ft6336_i2c_speed);
    ret = api_i2c_ioctl(ft6336_dev.i2c_id, OPEN_I2C_IOCTL_ISR_CB, ft6336_bus_cb);
    ret = api_i2c_ioctl(ft6336_dev.i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                        &ft6336_dev.slave_addr);
    ft6336_set_i2c_addr(&ft6336_dev, FT6336_ADDR);
    ret = api_i2c_write(ft6336_dev.i2c_id, wbuf, 1, TRUE);
    if(ret != 0)
    {
        EPAT_LOG(ft6336_scan_wr_failed, P_ERROR, "ft6336_scan write failed");
        return 0;
    }
    rlen = 1;
    ret = api_i2c_read(ft6336_dev.i2c_id, temp, rlen, false);
    if(ret != 0)
    {
        EPAT_LOG(ft6336_scan_rd_failed, P_ERROR, "ft6336_scan read failed");
        return 0;
    }
    fingerNum = temp[0];
    if(fingerNum)
    {
        wbuf[0] = FT6336_GET_LOC0;
        ret = api_i2c_write(ft6336_dev.i2c_id, wbuf, 1, TRUE);
        if(ret != 0)
        {
            EPAT_LOG(ft6336_scan_wr2_failed, P_ERROR,
                     "ft6336_scan write2 failed");
            return 0;
        }
        rlen = 4;
        ret = api_i2c_read(ft6336_dev.i2c_id, temp, rlen, false);
        if(ret != 0)
        {
            EPAT_LOG(ft6336_scan_rd2_failed, P_ERROR,
                     "ft6336_scan read2 failed");
            return 0;
        }
        if(pos != NULL)
        {
            pos[0] = ((uint16_t)(temp[0] & 0x0F) << 8) + temp[1];
            pos[1] = (((uint16_t)(temp[2] & 0x0F) << 8) + temp[3]);
        }
    }
    api_i2c_close(ft6336_dev.i2c_id);
    //EPAT_LOG(tp_scan_ft6336, P_INFO, "0x%x,0x%x,x%d,y%d",usrId,ft6336_dev.i2c_id,pos[0],pos[1]);
    return fingerNum;
}

static int ft6336_i2c_init(TpDevFt6336_t *ts_data, uint8_t port_id,
                           uint32_t speed, void (*tp_bus_cb)(uint32_t event))
{
    api_i2c_create(port_id, NULL, &ts_data->i2c_id);
    ft6336_i2c_port = port_id;
    ft6336_i2c_speed = speed;
    ft6336_bus_cb = tp_bus_cb;
    return 0;
}

TpDevFt6336_t *ft6336_dev_get(void)
{
    if(ft6336_dev.hyn_fuc_used == NULL)
    {
        memset(&ft6336_dev, 0, sizeof(TpDevFt6336_t));
        ft6336_dev.hyn_fuc_used = &ft6336_func;
    }

    return &ft6336_dev;
}

static void ft6336_set_i2c_addr(TpDevFt6336_t *ts_data, uint8_t addr)
{
    ts_data->slave_addr = addr;
    api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                  &ts_data->slave_addr);
}

static int ft6336_init(TpDevFt6336_t *ts_data)
{
    ft6336_set_i2c_addr(ts_data, FT6336_ADDR);
    ft6336_rst();
    osDelay(50);
    return 0;
}

int ft6336_dev_init(uint8_t port_id, uint32_t speed,
                    void (*tp_bus_cb_func)(uint32_t event))
{
    int ret = 0;
    TpDevFt6336_t *dev = ft6336_dev_get();
#ifdef TP_RST_PAD_INDEX
    dev->hyn_fuc_used->tp_reset();
#endif
    ret = ft6336_i2c_init(dev, port_id, speed, tp_bus_cb_func);
    if(ret != 0)
    {
        return ret;
    }
    ret = ft6336_init(dev);
    if(ret != 0)
    {
        EPAT_LOG(api_tp_create_failed, P_ERROR, "api_tp_create failed");
        return ret;
    }
    return ret;
}

const TsFunc_t ft6336_func = {.tp_reset = ft6336_rst,
                              .tp_report = ft6336_report,
                              .tp_supend = ft6336_supend,
                              .tp_resum = ft6336_resum,
                              .tp_updata_fw = ft6336_update_fw,
                              .tp_set_workmode = ft6336_set_workmode,
                              .tp_check_esd = ft6336_check_esd,
                              .tp_prox_handle = ft6336_prox_handle,
                              .tp_get_dbg_data = ft6336_get_dbg_data,
                              .tp_get_test_result = ft6336_get_test_result,
                              .tp_scan = ft6336_scan};
