#ifndef _OL_I2C_API_H_
#define _OL_I2C_API_H_


typedef enum
{
    OL_I2C_INDEX_0 = 0,
    OL_I2C_INDEX_1 = 1,
} ol_i2c_index_num;

typedef enum
{
    OL_I2C_SPEED_STANDARD  = 0x1,  //< Standard Speed (100kHz)
    OL_I2C_SPEED_FAST      = 0x2,  //< Fast Speed     (400kHz)
    OL_I2C_SPEED_FAST_PLUS = 0x3,  //< Fast+ Speed    (  1MHz)
    OL_I2C_SPEED_HIGH      = 0x4,  //< High Speed     (3.4MHz)
} ol_i2c_speed;

typedef void (*ol_i2c_event_cb_f)(uint32_t event);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to initialize the i2c
 * PARAMETERS
 *      id       [IN]: i2c index, enum ol_i2c_index_num
 *      speed    [IN]: i2c speed, enum ol_i2c_speed
 *      callback [IN]: when the mode is not POLLING_MODE, it needs to be set; This function is used to report the result event of i2c
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_i2c_init(ol_i2c_index_num id,ol_i2c_speed speed,ol_i2c_event_cb_f callback);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to write
 * PARAMETERS
 *      id         [IN] : i2c index, enum ol_i2c_index_num
 *      slave_addr [IN] : address of the slave device
 *      cmd        [IN] : register address
 *      cmdlen     [IN] : register address length(eg:0x89 length is 1;0x8970 length is 2)
 *      data       [IN] : data buffer
 *      datalen    [IN] : data buffer length
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 * 
 *****************************************************************************/
extern int ol_i2c_write(ol_i2c_index_num id, uint32_t slave_addr, uint8_t *cmd, uint32_t cmdlen, uint8_t *data, uint32_t datalen);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to read
 * PARAMETERS
 *      id         [IN] : i2c index, enum ol_i2c_index_num
 *      slave_addr [IN] : address of the slave device
 *      cmd        [IN] : register address
 *      cmdlen     [IN] : register address length(eg:0x89 length is 1;0x8970 length is 2)
 *      data       [IN] : data buffer
 *      datalen    [IN] : data buffer length
 * RETURN VALUES
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_i2c_read(ol_i2c_index_num id, uint32_t slave_addr, uint8_t *cmd, uint32_t cmdlen, uint8_t *data, uint32_t datalen);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to deinitialize the i2c
 * PARAMETERS
 *      id [IN] : i2c index, enum ol_i2c_index_num
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_i2c_deinit(ol_i2c_index_num id);


#endif

