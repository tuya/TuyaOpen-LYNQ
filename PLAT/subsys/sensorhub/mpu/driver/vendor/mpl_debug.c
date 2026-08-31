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

#define BUF_SIZE        (256)
#define PACKET_LENGTH   (23)

#define PACKET_DEBUG    (1)
#define PACKET_QUAT     (2)
#define PACKET_DATA     (3)




/**
 *  @brief      Prints a variable argument log message.
 *  USB output will be formatted as follows:\n
 *  packet[0]       = $\n
 *  packet[1]       = packet type (1: debug, 2: quat, 3: data)\n
 *  packet[2]       = \n for debug packets: log priority\n
 *                    for quaternion packets: unused\n
 *                    for data packets: packet content (accel, gyro, etc)\n
 *  packet[3-20]    = data\n
 *  packet[21]      = \\r\n
 *  packet[22]      = \\n
 *  @param[in]  priority    Log priority (based on Android).
 *  @param[in]  tag         File specific string.
 *  @param[in]  fmt         String of text with optional format tags.
 *
 *  @return     0 if successful.
 */
int _MLPrintLog (int priority, const char* tag, const char* fmt, ...)
{
    va_list args;
    int length, ii, i;
    char buf[BUF_SIZE], out[PACKET_LENGTH], this_length;

    /* This can be modified to exit for unsupported priorities. */
    switch (priority) {
    case MPL_LOG_UNKNOWN:
    case MPL_LOG_DEFAULT:
    case MPL_LOG_VERBOSE:
    case MPL_LOG_DEBUG:
    case MPL_LOG_INFO:
    case MPL_LOG_WARN:
    case MPL_LOG_ERROR:
    case MPL_LOG_SILENT:
        break;
    default:
        return 0;
    }

    va_start(args, fmt);

    length = vsprintf(buf, fmt, args);
    if (length <= 0) {
        va_end(args);
        return length;
    }

    memset(out, 0, PACKET_LENGTH);
    out[0] = '$';
    out[1] = PACKET_DEBUG;
    out[2] = priority;
    out[21] = '\r';
    out[22] = '\n';
    for (ii = 0; ii < length; ii += (PACKET_LENGTH-5)) {
#define min(a,b) ((a < b) ? a : b)
        this_length = min(length-ii, PACKET_LENGTH-5);
        memset(out+3, 0, 18);
        memcpy(out+3, buf+ii, this_length);
        for (i=0; i<PACKET_LENGTH; i++) {
          io_putchar(out[i]);
        }
    }    
    va_end(args);

    return 0;
}


void eMPL_send_data(unsigned char type, long *data)
{
    char out[PACKET_LENGTH];
    if (!data) return;
    memset(out, 0, PACKET_LENGTH);
    out[0] = '$';
    out[1] = PACKET_DATA;
    out[2] = type;
    out[21] = '\r';
    out[22] = '\n';
    switch (type) {
    /* Two bytes per-element. */
    case PACKET_DATA_ROT:
        out[3] = (char)(data[0] >> 24);
        out[4] = (char)(data[0] >> 16);
        out[5] = (char)(data[1] >> 24);
        out[6] = (char)(data[1] >> 16);
        out[7] = (char)(data[2] >> 24);
        out[8] = (char)(data[2] >> 16);
        out[9] = (char)(data[3] >> 24);
        out[10] = (char)(data[3] >> 16);
        out[11] = (char)(data[4] >> 24);
        out[12] = (char)(data[4] >> 16);
        out[13] = (char)(data[5] >> 24);
        out[14] = (char)(data[5] >> 16);
        out[15] = (char)(data[6] >> 24);
        out[16] = (char)(data[6] >> 16);
        out[17] = (char)(data[7] >> 24);
        out[18] = (char)(data[7] >> 16);
        out[19] = (char)(data[8] >> 24);
        out[20] = (char)(data[8] >> 16);
        break;
    /* Four bytes per-element. */
    /* Four elements. */
    case PACKET_DATA_QUAT:
        out[15] = (char)(data[3] >> 24);
        out[16] = (char)(data[3] >> 16);
        out[17] = (char)(data[3] >> 8);
        out[18] = (char)data[3];
    /* Three elements. */
    case PACKET_DATA_ACCEL:
    case PACKET_DATA_GYRO:
    case PACKET_DATA_COMPASS:
    case PACKET_DATA_EULER:
        out[3] = (char)(data[0] >> 24);
        out[4] = (char)(data[0] >> 16);
        out[5] = (char)(data[0] >> 8);
        out[6] = (char)data[0];
        out[7] = (char)(data[1] >> 24);
        out[8] = (char)(data[1] >> 16);
        out[9] = (char)(data[1] >> 8);
        out[10] = (char)data[1];
        out[11] = (char)(data[2] >> 24);
        out[12] = (char)(data[2] >> 16);
        out[13] = (char)(data[2] >> 8);
        out[14] = (char)data[2];
        break;
    case PACKET_DATA_HEADING:
        out[3] = (char)(data[0] >> 24);
        out[4] = (char)(data[0] >> 16);
        out[5] = (char)(data[0] >> 8);
        out[6] = (char)data[0];
        break;
    default:
        return;
    }
    for (int i =0; i<PACKET_LENGTH; i++) {
        io_putchar(out[i]);
    }
}

