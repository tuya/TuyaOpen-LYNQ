#include "cst816_func.h"
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

static TpDevCst816_t *hyn_816dev = NULL;
static int cst816_enter_boot(void);
static void cst816_set_i2c_addr(TpDevCst816_t *ts_data, uint8_t addr);
static void cst816_set_i2c_addr(TpDevCst816_t *ts_data, uint8_t addr);
static int cst816_write_data(TpDevCst816_t *ts_data, uint8_t *buf,
                             uint16_t len);
static void cst816_rst(void);
static int cst816_updata_tpinfo(void);
static int cst816_set_workmode(int mode, uint8_t enable);
static uint32_t cst816_read_checksum(void);
static int cst816_wr_reg(TpDevCst816_t *ts_data, uint32_t reg_addr,
                         uint8_t reg_len, uint8_t *rbuf, uint16_t rlen);
static TpDevCst816_t cst816_dev = {0};
const TsFunc_t cst816_func;
#ifdef TP_POWON_CHECK_UPDATE
#include "cst816_fw0.h"
#include "cst816_fw1.h"
#include "cst816_fw2.h"
static int cst816_updata_judge(uint8_t *p_fw, uint16_t len);
int cst816_init(TpDevCst816_t *ts_data, uint8_t fw_id)
{
    int ret = 0;
    uint8_t buf[4];
    uint8_t *fw_addr = NULL;
    uint32_t fw_size = 0;
    EPAT_LOG(cst816_init, P_INFO, "cst816_init enter");
    hyn_816dev = ts_data;
    ret = cst816_enter_boot();
    if(ret == FALSE)
    {
        EPAT_LOG(cst816_init_err_enterboot, P_ERROR,
                 "cst816_enter_boot failed");
        return -1;
    }
    if(fw_id == 0)
    {
        fw_addr = (uint8_t *)fw_bin_0;
        fw_size = CST816_BIN_SIZE;
    }
    else if(fw_id == 1)
    {
        fw_addr = (uint8_t *)fw_bin_1;
        fw_size = CST816_BIN_SIZE;
    }
    else if(fw_id == 2)
    {
        fw_addr = (uint8_t *)fw_bin_2;
        fw_size = CST816_BIN_SIZE;
    }
    hyn_816dev->fw_updata_addr = fw_addr;
    hyn_816dev->fw_updata_len = fw_size;
    EPAT_LOG(cst816_init_enterboot, P_INFO, "cst816_enter_boot success");
    hyn_816dev->hw_info.ic_fw_checksum = cst816_read_checksum();

    if(hyn_816dev->need_update_fw == 0)
    {
        cst816_wr_reg(hyn_816dev, 0xA006EE, 3, buf, 0);
        cst816_rst();
        osDelay(100);
        cst816_set_i2c_addr(hyn_816dev, MAIN_I2C_ADDR);
        ret = cst816_updata_tpinfo();
        cst816_set_workmode(NOMAL_MODE, 0);
        hyn_816dev->need_update_fw = cst816_updata_judge(fw_addr, fw_size);
    }
    if(hyn_816dev->need_update_fw)
    {
        EPAT_LOG(cst816_init_update_need, P_INFO, "need updata FW !!!");
    }
    else
    {
        EPAT_LOG(cst816_init_update_noneed, P_INFO, "no need updata FW");
    }
    return 0;
}

static int cst816_updata_judge(uint8_t *p_fw, uint16_t len)
{
    uint32_t f_checksum, f_fw_ver, f_ictype, f_fw_project_id;
    uint8_t *p_data = p_fw;
    TpInfo_t *ic = &hyn_816dev->hw_info;

    f_checksum = U8TO16(p_data[5], p_data[4]);

    p_data = p_fw + 6 + CST816_BIN_SIZE - 1 - 15;

    f_ictype = p_data[0];
    f_fw_project_id = (p_data[1] << 24) + (p_data[2] << 16) + (p_data[3] << 8) +
                      (p_data[4] << 0);
    f_fw_ver = p_data[5];

    EPAT_LOG(cst816_updata_judge, P_INFO,
             "Bin_info fw_project_id:%x ictype:%x fw_ver:%x checksum:%x",
             f_fw_project_id, f_ictype, f_fw_ver, f_checksum);

    if(f_checksum != ic->ic_fw_checksum)
    {
        return 1;  // need updata
    }
    return 0;
}
#else
int cst816_init(TpDevCst816_t *ts_data, uint8_t fw_id)
{
    int ret = 0;
    hyn_816dev = ts_data;
    cst816_set_i2c_addr(hyn_816dev, MAIN_I2C_ADDR);
    cst816_rst();
    osDelay(100);
    ret = cst816_updata_tpinfo();
    ret |= cst816_set_workmode(NOMAL_MODE, 0);
    return ret == TRUE ? 0 : -1;
}
#endif

