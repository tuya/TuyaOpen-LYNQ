#include "tuya_iot_config.h"

#if defined(ENABLE_BUILD_IN_GPS) && ENABLE_BUILD_IN_GPS == 1
#include "tkl_gpio.h"
#define GNSS_POWER_ON_PIN 100   //pin100控制gps 电源，拉高开启电源
void tkl_gnss_dev_powerctl_hook(bool enable)
{
    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode = TUYA_GPIO_PULLDOWN,
        .direct = TUYA_GPIO_OUTPUT,
        .level = enable ? TUYA_GPIO_LEVEL_HIGH : TUYA_GPIO_LEVEL_LOW,
    };
    tkl_gpio_init(GNSS_POWER_ON_PIN, &cfg);
}
#endif