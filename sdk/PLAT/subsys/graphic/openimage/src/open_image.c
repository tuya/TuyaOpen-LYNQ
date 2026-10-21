/****************************************************************************
 *
 * Copy right:   2024 -, Copyrigths of EigenComm Ltd.
 * File name:    open_image.c
 * Description:  ec7xx open_image source file
 * History:      Rev1.0   2024-09-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "mm_jpeg_if.h"
#include "mm_video_if.h"

#include "open_image.h"

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_tp.h"
#include "api_scr.h"
#include "api_comm.h"
extern uint32_t scr_dev_UsrId;
#endif

#define DISP_BUFFER_PIXEL   (240*320*2)
PLAT_FPSRAM_ZI_CUST uint8_t rgb2yuv_buf[DISP_BUFFER_PIXEL];
PLAT_FPSRAM_ZI_CUST uint8_t scaler_buf[DISP_BUFFER_PIXEL];
PLAT_FPSRAM_ZI_CUST uint8_t psram_buf[DISP_BUFFER_PIXEL];
PLAT_FPSRAM_ZI_CUST uint8_t psram_fb[DISP_BUFFER_PIXEL];

OPEN_IMAGE_ALIGN_ENUM image_align = OPEN_IMAGE_ALIGN_MIDDLE_MIDDLE;

void *jpeg_dec(uint8_t *jpeg,uint32_t size,uint16_t *pixels,uint8_t *output)
{
    void *pdec = NULL;
    JPEG_IMAGE_BUF ibuf;
    JPEG_INFO info;
    pdec = JpegD_Create();
    int32_t res = JpegD_DecodeInfo(pdec,jpeg,size,&info);
    if(res != 0)
    {
        goto end;
    }
    ibuf.eFmt = JPEG_COLOR_FMT_RGB565;
    ibuf.uWidth = info.uWidth;
    ibuf.uHeight = info.uHeight;
    ibuf.pData[0] = output;
    pixels[0] = info.uWidth;
    pixels[1] = info.uHeight;
    res = JpegD_DecodeImage(pdec,&ibuf);
    if(res != 0)
    {
        goto end;
    }
end:
    if(pdec != NULL)
    {
        JpegD_Destroy(pdec);
    }
    return output;
}

void *jpeg_enc(uint8_t* input,uint16_t* pixels,uint8_t *jpeg,uint32_t *size,int rgb2yuv)
{
    void *henc = NULL;
    JPEG_IMAGE_BUF ibuf;
    JPEG_ENC_PARAM param;
    JPEG_INFO info;
    henc = JpegE_Create();

    Sys_MemSet(&ibuf,0,sizeof(JPEG_IMAGE_BUF));
    
    ibuf.eFmt = JPEG_COLOR_FMT_YUYV;
    ibuf.uWidth = pixels[0];
	ibuf.uHeight = pixels[1];
    ibuf.pData[0] = input;
    SYSLOG_INFO("jpeg enc width:%d height:%d\r\n",ibuf.uWidth,ibuf.uHeight);
    if(rgb2yuv == 1)
    {
        Video_TransRgb565ToYuyv(input,rgb2yuv_buf,ibuf.uWidth,ibuf.uHeight);
        ibuf.pData[0] = rgb2yuv_buf;
    }

    param.eFmt = ibuf.eFmt;
    param.uWidth = ibuf.uWidth;
    param.uHeight = ibuf.uHeight;
    param.uQuality = 80;

    if(JpegE_SetParam(henc,&param,NULL) != 0)
    {
        SYSLOG_ERR("jpeg enc set param fail\r\n");
        goto end;
    }

    if(JpegE_Encode(henc,&ibuf,jpeg,size) != 0)
	{
        SYSLOG_ERR("jpeg enc fail\r\n");
		goto end;
	}
end:
    if(henc != NULL)
    {
        JpegE_Destroy(henc);
    }

    return jpeg;
}

OPEN_IMAGE_ERR open_image_read(char *filename,OPEN_IMAGE_INFO *info,uint8_t *buf)
{
    uint16_t pixel[2]= {0};
    FILE        *file         = NULL;
    struct stat  file_buf          = {0};
    SYSLOG_INFO("open_image_read image path:%s\r\n",filename);

    if(strstr(filename,".jpg")||strstr(filename,".jpeg"))
    {
        info->tpye = OPEN_IMAGE_TYPE_JPEG;
    }else if(strstr(filename,".bmp")){
        info->tpye = OPEN_IMAGE_TYPE_BMP;
    }else{
        return OPEN_IMAGE_NOT_SUPPORT;
    }
    
    file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        SYSLOG_ERR("file open:%s fail !!\r\n",filename);
        return OPEN_IMAGE_FILE_NOT_FOUND_ERR;
    }
    file_fstat((int)file, &file_buf);
    SYSLOG_INFO("image file size:%d\r\n",file_buf.st_size);

    if(info->tpye == OPEN_IMAGE_TYPE_JPEG)
    {
        if(file_buf.st_size>0)
        {
            uint8_t *jpeg = (uint8_t *)malloc(file_buf.st_size);
            file_fread(jpeg, file_buf.st_size, 1, file);
            // void *dispBuffer = jpeg_test(jpeg,file_buf.st_size,pixel); 

            buf = jpeg_dec(jpeg,file_buf.st_size,pixel,buf);
            info->width = pixel[0];
            info->height = pixel[1];
            free(jpeg);
            
        }

    }else if(info->tpye == OPEN_IMAGE_TYPE_BMP){
    
    }

    file_fclose(file);
    
    return OPEN_IMAGE_RET_OK;
}

OPEN_IMAGE_ERR open_image_align(OPEN_IMAGE_ALIGN_ENUM align)
{

}

OPEN_IMAGE_ERR open_image_show(char *filename)
{
    OPEN_IMAGE_INFO info={0};
    float w_scale_radio = 1.0,h_scale_radio = 1.0;
    int scale_width,scale_height;

    SYSLOG_INFO("open_image_show image path:%s\r\n",filename);
    void *image = psram_buf;
    memset(image,0,320*240*2);
    open_image_read(filename,&info, image);
    SYSLOG_INFO("info:w:%d,h:%d\r\n",info.width,info.height);

    if(info.width > 240 || info.height > 320)
    {
        w_scale_radio = info.width / 240.0;
        h_scale_radio = info.height / 320.0;
        if(w_scale_radio < h_scale_radio)
        {
            scale_width = floor(info.width/h_scale_radio);
            scale_height = 320;
        }else
        {
            scale_width = 240;
            scale_height = floor(info.height/w_scale_radio);
        }
        SYSLOG_INFO("scale_width:%d,scale_height:%d\r\n",scale_width,scale_height);
        Video_ScaleImageRgb565(image,info.width,info.height,psram_fb,scale_width,scale_height);
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        if (scr_dev_UsrId) 
        {
            api_screen_fill_t trans = {
                .sx = 0,
                .sy = 0,
                .ex = scale_width,
                .ey = scale_height,
                .data = psram_fb,
            };
            api_scr_write(scr_dev_UsrId, &trans, 1);
        }
        #endif
    }else{
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        if (scr_dev_UsrId) 
        {
            api_screen_fill_t trans = {
                .sx = 0,
                .sy = 0,
                .ex = info.width-1,
                .ey = info.height-1,
                .data = image,
            };
            api_scr_write(scr_dev_UsrId, &trans, 1);
        }
        #endif
    }

    // if(image_align == OPEN_IMAGE_ALIGN_MIDDLE_MIDDLE)
    // {
    //     int sx = (240-scale_width)/2;
    //     int sy = (320-scale_height)/2;
    //     int ex = sx+scale_width;
    //     int ey = sy+scale_height;
    //     SYSLOG_INFO("sx:%d,sy:%d,ex:%d,ey:%d\r\n",sx,sy,ex,ey);
    // }

    return OPEN_IMAGE_RET_OK;
}

OPEN_IMAGE_ERR open_image_save(char *filename,OPEN_IMAGE_INFO *info,uint8_t *data,int rgb2yuv)
{
    uint16_t pixel[2]= {0};
    uint32_t jpg_size = 0;
    void *jpg = psram_fb;
    FILE        *file         = NULL;
    SYSLOG_INFO("open_image_save image path:%s\r\n",filename);
    SYSLOG_INFO("info:w:%d,h:%d\r\n",info->width,info->height);
    SYSLOG_INFO("data addr:0x%x\r\n",data);
    file = file_fopen(filename, "wb");
    if(file == NULL)
    {
        SYSLOG_ERR("file open:%s fail !!\r\n",filename);
        return OPEN_IMAGE_FILE_NOT_FOUND_ERR;
    }
    
    pixel[0] = info->width;
    pixel[1] = info->height;
    jpg_size = 320*240*2;

    jpeg_enc(data,pixel,jpg,&jpg_size,rgb2yuv);
    SYSLOG_INFO("jpg_size:%d\r\n",jpg_size);
    file_fwrite(jpg, jpg_size, 1, file);
    file_fclose(file);
    return OPEN_IMAGE_RET_OK;
}

OPEN_IMAGE_ERR open_image_resize(char *filename,char *save_filename)
{
    OPEN_IMAGE_INFO info={0};
    uint16_t pixel[2]= {0};

    SYSLOG_INFO("open_image_resize image path:%s\r\n",filename);
    void *image = psram_buf;
    memset(image,0,320*240*2);
    open_image_read(filename,&info, image);
    SYSLOG_INFO("info:w:%d,h:%d\r\n",info.width,info.height);
    Video_ScaleImageRgb565(image,info.width,info.height,scaler_buf,128,160);
    info.width = 128;
    info.height = 160;
    open_image_save(save_filename,&info,scaler_buf,1);
    
}
