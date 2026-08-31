/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_i2c_ex.c
 * Description:  openhal i2c entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include DEBUG_LOG_HEADER_FILE
#include "ec7xx.h"
#include "api_comm.h"
#include "api_i2c.h"
#include "bsp.h"
#include "bsp_i2c.h"
#include "devicemanager.h"
#include "system_ec7xx.h"
#include "clock.h"
#include "bsp.h"

#ifndef ATTRIBUTE_GCC_HEAD
#define ATTRIBUTE_GCC_HEAD
#endif

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif
extern const int8_t list_pad2gpio[];
/* ---------------------------------------------------------------------------------------------- */
static ec_i2c_bus_t soft_i2c = {0}; // 修改为数组实现多个soft I2C
static ec_i2c_bus_t *actived_i2c = NULL;

/**
  \fn
  \brief
  \return
*/
ATTRIBUTE_GCC_HEAD  
static void gpio_cfg_delay(uint8_t speed)
{
    switch (speed)
    {
        case ARM_I2C_BUS_SPEED_STANDARD:    // Standard Speed (100kHz)
            for(volatile uint8_t i = 0; i < 82; i++) __NOP();
            break;
        case ARM_I2C_BUS_SPEED_FAST:        // Fast Speed     (400kHz)
            for(volatile uint8_t i = 0; i < 18; i++) __NOP();
            break;
        default:
            __NOP();__NOP();__NOP();__NOP();
            break;
    }
}

/**
  \fn
  \brief    SCL保持高电平期间，SDA从高电平变为低电平
  \return
*/
ATTRIBUTE_GCC_HEAD  
static void sw_i2c_start(ec_i2c_bus_t *i2c)
{
    GpioPinConfig_t pinConfig ={0};
    pinConfig.misc.initOutput = 1;
    pinConfig.pinDirection = 1U;
    GPIO_pinConfig(i2c->cfg.sda_port, i2c->cfg.sda_pin, &pinConfig);
    GPIO_pinConfig(i2c->cfg.scl_port, i2c->cfg.scl_pin, &pinConfig);
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, 0);
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, 0);
    gpio_cfg_delay(i2c->cfg.speed);
}

/**
  \fn
  \brief    SCL保持高电平期间，SDA从低电平变为高电平
  \return
*/
ATTRIBUTE_GCC_HEAD 
static void sw_i2c_stop(ec_i2c_bus_t *i2c)
{
    GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, 0);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, 0);
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, i2c->cfg.scl_mask);
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, i2c->cfg.sda_mask);
    gpio_cfg_delay(i2c->cfg.speed);
}

/**
  \fn          static int32_t soft_i2c_send_byte(ec_i2c_bus_t *i2c, uint8_t data)
  \brief       Send byte to I2C bus
  \param[in]   i2c    Pointer to I2C control block
  \param[in]   data   Byte to send
  \return      ARM_I2C_EVENT_TRANSFER_DONE on success, ARM_I2C_EVENT_BUS_ERROR on error
*/
ATTRIBUTE_GCC_HEAD 
static int32_t soft_i2c_send_byte(ec_i2c_bus_t *i2c, uint8_t data)
{
    GpioPinConfig_t pinConfig ={0};
    pinConfig.pinDirection = 0U ;
    // volatile uint32_t mask = SaveAndSetIRQMask();
    for (volatile uint8_t i = 0; i < 8; i++){
        if (data & 0x80)
        {
            GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, i2c->cfg.sda_mask);
        }
        else
        {
            GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, 0);
        }
        data <<= 1;
        gpio_cfg_delay(i2c->cfg.speed);
        GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, i2c->cfg.scl_mask);
        gpio_cfg_delay(i2c->cfg.speed);
        GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, 0);
    }
    GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, i2c->cfg.sda_mask);
    GPIO_pinConfig(i2c->cfg.sda_port, i2c->cfg.sda_pin, &pinConfig);
    // 读取从设备的ACK位
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, i2c->cfg.scl_mask);
    gpio_cfg_delay(i2c->cfg.speed);
    // 接收方在第9个时钟周期将SDA拉低，表示成功接收数据
    bool ack = !GPIO_pinRead(i2c->cfg.sda_port, i2c->cfg.sda_pin);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, 0);
    gpio_cfg_delay(i2c->cfg.speed);
    // 恢复SDA为输出模式
    pinConfig.misc.initOutput = 0;
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    GPIO_pinConfig(i2c->cfg.sda_port, i2c->cfg.sda_pin, &pinConfig);

    // RestoreIRQMask(mask);
    return ack ? ARM_I2C_EVENT_TRANSFER_DONE : ARM_I2C_EVENT_ADDRESS_NACK;
}

