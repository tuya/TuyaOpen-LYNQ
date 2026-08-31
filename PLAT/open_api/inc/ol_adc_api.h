/****************************************************************************
 *
 ****************************************************************************/
#ifndef __OL_ADC_API__
#define __OL_ADC_API__


typedef enum
{
    OL_ADC_0,
    OL_ADC_1,
    OL_ADC_2,
    OL_ADC_3,
    OL_ADC_VBAT,
    OL_ADC_TEMP,
} ol_adc_id_enum;


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is used to get adc voltage value.
 * PARAMETERS
 *    adc_id[IN]:    input ADC index, ol_adc_id_enum
 * RETURN
 *    >0:        the read voltage value (unit: uV)
 *    -1:        fail
 * NOTE
 *    ADC index are defined in project PIN map table
 *    When the IO is 1.8V, the ADC value is generally between 0.05 - 1.2V
 *
 *****************************************************************************/
extern int ol_adc_get_vol(ol_adc_id_enum adc_id);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is used to get adc temperature value.
 * PARAMETERS
 *    NULL
 * RETURN
 *    >0:        the read temperature value (unit: Cel)
 *    -1:        fail
 *****************************************************************************/
extern int ol_adc_get_temperature(void);

#endif /*__OL_ADC_API__*/