static int cst816_enter_boot(void)
{
    uint8_t t;
    cst816_set_i2c_addr(hyn_816dev, BOOT_I2C_ADDR);
    for(t = 5;; t += 2)
    {
        int ok = FALSE;
        uint8_t i2c_buf[4] = {0};

        if(t >= 15)
        {
            return FALSE;
        }

        cst816_rst();
        osDelay(t);
        ok = cst816_wr_reg(hyn_816dev, 0xA001AB, 3, i2c_buf, 0);
        if(ok != 0)
        {
            continue;
        }

        ok = cst816_wr_reg(hyn_816dev, 0xA003, 2, i2c_buf, 1);
        if(ok != 0)
        {
            continue;
        }

        if(i2c_buf[0] != 0xC1)
        {
            continue;
        }
        EPAT_LOG(cst816_enter_boot, P_INFO, "i2c_buf: 0x%x", i2c_buf[0]);

        break;
    }
    return TRUE;
}

static int write_code(uint8_t *bin_addr, uint8_t retry)
{
    uint16_t i, t;  //,j;
    int ok = TRUE;
    int ret = 0;
    uint8_t i2c_buf[512 + 2];

    bin_addr += 6;

    for(i = 0; i < CST816_BIN_SIZE; i += 512)
    {
        i2c_buf[0] = 0xA0;
        i2c_buf[1] = 0x14;
        i2c_buf[2] = i;
        i2c_buf[3] = i >> 8;
        ret = cst816_write_data(hyn_816dev, i2c_buf, 4);
        if(ret != 0)
        {
            ok = FALSE;
            break;
        }

        i2c_buf[0] = 0xA0;
        i2c_buf[1] = 0x18;
        memcpy(i2c_buf + 2, bin_addr + i, 512);
        EPAT_LOG(write_code_addr, P_INFO, "write offset: %d", i);
        ret = cst816_write_data(hyn_816dev, i2c_buf, 514);
        if(ret != 0)
        {
            ok = FALSE;
            break;
        }
        EPAT_LOG(write_code_data, P_INFO, "write data: %x", bin_addr + i);
        ret = cst816_wr_reg(hyn_816dev, 0xA004EE, 3, i2c_buf, 0);
        if(ret != 0)
        {
            ok = FALSE;
            break;
        }
        EPAT_LOG(write_code_end, P_INFO, "write end: %x", bin_addr + i);
        osDelay(100 * retry);

        for(t = 0;; t++)
        {
            if(t >= 50)
            {
                return FALSE;
            }
            osDelay(5);
            ret = cst816_wr_reg(hyn_816dev, 0xA005, 2, i2c_buf, 1);
            if(ret != 0)
            {
                continue;
            }
            EPAT_LOG(write_code_ok, P_INFO, "i2c buf: %x", i2c_buf[0]);
            if(i2c_buf[0] != 0x55)
            {
                continue;
            }

            break;
        }
    }
    return ok;
}

