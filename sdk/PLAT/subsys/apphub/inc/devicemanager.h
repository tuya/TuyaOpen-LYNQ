/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    devicemanager.h
 * Description:  EC718 devicemanage header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_DEVICEMANAGER_H
#define  SUBSYS_DEVICEMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>  //mbtk change
#include "subsys.h"

#define MAX_IPS_NUM		20
#define MAX_EXTS_NUM	20

#define PIN_ALT_FUNC_OFF	8

/// @brief static dev resource table start ////////////////////////////////////////////////////

#define PIN_MAX_SLOT		6
#define GPIO_PIN_MAX_SLOT	2
#define NPWM_PIN_MAX_SLOT	2
#define KPC_PIN_MAX_SLOT	3
#define SWD_PIN_MAX_SLOT	2
#define SIM_PIN_MAX_SLOT	2
#define LSPI_PIN_MAX_SLOT	2
#define CSPI_PIN_MAX_SLOT	2
#define APWM_PIN_MAX_SLOT	3
#define PWM_PIN_MAX_SLOT	PIN_MAX_SLOT


typedef enum
{
	PIN_SLOT0 = 0,
	PIN_SLOT1 = 1,
	PIN_SLOT2 = 2,
	PIN_SLOT3 = 3,
	PIN_SLOT4 = 4,
	PIN_SLOT6 = 5,
} PinSlotType;

typedef enum
{
	PIN_Alt0 = 0,
	PIN_Alt1 = 1,
	PIN_Alt2 = 2,
	PIN_Alt3 = 3,
	PIN_Alt4 = 4,
	PIN_Alt5 = 5,
	PIN_Alt6 = 6,
} PinAltFunType;

typedef enum _EPadType
{
    PAD_MIN = 10,
    PAD_11 = 11,
    PAD_12 = 12,
    PAD_13 = 13,
    PAD_14 = 14,
    PAD_15 = 15,
    PAD_16 = 16,
    PAD_17 = 17,
    PAD_18 = 18,
    PAD_19 = 19,
    PAD_20 = 20,
    PAD_21 = 21,
    PAD_22 = 22,
    PAD_23 = 23,
    PAD_24 = 24,
    PAD_25 = 25,
    PAD_26 = 26,
    PAD_27 = 27,
    PAD_28 = 28,
    PAD_29 = 29,
    PAD_30 = 30,
    PAD_31 = 31,
    PAD_32 = 32,
    PAD_33 = 33,
    PAD_34 = 34,
    PAD_35 = 35,
    PAD_36 = 36,
    PAD_37 = 37,
    PAD_38 = 38,
    PAD_39 = 39,
    PAD_40 = 40,
    PAD_41 = 41,
    PAD_42 = 42,
    PAD_43 = 43,
    PAD_44 = 44,
    PAD_45 = 45,
    PAD_46 = 46,
    PAD_47 = 47,
    PAD_48 = 48,
    PAD_49 = 49,
    PAD_50 = 50,
    PAD_51 = 51,
    PAD_52 = 52,
    PAD_53 = 53,
    PAD_MAX = 54,
} EPadType;

typedef enum
{
    IP_PAD    		= 1,
    IP_GPIO    		= 2,
	IP_AGPIO    	= 3,
	IP_TIMER		= 4,
	IP_UART			= 5,
	IP_PWM			= 6,
	IP_APWM			= 7,
	IP_I2C			= 8,
	IP_SPI			= 9,
	IP_LSPI			= 10,
	IP_CSPI			= 11,
	IP_I2S			= 12,
	IP_ONEWIRE		= 13,
	IP_KPC			= 14,
	IP_ADC			= 15,
	IP_SWD			= 16,
	IP_SIM			= 17,
	IP_MAX			= 18,
} IpType;

typedef enum
{
    DEV_LCD    		= 1,
	DEV_BLE    		= 2,
	DEV_WIFI    	= 3,
	DEV_CODEC    	= 4,
} ExtDevType;

typedef struct 
{
	uint16_t swd_clk_selector;
	uint16_t swd_dio_selector;
} IpSwdPad;

typedef struct 
{
	uint16_t sim_clk_selector;
	uint16_t sim_uio_selector;
	uint16_t sim_rst_selector
} IpSimPad;

typedef struct 
{
	uint16_t i2c_sck_selector;
	uint16_t i2c_sda_selector;
} IpI2cPad;

typedef struct 
{
	uint16_t spi_sclk_selector;
	uint16_t spi_miso_selector;
	uint16_t spi_mosi_selector;
	uint16_t spi_ss_selector;
} IpSpiPad;

typedef struct 
{
	uint16_t i2s_mclk_selector;
	uint16_t i2s_bclk_selector;
	uint16_t i2s_lrck_selector;
	uint16_t i2s_din_selector;
	uint16_t i2s_dout_selector;
} IpI2sPad;

