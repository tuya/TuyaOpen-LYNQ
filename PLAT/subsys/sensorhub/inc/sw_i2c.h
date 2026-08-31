#ifndef _SW_I2C_H_
#define _SW_I2C_H_

#include "ec_i2c.h"

#define SDA_GPIO_INSTANCE       (1)
#define SDA_GPIO_PIN            (8)
#define SDA_PAD_INDEX           (49)
#define SDA_PAD_ALT_FUNC        (PAD_MUX_ALT0)		//AGPIO4 = GPIO24

#define SCL_GPIO_INSTANCE       (1)
#define SCL_GPIO_PIN            (9)
#define SCL_PAD_INDEX           (50)
#define SCL_PAD_ALT_FUNC        (PAD_MUX_ALT0) 		//AGPIO5 = GPIO25

// #define SDA_GPIO_INSTANCE       (2)
// #define SDA_GPIO_PIN            (2)
// #define SDA_PAD_INDEX           (40)
// #define SDA_PAD_ALT_FUNC        (PAD_MUX_ALT0)		//GPIO34

// #define SCL_GPIO_INSTANCE       (2)
// #define SCL_GPIO_PIN            (3)
// #define SCL_PAD_INDEX           (41)
// #define SCL_PAD_ALT_FUNC        (PAD_MUX_ALT0) 		//GPIO35

#define READ_CMD            1
#define WRITE_CMD           0

typedef struct sw_i2c_dev
{
    uint8_t interval;
    void (*delay)(uint32_t);
    void (*output)(uint8_t);
    void (*sda)(uint8_t);
    void (*scl)(uint8_t);
    uint8_t (*val)(void);
}sw_i2c_bus;

bus_t sw_i2c_default(void);
uint8_t sw_i2c_init(uint8_t sn);
uint8_t sw_i2c_scan(uint8_t *data, uint8_t max);
uint8_t sw_i2c_wait(uint8_t dev_id, uint8_t *data, uint16_t len, uint32_t time) ;
uint8_t sw_i2c_send(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);
uint8_t sw_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);



#endif /* _SW_I2C_H_ */
