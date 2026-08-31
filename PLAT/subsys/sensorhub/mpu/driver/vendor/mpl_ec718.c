#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mpl_ec718.h"
#include "bsp.h"
#include "cmsis_os2.h"

#include "eMPL_outputs.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "invensense.h"
#include "invensense_adv.h"
#include "log.h"
#include "mltypes.h"
#include "mpl.h"
#include "mpu.h"
#include "mpl_ec718.h"

unsigned char *mpl_key = (unsigned char *)"eMPL 5.1";
volatile uint32_t hal_timestamp = 0;
struct hal_s hal = {0};

/* Every time new gyro data is available, this function is called in an
 * ISR context. In this example, it sets a flag protecting the FIFO read
 * function.
 */
void gyro_data_ready_cb(void)
{
    hal.new_gyro = 1;
}

static void tap_cb(unsigned char direction, unsigned char count)
{
    // ECOMM_TRACE(UNILOG_SENSOR, MPU_TAP, P_VALUE, 2, "dir 0x%X,cnt %d",direction,count);
    switch (direction)
    {
    case TAP_X_UP:
        MPL_LOGI("Tap X+ ");
        break;
    case TAP_X_DOWN:
        MPL_LOGI("Tap X- ");
        break;
    case TAP_Y_UP:
        MPL_LOGI("Tap Y+ ");
        break;
    case TAP_Y_DOWN:
        MPL_LOGI("Tap Y- ");
        break;
    case TAP_Z_UP:
        MPL_LOGI("Tap Z+ ");
        break;
    case TAP_Z_DOWN:
        MPL_LOGI("Tap Z- ");
        break;
    default:
        return;
    }
    MPL_LOGI("x%d\r\n", count);
    return;
}

static void android_orient_cb(unsigned char orientation)
{
    // ECOMM_TRACE(UNILOG_SENSOR, MPU_ORIENT, P_VALUE, 1, "0x%X",orientation);
    switch (orientation)
    {
    case ANDROID_ORIENT_PORTRAIT:
        MPL_LOGI("Portrait\n");
        break;
    case ANDROID_ORIENT_LANDSCAPE:
        MPL_LOGI("Landscape\n");
        break;
    case ANDROID_ORIENT_REVERSE_PORTRAIT:
        MPL_LOGI("Reverse Portrait\n");
        break;
    case ANDROID_ORIENT_REVERSE_LANDSCAPE:
        MPL_LOGI("Reverse Landscape\n");
        break;
    default:
        return;
    }
}

void ec_delay(unsigned long nTime)
{
    osDelay(nTime);
}

int ec_ticks(unsigned long *count)
{
    count[0] = osKernelGetTickCount();
    return 0;
}

/* Handle sensor on/off combinations. */
void setup_gyro(void)
{
    unsigned char mask = 0, lp_accel_was_on = 0;
    if (hal.sensors & ACCEL_ON)
        mask |= INV_XYZ_ACCEL;
    if (hal.sensors & GYRO_ON)
    {
        mask |= INV_XYZ_GYRO;
        lp_accel_was_on |= hal.lp_accel_mode;
    }
#ifdef COMPASS_ENABLED
    if (hal.sensors & COMPASS_ON)
    {
        mask |= INV_XYZ_COMPASS;
        lp_accel_was_on |= hal.lp_accel_mode;
    }
#endif
    /* If you need a power transition, this function should be called with a
     * mask of the sensors still enabled. The driver turns off any sensors
     * excluded from this mask.
     */
    mpu_set_sensors(mask);
    mpu_configure_fifo(mask);
    if (lp_accel_was_on)
    {
        unsigned short rate;
        hal.lp_accel_mode = 0;
        /* Switching out of LP accel, notify MPL of new accel sampling rate. */
        mpu_get_sample_rate(&rate);
        inv_set_accel_sample_rate(1000000L / rate);
    }
}

/* Get data from MPL.
 * TODO: Add return values to the inv_get_sensor_type_xxx APIs to differentiate
 * between new and stale data.
 */
