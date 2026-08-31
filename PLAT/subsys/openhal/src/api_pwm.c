/****************************************************************************
 *
 * Copy right:   2025-, Copyrigths of EigenComm Ltd.
 * File name:    api_pwm.c
 * Description:  ec7xx openhal pwm entry source file
 * History:      Rev1.0   2025-09-15
 *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE

#include "clock.h"
#include "bsp.h"
#include "mem_map.h"
#include "clock.h"
#include "slpman.h"
#include "cms_api.h"
#include "timer.h"
#include "gpio.h"
#include "devicemanager.h"

#include "api_comm.h"
#include "api_pwm.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif

extern ClockId_e gTimerClocks[];  // 需要修改timer.c中该数组的static定义

static uint32_t sPwmRuntimeList[EC_PWM_INDEX_LIMIT] = {0xFF};

static uint32_t sPwmUsrIdList[EC_PWM_INDEX_LIMIT] = {0};
static uint16_t sPwmUsrIdSeed[EC_PWM_INDEX_LIMIT] = {0};
static pwm_config_t sPwmConfigList[EC_PWM_INDEX_LIMIT] = {{0}};
// static IsrFunc pwm_isr_cb[6] = {NULL};

const int8_t pwm_PinTable[][3] = {
    {13, PAD_MUX_ALT5, 0},  // 13   PWM0
    {14, PAD_MUX_ALT5, 1},  // 14   PWM1
    {16, PAD_MUX_ALT5, 0},  // 16   PWM0
    {17, PAD_MUX_ALT5, 1},  // 17   PWM1
    {18, PAD_MUX_ALT5, 2},  // 18   PWM2
    {29, PAD_MUX_ALT5, 0},  // 29   PWM0
    {30, PAD_MUX_ALT5, 1},  // 30   PWM1
    {35, PAD_MUX_ALT5, 0},  // 35   PWM0
    {36, PAD_MUX_ALT5, 1},  // 36   PWM1
    {37, PAD_MUX_ALT5, 2},  // 37   PWM2
    {38, PAD_MUX_ALT5, 3},  // 38   PWM3
    {39, PAD_MUX_ALT5, 4},  // 39   PWM4
    {45, PAD_MUX_ALT5, 3},  // 45   PWM3
    {46, PAD_MUX_ALT5, 4},  // 46   PWM4
    {47, PAD_MUX_ALT5, 5},  // 47   PWM5
    {48, PAD_MUX_ALT5, 0},  // 48   PWM0
    {49, PAD_MUX_ALT5, 1},  // 49   PWM1
    {50, PAD_MUX_ALT5, 2},  // 50   PWM2
    {51, PAD_MUX_ALT5, 3},  // 51   PWM3
    {52, PAD_MUX_ALT5, 4},  // 52   PWM4
    {53, PAD_MUX_ALT5, 5},  // 53   PWM5
};

const int8_t pwm_n_PinTable[][3] = {
    {16, PAD_MUX_ALT3, 1},  // 16   PWM1n
    {45, PAD_MUX_ALT3, 4},  // 45   PWM4n
    {46, PAD_MUX_ALT3, 3},  // 46   PWM3n
    {47, PAD_MUX_ALT3, 4},  // 47   PWM4n
    {48, PAD_MUX_ALT3, 1},  // 48   PWM1n
    {49, PAD_MUX_ALT3, 0},  // 49   PWM0n
    {50, PAD_MUX_ALT3, 3},  // 50   PWM3n
    {51, PAD_MUX_ALT3, 2},  // 51   PWM2n
    {52, PAD_MUX_ALT3, 5},  // 52   PWM5n
    {53, PAD_MUX_ALT3, 4},  // 53   PWM4n
};

/* ----------------------------------------- 运行状态表
 * ------------------------------------------ */