/**
  \fn          static uint8_t soft_i2c_read_byte(ec_i2c_bus_t *i2c, bool send_ack)
  \brief       Read byte from I2C bus
  \param[in]   i2c        Pointer to I2C control block
  \param[in]   send_ack   Flag to send ACK (true) or NACK (false)
  \return      Byte read from I2C bus
*/
ATTRIBUTE_GCC_HEAD 
static uint8_t soft_i2c_read_byte(ec_i2c_bus_t *i2c, bool send_ack)
{
    uint8_t data = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
    GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, i2c->cfg.sda_mask);  // 释放SDA线
    GPIO_pinConfig(i2c->cfg.sda_port, i2c->cfg.sda_pin, &pinConfig);
    // volatile uint32_t mask = SaveAndSetIRQMask();
    for (volatile uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, i2c->cfg.scl_mask);
        // 等待一段时间读取数据位
        gpio_cfg_delay(i2c->cfg.speed);
        if (GPIO_pinRead(i2c->cfg.sda_port, i2c->cfg.sda_pin))
        {
            data |= 0x01;
        }
        // 降低SCL完成一个时钟周期
        GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, 0);
        gpio_cfg_delay(i2c->cfg.speed);
    }
    // 恢复SDA为输出模式
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    // 发送ACK或NACK
    if (send_ack)
    {
        pinConfig.misc.initOutput = 0;
    }
    else
    {
        pinConfig.misc.initOutput = 1;  // NACK: 释放SDA 
    }
    GPIO_pinConfig(i2c->cfg.sda_port, i2c->cfg.sda_pin, &pinConfig);
    // 产生ACK/NACK时钟脉冲
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, i2c->cfg.scl_mask);
    gpio_cfg_delay(i2c->cfg.speed);
    GPIO_pinWrite(i2c->cfg.scl_port, i2c->cfg.scl_mask, 0);
    GPIO_pinWrite(i2c->cfg.sda_port, i2c->cfg.sda_mask, i2c->cfg.sda_mask);
    // RestoreIRQMask(mask);
    gpio_cfg_delay(i2c->cfg.speed);
    return data;
}

/**
  \fn
  \brief
  \return
*/
ATTRIBUTE_GCC_HEAD 
static int32_t soft_i2c_send_bytes(uint32_t addr, const uint8_t *data, uint32_t length, bool xfer_pending)
{
    int32_t status = ARM_I2C_EVENT_TRANSFER_DONE;
    sw_i2c_start(actived_i2c);
    status = soft_i2c_send_byte(actived_i2c, (addr << 1));
    if (status != ARM_I2C_EVENT_TRANSFER_DONE)
    {
        sw_i2c_stop(actived_i2c);
        return status;
    }
    for (volatile uint32_t i = 0; i < length; i++)
    {
        status = soft_i2c_send_byte(actived_i2c, data[i]);
        if (status != ARM_I2C_EVENT_TRANSFER_DONE)
        {
            sw_i2c_stop(actived_i2c);
            return status;
        }
    }
    sw_i2c_stop(actived_i2c);
    return status;
}

/**
  \fn
  \brief
  \return
*/
ATTRIBUTE_GCC_HEAD 
static int32_t soft_i2c_read_bytes(uint32_t addr, uint8_t *data, uint32_t length, bool xfer_pending)
{
    int32_t status = ARM_I2C_EVENT_TRANSFER_DONE;
    if (length == 0)
    {
        return status;
    }
    sw_i2c_start(actived_i2c);
    status = soft_i2c_send_byte(actived_i2c, (addr << 1U) + 0x1);
    if (status != ARM_I2C_EVENT_TRANSFER_DONE)
    {
        sw_i2c_stop(actived_i2c);
        return status;
    }
    for (volatile uint32_t i = 0; i < length; i++)
    {
        data[i] = soft_i2c_read_byte(actived_i2c, i < (length - 1));
    }
    sw_i2c_stop(actived_i2c);
    return status;
}
/**
  \fn
  \brief
  \return
*/
ATTRIBUTE_GCC_HEAD 
static uint8_t soft_i2c_check(ec_i2c_bus_t *i2c, uint8_t addr)
{ 
    uint8_t ret = 0;
    sw_i2c_start(i2c);
    int32_t status = soft_i2c_send_byte(i2c, (addr << 1));
    if (status != ARM_I2C_EVENT_ADDRESS_NACK)
    {
        ret = 1;
    }
    return ret;
}

api_ret_t api_sw_i2c_setup(int8_t index, i2c_config_t* para)
{
    if(index == 2)  // 当前只实现了一路SW I2C2
    {
        uint8_t paddr = para->sda;
        int8_t mux = PAD_MUX_ALT0;
        if(check_pad_mux(paddr, mux) != OPEN_HAL_DONE)
        {
            EPAT_LOG(api_sw_i2c_setup_1, P_ERROR, "sw i2c%d,checkout sda pad%d fail", index, para->sda);
            return OPEN_HAL_NONE ; // 没有对应设备
        }
        paddr = para->scl;
        mux = PAD_MUX_ALT0;
        if(check_pad_mux(paddr, mux) != OPEN_HAL_DONE)
        {
            EPAT_LOG(api_sw_i2c_setup_2, P_ERROR, "sw i2c%d,checkout scl pad%d fail", index, para->scl);
            return OPEN_HAL_NONE ; // 没有对应设备
        }
        // 初始化GPIO
        int8_t sda = list_pad2gpio[para->sda];
        int8_t scl = list_pad2gpio[para->scl];
        GpioPinConfig_t pinConfig = {0};
        pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
        pinConfig.misc.initOutput = 1;
        if(sda >= 0 && scl >= 0){
            GPIO_pinConfig(sda/16, sda%16, &pinConfig);
            GPIO_pinConfig(scl/16, scl%16, &pinConfig);
            EPAT_LOG(api_sw_i2c_setup_ok, P_INFO, "sw i2c%d,sda %d(%d),scl %d(%d),clk %d", index, sda, para->sda, scl, para->scl, para->clk);
            return OPEN_HAL_DONE;
        }
    }
    return OPEN_HAL_INVALID_PARA;
}

