/****************************************************************************
 *
 ****************************************************************************/
#include "stdio.h"
#include "string.h"
#include "osasys.h"
#include "ol_gpio_api.h"
#include "ol_pwm_api.h"
#include "ol_log.h"


#define TEST_PWM_NUM    OL_PWM_NUM_1
#define TEST_PWM_PIN    mbtk_pin_31


void pwm_demo(void)
{
    ol_gpio_config_struct config;

    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC5;
    ol_pin_config(TEST_PWM_PIN, &config);

    ol_pwm_enable(TEST_PWM_NUM, 10000, 50); //10K HZ

    osDelay(30*1000);

    ol_pwm_updatedutycycle(TEST_PWM_NUM, 75);

    osDelay(30*1000);

    ol_pwm_disable(TEST_PWM_NUM);
}

