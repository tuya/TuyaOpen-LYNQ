/****************************************************************************
 *
 * Copy right:   2020-, Copyrigths of EigenComm Ltd.
 * File name:    pwmda.c
 * Description:  EC618 pwmda driver source file
 * History:      Rev1.0   2020-12-17
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#if defined TYPE_EC718S || defined TYPE_EC718P || defined TYPE_EC718H || defined TYPE_EC718U
#include "ec718.h"
#elif defined TYPE_EC716S || defined TYPE_EC716E
#include "ec716.h"
#endif
#include "bsp.h"
#include "timer.h"
#include "float.h"
#include "codecDrv.h"
#include "api_codec.h"
#include "audio.h"
#include DEBUG_LOG_HEADER_FILE


#ifdef DIFFERENCE_PWM
#define PWMDA1_PWM_INSTANCE                 1
#define PWMDA1_CLOCK_ID                     FCLK_TIMER1
#define PWMDA1_PWM_INSTANCE_IRQ             PXIC0_TIMER1_IRQn
#define PWMDA1_CLOCK_SOURCE                 FCLK_TIMER1_SEL_102M
#define PWMDA1_OUT_PIN                      36
#define PWMDA1_OUT_ALT_SEL                  PAD_MUX_ALT5
#define PWMDA1_OUT_N_PIN                    48
#define PWMDA1_OUT_N_ALT_SEL                PAD_MUX_ALT3
#else
#if (defined(TYPE_EC718P) || defined(TYPE_EC718PM))
#define PWMDA1_PWM_INSTANCE                 (3)
#define PWMDA1_CLOCK_ID                     (FCLK_TIMER3)
#define PWMDA1_PWM_INSTANCE_IRQ             (PXIC0_TIMER3_IRQn)
#if defined(TYPE_EC718P)
#define PWMDA1_CLOCK_SOURCE                 (FCLK_TIMER3_SEL_26M)
#elif defined(TYPE_EC718PM)
#define PWMDA1_CLOCK_SOURCE                 (FCLK_TIMER3_SEL_102M)
#endif
#define PWMDA1_OUT_PIN                      (38)
#define PWMDA1_OUT_ALT_SEL                  (PAD_MUX_ALT5)
#elif defined TYPE_EC716E
#define PWMDA1_PWM_INSTANCE                 (0)
#define PWMDA1_CLOCK_ID                     (FCLK_TIMER0)
#define PWMDA1_PWM_INSTANCE_IRQ             (PXIC0_TIMER0_IRQn)
#define PWMDA1_CLOCK_SOURCE                 (FCLK_TIMER0_SEL_26M)
#define PWMDA1_OUT_PIN                      (28)
#define PWMDA1_OUT_ALT_SEL                  (PAD_MUX_ALT3)
#endif
#endif


TimerPwmConfig_t gPwmDaConfig;
TimerPwmConfig_t gPwmDaConfig1;

int32_t gPcmDaPlayCnt = 0;
int32_t gPcmDaDataLen = 0;
uint16_t gPcmDaPadData = 32768 ;
uint16_t gPcmDaOutData = 32768 ;
uint16_t gPcmDaTemp = 0 ;
int16_t *gPcmDaPData = NULL;
int gVolShift = 5;
extern int16_t gPcmBuf[1024*60];

int gX=0;

PLAT_FM_RAMCODE void TimerISR(void)
{
    if (TIMER_getInterruptFlags(PWMDA1_PWM_INSTANCE) & TIMER_MATCH1_INTERRUPT_FLAG)
    {
        TIMER_clearInterruptFlags(PWMDA1_PWM_INSTANCE, TIMER_MATCH1_INTERRUPT_FLAG);

        if ((gPcmDaPlayCnt == 0xFFFFFFFF) || (gPcmDaPlayCnt == 0xFFFFFFFE))
        {

        }
        else if (gPcmDaPlayCnt >= (gPcmDaDataLen / 2))
		{
            gPcmDaPlayCnt = 0xFFFFFFFF;
            if (gCodecTx == CODEC_TX_START)
            {
                gCodecTx = CODEC_TX_FINISH;
            }
		}
        else
        {
            if(gX==0)
            {
                gPcmDaTemp = (uint16_t)((gPcmDaPData[gPcmDaPlayCnt]+32768));
                gPcmDaPadData = (uint32_t)((uint32_t)(PWM_RESOLUTION_RATIO) * gPcmDaTemp) >> 16;       //MSB
                TIMER_updatePwmDutyCycle(PWMDA1_PWM_INSTANCE,gPcmDaPadData);    //msb
                gPcmDaPlayCnt++;
                gX=1;
            }
            else
            {
                if(gX>=1)
                {
                    gX=0;
                }
            }
        }
    }
}

void pwmdaSetData(int16_t *data,int len,int shift)
{
	gPcmDaPData   = data;
	gPcmDaDataLen = len;
	gVolShift     = shift;
    gPcmDaPlayCnt =0;
}

void pwmdaInit(int sampleRate,int baseFreq,int shift)
{
    // sampleRate *= 2;
    baseFreq = 32500; //130000; //65000; ///101563;//97744 //sampleRate * 16;

    PadConfig_t config1;
    PAD_getDefaultConfig(&config1);
    config1.mux = PWMDA1_OUT_ALT_SEL;
    config1.driveStrength = 0;
    PAD_setPinConfig(PWMDA1_OUT_PIN, &config1);

#ifdef DIFFERENCE_PWM
    PAD_getDefaultConfig(&config1);
    config1.mux = PWMDA1_OUT_N_ALT_SEL;
    PAD_setPinConfig(PWMDA1_OUT_N_PIN, &config1);
#endif

    CLOCK_setClockSrc(PWMDA1_CLOCK_ID, PWMDA1_CLOCK_SOURCE);
    CLOCK_setClockDiv(PWMDA1_CLOCK_ID, 1);

	TIMER_driverInit();

    gPwmDaConfig1.pwmFreq_HZ = baseFreq;
    gPwmDaConfig1.srcClock_HZ = GPR_getClockFreq(PWMDA1_CLOCK_ID); 
    gPwmDaConfig1.stopOption = TIMER_PWM_STOP_LOW;
    gPwmDaConfig1.dutyCyclePercent = PWM_RESOLUTION_RATIO/2;
#if defined(TIMER_IP_VERSION_B1)
    gPwmDaConfig1.dutyCycleUpdateMode = TIMER_PWM_DC_DEFERRED_UPDATE;
#endif

	TIMER_deInit(PWMDA1_PWM_INSTANCE);
    TIMER_setupPwm(PWMDA1_PWM_INSTANCE, &gPwmDaConfig1);

    TIMER_interruptConfig(PWMDA1_PWM_INSTANCE, TIMER_MATCH0_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_interruptConfig(PWMDA1_PWM_INSTANCE, TIMER_MATCH1_INTERRUPT, TIMER_INTERRUPT_LEVEL);
    TIMER_interruptConfig(PWMDA1_PWM_INSTANCE, TIMER_MATCH2_INTERRUPT, TIMER_INTERRUPT_DISABLE);

    // Enable TIMER IRQ
    XIC_SetVector(PWMDA1_PWM_INSTANCE_IRQ, TimerISR);
    XIC_EnableIRQ(PWMDA1_PWM_INSTANCE_IRQ);

	TIMER_start(PWMDA1_PWM_INSTANCE);
}

void pwmdaStart(void)
{
	gPcmDaPlayCnt = 0;
	// TIMER_start(PWMDA_PWM_INSTANCE);
	TIMER_start(PWMDA1_PWM_INSTANCE);
}

void pwmdaStop(void)
{
    gPcmDaPlayCnt = 0xFFFFFFFE;
    osDelay(1);

	TIMER_stop(PWMDA1_PWM_INSTANCE);
    // TIMER_deInit(PWMDA1_PWM_INSTANCE);
}