#include "tkl_gpio.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_gpio_api.h"
#include "vlog.h"
#include "slpman.h"

typedef struct {
	uint8_t pin_id;
	uint8_t function;
	ol_gpio_interrupt_enum intr;
	bool	irq_flag;
	bool 	init_flag;				//初始化标志
	TUYA_GPIO_IRQ_CB cb;
	VOID_T* arg;
	TUYA_GPIO_BASE_CFG_T cfg;
} gpio_map_t;

#define GPIO_NUM_MAX    sizeof(pinMap)/sizeof(gpio_map_t)
extern bool tkl_cellular_get_sim_hotplug_status(UINT8_T sim_id);

static gpio_map_t pinMap[] = {
	{5,  0, 0, false, false, 0, 0}, 			//GPIO1 		-> GPIO20			//ok,wakeup3
	{6,  0, 0, false, false, 0, 0}, 			//GPIO2	 		-> GPIO21			//ok,wakeup4
	{16, 0, 0, false, false, 0, 0}, 			//NET_STATUS 	-> GPIO14			//ok
	{19, 0, 0, false, false, 0, 0}, 			//MAIN_DTR 		-> GPIO22			//ok,wakeup5
	{20, 0, 0, false, false, 0, 0}, 			//MAIN_RI		-> GPIO27			//ok
	{21, 4, 0, false, false, 0, 0}, 			//MAIN_DCD		-> GPIO17			//ok
	{22, 0, 0, false, false, 0, 0}, 			//MAIN_CTS		-> GPIO1			//ok
	// {23, 4, 0, false, false, 0, 0}, 			//MAIN_RTS		-> GPIO2			//配置成输出,输出异常
	{25, 0, 0, false, false, 0, 0}, 			//STATUS		-> GPIO26			//ok
	{26, 0, 0, false, false, 0, 0}, 			//I2S_MCLK		-> GPIO33			//ok
	{28, 0, 0, false, false, 0, 0}, 			//AUX_RXD 		-> GPIO10			//ok
	{29, 0, 0, false, false, 0, 0}, 			//AUX_TXD 		-> GPIO11			//ok
	{30, 0, 0, false, false, 0, 0}, 			//PCM_CLK 		-> GPIO29			//ok
	{31, 0, 0, false, false, 0, 0}, 			//PCM_SYNC 		-> GPIO30			//ok
	{32, 0, 0, false, false, 0, 0}, 			//PCM_DIN 		-> GPIO31			//设置成中断,不响应
	{33, 0, 0, false, false, 0, 0}, 			//PCM_DOUT 		-> GPIO32			//ok
	{49, 0, 0, false, false, 0, 0}, 			//LCD_RST 		-> GPIO15			//ok
	{50, 0, 0, false, false, 0, 0}, 			//LCD_SPI_OUT 	-> GPIO37			//ok
	{51, 0, 0, false, false, 0, 0}, 			//LCD_SPI_RS 	-> GPIO38			//ok
	{52, 0, 0, false, false, 0, 0}, 			//LCD_SPI_CS 	-> GPIO35			//ok
	{53, 0, 0, false, false, 0, 0}, 			//LCD_SPI_CLK 	-> GPIO34			//ok
	{54, 0, 0, false, false, 0, 0}, 			//CAM_MCLK 		-> GPIO3			//ok
	{55, 0, 0, false, false, 0, 0}, 			//CAM_SPI_DATA0 -> GPIO6			//ok
	{56, 0, 0, false, false, 0, 0}, 			//CAM_SPI_DATA1 -> GPIO7			//ok
	{57, 4, 0, false, false, 0, 0}, 			//CAM_I2C_SCL 	-> GPIO18			//ok
	{58, 4, 0, false, false, 0, 0}, 			//CAM_I2C_SDA 	-> GPIO19			//设置成中断,不响应
	// {62, 0, 0, false, false, 0, 0}, 			//USIM2_CLK 	-> GPIO14			//无法测试
	// {63, 0, 0, false, false, 0, 0}, 			//USIM2_RST 	-> GPIO13			//无法测试
	// {64, 0, 0, false, false, 0, 0}, 			//USIM2_DATA 	-> GPIO12			//无法测试
	// {66, 0, 0, false, false, 0, 0}, 			//I2C_SDA 		-> GPIO8			//无法测试
	// {67, 0, 0, false, false, 0, 0}, 			//I2C_SCL 		-> GPIO9			//无法测试
	// {78, 0, 0, false, false, 0, 0}, 			//LCD_TE 		-> GPIO36			//无法测试
	{79, 0, 0, false, false, 0, 0}, 			//USIM1_DET 						//wake_pad2, sim卡检测脚，关闭sim卡热插拔功能后可设置成中断
	{80, 0, 0, false, false, 0, 0}, 			//CAM_SPI_CLK 	-> GPIO4			//ok
	{81, 0, 0, false, false, 0, 0}, 			//CAM_PWDN 		-> GPIO5			//ok
	{82, 0, 0, false, false, 0, 0}, 			//USB_BOOT 		-> GPIO0			//ok
	{100,0, 0, false, false, 0, 0}, 			//GPIO3 		-> GPIO23			//无法测试
	// {101,0, 0, false, false, 0, 0}, 			//GPIO4 		-> GPIO24			//无法测试
	// {103,4, 0, false, false, 0, 0}, 			//CAM_RST 		-> GPIO16			//无法测试
};

