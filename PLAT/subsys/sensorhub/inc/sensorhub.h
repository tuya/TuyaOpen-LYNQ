
#ifndef _SENSOR_HUB_H_
#define _SENSOR_HUB_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "sensor_type.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#define SHUB_TASK_STATK_SIZE          (1024*10)

#define IRQ_GPIO_INSTANCE     (2)
#define IRQ_GPIO_PIN          (4)
#define IRQ_PAD_INDEX         (42)      // GPIO36 - LSP-SDO
#define IRQ_PAD_ALT_FUNC      (PAD_MUX_ALT0)


typedef enum sensor_event_bits {
    IMU_WAKEUP = ( 1 << 0 ),
    IMU_MOTION = ( 1 << 1 ),
    IMU_SLEEP = ( 1 << 2 ),
    IMU_READY = ( 1 << 3 ),
    IMU_ERROR = ( 1 << 6 )
} sensorEventBits;

extern EventGroupHandle_t g_sensorEvents;

void sensorhub_init(void);
void sensorhub_poll(void);


#ifdef __cplusplus
}
#endif
#endif
