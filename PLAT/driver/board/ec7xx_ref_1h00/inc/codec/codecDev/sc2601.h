/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: sc2601.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: sc2601 interface
*
******************************************************************************/


#ifndef _CODEC_SC2601_H
#define _CODEC_SC2601_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "codecDrv.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/



// SC2601_REGISTER NAME_REG_REGISTER ADDRESS
#define SC2601_RESET_REG00              0x00  /*reset digital,csm,clock manager etc.*/

// Clock Scheme Register definition
#define SC2601_CLK_MANAGER_REG01        0x01 // select clk src for mclk, enable clock for codec 
#define SC2601_CLK_MANAGER_REG02        0x02 // clk divider and clk multiplier 
#define SC2601_CLK_MANAGER_REG03        0x03 // adc fsmode and osr  
#define SC2601_CLK_MANAGER_REG04        0x04 // dac osr 
#define SC2601_CLK_MANAGER_REG05        0x05 // clk divier for adc and dac 
#define SC2601_CLK_MANAGER_REG06        0x06 // bclk inverter and divider 
#define SC2601_CLK_MANAGER_REG07        0x07 // tri-state, lrck divider 
#define SC2601_CLK_MANAGER_REG08        0x08 // lrck divider 

// SDP
#define SC2601_SDPIN_REG09              0x09 // dac serial digital port */
#define SC2601_SDPOUT_REG0A             0x0A // adc serial digital port */

// SYSTEM
#define SC2601_SYSTEM_REG0B             0x0B // system 
#define SC2601_SYSTEM_REG0C             0x0C // system 
#define SC2601_SYSTEM_REG0D             0x0D // system, power up/down 
#define SC2601_SYSTEM_REG0E             0x0E // system, power up/down 
#define SC2601_SYSTEM_REG0F             0x0F // system, low power 
#define SC2601_SYSTEM_REG10             0x10 // system 
#define SC2601_SYSTEM_REG11             0x11 // system 
#define SC2601_SYSTEM_REG12             0x12 // system, Enable DAC 
#define SC2601_SYSTEM_REG13             0x13 // system 
#define SC2601_SYSTEM_REG14             0x14 // system, mic gain, select DMIC, select analog pga gain 

// ADC
#define SC2601_ADC_REG15                0x15 // ADC, adc ramp rate, dmic sense 
#define SC2601_ADC_REG16                0x16 // ADC 
#define SC2601_ADC_REG17                0x17 // ADC, mic volume 
#define SC2601_ADC_REG18                0x18 // ADC, alc enable and winsize 
#define SC2601_ADC_REG19                0x19 // ADC, alc maxlevel 
#define SC2601_ADC_REG1A                0x1A // ADC, alc automute 
#define SC2601_ADC_REG1B                0x1B // ADC, alc automute, adc hpf s1 
#define SC2601_ADC_REG1C                0x1C // ADC, equalizer, hpf s2 
#define SC2601_ADC_REG1D                0x1D // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG1E                0x1E // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG1F                0x1F // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG20                0x20 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG21                0x21 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG22                0x22 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG23                0x23 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG24                0x24 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG25                0x25 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG26                0x26 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG27                0x27 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG28                0x28 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG29                0x29 // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG2A                0x2A // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG2B                0x2B // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG2C                0x2C // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG2D                0x2D // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG2E                0x2E // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG2F                0x2F // ADC, 30-bit B0 coefficient for ADCEQ
#define SC2601_ADC_REG30                0x30 // ADC, 30-bit B0 coefficient for ADCEQ

// DAC
#define SC2601_DAC_REG31                0x31 // DAC, mute 
#define SC2601_DAC_REG32                0x32 // DAC, volume 
#define SC2601_DAC_REG33                0x33 // DAC, offset 
#define SC2601_DAC_REG34                0x34 // DAC, drc enable, drc winsize 
#define SC2601_DAC_REG35                0x35 // DAC, drc maxlevel, minilevel 
#define SC2601_DAC_REG37                0x37 // DAC, ramprate 