static gpio_map_t *get_pinmap(int pin)
{
	for(int i = 0;i<GPIO_NUM_MAX;i++) {
		if(pin == pinMap[i].pin_id)
			return &pinMap[i];
	}
	return NULL;
}

static void gpio_irq_handle(uint8_t pin)   
{
	gpio_map_t* map = &pinMap[pin];
	if(!map->irq_flag)
		return ;
	
	unsigned int irqmask = 0;
	irqmask = ol_pin_get_irqmask(map->pin_id);			//关闭中断

	//是否是该pin脚的中断
	if (ol_pin_get_interrupt_flag(map->pin_id)) {
		ol_pin_clean_interrupt_flag(map->pin_id);			//清除中断
		if (map->cb) {
			map->cb(map->arg);
		} 
	}
	ol_pin_restore_irqmask(map->pin_id, irqmask);			//恢复中断
}

static void isr_cb(void)
{
	for(int i=0;i<sizeof(pinMap)/sizeof(gpio_map_t);i++) {
		gpio_irq_handle(i);
	}
}

static int gpio_config(gpio_map_t *map, const TUYA_GPIO_BASE_CFG_T *cfg)
{
	ol_gpio_config_struct config = {0};
	memset(&config, 0, sizeof(ol_gpio_config_struct));
	config.gpio_func = map->function;
	config.gpio_dir = (TUYA_GPIO_INPUT == cfg->direct) ? OL_GPIO_INPUT : OL_GPIO_OUTPUT;
	return ol_pin_config(map->pin_id, &config);
}

