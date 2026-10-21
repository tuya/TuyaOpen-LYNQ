/****************************************************************************
 *
 ****************************************************************************/
#include "stdio.h"
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "ol_gpio_api.h"


#define TEST_LED_PIN          16 //NET_LIGHT
#define TEST_INTERRUPT_PIN    20 //UART1_RI


void test_gpio_led(void)
{
    int ret = 0;
    int count = 0;
    ol_gpio_config_struct config;

    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_OUTPUT;

    ret = ol_pin_config(TEST_LED_PIN,&config);
    OL_LOG_INFO("config pin[%d] ret = %d",TEST_LED_PIN, ret);

    while(count++ < 10)
    {
        ol_pin_set_level(TEST_LED_PIN, OL_GPIO_LEVEL_HIGH);
        osDelay(1000);
        ol_pin_set_level(TEST_LED_PIN, OL_GPIO_LEVEL_LOW);
        osDelay(1000);
    }
}

void test_isr_cb(void)
{
    unsigned int irqmask = 0;
    BOOL flag = FALSE;

    //Save current irq mask and diable whole port interrupts to get rid of interrupt overflow
    irqmask = ol_pin_get_irqmask(TEST_INTERRUPT_PIN);

    flag = ol_pin_get_interrupt_flag(TEST_INTERRUPT_PIN);
    OL_LOG_INFO("test gpio isr cb: irqmask = %d, flag = %d ", irqmask, flag);
    if(flag)
    {
        ol_pin_clean_interrupt_flag(TEST_INTERRUPT_PIN);
    }

    ol_pin_restore_irqmask(TEST_INTERRUPT_PIN,irqmask);
}

void test_gpio_interupt(void)
{
    ol_gpio_config_struct config;
    int ret = 0;

    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_INPUT;
    config.gpio_initoutput = 0;

    if(0 != ol_pin_config(TEST_INTERRUPT_PIN,&config))
    {
        OL_LOG_INFO("config pin[%d] fail", TEST_INTERRUPT_PIN);
        return;
    }

    ret = ol_pin_set_interrupt(TEST_INTERRUPT_PIN, OL_GPIO_INTERRUPT_RISING_EDGE, test_isr_cb);
    OL_LOG_INFO("set pin[%d] interrupt ret = %d", TEST_INTERRUPT_PIN, ret);
}

void test_aon_gpio_high(void)
{
    ol_gpio_config_struct config = { 0 };

    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_OUTPUT;

    ol_pin_config(16, &config);
    ol_pin_set_level(16, OL_GPIO_LEVEL_HIGH);//HIGH TEST

    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_OUTPUT;

    ol_pin_config(20, &config);
    ol_pin_set_level(20, OL_GPIO_LEVEL_HIGH);//HIGH TEST

    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_OUTPUT;

    ol_pin_config(25, &config);
    ol_pin_set_level(25, OL_GPIO_LEVEL_HIGH);//HIGH TEST
}

void gpio_demo(void)
{
    test_gpio_interupt();
    test_gpio_led();
}