AP_PLAT_COMMON_DATA static int8_t pwmList[][4] = {
    {-1, 5, UNILOG_MODE, -1}, /* PWM0: freq/UNILOG_MODE */
    {-1, 5, IRQ_MODE, -1},    /* PWM1: freq/IRQ */
    {-1, 5, DMA_MODE, -1},    /* PWM2: freq/DMA_MODE */
    {-1, 5, DMA_MODE, -1},    /* PWM3: freq/DMA_MODE */
    {-1, 5, DMA_MODE, -1},    /* PWM4: freq/DMA_MODE */
    {-1, 5, DMA_MODE, -1}     /* PWM5: freq/DMA_MODE */
};

#define TIMER_IRQn             \
    {                          \
        47, 46, 45, 44, 43, 42 \
    }

static pwm_config_t* get_cur_cfg(uint32_t index)
{
    return &sPwmConfigList[index];
}

/**
  \fn          api_ret_t api_pwm_checkout(int8_t pin, int8_t pwm_n)
  \brief       检查PWM接口的引脚配置是否正确
  \param[in]   pin    PWM正极引脚编号
  \param[in]   pwm_n  PWM负极引脚编号（暂未使用）
  \return      检查结果，OPEN_HAL_DONE表示成功，其他值表示失败
  \details     该函数用于检查指定的PWM引脚配置是否正确
               在预定义的引脚表中，并且对应的PAD是否已配置为PWM功能。
*/
api_ret_t api_pwm_checkout(int8_t pin, int8_t pwm_n)
{
    int8_t select = -1;
    select = select;
    int8_t mux = -1;
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(pin > 0)
    {
        for(int8_t i = 0; i < sizeof(pwm_PinTable) / sizeof(pwm_PinTable[0]);
            i++)
        {
            if(pwm_PinTable[i][0] == pin)
            {
                mux = pwm_PinTable[i][1];
                if(check_pad_mux(pin, mux) == OPEN_HAL_DONE)
                {
                    select = pwm_PinTable[i][2];
                    ret = OPEN_HAL_DONE;
                    EPAT_LOG(pwm_checkout_pad, P_INFO, "pwm%d-pad%d-mux%d",
                             select, pin, mux);
                }
                else
                {
                    ret = OPEN_HAL_NONE;
                }
            }
        }
    }
    if(pwm_n > 0)
    {
        for(int8_t i = 0;
            i < sizeof(pwm_n_PinTable) / sizeof(pwm_n_PinTable[0]); i++)
        {
            if(pwm_n_PinTable[i][0] == pwm_n)
            {
                mux = pwm_n_PinTable[i][1];
                if(check_pad_mux(pwm_n, mux) == OPEN_HAL_DONE)
                {
                    select = pwm_n_PinTable[i][2];
                    ret = OPEN_HAL_DONE;
                    EPAT_LOG(pwm_n_checkout_pad, P_INFO, "pwm%d-pad%d-mux%d",
                             select, pwm_n, mux);
                }
                else
                {
                    ret = OPEN_HAL_NONE;
                }
            }
        }
    }
    return ret;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          uint32_t usrId_to_pwm(uint32_t usrId)
  \brief       根据用户ID提取PWM索引
  \param[in]   usrId  用户ID
  \return      PWM索引，如果ID无效则返回EC_PWM_INDEX_LIMIT
  \details     该函数用于从用户ID中提取出PWM索引。用户ID包含了PWM索引信息，
               通过位运算提取出索引并与有效范围比较，确保索引有效。
*/
static uint32_t usrId_to_pwm(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t pwm = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);  // 位提取
    ASSERT(pwm < EC_PWM_INDEX_LIMIT);
    if(pwm == (sPwmUsrIdList[pwm] & OPEN_HAL_PORT_MUSK))
    {
        return pwm;
    }
    return EC_PWM_INDEX_LIMIT;
}

