/****************************************************************************
 *
 ****************************************************************************/
#ifndef __OL_GPIO_API__
#define __OL_GPIO_API__

#include "commontypedef.h"
#include "ol_gpio_define.h"

typedef enum
{
    OL_GPIO_FUNC0,
    OL_GPIO_FUNC1,
    OL_GPIO_FUNC2,
    OL_GPIO_FUNC3,
    OL_GPIO_FUNC4,
    OL_GPIO_FUNC5,
    OL_GPIO_FUNC6,
    OL_GPIO_FUNC7
}ol_gpio_func_enum;

typedef enum
{
    OL_GPIO_INPUT = 0U,
    OL_GPIO_OUTPUT = 1U,
}ol_gpio_direction_enum;

typedef enum
{
    OL_GPIO_INTERRUPT_DISABLED     = 0U,  /**< Disable interrupt */
    OL_GPIO_INTERRUPT_LOW_LEVEL    = 1U,  /**< Low-level interrupt */
    OL_GPIO_INTERRUPT_HIGH_LEVEL   = 2U,  /**< High-level interrupt */
    OL_GPIO_INTERRUPT_FALLING_EDGE = 3U,  /**< Falling edge interrupt */
    OL_GPIO_INTERRUPT_RISING_EDGE  = 4U,  /**< Rising edge interrupt */
    OL_GPIO_INTERRUPT_BOTH_EDGE    = 5U,  /**< Falling and rising edge interrupt */
}ol_gpio_interrupt_enum;

typedef enum
{
    OL_GPIO_LEVEL_LOW,
    OL_GPIO_LEVEL_HIGH,
}ol_gpio_level_enum;

typedef enum
{
    OL_PULL_UP   = 0U,  /**< select internal pull up */
    OL_PULL_DOWN = 1U,  /**< select internal pull down */
    OL_PULL_AUTO = 2U,  /**< Pull up/down is controlled by muxed alt function */
}ol_gpio_pull_enum;

typedef struct
{
    ol_gpio_func_enum gpio_func;
    ol_gpio_direction_enum gpio_dir;
    ol_gpio_interrupt_enum gpio_interrupt;
    unsigned long gpio_initoutput;//initial value for out mode
}ol_gpio_config_struct;


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to config pin function.
 * PARAMETERS
 *      pin_num     [IN]: pin number define in mbtk_pin_num_enum
 *      gpio_config [IN]: point to input pin config params
 * RETURN VALUES
 *      0    : Interface executed successfully
 *      -1   : error
 *
 *****************************************************************************/
extern int ol_pin_config(int pin_num,ol_gpio_config_struct *gpio_config);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get pin level
 * PARAMETERS
 *      pin_num    [IN] : pin number define in mbtk_pin_num_enum
 *      gpio_level [out]: point to get pin level value
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern int ol_pin_get_level(int pin_num,ol_gpio_level_enum *gpio_level);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to set pin level.
 * PARAMETERS
 *      pin_num    [IN]: pin number define in mbtk_pin_num_enum
 *      gpio_level [IN]: input set pin level value
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern int ol_pin_set_level(int pin_num,ol_gpio_level_enum gpio_level);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to set pin interrupt.
 * PARAMETERS
 *      pin_num        [IN]: pin number define in mbtk_pin_num_enum
 *      gpio_interrupt [IN]: interrupt triggering method
 *      isr_callback   [IN]: interrupt callback function
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern int ol_pin_set_interrupt(int pin_num,ol_gpio_interrupt_enum gpio_interrupt,void *isr_callback);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get pin interrupt flag.
 * PARAMETERS
 *      pin_num [IN]: pin number define in mbtk_pin_num_enum
 * RETURN
 *      TRUE    : interrupt
 *      FALSE   : not interrupt
 *
 *****************************************************************************/
extern BOOL ol_pin_get_interrupt_flag(int pin_num);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to clean pin interrupt flag.
 * PARAMETERS
 *      pin_num [IN]: pin number define in mbtk_pin_num_enum
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern int ol_pin_clean_interrupt_flag(int pin_num);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get interrupt mask.
 * PARAMETERS
 *      pin_num [IN]: pin number define in mbtk_pin_num_enum
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern unsigned int ol_pin_get_irqmask(int pin_num);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to restore interrupt mask.
 * PARAMETERS
 *      pin_num     [IN]: pin number define in mbtk_pin_num_enum
 *      irqmask     [IN]: irq mask
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern int ol_pin_restore_irqmask(int pin_num,unsigned int irqmask);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to set pin pull mode(down or up).
 * PARAMETERS
 *      pin_num   [IN]: pin number define in mbtk_pin_num_enum
 *      gpio_pull [IN]: pin pull select define in ol_gpio_pull_enum
 * RETURN
 *      0  : success
 *      -1 : error
 *
 *****************************************************************************/
extern int ol_pin_set_pull(int pin_num,ol_gpio_pull_enum gpio_pull);


#endif/*__OL_GPIO_API__*/
