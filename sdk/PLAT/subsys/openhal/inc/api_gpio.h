/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_gpio.h
 * Description:  openhal gpio entry header file
 * History:      Rev1.0   2024-01-10
 *
 ****************************************************************************/
#ifndef  _API_GPIO_H_
#define  _API_GPIO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#define EC_GPIO_INDEX_START         (0)
#define EC_GPIO_INDEX_LIMIT         (39)

#define GPIO_PIN_0                 ((uint16_t)0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint16_t)0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint16_t)0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint16_t)0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint16_t)0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint16_t)0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint16_t)0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint16_t)0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint16_t)0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint16_t)0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint16_t)0x0400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint16_t)0x2000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint16_t)0x4000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint16_t)0x8000)  /* Pin 15 selected   */
#define GPIO_PIN_All               ((uint16_t)0xFFFF)  /* All pins selected */

typedef enum {
    OPEN_GPIO_IOCTL_DIR_OUT = 0,
    OPEN_GPIO_IOCTL_OUT_ACT ,       //output active level
    OPEN_GPIO_IOCTL_DIR_IN  ,
    OPEN_GPIO_IOCTL_ISR_CB  ,       //输入中断回调
    OPEN_GPIO_IOCTL_LOCKED  ,
} api_gpio_ioctl_t;

typedef struct
{
    uint8_t Interrupt   : 3; 
    uint8_t dummy       : 1;
    uint8_t Active      : 1;
    uint8_t Default     : 1;  //上电输出状态
    uint8_t Direction   : 1; 
    uint8_t Locked      : 1;
} gpioConfigLite_t;

typedef struct {
    int (*setDirection)(void*);
    int (*setActiveType)(void*);
    int (*setGpioCallback)(void*);
    int (*setEdgeTriggerType)(void*);
}api_gpio_inf, *api_gpio_infp;

int api_gpio_startup(void* para);
uint32_t api_gpio_default(char* str,gpioConfigLite_t *cfg);
//TODO:use gpio enum
uint32_t api_gpio_create(uint32_t pin,void *config);
api_ret_t api_gpio_delete(uint32_t usrId);
api_ret_t api_gpio_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_gpio_close(uint32_t usrId);
api_ret_t api_gpio_ioctl(uint32_t usrId,api_gpio_ioctl_t type, void *para);
api_ret_t api_gpio_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
api_ret_t api_gpio_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_gpio_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_gpio_query(uint32_t usrId);
int api_register_gpio(api_register_inf *func);
int api_test_gpio(void);
#ifdef __cplusplus
}
#endif
#endif /* _API_GPIO_H_ */