typedef struct 
{
	uint16_t lspi_clk_selector;
	uint16_t lspi_cs_selector;
	uint16_t lspi_sda_selector;
	uint16_t lspi_wr_selector;
} IpLspiPad;

typedef struct 
{
	uint16_t cspi_mclk_selector;
	uint16_t cspi_bclk_selector;
	uint16_t cspi_cs_selector;
	uint16_t cspi_rx0_selector;
	uint16_t cspi_rx1_selector;
} IpCspiPad;

typedef struct
{
	uint16_t uart_tx_selector;
	uint16_t uart_rx_selector;
	uint16_t uart_ctx_selector;
	uint16_t uart_rtx_selector;
	uint16_t uart_dcd_selector;
	uint16_t uart_dtr_selector;
} IpUartPad;

typedef struct 
{
	uint16_t gpio_pad_selector;
} IpGpioPad;

typedef struct 
{
	uint16_t apwm_pad_selector;
} IpApwmPad;

typedef struct 
{
	uint16_t pwm_pad_selector;
	uint16_t npwm_pad_selector;
} IpPwmPad;

typedef struct 
{
	uint16_t onewire_pad_selector;
} IpOneWirePad;

typedef struct 
{
	uint16_t Kpc_c0_pad_selector;
	uint16_t Kpc_c1_pad_selector;
	uint16_t Kpc_c2_pad_selector;
	uint16_t Kpc_c3_pad_selector;
	uint16_t Kpc_c4_pad_selector;
	uint16_t Kpc_r0_pad_selector;
	uint16_t Kpc_r1_pad_selector;
	uint16_t Kpc_r2_pad_selector;
	uint16_t Kpc_r3_pad_selector;
	uint16_t Kpc_r4_pad_selector;
} IpKpcPad;

// ips 
typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpSwdPad ip_pad[SWD_PIN_MAX_SLOT];
} IpSwd;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpSimPad ip_pad[SIM_PIN_MAX_SLOT];
} IpSim;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpSpiPad ip_pad[PIN_MAX_SLOT];
} IpSpi;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpI2cPad ip_pad[PIN_MAX_SLOT];
} Ipi2c;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpUartPad ip_pad[PIN_MAX_SLOT];
} IpUart;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpKpcPad ip_pad[KPC_PIN_MAX_SLOT];
} IpKpc;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpPwmPad ip_pad[PWM_PIN_MAX_SLOT];
} IpPwm;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpApwmPad ip_pad[APWM_PIN_MAX_SLOT];
} IpApwm;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
} IpAdc;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpGpioPad ip_pad[GPIO_PIN_MAX_SLOT];
} IpGpio;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpGpioPad ip_pad[GPIO_PIN_MAX_SLOT];
} IpAGpio;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpOneWirePad ip_pad[PIN_MAX_SLOT];
} IpOnewire;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpI2sPad ip_pad[PIN_MAX_SLOT];
} IpI2s;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpCspiPad ip_pad[CSPI_PIN_MAX_SLOT];
} IpCspi;

typedef struct
{
	IpType ip_type;
	int8_t ip_index;
	IpLspiPad ip_pad[LSPI_PIN_MAX_SLOT];
} IpLspi;

typedef struct
{
	IpUart uart[4];		//ok
	Ipi2c i2c[2];		//ok
	IpSpi spi[2];		//ok
	IpPwm pwm[6];		//ok
	IpApwm apwm[3];		//ok
	IpAdc adc[4];		//ok
	IpSwd swd[2];	    //ok
	IpI2s i2s[3];		//ok
	IpCspi cspi;		//ok
	IpLspi lspi;		//ok
	IpGpio gpio[39];	//ok
	IpSim sim;			//ok
	IpOnewire onewire;	//ok
	IpKpc kpc;			//ok
} Ec718IpRes;

/// @brief static dev resource table end ////////////////////////////////////////////////////

/// @brief dev node desciption start  ////////////////////////////////////////////////////
typedef enum _dnode_stat
{
	DNODE_FREE,		//no used
	DNODE_IDLE,		//opened
	DNODE_USED,		//used
	DNODE_INVALIED,	//invalid
} DnodeStat_e;

typedef enum 
{
    POWER_OFF1,                                  ///< Power off: no operation possible
    POWER_FULL                                  ///< Power on: full operation at maximum performance
} PowerState_e;

typedef enum _runtime_mode
{
	RUNTIME_IDLE,
	RUNTIME_SUSPEND,
	RUNTIME_RESUME,
} RuntimeMode_e;

typedef enum  _pm_mode{
	PM_COMMON,		// common run mode
	PM_LOWPOW,		// lowpower run mode
	PM_SLEEP,		// sleep mode
	PM_HIB,			// super sleep mode 
} PmMode_e;

