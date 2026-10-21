#ifndef __OL_PWM_API__
#define __OL_PWM_API__


typedef enum
{
    OL_PWM_NUM_0 = 0,
    OL_PWM_NUM_1,
    OL_PWM_NUM_2,
    OL_PWM_NUM_3,
    OL_PWM_NUM_4,
    OL_PWM_NUM_5
}ol_pwm_num;

typedef enum
{
    OL_PWM_DEINIT = 0,
    OL_PWM_INIT,
}ol_pwm_init_flag;


/*****************************************************************************
 *
 * DESCRIPTION
 *       This API is to enable PWM
 * PARAMETERS
 *       pwm_num   [IN]:  PWM instance number define in ol_pwm_num
 *       freq      [IN]:  PWM signal frequency in HZ
 *       dutycycle [IN]:  PWM pulse width, the valid range is 0 to 100
 * RETURN
 *       =0 : success
 *       <0 : error
 *
 *****************************************************************************/
extern int ol_pwm_enable(ol_pwm_num pwm_num,unsigned long freq,unsigned long dutycycle);


/*****************************************************************************
 *
 * DESCRIPTION 
 *       This API is to disable PWM
 * PARAMETERS 
 *       pwm_num   [IN]:  PWM instance number define in ol_pwm_num
 * RETURN
 *       =0 : success
 *       <0 : error
 *
 *****************************************************************************/
extern int ol_pwm_disable(ol_pwm_num pwm_num);


/*****************************************************************************
 *
 * DESCRIPTION 
 *       This API is to update PWM duty cycle
 * PARAMETERS 
 *       pwm_num   [IN]:  PWM instance number define in ol_pwm_num
 *       dutycycle [IN]:  PWM pulse width, the valid range is 0 to 100
 * RETURN
 *       =0 : success
 *       <0 : error
 *
 *****************************************************************************/
extern int ol_pwm_updatedutycycle(ol_pwm_num pwm_num,unsigned long dutycycle);


#endif //__OL_PWM_API__