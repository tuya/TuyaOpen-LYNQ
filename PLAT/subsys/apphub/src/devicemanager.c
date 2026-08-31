
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "clock.h"
#include "devicemanager.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#else
#define SYSLOG_DEBUG(fmt, ...)
#define SYSLOG_INFO(fmt, ...)  
#define SYSLOG_NOTICE(fmt, ...) 
#define SYSLOG_WARNING(fmt, ...) 
#define SYSLOG_ERR(fmt, ...) 
#define SYSLOG_CRIT(fmt, ...) 
#define SYSLOG_ALERT(fmt, ...)  
#define SYSLOG_EMERG(fmt, ...) 
#endif
#include "device_parser.h"
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_def.h"
#include "api_comm.h"
#include "dev_comm.h"
#else
typedef enum
{
    OPEN_HAL_DONE   = 0,    //执行成功
    OPEN_HAL_LOCK   ,       //已锁定不释放
    OPEN_HAL_NONE   ,       //没有查到对应设备
    OPEN_HAL_FREE   ,       //未初始化
    OPEN_HAL_IDLE   ,       //初始化，未使用
    OPEN_HAL_USED   ,       //初始化，已使用
    OPEN_HAL_INVALID_PARA,  //参数错误
    OPEN_HAL_MAX            
} api_ret_t;

typedef struct
{
    PmMode_e mode;
    RuntimeMode_e runtime;
} open_hal_pm_t;

#endif

DevTable_t gDevTab={0};