static uint32_t cst816_read_checksum(void)
{
    int ok = FALSE, t;
    uint8_t i2c_buf[4] = {0};
    uint32_t value = 0;
    int chip_checksum_ok = FALSE;
    // firmware checksum
    ok = cst816_wr_reg(hyn_816dev, 0xA00300, 3, i2c_buf, 0);
    if(ok != 0)
    {
        return value;
    }

    osDelay(100);

    for(t = 0;; t += 10)
    {
        if(t >= 1000)
        {
            break;
        }

        osDelay(10);
        ok = cst816_wr_reg(hyn_816dev, 0xA000, 2, i2c_buf, 1);
        if(ok != 0)
        {
            continue;
        }
        if(i2c_buf[0] == 1)
        {
            chip_checksum_ok = TRUE;
            break;
        }
        else if(i2c_buf[0] == 2)
        {
            chip_checksum_ok = FALSE;
            continue;
        }
    }

    if(chip_checksum_ok == FALSE)
    {
        hyn_816dev->need_update_fw = 1;
        EPAT_LOG(cst816_read_checksum_no_ok, P_INFO,
                 "cst816_read_checksum: not ok");
    }
    else
    {
        ok = cst816_wr_reg(hyn_816dev, 0xA008, 2, i2c_buf, 2);
        if(ok != 0)
        {
            return value;
        }
        value = i2c_buf[0];
        value |= (uint16_t)(i2c_buf[1]) << 8;
        EPAT_LOG(cst816_read_checksum, P_INFO, "cst816_read_checksum: ok, %x",
                 value);
    }
    return value;
}

static int cst816_updata_fw(uint8_t *bin_addr, uint16_t len)
{
    int retry = 0;
    int ok_copy = TRUE;
    int ok = FALSE;
    uint8_t i2c_buf[4];
    uint32_t fw_checksum = 0;
    EPAT_LOG(cst816_updata_fw_enter, P_INFO, "cst816 update firmware begin...");
    fw_checksum = U8TO16(bin_addr[5], bin_addr[4]);

    for(retry = 1; retry < 10; retry++)
    {
        ok = cst816_enter_boot();
        if(ok == FALSE)
        {
            continue;
        }
        EPAT_LOG(cst816_updata_fw_enter_enter_boot, P_INFO,
                 "cst816 update cst816_enter_boot ok");
        ok = write_code(bin_addr, retry);
        if(ok == FALSE)
        {
            continue;
        }
        EPAT_LOG(cst816_updata_fw_enter_write_code, P_INFO,
                 "cst816 update write_code ok");
        hyn_816dev->hw_info.ic_fw_checksum = cst816_read_checksum();

        if(fw_checksum != hyn_816dev->hw_info.ic_fw_checksum)
        {
            EPAT_LOG(cst816_updata_fw_enter_checksum, P_INFO,
                     "cst816 update checksum not matched, %x, %x", fw_checksum,
                     hyn_816dev->hw_info.ic_fw_checksum);
            continue;
        }

        if(retry >= 5)
        {
            ok_copy = FALSE;
            EPAT_LOG(cst816_updata_fw_retry, P_INFO,
                     "cst816 update failed  retry %d", retry);
            break;
        }
        break;
    }

    cst816_wr_reg(hyn_816dev, 0xA006EE, 3, i2c_buf, 0);
    osDelay(2);
    cst816_rst();
    osDelay(50);

    cst816_set_i2c_addr(hyn_816dev, MAIN_I2C_ADDR);
    if(ok_copy == TRUE)
    {
        cst816_updata_tpinfo();
        EPAT_LOG(cst816_updata_fw_success, P_ERROR, "updata_fw success");
    }
    else
    {
        EPAT_LOG(cst816_updata_fw_failed, P_ERROR, "updata_fw failed");
    }

    return ok_copy;
}

static int cst816_updata_tpinfo(void)
{
    uint8_t buf[8];
    TpInfo_t *ic = &hyn_816dev->hw_info;
    int ret = 0;
    ret = cst816_wr_reg(hyn_816dev, 0xA7, 1, buf, 4);
    if(ret != 0)
    {
        EPAT_LOG(cst816_updata_tpinfo_err, P_ERROR,
                 "cst816_updata_tpinfo failed");
        return FALSE;
    }

    ic->fw_sensor_txnum = 2;
    ic->fw_sensor_rxnum = CUSTOM_SENSOR_NUM;
    ic->fw_key_num = hyn_816dev->plat_data.key_num;
    ic->fw_res_y = hyn_816dev->plat_data.y_resolution;
    ic->fw_res_x = hyn_816dev->plat_data.x_resolution;
    ic->fw_project_id = buf[1];
    ic->fw_chip_type = buf[0];
    ic->fw_ver = buf[2];

    EPAT_LOG(cst816_updata_tpinfo, P_INFO,
             "IC_info fw_project_id:%x ictype:%x fw_ver:%x checksum:%x",
             ic->fw_project_id, ic->fw_chip_type, ic->fw_ver,
             ic->ic_fw_checksum);
    return TRUE;
}
//------------------------------------------------------------------------------//

