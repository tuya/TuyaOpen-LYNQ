#ifndef __MPL_EC718_H__
#define __MPL_EC718_H__

#include "mltypes.h"
#include "sensor_mpu.h"

/* Private typedef -----------------------------------------------------------*/
/* Data read from MPL. */
#define PRINT_ACCEL             (0x01)
#define PRINT_GYRO              (0x02)
#define PRINT_QUAT              (0x04)
#define PRINT_COMPASS           (0x08)
#define PRINT_EULER             (0x10)
#define PRINT_ROT_MAT           (0x20)
#define PRINT_HEADING           (0x40)
#define PRINT_PEDO              (0x80)
#define PRINT_LINEAR_ACCEL      (0x100)
#define PRINT_GRAVITY_VECTOR    (0x200)


#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)
#define COMPASS_ON      (0x04)

#define MOTION          (0)
#define NO_MOTION       (1)

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (5)     //1.25-1000

#define FLASH_SIZE      (512)
#define FLASH_MEM_START ((void*)0x1800)

#define PEDO_READ_MS    (1000)
#define TEMP_READ_MS    (500)
#define COMPASS_READ_MS (100)
struct rx_s {
    unsigned char header[3];
    unsigned char cmd;
};
struct hal_s {
    unsigned char lp_accel_mode;
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned char motion_int_mode;
    unsigned long no_dmp_hz;
    unsigned long next_pedo_ms;
    unsigned long next_temp_ms;
    unsigned long next_compass_ms;
    unsigned int report;
    unsigned short dmp_features;
    struct rx_s rx;
};


/* Platform-specific information. Kinda like a boardfile. */
struct platform_data_s {
    signed char orientation[9];
};

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 * TODO: The following matrices refer to the configuration on internal test
 * boards at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static struct platform_data_s gyro_pdata = {
    .orientation = { 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1}
};

#if defined MPU9150 || defined MPU9250
static struct platform_data_s compass_pdata = {
    .orientation = { 0, 1, 0,
                     1, 0, 0,
                     0, 0, -1}
};
#define COMPASS_ENABLED 1
#elif defined AK8975_SECONDARY
static struct platform_data_s compass_pdata = {
    .orientation = {-1, 0, 0,
                     0, 1, 0,
                     0, 0,-1}
};
#define COMPASS_ENABLED 1
#elif defined AK8963_SECONDARY
static struct platform_data_s compass_pdata = {
    .orientation = {-1, 0, 0,
                     0,-1, 0,
                     0, 0, 1}
};
#define COMPASS_ENABLED 1
#endif


typedef enum {
    PACKET_DATA_ACCEL = 0,
    PACKET_DATA_GYRO,
    PACKET_DATA_COMPASS,
    PACKET_DATA_QUAT,
    PACKET_DATA_EULER,
    PACKET_DATA_ROT,
    PACKET_DATA_HEADING,
    PACKET_DATA_LINEAR_ACCEL,
    NUM_DATA_PACKETS
} eMPL_packet_e;

/**
 *  @brief      Send a quaternion packet via UART.
 *  The host is expected to use the data in this packet to graphically
 *  represent the device orientation. To send quaternion in the same manner
 *  as any other data packet, use eMPL_send_data.
 *  @param[in]  quat    Quaternion data.
 */
extern void eMPL_send_quat(long *quat);
extern void eMPL_send_data(unsigned char type, long *data);
extern void handle_input(char cmd);

void ec_delay(unsigned long nTime);
int ec_ticks(unsigned long *count);

int mpl_data_loop(long *temperature);
void read_from_mpl(imu_data_t *data);
void setup_gyro(void);

void gyro_data_ready_cb(void);

#endif /* __PACKET_H__ */

/**
 * @}
 */
