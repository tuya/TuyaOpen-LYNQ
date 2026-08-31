/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: es7111.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: es7111 interface
*
******************************************************************************/


#ifndef _CODEC_ES7111_H
#define _CODEC_ES7111_H

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

extern HalCodecFuncList_t es7111DefaultHandle;


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

 /**
  \brief    Enables or disables PA
  \param[in] enable     true/false
  \return    NULL
  \note
 */
void es7111EnablePA(bool enable);

/**
  \brief    Initialize ES7111 codec chip
  \param[in] codec_cfg  configuration of ES7111
  \return    -CODEC_EOK   -CODEC_INIT_ERR
  \note
 */ 
HalCodecSts_e es7111Init(HalCodecCfg_t *codecCfg);

 /**
  \brief    Deinitialize ES7111 codec chip
  \param[in] NULL
  \return    NULL
  \note
 */
void es7111DeInit(void);

/**
  \brief    start/stop ES7111 codec chip
  \param[in] mode codec mode
  \param[in] ctrlState start or stop decode or encode progress
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState);

/**
  \brief    Configure ES7111 codec mode and I2S interface
  \param[in] mode codec mode
  \param[in] iface I2S config
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111Config(HalCodecMode_e mode, HalCodecIface_t *iface);

/**
  \brief    Configure ES7111 DAC mute or not. Basically you can use this function to mute the output or unmute
  \param[in] enable enable(1) or disable(0)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111SetMute(HalCodecCfg_t* codecHalCfg, bool enable);

/**
  \brief    Set voice volume
  \param[in] volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111SetVolume(HalCodecCfg_t* codecHalCfg, int volume);

/**
  \brief    Get voice volume
  \param[out] *volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111GetVolume(HalCodecCfg_t* codecHalCfg, int *volume);

/**
  \brief    Configure ES7111 I2S format
  \param[in] mod:  set ADC or DAC or both
  \param[in] cfg:   ES8388 I2S format
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111ConfigFmt(HalCodecIfaceFormat_e fmt);

/**
  \brief    Configure ES7111 data sample bits
  \param[in] mode:  set ADC or DAC or both
  \param[in] bits:  bit number of per sample
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111SetBitsPerSample(HalCodecIfaceBits_e bits);

/**
  \brief    Start ES7111 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111Start(HalCodecMode_e mode);

/**
  \brief    Stop ES7111 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111Stop(HalCodecMode_e mode);

/**
  \brief    Get ES7111 DAC mute status
  \param[out] mute  get mute
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111GetVoiceMute(int *mute);

/**
  \brief    Set ES7111 mic gain and volume
  \param[in] micGain db of mic gain, varies from 0~10, default is 8
  \param[in] micVolume micVolume, varies from 0~100, default is 75
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es7111SetMicVolume(HalCodecCfg_t* codecHalCfg, uint8_t micGain, int micVolume);

/**
 * @brief Print all ES7111 registers
 *
 * @return
 *     - void
 */
/**
  \brief    Print all ES7111 registers
  \param[in] NULL
  \return    NULL
  \note
 */ 
void es7111ReadAll();

/**
  \brief    get es7111 default config
  \param[in] NULL
  \return    HalCodecCfg_t codecCfg
  \note
 */
HalCodecCfg_t es7111GetDefaultCfg();

HalCodecSts_e es7111Resume(HalCodecMode_e mode);



#ifdef __cplusplus
}
#endif

#endif /* _CODEC_ES7111_H */

