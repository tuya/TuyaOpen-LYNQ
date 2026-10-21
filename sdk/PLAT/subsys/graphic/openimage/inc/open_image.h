/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    open_image.h
 * Description:  open imageheader file
 * History:      Rev1.0   2024-09-20
 *
 ****************************************************************************/
#ifndef __OPEN_IMAGE_H__
#define __OPEN_IMAGE_H__

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    OPEN_IMAGE_TYPE_BMP,
    OPEN_IMAGE_TYPE_JPEG,
} OPEN_IMAGE_TYPE;

typedef struct open_image_info
{
    OPEN_IMAGE_TYPE tpye;
    int    width;
    int    height;
} OPEN_IMAGE_INFO;

typedef enum {
    OPEN_IMAGE_ALIGN_TOP_LEFT,
    OPEN_IMAGE_ALIGN_TOP_MIDDLE,
    OPEN_IMAGE_ALIGN_TOP_RIGHT,
    OPEN_IMAGE_ALIGN_MIDDLE_LEFT,
    OPEN_IMAGE_ALIGN_MIDDLE_MIDDLE,
    OPEN_IMAGE_ALIGN_MIDDLE_RIGHT,
    OPEN_IMAGE_ALIGN_BOTTOM_LEFT,
    OPEN_IMAGE_ALIGN_BOTTOM_MIDDLE,
    OPEN_IMAGE_ALIGN_BOTTOM_RIGHT
} OPEN_IMAGE_ALIGN_ENUM;
typedef enum {
    OPEN_IMAGE_RET_OK,
    OPEN_IMAGE_NOT_SUPPORT,
    OPEN_IMAGE_MALLOC_ERR,
    OPEN_IMAGE_FILE_NOT_FOUND_ERR,
} OPEN_IMAGE_ERR;

OPEN_IMAGE_ERR open_image_read(char *filename,OPEN_IMAGE_INFO *info,uint8_t *buf);
OPEN_IMAGE_ERR open_image_save(char *filename,OPEN_IMAGE_INFO *info,uint8_t *data,int rgb2yuv);

#endif //__OPEN_IMAGE_H__
