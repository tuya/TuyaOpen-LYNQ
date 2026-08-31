/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of EigenComm Ltd.
 * File name:    codecCtl.c
 * Description:  codec control 
 * History:      Rev1.0   2024-7-25
 *
 ****************************************************************************/

#include <stdint.h>
#include "bsp.h"


#define CODEC_MIC_CTL_PAD           53
#define CODEC_MIC_CTL_GPIO          28


void codecCtlInit(void)
{
    PadConfig_t     padConfig = {0};
    GpioPinConfig_t pinConfig = {0};

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(CODEC_MIC_CTL_PAD, &padConfig);

    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 1;
    GPIO_pinConfig(CODEC_MIC_CTL_GPIO / 16, CODEC_MIC_CTL_GPIO % 16, &pinConfig);
}
