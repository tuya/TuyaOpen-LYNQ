#include "tuya_iot_config.h"
#include <stdbool.h>

#if defined(ENABLE_BUILD_IN_GPS) && ENABLE_BUILD_IN_GPS == 1
#include "ol_gpio_api.h"
#define GNSS_POWER_ON_PIN 100   //pin100控制gps 电源，拉高开启电源，直接调用原厂接口，不对用户开放为普通GPIO
void tkl_gnss_dev_powerctl_hook(bool enable)
{
    ol_gpio_config_struct config = {0};
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_OUTPUT;
    ol_pin_config(GNSS_POWER_ON_PIN, &config);
    ol_pin_set_level(GNSS_POWER_ON_PIN, enable ? OL_GPIO_LEVEL_HIGH : OL_GPIO_LEVEL_LOW);
}
#endif