typedef struct
{
	uint32_t hdr;
} DevHandler_t;

typedef struct 
{                          
  int32_t              (*open)          	(DevHandler_t dev_handler);                             
  int32_t              (*close)            	(DevHandler_t dev_handler);         					
  int32_t              (*write)            	(DevHandler_t dev_handler,char* buf,uint32_t size);					
  int32_t              (*read)            	(DevHandler_t dev_handler,char* buf,uint32_t size);					
  int32_t              (*ioctrl)            (DevHandler_t dev_handler,uint32_t control, int cmd, uint32_t arg);   
  int32_t              (*runtime)      		(DevHandler_t dev_handler,RuntimeMode_e run_mode, PmMode_e pm_e);
  int32_t              (*powerCtrl)       	(DevHandler_t dev_handler,PowerState_e state); 
} Dnode_t;

/// @brief dev node desciption end  ////////////////////////////////////////////////////


/// @brief dev dyn res start  ////////////////////////////////////////////////////

typedef struct
{
	uint32_t pinValid[4];
	uint32_t pinUnused[4];
	uint32_t pinStat[4];
	Dnode_t *pin_fun;
} PinTable_t;

typedef struct
{
	IpType ip;
	int8_t index;
	int8_t stat;
	Dnode_t *dev_fun;
} IpDev_t;

typedef struct
{
	IpDev_t swd_dev;
	IpSwdPad ip_pad;
} SwdDev_t;

typedef struct
{
	IpDev_t sim_dev;
	IpSimPad ip_pad;
} SimDev_t;

typedef struct
{
	IpDev_t sim_dev;
	IpSpiPad ip_pad;
} SpiDev_t;

typedef struct
{
	IpDev_t i2c_dev;
	IpI2cPad ip_pad;
} I2cDev_t;

typedef struct
{
	IpDev_t uart_dev;
	IpUartPad ip_pad;
} UartDev_t;

typedef struct
{
	IpDev_t kpc_dev;
	IpKpcPad ip_pad;
} KpcDev_t;

typedef struct
{
	IpDev_t pwm_dev;
	IpPwmPad ip_pad;
} PwmDev_t;

typedef struct
{
	IpDev_t apwm_dev;
	IpApwmPad ip_pad;
} ApwmDev_t;

typedef struct
{
	IpDev_t adc_dev;
} AdcDev_t;

typedef struct
{
	IpDev_t gpio_dev;
	IpGpioPad ip_pad;
} GpioDev_t;

typedef struct
{
	IpDev_t agpio_dev;
	IpGpioPad ip_pad;
} AGpioDev_t;

typedef struct
{
	IpDev_t onewire_dev;
	IpOneWirePad ip_pad;
} OnewireDev_t;

typedef struct
{
	IpDev_t i2s_dev;
	IpI2sPad ip_pad;
} I2sDev_t;

typedef struct
{
	IpDev_t cspi_dev;
	IpCspiPad ip_pad;
} CspiDev_t;

typedef struct
{
	IpDev_t cspi_dev;
	IpLspiPad ip_pad;
} LspiDev_t;

typedef struct
{
	UartDev_t uart[4];		//ok
	I2cDev_t i2c[2];		//ok
	SpiDev_t spi[2];		//ok
	PwmDev_t pwm[6];		//ok
	ApwmDev_t apwm[3];		//ok
	AdcDev_t adc[4];		//ok
	SwdDev_t swd[2];	    //ok
	I2sDev_t i2s[3];		//ok
	CspiDev_t cspi;			//ok
	LspiDev_t lspi;			//ok
	GpioDev_t gpio[39];		//ok
	SimDev_t sim;			//ok
	OnewireDev_t onewire;	//ok
	KpcDev_t kpc;			//ok
} DevTable_t;

typedef struct
{
	ExtDevType extdev;
	//extdev stat
	int8_t stat;
	Dnode_t *dev_fun;	//ext dev drv

	// connect to ip
	IpType ip;
	int8_t index;
} EextDev_t;

typedef struct
{
	EextDev_t ext[MAX_EXTS_NUM];
	int8_t stat[MAX_EXTS_NUM];
} ExtDevTable_t;

typedef struct
{
	ExtDevTable_t extTable;
	DevTable_t devTable;
	PinTable_t pinTable;
} DevManagerTable_t;

/// @brief dev dyn res end  ////////////////////////////////////////////////////

// init(struct dnode* node)
// open(struct dnode* node)
// close(struct dnode* node)
// read(struct dnode* node)
// write(struct dnode* node)
// ioctl(struct dnode* node,char inout *param ,char *in ,char *out)
// deinit(struct dev* node)


#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_DEVICEMANAGER_H */