static int cst816_set_workmode(int mode, uint8_t enable)
{
    int ret = 0;
    hyn_816dev->work_mode = mode;
    ret = cst816_wr_reg(hyn_816dev, 0x00, 1, NULL, 0);
    if(ret != 0)
    {
        cst816_rst();
        osDelay(80);
    }
    switch(mode)
    {
        case NOMAL_MODE:
            break;
        case GESTURE_MODE:
            ret = cst816_wr_reg(hyn_816dev, 0xE501, 2, NULL, 0);
            break;
        case LP_MODE:
            break;
        case DIFF_MODE:
        case RAWDATA_MODE:
            ret = cst816_wr_reg(hyn_816dev, 0xFEF8, 2, NULL, 0);
            break;
        case FAC_TEST_MODE:
            break;
        case DEEPSLEEP:
            ret = cst816_wr_reg(hyn_816dev, 0xE503, 2, NULL, 0);
            break;
        default:
            hyn_816dev->work_mode = NOMAL_MODE;
            break;
    }
    return ret;
}

static void cst816_rst(void)
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

static int cst816_supend(void)
{
    cst816_set_workmode(DEEPSLEEP, 0);
    return 0;
}

static int cst816_resum(void)
{
    cst816_rst();
    osDelay(50);
    cst816_set_workmode(NOMAL_MODE, 0);
    return 0;
}

static int cst816_report(void)
{
    uint8_t i = 0;
    int ret = 0;
    uint8_t i2c_buf[3 + 6 * MAX_POINTS_REPORT] = {0};
    uint16_t x, y;
    uint8_t id = 0, index = 0;
    PlatData_t *dt = &hyn_816dev->plat_data;

    memset(&hyn_816dev->rp_buf, 0, sizeof(hyn_816dev->rp_buf));
    hyn_816dev->rp_buf.report_need = REPORT_NONE;

    if(hyn_816dev->work_mode == GESTURE_MODE)
    {
        ret = cst816_wr_reg(hyn_816dev, 0x00, 1, i2c_buf, 2);
        if(ret != 0)
        {
            goto FAILD_END;
        }
        hyn_816dev->gesture_id = IDX_NULL;
        if(i2c_buf[1] == 0x05)
        {  // click
            hyn_816dev->gesture_id = IDX_POWER;
            hyn_816dev->rp_buf.report_need = REPORT_GES;
        }
    }
    else
    {
        ret = cst816_wr_reg(hyn_816dev, 0x00, 1, i2c_buf, (3 + 6 * 2));
        if(ret != 0)
        {
            goto FAILD_END;
        }
        hyn_816dev->rp_buf.rep_num = i2c_buf[2];
        for(i = 0; i < 2; i++)
        {
            id = (i2c_buf[5 + i * 6] & 0xf0) >> 4;
            if(id > 1) continue;

            x = (i2c_buf[3 + i * 6] & 0x0f);
            x = (x << 8) + i2c_buf[4 + i * 6];

            y = (i2c_buf[5 + i * 6] & 0x0f);
            y = (y << 8) + i2c_buf[6 + i * 6];

            hyn_816dev->rp_buf.pos_info[index].pos_id = id;
            hyn_816dev->rp_buf.pos_info[index].event =
                (i2c_buf[3 + i * 6] & 0x40) ? 0 : 1;
            hyn_816dev->rp_buf.pos_info[index].pos_x = x;
            hyn_816dev->rp_buf.pos_info[index].pos_y = y;
            hyn_816dev->rp_buf.pos_info[index].pres_z =
                3 + (x & 0x03);  // press mast chang
            index++;
        }
        if(index != 0) hyn_816dev->rp_buf.report_need = REPORT_POS;
        if(dt->key_num)
        {
            i = dt->key_num;
            while(i)
            {
                i--;
                if(dt->key_y_coords == hyn_816dev->rp_buf.pos_info[0].pos_y &&
                   dt->key_x_coords[i] == hyn_816dev->rp_buf.pos_info[0].pos_x)
                {
                    hyn_816dev->rp_buf.key_id = i;
                    hyn_816dev->rp_buf.key_state =
                        hyn_816dev->rp_buf.pos_info[0].event;
                    hyn_816dev->rp_buf.report_need = REPORT_KEY;
                }
            }
        }
    }
    return TRUE;
FAILD_END:
    EPAT_LOG(cst816_report_failed, P_ERROR, "read report data failed");
    return FALSE;
}