void eMPL_send_quat(long *quat)
{
    char out[PACKET_LENGTH];
    if (!quat) return;
    memset(out, 0, PACKET_LENGTH);
    out[0] = '$';
    out[1] = PACKET_QUAT;
    out[3] = (char)(quat[0] >> 24);
    out[4] = (char)(quat[0] >> 16);
    out[5] = (char)(quat[0] >> 8);
    out[6] = (char)quat[0];
    out[7] = (char)(quat[1] >> 24);
    out[8] = (char)(quat[1] >> 16);
    out[9] = (char)(quat[1] >> 8);
    out[10] = (char)quat[1];
    out[11] = (char)(quat[2] >> 24);
    out[12] = (char)(quat[2] >> 16);
    out[13] = (char)(quat[2] >> 8);
    out[14] = (char)quat[2];
    out[15] = (char)(quat[3] >> 24);
    out[16] = (char)(quat[3] >> 16);
    out[17] = (char)(quat[3] >> 8);
    out[18] = (char)quat[3];
    out[21] = '\r';
    out[22] = '\n';
    for (int i=0; i<PACKET_LENGTH; i++) {
      io_putchar(out[i]);
    }
}

static inline void run_self_test(void)
{
    int result;
    long gyro[3], accel[3];

#if defined (MPU6500) || defined (MPU9250)
    result = mpu_run_6500_self_test(gyro, accel, 0);
#elif defined (MPU6050) || defined (MPU9150)
    result = mpu_run_self_test(gyro, accel);
#endif
    if (result == 0x7) {
	MPL_LOGI("Passed!\n");
        MPL_LOGI("accel: %7.4f %7.4f %7.4f\n",
                    accel[0]/65536.f,
                    accel[1]/65536.f,
                    accel[2]/65536.f);
        MPL_LOGI("gyro: %7.4f %7.4f %7.4f\n",
                    gyro[0]/65536.f,
                    gyro[1]/65536.f,
                    gyro[2]/65536.f);
        /* Test passed. We can trust the gyro data here, so now we need to update calibrated data*/

#ifdef USE_CAL_HW_REGISTERS
        /*
         * This portion of the code uses the HW offset registers that are in the MPUxxxx devices
         * instead of pushing the cal data to the MPL software library
         */
        unsigned char i = 0;

        for(i = 0; i<3; i++) {
        	gyro[i] = (long)(gyro[i] * 32.8f); //convert to +-1000dps
        	accel[i] *= 2048.f; //convert to +-16G
        	accel[i] = accel[i] >> 16;
        	gyro[i] = (long)(gyro[i] >> 16);
        }

        mpu_set_gyro_bias_reg(gyro);

#if defined (MPU6500) || defined (MPU9250)
        mpu_set_accel_bias_6500_reg(accel);
#elif defined (MPU6050) || defined (MPU9150)
        mpu_set_accel_bias_6050_reg(accel);
#endif
#else
        /* Push the calibrated data to the MPL library.
         *
         * MPL expects biases in hardware units << 16, but self test returns
		 * biases in g's << 16.
		 */
    	unsigned short accel_sens;
    	float gyro_sens;

		mpu_get_accel_sens(&accel_sens);
		accel[0] *= accel_sens;
		accel[1] *= accel_sens;
		accel[2] *= accel_sens;
		inv_set_accel_bias(accel, 3);
		mpu_get_gyro_sens(&gyro_sens);
		gyro[0] = (long) (gyro[0] * gyro_sens);
		gyro[1] = (long) (gyro[1] * gyro_sens);
		gyro[2] = (long) (gyro[2] * gyro_sens);
		inv_set_gyro_bias(gyro, 3);
#endif
    }
    else {
            if (!(result & 0x1))
                MPL_LOGE("Gyro failed.\n");
            if (!(result & 0x2))
                MPL_LOGE("Accel failed.\n");
            if (!(result & 0x4))
                MPL_LOGE("Compass failed.\n");
     }
}

