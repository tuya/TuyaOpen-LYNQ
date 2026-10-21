#include <stdint.h>
#include "tkl_pwm.h"
#include "ol_pwm_api.h"
#include "ol_gpio_api.h"
#include "vlog.h"

#define PWM_NUM    5

typedef struct {
    int mbtk_pin;
    ol_pwm_num chId;
    ol_gpio_func_enum func;
    ol_gpio_func_enum orgFunc;
    bool startflag;
} pwm_map_t;

static pwm_map_t pwm_map[] = {
    { mbtk_pin_22,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_30,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_57,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC4 },                   //ok
    // { mbtk_pin_62,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_100, OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_16,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //PWM无输出
    { mbtk_pin_23,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_31,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_49,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_58,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC4 },                   //PWM无输出
    // { mbtk_pin_101, OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_32,  OL_PWM_NUM_2, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    { mbtk_pin_54,  OL_PWM_NUM_2, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    { mbtk_pin_25,  OL_PWM_NUM_3, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_33,  OL_PWM_NUM_3, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    { mbtk_pin_20,  OL_PWM_NUM_4, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_26,  OL_PWM_NUM_4, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
};

static TUYA_PWM_BASE_CFG_T pwm_cfg[PWM_NUM] = {0};

/**
 * @brief pwm init
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[in] cfg: pwm config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_init(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_BASE_CFG_T *cfg)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_init failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    if (NULL == cfg) {
        LOGE("tkl_pwm_init failed, cfg null");
        return OPRT_INVALID_PARM;
    }

    ol_gpio_config_struct gpio_cfg;
    memset(&gpio_cfg, 0, sizeof(gpio_cfg));
    gpio_cfg.gpio_func = pwm_map[ch_id].func;
    int ret = ol_pin_config(pwm_map[ch_id].mbtk_pin, &gpio_cfg);
    if (0 != ret) {
        LOGE("init pwm %d failed, set pin mux failed, ret/%d", ch_id, ret);
        return OPRT_COM_ERROR;
    }
    memcpy(&pwm_cfg[ch_id], cfg, sizeof(*cfg));
    pwm_map[ch_id].startflag = false;
    LOGI("init pwm %d success, duty/%u, freq/%u", ch_id, cfg->duty, cfg->frequency);
    return OPRT_OK;
}

/**
 * @brief pwm deinit
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_deinit(TUYA_PWM_NUM_E ch_id)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_deinit failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    ol_gpio_config_struct cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.gpio_func = pwm_map[ch_id].orgFunc;
    int ret = ol_pin_config(pwm_map[ch_id].mbtk_pin, &cfg);
    if (0 != ret) {
        LOGE("pwm %d deinit failed, restore mux pin failed, ret/%d", ch_id, ret);
        return OPRT_COM_ERROR;
    }

    memset(&pwm_cfg[ch_id], 0, sizeof(TUYA_PWM_BASE_CFG_T));
    LOGI("pwm %d deinit success", ch_id);
    return OPRT_OK;
}

/**
 * @brief pwm start
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_start(TUYA_PWM_NUM_E ch_id)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_start failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    if(pwm_map[ch_id].startflag) {
        return OPRT_OK;
    }

    unsigned long duty_cycle = pwm_cfg[ch_id].duty * 100 / (1e6 / pwm_cfg[ch_id].frequency);
    duty_cycle = duty_cycle > 100 ? 100 : duty_cycle;
    int ret = ol_pwm_enable(pwm_map[ch_id].chId, pwm_cfg[ch_id].frequency, duty_cycle);
    if (0 != ret) {
        LOGE("start pwm %d failed, enable pwm failed, ret/%d", ch_id, ret);
        return OPRT_COM_ERROR;
    }
    pwm_map[ch_id].startflag = true;
    LOGI("start pwm %d %d success", ch_id, duty_cycle);
    return OPRT_OK;
}

/**
 * @brief pwm stop
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_stop(TUYA_PWM_NUM_E ch_id)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_stop failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    if(!pwm_map[ch_id].startflag) {
        return OPRT_OK;
    }

    int ret = ol_pwm_disable(pwm_map[ch_id].chId);
    if (0 != ret) {
        LOGE("stop pwm %d failed, ret/%d", ch_id, ret);
        return OPRT_COM_ERROR;
    }
    pwm_map[ch_id].startflag = false;
    LOGI("stop pwm %d success", ch_id);
    return OPRT_OK;
}

/**
 * @brief multiple pwm channel start
 *
 * @param[in] ch_id: pwm channal id list
 * @param[in] num  : num of pwm channal to start
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_multichannel_start(TUYA_PWM_NUM_E *ch_id, UINT8_T num)
{
    LOGE("tkl_pwm_multichannel_start not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief multiple pwm channel stop
 *
 * @param[in] ch_id: pwm channal id list
 * @param[in] num  : num of pwm channal to stop
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_multichannel_stop(TUYA_PWM_NUM_E *ch_id, UINT8_T num)
{
    LOGE("tkl_pwm_multichannel_stop not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief pwm duty set
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[in] duty:  pwm duty cycle
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_duty_set(TUYA_PWM_NUM_E ch_id, UINT32_T duty)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_duty_set failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    if(!pwm_map[ch_id].startflag) {
        return OPRT_COM_ERROR;
    }

    unsigned long duty_cycle = duty * 100 / (1e6 / pwm_cfg[ch_id].frequency);
    duty_cycle = duty_cycle > 100 ? 100 : duty_cycle;
    int ret = ol_pwm_updatedutycycle(pwm_map[ch_id].chId, duty_cycle);
    if (0 != ret) {
        LOGE("set pwm %d duty(%d) failed, ret/%d", ch_id, duty, ret);
        return OPRT_COM_ERROR;
    }

    pwm_cfg[ch_id].duty = duty;
    LOGI("set pwm %d duty(%d) success", ch_id, duty);
    return OPRT_OK;
}

/**
 * @brief pwm frequency set
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[in] frequency: pwm frequency
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_frequency_set(TUYA_PWM_NUM_E ch_id, UINT32_T frequency)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_frequency_set failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    tkl_pwm_stop(ch_id);
    unsigned long duty_cycle = pwm_cfg[ch_id].duty * 100 / (1e6 / pwm_cfg[ch_id].frequency);
    pwm_cfg[ch_id].frequency = frequency;    
    pwm_cfg[ch_id].duty = (1e6 / pwm_cfg[ch_id].frequency) * duty_cycle / 100;
    LOGI("set pwm %d frequency(%d) success, duty:%d", ch_id, frequency, pwm_cfg[ch_id].duty);
    return tkl_pwm_start(ch_id);
}

/**
 * @brief pwm polarity set
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[in] polarity: pwm polarity
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_polarity_set(TUYA_PWM_NUM_E ch_id, TUYA_PWM_POLARITY_E polarity)
{
    LOGE("tkl_pwm_polarity_set not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief set pwm info
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[in] info: pwm info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_info_set(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_BASE_CFG_T *info)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_info_set failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    if (NULL == info) {
        LOGE("tkl_pwm_info_set failed, info null");
        return OPRT_INVALID_PARM;
    }

    tkl_pwm_stop(ch_id);
    memcpy(&pwm_cfg[ch_id], info, sizeof(TUYA_PWM_BASE_CFG_T));
    return tkl_pwm_start(ch_id);
}

/**
 * @brief get pwm info
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[out] info: pwm info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_info_get(TUYA_PWM_NUM_E ch_id, TUYA_PWM_BASE_CFG_T *info)
{
    if (ch_id >= PWM_NUM) {
        LOGE("tkl_pwm_info_get failed, invalid ch_id(%d)", ch_id);
        return OPRT_INVALID_PARM;
    }

    if (NULL == info) {
        LOGE("tkl_pwm_info_get failed, info null");
        return OPRT_INVALID_PARM;
    }

    memcpy(info, &pwm_cfg[ch_id], sizeof(TUYA_PWM_BASE_CFG_T));
    LOGI("get pwm %d info success", ch_id);
    return OPRT_OK;
}

/**
 * @brief pwm capture mode start
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 * @param[in] cfg: pwm capture irq config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_cap_start(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_CAP_IRQ_T *cfg)
{
    LOGE("tkl_pwm_cap_start not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief pwm capture mode stop
 *
 * @param[in] ch_id: pwm channal id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_pwm_cap_stop(TUYA_PWM_NUM_E ch_id)
{
    LOGE("tkl_pwm_cap_stop not support");
    return OPRT_NOT_SUPPORTED;
}

#if 0
// 复用pwm功能测试
static pwm_map_t debug_pwm_map[] = {
    // { mbtk_pin_22,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_30,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_57,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC4 },                   //ok
    // { mbtk_pin_62,  OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_100, OL_PWM_NUM_0, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_16,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //PWM无输出
    // { mbtk_pin_23,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_31,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_49,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_58,  OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC4 },                   //PWM无输出
    // { mbtk_pin_101, OL_PWM_NUM_1, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_32,  OL_PWM_NUM_2, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_54,  OL_PWM_NUM_2, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_5,   OL_PWM_NUM_3, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_25,  OL_PWM_NUM_3, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_33,  OL_PWM_NUM_3, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_6,   OL_PWM_NUM_4, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_20,  OL_PWM_NUM_4, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //ok
    // { mbtk_pin_26,  OL_PWM_NUM_4, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio异常
    // { mbtk_pin_19,  OL_PWM_NUM_5, OL_GPIO_FUNC5, OL_GPIO_FUNC0 },                   //复用pwm异常,复用gpio正常
};
void tkl_pwm_test(void)
{
    int ret; 
    ol_gpio_config_struct config;
    for(int i = 0;i<sizeof(debug_pwm_map)/sizeof(pwm_map_t);i++) {
        memset(&config,0x0,sizeof(config));
        config.gpio_func = debug_pwm_map[i].func;

        ret = ol_pin_config(debug_pwm_map[i].mbtk_pin, &config);
        if(ret) {
            LOGD("ol_pin_config_pwm pin:%d func:%d pwm_ch:%d failed", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].func, debug_pwm_map[i].chId);

            config.gpio_func = debug_pwm_map[i].orgFunc;
            ret = ol_pin_config(debug_pwm_map[i].mbtk_pin, &config);
            if(ret) {
                LOGD("ol_pin_config_gpio pin:%d orgfunc:%d failed", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].orgFunc);
            } else {
                LOGD("ol_pin_config_gpio pin:%d orgfunc:%d succ", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].orgFunc);
            }
            continue;
        } 
        LOGD("ol_pin_config_pwm pin:%d func:%d pwm_ch:%d succ", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].func, debug_pwm_map[i].chId);

        ret = ol_pwm_enable(debug_pwm_map[i].chId, 10000, 50); //10K HZ
        if(ret) {
            LOGD("pin:%d ol_pwm_enable %d failed", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].chId);
        } else {
            LOGD("pin:%d ol_pwm_enable %d succ", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].chId);
        }

        osDelay(1*1000);


        int ret = ol_pwm_disable(debug_pwm_map[i].chId);
        if (0 != ret) {
            LOGD("pin:%d ol_pwm_disable %d failed", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].chId);
        }

        config.gpio_func = debug_pwm_map[i].orgFunc;
        ret = ol_pin_config(debug_pwm_map[i].mbtk_pin, &config);
        if(ret) {
            LOGD("ol_pin_config_gpio pin:%d orgfunc:%d failed", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].orgFunc);
        } else {
            LOGD("ol_pin_config_gpio pin:%d orgfunc:%d succ", debug_pwm_map[i].mbtk_pin, debug_pwm_map[i].orgFunc);
        }
    }
}
#endif

#if 1
void tkl_pwm_test(void)
{
    TUYA_PWM_BASE_CFG_T cfg = {
        .frequency = 1000,
        .duty = 100,
    };

    for(int i = 0;i<PWM_NUM;i++) {
        cfg.duty = 100 * (i + 1);
        tkl_pwm_init(i, &cfg);
    }

    for(int i = 0;i<PWM_NUM;i++) {
        tkl_pwm_start(i);
    }
    osDelay(2*1000);

    for(int i = 0;i<PWM_NUM;i++) {
        tkl_pwm_stop(i);
    }
    osDelay(2*1000);

    for(int i = 0;i<PWM_NUM;i++) {
        tkl_pwm_start(i);
    }
    osDelay(2*1000);
    
    for(int i = 0;i<PWM_NUM;i++) {
        tkl_pwm_frequency_set(i, 2000);
    }
    osDelay(2*1000);

    for(int i = 0;i<PWM_NUM;i++) {
        tkl_pwm_duty_set(i, 400);
    }
    osDelay(2*1000);

    while(1) {
        osDelay(30*1000);
    }
}

#endif
