#ifndef _OL_SPI_API_H_
#define _OL_SPI_API_H_

#include "Driver_SPI.h"


typedef enum
{
    OL_SPI_INDEX_0 = 0,
    OL_SPI_INDEX_1 = 1,
} ol_spi_index_num;


//SPI driver event define in Driver_SPI.h
typedef void (*ol_spi_event_cb_f)(uint32_t event);


typedef struct
{
    uint32_t control;
    uint32_t arg;                 //frequency to output
    ol_spi_event_cb_f cb_event;
}ol_spi_config_t;


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to initialize the spi
 * PARAMETERS
 *      id       [IN]: spi index, enum ol_spi_index_num
 *      config : [IN]: spi config params defined in ol_spi_config_t
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_spi_init(ol_spi_index_num id, ol_spi_config_t *config);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to read and write
 * PARAMETERS
 *      id         [IN] : spi index, enum ol_spi_index_num
 *      write_buf  [IN] : wrtite data
 *      read_buf   [out]: read data
 *      data_len   [IN] : write/read data buffer length
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_spi_write_read(ol_spi_index_num id, unsigned char* write_buf, unsigned char* read_buf, unsigned long data_len);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to write
 * PARAMETERS
 *      id        [IN] : spi index, enum ol_spi_index_num
 *      write_buf [IN] : wrtite data
 *      data_len  [IN] : data buffer length
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_spi_write(ol_spi_index_num id, unsigned char* write_buf, unsigned long data_len);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to read
 * PARAMETERS
 *      id       [IN] : spi index, enum ol_spi_index_num
 *      read_buf [out]: read data
 *      data_len [IN] : data buffer length
 * RETURN VALUES
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_spi_read(ol_spi_index_num id, unsigned char* read_buf, unsigned long data_len);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to deinitialize the spi
 * PARAMETERS
 *      id [IN] : spi index, enum ol_spi_index_num
 * RETURN
 *      =0  : success
 *      <0  : error codes, define in Driver_Common.h ARM_DRIVER_XXX
 *
 *****************************************************************************/
extern int ol_spi_deinit(ol_spi_index_num id);


#endif