void read_from_mpl(imu_data_t *data)
{
    long msg;
    int8_t accuracy;
    unsigned long timestamp;
    long buff[3] = {0};
    float float_data[3] = {0};
    if (inv_get_sensor_type_quat(buff, &accuracy, (inv_time_t *)&timestamp))
    {
        /* Sends a quaternion packet to the PC. Since this is used by the Python
         * test app to visually represent a 3D quaternion, it's sent each time
         * the MPL has new data.
         */
        // eMPL_send_quat(buff);

        /* Specific data packets can be sent or suppressed using USB commands. */
        if (hal.report & PRINT_QUAT)
            eMPL_send_data(PACKET_DATA_QUAT, buff);
    }

    if (hal.report & PRINT_ACCEL)
    {
        if (inv_get_sensor_type_accel(buff, &accuracy, (inv_time_t *)&timestamp))
        {
            eMPL_send_data(PACKET_DATA_ACCEL, buff);
        }
    }
    if (hal.report & PRINT_GYRO)
    {
        if (inv_get_sensor_type_gyro(buff, &accuracy, (inv_time_t *)&timestamp))
            eMPL_send_data(PACKET_DATA_GYRO, buff);
    }
#ifdef COMPASS_ENABLED
    if (hal.report & PRINT_COMPASS)
    {
        if (inv_get_sensor_type_compass(data, &accuracy,
                                        (inv_time_t *)&timestamp))
            eMPL_send_data(PACKET_DATA_COMPASS, data);
    }
#endif
    if (hal.report & PRINT_EULER)
    {
        if (inv_get_sensor_type_euler(data->euler, &data->accuracy, (inv_time_t *)&timestamp))
        {
            // data->eulerX = buff[0];
            // data->eulerY = buff[1];
            // data->eulerZ = buff[2];
            // eMPL_send_data(PACKET_DATA_EULER, buff);
        }
    }
    if (hal.report & PRINT_ROT_MAT)
    {
        if (inv_get_sensor_type_rot_mat(buff, &accuracy, (inv_time_t *)&timestamp))
            eMPL_send_data(PACKET_DATA_ROT, buff);
    }
    if (hal.report & PRINT_HEADING)
    {
        if (inv_get_sensor_type_heading(buff, &accuracy, (inv_time_t *)&timestamp))
            eMPL_send_data(PACKET_DATA_HEADING, buff);
    }
    if (hal.report & PRINT_LINEAR_ACCEL)
    {
        if (inv_get_sensor_type_linear_acceleration(float_data, &accuracy, (inv_time_t *)&timestamp))
        {
            // MPL_LOGI("\r\nLinear Accel: %7.5f %7.5f %7.5f",float_data[0], float_data[1], float_data[2]);
        }
    }
    if (hal.report & PRINT_GRAVITY_VECTOR)
    {
        if (inv_get_sensor_type_gravity(float_data, &accuracy, (inv_time_t *)&timestamp))
        {
            // MPL_LOGI("\r\nGravity Vector: %7.5f %7.5f %7.5f",float_data[0], float_data[1], float_data[2]);
        }
    }
    if (hal.report & PRINT_PEDO)
    {
        unsigned long timestamp;
        ec_ticks(&timestamp);
        if (timestamp > hal.next_pedo_ms)
        {
            hal.next_pedo_ms = timestamp + PEDO_READ_MS;
            unsigned long step_count, walk_time;
            dmp_get_pedometer_step_count(&step_count);
            dmp_get_pedometer_walk_time(&walk_time);
            // MPL_LOGI("\r\nWalked %ld steps over %ld milliseconds..", step_count,walk_time);
        }
    }

    /* Whenever the MPL detects a change in motion state, the application can
     * be notified. For this example, we use an LED to represent the current
     * motion state.
     */
    msg = inv_get_message_level_0(INV_MSG_MOTION_EVENT | INV_MSG_NO_MOTION_EVENT);
    if (msg)
    {
        if (msg & INV_MSG_MOTION_EVENT)
        {
            MPL_LOGI("Motion!\n");
        }
        else if (msg & INV_MSG_NO_MOTION_EVENT)
        {
            MPL_LOGI("No motion!\n");
        }
    }
    /* These commands turn off individual sensors. */
    // hal.sensors ^= GYRO_ON;
    // hal.sensors ^= ACCEL_ON;
    // setup_gyro();
}

