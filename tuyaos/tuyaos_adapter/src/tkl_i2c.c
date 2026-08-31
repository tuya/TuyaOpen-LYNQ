#include <stdint.h>
#include "tkl_i2c.h"
#include "cmsis_os2.h"
#include "ol_gpio_api.h"
#include "bsp.h"
#include "vlog.h"
#include "Driver_I2C.h"

#define I2C_DEV_NUM    2

typedef struct {
    bool init;
    ARM_DRIVER_I2C* instance;
} I2C_DEV_CTRL_T;

extern ARM_DRIVER_I2C Driver_I2C0;
extern ARM_DRIVER_I2C Driver_I2C1;         

I2C_DEV_CTRL_T gDevCtrl[I2C_DEV_NUM] = {
    {false, &Driver_I2C0},
    {false, &Driver_I2C1},
};

/**
 * @brief i2c init
 *
 * @param[in] port: i2c port
 * @param[in] cfg: i2c config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_init(TUYA_I2C_NUM_E port, CONST TUYA_IIC_BASE_CFG_T *cfg)
{
    if (port >= I2C_DEV_NUM) {
        LOGE("invalid port %d", port);
        return OPRT_COM_ERROR;
    }

    if (NULL == cfg) {
        LOGE("cfg is null");
        return OPRT_INVALID_PARM;
    }

    if (TUYA_IIC_MODE_MASTER != cfg->role) {
        LOGE("only support master role: %d", cfg->role);
        return OPRT_INVALID_PARM;
    }

    ol_gpio_config_struct gpio_cfg;
    memset(&gpio_cfg, 0, sizeof(ol_gpio_config_struct));
    if(port == 0) {
        gpio_cfg.gpio_func = OL_GPIO_FUNC2;
        ol_pin_config(57, &gpio_cfg);
        ol_pin_config(58, &gpio_cfg);
    } else {
        gpio_cfg.gpio_func = OL_GPIO_FUNC1;
        ol_pin_config(66, &gpio_cfg);
        ol_pin_config(67, &gpio_cfg);
    }

    int ret;
    ret = gDevCtrl[port].instance->Initialize(NULL);
    if (0 != ret) {
        LOGE("init i2c0 failed, ret/%d", ret);
        return OPRT_COM_ERROR;
    }

    ret = gDevCtrl[port].instance->PowerControl(ARM_POWER_FULL);
    if (0 != ret) {
        LOGE("power i2c0 failed, ret/%d", ret);
        return OPRT_COM_ERROR;
    }

    uint32_t speed;
    switch (cfg->speed) {
        case TUYA_IIC_BUS_SPEED_100K:
            speed = ARM_I2C_BUS_SPEED_STANDARD;
            break;
        case TUYA_IIC_BUS_SPEED_400K:
            speed = ARM_I2C_BUS_SPEED_FAST;
            break;
        case TUYA_IIC_BUS_SPEED_1M:
            speed = ARM_I2C_BUS_SPEED_FAST_PLUS;
            break;
        case TUYA_IIC_BUS_SPEED_3_4M:
            speed = ARM_I2C_BUS_SPEED_HIGH;
            break;
        default:
            LOGE("invalid speed %d", cfg->speed);
            return OPRT_COM_ERROR;
    }
    ret = gDevCtrl[port].instance->Control(ARM_I2C_BUS_SPEED, speed);
    if (0 != ret) {
        LOGE("set speed %d failed, ret/%d", cfg->speed, ret);
        return OPRT_COM_ERROR;
    }

    ret = gDevCtrl[port].instance->Control(ARM_I2C_BUS_CLEAR, 0);
    if (0 != ret) {
        LOGE("clear bus failed, ret/%d", ret);
        return OPRT_COM_ERROR;
    }

    gDevCtrl[port].init = true;
    LOGI("init i2c %d success", port);
    return OPRT_OK;
}

/**
 * @brief i2c deinit
 *
 * @param[in] port: i2c port
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_deinit(TUYA_I2C_NUM_E port)
{
    int ret;

    if (port >= I2C_DEV_NUM) {
        LOGE("invalid port %d", port);
        return OPRT_COM_ERROR;
    }

    if (true != gDevCtrl[port].init) {
        LOGI("i2c %d is already deinit", port);
        return OPRT_COM_ERROR;
    }

    ret = gDevCtrl[port].instance->PowerControl(ARM_POWER_OFF);
    if (0 != ret) {
        LOGE("power off i2c %d failed, ret/%d", port, ret);
        return OPRT_COM_ERROR;
    }

    ret = gDevCtrl[port].instance->Uninitialize();
    if (0 != ret) {
        LOGE("uninit i2c %d failed, ret/%d", port, ret);
        return OPRT_COM_ERROR;
    }
    
    gDevCtrl[port].init = false;
    LOGI("i2c %d deinit success", port);
    return OPRT_OK;
}

/**
 * @brief i2c irq init
 * NOTE: call this API will not enable interrupt
 *
 * @param[in] port: i2c port, id index starts at 0
 * @param[in] cb:  i2c irq cb
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_irq_init(TUYA_I2C_NUM_E port, TUYA_I2C_IRQ_CB cb)
{
    LOGE("tkl_i2c_irq_init not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c irq enable
 *
 * @param[in] port: i2c port id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_irq_enable(TUYA_I2C_NUM_E port)
{
    LOGE("tkl_i2c_irq_enable not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c irq disable
 *
 * @param[in] port: i2c port id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_irq_disable(TUYA_I2C_NUM_E port)
{
    LOGE("tkl_i2c_irq_disable not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c master send
 *
 * @param[in] port: i2c port
 * @param[in] dev_addr: iic addrress of slave device.
 * @param[in] data: i2c data to send
 * @param[in] size: Number of data items to send
 * @param[in] xfer_pending: xfer_pending: TRUE : not send stop condition, FALSE : send stop condition.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_master_send(TUYA_I2C_NUM_E port, UINT16_T dev_addr, CONST VOID_T *data, UINT32_T size, BOOL_T xfer_pending)
{
    if (port >= I2C_DEV_NUM) {
        LOGE("invalid port %d", port);
        return OPRT_COM_ERROR;
    }

    if (true != gDevCtrl[port].init) {
        LOGE("send error, i2c %d not init", port);
        return OPRT_COM_ERROR;
    }

    int ret = gDevCtrl[port].instance->MasterTransmit((uint32_t)dev_addr, (uint8_t*)data, size, xfer_pending);
    if (0 != ret) {
        LOGE("i2c %d master xmit failed, ret/%d", port, ret);
        return OPRT_COM_ERROR;
    }
    delay_us(200);
    return OPRT_OK;
}

/**
 * @brief i2c master recv
 *
 * @param[in] port: i2c port
 * @param[in] dev_addr: iic addrress of slave device.
 * @param[in] data: i2c buf to recv
 * @param[in] size: Number of data items to receive
 * @param[in] xfer_pending: TRUE : not send stop condition, FALSE : send stop condition.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_master_receive(TUYA_I2C_NUM_E port, UINT16_T dev_addr, VOID *data, UINT32_T size, BOOL_T xfer_pending)
{
    if (port >= I2C_DEV_NUM) {
        LOGE("invalid port %d", port);
        return OPRT_COM_ERROR;
    }

    if (true != gDevCtrl[port].init) {
        LOGE("receive error, i2c %d not init", port);
        return OPRT_COM_ERROR;
    }

    int ret = gDevCtrl[port].instance->MasterReceive((uint32_t)dev_addr, (uint8_t*)data, size, xfer_pending);
    if (0 != ret) {
        LOGE("i2c %d master recv failed, ret/%d", port, ret);
        return OPRT_COM_ERROR;
    }
    delay_us(200);
    return OPRT_OK;
}

/**
 * @brief i2c slave
 *
 * @param[in] port: i2c port
 * @param[in] dev_addr: slave device addr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_set_slave_addr(TUYA_I2C_NUM_E port, UINT16_T dev_addr)
{
    LOGE("tkl_i2c_set_slave_addr not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c slave send
 *
 * @param[in] port: i2c port
 * @param[in] data: i2c buf to send
 * @param[in] size: Number of data items to send
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

OPERATE_RET tkl_i2c_slave_send(TUYA_I2C_NUM_E port, CONST VOID *data, UINT32_T size)
{
    LOGE("tkl_i2c_slave_send not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief IIC slave receive, Start receiving data as IIC Slave.
 *
 * @param[in] port: i2c port
 * @param[in] data: Pointer to buffer for data to receive from IIC Master
 * @param[in] size: Number of data items to receive
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

OPERATE_RET tkl_i2c_slave_receive(TUYA_I2C_NUM_E port, VOID *data, UINT32_T size)
{
    LOGE("tkl_i2c_slave_receive not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief IIC get status.
 *
 * @param[in] port: i2c port
 * @param[out]  TUYA_IIC_STATUS_T
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_get_status(TUYA_I2C_NUM_E port, TUYA_IIC_STATUS_T *status)
{
    LOGE("tkl_i2c_get_status not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c's reset
 *
 * @param[in] port: i2c port number
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_reset(TUYA_I2C_NUM_E port)
{
    LOGE("tkl_i2c_reset not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c transferred data count.
 *
 * @param[in] port: i2c port id, id index starts at 0
 *
 * @return >=0,number of currently transferred data items. <0,err.
 * tkl_i2c_master_send:number of data bytes transmitted and acknowledged
 * tkl_i2c_master_receive:number of data bytes received
 * tkl_i2c_slave_send:number of data bytes transmitted
 * tkl_i2c_slave_receive:number of data bytes received and acknowledged
 */
