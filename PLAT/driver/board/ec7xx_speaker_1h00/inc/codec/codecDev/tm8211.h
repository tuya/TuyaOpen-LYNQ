/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: tm8211.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: tm8211 interface
*
******************************************************************************/


#ifndef _CODEC_TM8211_H
#define _CODEC_TM8211_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "codecDrv.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
*                   DATA TYPE DEFINITION                                     *
*----------------------------------------------------------------------------*/

extern HalCodecFuncList_t tm8211DefaultHandle;


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

 /**
  \brief    Enables or disables PA
  \param[in] enable     true/false
  \return    NULL
  \note
 */
void tm8211EnablePA(bool enable);

/**
  \brief    Initialize TM8211 codec chip
  \param[in] codec_cfg  configuration of TM8211
  \return    -CODEC_EOK   -CODEC_INIT_ERR
  \note
 */ 
HalCodecSts_e tm8211Init(HalCodecCfg_t *codecCfg);

 /**
  \brief    Deinitialize TM8211 codec chip
  \param[in] NULL
  \return    NULL
  \note
 */
void tm8211DeInit(void);

/**
  \brief    start/stop TM8211 codec chip
  \param[in] mode codec mode
  \param[in] ctrlState start or stop decode or encode progress
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState);

/**
  \brief    Configure TM8211 codec mode and I2S interface
  \param[in] mode codec mode
  \param[in] iface I2S config
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211Config(HalCodecMode_e mode, HalCodecIface_t *iface);

/**
  \brief    Configure TM8211 DAC mute or not. Basically you can use this function to mute the output or unmute
  \param[in] enable enable(1) or disable(0)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211SetMute(HalCodecCfg_t* codecHalCfg,  bool mute);

/**
  \brief    Set voice volume
  \param[in] volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211SetVolume(HalCodecCfg_t* codecHalCfg, int volume);

/**
  \brief    Get voice volume
  \param[out] *volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211GetVolume(HalCodecCfg_t* codecHalCfg, int *volume);

/**
  \brief    Configure TM8211 I2S format
  \param[in] mod:  set ADC or DAC or both
  \param[in] cfg:   ES8388 I2S format
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211ConfigFmt(HalCodecIfaceFormat_e fmt);

/**
  \brief    Configure TM8211 data sample bits
  \param[in] mode:  set ADC or DAC or both
  \param[in] bits:  bit number of per sample
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211SetBitsPerSample(HalCodecIfaceBits_e bits);

/**
  \brief    Start TM8211 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211Start(HalCodecMode_e mode);

/**
  \brief    Stop TM8211 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211Stop(HalCodecMode_e mode);

/**
  \brief    Get TM8211 DAC mute status
  \param[out] mute  get mute
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211GetVoiceMute(HalCodecCfg_t* codecHalCfg, uint8_t* micGain, int *volume);

/**
  \brief    Set TM8211 mic gain and volume
  \param[in] micGain db of mic gain, varies from 0~10, default is 8
  \param[in] micVolume micVolume, varies from 0~100, default is 75
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e tm8211SetMicVolume(HalCodecCfg_t* codecHalCfg, uint8_t micGain, int volume);

/**
 * @brief Print all TM8211 registers
 *
 * @return
 *     - void
 */
/**
  \brief    Print all TM8211 registers
  \param[in] NULL
  \return    NULL
  \note
 */ 
void tm8211ReadAll();

/**
  \brief    get tm8211 default config
  \param[in] NULL
  \return    HalCodecCfg_t codecCfg
  \note
 */
HalCodecCfg_t tm8211GetDefaultCfg();

HalCodecSts_e tm8211Resume(HalCodecMode_e mode);



#ifdef __cplusplus
}
#endif

#endif /* _CODEC_TM8211_H */