void sensor_mpu_config(void)
{
    unsigned short gyro_rate = 0, gyro_fsr = 0;
    static char accel_fsr = 0;
#ifdef COMPASS_ENABLED
    static unsigned short compass_fsr;
#endif
    /* Compute 6-axis and 9-axis quaternions. */
    inv_enable_quaternion();
    inv_enable_9x_sensor_fusion();
    /* The MPL expects compass data at a constant rate (matching the rate
     * passed to inv_set_compass_sample_rate). If this is an issue for your
     * application, call this function, and the MPL will depend on the
     * timestamps passed to inv_build_compass instead.
     *
     * inv_9x_fusion_use_timestamps(1);
     */

    /* Update gyro biases when not in motion.
     * WARNING: These algorithms are mutually exclusive.
     */
    inv_enable_fast_nomot();
    /* inv_enable_motion_no_motion(); */
    /* inv_set_no_motion_time(1000); */

    /* Update gyro biases when temperature changes. */
    inv_enable_gyro_tc();

    /* This algorithm updates the accel biases when in motion. A more accurate
     * bias measurement can be made when running the self-test (see case 't' in
     * handle_input), but this algorithm can be enabled if the self-test can't
     * be executed in your application.
     *
     * inv_enable_in_use_auto_calibration();
     */
#ifdef COMPASS_ENABLED
    /* Compass calibration algorithms. */
    inv_enable_vector_compass_cal();
    inv_enable_magnetic_disturbance();
#endif
    /* If you need to estimate your heading before the compass is calibrated,
     * enable this algorithm. It becomes useless after a good figure-eight is
     * detected, so we'll just leave it out to save memory.
     * inv_enable_heading_from_gyro();
     */
    /* Allows use of the MPL APIs in read_from_mpl. */
    inv_enable_eMPL_outputs();

    inv_error_t result = inv_start_mpl();
    if (result == INV_ERROR_NOT_AUTHORIZED)
    {
        MPL_LOGI("Not authorized.\n");
        return;
        // while (1) {
        //     // MPL_LOGI("Not authorized.\n");
        // }
    }
    if (result)
    {
        MPL_LOGI("Could not start the MPL.\n");
    }
    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
#ifdef COMPASS_ENABLED
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
#else
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#endif
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(DEFAULT_MPU_HZ);
#ifdef COMPASS_ENABLED
    /* The compass sampling rate can be less than the gyro/accel sampling rate.
     * Use this function for proper power management.
     */
    mpu_set_compass_sample_rate(1000 / COMPASS_READ_MS);
#endif
    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
#ifdef COMPASS_ENABLED
    mpu_get_compass_fsr(&compass_fsr);
#endif
    // MPL_LOGI("\r\ngyro_rate:%d,gyro_fsr:%d,accel_fsr:%d\r\n",gyro_rate,gyro_fsr,accel_fsr);
    /* Sync driver configuration with MPL. */
    /* Sample rate expected in microseconds. */
    inv_set_gyro_sample_rate(1000000L / gyro_rate);
    inv_set_accel_sample_rate(1000000L / gyro_rate);
#ifdef COMPASS_ENABLED
    /* The compass rate is independent of the gyro and accel rates. As long as
     * inv_set_compass_sample_rate is called with the correct value, the 9-axis
     * fusion algorithm's compass correction gain will work properly.
     */
    inv_set_compass_sample_rate(COMPASS_READ_MS * 1000L);
#endif
    /* Set chip-to-body orientation matrix.
     * Set hardware units to dps/g's/degrees scaling factor.
     */
    inv_set_gyro_orientation_and_scale(
        inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
        (long)gyro_fsr << 15);
    inv_set_accel_orientation_and_scale(
        inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
        (long)accel_fsr << 15);
#ifdef COMPASS_ENABLED
    inv_set_compass_orientation_and_scale(
        inv_orientation_matrix_to_scalar(compass_pdata.orientation),
        (long)compass_fsr << 15);
#endif
    /* Initialize HAL state variables. */
#ifdef COMPASS_ENABLED
    hal.sensors = ACCEL_ON | GYRO_ON | COMPASS_ON;
#else
    hal.sensors = ACCEL_ON | GYRO_ON;
#endif
    hal.dmp_on = 0;
    hal.report = 0;
    hal.rx.cmd = 0;
    hal.next_temp_ms = 0;
    hal.next_pedo_ms = 0;
    hal.next_compass_ms = 0;
    // hal.report ^= PRINT_GYRO;
    // hal.report ^= PRINT_ACCEL;
    hal.report ^= PRINT_EULER;
    // hal.report ^= PRINT_PEDO;
    // hal.motion_int_mode = 0;
    /* Compass reads are handled by scheduler. */
    // ec_ticks(&timestamp);
    /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_pdata.orientation));
    dmp_register_tap_cb(tap_cb);
    dmp_register_android_orient_cb(android_orient_cb);

    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     *
     * DMP sensor fusion works only with gyro at +-2000dps and accel +-2G
     */
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP | DMP_FEATURE_ANDROID_ORIENT |
                       DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(hal.dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    mpu_set_dmp_state(1);
    /* LP accel is not compatible with the DMP. */
    hal.dmp_on = 1;
}

