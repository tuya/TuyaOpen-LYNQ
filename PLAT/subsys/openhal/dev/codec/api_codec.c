/****************************************************************************
 *
 * Copy right:   2022-, Copyrigths of EigenComm Ltd.
 * File name:    codec.c
 * Description:  EC618 codec source file
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "api_codec.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef CODEC_ES8311_ENABLE
#include "es8311.h"
#endif
#ifdef CODEC_ES8374_ENABLE
#include "es8374.h"
#endif
#ifdef CODEC_ES7149_ENABLE
#include "es7149.h"
#endif
#ifdef CODEC_TM8211_ENABLE
#include "tm8211.h"
#endif
#ifdef CODEC_ES7111_ENABLE
#include "es7111.h"
#endif
#ifdef CODEC_JY6311_ENABLE
#include "jy6311.h"
#endif
#ifdef PWM_CODEC_ENABLE
#include "pwmCodec.h"
#endif
#ifdef SPI_CODEC_ENABLE
#include "spiCodec.h"
#endif


#if defined(CHIP_EC718)
#define PA_CTRL_PAD             36  //51 //mbtk change
#define PA_CTRL_GPIO            30  //26 //mbtk change
#elif defined(CHIP_EC716)
#define PA_CTRL_PAD             13
#define PA_CTRL_GPIO            1
#endif
#define PA_CTRL_PORT            ((PA_CTRL_GPIO) / 16)
#define PA_CTRL_PIN             ((PA_CTRL_GPIO) % 16)


static uint8_t gApiCodec = 0xFF;


void apiCodecPaInit(void)
{
    PadConfig_t     padConfig = {0};
    GpioPinConfig_t pinConfig = {0};

    slpManAONIOPowerOn();
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(PA_CTRL_PAD, &padConfig);

    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    GPIO_pinConfig(PA_CTRL_PORT, PA_CTRL_PIN, &pinConfig);
}

static void setPaState(bool enable)
{
    GPIO_pinWrite(PA_CTRL_PORT, 1 << PA_CTRL_PIN, enable ? (1 << PA_CTRL_PIN) : 0);
}

static bool isIcCodec(void)
{
    bool retVal = true;

    if ((gApiCodec == API_CODEC_PWM) || (gApiCodec == API_CODEC_SPI_PWM) || (gApiCodec == API_CODEC_SPI_SIGMADELTA))
    {
        retVal = false;
    }

    return retVal;
}

void codecVoltageSet(void)
{
    if ((gApiCodec == API_CODEC_SPI_PWM) || (gApiCodec == API_CODEC_SPI_SIGMADELTA))
    {
        slpManNormalIOVoltSet(IOVOLT_2_80V);
    }
    else
    {
        slpManNormalIOVoltSet(IOVOLT_1_80V);
    }
}

void apiCodecInit(ApiCodecT apiCodec)
{
    gApiCodec = apiCodec;
    apiCodecPaInit();
    codecVoltageSet();
    if (isIcCodec() == true)
    {
        codecI2sInit(apiCodec);
    }
#ifdef SPI_CODEC_ENABLE
    else if ((gApiCodec == API_CODEC_SPI_PWM) || (gApiCodec == API_CODEC_SPI_SIGMADELTA))
    {
        spiCodecInit();
    }
#endif
}

void apiCodecSetPaState(bool enable)
{
#ifdef SPEAKER_APP
    setPaState(enable); // White Cloud Speaker
#endif

    switch (gApiCodec)
    {
#ifdef CODEC_ES8311_ENABLE
        case API_CODEC_ES8311: es8311EnablePA(enable); break;
#endif
#ifdef CODEC_ES8374_ENABLE
        case API_CODEC_ES8374: es8374EnablePA(enable); break;
#endif
#ifdef CODEC_ES8388_ENABLE
        case API_CODEC_ES8388: es8388EnablePA(enable); break;
#endif
#ifdef CODEC_ES7148_ENABLE
        case API_CODEC_ES7148: es7148EnablePA(enable); break;
#endif
#ifdef CODEC_ES7149_ENABLE
        case API_CODEC_ES7149: es7149EnablePA(enable); break;
#endif
#ifdef CODEC_TM8211_ENABLE
        case API_CODEC_TM8211: tm8211EnablePA(enable); break;
#endif
#ifdef CODEC_ES7111_ENABLE
        case API_CODEC_ES7111: es7111EnablePA(enable); break;
#endif
#ifdef CODEC_JY6311_ENABLE
        case API_CODEC_JY6311: jy6311EnablePA(enable); break;
#endif
#if (defined(PWM_CODEC_ENABLE) || defined(SPI_CODEC_ENABLE))
        case API_CODEC_PWM:
        case API_CODEC_SPI_PWM:
        case API_CODEC_SPI_SIGMADELTA:
            setPaState(enable);
            break;
#endif
        default:
            break;
    }
}

void apiCodecBoot(void)
{
    if (isIcCodec() == true)
    {
        codecBoot(gApiCodec);
    }
}

int32_t sampleRateConvert(uint32_t rate, bool toEnum)
{
    if (toEnum == true)
    {
        switch (rate)
        {
            case 8000:  return SAMPLERATE_8K;
            case 16000: return SAMPLERATE_16K;
            case 22050: return SAMPLERATE_22_05K;
            case 24000: return SAMPLERATE_24K;
            case 32000: return SAMPLERATE_32K;
            case 44100: return SAMPLERATE_44_1K;
            case 48000: return SAMPLERATE_48K;
            case 96000: return SAMPLERATE_96K;
            default:    break;
        }
    }
    else
    {
        switch (rate)
        {
            case SAMPLERATE_8K:     return 8000;
            case SAMPLERATE_16K:    return 16000;
            case SAMPLERATE_22_05K: return 22050;
            case SAMPLERATE_24K:    return 24000;
            case SAMPLERATE_32K:    return 32000;
            case SAMPLERATE_44_1K:  return 44100;
            case SAMPLERATE_48K:    return 48000;
            case SAMPLERATE_96K:    return 96000;
            default:                break;
        }
    }

    SYSLOG_DEBUG("Unsupported %s sample rate: %d\r\n", (toEnum == true) ? "init" : "enum", rate);
    return -1;
}

void apiCodecStart(I2sSampleRate_e rate, I2sChannelSel_e channel)
{
    if (isIcCodec() == true)
    {
        codecStart(rate, channel);
    }
#ifdef PWM_CODEC_ENABLE
    else if (gApiCodec == API_CODEC_PWM)
    {
        pwmdaInit(sampleRateConvert(rate, false), 1000, 8);
        apiCodecSetPaState(true);
    }
#endif
#ifdef SPI_CODEC_ENABLE
    else if ((gApiCodec == API_CODEC_SPI_PWM) || (gApiCodec == API_CODEC_SPI_SIGMADELTA))
    {
        spiCodecStart();
    }
#endif
}

void apiCodecStop(void)
{
    apiCodecSetPaState(false);
    if (isIcCodec() == true)
    {
        codecStop();
    }
#ifdef PWM_CODEC_ENABLE
    else if (gApiCodec == API_CODEC_PWM)
    {
        pwmdaStop();
    }
#endif
#ifdef SPI_CODEC_ENABLE
    else if ((gApiCodec == API_CODEC_SPI_PWM) || (gApiCodec == API_CODEC_SPI_SIGMADELTA))
    {
        spiCodecStop();
    }
#endif
}

void apiCodecPlay(uint8_t *data, uint32_t length)
{
    if (isIcCodec() == true)
    {
        codecPlay(data, length);
    }
#ifdef PWM_CODEC_ENABLE
    else if (gApiCodec == API_CODEC_PWM)
    {
        pwmdaSetData((int16_t *)data, length, 8);
        pwmdaStart();
    }
#endif
#ifdef SPI_CODEC_ENABLE
    else if (gApiCodec == API_CODEC_SPI_PWM)
    {
        spiPwmCodecPlay(data, length);
    }
    else if (gApiCodec == API_CODEC_SPI_SIGMADELTA)
    {
        spiSigmadeltaCodecPlay(data, length);
    }
#endif
}

#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
void apiCodecRecord(uint8_t *data, uint32_t length)
{
    if (gApiCodec == API_CODEC_ES8311)
    {
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
        codecRecord(data, length);
#endif
    }
}
#endif