//静态表文件，用于资源依赖查询
const Ec718IpRes ec718_ip_res={
    .gpio[0] = {.ip_type=IP_GPIO,.ip_index=0,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_15 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[1] = {.ip_type=IP_GPIO,.ip_index=1,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_16 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[2] = {.ip_type=IP_GPIO,.ip_index=2,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_17 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[3] = {.ip_type=IP_GPIO,.ip_index=3,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_18 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[4] = {.ip_type=IP_GPIO,.ip_index=4,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_19 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[5] = {.ip_type=IP_GPIO,.ip_index=5,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_20 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[6] = {.ip_type=IP_GPIO,.ip_index=6,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_21 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[7] = {.ip_type=IP_GPIO,.ip_index=7,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_22 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[8] = {.ip_type=IP_GPIO,.ip_index=8,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_23 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[9] = {.ip_type=IP_GPIO,.ip_index=9,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_24 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[10] = {.ip_type=IP_GPIO,.ip_index=10,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_25 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[11] = {.ip_type=IP_GPIO,.ip_index=11,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_26 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[12] = {.ip_type=IP_GPIO,.ip_index=12,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_27 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[13] = {.ip_type=IP_GPIO,.ip_index=13,.ip_pad[PIN_SLOT0]={.gpio_pad_selector= PAD_28 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[14] = {.ip_type=IP_GPIO,.ip_index=14,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_29 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[15] = {.ip_type=IP_GPIO,.ip_index=15,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_30 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},

    .gpio[16] = {.ip_type=IP_GPIO,.ip_index=16,
        .ip_pad[PIN_SLOT0]={
            .gpio_pad_selector = PAD_31 | PIN_Alt0<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .gpio_pad_selector = PAD_11 | PIN_Alt4<<PIN_ALT_FUNC_OFF}},
    .gpio[17] = {.ip_type=IP_GPIO,.ip_index=17,
        .ip_pad[PIN_SLOT0]={
            .gpio_pad_selector = PAD_32 | PIN_Alt0<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .gpio_pad_selector = PAD_12 | PIN_Alt4<<PIN_ALT_FUNC_OFF}},
    .gpio[18] = {.ip_type=IP_GPIO,.ip_index=18,
        .ip_pad[PIN_SLOT0]={
            .gpio_pad_selector = PAD_33 | PIN_Alt0<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .gpio_pad_selector = PAD_13 | PIN_Alt4<<PIN_ALT_FUNC_OFF }},
    .gpio[19] = {.ip_type=IP_GPIO,.ip_index=19,
        .ip_pad[PIN_SLOT0]={
            .gpio_pad_selector = PAD_34 | PIN_Alt0<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1] ={
            .gpio_pad_selector = PAD_14 | PIN_Alt4<<PIN_ALT_FUNC_OFF }
        },

    .gpio[20] = {.ip_type=IP_AGPIO,.ip_index=20,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_45 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[21] = {.ip_type=IP_AGPIO,.ip_index=21,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_46 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[22] = {.ip_type=IP_AGPIO,.ip_index=22,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_47 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[23] = {.ip_type=IP_AGPIO,.ip_index=23,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_48 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[24] = {.ip_type=IP_AGPIO,.ip_index=24,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_49 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[25] = {.ip_type=IP_AGPIO,.ip_index=25,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_50 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[26] = {.ip_type=IP_AGPIO,.ip_index=26,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_51 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[27] = {.ip_type=IP_AGPIO,.ip_index=27,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_52 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[28] = {.ip_type=IP_AGPIO,.ip_index=28,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_53 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},

    .gpio[29] = {.ip_type=IP_GPIO,.ip_index=29,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_35 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[30] = {.ip_type=IP_GPIO,.ip_index=30,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_36 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[31] = {.ip_type=IP_GPIO,.ip_index=31,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_37 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[32] = {.ip_type=IP_GPIO,.ip_index=32,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_38 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[33] = {.ip_type=IP_GPIO,.ip_index=33,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_39 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[34] = {.ip_type=IP_GPIO,.ip_index=34,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_40 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[35] = {.ip_type=IP_GPIO,.ip_index=35,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_41 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[36] = {.ip_type=IP_GPIO,.ip_index=36,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_42 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[37] = {.ip_type=IP_GPIO,.ip_index=37,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_43 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},
    .gpio[38] = {.ip_type=IP_GPIO,.ip_index=38,.ip_pad[PIN_SLOT0]={.gpio_pad_selector = PAD_44 | PIN_Alt0<<PIN_ALT_FUNC_OFF }},

    .i2c[0] = {.ip_type=IP_I2C,.ip_index=0,.ip_pad[PIN_SLOT0]= {
                .i2c_sck_selector = PAD_13 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                .i2c_sda_selector = PAD_14 | PIN_Alt2<<PIN_ALT_FUNC_OFF},
            .ip_pad[PIN_SLOT1]={
                 .i2c_sck_selector = PAD_30 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_29 | PIN_Alt2<<PIN_ALT_FUNC_OFF},             
            .ip_pad[PIN_SLOT2]= {
                 .i2c_sck_selector = PAD_32 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_31 | PIN_Alt2<<PIN_ALT_FUNC_OFF},                  
            .ip_pad[PIN_SLOT3]= {
                 .i2c_sck_selector = PAD_41 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_40 | PIN_Alt2<<PIN_ALT_FUNC_OFF}},
    .i2c[1] = {.ip_type=IP_I2C,.ip_index=1,.ip_pad[PIN_SLOT0]={
                 .i2c_sck_selector = PAD_13 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_14 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
            .ip_pad[PIN_SLOT1]={
                 .i2c_sck_selector = PAD_19 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_20 | PIN_Alt2<<PIN_ALT_FUNC_OFF},
            .ip_pad[PIN_SLOT2]={
                 .i2c_sck_selector = PAD_24 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_23 | PIN_Alt2<<PIN_ALT_FUNC_OFF},
            .ip_pad[PIN_SLOT3]={
                 .i2c_sck_selector = PAD_42 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
                 .i2c_sda_selector = PAD_43 | PIN_Alt2<<PIN_ALT_FUNC_OFF}},

    .spi[0] = {.ip_type=IP_SPI,.ip_index=0,.ip_pad[PIN_SLOT0]= \
                {.spi_sclk_selector = PAD_26 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
                 .spi_miso_selector = PAD_25 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
                 .spi_mosi_selector = PAD_24 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
                 .spi_ss_selector = PAD_23 | PIN_Alt1<<PIN_ALT_FUNC_OFF }},
    .spi[1] = {.ip_type=IP_SPI,.ip_index=1,.ip_pad[PIN_SLOT0]= \
                {.spi_sclk_selector = PAD_30 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
                 .spi_miso_selector = PAD_29 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
                 .spi_mosi_selector = PAD_28 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
                 .spi_ss_selector = PAD_27 | PIN_Alt1<<PIN_ALT_FUNC_OFF}},
    
    .uart[0] = {.ip_type=IP_UART,.ip_index=0,.ip_pad[PIN_SLOT0]={
            .uart_rx_selector = PAD_31 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector = PAD_32 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .uart_rtx_selector= PAD_23 | PIN_Alt4<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_24 | PIN_Alt4<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_43 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .uart_rtx_selector= PAD_42 | PIN_Alt3<<PIN_ALT_FUNC_OFF}},
    .uart[1] = {.ip_type=IP_UART,.ip_index=1,.ip_pad[PIN_SLOT0]={
            .uart_rx_selector = PAD_33 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector = PAD_34 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .uart_rtx_selector= PAD_16 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_17 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_dcd_selector= PAD_16 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
            .uart_dtr_selector= PAD_17 | PIN_Alt2<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .uart_rtx_selector= PAD_19 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_20 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
            .uart_rtx_selector= PAD_21 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_22 | PIN_Alt3<<PIN_ALT_FUNC_OFF}}, 
    .uart[2] = {.ip_type=IP_UART,.ip_index=2,.ip_pad[PIN_SLOT0]={
            .uart_rx_selector = PAD_21 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector= PAD_22 | PIN_Alt2<<PIN_ALT_FUNC_OFF,
            .uart_rtx_selector= PAD_23 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_24 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .uart_rx_selector = PAD_25 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector = PAD_26 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
            .uart_rx_selector = PAD_27 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector = PAD_28 | PIN_Alt3<<PIN_ALT_FUNC_OFF}},
    .uart[3] = {.ip_type=IP_UART,.ip_index=3,.ip_pad[PIN_SLOT0]={
            .uart_rx_selector = PAD_29 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector = PAD_30 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_rtx_selector= PAD_27 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .uart_ctx_selector= PAD_28 | PIN_Alt5<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .uart_rx_selector = PAD_40 | PIN_Alt3<<PIN_ALT_FUNC_OFF,
            .uart_tx_selector = PAD_41 | PIN_Alt3<<PIN_ALT_FUNC_OFF}},

    .pwm[0] = {.ip_type=IP_PWM,.ip_index=0,.ip_pad[PIN_SLOT0]={
            .pwm_pad_selector = PAD_13 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_49 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .pwm_pad_selector = PAD_16 | PIN_Alt5<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
            .pwm_pad_selector = PAD_29 | PIN_Alt5<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT3]={
            .pwm_pad_selector = PAD_35 | PIN_Alt5<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT4]={
            .pwm_pad_selector = PAD_48 | PIN_Alt5<<PIN_ALT_FUNC_OFF}},
    .pwm[1] = {.ip_type=IP_PWM,.ip_index=1,.ip_pad[PIN_SLOT0]={
            .pwm_pad_selector = PAD_14 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_48 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .pwm_pad_selector = PAD_17 | PIN_Alt5<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
            .pwm_pad_selector = PAD_30 | PIN_Alt5<<PIN_ALT_FUNC_OFF},             
        .ip_pad[PIN_SLOT3]={
            .pwm_pad_selector = PAD_36 | PIN_Alt5<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT4]={
            .pwm_pad_selector = PAD_49 | PIN_Alt5<<PIN_ALT_FUNC_OFF}},
    .pwm[2] = {.ip_type=IP_PWM,.ip_index=2,.ip_pad[PIN_SLOT0]={
            .pwm_pad_selector= PAD_18 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_51 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .pwm_pad_selector = PAD_37 | PIN_Alt5<<PIN_ALT_FUNC_OFF}, 
        .ip_pad[PIN_SLOT2]={
            .pwm_pad_selector = PAD_50 | PIN_Alt5<<PIN_ALT_FUNC_OFF}},
    .pwm[3] = {.ip_type=IP_PWM,.ip_index=3,.ip_pad[PIN_SLOT0]={
            .pwm_pad_selector = PAD_38 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_46 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
                .pwm_pad_selector = PAD_45 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
                .npwm_pad_selector = PAD_50 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
                .pwm_pad_selector = PAD_51 | PIN_Alt5<<PIN_ALT_FUNC_OFF}},
    .pwm[4] = {.ip_type=IP_PWM,.ip_index=4,.ip_pad[PIN_SLOT0]={
            .pwm_pad_selector = PAD_39 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_47 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .pwm_pad_selector = PAD_46 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_53 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2] ={
            .pwm_pad_selector= PAD_52 | PIN_Alt5<<PIN_ALT_FUNC_OFF}},
    .pwm[5] = {.ip_type=IP_PWM,.ip_index=5,.ip_pad[PIN_SLOT0]={
            .pwm_pad_selector = PAD_47 | PIN_Alt5<<PIN_ALT_FUNC_OFF,
            .npwm_pad_selector = PAD_52 | PIN_Alt3<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .pwm_pad_selector = PAD_53 | PIN_Alt5<<PIN_ALT_FUNC_OFF}},
    
    .apwm[0] = {.ip_type=IP_APWM,.ip_index=0,.ip_pad[PIN_SLOT0]={
        .apwm_pad_selector = PAD_48 | PIN_Alt0<<PIN_ALT_FUNC_OFF}},
    .apwm[1] = {.ip_type=IP_APWM,.ip_index=1,.ip_pad[PIN_SLOT0]={
        .apwm_pad_selector = PAD_49 | PIN_Alt0<<PIN_ALT_FUNC_OFF}},
    .apwm[2] = {.ip_type=IP_APWM,.ip_index=2,.ip_pad[PIN_SLOT0]={
        .apwm_pad_selector = PAD_50 | PIN_Alt0<<PIN_ALT_FUNC_OFF}},

    .i2s[0] = {.ip_type=IP_I2S,.ip_index=0,.ip_pad[PIN_SLOT0]={
        .i2s_mclk_selector = PAD_39 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_bclk_selector = PAD_35 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_lrck_selector = PAD_36 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_din_selector = PAD_37 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_dout_selector = PAD_38 | PIN_Alt1<<PIN_ALT_FUNC_OFF}},

    .i2s[1] = {.ip_type=IP_I2S,.ip_index=1,.ip_pad[PIN_SLOT0]={
            .i2s_mclk_selector = PAD_18 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .i2s_bclk_selector = PAD_19 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .i2s_lrck_selector = PAD_20 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .i2s_din_selector = PAD_21 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
            .i2s_dout_selector = PAD_22 | PIN_Alt1<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
                .i2s_mclk_selector = PAD_37 | PIN_Alt4<<PIN_ALT_FUNC_OFF}},

    .i2s[2] = {.ip_type=IP_I2S,.ip_index=2,.ip_pad[PIN_SLOT0]={
        .i2s_mclk_selector = PAD_44 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_bclk_selector = PAD_40 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_lrck_selector = PAD_41 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_din_selector = PAD_42 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .i2s_dout_selector = PAD_43 | PIN_Alt1<<PIN_ALT_FUNC_OFF}},

    .lspi = {.ip_type=IP_LSPI,.ip_index=0,.ip_pad[PIN_SLOT0]={
        .lspi_clk_selector = PAD_40 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .lspi_cs_selector = PAD_41 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .lspi_sda_selector = PAD_43 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .lspi_wr_selector = PAD_44 | PIN_Alt2<<PIN_ALT_FUNC_OFF}},

    .cspi = {.ip_type=IP_CSPI,.ip_index=0,.ip_pad[PIN_SLOT0]={
        .cspi_mclk_selector = PAD_18 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .cspi_bclk_selector = PAD_19 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .cspi_cs_selector = PAD_20 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .cspi_rx0_selector = PAD_21 | PIN_Alt1<<PIN_ALT_FUNC_OFF,
        .cspi_rx1_selector = PAD_22 | PIN_Alt1<<PIN_ALT_FUNC_OFF}},

    .onewire = {.ip_type=IP_ONEWIRE,.ip_index=0,.ip_pad[PIN_SLOT0]={
            .onewire_pad_selector = PAD_17 | PIN_Alt4<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .onewire_pad_selector = PAD_18 | PIN_Alt4<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
            .onewire_pad_selector = PAD_22 | PIN_Alt4<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT3]={
            .onewire_pad_selector = PAD_53 | PIN_Alt4<<PIN_ALT_FUNC_OFF}},
        
    .adc[0] = {.ip_type=IP_ADC,.ip_index=0},
    .adc[1] = {.ip_type=IP_ADC,.ip_index=1},
    .adc[2] = {.ip_type=IP_ADC,.ip_index=2},
    .adc[3] = {.ip_type=IP_ADC,.ip_index=3},
    
    .kpc = {.ip_type=IP_KPC,.ip_index=0,.ip_pad[PIN_SLOT0]={
            .Kpc_c0_pad_selector = PAD_28 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c1_pad_selector = PAD_27 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c2_pad_selector = PAD_22 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c3_pad_selector = PAD_21 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c4_pad_selector = PAD_14 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r0_pad_selector = PAD_20 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r1_pad_selector = PAD_19 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r2_pad_selector = PAD_17 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r3_pad_selector = PAD_16 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r4_pad_selector = PAD_13 | PIN_Alt6<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .Kpc_c2_pad_selector = PAD_30 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c3_pad_selector = PAD_29 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c4_pad_selector = PAD_18 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r0_pad_selector = PAD_52 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r1_pad_selector = PAD_51 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r2_pad_selector = PAD_50 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r3_pad_selector = PAD_49 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r4_pad_selector = PAD_15 | PIN_Alt6<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT2]={
            .Kpc_c2_pad_selector = PAD_45 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c3_pad_selector = PAD_46 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_c4_pad_selector = PAD_47 | PIN_Alt6<<PIN_ALT_FUNC_OFF,
            .Kpc_r4_pad_selector = PAD_48 | PIN_Alt6<<PIN_ALT_FUNC_OFF}},

    .swd[0] = {.ip_type=IP_SWD,.ip_index=0,.ip_pad[PIN_SLOT0]={
        .swd_clk_selector = PAD_11 | PIN_Alt0<<PIN_ALT_FUNC_OFF,
        .swd_dio_selector = PAD_12 | PIN_Alt0<<PIN_ALT_FUNC_OFF}},
    .swd[1] = {.ip_type=IP_SWD,.ip_index=1,.ip_pad[PIN_SLOT0]={
        .swd_clk_selector = PAD_13 | PIN_Alt0<<PIN_ALT_FUNC_OFF,
        .swd_dio_selector = PAD_14 | PIN_Alt0<<PIN_ALT_FUNC_OFF}},

    .sim = {.ip_type=IP_SIM,.ip_index=0,.ip_pad[PIN_SLOT0]={
            .sim_clk_selector = PAD_20 | PIN_Alt4<<PIN_ALT_FUNC_OFF,
            .sim_uio_selector = PAD_21 | PIN_Alt4<<PIN_ALT_FUNC_OFF,
            .sim_rst_selector = PAD_19 | PIN_Alt4<<PIN_ALT_FUNC_OFF},
        .ip_pad[PIN_SLOT1]={
            .sim_clk_selector = PAD_29 | PIN_Alt4<<PIN_ALT_FUNC_OFF,
            .sim_uio_selector = PAD_27 | PIN_Alt4<<PIN_ALT_FUNC_OFF,
            .sim_rst_selector = PAD_28 | PIN_Alt4<<PIN_ALT_FUNC_OFF}
        }
};


// 最大设备数量
#define MAX_DEVICES 64

// 设备管理器全局变量
typedef struct {
    device_info_t devices[MAX_DEVICES];  // 设备数组
    uint32_t device_count;               // 当前设备数量
    void *global_mutex;                  // 全局互斥锁
    bool initialized;                    // 初始化标志
} device_manager_t;

static device_manager_t g_device_manager = {0};

// 内部函数声明
static int find_device_by_path(const char *device_path);
static int find_free_device_slot(void);
static int lock_device_mutex(int index);
static int unlock_device_mutex(int index);

#ifdef EXTERNAL_PIN_CONFIG
#include "pin_cfg.h"
#else
// 此处配置快速初始化的端口，适用于在应用加载前进行IO配置，请针对板级修改
#if (LCD_INTERFACE_SPI == 1)    // minidkb +st7789 二次配置
const int8_t pad_list[][4] = {
    {11, PAD_MUX_ALT4, PAD_AUTO_PULL, 0},           /* CAMERA external LDO control ->gpio16*/
    {15, PAD_MUX_ALT0, PAD_INTERNAL_PULL_DOWN, 0},  /* boot key useed as input gpio*/
    {14, PAD_MUX_ALT4, PAD_INTERNAL_PULL_UP, 0},    /* LCD_RST */
    {18, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* CAMERA CSPI1 MCLK*/
    {19, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* CAMERA CSPI1 PCLK*/
    {20, PAD_MUX_ALT0, PAD_AUTO_PULL, 0},           /* CAMERA RESET ->gpio5 */
    {21, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* CAMERA CSPI1 SDO0 */
    {22, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* CAMERA CSPI1 SDO1 */
    {23, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 0},    /* SPI0 SSn0 -> gpio8 */
    {24, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* SPI0 mosi */
    {25, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* SPI0 miso */
    {26, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* SPI0 miso */
    {40, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* USP2 CLK: gpio34 */
    {41, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* USP2 CSn: gpio35 */
    {43, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* USP2 MOSI: gpio37 */
    {44, PAD_MUX_ALT2, PAD_AUTO_PULL, 0},           /* USP2 wrx/dcx: gpio38 */
    {42, PAD_MUX_ALT0, PAD_INTERNAL_PULL_DOWN, 0},  /* TP IRQ: gpio36 */
    {16, PAD_MUX_ALT5, PAD_AUTO_PULL, 0},           /* LCD backlight pwm*/
    {47, PAD_MUX_ALT0, PAD_AUTO_PULL, 0},           /* LCD Power */
    {48, PAD_MUX_ALT0, PAD_AUTO_PULL, 0},           /* GNSS_MAIN */
    {53, PAD_MUX_ALT0, PAD_AUTO_PULL, 0},           /* GNSS_BCKP */
    {-1, PAD_MUX_ALT7, PAD_AUTO_PULL, 0}            /* End of list */
};  // 针对miniDKB st7789配置，不同硬件需要修改
const int8_t gpio_list[][4] = {
    {0, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_RISING_EDGE, 0},    /* boot key gpio 0 */  
    {5, GPIO_DIRECTION_OUTPUT, 0, 0},                            /* CAMERA RESET ->gpio5 */
    {16, GPIO_DIRECTION_OUTPUT, GPIO_INTERRUPT_DISABLED, 0},    /* CAMERA external LDO control: gpio16 */
    {36, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_RISING_EDGE, 0},  /* TP IRQ irsr: gpio36 */
    {19, GPIO_DIRECTION_OUTPUT, 0, 0},                          /* LCD reset: gpio19 */
    {22, GPIO_DIRECTION_OUTPUT, 1, 0},                          /* LCD Power */
    {23, GPIO_DIRECTION_OUTPUT, 1, 0},                          /* GNSS_MAIN */
    {28, GPIO_DIRECTION_OUTPUT, 1, 0},                          /* GNSS_BCKP */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, 0}      /* End of List */
};

#elif (LCD_INTERFACE_MSPI == 1) // minidkb +co5300 二次配置
const int8_t pad_list[][4] = {
    {51, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 0},    /* LCD RST: PAD51-GPIO26-AGPIO5 */
    {48, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 0},    /* TP RST: PAD48-GPIO23 */
    {46, PAD_MUX_ALT0, PAD_AUTO_PULL, 0},           /* TP IRQ: PAD46-GPIO21 */
    {34, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 0},    /* PWR EN: PAD34-GPIO19 */
    {23, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 1},    /* SPI0 SSn0: PAD23 */
    {24, PAD_MUX_ALT1, PAD_AUTO_PULL, 1},           /* SPI0 MOSI: PAD24 */
    {25, PAD_MUX_ALT1, PAD_AUTO_PULL, 1},           /* SPI0 MISO: PAD25 */
    {26, PAD_MUX_ALT1, PAD_AUTO_PULL, 1},           /* SPI0 SCLK: PAD26 */
    {29, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 0},    /* PAD29: GPIO14/I2C0_SDA */
    {30, PAD_MUX_ALT0, PAD_INTERNAL_PULL_UP, 0},    /* PAD29: GPIO14/I2C0_SDA */
    {14, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* MSPI TE: PAD14-gpio19 */
    {40, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* MSPI SCL: PAD40-gpio34 */
    {41, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* MSPI CSn: PAD41-gpio35 */
    {42, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* MSPI SDI: PAD42-gpio36 */
    {43, PAD_MUX_ALT4, PAD_AUTO_PULL, 0},           /* MSPI D0: pad43-gpio37 */
    {44, PAD_MUX_ALT4, PAD_AUTO_PULL, 0},           /* MSPI D1: pad44-gpio38 */
    {17, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* MSPI D2: pad17-gpio2 */
    {16, PAD_MUX_ALT1, PAD_AUTO_PULL, 0},           /* MSPI D3: pad16-gpio1 */
    {-1, PAD_MUX_ALT7, PAD_AUTO_PULL, 0}            /* End of List */
};
const int8_t gpio_list[][4] = {
    {26, GPIO_DIRECTION_OUTPUT, 1, 0},                          /* LCD_RST: GPIO26-AGPIO5 */
    {23, GPIO_DIRECTION_OUTPUT, 1, 0},                          /* TP_RST: GPIO23 */
    {21, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, 0},     /* TP_IRQ: GPIO21 使用timer */
    {19, GPIO_DIRECTION_OUTPUT, 1, 1},                          /* PWR_EN: GPIO19 */
    {8, GPIO_DIRECTION_OUTPUT, 1, 1},                           /* SPI0_CS: GPIO8 */
    {14, GPIO_DIRECTION_OUTPUT, 1, 0},
    {15, GPIO_DIRECTION_OUTPUT, 1, 0},
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, 0}      /* End of List */
};
#else
const int8_t pad_list[][4] = {0};
const int8_t gpio_list[][4] = {0};
#endif
#endif
/**
 * @brief 初始化设备管理器
 */
int Device_manager_init(void)
{
    if (g_device_manager.initialized) {
        SYSLOG_INFO("Device manager already initialized\r\n");
        return OPEN_HAL_DONE;
    }

    // 初始化全局互斥锁
    g_device_manager.global_mutex = osMutexNew(NULL);
    if (g_device_manager.global_mutex == NULL) {
        SYSLOG_ERR("Failed to create global mutex\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 初始化设备数组
    memset(g_device_manager.devices, 0, sizeof(g_device_manager.devices));
    g_device_manager.device_count = 0;

    // config sdk pin res
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    open_hal_startup(HAL_PAD, (void *)pad_list, NULL);
    open_hal_startup(HAL_GPIO, (void *)gpio_list, NULL);
#endif

    // config sdk ip res used
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    for (HalType_t type = HAL_AGPIO; type < HAL_MAX; type++) {
        open_hal_startup(type, NULL, NULL);
    }
#endif

    // app ext res used
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    for (devType_t type = DEV_TP; type < DEV_MAX; type++) {
        open_dev_startup(type, NULL);
    }
#endif

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    osThreadId_t exception = exceptionTaskInit();
    if (exception == NULL) {
        SYSLOG_ERR("Failed to initialize exception task\r\n");
        return OPEN_HAL_INVALID_PARA;
    }
#endif

    g_device_manager.initialized = true;

    // 注册openhal dev和src下的所有设备
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    // 通用设备操作函数 - 使用api_comm.c中的统一接口
    static const device_ops_t hal_device_ops = {
        .create = (int (*)(uint32_t, uint32_t, void *, uint32_t *))open_hal_create,
        .delete = (int (*)(uint32_t, uint32_t))open_hal_delete,
        .open = (int (*)(uint32_t, uint32_t, void *, size_t))open_hal_open,
        .close = (int (*)(uint32_t, uint32_t))open_hal_close,
        .ioctl = (int (*)(uint32_t, uint32_t, uint32_t, void *))open_hal_ioctl,
        .pmctl = (int (*)(uint32_t, uint32_t, void *, size_t))open_hal_pmctl,
        .read = (int (*)(uint32_t, uint32_t, void *, size_t))open_hal_read,
        .write = (int (*)(uint32_t, uint32_t, void *, size_t))open_hal_write
    };

    // DEV设备操作函数 - 使用dev_comm.h中的接口
    static const device_ops_t dev_device_ops = {
        .create = (int (*)(uint32_t, uint32_t, void *, uint32_t*))open_dev_create,
        .delete = (int (*)(uint32_t, uint32_t))open_dev_delete,
        .open = (int (*)(uint32_t, uint32_t, void *, size_t))open_dev_open,
        .close = (int (*)(uint32_t, uint32_t))open_dev_close,
        .ioctl = (int (*)(uint32_t, uint32_t, uint32_t, void *))open_dev_ioctl,
        .pmctl = (int (*)(uint32_t, uint32_t, void *, size_t))open_dev_pmctl,
        .read =(int (*)(uint32_t, uint32_t, void *, size_t))open_dev_read,
        .write =(int (*)(uint32_t, uint32_t, void *, size_t))open_dev_write
    };
    // 注册HAL设备
    // 注册HAL设备
    api_ret_t ret = OPEN_HAL_DONE;
    char pad_path[16] = {0};
    uint32_t pad_list_cnt = sizeof(pad_list) / sizeof(pad_list[0]);
    for(uint32_t i = 0; i < pad_list_cnt; i++)
    {
        int8_t pad_index = pad_list[i][0];
        if(pad_index < 0)
        {
            continue;
        }
        snprintf(pad_path, 16, "dev:/pad/%d", pad_index);
        ret = Device_reg(pad_path, HAL_PAD, pad_index, &hal_device_ops);
        if (ret != OPEN_HAL_DONE) {
            SYSLOG_INFO("Failed to register %s: %d\r\n", pad_path, ret);
        }
    }

    char gpio_path[16] = {0};
    uint32_t gpio_list_cnt = sizeof(gpio_list) / sizeof(gpio_list[0]);
    for(uint32_t i = 0; i < gpio_list_cnt; i++)
    {
        int8_t gpio_index = gpio_list[i][0];
        if(gpio_index < 0)
        {
            continue;
        }
        snprintf(gpio_path, 16, "dev:/gpio/%d", gpio_index);
        ret = Device_reg(gpio_path, HAL_GPIO, gpio_index, &hal_device_ops);
        if (ret != OPEN_HAL_DONE) {
            SYSLOG_INFO("Failed to register dev:/gpio: %d\r\n", ret);
        }
    }

    Device_reg("dev:/i2c/0", HAL_I2C, 0, &hal_device_ops);
    Device_reg("dev:/i2c/1", HAL_I2C, 1, &hal_device_ops);
#if 0
    Device_reg("dev:/spi/0", HAL_SPI, 0, &hal_device_ops);
    Device_reg("dev:/spi/1", HAL_SPI, 1, &hal_device_ops);
    Device_reg("dev:/uart/0", HAL_UART, 0, &hal_device_ops);
    Device_reg("dev:/uart/1", HAL_UART, 1, &hal_device_ops);
#endif
    // 注册DEV设备
    Device_reg("dev:/camera", DEV_CAM, 0, &dev_device_ops);
    ret = Device_reg("dev:/tp", DEV_TP, 0, &dev_device_ops);
    if (ret != OPEN_HAL_DONE) {
        SYSLOG_INFO("Failed to register dev:/tp\r\n");
        return ret;
    }
    ret = Device_reg("dev:/lcd", DEV_SCR, 0, &dev_device_ops);
        if (ret != OPEN_HAL_DONE) {
        SYSLOG_INFO("Failed to register dev:/lcd\r\n");
        return ret;
    }
#endif

    SYSLOG_INFO("Device manager initialized successfully\r\n");
    return OPEN_HAL_DONE;
}

/**
 * @brief 注册设备
 */
int Device_reg(const char *device_path, uint32_t device_type, uint32_t device_id, const device_ops_t *ops)
{
    if (!g_device_manager.initialized) {
        SYSLOG_INFO("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL || ops == NULL) {
        SYSLOG_INFO("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 检查设备是否已存在
    int existing_index = find_device_by_path(device_path);
    if (existing_index >= 0) {
        SYSLOG_ERR("Device %s already registered\r\n", device_path);
        return OPEN_HAL_USED;
    }

    // 查找空闲设备槽
    int free_index = find_free_device_slot();
    if (free_index < 0) {
        SYSLOG_ERR("No free device slots available\r\n");
        return OPEN_HAL_NONE;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = 0;
    uint32_t device_usr_id = 0;
    if (ops->create != NULL) {
        ret = ops->create(device_type, device_id, NULL, &device_usr_id);
        if (ret == OPEN_HAL_DONE) {
            // 初始化设备信息
            device_info_t *device = &g_device_manager.devices[free_index];
            strncpy(device->device_path, device_path, sizeof(device->device_path) - 1);
            device->device_path[sizeof(device->device_path) - 1] = '\0';
            device->device_type = device_type;
            device->device_id = device_usr_id;
            device->state = DEVICE_STATE_REGISTERED;
            device->power_mode = DEVICE_POWER_OFF;
            device->reference_count = 0;
            device->device_handle = (void *)ops;  // 存储操作函数指针
            device->last_access_time = osKernelGetTickCount();
            // 创建设备互斥锁
            device->mutex = osMutexNew(NULL);
            if (device->mutex == NULL) {
                SYSLOG_ERR("Failed to create device mutex for %s\r\n", device_path);
                osMutexRelease(g_device_manager.global_mutex);
                return OPEN_HAL_INVALID_PARA;
            }

            g_device_manager.device_count++;

            // SYSLOG_INFO("Device %s created successfully\r\n", device_path);
        } else {
            SYSLOG_ERR("Failed to create device %s: %d\r\n", device_path, ret);
        }
    } else {
        SYSLOG_ERR("No create operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    osMutexRelease(g_device_manager.global_mutex);

    // SYSLOG_INFO("Device %s registered successfully (type: %u, id: %u)\r\n", device_path, device_type, device_id);
    return OPEN_HAL_DONE;
}

/**
 * @brief 注销设备
 */
int Device_unreg(const char *device_path)
{
    if (!g_device_manager.initialized) {
        SYSLOG_INFO("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 检查设备状态
    if (device->state == DEVICE_STATE_OPENED || device->reference_count > 0) {
        SYSLOG_INFO("Device %s is in use (ref count: %u)\r\n", device_path, device->reference_count);
        return OPEN_HAL_USED;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = 0;
    if (ops->delete != NULL) {
        ret = ops->delete(device->device_type, device->device_id);
        if (ret == OPEN_HAL_DONE) {
            SYSLOG_INFO("Device %s deleted successfully\r\n", device_path);
        } else {
            SYSLOG_ERR("Failed delete open device %s: %d\r\n", device_path, ret);
        }
    } else {
        SYSLOG_ERR("No delete operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    // 销毁设备互斥锁
    if (device->mutex != NULL) {
        osMutexDelete(device->mutex);
        device->mutex = NULL;
    }

    // 清除设备信息
    memset(device, 0, sizeof(device_info_t));
    g_device_manager.device_count--;

    osMutexRelease(g_device_manager.global_mutex);

    SYSLOG_INFO("Device %s unregistered successfully\r\n", device_path);
    return OPEN_HAL_DONE;
}

/**
 * @brief 获取设备列表
 */
int Devices_list(device_info_t *list, uint32_t max_count)
{
    if (!g_device_manager.initialized) {
        SYSLOG_INFO("Device manager not initialized\r\n");
        return 0;
    }

    if (list == NULL || max_count == 0) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return 0;
    }

    uint32_t count = 0;

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return 0;
    }

    // 复制设备信息
    for (uint32_t i = 0; i < MAX_DEVICES && count < max_count; i++) {
        if (g_device_manager.devices[i].device_path[0] != '\0') {
            list[count++] = g_device_manager.devices[i];
        }
    }

    osMutexRelease(g_device_manager.global_mutex);

    return count;
}

/**
 * @brief 获取设备状态
 */
int device_get_stat(const char *device_path, device_info_t *stat)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL || stat == NULL) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 复制设备状态信息
    *stat = g_device_manager.devices[index];
    
    unlock_device_mutex(index);

    return OPEN_HAL_DONE;
}



/**
 * @brief 打开设备
 */
int Device_open(const char *device_path, void *cfg, size_t timeout, uint32_t *handle)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    if (handle == NULL) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    *handle = index;

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 检查设备状态
    if (device->state != DEVICE_STATE_REGISTERED) {
        SYSLOG_ERR("Device %s in invalid state: %d\r\n", device_path, device->state);
        unlock_device_mutex(index);
        return OPEN_HAL_INVALID_PARA;
    }

    // 检查D盘是否有设备配置文件，如果有则覆盖配置
    uint8_t* dev_cfg = NULL;
    uint32_t dev_cfg_size = 0;
    bool is_cfg_parsed = false;
    if (device_parser_parse(device_path, (void*)&dev_cfg, &dev_cfg_size, cfg) != 0) 
    {
        SYSLOG_WARNING("Failed to parse device config file %s\r\n", device_path);
        dev_cfg = cfg;
    }
    else 
    {
        SYSLOG_INFO("Device config file %s parsed successfully, size: %d\r\n", device_path, dev_cfg_size);
        is_cfg_parsed = true;
    }

    // 调用设备打开操作
    if (ops->open != NULL) {
        ret = ops->open(device->device_type, device->device_id, dev_cfg, timeout);
        if (ret == OPEN_HAL_DONE) {
            device->state = DEVICE_STATE_OPENED;
            device->reference_count++;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s opened successfully\r\n", device_path);
        } else {
            SYSLOG_ERR("Failed to open device %s: %d\r\n", device_path, ret);
        }
    } else {
        ret = OPEN_HAL_INVALID_PARA;
    }
    if(is_cfg_parsed)
    {
        device_parser_free(dev_cfg);
    }
    unlock_device_mutex(index);
    return ret;
}

// 内部辅助函数实现...

/**
 * @brief 通过设备路径查找设备索引
 */
static int find_device_by_path(const char *device_path)
{
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (g_device_manager.devices[i].device_path[0] != '\0' &&
            strcmp(g_device_manager.devices[i].device_path, device_path) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 查找空闲设备槽
 */
static int find_free_device_slot(void)
{
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (g_device_manager.devices[i].device_path[0] == '\0') {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 锁定设备互斥锁
 */
static int lock_device_mutex(int index)
{
    if (index < 0 || index >= MAX_DEVICES) {
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    if (device->mutex == NULL) {
        return OPEN_HAL_FREE;
    }

    if (osMutexAcquire(device->mutex, osWaitForever) != osOK) {
        return OPEN_HAL_LOCK;
    }

    return OPEN_HAL_DONE;
}

/**
 * @brief 解锁设备互斥锁
 */
static int unlock_device_mutex(int index)
{
    if (index < 0 || index >= MAX_DEVICES) {
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    if (device->mutex == NULL) {
        return OPEN_HAL_FREE;
    }

    if (osMutexRelease(device->mutex) != osOK) {
        return OPEN_HAL_LOCK;
    }

    return OPEN_HAL_DONE;
}

/**
 * @brief 关闭设备
 */
int Device_close(const char *device_path)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 检查设备状态
    if (device->state != DEVICE_STATE_OPENED) {
        SYSLOG_INFO("Device %s not opened (state: %d)\r\n", device_path, device->state);
        unlock_device_mutex(index);
        return OPEN_HAL_INVALID_PARA;
    }

    // 调用设备关闭操作
    if (ops->close != NULL) {
        ret = ops->close(device->device_type, device->device_id);
        if (ret == OPEN_HAL_DONE) {
            device->state = DEVICE_STATE_REGISTERED;
            if (device->reference_count > 0) {
                device->reference_count--;
            }
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s closed successfully\r\n", device_path);
        } else {
            SYSLOG_ERR("Failed to close device %s: %d\r\n", device_path, ret);
        }
    } else {
        SYSLOG_ERR("No close operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

/**
 * @brief 设置设备电源模式
 */
int Devices_set_power_mode(const char *device_path, device_power_mode_t power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 调用设备电源控制操作
    if (ops->pmctl != NULL) {
        open_hal_pm_t pm_cfg = {
            .mode = (PmMode_e)power_mode,
            .runtime = RUNTIME_IDLE  // 默认运行时模式
        };
        
        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
        if (ret == OPEN_HAL_DONE) {
            device->power_mode = power_mode;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s power mode set to %d\r\n", device_path, power_mode);
        } else {
            SYSLOG_ERR("Failed to set power mode for device %s: %d\r\n", device_path, ret);
        }
    } else {
        // 如果没有专门的电源控制操作，直接更新电源模式
        device->power_mode = power_mode;
        device->last_access_time = osKernelGetTickCount();
        SYSLOG_INFO("Device %s power mode set to %d (no pmctl operation)\r\n", device_path, power_mode);
        ret = OPEN_HAL_DONE;
    }

    unlock_device_mutex(index);
    return ret;
}

/**
 * @brief 获取设备电源模式
 */
int Devices_get_power_mode(const char *device_path, device_power_mode_t *power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL || power_mode == NULL) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    *power_mode = device->power_mode;
    
    unlock_device_mutex(index);

    SYSLOG_INFO("Device %s power mode: %d\r\n", device_path, *power_mode);
    return OPEN_HAL_DONE;
}

/**
 * @brief 挂起设备
 */
int Devices_suspend(const char *device_path)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = OPEN_HAL_DONE;
    int suspended_count = 0;
    device_ops_t *ops = NULL;

    if (device_path == NULL) {
        // 挂起所有设备
        for (int i = 0; i < MAX_DEVICES; i++) {
            device_info_t *device = &g_device_manager.devices[i];
            if (device->device_path[0] != '\0' && 
                (device->state == DEVICE_STATE_OPENED || device->state == DEVICE_STATE_REGISTERED)) {
                
                // 获取设备锁
                if (lock_device_mutex(i) == OPEN_HAL_DONE) {
                    device->state = DEVICE_STATE_SUSPENDED;
                    device->last_access_time = osKernelGetTickCount();
                    suspended_count++;

                    // 调用设备电源控制操作启用特定模式
                    ops = (device_ops_t *)device->device_handle;
                    if (ops->pmctl != NULL) {
                        open_hal_pm_t pm_cfg = {
                            .runtime = RUNTIME_SUSPEND
                        };
                        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
                        if (ret == OPEN_HAL_DONE) {
                            // SYSLOG_INFO("Device %s power suspend\r\n", device->device_path);
                        } else {
                            SYSLOG_ERR("Failed to suspend device %s: %d\r\n", device->device_path, ret);
                        }
                    } else {
                        SYSLOG_ERR("No pmctl operation for device %s\r\n", device->device_path);
                        ret = OPEN_HAL_INVALID_PARA;
                    }

                    unlock_device_mutex(i);
                }
            }
        }
        SYSLOG_INFO("All devices suspended (%d devices)\r\n", suspended_count);
    } else {
        // 挂起单个设备
        int index = find_device_by_path(device_path);
        if (index < 0) {
            SYSLOG_ERR("Device %s not found\r\n", device_path);
            ret = OPEN_HAL_NONE;
        } else {
            device_info_t *device = &g_device_manager.devices[index];

            // 获取设备锁
            ret = lock_device_mutex(index);
            if (ret == OPEN_HAL_DONE) {
                // 检查设备状态
                if (device->state != DEVICE_STATE_OPENED && device->state != DEVICE_STATE_REGISTERED) {
                    SYSLOG_ERR("Device %s in invalid state for suspend: %d\r\n", device_path, device->state);
                    ret = OPEN_HAL_INVALID_PARA;
                } else {
                    // 保存当前状态并设置为挂起状态
                    device->state = DEVICE_STATE_SUSPENDED;
                    device->last_access_time = osKernelGetTickCount();
                    // 调用设备电源控制操作启用特定模式
                    ops = (device_ops_t *)device->device_handle;
                    if (ops->pmctl != NULL) {
                        open_hal_pm_t pm_cfg = {
                            .runtime = RUNTIME_SUSPEND
                        };
                        SYSLOG_DEBUG("Devices_suspend single device->device_type = %d, device->device_id = %d, runtime = %d\r\n", device->device_type, device->device_id, pm_cfg.runtime);
                        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
                        if (ret == OPEN_HAL_DONE) {
                            SYSLOG_INFO("Device %s power suspend\r\n", device->device_path);
                        } else {
                            SYSLOG_ERR("Failed to suspend device %s: %d\r\n", device->device_path, ret);
                            osMutexRelease(g_device_manager.global_mutex);
                            return -1;
                        }
                    } else {
                        SYSLOG_ERR("No pmctl operation for device %s\r\n", device->device_path);
                        ret = OPEN_HAL_INVALID_PARA;
                    }
                }
                unlock_device_mutex(index);
            }
        }
    }

    Device_bus_clock_disable(device_path);

    osMutexRelease(g_device_manager.global_mutex);
    return ret;
}

/**
 * @brief 恢复设备
 */
int Devices_resume(const char *device_path)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = OPEN_HAL_DONE;
    int resumed_count = 0;
    device_ops_t *ops = NULL;

    Device_bus_clock_enable(device_path);

    if (device_path == NULL) {
        // 恢复所有设备
        for (int i = 0; i < MAX_DEVICES; i++) {
            device_info_t *device = &g_device_manager.devices[i];
            if (device->device_path[0] != '\0' && device->state == DEVICE_STATE_SUSPENDED) {
                
                // 获取设备锁
                if (lock_device_mutex(i) == OPEN_HAL_DONE) {
                    device->state = DEVICE_STATE_REGISTERED;
                    device->last_access_time = osKernelGetTickCount();
                    resumed_count++;

                    // 调用设备电源控制操作启用特定模式
                    ops = (device_ops_t *)device->device_handle;
                    if (ops->pmctl != NULL) {
                        open_hal_pm_t pm_cfg = {
                            .runtime = RUNTIME_RESUME
                        };
                        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
                        if (ret == OPEN_HAL_DONE) {
                            // SYSLOG_INFO("Device %s power resume\r\n", device->device_path);
                        } else {
                            SYSLOG_ERR("Failed to resume device %s: %d\r\n", device->device_path, ret);
                        }
                    } else {
                        SYSLOG_ERR("No pmctl operation for device %s\r\n", device->device_path);
                        ret = OPEN_HAL_INVALID_PARA;
                    }

                    unlock_device_mutex(i);
                }
            }
        }
        SYSLOG_INFO("All devices resumed (%d devices)\r\n", resumed_count);
    } else {
        // 恢复单个设备
        int index = find_device_by_path(device_path);
        if (index < 0) {
            SYSLOG_ERR("Device %s not found\r\n", device_path);
            ret = OPEN_HAL_NONE;
        } else {
            device_info_t *device = &g_device_manager.devices[index];

            // 获取设备锁
            ret = lock_device_mutex(index);
            if (ret == OPEN_HAL_DONE) {
                // 检查设备状态
                if (device->state != DEVICE_STATE_SUSPENDED) {
                    SYSLOG_ERR("Device %s not suspended (state: %d)\r\n", device_path, device->state);
                    ret = OPEN_HAL_INVALID_PARA;
                } else {
                    // 恢复设备到注册状态
                    device->state = DEVICE_STATE_REGISTERED;
                    device->last_access_time = osKernelGetTickCount();
                    SYSLOG_INFO("Device %s resumed\r\n", device_path);
                }
                unlock_device_mutex(index);
            }
        }
    }

    osMutexRelease(g_device_manager.global_mutex);
    return ret;
}

/**
 * @brief 检查是否有活跃设备
 */
bool Device_any_active(void)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return false;
    }

    bool any_active = false;

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return false;
    }

    // 检查是否有打开的或挂起的设备
    for (uint32_t i = 0; i < MAX_DEVICES; i++) {
        if (g_device_manager.devices[i].device_path[0] != '\0' &&
            (g_device_manager.devices[i].state == DEVICE_STATE_OPENED || 
             g_device_manager.devices[i].state == DEVICE_STATE_SUSPENDED)) {
            any_active = true;
            break;
        }
    }

    osMutexRelease(g_device_manager.global_mutex);

    SYSLOG_INFO("Any active devices: %s\r\n", any_active ? "true" : "false");
    return any_active;
}

/**
 * @brief 启用设备电源模式
 */
int Device_enable_power_mode(const char *device_path, device_power_mode_t power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 调用设备电源控制操作启用特定模式
    if (ops->pmctl != NULL) {
        open_hal_pm_t pm_cfg = {
            .mode = (PmMode_e)power_mode,
            .runtime = RUNTIME_IDLE
        };
        
        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
        if (ret == OPEN_HAL_DONE) {
            device->power_mode = power_mode;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s power mode %d enabled\r\n", device_path, power_mode);
        } else {
            SYSLOG_ERR("Failed to enable power mode %d for device %s: %d\r\n", power_mode, device_path, ret);
        }
    } else {
        SYSLOG_ERR("No pmctl operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

/**
 * @brief 禁用设备电源模式
 */
int Device_disable_power_mode(const char *device_path, device_power_mode_t power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 如果当前电源模式是要禁用的模式，则切换到低功耗模式
    if (device->power_mode == power_mode) {
        if (ops->pmctl != NULL) {
            open_hal_pm_t pm_cfg = {
                .mode = PM_COMMON,  // 切换到低功耗模式
                .runtime = RUNTIME_IDLE
            };
            
            ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
            if (ret == OPEN_HAL_DONE) {
                device->power_mode = DEVICE_POWER_LOW;
                device->last_access_time = osKernelGetTickCount();
                SYSLOG_INFO("Device %s power mode %d disabled, switched to low power\r\n", device_path, power_mode);
            } else {
                SYSLOG_ERR("Failed to disable power mode %d for device %s: %d\r\n", power_mode, device_path, ret);
            }
        } else {
            // 如果没有专门的电源控制操作，直接更新电源模式
            device->power_mode = DEVICE_POWER_LOW;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s power mode %d disabled (no pmctl operation)\r\n", device_path, power_mode);
            ret = OPEN_HAL_DONE;
        }
    } else {
        SYSLOG_INFO("Device %s power mode %d is not active, no need to disable\r\n", device_path, power_mode);
        ret = OPEN_HAL_DONE;
    }

    unlock_device_mutex(index);
    return ret;
}

/**
 * @brief 启用设备总线时钟
 */
int Device_bus_clock_enable(const char *device_path)
{
    SYSLOG_DEBUG("enable clock\r\n");
    CLOCK_setClockSrc(CLK_APB_MP,CLK_APB_MP_SEL_102M);
    CLOCK_clockEnable(CLK_HF102M);

    return 0;
}

/**
 * @brief 禁用设备总线时钟
 */
int Device_bus_clock_disable(const char *device_path)
{
    SYSLOG_DEBUG("disable clock\r\n");
    CLOCK_setClockSrc(FCLK_USP2,FCLK_USP2_SEL_26M);
    CLOCK_clockDisable(CLK_HF102M);

    return 0;
}

int Device_ioctl(const char *device_path, uint32_t cmd, void *para)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    if (ops->ioctl != NULL) {
        ret = ops->ioctl(device->device_type, device->device_id, cmd, para);
        if (ret == OPEN_HAL_DONE) {
            //SYSLOG_INFO("Device %s ioctl successfully\r\n", device_path);
        } else {
            //SYSLOG_ERR("Failed to ioctl device %s: %d\r\n", device_path, ret);
        }
    } else {
        SYSLOG_ERR("No ioctl operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_read(const char *device_path, void *buf, size_t count)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    if (ops->read != NULL) {
        ret = ops->read(device->device_type, device->device_id, buf, count);
        if (ret != OPEN_HAL_DONE)  {
            SYSLOG_ERR("Failed to read device %s: %d\r\n", device_path, ret);
        }
    } else {
        SYSLOG_ERR("No read operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_write(const char *device_path, void *buf, size_t count)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (device_path == NULL) {
        SYSLOG_ERR("Invalid device path\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 查找设备
    int index = find_device_by_path(device_path);
    if (index < 0) {
        SYSLOG_ERR("Device %s not found\r\n", device_path);
        return OPEN_HAL_NONE;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    if (ops->write != NULL) {
        ret = ops->write(device->device_type, device->device_id, buf, count);
        if (ret != OPEN_HAL_DONE) {
            SYSLOG_ERR("Failed to write device %s: %d\r\n", device_path, ret);
        }
    } else {
        SYSLOG_ERR("No write operation for device %s\r\n", device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_unreg_by_index(uint32_t index)
{
    if (!g_device_manager.initialized) {
        SYSLOG_INFO("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 检查设备状态
    if (device->state == DEVICE_STATE_OPENED || device->reference_count > 0) {
        SYSLOG_INFO("Device %s is in use (ref count: %u)\r\n", device->device_path, device->reference_count);
        return OPEN_HAL_USED;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = 0;
    if (ops->delete != NULL) {
        ret = ops->delete(device->device_type, device->device_id);
        if (ret == OPEN_HAL_DONE) {
            SYSLOG_INFO("Device %s deleted successfully\r\n", device->device_path);
        } else {
            SYSLOG_ERR("Failed delete open device %s: %d\r\n", device->device_path, ret);
        }
    } else {
        SYSLOG_ERR("No delete operation for device %s\r\n", device->device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    // 销毁设备互斥锁
    if (device->mutex != NULL) {
        osMutexDelete(device->mutex);
        device->mutex = NULL;
    }

    // 清除设备信息
    memset(device, 0, sizeof(device_info_t));
    g_device_manager.device_count--;

    osMutexRelease(g_device_manager.global_mutex);

    SYSLOG_INFO("Device %s unregistered successfully\r\n", device->device_path);
    return OPEN_HAL_DONE;
}

int device_get_stat_by_index(uint32_t index, device_info_t *stat)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if ((index >= MAX_DEVICES) || (stat == NULL)) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 复制设备状态信息
    *stat = g_device_manager.devices[index];
    
    unlock_device_mutex(index);

    return OPEN_HAL_DONE;
}

int Device_close_by_index(uint32_t index)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 检查设备状态
    if (device->state != DEVICE_STATE_OPENED) {
        SYSLOG_INFO("Device %s not opened (state: %d)\r\n", device->device_path, device->state);
        unlock_device_mutex(index);
        return OPEN_HAL_INVALID_PARA;
    }

    // 调用设备关闭操作
    if (ops->close != NULL) {
        ret = ops->close(device->device_type, device->device_id);
        if (ret == OPEN_HAL_DONE) {
            device->state = DEVICE_STATE_REGISTERED;
            if (device->reference_count > 0) {
                device->reference_count--;
            }
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s closed successfully\r\n", device->device_path);
        } else {
            SYSLOG_ERR("Failed to close device %s: %d\r\n", device->device_path, ret);
        }
    } else {
        SYSLOG_ERR("No close operation for device %s\r\n", device->device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Devices_set_power_mode_by_index(uint32_t index, device_power_mode_t power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 调用设备电源控制操作
    if (ops->pmctl != NULL) {
        open_hal_pm_t pm_cfg = {
            .mode = (PmMode_e)power_mode,
            .runtime = RUNTIME_IDLE  // 默认运行时模式
        };
        
        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
        if (ret == OPEN_HAL_DONE) {
            device->power_mode = power_mode;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s power mode set to %d\r\n", device->device_path, power_mode);
        } else {
            SYSLOG_ERR("Failed to set power mode for device %s: %d\r\n", device->device_path, ret);
        }
    } else {
        // 如果没有专门的电源控制操作，直接更新电源模式
        device->power_mode = power_mode;
        device->last_access_time = osKernelGetTickCount();
        SYSLOG_INFO("Device %s power mode set to %d (no pmctl operation)\r\n", device->device_path, power_mode);
        ret = OPEN_HAL_DONE;
    }

    unlock_device_mutex(index);
    return ret;
}

/**
 * @brief 获取设备电源模式
 */
int Devices_get_power_mode_by_index(uint32_t index, device_power_mode_t *power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if ((index >= MAX_DEVICES) || (power_mode == NULL)) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    *power_mode = device->power_mode;
    
    unlock_device_mutex(index);

    SYSLOG_INFO("Device %s power mode: %d\r\n", device->device_path, *power_mode);
    return OPEN_HAL_DONE;
}

int Devices_suspend_by_index(uint32_t index)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = OPEN_HAL_DONE;
    int suspended_count = 0;
    device_ops_t *ops = NULL;

    if (index >= MAX_DEVICES) {
        // 挂起所有设备
        for (int i = 0; i < MAX_DEVICES; i++) {
            device_info_t *device = &g_device_manager.devices[i];
            if (device->device_path[0] != '\0' && 
                (device->state == DEVICE_STATE_OPENED || device->state == DEVICE_STATE_REGISTERED)) {
                
                // 获取设备锁
                if (lock_device_mutex(i) == OPEN_HAL_DONE) {
                    device->state = DEVICE_STATE_SUSPENDED;
                    device->last_access_time = osKernelGetTickCount();
                    suspended_count++;

                    // 调用设备电源控制操作启用特定模式
                    ops = (device_ops_t *)device->device_handle;
                    if (ops->pmctl != NULL) {
                        open_hal_pm_t pm_cfg = {
                            .runtime = RUNTIME_SUSPEND
                        };
                        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
                        if (ret == OPEN_HAL_DONE) {
                            SYSLOG_INFO("Device %s power suspend\r\n", device->device_path);
                        } else {
                            SYSLOG_ERR("Failed to suspend device %s: %d\r\n", device->device_path, ret);
                        }
                    } else {
                        SYSLOG_ERR("No pmctl operation for device %s\r\n", device->device_path);
                        ret = OPEN_HAL_INVALID_PARA;
                    }

                    unlock_device_mutex(i);
                }
            }
        }
        SYSLOG_INFO("All devices suspended (%d devices)\r\n", suspended_count);
    } else {
        // 挂起单个设备
        device_info_t *device = &g_device_manager.devices[index];

        // 获取设备锁
        ret = lock_device_mutex(index);
        if (ret == OPEN_HAL_DONE) {
            // 检查设备状态
            if (device->state != DEVICE_STATE_OPENED && device->state != DEVICE_STATE_REGISTERED) {
                SYSLOG_ERR("Device %s in invalid state for suspend: %d\r\n", device->device_path, device->state);
                ret = OPEN_HAL_INVALID_PARA;
            } else {
                // 保存当前状态并设置为挂起状态
                device->state = DEVICE_STATE_SUSPENDED;
                device->last_access_time = osKernelGetTickCount();
                SYSLOG_INFO("Device %s suspended\r\n", device->device_path);
            }
            unlock_device_mutex(index);
        }
    }

    Device_bus_clock_disable_by_index(index);

    osMutexRelease(g_device_manager.global_mutex);
    return ret;
}

/**
 * @brief 恢复设备
 */
int Devices_resume_by_index(uint32_t index)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    // 获取全局锁
    if (osMutexAcquire(g_device_manager.global_mutex, osWaitForever) != osOK) {
        SYSLOG_ERR("Failed to acquire global mutex\r\n");
        return OPEN_HAL_LOCK;
    }

    int ret = OPEN_HAL_DONE;
    int resumed_count = 0;
    device_ops_t *ops = NULL;

    Device_bus_clock_enable_by_index(index);

    if (index >= MAX_DEVICES) {
        // 恢复所有设备
        for (int i = 0; i < MAX_DEVICES; i++) {
            device_info_t *device = &g_device_manager.devices[i];
            if (device->device_path[0] != '\0' && device->state == DEVICE_STATE_SUSPENDED) {
                
                // 获取设备锁
                if (lock_device_mutex(i) == OPEN_HAL_DONE) {
                    device->state = DEVICE_STATE_REGISTERED;
                    device->last_access_time = osKernelGetTickCount();
                    resumed_count++;

                    // 调用设备电源控制操作启用特定模式
                    ops = (device_ops_t *)device->device_handle;
                    if (ops->pmctl != NULL) {
                        open_hal_pm_t pm_cfg = {
                            .runtime = RUNTIME_RESUME
                        };
                        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
                        if (ret == OPEN_HAL_DONE) {
                            SYSLOG_INFO("Device %s power resume\r\n", device->device_path);
                        } else {
                            SYSLOG_ERR("Failed to resume device %s: %d\r\n", device->device_path, ret);
                        }
                    } else {
                        SYSLOG_ERR("No pmctl operation for device %s\r\n", device->device_path);
                        ret = OPEN_HAL_INVALID_PARA;
                    }

                    unlock_device_mutex(i);
                }
            }
        }
        SYSLOG_INFO("All devices resumed (%d devices)\r\n", resumed_count);
    } else {
        // 恢复单个设备
        device_info_t *device = &g_device_manager.devices[index];

        // 获取设备锁
        ret = lock_device_mutex(index);
        if (ret == OPEN_HAL_DONE) {
            // 检查设备状态
            if (device->state != DEVICE_STATE_SUSPENDED) {
                SYSLOG_ERR("Device %s not suspended (state: %d)\r\n", device->device_path, device->state);
                ret = OPEN_HAL_INVALID_PARA;
            } else {
                // 恢复设备到注册状态
                device->state = DEVICE_STATE_REGISTERED;
                device->last_access_time = osKernelGetTickCount();
                SYSLOG_INFO("Device %s resumed\r\n", device->device_path);
            }
            unlock_device_mutex(index);
        }
    }

    osMutexRelease(g_device_manager.global_mutex);
    return ret;
}

int Device_enable_power_mode_by_index(uint32_t index, device_power_mode_t power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 调用设备电源控制操作启用特定模式
    if (ops->pmctl != NULL) {
        open_hal_pm_t pm_cfg = {
            .mode = (PmMode_e)power_mode,
            .runtime = RUNTIME_IDLE
        };
        
        ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
        if (ret == OPEN_HAL_DONE) {
            device->power_mode = power_mode;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s power mode %d enabled\r\n", device->device_path, power_mode);
        } else {
            SYSLOG_ERR("Failed to enable power mode %d for device %s: %d\r\n", power_mode, device->device_path, ret);
        }
    } else {
        SYSLOG_ERR("No pmctl operation for device %s\r\n", device->device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_disable_power_mode_by_index(uint32_t index, device_power_mode_t power_mode)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    // 如果当前电源模式是要禁用的模式，则切换到低功耗模式
    if (device->power_mode == power_mode) {
        if (ops->pmctl != NULL) {
            open_hal_pm_t pm_cfg = {
                .mode = PM_COMMON,  // 切换到低功耗模式
                .runtime = RUNTIME_IDLE
            };
            
            ret = ops->pmctl(device->device_type, device->device_id, &pm_cfg, 1);
            if (ret == OPEN_HAL_DONE) {
                device->power_mode = DEVICE_POWER_LOW;
                device->last_access_time = osKernelGetTickCount();
                SYSLOG_INFO("Device %s power mode %d disabled, switched to low power\r\n", device->device_path, power_mode);
            } else {
                SYSLOG_ERR("Failed to disable power mode %d for device %s: %d\r\n", power_mode, device->device_path, ret);
            }
        } else {
            // 如果没有专门的电源控制操作，直接更新电源模式
            device->power_mode = DEVICE_POWER_LOW;
            device->last_access_time = osKernelGetTickCount();
            SYSLOG_INFO("Device %s power mode %d disabled (no pmctl operation)\r\n", device->device_path, power_mode);
            ret = OPEN_HAL_DONE;
        }
    } else {
        SYSLOG_INFO("Device %s power mode %d is not active, no need to disable\r\n", device->device_path, power_mode);
        ret = OPEN_HAL_DONE;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_bus_clock_enable_by_index(uint32_t index)
{
    SYSLOG_DEBUG("enable clock\r\n");
    CLOCK_setClockSrc(CLK_APB_MP,CLK_APB_MP_SEL_102M);
    CLOCK_clockEnable(CLK_HF102M);

    return 0;
}

int Device_bus_clock_disable_by_index(uint32_t index)
{
    SYSLOG_DEBUG("disable clock\r\n");
    CLOCK_setClockSrc(FCLK_USP2,FCLK_USP2_SEL_26M);
    CLOCK_clockDisable(CLK_HF102M);

    return 0;
}

int Device_ioctl_by_index(uint32_t index, uint32_t cmd, void *para)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    if (ops->ioctl != NULL) {
        ret = ops->ioctl(device->device_type, device->device_id, cmd, para);
        if (ret == OPEN_HAL_DONE) {
            // SYSLOG_INFO("Device %s ioctl successfully\r\n", device->device_path);
        } else {
            SYSLOG_ERR("Failed to ioctl device %s: %d\r\n", device->device_path, ret);
        }
    } else {
        SYSLOG_ERR("No ioctl operation for device %s\r\n", device->device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_read_by_index(uint32_t index, void *buf, size_t count)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;
    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    if (ops->read != NULL) {
        ret = ops->read(device->device_type, device->device_id, buf, count);
        if (ret != OPEN_HAL_DONE) {
            SYSLOG_ERR("Failed to read device %s: %d\r\n", device->device_path, ret);
        }
    } else {
        SYSLOG_ERR("No read operation for device %s\r\n", device->device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}

int Device_write_by_index(uint32_t index, void *buf, size_t count)
{
    if (!g_device_manager.initialized) {
        SYSLOG_ERR("Device manager not initialized\r\n");
        return OPEN_HAL_FREE;
    }

    if (index >= MAX_DEVICES) {
        SYSLOG_ERR("Invalid parameters\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    device_info_t *device = &g_device_manager.devices[index];
    device_ops_t *ops = (device_ops_t *)device->device_handle;

    // 获取设备锁
    int ret = lock_device_mutex(index);
    if (ret != OPEN_HAL_DONE) {
        return ret;
    }

    if (ops->write != NULL) {
        ret = ops->write(device->device_type, device->device_id, buf, count);
        if (ret != OPEN_HAL_DONE) {
            SYSLOG_ERR("Failed to write device %s: %d\r\n", device->device_path, ret);
        }
    } else {
        SYSLOG_ERR("No write operation for device %s\r\n", device->device_path);
        ret = OPEN_HAL_INVALID_PARA;
    }

    unlock_device_mutex(index);
    return ret;
}