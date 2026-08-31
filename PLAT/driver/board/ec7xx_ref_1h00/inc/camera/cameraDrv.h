#ifndef __CAMERA_DRV_H__
#define __CAMERA_DRV_H__

#include "cspi.h"
/**
  \addtogroup cam_interface_gr
  \{
 */

typedef struct
{
    uint8_t regAddr;                            ///< Sensor I2C register address
    uint8_t regVal;                             ///< Sensor I2C register value
}camI2cCfg_t;

typedef enum
{
    CAM_LSB_MODE    = 0,                            ///< Little endian
    CAM_MSB_MODE    = 1,                            ///< Big endian
}endianMode_e;

typedef enum
{
    WIRE_1      = 0,                            ///< 1 wire
    WIRE_2      = 1,                            ///< 2 wire
}wireNum_e;


typedef enum
{
    SEQ_0       = 0,                            ///< rxd[0] 6 4 2 0
                                                ///< rxd[1] 7 5 3 1    
    SEQ_1       = 1,                            ///< rxd[1] 6 4 2 0
                                                ///< rxd[0] 7 5 3 1
}rxSeq_e;

typedef enum
{
	CSPI_0		= 0,
	CSPI_1		= 1,
}cspiInstance_e;

typedef enum
{
	CSPI_START	 = 1,			///< cspi enable
	CSPI_STOP    = 0,			///< Cspi disable
}cspiStartStop_e;

typedef enum
{
	CSPI_INT_ENABLE	    = 1,		///< cspi interrupt enable
	CSPI_INT_DISABLE    = 0,		///< Cspi interrupt disable
}cspiIntEnable_e;


typedef struct
{
    endianMode_e    endianMode;                 ///< Endian mode
    wireNum_e       wireNum;                    ///< Wire numbers
    rxSeq_e         rxSeq;                      ///< Bit sequence in 2 wire mode
    uint8_t 		cpol;
    uint8_t			cpha;
    uint8_t         ddrMode;
    uint8_t         wordIdSeq;
	uint8_t         yOnly;
    uint8_t         rowScaleRatio;
    uint8_t         colScaleRatio;
    uint8_t         scaleBytes;
    uint8_t         dummyAllowed;
}camParamCfg_t;

typedef struct
{
	uint32_t enableForCamera	: 1; // 0: isn't work for camera now; 1: is working for camera now
	uint32_t enableForUsr		: 1; // 0: isn't ready for usr;       1: is ready for usr
	uint32_t workingForUsr		: 1; // 0: usr has used this buf;     1: usr is using this buf
	uint32_t camErrCnt			: 3; // record camera err count
	uint32_t rsvd				: 26;
	uint32_t timeStamp;
	uint8_t  data[320*240*2];   // can be configed
}CameraBuf_t;

typedef enum 
{
    CAM_DIR_NORMAL = 0,
    CAM_DIR_MIRROR,
    CAM_DIR_FLIP,
    CAM_DIR_MIRROR_FLIP
} CamImgDir_e;

typedef enum 
{
    CAM_WB_AUTO = 0,
    CAM_WB_CLOUD,
    CAM_WB_DAYLIGHT,
    CAM_WB_INCANDESCENCE,
    CAM_WB_FLUORESCENT,
    CAM_WB_TUNGSTEN
} CamWb_e;

typedef enum 
{
    CAM_SCENE_DAY = 0,
    CAM_SCENE_NIGHT
} CamScene_e;


// 定义函数指针类型
typedef int (*SensorSetMirrorFlipFunc)(CamImgDir_e direct);
typedef int (*SensorSetFpsFunc)(uint32_t fps);
typedef int (*SensorGetSensorIDFunc)(uint32_t *id);
typedef int (*SensorSetEvFunc)(uint8_t ev);
typedef int (*SensorSetContrastFunc)(uint8_t contrast);
typedef int (*SensorSetSaturationFunc)(uint8_t sat);
typedef int (*SensorSetSharpFunc)(uint8_t sharp);
typedef int (*SensorSetAwbFunc)(bool awbEnable, uint8_t scenec);
typedef int (*SensorSetGammaFunc)(uint8_t *table, uint32_t size);
typedef int (*SensorSetAEFunc)(uint8_t aeEnable);
typedef int (*SensorSetSceneMode)(uint8_t scene);

