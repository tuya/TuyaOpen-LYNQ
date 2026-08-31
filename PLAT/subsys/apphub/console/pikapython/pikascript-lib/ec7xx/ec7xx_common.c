#include "ec7xx_common.h"
#include "dataStrs.h"


#include "bsp.h"
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"


int64_t pika_platform_get_tick(void) {
    return osKernelGetTickCount();
}

void pika_platform_sleep_ms(uint32_t ms){
    pika_debug("sleep_ms %d",ms);
    osDelay(ms);
}

void pika_platform_sleep_s(uint32_t s){
    pika_debug("sleep_s %d",s);
    for (int i = 0; i < s; i++) {
        osDelay(1000);
    }
}

void ec7xx_Time_sleep_ms(PikaObj *self, int ms){
    pika_platform_sleep_ms(ms);
}
void ec7xx_Time_sleep_s(PikaObj *self, int s){
    pika_platform_sleep_s(s);
}