#ifndef  BSP_CUSTOM_H
#define  BSP_CUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"


/*
the following types define the scenarios of LdoFemVbat,

Scenario type1: LdoFemVbat does NOT reuse to RF FrontEnd (PA and switches) at all;
Scenario type2: LdoFemVbat is used for RF FrontEnd (PA or switches) at all, while also multiplexing other peripherals (such as cameras, vovice codecs, etc);
Scenario type3: LdoFemVbat is only used for RF FrontEnd (PA or switches) and is NOT reused for other peripherals (such as cameras, vovice codecs, etc);

The default setting is Scenario type3.

*/
enum
{
    /*For this type1, the application needs to open or close LdoFemVbat on its own.
    Please Note that LdoFemVbat will be turned off after entering sleep mode,
    after exiting sleep, the application also needs to open LdoFemVbat and re-initialize the relevant peripherals under this power supply.
    */
    LDOFEMVBAT_SCENARIO_TYPE1 = 0x0,

    /*For this type2, the application needs to re-open LdoFemVbat before each use of peripherals under this power supply,
    after use, LdoFemVbat can NOT be turned off (nor can it be turned off) because LdoFemVbat is also used for RF devices (PA or switches).
    Meanwhile, due to the sleep mode, the logic implemented by the software will close LdoFemVbat before entering into sleep, and then open after exiting from sleep.
    So, the application needs to re-open LdoFemVbat before use.
    During the period when the application is using the peripheral, if need to keep LdoFemVbat in a powered state,
    it can NOT enter any sleep mode.
    */
    LDOFEMVBAT_SCENARIO_TYPE2,

    /*For this type3, default mode, application does NOT use LdoFemVbat.*/
    LDOFEMVBAT_SCENARIO_TYPE3
};


void BSP_CustomInit(void);
uint32_t BSP_UsbGetVBUSMode(void);
uint32_t BSP_UsbGetVBUSWkupPad(void);
void SimHotSwapInit(void);


extern int mbtk_get_EN_gpio_num(void);
extern int mbtk_get_EN_gpio_addr(void);
extern void mbtkInitENGpio(void);


#ifdef __cplusplus
}
#endif

#endif /* BSP_CUSTOM_H */