int read_mp_fifo(char temp, long *temperature)
{
    int new_data = 0;
    unsigned long sensor_timestamp;
    short gyro[3], accel_short[3];
    unsigned char sensors, more;
    long accel[3];
    /* This function gets new data from the FIFO. The FIFO can contain
     * gyro, accel, both, or neither. The sensors parameter tells the
     * caller which data fields were actually populated with new data.
     * For example, if sensors == INV_XYZ_GYRO, then the FIFO isn't
     * being filled with accel data. The more parameter is non-zero if
     * there are leftover packets in the FIFO. The HAL can use this
     * information to increase the frequency at which this function is
     * called.
     */
    hal.new_gyro = 0;
    mpu_read_fifo(gyro, accel_short, &sensor_timestamp, &sensors, &more);
    if (more)
        hal.new_gyro = 1;
    if (sensors & INV_XYZ_GYRO)
    {
        /* Push the new data to the MPL. */
        inv_build_gyro(gyro, sensor_timestamp);
        new_data = 1;
        if (temp)
        {
            /* Temperature only used for gyro temp comp. */
            mpu_get_temperature(temperature, &sensor_timestamp);
            inv_build_temp(*temperature, sensor_timestamp);
        }
    }
    if (sensors & INV_XYZ_ACCEL)
    {
        accel[0] = (long)accel_short[0];
        accel[1] = (long)accel_short[1];
        accel[2] = (long)accel_short[2];
        inv_build_accel(accel, 0, sensor_timestamp);
        new_data = 1;
    }
    // MPL_LOGI("\r\nINV_XYZ_ACCEL:%d %d %d",gyro,temperature,sensor_timestamp);
    return new_data;
}