/* ---------------------------------------------------------------------------------------------- */
extern int8_t ec_i2c_checkout(uint8_t index, int8_t sda, int8_t scl);
/**
  \fn
  \brief    接口用于确认资源是否存在，传入的是paddr参数不是gpio需要转换
  \return   返回 0 无效
*/
extern const int8_t list_pad2gpio[];
uint32_t api_sw_i2c_create(int8_t index, i2c_config_t* cfg)
{
    uint32_t usrId = 0;
    if(cfg == NULL) 
    {
        EPAT_LOG(api_sw_i2c_create_0, P_WARNING, "sw i2c%d invalid cfg", index);
        return usrId;
    }
    int8_t sda = cfg->sda;  // list_pad2gpio[cfg->sda];
    int8_t scl = cfg->scl;  // list_pad2gpio[cfg->scl];
    int8_t select = ec_i2c_checkout(index, sda, scl);
    if(select < 0){
        EPAT_LOG(api_sw_i2c_create_1, P_WARNING, "sw i2c%d checkout fail %d with sda=gpio%d,sda=gpio%d,clk %d", index, select, sda, scl, cfg->clk);
        return usrId;
    }
    if(index >= EC_I2C_INDEX_LIMIT)
    {
        usrId = index;
        usrId |= OPEN_HAL_STAT_UNUSED;
    }
    ec_i2c_bus_t *i2c = &soft_i2c;  // 后续可扩展更多，通过index索引
    sda = list_pad2gpio[cfg->sda];  // 转换后是gpio编号
    scl = list_pad2gpio[cfg->scl];  // 转换后是gpio编号
    EPAT_LOG(api_sw_i2c_create_2, P_DEBUG, "%x,sw i2c%d,sda %d-gpio%d,sda %d-gpio%d", usrId, index, cfg->sda, sda, cfg->scl, scl);
    if(sda >=0 && sda <= 38){
        i2c->cfg.sda_port = (sda/16);
        i2c->cfg.sda_pin = (sda%16);
        i2c->cfg.sda_mask = 0x1U << (sda%16);
    }
    if(scl <= 38 && scl >=0){
        i2c->cfg.scl_port = (scl/16);
        i2c->cfg.scl_pin = (scl%16);
        i2c->cfg.scl_mask = 0x1U << (scl%16);
    }
    i2c->cfg.speed = cfg->clk;
    i2c->MasterTransmit = (int32_t (*)(uint32_t, uint8_t*, uint32_t, bool))soft_i2c_send_bytes;
    i2c->MasterReceive = soft_i2c_read_bytes;
    i2c->CheckAddress = (uint8_t (*)(ec_i2c_cfg_t*, uint8_t))soft_i2c_check;
    EPAT_LOG(api_sw_i2c_create_3, P_INFO, "%x,sw i2c%d,sda=gpio%d,scl=gpio%d,clk %d", usrId, index, sda, scl, cfg->clk);
    return usrId;
}

/**
  \fn
  \brief
  \return 
*/
api_ret_t api_sw_i2c_delete(uint32_t usrId)
{
    return OPEN_HAL_DONE;
}

/**
  \fn
  \brief    针对sw i2c的特定接口，方便扩展多个，后续配置gpio应该放这里
  \return
*/
api_ret_t api_sw_i2c_open(int8_t index, void **i2cDevice)
{
    // void *i2cDevice = NULL;
    // if(index < EC_I2C_INDEX_LIMIT){
    //     EPAT_LOG(api_sw_i2c_open_0, P_WARNING, "sw i2c index %d invalid,(index>=%d)", index, EC_I2C_INDEX_LIMIT);
    // }
    if(index  >= EC_I2C_INDEX_LIMIT)
    {
        // i2c_config_t Config = {0};
        // Config.sda = sda;
        // Config.scl = scl;
        // Config.clk = ARM_I2C_BUS_SPEED_FAST;
        // Config.poll = POLLING_MODE;
        // api_ret_t check = api_i2c_setup(index, &Config);
        actived_i2c = &soft_i2c;
        *i2cDevice = (void *)actived_i2c;
        return OPEN_HAL_DONE;
    }
    return OPEN_HAL_INVALID_PARA;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_sw_i2c_close(uint32_t usrId)
{
    return OPEN_HAL_DONE;
}