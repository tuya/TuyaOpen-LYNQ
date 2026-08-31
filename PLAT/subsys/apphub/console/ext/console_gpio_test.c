/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "time.h"
#include "storage.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "bsp.h"
#include "packet.h"

#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "ff.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#endif
#if defined(FEATURE_SUBSYS_AUDIO_ENABLE) || defined(FEATURE_SUBSYS_MEDIA_ENABLE)
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#else
#include "media.h"
#endif
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_PCM_ENABLE
#include "pcm.h"
#endif
#endif

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
int gpio_create(int gpio_num)
{
    rt_kprintf("create gpio :%d\r\n", gpio_num);
    uint32_t test_gpio_id = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;

    test_gpio_id = api_gpio_create(gpio_num, &pinConfig);
    EC_API_CHECK(api_gpio_open(test_gpio_id, NULL, 0));
    uint8_t level = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_DIR_OUT, &level));
    uint8_t active = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_OUT_ACT, &active));
    open_hal_pm_t cfg = {
        .mode = PM_LOWPOW,
        .runtime = RUNTIME_SUSPEND};
    EC_API_CHECK(api_gpio_pmctl(test_gpio_id, &cfg, sizeof(cfg)));
    return 0;
}

int gpio_delete(int gpio_num)
{
    rt_kprintf("delete gpio :%d\r\n", gpio_num);
    uint32_t test_gpio_id = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;

    test_gpio_id = api_gpio_create(gpio_num, &pinConfig);
    EC_API_CHECK(api_gpio_open(test_gpio_id, NULL, 0));
    uint8_t level = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_DIR_OUT, &level));
    uint8_t active = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_OUT_ACT, &active));
    EC_API_CHECK(api_gpio_close(test_gpio_id));
    EC_API_CHECK(api_gpio_delete(test_gpio_id));
    return 0;
}

int gpio_write(int gpio_num, int level)
{
    rt_kprintf("write gpio[%d]: %d\r\n", gpio_num, level);
    uint32_t test_gpio_id = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;

    test_gpio_id = api_gpio_create(gpio_num, &pinConfig);
    EC_API_CHECK(api_gpio_open(test_gpio_id, NULL, 0));
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_DIR_OUT, &level));
    uint8_t active = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_OUT_ACT, &active));
    EC_API_CHECK(api_gpio_write(test_gpio_id, &level, 1));
    return 0;
}

int gpio_read(int gpio_num)
{
    uint32_t test_gpio_id = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_INPUT;

    test_gpio_id = api_gpio_create(gpio_num, &pinConfig);
    EC_API_CHECK(api_gpio_open(test_gpio_id, NULL, 0));
    uint8_t level = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_DIR_OUT, &level));
    uint8_t active = 0;
    EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_OUT_ACT, &active));
    EC_API_CHECK(api_gpio_read(test_gpio_id, &level, 1));
    rt_kprintf("read gpio[%d] res: %d\r\n", gpio_num, level);
    return 0;
}

