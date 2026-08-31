#ifndef _CST816_FUNC_H
#define _CST816_FUNC_H

#include <stdbool.h>
#include <stdint.h>
#include "api_tp.h"

#define MAX_POINTS_REPORT (5)
#define BOOT_I2C_ADDR (0x6A)
#define MAIN_I2C_ADDR (0x15)
#define CST816_BIN_SIZE (15 * 1024)
#define CUSTOM_SENSOR_NUM (10)
#define RW_REG_LEN (2)
#define TOTAL_FW_NUM (2)
typedef enum GesIdx_
{
    IDX_U = 0,
    IDX_UP,
    IDX_DOWN,
    IDX_LEFT,
    IDX_RIGHT,
    IDX_O,
    IDX_e,
    IDX_M,
    IDX_L,
    IDX_W,
    IDX_S,
    IDX_V,
    IDX_C,
    IDX_Z,
    IDX_POWER,
    IDX_NULL = 0xFF,
} GesIdx_e;

typedef enum
{
    REPORT_NONE = 0,
    REPORT_POS = 0x01,
    REPORT_KEY = 0x02,
    REPORT_GES = 0x04,
    REPORT_PROX = 0x08
} ReportType_e;

typedef struct
{
    int32_t reset_gpio;
    int32_t irq_gpio;

    uint32_t x_resolution;
    uint32_t y_resolution;
    int32_t swap_xy;
    int32_t reverse_x;
    int32_t reverse_y;
    uint32_t output_x_resolution;
    uint32_t output_y_resolution;
    int32_t max_touch_num;
    int32_t key_num;
    int32_t key_x_coords[8];  // max support 8 keys
    int32_t key_y_coords;
    int32_t key_code[8];
} PlatData_t;

typedef struct
{
    uint8_t fw_sensor_txnum;
    uint8_t fw_sensor_rxnum;
    uint8_t fw_key_num;
    uint8_t reserve;
    uint16_t fw_res_y;
    uint16_t fw_res_x;
    uint32_t fw_boot_time;
    uint32_t fw_project_id;
    uint32_t fw_chip_type;
    uint32_t fw_ver;
    uint32_t ic_fw_checksum;
    uint32_t fw_module_id;
} TpInfo_t;

typedef struct
{
    uint8_t rep_num;
    ReportType_e report_need;
    uint8_t key_id;
    uint8_t key_state;
    struct
    {
        uint8_t pos_id;
        uint8_t event;
        uint16_t pos_x;
        uint16_t pos_y;
        uint16_t pres_z;
    } pos_info[MAX_POINTS_REPORT];
} TsFrame_t;


typedef enum
{
    NOMAL_MODE = 0,
    GESTURE_MODE = 1,
    LP_MODE = 2,
    DEEPSLEEP = 3,
    DIFF_MODE = 4,
    RAWDATA_MODE = 5,
    BASELINE_MODE = 6,
    CALIBRATE_MODE = 7,
    FAC_TEST_MODE = 8,
    ENTER_BOOT_MODE = 0xCA,
} WorkMode_e;

typedef struct TpDev_
{
    uint16_t bus_type;
    uint8_t slave_addr;
    uint32_t i2c_id;
    int32_t gpio_irq;
    int32_t esd_fail_cnt;
    uint32_t esd_last_value;
    WorkMode_e work_mode;

    int32_t power_is_on;
    uint8_t hyn_irq_flg;
    PlatData_t plat_data;
    TpInfo_t hw_info;
    TsFrame_t rp_buf;

    int32_t boot_is_pass;
    int32_t need_update_fw;
    uint8_t fw_file_name[128];
    uint8_t *fw_updata_addr;
    int32_t fw_updata_len;
    int32_t fw_dump_state;
    uint8_t fw_updata_process;
    uint8_t host_cmd_save[16];

    uint8_t log_level;
    uint8_t prox_is_enable;
    uint8_t prox_state;

    uint8_t gesture_is_enable;
    uint8_t gesture_id;
    const TsFunc_t *hyn_fuc_used;
} TpDevCst816_t;

int cst816_dev_init(uint8_t port_id, uint32_t speed, uint8_t fw_id,
                         void (*tp_bus_cb_func)(uint32_t event));
TpDevCst816_t *cst816_dev_get(void);

#endif  //_CST816_FUNC_H