int dump_mp_fifo(char temp, long *temperature)
{
    int new_data = 0;
    unsigned long sensor_timestamp;
    short gyro[3], accel_short[3], sensors;
    unsigned char more;
    long accel[3], quat[4];
    /* This function gets new data from the FIFO when the DMP is in use.
     * The FIFO can contain any combination of gyro, accel,
     * quaternion, and gesture data. The sensors parameter tells the
     * caller which data fields were actually populated with new data.
     * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
     * the FIFO isn't being filled with accel data.
     * The driver parses the gesture data to determine if a gesture
     * event has occurred; on an event, the application will be notified
     * via a callback (assuming that a callback function was properly
     * registered). The more parameter is non-zero if there are
     * leftover packets in the FIFO.
     */
    dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);
    if (!more)
        hal.new_gyro = 0;
    if (sensors & INV_XYZ_GYRO)
    {
        /* Push the new data to the MPL. */
        inv_build_gyro(gyro, sensor_timestamp);
        new_data = 1;
        if (temp)
        {
            /* Temperature only used for gyro temp comp. */
            mpu_get_temperature(temperature, &sensor_timestamp);
            inv_build_temp(*temperature, sensor_timestamp);
        }
    }
    if (sensors & INV_XYZ_ACCEL)
    {
        accel[0] = (long)accel_short[0];
        accel[1] = (long)accel_short[1];
        accel[2] = (long)accel_short[2];
        inv_build_accel(accel, 0, sensor_timestamp);
        new_data = 1;
    }
    if (sensors & INV_WXYZ_QUAT)
    {
        inv_build_quat(quat, 0, sensor_timestamp);
        new_data = 1;
    }
    return new_data;
}

int mpl_data_loop(long *temperature)
{
    unsigned long sensor_timestamp;
    int new_data = 0;
    unsigned long timestamp;
    ec_ticks(&timestamp);
#ifdef COMPASS_ENABLED
    /* We're not using a data ready interrupt for the compass, so we'll
     * make our compass reads timer-based instead.
     */
    if ((timestamp > hal.next_compass_ms) && !hal.lp_accel_mode && hal.new_gyro && (hal.sensors & COMPASS_ON))
    {
        hal.next_compass_ms = timestamp + COMPASS_READ_MS;
        short compass_short[3];
        long compass[3];
        /* For any MPU device with an AKM on the auxiliary I2C bus, the raw
         * magnetometer registers are copied to special gyro registers.
         */
        if (!mpu_get_compass_reg(compass_short, &sensor_timestamp))
        {
            compass[0] = (long)compass_short[0];
            compass[1] = (long)compass_short[1];
            compass[2] = (long)compass_short[2];
            /* NOTE: If using a third-party compass calibration library,
             * pass in the compass data in uT * 2^16 and set the second
             * parameter to INV_CALIBRATED | acc, where acc is the
             * accuracy from 0 to 3.
             */
            inv_build_compass(compass, 0, sensor_timestamp);
        }
        new_data = 1;
    }
#endif
    // if (hal.motion_int_mode)
    // {
    //     /* Enable motion interrupt. */
    //     mpu_lp_motion_interrupt(500, 1, 5);
    //     /* Notify the MPL that contiguity was broken. */
    //     inv_accel_was_turned_off();
    //     inv_gyro_was_turned_off();
    //     inv_compass_was_turned_off();
    //     inv_quaternion_sensor_was_turned_off();
    //     /* Wait for the MPU interrupt. */
    //     while (!hal.new_gyro) {
    //         osDelay(2);
    //     }
    //     /* Restore the previous sensor configuration. */
    //     mpu_lp_motion_interrupt(0, 0, 0);
    //     hal.motion_int_mode = 0;
    // }
    if (hal.new_gyro)
    {
        hal.new_gyro = 0;
        char new_temp = 0;
        if (timestamp > hal.next_temp_ms)
        {
            hal.next_temp_ms = timestamp + TEMP_READ_MS;
            new_temp = 1;
        }
        if (hal.lp_accel_mode)
        {
            short accel_short[3];
            long accel[3];
            mpu_get_accel_reg(accel_short, &sensor_timestamp);
            accel[0] = (long)accel_short[0];
            accel[1] = (long)accel_short[1];
            accel[2] = (long)accel_short[2];
            inv_build_accel(accel, 0, sensor_timestamp);
            new_data = 1;
        }
        else if (hal.dmp_on)
        {
            new_data = dump_mp_fifo(new_temp, temperature);
        }
        else
        {
            new_data = read_mp_fifo(new_temp, temperature);
        }
    }
    else
    {
    }
    return new_data;
}
