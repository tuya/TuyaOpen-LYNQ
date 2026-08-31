/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "bsp_custom.h"
#include "cmips.h"
#include "cmisim.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "networkmgr.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "queue.h"
#include "semphr.h"
#include "slpman.h"
#include "storage.h"
#include "string.h"
#include "task.h"
#include "time.h"

#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "bsp.h"
#include "console.h"
#include "console_ex.h"
#include "console_file.h"
#include "console_hal.h"
#include "packet.h"

#ifdef FEATURE_SUBSYS_GRAPHIC_OPENIMAGE_ENABLE
#include "open_image.h"
#include "open_anim.h"
#endif
#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "ff.h"
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#endif

#ifdef FEATURE_HAL_SCREEN_ENABLE
#include "api_scr.h"
extern uint32_t scr_dev_UsrId;
#endif

#ifdef FEATURE_SUBSYS_GRAPHIC_OPENIMAGE_ENABLE

void console_image_read_png(int argc, char **argv)
{
    printf("read PNG file:\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    open_image_read(argv[1], image, NULL);
    open_image_save("D:/readpng.jpg", image, NULL);
    open_image_destroy(image);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_read_png, read_png,
                     read png to save a jpeg file);
#ifdef FEATURE_HAL_SCREEN_ENABLE
static int clear_screen()
{
    ScrWriteParam_t info = {0};
    info.start_x = 0;
    info.width = LCD_WIDTH;
    info.start_y = 0;
    info.height = LCD_HEIGHT;
    info.data = malloc(LCD_WIDTH * LCD_HEIGHT * 2);
    info.size = LCD_WIDTH * LCD_HEIGHT * 2;
    info.fmt = OPEN_SCREEN_DATA_FMT_RGB565;
    memset(info.data, 0, LCD_WIDTH * LCD_HEIGHT * 2);
    Device_write_by_index(scr_dev_UsrId, &info, sizeof(ScrWriteParam_t));
    free(info.data);
    osDelay(500);

    return 0;
}
#endif

void console_image_show(int argc, char **argv)
{
    int ret = 0;
    printf("console_image_show\r\n");
    printf("argv =%s\r\n", argv[1]);
    char *file_name = argv[1];
    if(strstr(file_name, ".jpg") || strstr(file_name, ".jpeg") ||
       strstr(file_name, ".JPEG") || strstr(file_name, ".JPG"))
    {
        printf("show jpeg file\r\n");
        JpegInfo_t jpeg_info;
        ret = open_image_get_jpgfile_info(file_name, &jpeg_info);
        if(ret != 0)
        {
            printf("get jpeg file info failed\r\n");
            return;
        }
        printf("jpeg file resolution is %d x %d\r\n", jpeg_info.width,
               jpeg_info.height);
        OpImg_t *image = open_image_create(jpeg_info.width, jpeg_info.height,
                                           IMG_FMT_RGB565, NULL, 0);
        ret = open_image_read(argv[1], image, NULL);
        if(ret != 0)
        {
            printf("read jpeg file failed\r\n");
            return;
        }
#ifdef FEATURE_HAL_SCREEN_ENABLE
        clear_screen();
        open_image_show(image, false);
#endif
        open_image_destroy(image);
    }
    if(strstr(file_name, ".gif") || strstr(file_name, ".GIF"))
    {
        printf("show gif file\r\n");
#ifdef FEATURE_SUBSYS_GRAPHIC_OPENIMAGE_ENABLE
        open_anim_play(argv[1]);
#endif
    }
    else
    {
        if(argc < 4)
        {
            printf(
                "console_image_show: no enough parameters, file not jpeg "
                "should specify width and height\r\n");
            return;
        }

        OpImg_t *image = open_image_create(atoi(argv[2]), atoi(argv[3]),
                                           IMG_FMT_RGB565, NULL, 0);
        if(!image)
        {
            printf("create image failed\r\n");
            return;
        }
        int ret = open_image_read(argv[1], image, NULL);
        if(ret != 0)
        {
            printf("read image failed\r\n");
            return;
        }
#ifdef FEATURE_HAL_SCREEN_ENABLE
        clear_screen();
        open_image_show(image, false);
#endif
        open_image_destroy(image);
    }
    
}
MSH_CMD_EXPORT_ALIAS(console_image_show, show, show image);

void console_image_resize(int argc, char **argv)
{
    printf("console_image_resize\r\n");
    printf("argv =%s\r\n", argv[1]);
    CropInfo_t info = {0};

    OpImg_t *image = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    OpImg_t *image_crop = open_image_create(128, 160, IMG_FMT_RGB565, NULL, 0);
    OpImg_t *image_scale = open_image_create(128, 160, IMG_FMT_RGB565, NULL, 0);
    open_image_read(argv[1], image, NULL);
    info.x = 0;
    info.y = 0;
    info.width = 128;
    info.height = 160;
    open_image_scale(image, image_crop, &info);
    open_image_save("D:/crop.jpg", image_crop, NULL);
    info.x = 0;
    info.y = 0;
    info.width = 320;
    info.height = 240;
    open_image_scale(image, image_scale, &info);
    open_image_save("D:/scale.jpg", image_scale, NULL);
    open_image_destroy(image);
    open_image_destroy(image_crop);
    open_image_destroy(image_scale);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_resize, imgresize, resize image);

void console_image_mirror(int argc, char **argv)
{
    printf("console_image_mirror\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    open_image_read(argv[1], image, NULL);
    OpImg_t *image_mirror =
        open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    open_image_rotate_mirror(image, image_mirror, true, false);
    open_image_save("D:/mirror.jpg", image_mirror, NULL);
    open_image_destroy(image);
    open_image_destroy(image_mirror);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_mirror, mirror, mirro image);

void console_image_rotate(int argc, char **argv)
{
    printf("console_image_rotate\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    open_image_read(argv[1], image, NULL);
    OpImg_t *image_rotate =
        open_image_create(240, 320, IMG_FMT_RGB565, NULL, 0);
    open_image_rotate_mirror(image, image_rotate, false, true);
    open_image_save("D:/rotate.jpg", image_rotate, NULL);
    open_image_destroy(image);
    open_image_destroy(image_rotate);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_rotate, rotate, rotate image);

void console_image_conv_fmt(int argc, char **argv)
{
    printf("read jpeg file:\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_YUV422S, NULL, 0);
    open_image_read(argv[1], image, NULL);
    OpImg_t *image_conv = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    open_image_conv_fmt(image, image_conv);
    open_image_save("D:/yuv.jpg", image_conv, NULL);
    open_image_destroy(image);
    open_image_destroy(image_conv);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_conv_fmt, imgconv, convert image format);

void console_image_enc_jpg(int argc, char **argv)
{
    printf("read yuv file:\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    open_image_read(argv[1], image, NULL);
    uint8_t *jpeg_data = NULL;
    uint32_t jpeg_size = 320 * 240 * 2;
    jpeg_data = malloc(jpeg_size);
    if(jpeg_data == NULL)
    {
        printf("malloc jpeg data failed\r\n");
        return;
    }
    OPEN_IMAGE_INFO info;
    info.type = OPEN_IMAGE_TYPE_JPEG;
    open_image_encode(image, jpeg_data, &jpeg_size, &info, 80);

    FILE *file_jpeg = NULL;
    file_jpeg = file_fopen("D:/encode.jpg", "wb");
    if(file_jpeg == NULL)
    {
        printf("open file failed\r\n");
        free(jpeg_data);
        return;
    }
    file_fwrite(jpeg_data, jpeg_size, 1, file_jpeg);
    file_fclose(file_jpeg);
    free(jpeg_data);
    open_image_destroy(image);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_enc_jpg, enc_jpg, encode to jpeg);

void console_image_dec_jpg(int argc, char **argv)
{
    printf("read jpeg file:\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_RGB565, NULL, 0);
    uint8_t *jpeg_data = NULL;
    uint32_t jpeg_size = 320 * 240 * 2;
    jpeg_data = malloc(jpeg_size);
    if(jpeg_data == NULL)
    {
        printf("malloc jpeg data failed\r\n");
        return;
    }
    FILE *file_jpeg = NULL;
    file_jpeg = file_fopen(argv[1], "rb");
    if(file_jpeg == NULL)
    {
        printf("open file failed\r\n");
        free(jpeg_data);
        return;
    }
    struct stat file_stat = {0};
    file_fstat((int)file_jpeg, &file_stat);
    jpeg_size = file_stat.st_size;
    file_fread(jpeg_data, jpeg_size, 1, file_jpeg);
    file_fclose(file_jpeg);
    OPEN_IMAGE_INFO info;
    info.type = OPEN_IMAGE_TYPE_JPEG;
    open_image_decode(image, jpeg_data, &jpeg_size, &info);
    open_image_save("D:/decode.jpg", image, NULL);
    free(jpeg_data);
    open_image_destroy(image);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_dec_jpg, dec_jpg, decode jpeg);

void console_image_dec_png(int argc, char **argv)
{
    printf("read png file:\r\n");
    printf("argv =%s\r\n", argv[1]);
    OpImg_t *image = open_image_create(320, 240, IMG_FMT_YUV422S, NULL, 0);
    uint8_t *jpeg_data = NULL;
    uint32_t jpeg_size = 320 * 240 * 2;
    jpeg_data = malloc(jpeg_size);
    if(jpeg_data == NULL)
    {
        printf("malloc png data failed\r\n");
        return;
    }
    FILE *file_jpeg = NULL;
    file_jpeg = file_fopen(argv[1], "rb");
    if(file_jpeg == NULL)
    {
        printf("open file failed\r\n");
        free(jpeg_data);
        return;
    }
    struct stat file_stat = {0};
    file_fstat((int)file_jpeg, &file_stat);
    jpeg_size = file_stat.st_size;
    file_fread(jpeg_data, jpeg_size, 1, file_jpeg);
    file_fclose(file_jpeg);
    OPEN_IMAGE_INFO info;
    info.type = OPEN_IMAGE_TYPE_PNG;
    open_image_decode(image, jpeg_data, &jpeg_size, &info);
    open_image_save("D:/decode.jpg", image, NULL);
    free(jpeg_data);
    open_image_destroy(image);
    
}
MSH_CMD_EXPORT_ALIAS(console_image_dec_png, dec_png, decode png);

#endif

#endif

#endif