INT32_T tkl_i2c_get_data_count(TUYA_I2C_NUM_E port)
{
    LOGE("tkl_i2c_get_data_count not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c ioctl
 *
 * @param[in]       cmd     user def
 * @param[in]       args    args associated with the command
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_i2c_ioctl(TUYA_I2C_NUM_E port, UINT32_T cmd,  VOID *args)
{
    LOGE("tkl_i2c_ioctl not support");
    return OPRT_NOT_SUPPORTED;
}

#if 0
void tkl_i2c_test(void)
{
    TUYA_IIC_BASE_CFG_T cfg = {
        .role = TUYA_IIC_MODE_MASTER,
        .speed = TUYA_IIC_BUS_SPEED_400K,
        .addr_width = TUYA_IIC_ADDRESS_7BIT
    };

    int ret = tkl_i2c_init(0, &cfg);
    if(ret) {
        LOGE("i2c init failed");
        return;
    }

    #define EEPROM_DEVICE_ADDR           0x50
    #define BUFFER_SIZE 32
    uint16_t reg_addr = 0x100;
    uint8_t write_buffer[BUFFER_SIZE+2], read_buffer[BUFFER_SIZE];
    for(int i = 0; i < BUFFER_SIZE; i++) {
        write_buffer[i+2] = i;
    }

    write_buffer[0] = reg_addr >> 8;
    write_buffer[1] = reg_addr;
    ret = tkl_i2c_master_send(0, EEPROM_DEVICE_ADDR, write_buffer, BUFFER_SIZE+2, true);
    if(ret) {
        LOGE("i2c write failed");
        return ;
    }

    delay_us(5000); 

    ret = tkl_i2c_master_send(0, EEPROM_DEVICE_ADDR, write_buffer, 2, true);
    ret = tkl_i2c_master_receive(0, EEPROM_DEVICE_ADDR, read_buffer, BUFFER_SIZE, true);
    if(ret) {
        LOGE("i2c read failed");
        return ;
    }

    int i ;
    for(i = 0; i < BUFFER_SIZE; i++)
    {
        if(write_buffer[i+2] != read_buffer[i])
            break;
    }

    if(i != BUFFER_SIZE)
    {
        OL_LOG_PRINTF("i2c_demo : data write/read fail");
    }
    else
    {
        OL_LOG_PRINTF("i2c_demo : data write/read success");
    }
}
#endif

#if 0
#include "tkl_system.h"
#include "tkl_thread.h"
#include "tkl_memory.h"
// 使用 SC7A22H 加速度传感器进行测试
#define I2C_DEV_ADDR  0x19
#define DEV_I2C_PORT  0
#define CHIP_ID       0x18

static TKL_THREAD_HANDLE test_thread = NULL;

static int dev_write_reg(uint8_t reg_addr, uint8_t value)
{
    uint8_t tmp[2] = {reg_addr, value};
    return tkl_i2c_master_send(DEV_I2C_PORT, I2C_DEV_ADDR, tmp, 2, false);
}
static int dev_write_regs(uint8_t reg_addr, uint8_t *value, int len)
{
    uint8_t *tmp = tkl_system_malloc(len+1);
    tmp[0] = reg_addr;
    memcpy(tmp+1, value, len);
    int ret = tkl_i2c_master_send(DEV_I2C_PORT, I2C_DEV_ADDR, tmp, len+1, false);
    tkl_system_free(tmp);
    return ret;
}
static int dev_read_reg(uint8_t reg_addr, uint8_t *value)
{
    tkl_i2c_master_send(DEV_I2C_PORT, I2C_DEV_ADDR, &reg_addr, 1, true);
    return tkl_i2c_master_receive(DEV_I2C_PORT, I2C_DEV_ADDR, value, 1, false);
}
static int dev_read_regs(uint8_t reg_addr, uint8_t *value, int len)
{
    tkl_i2c_master_send(DEV_I2C_PORT, I2C_DEV_ADDR, &reg_addr, 1, true);
    return tkl_i2c_master_receive(DEV_I2C_PORT, I2C_DEV_ADDR, value, len, false);
}
static int dev_check_id(void)
{
    uint8_t id = 0;
    int ret = dev_read_reg(1, &id);
    LOGI("chip id ret:%d id:%d %d", ret, id, CHIP_ID);
    return (id == CHIP_ID) ? 0 : -1;
}
static int dev_init(void)
{
    TUYA_IIC_BASE_CFG_T cfg = {
        .speed = 0,
    };
    int ret = tkl_i2c_init(DEV_I2C_PORT, &cfg);
    if(ret) {
        LOGE("i2c init failed");
        return -1;
    }
    return dev_check_id();
}
static void i2c_test(void)
{
    int ret = dev_init();
    if(ret) {
        LOGE("dev_init failed");
        goto exit;
    }   
        
    dev_write_reg(0x7f, 0);
    dev_write_reg(0x7d, 0x04);
    dev_write_reg(0x40, 0x26);
    dev_write_reg(0x41, 0x1);      //      # 0:±2g  1:±4g  2:±8g  3:±16g
    dev_write_reg(0x05, 0x50);

    dev_write_reg(0x1C, 0);
    dev_write_reg(0x1D, 0);
    dev_write_reg(0x05, 0x10);

    uint8_t raw_data[6];
    uint16_t accel_x, accel_y, accel_z;
    while(1) {
        dev_read_regs(0x0c, raw_data, 6);
        accel_x = ((raw_data[0] << 8) | raw_data[1]) >> 4;
        accel_y = ((raw_data[2] << 8) | raw_data[3]) >> 4;
        accel_z = ((raw_data[4] << 8) | raw_data[5]) >> 4;
        LOGI("x:%d y:%d z:%d", accel_x, accel_y, accel_z);
        tkl_system_sleep(100);
    }
exit:
    while(1) {
        tkl_system_sleep(100);
    }
}
void tkl_i2c_test(void)
{
    tkl_thread_create(&test_thread, "test_thread1", 4096,  0, i2c_test, NULL);
}
#endif
