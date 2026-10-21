
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "devicemanager.h"
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#endif
#include "cmsis_os2.h" //mbtk change

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

void deviceManagerInit()
{
    //config sdk pin res
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    osThreadId_t exception = exceptionTaskInit();
#endif
    //config sdk ip res used

    //app ext res used 

}

// void extDevReg(EextDev_t* pdev)
// {

// }

// void extDevDereg(EextDev_t* pdev)
// {

// }

// void extDevStat(EextDev_t* pdev)
// {

// }

// void ipDevReg(IpDev_t* pip)
// {

// }

// void ipDevDereg(IpDev_t* pip)
// {

// }

// void ipDevStat(IpDev_t* pip)
// {

// }