
#ifndef _SENSOR_TYPE_H_
#define _SENSOR_TYPE_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 16bit sensor id list
 * 
 */

typedef enum {
    SENSOR_BME280_ID = 1, 
    SENSOR_BME680_ID ,          /*!< bme680 sensor id*/
    SENSOR_SHT3X_ID ,           /*!< sht3x sensor id*/
    SENSOR_HTS221_ID ,          /*!< hts221 sensor id*/
    SENSOR_BH1750_ID ,          /*!< bh1750 sensor id*/
    SENSOR_VEML6040_ID ,        /*!< veml6040 sensor id*/
    SENSOR_VEML6075_ID ,        /*!< veml6075 sensor id*/
    SENSOR_MPU6050_ID ,         /*!< mpu6050 sensor id*/
    SENSOR_MPU9250_ID ,         /*!< mpu9250 sensor id*/
    SENSOR_LIS2DH12_ID ,        /*!< lis2dh12 sensor id*/
} sensor_id_t;

#ifdef __cplusplus
}
#endif
#endif
