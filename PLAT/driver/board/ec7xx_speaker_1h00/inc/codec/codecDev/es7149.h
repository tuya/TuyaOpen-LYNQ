/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: es7149.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: es7149 interface
*
******************************************************************************/


#ifndef _CODEC_ES7149_H
#define _CODEC_ES7149_H

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

extern HalCodecFuncList_t es7149DefaultHandle;


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

 /**
  \brief    Enables or disables PA
  \param[in] enable     true/false
  \return    NULL
  \note
 */
void es7149EnablePA(bool enable);

/**
  \brief    Initialize ES7149 codec chip
  \param[in] codec_cfg  configuration of ES7149
  \return    -CODEC_EOK   -CODEC_INIT_ERR
  \note
 */ 
HalCodecSts_e es7149Init(HalCodecCfg_t *codecCfg);

 /**
  \brief    Deinitialize ES7149 codec chip
  \param[in] NULL
  \return    NULL
  \note
 */
void es7149DeInit(void);

/**
  \brief    start/stop ES7149 codec chip
  \param[in] mode codec mode
  \param[in] ctrlState start or stop decode or encode progress
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState);

/**
  \brief    Configure ES7149 codec mode and I2S interface
  \param[in] mode codec mode
  \param[in] iface I2S config
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149Config(HalCodecMode_e mode, HalCodecIface_t *iface);

/**
  \brief    Configure ES7149 DAC mute or not. Basically you can use this function to mute the output or unmute
  \param[in] enable enable(1) or disable(0)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149SetMute(HalCodecCfg_t* codecHalCfg,  bool mute);

/**
  \brief    Set voice volume
  \param[in] volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149SetVolume(HalCodecCfg_t* codecHalCfg, int volume);

/**
  \brief    Get voice volume
  \param[out] *volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149GetVolume(HalCodecCfg_t* codecHalCfg, int *volume);

/**
  \brief    Configure ES7149 I2S format
  \param[in] mod:  set ADC or DAC or both
  \param[in] cfg:   ES8388 I2S format
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149ConfigFmt(HalCodecIfaceFormat_e fmt);

/**
  \brief    Configure ES7149 data sample bits
  \param[in] mode:  set ADC or DAC or both
  \param[in] bits:  bit number of per sample
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149SetBitsPerSample(HalCodecIfaceBits_e bits);

/**
  \brief    Start ES7149 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149Start(HalCodecMode_e mode);

/**
  \brief    Stop ES7149 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149Stop(HalCodecMode_e mode);

/**
  \brief    Get ES7149 DAC mute status
  \param[out] mute  get mute
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149GetVoiceMute(int *mute);

/**
  \brief    Set ES7149 mic gain and volume
  \param[in] micGain db of mic gain, varies from 0~10, default is 8
  \param[in] micVolume micVolume, varies from 0~100, default is 75
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7149SetMicVolume(HalCodecCfg_t* codecHalCfg, uint8_t micGain, int volume);

/**
 * @brief Print all ES7149 registers
 *
 * @return
 *     - void
 */
/**
  \brief    Print all ES7149 registers
  \param[in] NULL
  \return    NULL
  \note
 */ 
void es7149ReadAll();

/**
  \brief    get es7149 default config
  \param[in] NULL
  \return    HalCodecCfg_t codecCfg
  \note
 */
HalCodecCfg_t es7149GetDefaultCfg();

HalCodecSts_e es7149Resume(HalCodecMode_e mode);



#ifdef __cplusplus
}
#endif

#endif /* _CODEC_ES7149_H */

