/****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "adc.h"
#include "ol_gpio_define.h"
#include "ol_adc_api.h"


static mbtk_pin_map_struct mbtk_pin_map[] =
{
	{mbtk_pin_5,		 mbtk_gpio_20,		 mbtk_paddr_45},
	{mbtk_pin_6,		 mbtk_gpio_21,		 mbtk_paddr_46},
    {mbtk_pin_16,        mbtk_gpio_25,       mbtk_paddr_50},
	{mbtk_pin_17,        mbtk_gpio_18,       mbtk_paddr_33},//
	{mbtk_pin_18,        mbtk_gpio_19,       mbtk_paddr_34},//
    {mbtk_pin_19,        mbtk_gpio_22,       mbtk_paddr_47},//

    {mbtk_pin_20,        mbtk_gpio_27,       mbtk_paddr_52},//
    {mbtk_pin_21,        mbtk_gpio_17,       mbtk_paddr_12},//
    {mbtk_pin_22,        mbtk_gpio_1,        mbtk_paddr_16},
    {mbtk_pin_23,        mbtk_gpio_2,        mbtk_paddr_17},
    {mbtk_pin_25,        mbtk_gpio_26,       mbtk_paddr_51},
    {mbtk_pin_26,        mbtk_gpio_33,       mbtk_paddr_39},
    {mbtk_pin_28,        mbtk_gpio_10,       mbtk_paddr_25},//
    {mbtk_pin_29,        mbtk_gpio_11,       mbtk_paddr_26},//

    {mbtk_pin_30,        mbtk_gpio_29,       mbtk_paddr_35},//
    {mbtk_pin_31,        mbtk_gpio_30,       mbtk_paddr_36},//
    {mbtk_pin_32,        mbtk_gpio_31,       mbtk_paddr_37},//
    {mbtk_pin_33,        mbtk_gpio_32,       mbtk_paddr_38},//
	{mbtk_pin_38,		 mbtk_gpio_16,		 mbtk_paddr_31},//
	{mbtk_pin_39,		 mbtk_gpio_17,		 mbtk_paddr_32},//

    {mbtk_pin_49,        mbtk_gpio_15,       mbtk_paddr_30},

    {mbtk_pin_50,        mbtk_gpio_37,       mbtk_paddr_43},
    {mbtk_pin_51,        mbtk_gpio_38,       mbtk_paddr_44},
    {mbtk_pin_52,        mbtk_gpio_35,       mbtk_paddr_41},
    {mbtk_pin_53,        mbtk_gpio_34,       mbtk_paddr_40},
    {mbtk_pin_54,        mbtk_gpio_3,        mbtk_paddr_18},
    {mbtk_pin_55,        mbtk_gpio_6,        mbtk_paddr_21},
    {mbtk_pin_56,        mbtk_gpio_7,        mbtk_paddr_22},
    {mbtk_pin_57,        mbtk_gpio_18,       mbtk_paddr_13},
    {mbtk_pin_58,        mbtk_gpio_19,       mbtk_paddr_14},
    {mbtk_pin_62,        mbtk_gpio_14,       mbtk_paddr_29},
	{mbtk_pin_63,		 mbtk_gpio_13,		 mbtk_paddr_28},
	{mbtk_pin_64,		 mbtk_gpio_12,		 mbtk_paddr_27},
	{mbtk_pin_66,		 mbtk_gpio_8,		 mbtk_paddr_23},
	{mbtk_pin_67,		 mbtk_gpio_9,		 mbtk_paddr_24},

    {mbtk_pin_78,        mbtk_gpio_36,       mbtk_paddr_42},

    {mbtk_pin_80,        mbtk_gpio_4,        mbtk_paddr_19},
    {mbtk_pin_81,        mbtk_gpio_5,        mbtk_paddr_20},
	{mbtk_pin_82,		 mbtk_gpio_0,		 mbtk_paddr_15},
    {mbtk_pin_100,       mbtk_gpio_23,       mbtk_paddr_48},
    {mbtk_pin_101,       mbtk_gpio_24,       mbtk_paddr_49},
    {mbtk_pin_103,       mbtk_gpio_16,       mbtk_paddr_11},
};


int mbtk_pin_get_gpio_map(mbtk_pin_num_enum pin_num, int* gpio_num, int* paddr_num)
{
    int i = 0;

    for(i = 0; i < sizeof(mbtk_pin_map)/sizeof(mbtk_pin_map[0]); i++)
    {
        if(pin_num == mbtk_pin_map[i].mbtk_pin_num)
        {
            if(NULL != gpio_num)
                *gpio_num = mbtk_pin_map[i].mbtk_gpio_num;
            if(NULL != paddr_num)
                *paddr_num = mbtk_pin_map[i].mbtk_paddr_num;
            return 0;
        }
    }

    return -1;
}

int mbtk_get_prj_adc_channel(int adc_index)
{
    if(OL_ADC_0 == adc_index)
    {
        return ADC_CHANNEL_AIO1;
    }
    if(OL_ADC_1 == adc_index)
    {
        return ADC_CHANNEL_AIO2;
    }
    if(OL_ADC_VBAT == adc_index)
    {
        return ADC_CHANNEL_VBAT;
    }
    if(OL_ADC_TEMP == adc_index)
    {
        return ADC_CHANNEL_THERMAL;
    }
    return -1;
}

uint8_t* ol_build_module(void)
{
    return (uint8_t *)MBTK_MODEL_ID;
}

uint8_t* ol_hw_version(void)
{
    return (uint8_t *)MBTK_HW_VERSION;
}

int mbtk_get_EN_gpio_num(void)
{
    return mbtk_gpio_28;  //AGPIO8
}

int mbtk_get_EN_gpio_addr(void)
{
    return mbtk_paddr_53;  //AGPIO8
}

