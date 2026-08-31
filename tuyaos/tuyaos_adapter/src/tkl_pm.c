#include "tkl_pm.h"
#include "tkl_gpio.h"
#include "cmsis_os2.h"
#include "vlog.h"

void tkl_pm_init(void)
{
}

TUYA_PM_DEV_DESC_T *tkl_pm_get_dev_info(CONST CHAR_T *devname)
{
    return NULL;
}

OPERATE_RET tkl_pm_dev_unregistor(CONST CHAR_T *devname)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_PM_DEV_DESC_T *tkl_pm_get_dev_list_head(void)
{
    return NULL;
}

OPERATE_RET tkl_pm_set_voltage(CONST CHAR_T *devname, INT_T mV)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pm_get_voltage(CONST CHAR_T *devname)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pm_set_current(CONST CHAR_T *devname,INT_T mA)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pm_get_current(CONST CHAR_T *devname)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pm_enable(CONST CHAR_T *devname,INT_T lp_en)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 关闭设备电源功能。
 *
 * @param devname 设备名称
 *
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_pm_disable(CONST CHAR_T *devname)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 获取设备电源功能是否使能。
 *
 * @param devname 设备名称
 *
 * @return 1 打开；0 关闭
 */
OPERATE_RET tkl_pm_is_enable(CONST CHAR_T *devname,BOOL_T *status)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 关闭系统电源，设备关机。
 *
 * @param devname 设备名称
 *
 * @return 1 打开；0 关闭
 */
extern void ol_power_off(void);
OPERATE_RET tkl_pm_power_off(CONST CHAR_T *devname)
{
    if (!strcmp(devname,"SYS_DEV")) {
        ol_power_off();
        return OPRT_OK;
    }
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 重启设备，该设备可以是整个系统，也可以是某个单独管理的设备。
 *
 * @param devname 设备名称
 *
 * @return 0 设置成功 其它 设置失败
 */
extern void ol_power_reset(void);
OPERATE_RET tkl_pm_reset(CONST CHAR_T *devname)
{
    if (!strcmp(devname,"SYS_DEV")) {
        ol_power_reset();
        return OPRT_OK;
    }
    return OPRT_NOT_SUPPORTED;
}

static bool isLedSrvInit = true;
static osTimerId_t net_led_timerid = NULL;
static unsigned int iTimeVal = 0;
static bool net_led_state = false;
#define NET_STATUS_LED TUYA_GPIO_NUM_16

static int tuya_net_mode_led_srv_ctrol(int t_interval)
{
    if (false == isLedSrvInit) {
        LOGE( "led service should init first");
        return -1;
    }

    iTimeVal = t_interval;
    osTimerStop(net_led_timerid);

    if (t_interval == 0) {
        tkl_gpio_write(NET_STATUS_LED, TUYA_GPIO_LEVEL_LOW);
        net_led_state = false;
    } else if (t_interval == (-1)) {
        tkl_gpio_write(NET_STATUS_LED, TUYA_GPIO_LEVEL_HIGH);
        net_led_state = true;
    } else {
        osTimerStart(net_led_timerid, iTimeVal);
    }
    LOGI("tuya_net_mode_led_srv_ctrol -->t_interval=%d", t_interval);
    return 0;
}

static void netLed_flicker(void)
{
    net_led_state = !net_led_state;
    tkl_gpio_write(NET_STATUS_LED, net_led_state);
    int time = net_led_state ? 100 : iTimeVal;
    osTimerStart(net_led_timerid, time);
}

static int tuya_net_mode_led_srv_init(void)
{
    TUYA_GPIO_BASE_CFG_T cfg;
    cfg.mode = TUYA_GPIO_PULLUP;
    cfg.direct =TUYA_GPIO_OUTPUT;
    cfg.level = TUYA_GPIO_LEVEL_LOW;            //高电平亮灯
    tkl_gpio_init(NET_STATUS_LED,&cfg);

    if (net_led_timerid == NULL) {
        net_led_timerid = osTimerNew((osTimerFunc_t)netLed_flicker, osTimerOnce, NULL, NULL);
    }
    LOGI("tuya_net_mode_led_srv_init");
    isLedSrvInit = true;
    net_led_state = false;
    tuya_net_mode_led_srv_ctrol(0);
    return 0;
}

static int tuya_net_mode_led_srv_deinit(void)
{
    if (false == isLedSrvInit) {
        LOGE( "led service should init first");
        return -1;
    }
    isLedSrvInit = false;

    if(net_led_timerid) {
        osTimerStop(net_led_timerid);
        osTimerDelete(net_led_timerid);
        net_led_timerid = NULL;
    }
    tkl_gpio_write(NET_STATUS_LED, TUYA_GPIO_LEVEL_LOW);
    tkl_gpio_deinit(NET_STATUS_LED);
    iTimeVal = 0;
    return 0;
}


/**
 * @brief 设备的ioctl接口，这个接口是为了给设备提供了扩展的接口
 *
 * @param devname  设备名称
 * @param ctl_cmd  控制命令
 * @param param    输入参数指针
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_pm_ioctl(CONST CHAR_T *devname, INT_T ctl_cmd, void *param)
{
    if(!strcasecmp(devname, "net_mode")) {
        if (IOCTRL_CMD_SET == ctl_cmd) {
            if (param) {
                if (!strcasecmp("init",(char*)param)) {
                    return tuya_net_mode_led_srv_init();
                }
                else if (!strcasecmp("deinit",(char*)param)) {
                    return tuya_net_mode_led_srv_deinit();
                }
            }
        } else if (IOCTRL_CMD_TRANS_INT_STRING == ctl_cmd) {
            if (param) {
                int time = atoi((char*)param);
                return tuya_net_mode_led_srv_ctrol(time);
            }
        }
    }
    return OPRT_NOT_SUPPORTED;
}