void test_gpio_isr_handle(void)
{
    uint16_t pin = 0;
    uint32_t instance = 0;
    for (int i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
    {
        instance = i / 16;
        pin = i % 16;
        if (GPIO_getInterruptFlags(instance) & (1 << pin))
        {
            uint16_t portIrqMask = GPIO_saveAndSetIrqMask(instance);
            // if(sGpioIsrFunc[i] != NULL){
            //     sGpioIsrFunc[i](i);
            // }
            GPIO_clearInterruptFlags(instance, 1 << pin);
            GPIO_restoreIrqMask(instance, portIrqMask);
        }
        else
        {
        }
    }
}

int gpio_interrupt(int gpio_num)
{
    rt_kprintf("set gpio[%d] interrupt\r\n", gpio_num);
    uint32_t tp_irq_PinId = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
    pinConfig.misc.interruptConfig = GPIO_INTERRUPT_RISING_EDGE;
    tp_irq_PinId = api_gpio_create(gpio_num, &pinConfig);
    api_gpio_open(tp_irq_PinId, NULL, 0);
    api_gpio_ioctl(tp_irq_PinId, OPEN_GPIO_IOCTL_ISR_CB, (void *)test_gpio_isr_handle);
    return 0;
}

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
int cmd_gpio(int argc, char **argv)
{
    // rt_kprintf("%d args: %s %s\r\n", argc, argv[1], skip_atoi(&argv[2]));
    // for (int i = 0; i < argc; i++)
    // {
    //     rt_kprintf("arg[%d]: %s\r\n", i, argv[i]);
    //     // rt_kprintf("arg[%d]: %s\r\n", i, skip_atoi(&argv[i]));
    // }

    if (argc > 4)
    {
        rt_kprintf("argc > 4");
        // rt_kprintf("%d:%s %d %d\r\n",argc,argv[1],skip_atoi(&argv[2]),skip_atoi(&argv[3]));
        return 0;
    }
    else
    {
        char *sub_cmd = argv[1];
        // gpio create 28
        if (strcmp(sub_cmd, "create") == 0)
        {
            gpio_create(skip_atoi(&argv[2]));
            
        }
        // gpio create_all
        else if (strcmp(sub_cmd, "create_all") == 0)
        {
            // uint32_t test_gpio_id = 0;
            // GpioPinConfig_t pinConfig = {0};
            // pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
            // pinConfig.misc.initOutput = 0;
            // for (int i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
            // {
            //     rt_kprintf("create gpio[%d]\r\n", i);
            //     test_gpio_id = api_gpio_create(i, &pinConfig);
            //     EC_API_CHECK(api_gpio_open(test_gpio_id, NULL, 0));
            //     uint8_t level = 0;
            //     EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_DIR_OUT, &level));
            //     uint8_t active = 0;
            //     EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_OUT_ACT, &active));
            //     open_hal_pm_t cfg = {
            //         .mode = PM_LOWPOW,
            //         .runtime = RUNTIME_SUSPEND};
            //     EC_API_CHECK(api_gpio_pmctl(test_gpio_id, &cfg, sizeof(cfg)));
            // }
            for (int i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
            {
                gpio_create(i);
            }
            
        }
        // gpio delete 28
        else if (strcmp(sub_cmd, "delete") == 0)
        {
            gpio_delete(skip_atoi(&argv[2]));
            
        }
        // gpio delete_all
        else if (strcmp(sub_cmd, "delete_all") == 0)
        {
            // uint32_t test_gpio_id = 0;
            // GpioPinConfig_t pinConfig = {0};
            // pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
            // pinConfig.misc.initOutput = 0;
            // for (int i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
            // {
            //     rt_kprintf("delete gpio[%d]\r\n", i);
            //     test_gpio_id = api_gpio_create(i, &pinConfig);
            //     EC_API_CHECK(api_gpio_open(test_gpio_id, NULL, 0));
            //     uint8_t level = 0;
            //     EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_DIR_OUT, &level));
            //     uint8_t active = 0;
            //     EC_API_CHECK(api_gpio_ioctl(test_gpio_id, OPEN_GPIO_IOCTL_OUT_ACT, &active));
            //     EC_API_CHECK(api_gpio_close(test_gpio_id));
            //     EC_API_CHECK(api_gpio_delete(test_gpio_id));
            // }
            for (int i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
            {
                gpio_delete(i);
            }
            
            // gpio_delete(skip_atoi(&argv[2]));
        }
        // gpio write 28 0
        else if (strcmp(sub_cmd, "write") == 0)
        {
            gpio_write(skip_atoi(&argv[2]), skip_atoi(&argv[3]));
            
        }
        // gpio read 28
        else if (strcmp(sub_cmd, "read") == 0)
        {
            gpio_read(skip_atoi(&argv[2]));
            
        }
        // gpio write_all
        else if (strcmp(sub_cmd, "write_all") == 0)
        {
            for (int i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
            {
                gpio_write(i, skip_atoi(&argv[2]));
            }
            
        }
        // gpio interrupt 28
        else if (strcmp(sub_cmd, "interrupt") == 0)
        {
            gpio_interrupt(skip_atoi(&argv[2]));
            
        }
        else if (strcmp(sub_cmd, "test") == 0)
        {
            api_test_gpio();
            
        }
        else
        {
            goto _usage;
        }
        return 0;
    }
_usage:
    rt_kprintf("Usage: gpio [options]\r\n");
    rt_kprintf("[options]:\r\n");
    rt_kprintf("    %-12s - create gpio\r\n", "create");
    rt_kprintf("    %-12s - delete gpio\r\n", "delete");
    rt_kprintf("    %-12s - create all gpio\r\n", "create_all");
    rt_kprintf("    %-12s - delete all gpio\r\n", "delete_all");
    rt_kprintf("    %-12s - write gpio\r\n", "write");
    rt_kprintf("    %-12s - read gpio\r\n", "read");
    rt_kprintf("    %-12s - write all gpio\r\n", "write_all");
    rt_kprintf("    %-12s - gpio interrupt\r\n", "interrupt");
    rt_kprintf("    %-12s - test gpio\r\n", "test");
    
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_gpio, gpio, gpio test);

#endif
#endif
#endif