static uint32_t cst816_check_esd(void) { return TRUE; }

static int cst816_prox_handle(uint8_t cmd) { return TRUE; }

static int cst816_get_dbg_data(uint8_t *buf, uint16_t len)
{
    int ret = -1, read_len = len;
    switch(hyn_816dev->work_mode)
    {
        case DIFF_MODE:
            ret = cst816_wr_reg(hyn_816dev, 0x41, 1, buf, read_len);
            break;
        case RAWDATA_MODE:
            ret = cst816_wr_reg(hyn_816dev, 0x61, 1, buf, read_len);
            break;
        default:
            EPAT_LOG(cst816_get_dbg_data_error, P_ERROR, "work_mode:%d",
                     hyn_816dev->work_mode);
            break;
    }
    return ret == 0 ? read_len : -1;
}

static void cst816_set_i2c_addr(TpDevCst816_t *ts_data, uint8_t addr)
{
    ts_data->slave_addr = addr;
    api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                  &ts_data->slave_addr);
}

int cst816_i2c_init(TpDevCst816_t *ts_data, uint8_t port_id, uint32_t speed,
                    void (*tp_bus_cb)(uint32_t event))
{
    api_i2c_create(port_id, NULL, &ts_data->i2c_id);
    int ret = api_i2c_open(ts_data->i2c_id, NULL, 1000);
    if(ret != 0)
    {
        EPAT_LOG(cst816_i2c_init_failed, P_ERROR,
                 "cst816_i2c_init open failed");
    }
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SPEED, &speed);
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_ISR_CB, tp_bus_cb);
    ret = api_i2c_ioctl(ts_data->i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR,
                        &ts_data->slave_addr);
    return ret;
}

static int cst816_write_data(TpDevCst816_t *ts_data, uint8_t *buf, uint16_t len)
{
    int ret = 0;
    ret = api_i2c_write(ts_data->i2c_id, buf, len, false);
    return ret != 0 ? -1 : 0;
}

static int cst816_wr_reg(TpDevCst816_t *ts_data, uint32_t reg_addr,
                         uint8_t reg_len, uint8_t *rbuf, uint16_t rlen)
{
    int ret = 0;
    int i = 0;
    uint8_t wbuf[4];
    reg_len = reg_len & 0x0F;
    memset(wbuf, 0, sizeof(wbuf));
    i = reg_len;
    while(i)
    {
        i--;
        wbuf[i] = reg_addr;
        reg_addr >>= 8;
    }
    ret = api_i2c_write(ts_data->i2c_id, wbuf, reg_len, rlen != 0);
    if(rlen != 0)
    {
        ret |= api_i2c_read(ts_data->i2c_id, rbuf, rlen, false);
    }
    return ret < 0 ? -1 : 0;
}

static int cst816_get_test_result(uint8_t *buf, uint16_t len) { return 0; }

TpDevCst816_t *cst816_dev_get(void)
{
    if(cst816_dev.hyn_fuc_used == NULL)
    {
        memset(&cst816_dev, 0, sizeof(TpDevCst816_t));
        cst816_dev.hyn_fuc_used = &cst816_func;
        cst816_dev.plat_data.max_touch_num = MAX_POINTS_REPORT;
        cst816_dev.plat_data.x_resolution = 128;
        cst816_dev.plat_data.y_resolution = 128;
        cst816_dev.plat_data.swap_xy = 0;
        cst816_dev.plat_data.reverse_x = 0;
        cst816_dev.plat_data.reverse_y = 0;
        // cst816_dev.plat_data.irq_gpio = inf->PadId;
    }
    else
    {
        cst816_dev.need_update_fw = 0;
    }

    return &cst816_dev;
}

