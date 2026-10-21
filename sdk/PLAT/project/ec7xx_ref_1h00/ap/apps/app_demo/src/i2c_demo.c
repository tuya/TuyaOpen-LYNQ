 /****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "osasys.h"
#include "cmsis_os2.h"

#include "bsp_custom.h"
#include "ol_log.h"
#include "ol_i2c_api.h"


#if RTE_I2C1_IO_MODE != POLLING_MODE
volatile bool i2c_ok = false;

void i2c_callback(uint32_t event)
{
    OL_LOG_PRINTF("i2c callback event = %d",event);
    if(event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        i2c_ok = true;
    }
}
#endif

void i2c_demo(void)
{
    int ret = 0;
    uint8_t w_buf[] = "test i2c data";
    uint8_t r_buf[32] = {0};
    uint8_t cmd[] = {0x78,0x89};//adddr = 0x7889;cmdlen = 2
    uint8_t slaveaddr = 0x50;

    #if RTE_I2C1_IO_MODE != POLLING_MODE
    ret = ol_i2c_init(OL_I2C_INDEX_1,OL_I2C_SPEED_STANDARD,i2c_callback);
    #else
    ret = ol_i2c_init(OL_I2C_INDEX_1,OL_I2C_SPEED_STANDARD,NULL);
    #endif
    OL_LOG_PRINTF("ol_i2c_init ret = %d",ret);

    ret = ol_i2c_write(OL_I2C_INDEX_1,slaveaddr,cmd,2,w_buf,strlen((char *)w_buf));
    OL_LOG_PRINTF("ol_i2c_write ret = %d",ret);

    #if RTE_I2C1_IO_MODE != POLLING_MODE
    while(i2c_ok != true)
    {
        i2c_ok = false;
    }
    #endif

    // minimal 5ms interval required between write and read opration
    delay_us(5000);

    memset(r_buf,0x0,sizeof(r_buf));
    ret = ol_i2c_read(OL_I2C_INDEX_1,slaveaddr,cmd,2,r_buf,sizeof(r_buf));
    OL_LOG_PRINTF("ol_i2c_read ret = %d",ret);
    #if RTE_I2C1_IO_MODE != POLLING_MODE
    while(i2c_ok != true)
    {
        i2c_ok = false;
    }
    #endif
    OL_LOG_PRINTF("ol_i2c_read data = %s",r_buf);
}