/**
 * @brief gpio init
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 * @param[in] cfg:  gpio config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_init(TUYA_GPIO_NUM_E pin_id, const TUYA_GPIO_BASE_CFG_T *cfg)
{
    int ret;

	gpio_map_t *map = get_pinmap(pin_id);
    if (map == NULL|| NULL == cfg) {
        LOGE("init failed, invalid para, pin: %d, cfg: %p", pin_id, cfg);
        return OPRT_COM_ERROR;
    }

	APmuWakeupPadSettings_t wakeupPadSetting;
	memset(&wakeupPadSetting, 0, sizeof(APmuWakeupPadSettings_t));
    wakeupPadSetting.negEdgeEn = false;
    wakeupPadSetting.posEdgeEn = false;
	if(cfg->mode == TUYA_GPIO_PULLDOWN)	
		wakeupPadSetting.pullDownEn = true;
	else 
		wakeupPadSetting.pullUpEn = true;
	switch(map->pin_id)
	{
		case 5:
			slpManSetWakeupPadCfg(WAKEUP_PAD_3, false, &wakeupPadSetting);
			NVIC_DisableIRQ(PadWakeup3_IRQn);
		break;
		case 6:
			slpManSetWakeupPadCfg(WAKEUP_PAD_4, false, &wakeupPadSetting);
			NVIC_DisableIRQ(PadWakeup4_IRQn);
		break;
		case 19:
			slpManSetWakeupPadCfg(WAKEUP_PAD_5, false, &wakeupPadSetting);
			NVIC_DisableIRQ(PadWakeup5_IRQn);
		break;
		case 79:
			return OPRT_NOT_SUPPORTED;
	}
	ret = gpio_config(map, cfg);
	if(ret) {
		LOGE("ol_pin_config %d failed :%d",pin_id, ret);
		return ret;
	}

	ol_gpio_pull_enum pullconfig = OL_PULL_AUTO;
	switch (cfg->mode) {
		case TUYA_GPIO_PULLUP:
			pullconfig = OL_PULL_UP; break;
		case TUYA_GPIO_PULLDOWN:
			pullconfig = OL_PULL_DOWN; break;
		default:
			pullconfig = OL_PULL_AUTO; break;
	}
	ret = ol_pin_set_pull(map->pin_id, pullconfig);
	if (0 != ret) {
		LOGE("gpio %d init failed, set pull ret: %d", pin_id, ret);
		return OPRT_COM_ERROR;
	}
	
	map->init_flag = true;
	memcpy(&map->cfg, cfg, sizeof(TUYA_GPIO_BASE_CFG_T));
    LOGI("gpio init success, pin/%d, mode/%d, direct/%d, level/%d", pin_id, cfg->mode, cfg->direct, cfg->level);

	if(cfg->direct == TUYA_GPIO_OUTPUT) {
		ol_gpio_level_enum ol_level = (TUYA_GPIO_LEVEL_HIGH == cfg->level) ? OL_GPIO_LEVEL_HIGH : OL_GPIO_LEVEL_LOW;
		return ol_pin_set_level(map->pin_id, ol_level);
	}
    return OPRT_OK;
}

/**
 * @brief gpio deinit
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_deinit(TUYA_GPIO_NUM_E pin_id)
{
    int ret;
	gpio_map_t *map = get_pinmap(pin_id);
	if(map == NULL) {
		LOGE("gpio deinit failed, invalid pin: %d", pin_id);
        return OPRT_COM_ERROR;
	}

	if(pin_id == 79) {
		tkl_gpio_irq_disable(pin_id);
		map->irq_flag = false;
		return OPRT_OK;
	}

	ol_gpio_config_struct config = {
		.gpio_func = map->function,
		.gpio_dir = OL_GPIO_INPUT,
		.gpio_interrupt = OL_GPIO_INTERRUPT_DISABLED,
		.gpio_initoutput = 0,
	};
	ol_gpio_pull_enum pullconfig = OL_PULL_AUTO;
	
	ret = ol_pin_config(map->pin_id, &config);
	if (0 != ret) {
		LOGE("gpio %d deinit failed, ret: %d", pin_id, ret);
		return OPRT_COM_ERROR;
	}

	ret = ol_pin_set_pull(map->pin_id, pullconfig);
	if (0 != ret) {
		LOGE("gpio %d deinit failed, set pull ret: %d", ret);
		return OPRT_COM_ERROR;
	}

	map->intr = OL_GPIO_INTERRUPT_DISABLED;
	map->cb = NULL;
	map->arg = NULL;
	map->init_flag = false;

    LOGI("gpio deinit success, pin: %d", pin_id);
    return OPRT_OK;
}

/**
 * @brief gpio write
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 * @param[in] level: gpio output level value
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_write(TUYA_GPIO_NUM_E pin_id, TUYA_GPIO_LEVEL_E level)
{
    int ret;
    gpio_map_t *map = get_pinmap(pin_id);
	if(map == NULL) {
		LOGE("gpio deinit failed, invalid pin: %d", pin_id);
        return OPRT_COM_ERROR;

	}
	//低功耗模式下,gpio需重新初始化
	if(!map->init_flag) {
		return OPRT_COM_ERROR;
	}

	// gpio_config(map, &map->cfg);
	ol_gpio_level_enum ol_level = (TUYA_GPIO_LEVEL_HIGH == level) ? OL_GPIO_LEVEL_HIGH : OL_GPIO_LEVEL_LOW;
	ret = ol_pin_set_level(map->pin_id, ol_level);
	if (0 != ret) {
		LOGE("gpio %d write %d failed, ret: %d", pin_id, level, ret);
		return OPRT_COM_ERROR;
	}

    return OPRT_OK;
}

/**
 * @brief gpio read
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 * @param[out] level: gpio output level
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_read(TUYA_GPIO_NUM_E pin_id, TUYA_GPIO_LEVEL_E *level)
{
    OPERATE_RET ret;
    gpio_map_t *map = get_pinmap(pin_id);
	if(map == NULL) {
		LOGE("gpio deinit failed, invalid pin: %d", pin_id);
        return OPRT_COM_ERROR;
	}

	//agpio 中断方式下,按一下方式读取电平
	if(map->pin_id == 5 || map->pin_id == 6 || map->pin_id == 19 || map->pin_id == 79) {
		if(map->irq_flag) {
			int bit = 0;
			switch (map->pin_id)
			{
			case 5:	bit = 3;	break;
			case 6:	bit = 4;	break;
			case 19:bit = 5;	break;
			case 79:bit = 2;	break;
			}
			*level = ((slpManGetWakeupPinValue() >> bit) & 0x1);
			return OPRT_OK;
		}
	}

	//低功耗模式下,gpio需重新初始化
	if(!map->init_flag) {
		return OPRT_COM_ERROR;
	}
	// gpio_config(map, &map->cfg);

	ol_gpio_level_enum ol_level;
	ret = ol_pin_get_level(map->pin_id, &ol_level);
	if (0 != ret) {
		LOGE("gpio %d read failed, ret: %d", pin_id, ret);
		return OPRT_COM_ERROR;
	}

	*level = (OL_GPIO_LEVEL_HIGH == ol_level) ? TUYA_GPIO_LEVEL_HIGH : TUYA_GPIO_LEVEL_LOW;
    return OPRT_OK;
}

static OPERATE_RET agpio_irq_init(gpio_map_t *map, CONST TUYA_GPIO_IRQ_T *cfg)
{
	APmuWakeupPadSettings_t wakeupPadSetting;
	memset(&wakeupPadSetting, 0, sizeof(APmuWakeupPadSettings_t));
	wakeupPadSetting.pullUpEn = true;
	switch (cfg->mode) {
		case TUYA_GPIO_IRQ_RISE:	
			wakeupPadSetting.posEdgeEn = true; 	
		break;
		case TUYA_GPIO_IRQ_FALL:	
			wakeupPadSetting.negEdgeEn = true; 
		break;
		case TUYA_GPIO_IRQ_RISE_FALL:
			wakeupPadSetting.posEdgeEn = true; 	
			wakeupPadSetting.negEdgeEn = true; 
		break;
		default:
			LOGE("gpio irq init failed, unsupport irq mode: %d", cfg->mode);
			return OPRT_NOT_SUPPORTED;
	}

	switch(map->pin_id)
	{
		case 5:
			slpManSetWakeupPadCfg(WAKEUP_PAD_3, true, &wakeupPadSetting);
			NVIC_EnableIRQ(PadWakeup3_IRQn);
		break;
		case 6:
			slpManSetWakeupPadCfg(WAKEUP_PAD_4, true, &wakeupPadSetting);
			NVIC_EnableIRQ(PadWakeup4_IRQn);
		break;
		case 19:
			slpManSetWakeupPadCfg(WAKEUP_PAD_5, true, &wakeupPadSetting);
			NVIC_EnableIRQ(PadWakeup5_IRQn);
		break;
		case 79:
			if(tkl_cellular_get_sim_hotplug_status(0)) {
				LOGE("sim hotplug enabled, can't set sim detect pin irq");
				return OPRT_COM_ERROR;
			}
			slpManSetWakeupPadCfg(WAKEUP_PAD_2, true, &wakeupPadSetting);
			NVIC_EnableIRQ(PadWakeup2_IRQn);
		break;
	}

	map->intr = cfg->mode;
	map->cb = cfg->cb;
	map->arg = cfg->arg;
	map->irq_flag = true;
    LOGI("agpio irq init success, pin: %d", map->pin_id);
    return OPRT_OK;
}

int agpio_irq_callback(uint32_t pad_num)
{
	gpio_map_t *map = get_pinmap(pad_num);
	if(map == NULL || map->irq_flag == false) {
		return -1;
	}

	if(map->cb) {
		map->cb(map->arg);
	}
	return 0;
}

/**
 * @brief gpio irq init
 * NOTE: call this API will not enable interrupt
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 * @param[in] cfg:  gpio irq config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_irq_init(TUYA_GPIO_NUM_E pin_id, CONST TUYA_GPIO_IRQ_T *cfg)
{
	gpio_map_t *map = get_pinmap(pin_id);
	if(map == NULL || NULL == cfg) {
		LOGE("gpio deinit failed, invalid pin: %d", pin_id);
        return OPRT_COM_ERROR;
	}

	if(map->pin_id == 5 || map->pin_id == 6 || map->pin_id == 19 || map->pin_id == 79) {
		return agpio_irq_init(map, cfg);
	}

	ol_gpio_config_struct config = {
		.gpio_func = map->function,
		.gpio_dir = OL_GPIO_INPUT,
		.gpio_initoutput = 0,
	};

	int ret = ol_pin_config(map->pin_id, &config);
	if (0 != ret) {
		LOGE("irq init failed, pin: %d, ret: %d", pin_id, ret);
		return OPRT_COM_ERROR;
	}

	ol_gpio_interrupt_enum int_cfg;
	switch (cfg->mode) {
		case TUYA_GPIO_IRQ_RISE:
			int_cfg = OL_GPIO_INTERRUPT_RISING_EDGE; break;
		case TUYA_GPIO_IRQ_FALL:
			int_cfg = OL_GPIO_INTERRUPT_FALLING_EDGE; break;
		case TUYA_GPIO_IRQ_LOW:
			int_cfg = OL_GPIO_INTERRUPT_LOW_LEVEL; break;
		case TUYA_GPIO_IRQ_HIGH:
			int_cfg = OL_GPIO_INTERRUPT_HIGH_LEVEL; break;
		case TUYA_GPIO_IRQ_RISE_FALL:
			int_cfg = OL_GPIO_INTERRUPT_BOTH_EDGE; break;
		default:
			LOGE("gpio irq init failed, unsupport irq mode: %d", cfg->mode);
			return OPRT_NOT_SUPPORTED;
	}
	ret = ol_pin_set_interrupt(map->pin_id, int_cfg, isr_cb);
	if (0 != ret) {
		LOGE("gpio irq init failed, pin: %d, set int ret: %d", pin_id, ret);
		return OPRT_COM_ERROR;
	}
	
	map->intr = int_cfg;
	map->cb = cfg->cb;
	map->arg = cfg->arg;
	map->irq_flag = true;
    LOGI("gpio irq init success, pin: %d", pin_id);
    return OPRT_OK;
}

static OPERATE_RET agpio_irq_set(gpio_map_t *map, bool enable)
{
	IRQn_Type type;
	switch(map->pin_id)
	{
		case 5:		type = PadWakeup3_IRQn;		break;
		case 6:		type = PadWakeup4_IRQn;		break;
		case 19:	type = PadWakeup5_IRQn;		break;
		case 79:	type = PadWakeup2_IRQn;		break;
		default:
			return OPRT_INVALID_PARM;
	}

	if(enable)
		NVIC_EnableIRQ(type);
	else
		NVIC_DisableIRQ(type);
	return OPRT_OK;
}

static OPERATE_RET tkl_gpio_irq_set(TUYA_GPIO_NUM_E pin_id, bool enable)
{
	gpio_map_t *map = get_pinmap(pin_id);
	if(map == NULL) {
		LOGE("gpio deinit failed, invalid pin: %d", pin_id);
        return OPRT_COM_ERROR;
	}

	if(map->pin_id == 5 || map->pin_id == 6 || map->pin_id == 19 || map->pin_id == 79 ) {
		return agpio_irq_set(map, enable);
	}

	if (OL_GPIO_INTERRUPT_DISABLED == map->intr) {
		LOGE("gpio irq set error, irq not init, pin: %d, enable: %d", pin_id, enable);
		return OPRT_COM_ERROR;
	}
	
	int ret;
	if (false == enable) {
		ret = ol_pin_set_interrupt(map->pin_id, OL_GPIO_INTERRUPT_DISABLED, isr_cb);
		map->irq_flag = false;
	} else {
		ret = ol_pin_set_interrupt(map->pin_id, map->intr, isr_cb);
		map->irq_flag = true;
	}
	
	return (0 == ret) ? OPRT_OK : OPRT_COM_ERROR;
}

/**
 * @brief gpio irq enable
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_irq_enable(TUYA_GPIO_NUM_E pin_id)
{
    return tkl_gpio_irq_set(pin_id, true);
}

/**
 * @brief gpio irq disable
 *
 * @param[in] pin_id: gpio pin id, id index starts at 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_gpio_irq_disable(TUYA_GPIO_NUM_E pin_id)
{
    return tkl_gpio_irq_set(pin_id, false);
}

#if 0

#include "tkl_thread.h"
#include "tkl_semaphore.h"
#include "tkl_system.h"
static TKL_THREAD_HANDLE test_thread;
static void test_output_thread(void *args)
{
    int ret = 0;
    TUYA_GPIO_BASE_CFG_T gpio_cfg;
    int gpio_sum = sizeof(pinMap)/sizeof(pinMap[0]);

    //设置成输出
    for(int i =  0 ;i < gpio_sum;i++) {
        gpio_cfg.direct = TUYA_GPIO_OUTPUT;
        gpio_cfg.level = TUYA_GPIO_LEVEL_LOW;
        gpio_cfg.mode = TUYA_GPIO_PULLUP;
        ret = tkl_gpio_init(pinMap[i].pin_id, &gpio_cfg);
        if(ret) {
            LOGE("gpio init failed, pin: %d, ret: %d", pinMap[i].pin_id, ret);
        }
    }

    while (1)
    {
        for(int i =  0 ;i < gpio_sum;i++) {
            ret = tkl_gpio_write(pinMap[i].pin_id, TUYA_GPIO_LEVEL_HIGH);
            if(ret) {
                LOGE("gpio write failed, pin: %d, ret: %d", pinMap[i].pin_id, ret);
            }
        }
        tkl_system_sleep(500);
        for(int i =  0 ;i < gpio_sum;i++) {
            ret = tkl_gpio_write(pinMap[i].pin_id, TUYA_GPIO_LEVEL_LOW);
            if(ret) {
                LOGE("gpio write failed, pin: %d, ret: %d", pinMap[i].pin_id, ret);
            }
        }
        tkl_system_sleep(500);
    }
}

static void test_input_thread(void *args)
{
    int ret = 0;
    TUYA_GPIO_LEVEL_E  level;

    // le370     //5, 6, 16, 17, 18, 19, 20, 21, 22, 23, 25, 28, 29, 82
    // le270     //5, 6, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26, 28, 29, 30, 31, 32, 33, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 63, 66, 67, 78, 80, 81, 82, 103
	
    uint8_t pin_list[] = {49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 63, 66, 67, 78, 80, 81, 82, 103};
    TUYA_GPIO_BASE_CFG_T gpio_output_cfg = {
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_LOW,
        .mode = TUYA_GPIO_PULLUP
    };
    TUYA_GPIO_BASE_CFG_T gpio_intput_cfg = {
        .direct = TUYA_GPIO_INPUT,
        .mode = TUYA_GPIO_FLOATING
    };
    for(int i=0;i< sizeof(pin_list)/2;i++) {
        ret = tkl_gpio_init(pin_list[i*2], &gpio_output_cfg);
        if(ret) {
            LOGE("gpio init failed, pin: %d, ret: %d", pin_list[i*2], ret);
        }
        ret = tkl_gpio_write(pin_list[i*2], TUYA_GPIO_LEVEL_HIGH);
        if(ret) {
            LOGE("gpio write failed, pin: %d, ret: %d", pin_list[i*2], ret);
        }
        tkl_system_sleep(20);

        ret = tkl_gpio_init(pin_list[i*2+1], &gpio_intput_cfg);
        if(ret) {
            LOGE("gpio init failed, pin: %d, ret: %d", pin_list[i*2+1], ret);
        }
        ret = tkl_gpio_read(pin_list[i*2+1], &level);
        if(ret) {
            LOGE("gpio read failed, pin: %d, ret: %d", pin_list[i*2+1], ret);
        } 

        if(level != TUYA_GPIO_LEVEL_HIGH) {
            LOGE("gpio read error pin:%d  %d != %d", pin_list[i*2+1], TUYA_GPIO_LEVEL_HIGH, level);
        } else {
            LOGI("gpio read success pin_id:%d %d", pin_list[i*2+1], level);
        }

        ret = tkl_gpio_write(pin_list[i*2], TUYA_GPIO_LEVEL_LOW);
        tkl_system_sleep(20);
        ret = tkl_gpio_read(pin_list[i*2+1], &level);
        if(level != TUYA_GPIO_LEVEL_LOW) {
            LOGE("gpio read error pin_id:%d %d != %d", pin_list[i*2+1], TUYA_GPIO_LEVEL_LOW, level);
        } else {
            LOGI("gpio read success pin_id:%d %d", pin_list[i*2+1], level);
        }

        tkl_gpio_deinit(pin_list[i*2]);
        tkl_gpio_deinit(pin_list[i*2+1]);

        ret = tkl_gpio_init(pin_list[i*2+1], &gpio_output_cfg);
        if(ret) {
            LOGE("gpio init failed, pin: %d, ret: %d", pin_list[i*2+1], ret);
        }
        ret = tkl_gpio_write(pin_list[i*2+1], TUYA_GPIO_LEVEL_HIGH);
        if(ret) {
            LOGE("gpio write failed, pin: %d, ret: %d", pin_list[i*2+1], ret);
        }
        tkl_system_sleep(20);

        ret = tkl_gpio_init(pin_list[i*2], &gpio_intput_cfg);
        if(ret) {
            LOGE("gpio init failed, pin: %d, ret: %d", pin_list[i*2], ret);
        }
        ret = tkl_gpio_read(pin_list[i*2], &level);
        if(ret) {
            LOGE("gpio read failed, pin: %d, ret: %d", pin_list[i*2], ret);
        } 

        if(level != TUYA_GPIO_LEVEL_HIGH) {
            LOGE("gpio read error pin:%d  %d != %d", pin_list[i*2], TUYA_GPIO_LEVEL_HIGH, level);
        } else {
            LOGI("gpio read success pin_id:%d %d", pin_list[i*2], level);
        }

        ret = tkl_gpio_write(pin_list[i*2+1], TUYA_GPIO_LEVEL_LOW);
        tkl_system_sleep(20);
        ret = tkl_gpio_read(pin_list[i*2], &level);
        if(level != TUYA_GPIO_LEVEL_LOW) {
            LOGE("gpio read error pin:%d %d != %d", pin_list[i*2], TUYA_GPIO_LEVEL_LOW, level);
        } else {
            LOGI("gpio read success pin_id:%d %d", pin_list[i*2], level);
        }

        tkl_gpio_deinit(pin_list[i*2]);
        tkl_gpio_deinit(pin_list[i*2+1]);
    }
}

static void test_gpio_irq(void *param)
{
    TUYA_GPIO_NUM_E pin = (TUYA_GPIO_NUM_E)param;
    TUYA_GPIO_LEVEL_E  level;
    tkl_gpio_read(pin, &level);
    LOGI("pin irq %d %d", pin, level);
}

static void test_irq_thread(void *args)
{
    int ret = 0;
    TUYA_GPIO_IRQ_T irq_cfg = {
        .mode = TUYA_GPIO_IRQ_RISE_FALL,
        .cb = test_gpio_irq
    };

    for(int i=0;i<sizeof(pinMap)/sizeof(pinMap[0]);i++) {
        irq_cfg.arg = (void *)pinMap[i].pin_id;
        ret = tkl_gpio_irq_init(pinMap[i].pin_id, &irq_cfg);
        if(ret) {
            LOGE("gpio irq init failed, pin: %d, ret: %d", pinMap[i].pin_id, ret);
        }
    }
    while(1) {
        tkl_system_sleep(1000);
    }
}

void tkl_gpio_test(void)
{
    // tkl_thread_create(&test_thread, "test_thread", 4096,  2, test_output_thread, NULL);
    tkl_thread_create(&test_thread, "test_thread", 4096,  2, test_input_thread, NULL);
    // tkl_thread_create(&test_thread, "test_thread", 4096,  2, test_irq_thread, NULL);
}

#endif 

#if 0
#include "tkl_system.h"
void tkl_gpio_test(void)
{
	// tkl_gpio_init(TUYA_GPIO_NUM_E pin_id, const TUYA_GPIO_BASE_CFG_T *cfg)
	TUYA_GPIO_BASE_CFG_T cfg = {
		.mode = TUYA_GPIO_PUSH_PULL,
		.direct = TUYA_GPIO_OUTPUT,
		.level = 1
	};

	int ret;
	for(int i = 0;i< GPIO_NUM_MAX;i++) {
		ret = tkl_gpio_init(pinMap[i].pin_id, &cfg);
		if(ret) {
			LOGE("gpio init failed pin: %d", pinMap[i].pin_id);
		}
	}

	while(1) {
		for(int i = 0;i< GPIO_NUM_MAX;i++) {
			ret = tkl_gpio_write(pinMap[i].pin_id, 1);
			if(ret) {
				LOGE("gpio write failed pin: %d", pinMap[i].pin_id);
			}
		}
		tkl_system_sleep(500);

		for(int i = 0;i< GPIO_NUM_MAX;i++) {
			ret = tkl_gpio_write(pinMap[i].pin_id, 0);
			if(ret) {
				LOGE("gpio write failed pin: %d", pinMap[i].pin_id);
			}
		}
		tkl_system_sleep(500);
	}
}

static void test_gpio_irq_cb(VOID_T *args)
{
	int pin = (int)args;
	static int cnt  = 0;
	TUYA_GPIO_LEVEL_E level;
	tkl_gpio_read(pin, &level);
	LOGD("pin:%d irq_cb: %d level:%d", pin, cnt++, level);
}

void tkl_gpio_irq_test(void)
{
	LOGD("tkl_gpio_irq_test");
	TUYA_GPIO_IRQ_T cfg = {
		.mode = TUYA_GPIO_IRQ_RISE_FALL,
		.cb = test_gpio_irq_cb,
		.arg = NULL
	};

	int ret;
	for(int i = 0;i<GPIO_NUM_MAX;i++) {
		cfg.arg = (void *)pinMap[i].pin_id;
		ret = tkl_gpio_irq_init(pinMap[i].pin_id, &cfg);
		if(ret)
			LOGE("gpio irq init failed pin: %d", pinMap[i].pin_id);
	}
	while(1) {
		tkl_system_sleep(500);
	}
}

// void tkl_gpio_irq_test(void)
// {
// 	LOGD("tkl_gpio_irq_test");
// 	TUYA_GPIO_IRQ_T cfg = {
// 		.mode = TUYA_GPIO_IRQ_FALL,
// 		.cb = test_gpio_irq_cb,
// 		.arg = NULL
// 	};

// 	cfg.arg = (void *)16;
// 	tkl_gpio_irq_init(16, &cfg);

// 	cfg.arg = (void *)20;
// 	tkl_gpio_irq_init(20, &cfg);

// 	cfg.arg = (void *)25;
// 	cfg.mode = TUYA_GPIO_IRQ_RISE;
// 	tkl_gpio_irq_init(25, &cfg);

// 	cfg.arg = (void *)21;
// 	tkl_gpio_irq_init(21, &cfg);

// 	cfg.arg = (void *)22;
// 	tkl_gpio_irq_init(22, &cfg);

// 	cfg.arg = (void *)23;
// 	tkl_gpio_irq_init(23, &cfg);

// 	while(1) {
// 		tkl_system_sleep(500);
// 	}
// }
#endif