int cst816_dev_init(uint8_t port_id, uint32_t speed, uint8_t fw_id,
                    void (*tp_bus_cb_func)(uint32_t event))
{
    int ret = 0;
    TpDevCst816_t *dev = cst816_dev_get();
#ifdef TP_RST_PAD_INDEX
    dev->plat_data.reset_gpio = TP_RST_PAD_INDEX;
    dev->hyn_fuc_used->tp_reset();
#endif
    ret = cst816_i2c_init(dev, port_id, speed, tp_bus_cb_func);
    if(ret != 0)
    {
        return ret;
    }
    ret = cst816_init(dev, fw_id);
    if(ret != 0)
    {
        EPAT_LOG(api_tp_create_failed, P_ERROR, "api_tp_create failed");
        return ret;
    }
#ifdef TP_POWON_CHECK_UPDATE
    if(dev->need_update_fw)
    {
        dev->fw_file_name[0] = 0;
        dev->hyn_fuc_used->tp_updata_fw(dev->fw_updata_addr,
                                        dev->fw_updata_len);
    }
#endif
    return ret;
}

static int cst816_scan(uint32_t usrId, int16_t *pos)
{
    int ret = 0;
    if(!cst816_dev.hyn_fuc_used)
    {
        EPAT_LOG(tp_scan_cst816_not_init, P_ERROR, "cst816 driver not init");
        return -1;
    }
    cst816_dev.hyn_irq_flg = 1;
    if(cst816_dev.work_mode < DIFF_MODE)
    {
        ret = cst816_report();  // 读点
        for(uint8_t i = 0; i < cst816_dev.rp_buf.rep_num; i++)
        {  // 根据配置修改坐标原点
            if(cst816_dev.plat_data.swap_xy)
            {
                uint16_t tmp = cst816_dev.rp_buf.pos_info[i].pos_x;
                cst816_dev.rp_buf.pos_info[i].pos_x =
                    cst816_dev.rp_buf.pos_info[i].pos_y;
                cst816_dev.rp_buf.pos_info[i].pos_y = tmp;
            }
            if(cst816_dev.plat_data.reverse_x)
                cst816_dev.rp_buf.pos_info[i].pos_x =
                    cst816_dev.plat_data.x_resolution -
                    cst816_dev.rp_buf.pos_info[i].pos_x;
            if(cst816_dev.plat_data.reverse_y)
                cst816_dev.rp_buf.pos_info[i].pos_y =
                    cst816_dev.plat_data.y_resolution -
                    cst816_dev.rp_buf.pos_info[i].pos_y;
        }
        EPAT_LOG(tpScan, P_INFO, "ret:%d num:%d xy:(%d,%d)", ret,
                 cst816_dev.rp_buf.rep_num, cst816_dev.rp_buf.pos_info[0].pos_x,
                 cst816_dev.rp_buf.pos_info[0].pos_y);
    }
    cst816_dev.rp_buf.report_need = REPORT_NONE;
    if(cst816_dev.plat_data.output_x_resolution == 0)
    {
        pos[0] = cst816_dev.rp_buf.pos_info[0].pos_x;
    }
    else
    {
        pos[0] = cst816_dev.rp_buf.pos_info[0].pos_x *
                 cst816_dev.plat_data.output_x_resolution /
                 cst816_dev.plat_data.x_resolution;
    }

    if(cst816_dev.plat_data.output_y_resolution == 0)
    {
        pos[1] = cst816_dev.rp_buf.pos_info[0].pos_y;
    }
    else
    {
        pos[1] = cst816_dev.rp_buf.pos_info[0].pos_y *
                 cst816_dev.plat_data.output_y_resolution /
                 cst816_dev.plat_data.y_resolution;
    }
    return cst816_dev.rp_buf.rep_num;
}

const TsFunc_t cst816_func = {.tp_reset = cst816_rst,
                              .tp_report = cst816_report,
                              .tp_supend = cst816_supend,
                              .tp_resum = cst816_resum,
                              .tp_updata_fw = cst816_updata_fw,
                              .tp_set_workmode = cst816_set_workmode,
                              .tp_check_esd = cst816_check_esd,
                              .tp_prox_handle = cst816_prox_handle,
                              .tp_get_dbg_data = cst816_get_dbg_data,
                              .tp_get_test_result = cst816_get_test_result,
                              .tp_scan = cst816_scan};
