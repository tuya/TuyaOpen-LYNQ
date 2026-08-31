/**
 * @file hal_screen.h
 * @brief LCD屏幕驱动硬件抽象层接口定义
 * @details 此文件定义了LCD屏幕驱动的统一接口、数据结构和枚举类型，
 *          支持多种显示屏类型和接口方式，提供屏幕初始化、显示控制等功能
 */
#ifndef _LCD_HAL_H_
#define _LCD_HAL_H_

#include <stdint.h>
#include <stdbool.h>
#include "ec7xx.h"
#include "bsp.h"
#include "api_def.h"

// 前向声明LcdDrvObj_t结构体
typedef struct LcdDrvObj_ LcdDrvObj_t;

#define LCD_COLOR565(r, g, b) \
    (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define RED (0x001f)
#define GREEN (0x07e0)
#define BLUE (0xf800)
#define WHITE (0xffff)
#define BLACK (0x0000)
#define YELLOW (0xffe0)
#define PURPLE (0x8010)
#define GOLDEN (0xFEA0)

/**
 * @def DMA_BULK_NUM
 * @brief DMA传输块的最大大小
 */
#define DMA_BULK_NUM (1023 * 8)

/**
 * @def DMA_DESC_MAX
 * @brief DMA描述符的最大数量
 */
#define DMA_DESC_MAX (160)

/**
 * @enum ScrlspiInstance_e
 * @brief 屏幕SPI实例枚举
 * @details 定义了可用于驱动屏幕的SPI实例
 */
typedef enum ScrlspiInstance_
{
    LSPI_2 = 2, /**< SPI2实例 */
} ScrlspiInstance_e;

/**
 * @enum ScreenType_e
 * @brief 屏幕类型枚举
 * @details 定义了支持的各种显示屏驱动芯片类型
 */
typedef enum ScreenType_
{
    SCREEN_TYPE_UNKNOWN = 0, /**< 未知屏幕类型 */
    SCREEN_TYPE_ST7789,      /**< ST7789驱动芯片 */
    SCREEN_TYPE_CO5300,      /**< CO5300驱动芯片 */
    SCREEN_TYPE_ST7735,      /**< ST7735驱动芯片 */
    SCREEN_TYPE_AXS15231,    /**< AXS15231驱动芯片 */
    SCREEN_TYPE_ST77922,     /**< ST77922驱动芯片 */
    SCREEN_TYPE_JD9850,      /**< JD9850驱动芯片 */
    SCREEN_TYPE_JD9855,      /**< JD9855驱动芯片 */
    SCREEN_TYPE_JD9853,      /**< JD9853驱动芯片 */
    // add new screen type here
    SCREEN_TYPE_TOTAL /**< 屏幕类型总数 */
} ScreenType_e;

/**
 * @enum camPreviewStartStop_e
 * @brief 摄像头预览控制枚举
 * @details 控制摄像头预览的启动和停止
 */
typedef enum
{
    stopPreview = 0,  /**< 停止预览 */
    startPreview = 1, /**< 启动预览 */
} camPreviewStartStop_e;

/**
 * @enum uspCbRole_e
 * @brief USP回调角色枚举
 * @details 定义USP(可能是某种特殊处理单元)回调函数的角色
 */
typedef enum
{
    cbForCam = 0,  /**< 摄像头相关回调 */
    cbForFill = 1, /**< 填充相关回调 */
} uspCbRole_e;

/**
 * @enum teEdgeSel_e
 * @brief TE信号(撕裂效应)触发沿选择枚举
 * @details 选择TE信号的上升沿或下降沿触发
 */
typedef enum
{
    TE_RISE_EDGE = 0, /**< 上升沿触发 */
    TE_FALL_EDGE = 1, /**< 下降沿触发 */
} teEdgeSel_e;

/**
 * @enum lcdConfig_e
 * @brief LCD配置选项枚举
 * @details 定义各种LCD配置命令和参数
 */
typedef enum
{
    OS_DELAY = 0,        /**< 操作系统延迟 */
    HAL_DELAY,           /**< HAL层延迟 */
    LCD_CMD_WRITE,       /**< LCD命令写入 */
    LCD_CMD_CONTINUE,    /**< LCD命令继续 */
    INF_COLOROUT = 0x0A, /**< 色彩输出信息 */
    INF_BUS_MCLK = 0x0C, /**< 总线主时钟信息 */
    INF_BUS_TYPE = 0x0E, /**< 总线类型信息 */
    INF_DATALANE = 0x0F, /**< 数据通道信息 */
} lcdConfig_e;

/**
 * @enum lcdInterfaceType_e
 * @brief LCD接口类型枚举
 * @details 定义了LCD支持的各种接口类型
 */
typedef enum lcdInterfaceType_
{
    INF_SPI_3W_I,   /**< 3线SPI接口模式I */
    INF_SPI_3W_II,  /**< 3线SPI接口模式II */
    INF_SPI_4W_I,   /**< 4线SPI接口模式I */
    INF_SPI_4W_II,  /**< 4线SPI接口模式II */
    INF_MSPI_4W_II, /**< 多SPI 4线接口模式II */
    INF_8080,       /**< 8080并行接口 */
} lcdInterfaceType_e;

/**
 * @enum DisDirection_e
 * @brief 显示方向枚举
 * @details 定义了屏幕的8种显示方向，以及镜像和轴交换标志
 */
typedef enum
{
    /* @---> X
       |
       Y
    */
    DIS_DIR_LRTB, /**< 从左到右，从上到下，屏幕原始方向 */

    /*  Y
        |
        @---> X
    */
    DIS_DIR_LRBT, /**< 从左到右，从下到上 */

    /* X <---@
             |
             Y
    */
    DIS_DIR_RLTB, /**< 从右到左，从上到下 */

    /*       Y
             |
       X <---@
    */
    DIS_DIR_RLBT, /**< 从右到左，从下到上 */

    /* @---> Y
       |
       X
    */
    DIS_DIR_TBLR, /**< 从上到下，从左到右 */

    /*  X
        |
        @---> Y
    */
    DIS_DIR_BTLR, /**< 从下到上，从左到右 */

    /* Y <---@
             |
             X
    */
    DIS_DIR_TBRL, /**< 从上到下，从右到左 */

    /*       X
             |
       Y <---@
    */
    DIS_DIR_BTRL, /**< 从下到上，从右到左 */

    DIS_DIR_MAX,
    /* 另一种表示旋转的方式，使用3位表示 */
    DIS_MIRROR_X = 0x40, /**< X轴镜像 */
    DIS_MIRROR_Y = 0x20, /**< Y轴镜像 */
    DIS_SWAP_XY = 0x80,  /**< 交换XY轴 */
} DisDirection_e;

/**
 * @enum DisplayMode_e
 * @brief 显示模式枚举
 * @details 定义了屏幕的镜像和轴交换显示模式
 */
typedef enum DisplayMode_
{
    DISPLAY_MODE_NORMAL = 0,   /**< 正常显示模式 */
    DISPLAY_MODE_MIRROR_X = 1, /**< X轴镜像模式 */
    DISPLAY_MODE_MIRROR_Y = 2, /**< Y轴镜像模式 */
    DISPLAY_MODE_SWAP_XY = 4,  /**< 交换XY轴模式 */
} DisplayMode_e;

/**
 * @enum DisplayPixMode_e
 * @brief 像素模式枚举
 * @details 定义了屏幕像素格式的RGB/BGR顺序
 */
typedef enum DisplayPixMode_
{
    DISPLAY_PIXMODE_RGB = 0, /**< RGB格式 */
    DISPLAY_PIXMODE_BGR = 1, /**< BGR格式 */
} DisplayPixMode_e;

/**
 * @enum DisplayDataFmt_e
 * @brief 显示数据格式枚举
 * @details 定义了支持的像素数据位深度格式
 */
typedef enum DisplayDataFmt_
{
    DISPLAY_DATA_FMT_RGB565 = 0, /**< RGB565格式，16位色 */
    DISPLAY_DATA_FMT_RGB444 = 1, /**< RGB444格式，12位色 */
    DISPLAY_DATA_FMT_RGB666 = 2, /**< RGB666格式，18位色 */
} DisplayDataFmt_e;

typedef enum lcdPreviewModeSel_
{
    CAM_PREVIEW_SET_AUTO = 0,
    CAM_PREVIEW_SET_MANUAL = 1
} lcdPreviewModeSel_e;

typedef struct lcdPreviewManulItem_
{
    uint16_t rowScaleFrac;
    uint16_t colScaleFrac;
    uint16_t tailorLeft;
    uint16_t tailorRight;
    uint16_t tailorTop;
    uint16_t tailorBottom;
} lcdPreviewManulItem_t;

typedef enum lcdRamWrIntSel_
{
    INT_PER_1_FRAME = 0,
    INT_PER_1_2_FRAME = 1,
    INT_PER_1_4_FRAME = 2,
    INT_PER_1_8_FRAME = 3,
    INT_PER_1_16_FRAME = 4,
} lcdRamWrIntSel_e;

typedef struct lcdIoCtrl_
{
    lcdPreviewModeSel_e previewModeSel;
    lcdPreviewManulItem_t previewManulSet;
    uint8_t ramWrIntEn;
    lcdRamWrIntSel_e ramWrIntSel;
} lcdIoCtrl_t;

/**
 * @struct ScrPrevInfo_t
 * @brief 屏幕预览信息结构体
 * @details 存储屏幕预览相关的配置信息
 */
typedef struct ScrPrevInfo_
{
    bool enable;         /**< 是否启用预览 */
    uint32_t img_width;  /**< 图像宽度 */
    uint32_t img_height; /**< 图像高度 */
} ScrPrevInfo_t;

/**
 * @struct ScrRegList_t
 * @brief 屏幕寄存器配置列表结构体
 * @details 存储LCD命令及其参数的结构体
 */
typedef struct ScrRegList_
{
    uint8_t cmd;      /**< 命令码 */
    uint8_t len;      /**< 参数长度 */
    uint8_t data[32]; /**< 命令参数数据 */
} ScrRegList_t;

/**
 * @enum ScreenBkLightMode_e
 * @brief 屏幕背光模式枚举
 * @details 定义了屏幕背光的控制方式
 */
typedef enum ScreenBkLightMode_
{
    SCREEN_BK_LIGHT_MODE_PWM = 0, /**< PWM控制模式 */
    SCREEN_BK_LIGHT_MODE_GPIO,    /**< GPIO控制模式 */
} ScreenBkLightMode_e;

/**
 * @struct ScrBkLightPwmCfg_t
 * @brief 屏幕背光PWM配置结构体
 * @details 存储PWM控制背光的相关参数
 */
typedef struct ScrBkLightPwmCfg_
{
    uint8_t pwm;         /**< PWM输出标号 */
    uint8_t pad;         /**< PWM输出PAD号 */
    uint32_t timer_port; /**< PWM定时器端口 */
} ScrBkLightPwmCfg_t;

/**
 * @enum MspiDataLane_e
 * @brief MSPI 数据总线个数枚举
 * @details 定义了MSPI 数据总线枚举
 */
typedef enum MspiDataLane_
{
    MSPI_DATA_LANE_1 = 0, /**< 1个数据总线 */
    MSPI_DATA_LANE_2 = 1, /**< 2个数据总线 */
    MSPI_DATA_LANE_4 = 2, /**< 1个数据总线 */
    MSPI_DATA_LANE_8 = 3, /**< 2个数据总线 */
    MSPI_DATA_LANE_MAX,
} MspiDataLane_e;

/**
 * @struct ScrBkLightGpioCfg_t
 * @brief 屏幕背光GPIO配置结构体
 * @details 存储GPIO控制背光的相关参数
 */
typedef struct ScrBkLightGpioCfg_
{
    uint8_t gpio_num; /**< GPIO标号 */
} ScrBkLightGpioCfg_t;

/**
 * @struct ScrConfig_t
 * @brief 屏幕配置结构体
 * @details 存储屏幕的完整配置信息
 */
typedef struct ScrConfig_
{
    ScreenType_e type;                 /**< 屏幕类型 */
    ScrlspiInstance_e drv_id;          /**< 驱动实例ID */
    lcdInterfaceType_e int_type;       /**< 接口类型 */
    ScreenBkLightMode_e bk_light_mode; /**< 背光控制模式 */
    ScrBkLightPwmCfg_t pwm_cfg;        /**< PWM背光配置 */
    ScrBkLightGpioCfg_t gpio_cfg;      /**< GPIO背光配置 */
    uint8_t reset_io_num;              /**< 复位IO号 */
    uint8_t reset_level;               /**< 复位电平 */
    uint8_t init_backlight_level;      /**< 初始背光级别 */
    uint32_t freq;                     /**< 通信频率 */
    uint32_t bpp;                      /**< 每像素位数 */
    uint32_t data_lane_num;            /**< 数据通道数量 */
    uint32_t width;                    /**< 屏幕宽度 */
    uint32_t height;                   /**< 屏幕高度 */
    uint32_t x_offset;                 /**< X轴偏移量 */
    uint32_t y_offset;                 /**< Y轴偏移量 */
    uint32_t te_circle;                /**< TE信号周期 */
    uint32_t te_wait_time;             /**< TE信号等待时间 */
    int ext_pwr_io;                 /**< 外部电源IO号 */
    uint8_t display_mode;             /**< 显示模式 */
    uint8_t pix_mode;         /**< 显示像素模式 */
} ScrConfig_t;

/**
 * @struct LcdDrvObj_t
 * @brief LCD驱动对象结构体
 * @details 定义了LCD驱动对象，包含屏幕属性和操作函数
 */
typedef struct LcdDrvObj_
{
    uint32_t id;                 /**< 驱动ID */
    ScreenType_e type;           /**< 屏幕类型 */
    uint16_t x_offset;           /**< X轴偏移量 */
    uint16_t y_offset;           /**< Y轴偏移量 */
    uint16_t width;              /**< 屏幕宽度 */
    uint16_t height;             /**< 屏幕高度 */
    uint8_t bpp;                 /**< 每像素位数 */
    uint8_t data_lane_num;       /**< 数据通道数量 */
    ScrRegList_t *init_reg_list; /**< 初始化寄存器列表 */
    uint32_t init_reg_list_len;  /**< 初始化寄存器列表长度 */
    ScrConfig_t *default_cfg;    /**< 默认配置 */
    uint32_t (*set_window)(uint16_t sx, uint16_t ex, uint16_t width,
                           uint16_t height);    /**< 设置窗口函数 */
    int (*set_direction)(DisDirection_e dir);   /**< 设置显示方向函数 */
    int (*set_display_mode)(DisplayMode_e dir); /**< 设置显示模式函数 */
    int (*set_display_pix_mode)(DisplayPixMode_e mode); /**< 设置像素模式函数 */
    int (*set_display_data_fmt)(DisplayDataFmt_e fmt); /**< 设置数据格式函数 */
    int (*set_backlight)(uint8_t level); /**< 设置背光函数 */
    int (*suspend)(void);                /**< 挂起函数 */
    int (*resume)(void);                 /**< 恢复函数 */
} LcdDrvObj_t;

/**
 * @struct dma_data_t
 * @brief DMA数据传输结构体
 * @details 存储DMA传输相关的配置和数据
 */
typedef struct dma_data_t
{
    DmaTransferConfig_t dmaTrans; /**< DMA传输配置 */
    struct dma_data_t *next;      /**< 指向下一个DMA数据块 */
    uint32_t totalBytes;          /**< 总字节数 */
    uint32_t dataLen;             /**< LSPI传输数据长度，小于0x3FFFF */
    uint16_t cmds[16];            /**< 命令数组 */
    void *data;                   /**< 数据指针 */
} dma_data_t;

/**
 * @typedef ScrDmaEventCb
 * @brief 屏幕DMA事件回调函数类型
 * @param event 事件类型
 */
typedef void (*ScrDmaEventCb)(uint32_t event);

/**
 * @typedef ScrUspEventCb
 * @brief 屏幕USP事件回调函数类型
 */
typedef void (*ScrUspEventCb)();

/**
 * @brief 打开屏幕设备
 * @param drv_id 驱动ID
 * @param cfg 屏幕配置结构体
 * @param dma_cb DMA事件回调函数
 * @param usp_cb USP事件回调函数
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_open(uint32_t drv_id, ScrConfig_t *cfg, ScrDmaEventCb dma_cb,
                    ScrUspEventCb usp_cb);

/**
 * @brief 关闭屏幕设备
 * @param drv_id 驱动ID
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_close(uint32_t drv_id);

/**
 * @brief 设置屏幕背光级别
 * @param drv_id 驱动ID
 * @param level 背光级别(0-100)
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_set_backlight(uint32_t drv_id, uint8_t level);

/**
 * @brief 获取当前屏幕背光级别
 * @param drv_id 驱动ID
 * @param level 返回的背光级别指针
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_get_backlight(uint32_t drv_id, uint8_t *level);

/**
 * @brief 设置屏幕显示方向
 * @param drv_id 驱动ID
 * @param direction 显示方向
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_set_direction(uint32_t drv_id, DisDirection_e direction);

/**
 * @brief 重置屏幕并清除FIFO
 * @param drv_id 驱动ID
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_rst_and_clear_fifo(uint32_t drv_id);

/**
 * @brief 设置摄像头预览
 * @param drv_id 驱动ID
 * @param prev_info 预览信息结构体
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_cam_preview(uint32_t drv_id, ScrPrevInfo_t *prev_info);

/**
 * @brief 设置屏幕刷新方向
 * @param drv_id 驱动ID
 * @param direction 显示方向
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_set_refresh_dir(uint32_t drv_id, DisDirection_e direction);

/**
 * @brief 设置屏幕显示模式
 * @param drv_id 驱动ID
 * @param mode 显示模式
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_set_display_mode(uint32_t drv_id, DisplayMode_e mode);

/**
 * @brief 设置屏幕像素模式
 * @param drv_id 驱动ID
 * @param mode 像素模式
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_set_display_pix_mode(uint32_t drv_id, DisplayPixMode_e mode);

/**
 * @brief 获取屏幕默认配置
 * @param type 屏幕类型
 * @return 返回对应类型的默认配置结构体指针
 */
ScrConfig_t *hal_scr_get_default_config(ScreenType_e type);

/**
 * @brief 挂起屏幕显示
 * @param drv_id 驱动ID
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_suspend(uint32_t drv_id);

/**
 * @brief 恢复屏幕显示
 * @param drv_id 驱动ID
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_resume(uint32_t drv_id);

/**
 * @brief 设置屏幕刷新区域
 * @param drv_id 驱动ID
 * @param start_x 起始X坐标
 * @param start_y 起始Y坐标
 * @param width 宽度
 * @param height 高度
 * @return 返回需要传输的数据总字节数
 */
uint32_t hal_screen_set_refresh_loc(uint32_t drv_id, uint32_t start_x,
                                    uint32_t start_y, uint32_t width,
                                    uint32_t height);

/**
 * @brief 刷新屏幕显示
 * @param drv_id 驱动ID
 * @param data 图像数据指针
 * @param size 数据大小
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_refresh(uint32_t drv_id, uint8_t *data, uint32_t size);

/**
 * @brief 在屏幕上绘制单个像素点
 * @param drv_id 驱动ID
 * @param x X坐标
 * @param y Y坐标
 * @param color 像素颜色
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_draw_piont(uint32_t drv_id, uint32_t x, uint32_t y,
                          uint32_t color);

/**
 * @brief 向屏幕发送命令
 * @param cmd 命令码
 * @param data 命令数据指针
 * @param len 数据长度
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_send_cmd(uint8_t cmd, uint8_t *data, uint32_t len);

/**
 * @brief 配置MSPI接口的寄存器
 * @param enable 是否使能MSPI
 * @param addr_lane 地址线数量
 * @param data_lane 数据线数量
 * @param inst 指令字节
 * @return 成功返回0，失败返回错误码
 */
int hal_screen_set_mspi(bool enable, MspiDataLane_e addr_lane,
                        MspiDataLane_e data_lane, uint8_t inst);

#endif