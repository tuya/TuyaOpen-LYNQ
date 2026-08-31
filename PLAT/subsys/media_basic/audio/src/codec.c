/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    codec.c
 * Description:  EC718 codec source file
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "bsp_custom.h"
#include "mw_nvm_audio.h"
#include "codecDrv.h"
#ifdef CODEC_ES8311_ENABLE
#include "es8311.h"
#endif
#ifdef CODEC_TM8211_ENABLE
#include "tm8211.h"
#endif
#ifdef CODEC_ES7111_ENABLE
#include "es7111.h"
#endif
#ifdef CODEC_ES7149_ENABLE
#include "es7149.h"
#endif
#ifdef CODEC_ES8374_ENABLE
#include "es8374.h"
#endif
#ifdef PWM_CODEC_ENABLE
#include "pwmCodec.h"
#endif
#include DEBUG_LOG_HEADER_FILE
#include "audio.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif



volatile uint8_t           gCodecTx = CODEC_TX_IDLE;
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
volatile uint8_t           gCodecRx = CODEC_RX_IDLE;
#endif
static I2sParamCtrl_t      gI2sParamCtrl;
static HalCodecState_e     gHalCodecState = CODEC_NONE;
static HalCodecCfg_t       gHalCodecCfg;


static void i2sTxCb(uint32_t event, uint32_t arg)
{
    if (gCodecTx == CODEC_TX_START)
    {
        gCodecTx = CODEC_TX_FINISH;
    }
}

static void i2sRxCb(uint32_t event, uint32_t arg)
{
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
    if (gCodecRx == CODEC_RX_START)
    {
        gCodecRx = CODEC_RX_FINISH;
    }
#endif
}

void codecI2sInit(HalCodecType_e codecType)
{
    HalCodecFuncList_t *halCodecFuncList = NULL;

    if (gHalCodecState == CODEC_NONE)
    {
        gHalCodecState = CODEC_INIT;
        switch (codecType)
        {
#ifdef CODEC_ES8311_ENABLE
            case ES8311: halCodecFuncList = &es8311DefaultHandle; break;
#endif
#ifdef CODEC_ES8374_ENABLE
            case ES8374: halCodecFuncList = &es8374DefaultHandle; break;
#endif
#ifdef CODEC_ES8388_ENABLE
            case ES8388: halCodecFuncList = &es8388DefaultHandle; break;
#endif
#ifdef CODEC_ES7148_ENABLE
            case ES7148: halCodecFuncList = &es7148DefaultHandle; break;
#endif
#ifdef CODEC_ES7149_ENABLE
            case ES7149: halCodecFuncList = &es7149DefaultHandle; break;
#endif
#ifdef CODEC_TM8211_ENABLE
            case TM8211: halCodecFuncList = &tm8211DefaultHandle; break;
#endif
            default:
                SYSLOG_ERR("No codec defined.\r\n");
                break;
        }

        gHalCodecCfg = halCodecGetDefaultCfg(halCodecFuncList);
        halI2sInit(i2sTxCb, i2sRxCb);
        halI2sSetDmaDescriptorNum(I2S_TX, 1);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
        halI2sSetDmaDescriptorNum(I2S_RX, 1);
#endif

        gI2sParamCtrl.frameSize  = gHalCodecCfg.codecIface.bits;    // i2s and codec use same bits
        gI2sParamCtrl.mode       = gHalCodecCfg.codecIface.fmt;     // i2s and codec use same format
        gI2sParamCtrl.role       = gHalCodecCfg.codecIface.mode;    // i2s plays master, codec is slave
        gI2sParamCtrl.channelSel = gHalCodecCfg.codecIface.channel; // i2s and codec use MONO channel
        gI2sParamCtrl.sampleRate = SAMPLERATE_16K;
        halI2sConfig(gI2sParamCtrl);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
        halI2sStartStop(STOP_ALL);
#else
        halI2sStartStop(STOP_SEND);
#endif

        gHalCodecCfg.codecIface.samples = gI2sParamCtrl.sampleRate;
        gHalCodecCfg.hasPA              = true;
        gHalCodecCfg.codecDeviceType    = AUDIO_CFG_DEVICE_HAND_FREE;
        halCodecInit(&gHalCodecCfg, halCodecFuncList, false);
        halCodecSetVolume(halCodecFuncList, &gHalCodecCfg, 80, false);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
        halCodecSetMicVolume(halCodecFuncList, &gHalCodecCfg, 8, 191, false);
#endif
        halCodecIfaceCfg(halCodecFuncList, gHalCodecCfg.codecMode, &gHalCodecCfg.codecIface, false);
        halCodecCtrlState(halCodecFuncList, gHalCodecCfg.codecMode, CODEC_CTRL_START, false);
    }
}

void codecStart(I2sSampleRate_e sampleRate, I2sChannelSel_e channel)
{
    gHalCodecState = CODEC_START;
    GPR_mclkEnable(MCLK0);
    halI2sSetSampleRate(I2S_MASTER_MODE, sampleRate);
    halI2sSetChannel(channel);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
    halI2sStartStop(START_ALL);
#else
    halI2sStartStop(START_SEND);
#endif
}

void codecPlay(uint8_t *data, uint32_t length)
{	
    if (gHalCodecState == CODEC_START)
    {
        halI2sTransfer(PLAY, data, length);
    }
}

void codecBoot(HalCodecType_e codecType)
{
    if ((codecType == ES8311) || (codecType == ES7149))
    {
        uint8_t data[128] = {0};

        codecStart(SAMPLERATE_96K, MONO);
        codecPlay(data, sizeof(data));
    }
}

void codecStop(void)
{
    if (gHalCodecState == CODEC_START)
    {
        gHalCodecState = CODEC_INIT;
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
        halI2sStartStop(STOP_ALL);
#else
        halI2sStartStop(STOP_SEND);
#endif
    }
}

#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
void codecRecord(uint8_t *data, uint32_t length)
{
    if (gHalCodecState == CODEC_START)
    {
        halI2sTransfer(RECORD, data, (length < AUDIO_RX_TRANSFER_SIZE) ? length : AUDIO_RX_TRANSFER_SIZE);
    }
}
#endif
