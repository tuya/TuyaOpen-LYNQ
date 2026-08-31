#ifndef _CAM_HAL_H_
#define _CAM_HAL_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include "api_cspi.h"
#include "api_i2c.h"

typedef enum CamType_
{
    CAM_TYPE_UNKNWON = -1,
    CAM_TYPE_BF20A6 = 1,
    CAM_TYPE_BF30A2,
    CAM_TYPE_GC032A,
    CAM_TYPE_GC6133,
    CAM_TYPE_GC6153,
    // add new camera type here
    CAM_TYPE_NUM
} CamType_e;

#if defined(HAL_CAM_BF20A6_BUILTIN)
typedef enum BF20A6ImgMode_
{
    BF20A6_IMG_MODE_SDR1BIT_640X480_10FPS = 0,
    BF20A6_IMG_MODE_SDR2BIT_640X480_15FPS = 1,
    BF20A6_IMG_MODE_SDR2BIT_640X480_30FPS_MONO = 2,
    BF20A6_IMG_MODE_MAX
} BF20A6ImgMode_e;
#endif
#if defined(HAL_CAM_BF30A2_BUILTIN)
typedef enum BF30A2ImgMode_
{
    BF30A2_IMG_MODE_SDR1BIT_240x320_15FPS = 0,
    BF30A2_IMG_MODE_MAX
} BF30A2ImgMode_e;
#endif
#if defined(HAL_CAM_GC032A_BUILTIN)
typedef enum GC032AImgMode_
{
    GC032A_IMG_MODE_SDR1BIT_640X480_15FPS = 0,
    GC032A_IMG_MODE_SDR2BIT_640X480_15FPS = 1,
    GC032A_IMG_MODE_DDR2BIT_640X480_15FPS = 2,
    GC032A_IMG_MODE_DDR2BIT_640X480_30FPS = 3,
    GC032A_IMG_MODE_MAX
} GC032AImgMode_e;
#endif
#if defined(HAL_CAM_GC6133_BUILTIN)
typedef enum GC6133ImgMode_
{
    GC6133_IMG_MODE_SDR1BIT_240x320_15FPS = 0,
    GC6133_IMG_MODE_MAX
} GC6133ImgMode_e;
#endif
#if defined(HAL_CAM_GC6153_BUILTIN)
typedef enum GC6153ImgMode_
{
    GC6153_IMG_MODE_SDR1BIT_240x320_15FPS = 0,
    GC6153_IMG_MODE_MAX
} GC6153ImgMode_e;
#endif

typedef enum
{
    CAM_DIR_NORMAL = 0,
    CAM_DIR_MIRROR,
    CAM_DIR_FLIP,
    CAM_DIR_MIRROR_FLIP,
    CAM_DIR_NUM
} CamImgDir_e;

typedef enum
{
    CAM_WB_AUTO = 0,
    CAM_WB_CLOUD,
    CAM_WB_DAYLIGHT,
    CAM_WB_INCANDESCENCE,
    CAM_WB_FLUORESCENT,
    CAM_WB_TUNGSTEN,
    CAM_WB_NUM
} CamWb_e;

typedef enum
{
    CAM_SCENE_DAY = 0,
    CAM_SCENE_NIGHT,
    CAM_SCENE_NUM
} CamScene_e;

typedef enum CamEndianMode_
{
    CAM_LSB_MODE = 0,  ///< Little endian
    CAM_MSB_MODE = 1,  ///< Big endian
} CamEndianMode_e;

typedef enum CamSpiWireNum_
{
    WIRE_1 = 0,  ///< 1 wire
    WIRE_2 = 1,  ///< 2 wire
} CamSpiWireNum_e;

typedef enum CamSpiErr_
{
    CAM_SPI_ERR_NONE = 0,
    CAM_SPI_ERR_RX_OVERFLOW = 1,
    CAM_SPI_ERR_DMA_ERR = 2,
    CAM_SPI_ERR_NO_FRAMESTART = 4,
} CamSpiErr_e;

typedef enum CamSpiRxSeq_
{
    SEQ_0 = 0,  ///< rxd[0] 6 4 2 0
                ///< rxd[1] 7 5 3 1
    SEQ_1 = 1,  ///< rxd[1] 6 4 2 0
                ///< rxd[0] 7 5 3 1
} CamSpiRxSeq_e;

typedef enum CamSpiCspiInstance_
{
    CSPI_0 = 0,
    CSPI_1 = 1,
} CamSpiCspiInstance_e;

typedef enum CamImgFmt_
{
    CAM_IMG_FMT_YUV420,
    CAM_IMG_FMT_YUV422,
    CAM_IMG_FMT_RGB565,
    CAM_IMG_FMT_MONO,
    CAM_IMG_FMT_MAX,
} CamImgFmt_e;

typedef struct ImgHead_
{
    uint32_t timestamp;
    uint32_t resevered;
} ImgHead_t;

typedef struct CamImgInfo_
{
    uint16_t width;     //图像数据宽度
    uint16_t height;    //图像数据高度
    uint16_t sns_width; //图像传感器输出图像宽度
    uint16_t sns_height; //图像传感器输出图像高度
    CamImgFmt_e fmt;
    uint16_t max_fps;  // 最大帧率乘100，如15fps，即1500
} CamImgInfo_t;

typedef struct CamSpiCfg_
{
    CamEndianMode_e endianMode;  ///< Endian mode
    CamSpiWireNum_e wireNum;     ///< Wire numbers
    CamSpiRxSeq_e rxSeq;         ///< Bit sequence in 2 wire mode
    uint8_t cpol;
    uint8_t cpha;
    uint8_t ddrMode;
    uint8_t wordIdSeq;
    uint8_t yOnly;
    uint8_t rowScaleRatio;
    uint8_t colScaleRatio;
    uint8_t scaleBytes;
    uint8_t dummyAllowed;
} CamSpiCfg_t;

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
typedef int (*SensorGetInitRegList)(uint8_t mode, uint32_t **reg_list,
                                    uint32_t *count);