// 定义包含函数指针的结构体
typedef struct {
    uint32_t imgWidth;
    uint32_t imgHeight;
    uint32_t maxFps;
    SensorSetMirrorFlipFunc SetMirrorFlip;
    SensorSetFpsFunc SetFps;
    SensorGetSensorIDFunc GetSensorID;
    SensorSetEvFunc SetEv;
    SensorSetContrastFunc SetContrast;
    SensorSetSaturationFunc SetSaturation;
    SensorSetSharpFunc SetSharp;
    SensorSetAwbFunc SetAwb;
    SensorSetGammaFunc SetGamma;
    SensorSetAEFunc SetAE;
    SensorSetSceneMode SetScene;
} SensorFuncObj_t;

typedef void (*camCbEvent_fn) (uint32_t event); ///< Camera callback event.
typedef void (*camIrq_fn)(void); 					///< Camera irq
typedef void (*camErrCb)(uint32_t stats);


void camInit(void* dataAddr, cspiCbEvent_fn uspCb, void* dmaCb, camErrCb errCb);

/**
  \brief Receive the picture has been taken.
  \param[out] dataIn     The buffer which is used to store the picture.
  \return              
*/
void camRecv(uint8_t * dataIn);

/**
  \brief Init sensor's registers.
  \return              
*/
void camRegCfg(void);

/**
  \brief Write some parameters into the sensor.
  \param[in] regInfo     Sensor I2C addr and value.
  \return              
*/
void camWriteReg(camI2cCfg_t* regInfo);

/**
  \brief Read from the sensor's I2C address.
  \param[in] regAddr     Sensor's I2C register address.
  \return              
*/
uint8_t camReadReg(uint8_t regAddr);

/**
  \brief Start or stop Camera controller.
  \param[in] startStop     If true, start camera controller. If false, stop camera controller.
  \return              
*/
void camStartStop(cspiStartStop_e startStop);

/**
  \brief Register irq for cspi.
  \param[in] instance     cspi0 or cspi1.
  \param[in] irqCb        irq cb.
  \return              
*/
void camRegisterIRQ(cspiInstance_e instance, camIrq_fn irqCb);

uint32_t camGetCspiStats();

/**
  \brief Enable or disable interrupt of cspi.
  \param[in] intEnable     interrupt enable or not.
  \return              
*/
void cspiStartIntEnable(cspiIntEnable_e intEnable);
void cspiEndIntEnable(cspiIntEnable_e endIntEnable);
void cspi2LspiEnable(uint8_t enable);
int camCheckErrStats();
#if (ENABLE_CAMERA_LDO == 1)
void camPowerOn(uint8_t ioInitVal);
#endif
void camGpioPulseCfg(uint8_t padAddr, uint8_t pinInstance, uint8_t pinNum);
void camGpioPulse(uint8_t pinInstance, uint8_t pinNum, uint32_t pulseDurationUs, uint8_t initialState, bool needLoop);
int camPicTake();
void camPicGive(int index);

int camSetMirrorFlip(CamImgDir_e direct);
int camSetFps(uint32_t fps);
int camGetSensorID(uint32_t *id);
int camSetEv(uint8_t ev);
int camSetContrast(uint8_t contrast);
int camSetSaturation(uint8_t sat);
int camSetSharp(uint8_t sharp);
int camSetAwb(bool awbEnable, CamWb_e scenec);
int camSetGamma(uint8_t *table, uint32_t size);
int camSetAE(bool aeEnable);
int camSetScene(uint8_t mode);
int camGetResolution(uint32_t *width, uint32_t *height);
int camGetMaxFps(uint32_t* maxFps);
#include "gc032a.h"
extern SensorFuncObj_t gc032aSnsObj;
#include "gc6153.h"
extern SensorFuncObj_t gc6153SnsObj;
#include "gc6133.h"
extern SensorFuncObj_t gc6133SnsObj;
#include "bf30a2.h"
extern SensorFuncObj_t bf30a2SnsObj;
#include "bf20a6.h"
extern SensorFuncObj_t bf20a6SnsObj;

/** \} */

#endif
