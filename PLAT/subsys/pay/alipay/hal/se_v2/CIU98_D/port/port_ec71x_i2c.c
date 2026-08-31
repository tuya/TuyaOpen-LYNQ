/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		port_stm32l433_i2c.c
 Author:			zhengwd
 Version:			V1.0
 Date:			2020-04-07
 Description:
 History:

******************************************************************************/

/***************************************************************************
 * Include Header Files
 ***************************************************************************/
#include "port_ec71x_i2c.h"
#include "hed_private.h"
#include "hal_i2c.h"
#include "pad.h"
#include "gpio.h"
#include "port_ec71x_util.h"
#ifdef HED_I2C

/**************************************************************************
 * Global Variable Declaration
 ***************************************************************************/
#if defined(HED_I2C_SE0)
int i2c_comm_handle_slave0 = 0;
i2c_comm_param_t i2c_comm_parm_slave0 = {&i2c_comm_handle_slave0, PORT_I2C_ADDRESS_2A, I2C_PERIPHERAL_SE0, FALSE};
#endif

#if defined(HED_I2C_SE1)
int i2c_comm_handle_slave1 = 0;
i2c_comm_param_t i2c_comm_parm_slave1 = {&i2c_comm_handle_slave1, PORT_I2C_ADDRESS_2C, I2C_PERIPHERAL_SE1, FALSE};
#endif
static uint8_t g_i2c_device_init[MAX_PERIPHERAL_DEVICE] = {FALSE};