typedef int (*SensorPowerDown)(void);
typedef int (*SensorPowerUp)(void);
typedef void (*CamSpiCbEventFunc)(uint32_t event, void *param);
typedef void (*CamDataIrqFunc)(uint32_t stats, void *param);
typedef void (*CamSpiErrFunc)(uint32_t stats, void *param);

typedef struct CamExtPwrCfg_
{
    uint8_t enable;
    uint8_t pad_num;
    uint8_t mux;
    uint8_t io_num;
} CamExtPwrCfg_t;

typedef struct CamRstPinCfg_
{
    uint8_t pad_num;
    uint8_t mux;
    uint8_t io_num;
    uint8_t reset_level;
} CamRstPinCfg_t;

typedef struct CamI2cCfg_
{
    uint8_t i2c_port;
    I2cSpeed_e speed;
    uint8_t dev_addr;
} CamI2cCfg_t;

typedef struct CamWndCfg_
{
    uint32_t start_x;
    uint32_t start_y;
    uint32_t width;
    uint32_t height;
} CamWndCfg_t;

typedef struct CamSeqCfg_
{
    uint32_t clock_delay;
} CamSeqCfg_t;

typedef struct CamReqBuf_
{
    uint8_t *pool_addr; /* 数据缓冲区池首地址, 如果为空则内部自动申请 */
    uint8_t buff_count; /* 数据缓冲区数量 */
    uint32_t buff_size; /* 数据缓冲区每个数据的长度 */
} CamReqBuf_t;

typedef struct CamImg_
{
    uint8_t *addr;      /* 图像数据地址 */
    uint32_t size;      /* 图像数据长度 */
    uint16_t width;     /* 图像宽度 */
    uint16_t height;    /* 图像高度 */
    CamImgFmt_e fmt;    /* 图像数据格式 */
    uint32_t timestamp; /* 图像数据采集时间戳 */
} CamImg_t;

typedef struct QBufCfg_
{
    uint8_t* pool_addr;
    uint32_t item_size;
    uint32_t item_count;
} QBufCfg_t;

typedef struct CamCfg_
{
    CamType_e type;
    CamSpiCspiInstance_e drv_id;
    uint8_t int_mode;
    camFrequence_e mclk_freq;
    camResolution_e reso;
    CamSeqCfg_t seq_cfg;
    bool img_out_wnd;
    CamWndCfg_t wnd_cfg;
    CamSpiCfg_t spi_cfg;
    CamExtPwrCfg_t ext_pwr_cfg;
    CamRstPinCfg_t rst_pin_cfg;
    CamI2cCfg_t i2c_cfg;
    QBufCfg_t qbuf_cfg;
} CamCfg_t;

typedef struct CamCbCfg_
{
    CamSpiCbEventFunc cb_event;
    void *cb_event_param;
    CamDataIrqFunc cb_data_irq;
    void *cb_data_irq_param;
    CamSpiErrFunc cb_err;
    void *cb_err_param;
} CamCbCfg_t;

// 定义包含函数指针的结构体
typedef struct
{
    CamType_e type;
    uint32_t sensor_id;
    uint32_t img_width;
    uint32_t img_height;
    uint32_t max_fps;
    uint8_t dev_addr;
    uint8_t reg_addr_size;
    uint8_t reg_data_size;
    CamCfg_t *default_cfg;
    SensorSetMirrorFlipFunc pfn_set_mirror_flip;
    SensorSetFpsFunc pfn_set_fps;
    SensorGetSensorIDFunc pfn_get_sensor_id;
    SensorSetEvFunc pfn_set_ev;
    SensorSetContrastFunc pfn_set_contrast;
    SensorSetSaturationFunc pfn_set_saturation;
    SensorSetSharpFunc pfn_set_sharp;
    SensorSetAwbFunc pfn_set_awb;
    SensorSetGammaFunc pfn_set_gamma;
    SensorSetAEFunc pfn_set_ae;
    SensorSetSceneMode pfn_set_scene;
    SensorGetInitRegList pfn_get_init_reg_list;
    SensorPowerDown pfn_power_down;
    SensorPowerUp pfn_power_up;
} SensorFuncObj_t;

CamType_e hal_cam_get_type(void);
int hal_cam_set_type(CamType_e type);
SensorFuncObj_t *hal_cam_get_func_obj(void);
int hal_cam_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_data);
int hal_cam_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data);
int hal_cam_init(CamCfg_t *cfg);
int hal_cam_deinit();
int hal_cam_init_reglist(uint8_t mode);
int hal_cam_register_cb(CamCbCfg_t *cfg);
int hal_cam_start();
int hal_cam_stop();
int hal_cam_recv(uint8_t *data, uint32_t size);
int hal_cam_cmos_init(int id, CamExtPwrCfg_t *ext_pwr_cfg,
                      CamRstPinCfg_t *rst_pin_cfg, CamI2cCfg_t *i2c_cfg);
CamCfg_t *hal_cam_get_default_cfg(CamType_e type);
void hal_cam_cspi_2_lspi(uint8_t enable);
int hal_cam_req_buf(CamReqBuf_t *req_buf);
int hal_cam_get_buf(CamImg_t *img, uint32_t timeout);
int hal_cam_release_buf(CamImg_t *img);
int hal_cam_clear_buff();
int hal_cam_read_data(CamImg_t *img, uint32_t size);
int hal_cam_get_img_info(CamImgInfo_t *img_info);
#ifdef __cplusplus
}
#endif
#endif /* _CAM_HAL_H_ */