/**
  \fn          api_ret_t api_pwm_query(uint32_t usrId)
  \brief       查询PWM设备状态
  \param[in]   usrId  PWM设备ID或物理地址
  \return      PWM设备当前状态
  \details     该函数用于查询指定PWM设备的当前状态（空闲、使用中、未使用等）。
*/
api_ret_t api_pwm_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_pwm(usrId);
    if(index >= EC_PWM_INDEX_START && index < EC_PWM_INDEX_LIMIT)
    {
        if(sPwmUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sPwmUsrIdList[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else
        {
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}

/**
  \fn          uint32_t pwm_set_free(uint32_t index, pwm_config_t *cfg)
  \brief       将PWM设备设置为空闲状态
  \param[in]   index  PWM索引
  \param[in]   cfg    PWM配置参数指针（暂未使用）
  \return      1表示成功，0表示失败
  \details     该函数用于将指定的PWM设备设置为空闲状态，清除中断回调函数。
*/
#if 0
static uint32_t pwm_set_free(uint32_t index,pwm_config_t *cfg)
{
    // ASSERT(index < EC_PWM_INDEX_LIMIT);
    GpioPinConfig_t pinConfig = {0};
    if(index >= EC_PWM_INDEX_START && index < EC_PWM_INDEX_LIMIT)
    {
        sPwmRuntimeList[index] = 0;
        pwm_isr_cb[index] = NULL; 
        return 1;  
    }
    return 0;
}
#endif

/**
  \fn          uint32_t pwm_set_idle(uint8_t index)
  \brief       将PWM设备设置为已创建但未使用状态
  \param[in]   index  PWM索引
  \return      用户ID，0表示失败
  \details     该函数用于将指定的PWM设备设置为已创建但未使用状态，
               并生成新的用户ID。
*/
static uint32_t pwm_set_idle(uint8_t index)
{
    // ASSERT(index < EC_PWM_INDEX_LIMIT);
    if(sPwmUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        sPwmUsrIdList[index] = index;
        sPwmUsrIdSeed[index]++;
        sPwmUsrIdList[index] |= (uint32_t)(sPwmUsrIdSeed[index] << 16);
    }
    else if(sPwmUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        sPwmUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
    }
    return sPwmUsrIdList[index];
}

/**
  \fn          uint32_t pwm_set_used(uint32_t usrId)
  \brief       将PWM设备设置为已创建且使用中状态
  \param[in]   usrId  PWM设备用户ID
  \return      0表示成功
  \details     该函数用于将指定的PWM设备设置为已创建且使用中状态。
*/
static uint32_t pwm_set_used(uint32_t usrId)
{
    api_ret_t stat = api_pwm_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_pwm(usrId);
    // ASSERT(index < EC_PWM_INDEX_LIMIT);
    sPwmUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
#ifdef FEATURE_OPENHAL_RTOS_ENABLE
#endif
    return 0;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          api_ret_t api_pwm_setup(int8_t index, pwm_config_t* para)
  \brief       PWM初始化，使用HAL统一的参数结构，会执行硬件配置
  \param[in]   index  PWM索引
  \param[in]   para   PWM配置参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于初始化指定的PWM设备并进行硬件配置。
               如果para为NULL，则停止PWM；否则根据参数配置PWM频率、占空比等。
*/
api_ret_t api_pwm_setup(int8_t index, pwm_config_t *para)
{
    if(index < EC_PWM_INDEX_START || index >= EC_PWM_INDEX_LIMIT)
    {
        EPAT_LOG(api_pwm_setup_1, P_ERROR, "error index %d [%d,%d]", index,
                 EC_PWM_INDEX_START, EC_PWM_INDEX_LIMIT);
        return OPEN_HAL_INVALID_PARA;
    }
    ClockId_e pclk = gTimerClocks[index * 2];
    ClockId_e fclk = gTimerClocks[index * 2 + 1];
    uint32_t fclk_val = GPR_getClockFreq(fclk);
    if(para == NULL)
    {
        TIMER_stop(index);
        if(fclk_val >= 0U)
        {
            CLOCK_clockDisable(fclk);
        }
        if(GPR_clockEnableCheck(pclk))
        {
            CLOCK_clockDisable(pclk);        // 默认由devicemanager统一管理，此处临时配置，后续版本移除
        }
        sPwmRuntimeList[index] = 0;     // 运行参数清空
        return OPEN_HAL_DONE;
    }
    else
    {
        if(!GPR_clockEnableCheck(pclk))
        {
            CLOCK_clockEnable(pclk);        // 默认由devicemanager统一管理，此处临时配置，后续版本移除
        }
        if(fclk_val == 0U)
        {
            CLOCK_clockEnable(fclk);
        }
        TimerPwmConfig_t oldConfig;
        TIMER_getDefaultConfig((TimerConfig_t *)&oldConfig);
        oldConfig.pwmFreq_HZ = (para->freq) * 1000;  // 1KHZ 可修改倍率和算法
        oldConfig.srcClock_HZ = GPR_getClockFreq(fclk);
        oldConfig.stopOption = para->act;
        oldConfig.dutyCyclePercent = para->duty;
        // TIMER_driverInit();
        TIMER_setupPwm(index, &oldConfig);
        TIMER_start(index);
        sPwmRuntimeList[index] = *(uint32_t *)para;  // 运行参数更新
        EPAT_LOG(api_pwm_setup, P_INFO,
                 "pwm%d:pin %d,act %d,duty %d,freq %d,mode %d", index,
                 para->pin, para->act, para->duty, para->freq, para->mode);
        return OPEN_HAL_DONE;
    }
}

/**
  \fn          int8_t *api_pwm_startup(void* para, int8_t *pad)
  \brief       所有PWM上电初始化为配置状态
  \param[in]   para  指向PWM配置参数数组的指针（暂未使用）
  \param[in]   pad   指向PAD配置参数数组的指针
  \return      返回指向PWM状态列表的指针
  \details     该函数用于在系统上电时初始化所有PWM设备。
*/
int8_t *api_pwm_startup(void *para, int8_t *pad)
{
    // int8_t (*padList)[4] = pad;
    for(int i = EC_PWM_INDEX_START; i < EC_PWM_INDEX_LIMIT; i++)
    {
        sPwmUsrIdList[i] = OPEN_HAL_STAT_UNUSED;
    }
    return (int8_t *)pwmList;
}

/**
  \fn          int32_t api_pwm_parse(char* str,pwm_config_t *cfg)
  \brief       解析PWM配置字符串
  \param[in]   str   PWM配置字符串
  \param[out]  cfg   PWM配置参数结构体指针
  \return      PWM索引，负值表示失败
  \details     该函数用于解析PWM配置字符串，并将解析结果存储到配置结构体中。
*/
extern int8_t list_pad2gpio[];
int32_t api_pwm_parse(char *str, pwm_config_t *cfg)
{
    if(str == NULL || cfg == NULL)
    {
        EPAT_LOG(api_pwm_parse_0, P_ERROR, "invalid para");
        return -1;
    }
    uint8_t ptr = 0;
    char *item_str[CSV_CFG_PWM_ITEMS];
    char *head = strtok(str, ",");
    while(head != NULL && ptr < CSV_CFG_PWM_ITEMS)
    {
        item_str[ptr] = head;
        head = strtok(NULL, ",");
        ptr++;
    }
    if(ptr < CSV_CFG_PWM_ITEMS)
    {
        EPAT_LOG(api_pwm_parse_1, P_ERROR, "input items %d < %d", ptr,
                 CSV_CFG_PWM_ITEMS);
        return -1;
    }
    ptr = 0;
    int8_t index = atoi(item_str[ptr++]);
    if(index < EC_PWM_INDEX_START || index > EC_PWM_INDEX_LIMIT)
    {
        EPAT_LOG(api_pwm_parse_2, P_ERROR, "pwm%d invalid", index);
        return -2;
    }
    else
    {
        cfg->port = index;
    }
    int32_t value = atoi(item_str[ptr++]);  // 1: pin
    if(value >= EC_PAD_INDEX_START && value < EC_PAD_INDEX_LIMIT)
    {
        cfg->pin = value;
        cfg->gpio = list_pad2gpio[value];
    }
    else
    {
        cfg->pin = 0;
        cfg->gpio = EC_GPIO_INDEX_LIMIT;
    }
    value = atoi(item_str[ptr++]);  // 2: act
    if(value >= 0 && value < 2)
    {
        cfg->act = value;
    }
    value = atoi(item_str[ptr++]);  // 3: duty
    if(value >= 0 && value <= 100)
    {
        cfg->duty = value;
    }
    value = atoi(item_str[ptr++]);  // 4: freq
    if(value >= 1000 && value < 16000)
    {
        cfg->freq = value / 1000;  // 后续可修改算法
    }
    value = atoi(item_str[ptr++]);  // 5: mode
    if(value >= 0 && value < 16)
    {
        cfg->mode = value;
    }
    EPAT_LOG(api_pwm_parse, P_INFO,
             "pwm%d:pin %d,act %d,duty %d,freq %d,mode %d;brief-%d:%s", index,
             cfg->pin, cfg->act, cfg->duty, cfg->freq, cfg->mode, ptr, head);
    return index;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          api_ret_t api_pwm_create(int8_t index, pwm_config_t *cfg, void
  *out) \brief       创建PWM设备实例，分配资源并检查依赖条件 \param[in]   index
  PWM索引 \param[in]   cfg    PWM配置参数指针 \param[out]  out
  输出参数，返回创建的PWM设备ID \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于创建一个新的PWM设备实例，分配相关资源并进行初始化。
               create接口只查询依赖硬件的状态（pad/clk）和分配mem空间资源，不执行硬件配置。
*/
api_ret_t api_pwm_create(int8_t index, pwm_config_t *cfg, void *out)
{
    if(index < EC_PWM_INDEX_START || index > EC_PWM_INDEX_LIMIT)
    {
        EPAT_LOG(api_pwm_create_0, P_ERROR, "pwm%d invalid", index);
        return OPEN_HAL_INVALID_PARA;
    }
    if(cfg != NULL)  // 查询是否具备相应依赖条件
    {
        pwm_config_t *cur_cfg = get_cur_cfg(index);
        memcpy(cur_cfg, cfg, sizeof(pwm_config_t));
        api_ret_t ret =
            api_pwm_checkout(cur_cfg->pin, -1);  // 只查询pwm，不查询pwm_n
        if(ret == OPEN_HAL_DONE)
        {
            EPAT_LOG(api_pwm_create_1, P_INFO, "pwm%d output with pin %d",
                     index, cur_cfg->pin);
        }
        else
        {
            EPAT_LOG(api_pwm_create_2, P_WARNING, "pwm%d no output pin %d",
                     index, cur_cfg->pin);
            return OPEN_HAL_INVALID_PARA;
        }
        // sPwmRuntimeList[index] = 0;
    }
    // 查询时钟
    ClockId_e pclk = gTimerClocks[index * 2];
    ClockId_e fclk = gTimerClocks[index * 2 + 1];
    fclk = fclk;
    if(GPR_clockEnableCheck(
           pclk))  // 不检查 GPR_clockEnableCheck(fclk) , 由用户open开启
    {
        EPAT_LOG(api_pwm_create_3, P_INFO, "pwm%d fclk=%d", index,
                 GPR_getClockFreq(fclk));
        // 配置分频和检查是否满足频率参数
    }
    else
    {
        // return OPEN_HAL_INVALID_PARA;
    }
    uint32_t usrId = pwm_set_idle(index);
    if(out != NULL)
    {
        *(uint32_t *)out = usrId;
    }
    if(usrId)
    {
        return OPEN_HAL_DONE;
    }
    return OPEN_HAL_INVALID_PARA;
}

/**
  \fn          api_ret_t api_pwm_delete(uint32_t usrId)
  \brief       删除PWM设备实例
  \param[in]   usrId  PWM设备用户ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于删除指定的PWM设备实例，释放相关资源。
*/
api_ret_t api_pwm_delete(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pwm_query(usrId);
    // uint32_t index = usrId_to_pwm(usrId);
    // ASSERT(index < EC_PWM_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE)
    {
        // if(pwm_set_free(index, &pwmStartupList[index]))
        {
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          api_ret_t api_pwm_open(uint32_t usrId, pwm_config_t *cfg, size_t
  timeout) \brief       打开PWM设备 \param[in]   usrId    PWM设备用户ID
  \param[in]   cfg      PWM配置参数指针
  \param[in]   timeout  超时时间（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于打开指定的PWM设备并进行配置。
*/
api_ret_t api_pwm_open(uint32_t usrId, pwm_config_t *cfg, size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pwm_query(usrId);
    uint32_t index = usrId_to_pwm(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        pwm_config_t *cur_cfg = get_cur_cfg(index);
        if(cfg != NULL)  // 查询是否具备相应依赖条件
        {
            memcpy(cur_cfg, cfg, sizeof(pwm_config_t));
            api_ret_t ret =
                api_pwm_checkout(cur_cfg->pin, -1);  // 只查询pwm，不查询pwm_n
            if(ret == OPEN_HAL_DONE)
            {
                EPAT_LOG(api_pwm_create_1, P_INFO, "pwm%d output with pin %d",
                         index, cur_cfg->pin);
            }
            else
            {
                EPAT_LOG(api_pwm_create_2, P_WARNING, "pwm%d no output pin %d",
                         index, cur_cfg->pin);
                return OPEN_HAL_INVALID_PARA;
            }
        }
        ret = api_pwm_setup(index, cur_cfg);
        pwm_set_used(usrId);
        ret = OPEN_HAL_DONE;
        //printf("\r\npwm:%d,0x%08X,0x%X", index, sPwmUsrIdList[index],
        //       *(uint32_t *)config);
    }

    return ret;
}

/**
  \fn          api_ret_t api_pwm_close(uint32_t usrId)
  \brief       关闭PWM设备
  \param[in]   usrId  PWM设备用户ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于关闭指定的PWM设备并释放相关资源。
*/
api_ret_t api_pwm_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pwm_query(usrId);
    uint32_t index = usrId_to_pwm(usrId);
    // ASSERT(index < EC_PWM_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED)
    {
        api_pwm_setup(index, NULL);
        pwm_set_idle(index);
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          api_ret_t api_pwm_ioctl(uint32_t usrId, api_pwm_ioctl_t type,
  void *para) \brief       控制PWM设备 \param[in]   usrId  PWM设备用户ID
  \param[in]   type   控制类型
  \param[in]   para   控制参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于对PWM设备进行各种控制操作，如设置中断回调、调整频率等。
*/
api_ret_t api_pwm_ioctl(uint32_t usrId, api_pwm_ioctl_t type, void *para)
{
    if(type >= OPEN_PWM_IOCTL_ALL || usrId == 0) return OPEN_HAL_INVALID_PARA;
    api_ret_t ret = api_pwm_query(usrId);
    uint32_t index = usrId_to_pwm(usrId);
    ClockId_e fclk = gTimerClocks[index * 2 + 1];
    if(ret == OPEN_HAL_USED)
    {
        // uint8_t val = 0;
        IRQn_Type irqSel[] = TIMER_IRQn;
        TIMER_TypeDef *Timer = EIGEN_TIMER(index);
        uint32_t v_clk = GPR_getClockFreq(fclk);
        switch(type)
        {
            case OPEN_PWM_IOCTL_ISR:
                if(para != NULL)
                {
                    TIMER_interruptConfig(index, TIMER_MATCH0_INTERRUPT,
                                          TIMER_INTERRUPT_LEVEL);
                    XIC_SetVector(irqSel[index], para);
                    XIC_EnableIRQ(irqSel[index]);
                    XIC_SuppressOvfIRQ(irqSel[index]);
                }
                else
                {
                    XIC_DisableIRQ(irqSel[index]);
                    XIC_ClearPendingIRQ(irqSel[index]);
                    TIMER_interruptConfig(index, TIMER_MATCH0_INTERRUPT,
                                          TIMER_INTERRUPT_DISABLE);
                }
                break;
            case OPEN_PWM_IOCTL_FREQ:
                if(para != NULL)
                {
                    CLOCK_clockDisable(fclk);
                    uint32_t pwmFreq = *(uint32_t *)para;
                    uint32_t period = Timer->TMR[1] + 1;
                    if(pwmFreq < v_clk)
                    {
                        period = v_clk / pwmFreq;
                    }
                    Timer->TMR[1] = period - 1;
                }
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          api_ret_t api_pwm_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t
  count) \brief       对设备功耗和模式进行配置 \param[in]   usrId  PWM设备用户ID
  \param[in]   cfg    功耗配置参数指针
  \param[in]   count  参数数量（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于控制PWM设备的功耗模式。
*/
api_ret_t api_pwm_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    if(cfg == NULL || usrId == 0) return OPEN_HAL_INVALID_PARA;
    api_ret_t ret = api_pwm_query(usrId);
    uint32_t index = usrId_to_pwm(usrId);
    ClockId_e fclk = gTimerClocks[index * 2 + 1];
    if(ret == OPEN_HAL_USED)
    {
        if(cfg->runtime == RUNTIME_SUSPEND)
        {
            if(cfg->mode == PM_LOWPOW)
            {
                if(GPR_getClockFreq(fclk) >=
                   26000000U)  // 如果时钟频率大于等于26M，则降频处理
                {
                }
            }
        }
        EPAT_LOG(api_pwm_pmctl_3, P_INFO, "pwm%d fclk=%d", index,
                 GPR_getClockFreq(fclk));
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          api_ret_t api_pwm_write(uint32_t usrId, void* buf, size_t count)
  \brief       写入占空比
  \param[in]   usrId  PWM设备用户ID
  \param[in]   buf    要写入的数据缓冲区指针（应为uint8_t类型，表示占空比）
  \param[in]   count  要写入的数据大小（字节数，暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于向PWM设备写入占空比值。
*/
api_ret_t api_pwm_write(uint32_t usrId, void *buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pwm_query(usrId);
    uint32_t index = usrId_to_pwm(usrId);
    if(ret == OPEN_HAL_USED)
    {
        TIMER_updatePwmDutyCycle(index, *(uint8_t *)buf);
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          api_ret_t api_pwm_read(uint32_t usrId, void* buf, size_t count)
  \brief       读取占空比
  \param[in]   usrId  PWM设备用户ID
  \param[out]  buf    读取数据的缓冲区指针（应为uint8_t类型，表示占空比）
  \param[in]   count  要读取的数据大小（字节数，暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于从PWM设备读取当前占空比值。
*/
api_ret_t api_pwm_read(uint32_t usrId, void *buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pwm_query(usrId);
    uint32_t index = usrId_to_pwm(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL)
        {
            TIMER_TypeDef *Timer = EIGEN_TIMER(index);
            if(Timer->TMR[0] == Timer->TMR[1])
            {
                *(uint8_t *)buf = 100;
            }
            else if(Timer->TMR[0] > Timer->TMR[1])
            {
                *(uint8_t *)buf = 0;
            }
            else
            {
                *(uint8_t *)buf =
                    (Timer->TMR[1] - Timer->TMR[0]) * 100 / Timer->TMR[1];
            }
            if(Timer->TCCR == 0)
            {
                *(uint8_t *)buf = 0;
            }
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/* ----------------------------------------------------------------------------------------------
 */

/**
  \fn          int api_test_pwm(void)
  \brief       PWM设备测试接口
  \return      测试结果
  \details     该函数用于测试PWM设备的基本功能。目前尚未实现具体功能。
  \note        待实现：需要实现PWM测试功能
*/
#if 0
int api_test_pwm(void)
{
    // TODO: 实现PWM测试功能
}
#endif
