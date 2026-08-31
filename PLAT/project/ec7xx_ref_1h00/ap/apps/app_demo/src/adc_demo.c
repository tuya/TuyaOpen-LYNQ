/****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "ol_adc_api.h"

void adc_demo(void)
{
    int index = 0;

    while(index++ < 5)
    {
        OL_LOG_INFO("--------------ADC test-----------------");
        OL_LOG_INFO("read Temp: %d (Cel)", ol_adc_get_temperature());
        OL_LOG_INFO("read ADC0: %d (uV)", ol_adc_get_vol(OL_ADC_0));
        OL_LOG_INFO("read ADC1: %d (uV)", ol_adc_get_vol(OL_ADC_1));
        OL_LOG_INFO("read Vbat: %d (uV)", ol_adc_get_vol(OL_ADC_VBAT));
        osDelay(3000);
    }
}