// GPIO
#define SC2601_GPIO_REG44               0x44 // GPIO, dac2adc for test 

// CHIP
#define SC2601_CHD1_REGFD               0xFD // CHIP ID1
#define SC2601_CHD2_REGFE               0xFE // CHIP ID2
#define SC2601_CHVER_REGFF              0xFF // VERSION 
#define SC2601_CHD1_REGFD               0xFD // CHIP ID1 

#define SC2601_MAX_REGISTER             0xFF


/*----------------------------------------------------------------------------*
*                   DATA TYPE DEFINITION                                     *
*----------------------------------------------------------------------------*/

extern HalCodecFuncList_t sc2601DefaultHandle;


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

 /**
  \brief    Enables or disables PA
  \param[in] enable     true/false
  \return    NULL
  \note
 */
void sc2601EnablePA(bool enable);

/**
  \brief    Initialize SC2601 codec chip
  \param[in] codec_cfg  configuration of SC2601
  \return    -CODEC_EOK   -CODEC_INIT_ERR
  \note
 */ 
HalCodecSts_e sc2601Init(HalCodecCfg_t *codecCfg);

 /**
  \brief    Deinitialize SC2601 codec chip
  \param[in] NULL
  \return    NULL
  \note
 */
void sc2601DeInit(void);

/**
  \brief    start/stop SC2601 codec chip
  \param[in] mode codec mode
  \param[in] ctrlState start or stop decode or encode progress
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState);

/**
  \brief    Configure SC2601 codec mode and I2S interface
  \param[in] mode codec mode
  \param[in] iface I2S config
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601Config(HalCodecMode_e mode, HalCodecIface_t *iface);

/**
  \brief    Configure SC2601 DAC mute or not. Basically you can use this function to mute the output or unmute
  \param[in] enable enable(1) or disable(0)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601SetMute(HalCodecCfg_t* codecHalCfg, bool enable);

/**
  \brief    Set voice volume
  \param[in] volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601SetVolume(HalCodecCfg_t* codecHalCfg, int volume);

/**
  \brief    Get voice volume
  \param[out] *volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601GetVolume(HalCodecCfg_t* codecHalCfg, int *volume);

/**
  \brief    Configure SC2601 I2S format
  \param[in] mod:  set ADC or DAC or both
  \param[in] cfg:   ES8388 I2S format
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601ConfigFmt(HalCodecIfaceFormat_e fmt);

/**
  \brief    Configure SC2601 data sample bits
  \param[in] mode:  set ADC or DAC or both
  \param[in] bits:  bit number of per sample
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601SetBitsPerSample(HalCodecIfaceBits_e bits);

/**
  \brief    Start SC2601 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601Start(HalCodecMode_e mode);

/**
  \brief    Stop SC2601 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601Stop(HalCodecMode_e mode);

/**
  \brief    Get SC2601 DAC mute status
  \param[out] mute  get mute
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601GetVoiceMute(int *mute);

/**
  \brief    Set SC2601 mic gain and volume
  \param[in] micGain db of mic gain, varies from 0~10, default is 8
  \param[in] micVolume micVolume, varies from 0~100, default is 75
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e sc2601SetMicVolume(HalCodecCfg_t* codecHalCfg, uint8_t micGain, int micVolume);

HalCodecSts_e sc2601GetMicVolume(HalCodecCfg_t* codecHalCfg, uint8_t *micGain, int *micVolume);


/**
 * @brief Print all SC2601 registers
 *
 * @return
 *     - void
 */
/**
  \brief    Print all SC2601 registers
  \param[in] NULL
  \return    NULL
  \note
 */ 
void sc2601ReadAll();

/**
  \brief    get sc2601 default config
  \param[in] NULL
  \return    HalCodecCfg_t codecCfg
  \note
 */
HalCodecCfg_t sc2601GetDefaultCfg();

HalCodecSts_e sc2601Resume(HalCodecMode_e mode);



#ifdef __cplusplus
}
#endif

#endif /* _CODEC_SC2601_H */