/*********************************************************************************
Function:       HAL_I2C_MspInit
Description:    This function configures the i2c hardware resources
Input:          hi2c I2C handle pointer
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void HAL_I2C_MspInit(void *hi2c)
{
}

/*********************************************************************************
Function:       HAL_I2C_MspDeInit
Description:    This function frees the i2c hardware resources
Input:          hi2c  I2C handle pointer
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void HAL_I2C_MspDeInit(void *hi2c)
{

}

#if defined(HED_I2C_SE0)
/*********************************************************************************
Function:       port_i2c_se0_gpio_init
Description:    外设SE0的gpio初始化：RST
Input:          no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void port_i2c_se0_gpio_init(void)
{
    //---SE RST 控制IO----
    ECPLAT_PRINTF(UNILOG_PLA_APP, port_i2c_se0_gpio_init, P_WARNING, "se0 reset io init");
    PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(PORT_I2C_SE0_RST_PAD_INDEX, &padConfig);
    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 1;
    GPIO_pinConfig(PORT_I2C_SE0_RST_IO_PORT, PORT_I2C_SE0_RST_IO_PIN, &config);
}
#endif

#if defined(HED_I2C_SE1)
/*********************************************************************************
Function:       port_i2c_se1_gpio_init
Description:    外设SE1的gpio初始化：RST
Input:          no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void port_i2c_se1_gpio_init(void)
{

}
#endif

/*********************************************************************************
Function:       port_i2c_init
Description:    i2c接口初始化，获取设备句柄
Input:          i2c_handle_instance   I2C handle pointer
                i2c_addr 从设备地址
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_init(void *i2c_handle_instance, uint8_t i2c_addr)
{
    if (halI2cInit(true) != 0)
    {
        return SE_ERR_INIT;
    }

    return SE_SUCCESS;
}

/*********************************************************************************
Function:       port_i2c_deinit
Description:    I2C接口移除，获取设备句柄清除
Input:          handle  I2C handle pointer
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_deinit(void *handle)
{
    if (halI2cDeInit(true) != 0)
    {
        return SE_ERR_INIT;
    }

    return SE_SUCCESS;
}
/*********************************************************************************
Function:       port_spi_periph_init
Description:   从设备periph通信初始化
               1. 调用port_i2c_init函数初始化SPI接口
               2. 初始化控制GPIO
Input:          periph 设备句柄
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_init(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            break;
        }

        if (g_i2c_device_init[p_comm_param->slave_id] == FALSE)
        {
            ret_code = port_i2c_init(p_comm_param->i2c_handle, p_comm_param->slave_addr);
            if (ret_code != SE_SUCCESS)
            {
                break;
            }
            g_i2c_device_init[p_comm_param->slave_id] = TRUE;
        }

        if (p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
        {
#if defined(HED_I2C_SE0)
            port_i2c_se0_gpio_init();
#if defined(SE_CIU98_D)
            PORT_I2C_SE0_RST_HIGH(); // 高电平
#endif

#if defined(HED_I2C_DUAL)
            port_i2c_se1_gpio_init();
            PORT_I2C_SE1_RST_HIGH(); // 高电平
#endif
#endif
        }

        else if (p_comm_param->slave_id == I2C_PERIPHERAL_SE1)
        {
#if defined(HED_I2C_SE1)
            port_i2c_se1_gpio_init();
            PORT_I2C_SE1_RST_HIGH(); // 高电平
#endif

#if defined(HED_I2C_DUAL)
            port_i2c_se0_gpio_init();
            PORT_I2C_SE0_RST_HIGH(); // 高电平
#endif
        }

    } while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_deinit
Description:    从设备periph通信终止化
               1. 调用port_i2c_deinit函数终止化SPI接口
               2. 设置RST控制IO为低电平
Input:          periph 设备句柄
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_deinit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            break;
        }

        // 终止化当前外设的I2C时，需检查其它外设是否已初始化，以防止对其它已初始化的I2C外设产生影响
        if (p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
        {
            if ((g_i2c_device_init[I2C_PERIPHERAL_SE0] == TRUE) && (g_i2c_device_init[I2C_PERIPHERAL_SE1] == FALSE))
            {
                ret_code = port_i2c_deinit(p_comm_param->i2c_handle);
            }
#if defined(HED_I2C_SE0)
            PORT_I2C_SE0_RST_LOW(); // 低电平
#endif
        }

        else if (p_comm_param->slave_id == I2C_PERIPHERAL_SE1)
        {
            if ((g_i2c_device_init[I2C_PERIPHERAL_SE1] == TRUE) && (g_i2c_device_init[I2C_PERIPHERAL_SE0] == FALSE))
            {
                ret_code = port_i2c_deinit(p_comm_param->i2c_handle);
            }
#if defined(HED_I2C_SE1)
            PORT_I2C_SE1_RST_LOW(); // 低电平
#endif
        }

        g_i2c_device_init[p_comm_param->slave_id] = FALSE;

    } while (0);

    return ret_code;
}

/*********************************************************************************
Function:      port_i2c_periph_power_on
Description:   外设上电，Demo板无上电控制操作
Input:          periph 设备句柄
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_power_on(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    return SE_SUCCESS;
}

/*********************************************************************************
Function:       port_i2c_periph_power_off
Description:   外设下电，Demo板无下电控制操作
Input:          periph 设备句柄
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_power_off(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    return SE_SUCCESS;
}

/*********************************************************************************
Function:       port_i2c_periph_lock
Description:   对从设备periph上锁，函数调用时，若已上锁，返回错误状态码
Input:          periph 设备句柄
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_lock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            break;
        }

        if (p_comm_param->locked == TRUE)
        {
            ret_code = SE_ERR_BUSY;
            break;
        }
        p_comm_param->locked = TRUE;

    } while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_unlock
Description:   对从设备periph解锁
Input:          periph 设备句柄
Output:         no
Return:         函数操作状态码
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_unlock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    // if(p_comm_param->locked==FALSE)
    //{
    //	return PORT_I2C_ERR_LOCK;
    // }
    p_comm_param->locked = FALSE;

    return SE_SUCCESS;
}

/*********************************************************************************
Function:       port_i2c_periph_transmit
Description:   通过I2C接口向从设备periph发送多字节数据
               1.调用mcu hal库的HAL_I2C_Master_Transmit函数发送多字节数据
Input:       periph 设备句柄
             inbuf 待发送数据的起始地址
             inbuf_len 待发送数据的长度
Output:      no
Return:      函数操作状态码
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_transmit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t inbuf_len)
{
    uint8_t rxNack = 0;
    se_error_t ret_code = SE_SUCCESS;
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            break;
        }

        if ((inbuf == NULL) || (inbuf_len == 0U))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            break;
        }

        if (g_i2c_device_init[p_comm_param->slave_id] == FALSE)
        {
            ret_code = SE_ERR_COMM;
            LOGE("Failed:i2c has no init!,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = halI2cWrite((uint8_t)p_comm_param->slave_addr, inbuf, inbuf_len, &rxNack, true);

        if (rxNack == 1)
        {
            ret_code = SE_ERR_BUSY;
            break;
        }
        else if (ret_code != 0)
        {
            ret_code = SE_ERR_COMM;
            break;
        }

    } while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_receive
Description:   通过I2C 接口从从设备periph接收多字节数据
             1.调用mcu hal库的HAL_I2C_Master_Receive函数接收多字节数据
Input:       periph 设备句柄
             outbuf 待接收数据的起始地址
             outbuf_len 待接收数据的长度
Output:      no
Return:      函数操作状态码
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_receive(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            LOGE("Failed:i2c handle invalid!,  ErrCode-%08X.", ret_code);
            break;
        }

        if ((outbuf == NULL) || (outbuf_len == NULL))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            LOGE("Failed:i2c param invalid!,  ErrCode-%08X.", ret_code);
            break;
        }

        if (g_i2c_device_init[p_comm_param->slave_id] == FALSE)
        {
            ret_code = SE_ERR_COMM;
            LOGE("Failed:i2c has no init!,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = halI2cReadBuf((uint8_t)p_comm_param->slave_addr,(uint16_t)(*outbuf_len), outbuf, true);
        if (ret_code != 0)
        {
            ret_code = SE_ERR_COMM;
            break;
        }

    } while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_control
Description:   根据控制码和输入数据，对从设备periph进行控制操作
Input:         periph 设备句柄
             ctrlcode 控制码
             inbuf 发送控制数据的起始地址
             inbuf_len 发送控制数据的长度
Output:      no
Return:      函数操作状态码
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_control(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t inbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            break;
        }

        if (ctrlcode == PORT_I2C_CTRL_RST)
        {
            if (p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
            {
#if defined(HED_I2C_SE0)
                PORT_I2C_SE0_RST_LOW();
                hal_delay(PORT_I2C_SE_RST_LOW_DELAY); // 复位时，RST低电平持续时间
                PORT_I2C_SE0_RST_HIGH();
                hal_delay(PORT_I2C_SE_RST_HIGH_DELAY); // 复位后，RST高电平持续时间
#endif
            }
            else if (p_comm_param->slave_id == I2C_PERIPHERAL_SE1)
            {
#if defined(HED_I2C_SE1)
                PORT_I2C_SE1_RST_LOW();
                hal_delay(PORT_I2C_SE_RST_LOW_DELAY); // 复位时，RST低电平持续时间
                PORT_I2C_SE1_RST_HIGH();
                hal_delay(PORT_I2C_SE_RST_HIGH_DELAY); // 复位后，RST高电平持续时间
#endif
            }
        }
    } while (0);
    return ret_code;
}

#if defined(HED_I2C_SE0)
I2C_PERIPHERAL_DEFINE_BEGIN(I2C_PERIPHERAL_SE0)
    port_i2c_periph_init,
    port_i2c_periph_deinit,
    port_i2c_periph_power_on,
    port_i2c_periph_power_off,
    port_i2c_periph_lock,
    port_i2c_periph_unlock,
    port_i2c_periph_transmit,
    port_i2c_periph_receive,
    port_i2c_periph_control,
    &i2c_comm_parm_slave0,
    I2C_PERIPHERAL_DEFINE_END()

I2C_PERIPHERAL_REGISTER(I2C_PERIPHERAL_SE0);
#endif

#if defined(HED_I2C_SE1)
I2C_PERIPHERAL_DEFINE_BEGIN(I2C_PERIPHERAL_SE1)
    port_i2c_periph_init,
    port_i2c_periph_deinit,
    port_i2c_periph_power_on,
    port_i2c_periph_power_off,
    port_i2c_periph_lock,
    port_i2c_periph_unlock,
    port_i2c_periph_transmit,
    port_i2c_periph_receive,
    port_i2c_periph_control,
    &i2c_comm_parm_slave1,
    I2C_PERIPHERAL_DEFINE_END()

I2C_PERIPHERAL_REGISTER(I2C_PERIPHERAL_SE1);
#endif

#endif // #ifdef HED_I2C