#ifdef COMPASS_ENABLED
void send_status_compass() {
	long data[3] = { 0 };
	int8_t accuracy = { 0 };
	unsigned long timestamp;
	inv_get_compass_set(data, &accuracy, (inv_time_t*) &timestamp);
	MPL_LOGI("Compass: %7.4f %7.4f %7.4f ",
			data[0]/65536.f, data[1]/65536.f, data[2]/65536.f);
	MPL_LOGI("Accuracy= %d\r\n", accuracy);

}
#endif

extern struct hal_s hal;
void handle_input(char c)
{
    // char c = USART_ReceiveData(USART2);
    switch (c) {
    /* These commands turn off individual sensors. */
    case '8':
        hal.sensors ^= ACCEL_ON;
        setup_gyro();
        if (!(hal.sensors & ACCEL_ON))
            inv_accel_was_turned_off();
        break;
    case '9':
        hal.sensors ^= GYRO_ON;
        setup_gyro();
        if (!(hal.sensors & GYRO_ON))
            inv_gyro_was_turned_off();
        break;
#ifdef COMPASS_ENABLED
    case '0':
        hal.sensors ^= COMPASS_ON;
        setup_gyro();
        if (!(hal.sensors & COMPASS_ON))
            inv_compass_was_turned_off();
        break;
#endif
    /* The commands send individual sensor data or fused data to the PC. */
    case 'a':
        hal.report ^= PRINT_ACCEL;
        break;
    case 'g':
        hal.report ^= PRINT_GYRO;
        break;
#ifdef COMPASS_ENABLED
    case 'c':
        hal.report ^= PRINT_COMPASS;
        break;
#endif
    case 'e':
        hal.report ^= PRINT_EULER;
        break;
    case 'r':
        hal.report ^= PRINT_ROT_MAT;
        break;
    case 'q':
        hal.report ^= PRINT_QUAT;
        break;
    case 'h':
        hal.report ^= PRINT_HEADING;
        break;
    case 'i':
        hal.report ^= PRINT_LINEAR_ACCEL;
        break;
    case 'o':
        hal.report ^= PRINT_GRAVITY_VECTOR;
        break;
#ifdef COMPASS_ENABLED
	case 'w':
		send_status_compass();
		break;
#endif
    /* This command prints out the value of each gyro register for debugging.
     * If logging is disabled, this function has no effect.
     */
    case 'd':
        mpu_reg_dump();
        break;
    /* Test out low-power accel mode. */
    case 'p':
        if (hal.dmp_on)
            /* LP accel is not compatible with the DMP. */
            break;
        mpu_lp_accel_mode(20);
        /* When LP accel mode is enabled, the driver automatically configures
         * the hardware for latched interrupts. However, the MCU sometimes
         * misses the rising/falling edge, and the hal.new_gyro flag is never
         * set. To avoid getting locked in this state, we're overriding the
         * driver's configuration and sticking to unlatched interrupt mode.
         *
         * TODO: The MCU supports level-triggered interrupts.
         */
        mpu_set_int_latched(0);
        hal.sensors &= ~(GYRO_ON|COMPASS_ON);
        hal.sensors |= ACCEL_ON;
        hal.lp_accel_mode = 1;
        inv_gyro_was_turned_off();
        inv_compass_was_turned_off();
        break;
    /* The hardware self test can be run without any interaction with the
     * MPL since it's completely localized in the gyro driver. Logging is
     * assumed to be enabled; otherwise, a couple LEDs could probably be used
     * here to display the test results.
     */
    case 't':
        run_self_test();
        /* Let MPL know that contiguity was broken. */
        inv_accel_was_turned_off();
        inv_gyro_was_turned_off();
        inv_compass_was_turned_off();
        break;
    /* Depending on your application, sensor data may be needed at a faster or
     * slower rate. These commands can speed up or slow down the rate at which
     * the sensor data is pushed to the MPL.
     *
     * In this example, the compass rate is never changed.
     */
    case '1':
        if (hal.dmp_on) {
            dmp_set_fifo_rate(10);
            inv_set_quat_sample_rate(100000L);
        } else
            mpu_set_sample_rate(10);
        inv_set_gyro_sample_rate(100000L);
        inv_set_accel_sample_rate(100000L);
        break;
    case '2':
        if (hal.dmp_on) {
            dmp_set_fifo_rate(20);
            inv_set_quat_sample_rate(50000L);
        } else
            mpu_set_sample_rate(20);
        inv_set_gyro_sample_rate(50000L);
        inv_set_accel_sample_rate(50000L);
        break;
    case '3':
        if (hal.dmp_on) {
            dmp_set_fifo_rate(40);
            inv_set_quat_sample_rate(25000L);
        } else
            mpu_set_sample_rate(40);
        inv_set_gyro_sample_rate(25000L);
        inv_set_accel_sample_rate(25000L);
        break;
    case '4':
        if (hal.dmp_on) {
            dmp_set_fifo_rate(50);
            inv_set_quat_sample_rate(20000L);
        } else
            mpu_set_sample_rate(50);
        inv_set_gyro_sample_rate(20000L);
        inv_set_accel_sample_rate(20000L);
        break;
    case '5':
        if (hal.dmp_on) {
            dmp_set_fifo_rate(100);
            inv_set_quat_sample_rate(10000L);
        } else
            mpu_set_sample_rate(100);
        inv_set_gyro_sample_rate(10000L);
        inv_set_accel_sample_rate(10000L);
        break;
	case ',':
        /* Set hardware to interrupt on gesture event only. This feature is
         * useful for keeping the MCU asleep until the DMP detects as a tap or
         * orientation event.
         */
        dmp_set_interrupt_mode(DMP_INT_GESTURE);
        break;
    case '.':
        /* Set hardware to interrupt periodically. */
        dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);
        break;
    case '6':
        /* Toggle pedometer display. */
        hal.report ^= PRINT_PEDO;
        break;
    case '7':
        /* Reset pedometer. */
        dmp_set_pedometer_step_count(0);
        dmp_set_pedometer_walk_time(0);
        break;
    case 'f':
        if (hal.lp_accel_mode)
            /* LP accel is not compatible with the DMP. */
            return;
        /* Toggle DMP. */
        if (hal.dmp_on) {
            unsigned short dmp_rate;
            unsigned char mask = 0;
            hal.dmp_on = 0;
            mpu_set_dmp_state(0);
            /* Restore FIFO settings. */
            if (hal.sensors & ACCEL_ON)
                mask |= INV_XYZ_ACCEL;
            if (hal.sensors & GYRO_ON)
                mask |= INV_XYZ_GYRO;
            if (hal.sensors & COMPASS_ON)
                mask |= INV_XYZ_COMPASS;
            mpu_configure_fifo(mask);
            /* When the DMP is used, the hardware sampling rate is fixed at
             * 200Hz, and the DMP is configured to downsample the FIFO output
             * using the function dmp_set_fifo_rate. However, when the DMP is
             * turned off, the sampling rate remains at 200Hz. This could be
             * handled in inv_mpu.c, but it would need to know that
             * inv_mpu_dmp_motion_driver.c exists. To avoid this, we'll just
             * put the extra logic in the application layer.
             */
            dmp_get_fifo_rate(&dmp_rate);
            mpu_set_sample_rate(dmp_rate);
            inv_quaternion_sensor_was_turned_off();
            MPL_LOGI("DMP disabled.\n");
        } else {
            unsigned short sample_rate;
            hal.dmp_on = 1;
            /* Preserve current FIFO rate. */
            mpu_get_sample_rate(&sample_rate);
            dmp_set_fifo_rate(sample_rate);
            inv_set_quat_sample_rate(1000000L / sample_rate);
            mpu_set_dmp_state(1);
            MPL_LOGI("DMP enabled.\n");
        }
        break;
    case 'm':
        /* Test the motion interrupt hardware feature. */
	#ifndef MPU6050 // not enabled for 6050 product
	hal.motion_int_mode = 1;
	#endif 
        break;

    case 'v':
        /* Toggle LP quaternion.
         * The DMP features can be enabled/disabled at runtime. Use this same
         * approach for other features.
         */
        hal.dmp_features ^= DMP_FEATURE_6X_LP_QUAT;
        dmp_enable_feature(hal.dmp_features);
        if (!(hal.dmp_features & DMP_FEATURE_6X_LP_QUAT)) {
            inv_quaternion_sensor_was_turned_off();
            MPL_LOGI("LP quaternion disabled.\n");
        } else
            MPL_LOGI("LP quaternion enabled.\n");
        break;
    default:
        break;
    }
    hal.rx.cmd = 0;
}



/**
 * @}
**/


