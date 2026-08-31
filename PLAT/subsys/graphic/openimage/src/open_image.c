/****************************************************************************
 *
 * Copy right:   2024 -, Copyrigths of EigenComm Ltd.
 * File name:    open_image.c
 * Description:  ec7xx open_image source file
 * History:      Rev1.0   2024-09-12
 *
 ****************************************************************************/
#include "open_image.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cmsis_os2.h"
#include "mm_jpeg_if.h"
#include "mm_video_if.h"

#include DEBUG_LOG_HEADER_FILE

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#include "devicemanager.h"

extern uint32_t scr_dev_UsrId;
#endif

#ifdef FEATURE_SUBSYS_DISPLAY_ENABLE
#include "display.h"
#endif

// BMP 文件头结构体
typedef struct
{
    uint8_t signature[2];
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
} __attribute__((packed)) BMPFileHeader_t;

// BMP 信息头结构体
typedef struct
{
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} __attribute__((packed)) BMPInfoHeader_t;

static osMutexId_t sJpegEncLock = NULL;

static osMutexId_t get_enc_lock()
{
    if(sJpegEncLock == NULL)
    {
        sJpegEncLock = osMutexNew(NULL);
    }
    return sJpegEncLock;
}

static void rgb565_to_rgb888(uint16_t rgb565, uint8_t *r, uint8_t *g,
                             uint8_t *b)
{
    *r = (rgb565 >> 11) << 3;
    *g = ((rgb565 >> 5) & 0x3F) << 2;
    *b = (rgb565 & 0x1F) << 3;
}

static int open_image_jpeg_dec(uint8_t *jpeg_data, uint32_t jpeg_size,
                               uint8_t *rgb_data, uint32_t *rgb_size,
                               uint32_t *width, uint32_t *height)
{
    void *jpg_codec = NULL;
    JPEG_IMAGE_BUF ibuf;
    JPEG_INFO info;
    osMutexId_t lock = get_enc_lock();
    osMutexAcquire(lock, osWaitForever);
    jpg_codec = JpegD_Create();
    if(JpegD_DecodeInfo(jpg_codec, jpeg_data, jpeg_size, &info) != 0)
    {
        JpegD_Destroy(jpg_codec);
        osMutexRelease(sJpegEncLock);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    if(*rgb_size < info.uWidth * info.uHeight * 2)
    {
        JpegD_Destroy(jpg_codec);
        osMutexRelease(sJpegEncLock);
        return OPEN_IMAGE_OUT_OF_MEMORY;
    }

    ibuf.eFmt = JPEG_COLOR_FMT_RGB565;
    ibuf.uWidth = info.uWidth;
    ibuf.uHeight = info.uHeight;
    ibuf.pData[0] = rgb_data;
    *width = info.uWidth;
    *height = info.uHeight;
    *rgb_size = info.uWidth * info.uHeight * 2;
    if(JpegD_DecodeImage(jpg_codec, &ibuf) != 0)
    {
        JpegD_Destroy(jpg_codec);
        osMutexRelease(sJpegEncLock);
        return OPEN_IMAGE_CODEC_FAILED;
    }

    JpegD_Destroy(jpg_codec);
    osMutexRelease(sJpegEncLock);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_jpeg_enc(uint8_t *rgb_data, uint32_t rgb_size,
                               uint8_t *jpeg_data, uint32_t *jpeg_size,
                               uint32_t width, uint32_t height, uint8_t quality, bool mono)
{
    void *jpg_codec = NULL;
    JPEG_IMAGE_BUF ibuf;
    JPEG_ENC_PARAM param;
    osMutexId_t lock = get_enc_lock();
    osMutexAcquire(lock, osWaitForever);
    jpg_codec = JpegE_Create();

    memset(&ibuf, 0, sizeof(JPEG_IMAGE_BUF));
    // 支持YUV格式的原始数据
    ibuf.eFmt = mono ? JPEG_COLOR_FMT_Y : JPEG_COLOR_FMT_YUYV;
    ibuf.uWidth = width;
    ibuf.uHeight = height;
    ibuf.pData[0] = rgb_data;

    param.eFmt = ibuf.eFmt;
    param.uWidth = ibuf.uWidth;
    param.uHeight = ibuf.uHeight;
    param.uQuality = quality;

    if(JpegE_SetParam(jpg_codec, &param, NULL) != 0)
    {
        JpegE_Destroy(jpg_codec);
        osMutexRelease(sJpegEncLock);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    unsigned int enc_size = *jpeg_size;
    if(JpegE_Encode(jpg_codec, &ibuf, jpeg_data, &enc_size) != 0)
    {
        JpegE_Destroy(jpg_codec);
        osMutexRelease(sJpegEncLock);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    *jpeg_size = (uint32_t)enc_size;
    JpegE_Destroy(jpg_codec);
    osMutexRelease(sJpegEncLock);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_png_dec(uint8_t *png_data, uint32_t png_size,
                              uint8_t *rgb_data, uint32_t *rgb_size,
                              uint32_t *width, uint32_t *height)
{
    void *png_codec = NULL;
    VIDEO_IMAGE_BUF ibuf;
    PNG_INFO info;
    png_codec = PngD_Create();
    if(PngD_DecodeInfo(png_codec, png_data, png_size, &info) != 0)
    {
        PngD_Destroy(png_codec);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    ibuf.eFmt = VIDEO_COLOR_FMT_RGB565;
    ibuf.uWidth = info.uWidth;
    ibuf.uHeight = info.uHeight;
    ibuf.pData[0] = rgb_data;
    *width = info.uWidth;
    *height = info.uHeight;
    if(*rgb_size < info.uWidth * info.uHeight * 2)
    {
        PngD_Destroy(png_codec);
        return OPEN_IMAGE_OUT_OF_MEMORY;
    }
    *rgb_size = info.uWidth * info.uHeight * 2;
    if(PngD_DecodeImage(png_codec, &ibuf) != 0)
    {
        PngD_Destroy(png_codec);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    PngD_Destroy(png_codec);
    return OPEN_IMAGE_RET_OK;
}

static uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

static int fmt_conv_y_to_yuv422s(const uint8_t *y_in, uint8_t *yuyv_out,
                                 uint32_t width, uint32_t height)
{
    // 参数验证
    if(!y_in || !yuyv_out || width == 0 || height == 0 || width % 2 != 0)
    {
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    uint8_t neutral_uv = 128;
    uint32_t row_size = width * 2;

    for(uint32_t row = 0; row < height; row++)
    {
        const uint8_t *y_ptr = y_in + row * width;
        uint8_t *yuyv_ptr = yuyv_out + row * row_size;
        for(uint32_t col = 0; col < width; col += 2)
        {
            // 处理一对像素
            *yuyv_ptr++ = y_ptr[col];      // Y0
            *yuyv_ptr++ = neutral_uv;      // U
            *yuyv_ptr++ = y_ptr[col + 1];  // Y1
            *yuyv_ptr++ = neutral_uv;      // V
        }
    }
    return OPEN_IMAGE_RET_OK;
}

static int fmt_conv_yuv422s_to_y(const uint8_t *yuyv_in, uint8_t *y_out,
                                 uint32_t width, uint32_t height)
{
    if(!yuyv_in || !y_out || width == 0 || height == 0)
    {
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if(width % 2 != 0)
    {
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    uint32_t yuyv_row_size = width * 2;
    uint32_t y_row_size = width;
    for(uint32_t row = 0; row < height; row++)
    {
        const uint8_t *yuyv_ptr = yuyv_in + row * yuyv_row_size;
        uint8_t *y_ptr = y_out + row * y_row_size;
        for(uint32_t col = 0; col < width; col += 2)
        {
            *y_ptr++ = yuyv_ptr[0];  // Y0
            *y_ptr++ = yuyv_ptr[2];  // Y1
            yuyv_ptr += 4;
        }
    }

    return OPEN_IMAGE_RET_OK;
}

static inline uint16_t y_to_rgb565(uint8_t y)
{
    uint8_t r = y >> 3;
    uint8_t g = y >> 2;
    uint8_t b = y >> 3;
    return (r << 11) | (g << 5) | b;
}

static inline uint8_t rgb565_to_y(uint16_t rgb)
{
    uint8_t r = (rgb >> 11) & 0x1F;
    uint8_t g = (rgb >> 5) & 0x3F;
    uint8_t b = rgb & 0x1F;
    uint16_t r8 = (r << 3) | (r >> 2);
    uint16_t g8 = (g << 2) | (g >> 4);
    uint16_t b8 = (b << 3) | (b >> 2);
    uint16_t y = (299 * r8 + 587 * g8 + 114 * b8 + 500) / 1000;
    return (uint8_t)y;
}

static int fmt_conv_y_to_rgb565(const uint8_t *y_in, uint16_t *rgb_out,
                                uint32_t width, uint32_t height)
{
    if(!y_in || !rgb_out || width == 0 || height == 0)
    {
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    const uint8_t *y_ptr = y_in;
    uint16_t *rgb_ptr = rgb_out;
    uint32_t total = width * height;
    for(uint32_t i = 0; i < total; i++)
    {
        *rgb_ptr++ = y_to_rgb565(*y_ptr++);
    }

    return OPEN_IMAGE_RET_OK;
}

static int fmt_conv_rgb565_to_y(const uint16_t *rgb_in, uint8_t *y_out,
                                uint32_t width, uint32_t height)
{
    if(!rgb_in || !y_out || width == 0 || height == 0)
    {
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    const uint16_t *rgb_ptr = rgb_in;
    uint8_t *y_ptr = y_out;
    uint32_t total = width * height;
    for(uint32_t i = 0; i < total; i++)
    {
        *y_ptr++ = rgb565_to_y(*rgb_ptr++);
    }

    return OPEN_IMAGE_RET_OK;
}

static int open_image_write_bmp_file(const char *filename, uint16_t *rgb565Data,
                                     uint32_t size, int32_t width,
                                     int32_t height)
{
    FILE *file = file_fopen(filename, "wb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_bmp_failed_0, P_INFO,
                      "file open %s failed", filename);
        return OPEN_IMAGE_FILE_NOT_FOUND_ERR;
    }

    if(size < width * height * 2)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_bmp_failed_1, P_INFO,
                      "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }

    int32_t rowSize = (width * 3 + 3) & ~3;
    uint32_t imageSize = rowSize * height;

    BMPFileHeader_t fileHeader;
    fileHeader.signature[0] = 'B';
    fileHeader.signature[1] = 'M';
    fileHeader.fileSize =
        sizeof(BMPFileHeader_t) + sizeof(BMPInfoHeader_t) + imageSize;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.dataOffset = sizeof(BMPFileHeader_t) + sizeof(BMPInfoHeader_t);

    BMPInfoHeader_t infoHeader;
    infoHeader.headerSize = sizeof(BMPInfoHeader_t);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitCount = 24;
    infoHeader.compression = 0;
    infoHeader.imageSize = imageSize;
    infoHeader.xPixelsPerMeter = 0;
    infoHeader.yPixelsPerMeter = 0;
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    file_fwrite(&fileHeader, sizeof(BMPFileHeader_t), 1, file);
    file_fwrite(&infoHeader, sizeof(BMPInfoHeader_t), 1, file);

    uint8_t *rgb888Data = (uint8_t *)malloc(rowSize);
    for(int32_t y = height - 1; y >= 0; y--)
    {
        for(int32_t x = 0; x < width; x++)
        {
            uint16_t rgb565 = rgb565Data[y * width + x];
            uint8_t r, g, b;
            rgb565_to_rgb888(rgb565, &r, &g, &b);
            rgb888Data[x * 3] = b;
            rgb888Data[x * 3 + 1] = g;
            rgb888Data[x * 3 + 2] = r;
        }
        for(int32_t i = width * 3; i < rowSize; i++)
        {
            rgb888Data[i] = 0;
        }
        file_fwrite(rgb888Data, rowSize, 1, file);
    }
    free(rgb888Data);

    file_fclose(file);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_read_bmp_file(const char *filename, uint16_t *rgb565Data,
                                    uint32_t size, uint32_t *width,
                                    uint32_t *height)
{
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_bmp_failed_0, P_INFO,
                      "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }

    BMPFileHeader_t fileHeader;
    file_fread(&fileHeader, sizeof(BMPFileHeader_t), 1, file);

    BMPInfoHeader_t infoHeader;
    file_fread(&infoHeader, sizeof(BMPInfoHeader_t), 1, file);

    if(infoHeader.bitCount != 24)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_bmp_failed_1, P_INFO,
                      "only 24bit bmp file supported");
        return OPEN_IMAGE_NOT_SUPPORT;
    }

    *width = infoHeader.width;
    *height = infoHeader.height;

    if(size < infoHeader.width * infoHeader.height * 2)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_bmp_failed_2, P_INFO,
                      "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }

    int32_t rowSize = (infoHeader.width * 3 + 3) & ~3;

    file_fseek(file, fileHeader.dataOffset, SEEK_SET);

    for(int y = infoHeader.height - 1; y >= 0; y--)
    {
        for(int x = 0; x < infoHeader.width; x++)
        {
            uint8_t b, g, r;
            file_fread(&b, 1, 1, file);
            file_fread(&g, 1, 1, file);
            file_fread(&r, 1, 1, file);

            uint16_t rgb565 = rgb888_to_rgb565(r, g, b);
            rgb565Data[y * infoHeader.width + x] = rgb565;
        }
        file_fseek(file, rowSize - infoHeader.width * 3, SEEK_CUR);
    }

    file_fclose(file);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_read_rgb565_file(const char *filename, uint16_t *src_data,
                                       uint32_t size, uint32_t width,
                                       uint32_t height, RawFmt_t type)
{
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_rgb565_failed_0,
                      P_INFO, "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    if((file_stat.st_size < width * height * 2) || (size < width * height * 2))
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_rgb565_failed_1,
                      P_INFO, "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *image_data = NULL;
    uint32_t image_size = width * height * 2;
    if(type != IMG_FMT_RGB565)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_rgb565_failed_2,
                          P_INFO, "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
    }
    else
    {
        image_data = (uint8_t *)src_data;
    }

    file_fread(image_data, image_size, 1, file);
    if(type == IMG_FMT_YUV422S)
    {
        Video_TransRgb565ToYuyv((uint16_t *)image_data, (uint8_t *)src_data,
                                width, height);
        free(image_data);
    }
    else if(type == IMG_FMT_Y)
    {
        fmt_conv_rgb565_to_y((uint16_t *)image_data, (uint8_t *)src_data, width,
                             height);
    }
    file_fclose(file);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_write_rgb565_file(const char *filename,
                                        uint16_t *src_data, uint32_t size,
                                        uint32_t width, uint32_t height,
                                        RawFmt_t type)
{
    FILE *file = file_fopen(filename, "wb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_rgb565_failed_0,
                      P_INFO, "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    uint8_t *image_data = NULL;
    uint32_t image_size = width * height * 2;
    if(type == IMG_FMT_YUV422S)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_rgb565_failed_2,
                          P_INFO, "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        Video_TransYuyvToRgb565((uint8_t *)src_data, (uint16_t *)image_data,
                                width, height);
    }
    else if(type == IMG_FMT_Y)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_rgb565_failed_2_1,
                          P_INFO, "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        fmt_conv_y_to_rgb565((uint8_t *)src_data, (uint16_t *)image_data, width,
                             height);
    }
    else
    {
        image_data = (uint8_t *)src_data;
    }
    if(size < image_size)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_rgb565_failed_1,
                      P_INFO, "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }
    file_fwrite(image_data, image_size, 1, file);
    file_fclose(file);
    if(type != IMG_FMT_RGB565)
    {
        free(image_data);
    }
    return OPEN_IMAGE_RET_OK;
}

static int open_image_read_yuv422s_file(const char *filename,
                                        uint16_t *src_data, uint32_t size,
                                        uint32_t width, uint32_t height,
                                        RawFmt_t type)
{
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_yuv422s_failed_0,
                      P_INFO, "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    if((file_stat.st_size < width * height * 2) || (size < width * height * 2))
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_yuv422s_failed_1,
                      P_INFO, "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *image_data = NULL;
    uint32_t image_size = width * height * 2;
    if(type != IMG_FMT_YUV422S)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_rgb565_failed_2,
                          P_INFO, "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
    }
    else
    {
        image_data = (uint8_t *)src_data;
    }

    file_fread(image_data, image_size, 1, file);
    file_fclose(file);
    if(type == IMG_FMT_RGB565)
    {
        Video_TransYuyvToRgb565(image_data, src_data, width, height);
        free(image_data);
    }
    else if(type == IMG_FMT_Y)
    {
        fmt_conv_yuv422s_to_y(image_data, (uint8_t *)src_data, width, height);
    }
    return OPEN_IMAGE_RET_OK;
}

int open_image_read_y_file(const char *filename, uint8_t *src_data,
                           uint32_t size, uint32_t width, uint32_t height,
                           RawFmt_t type)
{
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_y_failed_0, P_INFO,
                      "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    if((file_stat.st_size < width * height) || (size < width * height))
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_y_failed_1, P_INFO,
                      "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *image_data = NULL;
    uint32_t image_size = width * height;
    if(type != IMG_FMT_Y)
    {
        image_size = width * height * 2;
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_y_failed_2, P_INFO,
                          "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
    }
    else
    {
        image_data = (uint8_t *)src_data;
    }

    file_fread(image_data, image_size, 1, file);
    file_fclose(file);
    if(type == IMG_FMT_YUV422S)
    {
        fmt_conv_y_to_yuv422s(image_data, src_data, width, height);
        free(image_data);
    }
    else if(type == IMG_FMT_RGB565)
    {
        fmt_conv_y_to_rgb565(image_data, (uint16_t *)src_data, width, height);
        free(image_data);
    }
    return OPEN_IMAGE_RET_OK;
}

static int open_image_write_yuv422s_file(const char *filename,
                                         uint16_t *src_data, uint32_t size,
                                         uint32_t width, uint32_t height,
                                         RawFmt_t type)
{
    FILE *file = file_fopen(filename, "wb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_yuv422s_failed_0,
                      P_INFO, "file open %s failed");
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    if(size < width * height * 2)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_yuv422s_failed_1,
                      P_INFO, "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *image_data = NULL;
    uint32_t image_size = width * height * 2;

    if(type == IMG_FMT_RGB565)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_yuv422s_failed_2,
                          P_INFO, "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        Video_TransRgb565ToYuyv((uint16_t *)src_data, image_data, width,
                                height);
    }
    else if(type == IMG_FMT_Y)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                          open_image_write_yuv422s_failed_2_1, P_INFO,
                          "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        fmt_conv_y_to_yuv422s((uint8_t *)src_data, image_data, width, height);
    }
    else
    {
        image_data = (uint8_t *)src_data;
    }

    file_fwrite(image_data, image_size, 1, file);
    if(type != IMG_FMT_YUV422S)
    {
        free(image_data);
    }
    file_fclose(file);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_write_y_file(const char *filename, uint16_t *src_data,
                                   uint32_t size, uint32_t width,
                                   uint32_t height, RawFmt_t type)
{
    FILE *file = file_fopen(filename, "wb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_yuv422s_failed_0,
                      P_INFO, "file open %s failed");
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    if(size < width * height)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_yuv422s_failed_1,
                      P_INFO, "file size too small");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *image_data = NULL;
    uint32_t image_size = width * height;

    if(type == IMG_FMT_RGB565)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_yuv422s_failed_2,
                          P_INFO, "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        fmt_conv_rgb565_to_y(src_data, image_data, width, height);
    }
    else if(type == IMG_FMT_YUV422S)
    {
        image_data = malloc(image_size);
        if(image_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                          open_image_write_yuv422s_failed_2_1, P_INFO,
                          "file buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        fmt_conv_yuv422s_to_y((uint8_t *)src_data, image_data, width, height);
    }
    else
    {
        image_data = (uint8_t *)src_data;
    }
    file_fwrite(image_data, image_size, 1, file);
    if(type != IMG_FMT_Y)
    {
        free(image_data);
    }
    file_fclose(file);
    return OPEN_IMAGE_RET_OK;
}

static int open_image_write_jpeg_file(const char *filename, uint16_t *src_img,
                                      uint32_t size, uint32_t width,
                                      uint32_t height, RawFmt_t type)
{
    FILE *file = file_fopen(filename, "wb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_jpeg_failed_0, P_INFO,
                      "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }

    uint8_t *img_data = NULL;
    uint32_t img_size = width * height * 2;
    if(type == IMG_FMT_RGB565)
    {
        img_data = malloc(img_size);
        if(img_data == NULL)
        {
            file_fclose(file);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_jpeg_failed_1,
                          P_INFO, "jpeg buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        Video_TransRgb565ToYuyv(src_img, img_data, width, height);
    }
    else
    {
        img_data = (uint8_t *)src_img;
        img_size = size;
    }

    uint32_t jpegSize = width * height * 2;
    uint8_t *jpegData = malloc(jpegSize);  // 预估大小
    if(jpegData == NULL)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_jpeg_failed_2, P_INFO,
                      "jpeg buffer allocate failed");
        return OPEN_IMAGE_MALLOC_ERR;
    }

    if(open_image_jpeg_enc(img_data, img_size, jpegData, &jpegSize, width,
                           height, 80, type == IMG_FMT_Y) != OPEN_IMAGE_RET_OK)
    {
        free(jpegData);
        file_fclose(file);
        if(type == IMG_FMT_RGB565)
        {
            free(img_data);
        }
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_jpeg_failed_3, P_INFO,
                      "JPEG encoding failed");
        return OPEN_IMAGE_CODEC_FAILED;
    }
    file_fwrite(jpegData, jpegSize, 1, file);
    file_fclose(file);
    free(jpegData);
    if(type == IMG_FMT_RGB565)
    {
        free(img_data);
    }
    return OPEN_IMAGE_RET_OK;
}

static int open_image_read_jpeg_file(const char *filename, uint16_t *src_img,
                                     uint32_t *size, uint32_t *width,
                                     uint32_t *height, RawFmt_t type)

{
    int ret = 0;
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_jpeg_failed_0, P_INFO,
                      "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    if(file_stat.st_size <= 0)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_jpeg_failed_1, P_INFO,
                      "file size is zero");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *jpegData = malloc(file_stat.st_size);
    if(jpegData == NULL)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_jpeg_failed_2, P_INFO,
                      "jpeg buffer allocate failed");
        return OPEN_IMAGE_MALLOC_ERR;
    }
    file_fread(jpegData, file_stat.st_size, 1, file);
    file_fclose(file);
    uint8_t *img_data = NULL;
    uint32_t img_size = *size;
    if(type != IMG_FMT_RGB565)
    {
        img_size = (*width) * (*height) * 2;
        img_data = malloc(img_size);
        if(img_data == NULL)
        {
            free(jpegData);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_jpeg_failed_3,
                          P_INFO, "jpeg buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
    }
    else
    {
        img_data = (uint8_t *)src_img;
    }
    ret = open_image_jpeg_dec(jpegData, file_stat.st_size, img_data, size,
                              width, height);
    if(ret != OPEN_IMAGE_RET_OK)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_jpeg_failed_4, P_INFO,
                      "JPEG decoding failed, ret = %d", ret);
    }
    if(type == IMG_FMT_YUV422S)
    {
        Video_TransRgb565ToYuyv((uint16_t *)img_data, (uint8_t *)src_img,
                                *width, *height);
        free(img_data);
    }
    else if(type == IMG_FMT_Y)
    {
        fmt_conv_rgb565_to_y((uint16_t *)img_data, (uint8_t *)src_img, *width,
                             *height);
    }
    free(jpegData);
    return ret;
}

static uint32_t open_image_read_png_file(const char *filename,
                                         uint16_t *rgb565Data, uint32_t *size,
                                         uint32_t *width, uint32_t *height)
{
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_png_failed_0, P_INFO,
                      "file open %s failed");
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    if(file_stat.st_size <= 0)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_png_failed_1, P_INFO,
                      "file size is zero");
        return OPEN_IMAGE_SIZE_ERR;
    }
    uint8_t *pngData = malloc(file_stat.st_size);
    if(pngData == NULL)
    {
        file_fclose(file);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_png_failed_2, P_INFO,
                      "png buffer allocate failed");
        return OPEN_IMAGE_MALLOC_ERR;
    }
    file_fread(pngData, file_stat.st_size, 1, file);
    file_fclose(file);
    uint32_t pngSize = file_stat.st_size;
    if(open_image_png_dec(pngData, pngSize, (uint8_t *)rgb565Data, size, width,
                          height) != OPEN_IMAGE_RET_OK)
    {
        free(pngData);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_png_failed_3, P_INFO,
                      "PNG decoding failed");
        return OPEN_IMAGE_CODEC_FAILED;
    }
    free(pngData);
    return OPEN_IMAGE_RET_OK;
}

static uint32_t open_image_get_buffer_size(uint32_t width, uint32_t height,
                                           RawFmt_t fmt)
{
    uint32_t size = 0;
    switch(fmt)
    {
        case IMG_FMT_RGB565: {
            size = width * height * 2;
            break;
        }
        case IMG_FMT_YUV422S: {
            size = width * height * 2;
            break;
        }
        case IMG_FMT_Y: {
            size = width * height;
            break;
        }
        default:
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                          open_image_get_buffer_size_invalid_0, P_INFO,
                          "Unsupported format");
            return 0;
    }
    return size;
}

static uint32_t open_image_get_stride(uint32_t width, uint32_t height,
                                      RawFmt_t fmt)
{
    uint32_t size = 0;
    switch(fmt)
    {
        case IMG_FMT_RGB565: {
            size = width * 2;
            break;
        }
        case IMG_FMT_YUV422S: {
            size = width * 2;
            break;
        }
        case IMG_FMT_Y: {
            size = width;
            break;
        }
        default:
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_get_stride_invalid_0,
                          P_INFO, "Unsupported format");
            return 0;
    }
    return size;
}

OpImg_t *open_image_create(uint32_t width, uint32_t height, RawFmt_t fmt,
                           uint8_t *data, uint32_t data_size)
{
    if(width == 0 || height == 0)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_create_invalid_0, P_INFO,
                      "Invalid width or height");
        return NULL;
    }

    if(fmt >= IMG_FMT_MAX)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_create_invalid_1, P_INFO,
                      "Invalid format");
        return NULL;
    }

    OpImg_t *img = malloc(sizeof(OpImg_t));
    if(img == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_create_invalid_2, P_INFO,
                      "malloc failed");
        return NULL;
    }
    uint32_t size = open_image_get_buffer_size(width, height, fmt);
    if(!data)
    {
        img->data = malloc(size);
        if(img->data == NULL)
        {
            free(img);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_create_invalid_3,
                          P_INFO, "malloc failed");
            return NULL;  // 内存分配失败
        }
        img->is_alloc = true;
    }
    else
    {
        if(data_size < size)
        {
            free(img);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_create_invalid_4,
                          P_INFO, "data size is too small");
            return NULL;
        }
        img->data = data;
        img->size = data_size;
        img->is_alloc = false;
    }

    img->size = size;
    img->type = fmt;
    img->width = width;
    img->height = height;
    img->stride = open_image_get_stride(width, height, fmt);
    return img;
}

void open_image_destroy(OpImg_t *img)
{
    if(img == NULL)
    {
        return;
    }
    if((img->data != NULL) && (img->is_alloc))
    {
        free(img->data);
        img->data = NULL;
    }
    free(img);
}

static int open_image_get_file_info(char *filename, OPEN_IMAGE_INFO *info)
{
    int len = strlen(filename);
    char *dot = NULL;

    for(int i = len - 1; i >= 0; i--)
    {
        if(filename[i] == '.')
        {
            dot = &filename[i];
            break;
        }
    }

    if(!dot || dot == filename)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_get_file_info_invalid_1,
                      P_ERROR, "file:%s, dot:%s", filename, dot ? dot : "NULL");
        return -1;
    }

    char ext[5] = {0};
    strncpy(ext, dot + 1, sizeof(ext) - 1);
    for(int i = 0; ext[i]; i++)
    {
        ext[i] = tolower(ext[i]);
    }

    if(strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0)
    {
        info->type = OPEN_IMAGE_TYPE_JPEG;
    }
    else if(strcmp(ext, "png") == 0)
    {
        info->type = OPEN_IMAGE_TYPE_PNG;
    }
    else if(strcmp(ext, "bmp") == 0)
    {
        info->type = OPEN_IMAGE_TYPE_BMP;
    }
    else if(strcmp(ext, "rgb") == 0)
    {
        info->type = OPEN_IMAGE_TYPE_RAW;
        info->fmt = IMG_FMT_RGB565;
    }
    else if(strcmp(ext, "yuv") == 0)
    {
        info->type = OPEN_IMAGE_TYPE_RAW;
        info->fmt = IMG_FMT_YUV422S;
    }
    else if(strcmp(ext, "y") == 0)
    {
        info->type = OPEN_IMAGE_TYPE_RAW;
        info->fmt = IMG_FMT_Y;
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_get_file_info_invalid_0,
                      P_ERROR, "Unsupported file format");
        return -2;  // 不支持的文件格式
    }

    return 0;
}

int open_image_read(char *filename, OpImg_t *img, OPEN_IMAGE_INFO *info)
{
    int ret = 0;
    if(filename == NULL || img == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_0, P_INFO,
                      "Invalid img or info pointer");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    OPEN_IMAGE_INFO img_info = {0};
    if(info)
    {
        memcpy(&img_info, info, sizeof(OPEN_IMAGE_INFO));
    }
    else
    {
        ret = open_image_get_file_info(filename, &img_info);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_1, P_INFO,
                          "file info is invalid, %d", ret);
            return OPEN_IMAGE_INVLID_PROC_INFO;
        }
    }

    if(img_info.type == OPEN_IMAGE_TYPE_BMP)
    {
        if(img->type != IMG_FMT_RGB565)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_bmp_invalid,
                          P_INFO, "only support RGB565 read bmp");
            return OPEN_IMAGE_NOT_SUPPORT;
        }
        ret = open_image_read_bmp_file(filename, (uint16_t *)img->data,
                                       img->size, &img->width, &img->height);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_bmp,
                          P_INFO, "read bmp file failed");
            return ret;
        }
        img->size = img->width * img->height * 2;
        img->type = IMG_FMT_RGB565;
        img->stride =
            open_image_get_stride(img->width, img->height, IMG_FMT_RGB565);
        return OPEN_IMAGE_RET_OK;
    }
    else if(img_info.type == OPEN_IMAGE_TYPE_RAW)
    {
        if(img_info.fmt == IMG_FMT_RGB565)
        {
            ret = open_image_read_rgb565_file(filename, (uint16_t *)img->data,
                                              img->size, img->width,
                                              img->height, img->type);
            if(ret != OPEN_IMAGE_RET_OK)
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_rgb565,
                              P_INFO, "read rgb565 file failed");
                return ret;
            }
        }
        else if(img_info.fmt == IMG_FMT_YUV422S)
        {
            ret = open_image_read_yuv422s_file(filename, (uint16_t *)img->data,
                                               img->size, img->width,
                                               img->height, img->type);
            if(ret != OPEN_IMAGE_RET_OK)
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                              open_image_read_invalid_yuv422s, P_INFO,
                              "read yuv422s file failed");
                return ret;
            }
        }
        else if(img_info.fmt == IMG_FMT_Y)
        {
            ret = open_image_read_y_file(filename, img->data, img->size,
                                         img->width, img->height, img->type);
            if(ret != OPEN_IMAGE_RET_OK)
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_y,
                              P_INFO, "read y file failed");
                return ret;
            }
        }
        else
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_raw_fmt,
                          P_INFO, "Unsupported raw format");
            return OPEN_IMAGE_NOT_SUPPORT;
        }
        img->size =
            open_image_get_buffer_size(img->width, img->height, img_info.fmt);
        img->stride =
            open_image_get_stride(img->width, img->height, img_info.fmt);
    }
    else if(img_info.type == OPEN_IMAGE_TYPE_PNG)
    {
        if(img->type != IMG_FMT_RGB565)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_png_invalid,
                          P_INFO, "only support RGB565 read png");
            return OPEN_IMAGE_NOT_SUPPORT;
        }
        ret = open_image_read_png_file(filename, (uint16_t *)img->data,
                                       &img->size, &img->width, &img->height);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_png,
                          P_INFO, "read png file failed");
            return ret;
        }
    }
    else if(img_info.type == OPEN_IMAGE_TYPE_JPEG)
    {
        ret = open_image_read_jpeg_file(filename, (uint16_t *)img->data,
                                        &img->size, &img->width, &img->height,
                                        img->type);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid_jpg,
                          P_INFO, "read jpg file failed");
            return ret;
        }
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_read_invalid, P_INFO,
                      "Invalid image type info");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    return OPEN_IMAGE_RET_OK;
}

int open_image_save(char *filename, OpImg_t *img, OPEN_IMAGE_INFO *info)
{
    int ret = 0;
    if(filename == NULL || img == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_save_invalid_0, P_INFO,
                      "Invalid img or info pointer");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    OPEN_IMAGE_INFO img_info = {0};
    if(info)
    {
        memcpy(&img_info, info, sizeof(OPEN_IMAGE_INFO));
    }
    else
    {
        ret = open_image_get_file_info(filename, &img_info);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_save_invalid_1, P_INFO,
                          "file info is invalid, %d", ret);
            return OPEN_IMAGE_INVLID_PROC_INFO;
        }
    }
    if(img_info.type == OPEN_IMAGE_TYPE_BMP)
    {
        if(img->type != IMG_FMT_RGB565)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_save_bmp_invalid,
                          P_INFO, "only support RGB565 save bmp");
            return OPEN_IMAGE_NOT_SUPPORT;
        }
        return open_image_write_bmp_file(filename, (uint16_t *)img->data,
                                         img->size, img->width, img->height);
    }
    else if(img_info.type == OPEN_IMAGE_TYPE_RAW)
    {
        if(img_info.fmt == IMG_FMT_RGB565)
        {
            return open_image_write_rgb565_file(filename, (uint16_t *)img->data,
                                                img->size, img->width,
                                                img->height, img->type);
        }
        else if(img_info.fmt == IMG_FMT_YUV422S)
        {
            return open_image_write_yuv422s_file(
                filename, (uint16_t *)img->data, img->size, img->width,
                img->height, img->type);
        }
        else if(img_info.fmt == IMG_FMT_Y)
        {
            return open_image_write_y_file(filename, (uint16_t *)img->data,
                                           img->size, img->width, img->height,
                                           img->type);
        }
        else
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_save_invalid_raw_fmt,
                          P_INFO, "Unsupported raw format");
            return OPEN_IMAGE_NOT_SUPPORT;
        }
    }
    else if(img_info.type == OPEN_IMAGE_TYPE_JPEG)
    {
        return open_image_write_jpeg_file(filename, (uint16_t *)img->data,
                                          img->size, img->width, img->height,
                                          img->type);
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_write_invalid, P_INFO,
                      "Invalid image type info");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    return OPEN_IMAGE_RET_OK;
}

int open_image_scale(OpImg_t *in_img, OpImg_t *out_img, CropInfo_t *info)
{
    VIDEO_IMAGE_BUF ibuf = {0};
    VIDEO_IMAGE_BUF obuf = {0};
    VIDEO_REGION region = {0};

    if(in_img == NULL || out_img == NULL || info == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_crop_invalid_0, P_INFO,
                      "Invalid parameters");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if(info->x + info->width > in_img->width ||
       info->y + info->height > in_img->height)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_crop_invalid_1, P_INFO,
                      "Crop parameters out of bounds");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if((in_img->type != IMG_FMT_RGB565) || (out_img->type != IMG_FMT_RGB565))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_crop_invalid_2, P_INFO,
                      "scale or crop only support RGB565 format");
        return OPEN_IMAGE_NOT_SUPPORT;
    }

    ibuf.eFmt = VIDEO_COLOR_FMT_RGB565;
    ibuf.uWidth = in_img->width;
    ibuf.uHeight = in_img->height;
    ibuf.pData[0] = in_img->data;

    obuf.eFmt = VIDEO_COLOR_FMT_RGB565;
    obuf.uWidth = out_img->width;
    obuf.uHeight = out_img->height;
    obuf.pData[0] = out_img->data;

    region.uX = info->x;
    region.uY = info->y;
    region.uWidth = info->width;
    region.uHeight = info->height;

    Video_ScaleImage(&ibuf, &region, &obuf, NULL);

    return OPEN_IMAGE_RET_OK;
}

int open_image_rotate_mirror(OpImg_t *in_img, OpImg_t *out_img, bool mirror,
                             bool rotate)
{
    VIDEO_IMAGE_BUF ibuf = {0};
    VIDEO_IMAGE_BUF obuf = {0};
    VIDEO_REGION region = {0};

    if(in_img == NULL || out_img == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_rotate_invalid_0, P_INFO,
                      "Invalid parameters");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if(((out_img->width != in_img->height) ||
        (out_img->height != in_img->width)) &&
       rotate)
    {
        ECPLAT_PRINTF(
            UNILOG_PLAT_MEDIA, open_image_rotate_invalid_1, P_INFO,
            "output image size not equal input image size after rotate");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    else if(((out_img->width != in_img->width) ||
             (out_img->height != in_img->height)) &&
            mirror)
    {
        ECPLAT_PRINTF(
            UNILOG_PLAT_MEDIA, open_image_rotate_invalid_2, P_INFO,
            "output image size not equal input image size after mirror");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if((in_img->type != IMG_FMT_RGB565) || (out_img->type != IMG_FMT_RGB565))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_rotate_invalid_3, P_INFO,
                      "scale or crop only support RGB565 format");
        return OPEN_IMAGE_NOT_SUPPORT;
    }

    ibuf.eFmt = VIDEO_COLOR_FMT_RGB565;
    ibuf.uWidth = in_img->width;
    ibuf.uHeight = in_img->height;
    ibuf.pData[0] = in_img->data;

    obuf.eFmt = VIDEO_COLOR_FMT_RGB565;
    obuf.uWidth = out_img->width;
    obuf.uHeight = out_img->height;
    obuf.pData[0] = out_img->data;

    region.uX = 0;
    region.uY = 0;
    region.uWidth = in_img->width;
    region.uHeight = in_img->height;

    VIDEO_EFFECT img_effect = {0};
    img_effect.uMirror = mirror ? 1 : 0;
    img_effect.uRotate = rotate ? 1 : 0;

    Video_ScaleImage(&ibuf, &region, &obuf, &img_effect);
    return OPEN_IMAGE_RET_OK;
}

int open_image_conv_fmt(OpImg_t *in_img, OpImg_t *out_img)
{
    if(in_img == NULL || out_img == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_conv_fmt_invalid_0, P_INFO,
                      "Invalid parameters");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if(in_img->width != out_img->width || in_img->height != out_img->height)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_conv_fmt_invalid_1, P_INFO,
                      "Input and output image sizes do not match");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if(in_img->type == IMG_FMT_RGB565 && out_img->type == IMG_FMT_YUV422S)
    {
        Video_TransRgb565ToYuyv((uint16_t *)in_img->data, out_img->data,
                                in_img->width, in_img->height);
    }
    else if(in_img->type == IMG_FMT_YUV422S && out_img->type == IMG_FMT_RGB565)
    {
        Video_TransYuyvToRgb565(in_img->data, (uint16_t *)out_img->data,
                                in_img->width, in_img->height);
    }
    else if((in_img->type == IMG_FMT_RGB565) && (out_img->type == IMG_FMT_Y))
    {
        fmt_conv_rgb565_to_y((uint16_t *)in_img->data, out_img->data,
                             in_img->width, in_img->height);
    }
    else if((in_img->type == IMG_FMT_Y) && (out_img->type == IMG_FMT_RGB565))
    {
        fmt_conv_y_to_rgb565(in_img->data, (uint16_t *)out_img->data,
                             in_img->width, in_img->height);
    }
    else if((in_img->type == IMG_FMT_YUV422S) && (out_img->type == IMG_FMT_Y))
    {
        fmt_conv_yuv422s_to_y(in_img->data, out_img->data, in_img->width,
                              in_img->height);
    }
    else if((in_img->type == IMG_FMT_Y) && (out_img->type == IMG_FMT_RGB565))
    {
        fmt_conv_y_to_yuv422s(in_img->data, out_img->data, in_img->width,
                              in_img->height);
    }
    else if(in_img->type == out_img->type)
    {
        memcpy(out_img->data, in_img->data, in_img->size);
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_conv_fmt_invalid_2, P_INFO,
                      "Unsupported format conversion");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    return OPEN_IMAGE_RET_OK;
}

int open_image_decode(OpImg_t *img, uint8_t *buffer, uint32_t *size,
                      OPEN_IMAGE_INFO *info)
{
    uint32_t image_size = 0;
    uint8_t *image_data = NULL;
    int ret = 0;
    if(img == NULL || buffer == NULL || size == NULL || info == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_decode_invalid_0, P_INFO,
                      "Invalid parameters");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }

    if(img->type != IMG_FMT_RGB565)
    {
        image_data = malloc(img->size);
        if(image_data == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_decode_invalid_1,
                          P_INFO, "jpeg buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        image_size = img->size;
    }
    else
    {
        image_data = (uint8_t *)img->data;
        image_size = img->size;
    }

    if(info->type == OPEN_IMAGE_TYPE_JPEG)
    {
        ret = open_image_jpeg_dec(buffer, *size, image_data, &image_size,
                                  &img->width, &img->height);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_decode_invalid_3,
                          P_INFO, "jpeg codec failed");
        }
    }
    else if(info->type == OPEN_IMAGE_TYPE_PNG)
    {
        ret = open_image_png_dec(buffer, *size, image_data, &image_size,
                                 &img->width, &img->height);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_decode_invalid_4,
                          P_INFO, "png codec failed");
        }
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_decode_invalid_5, P_INFO,
                      "Unsupported image type");
        return OPEN_IMAGE_NOT_SUPPORT;
    }
    if(img->type != IMG_FMT_RGB565)
    {
        Video_TransRgb565ToYuyv((uint16_t *)image_data, img->data, img->width,
                                img->height);
        free(image_data);
    }
    return ret;
}

int open_image_encode(OpImg_t *img, uint8_t *buffer, uint32_t *size,
                      OPEN_IMAGE_INFO *info, uint8_t quality)
{
    int ret = 0;
    if(img == NULL || buffer == NULL || size == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_encode_invalid_0, P_INFO,
                      "Invalid parameters");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    if(info->type != OPEN_IMAGE_TYPE_JPEG)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_encode_invalid_1, P_INFO,
                      "only jpeg format support");
        return OPEN_IMAGE_NOT_SUPPORT;
    }
    uint8_t *img_data = NULL;
    uint32_t img_size = img->width * img->height * 2;
    if(img->type == IMG_FMT_RGB565)
    {
        img_data = malloc(img_size);
        if(img_data == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_encode_invalid_2,
                          P_INFO, "jpeg buffer allocate failed");
            return OPEN_IMAGE_MALLOC_ERR;
        }
        Video_TransRgb565ToYuyv((uint16_t *)img->data, img_data, img->width,
                                img->height);
        img_size = img->width * img->height * 2;
    }
    else
    {
        img_data = (uint8_t *)img->data;
        img_size = img->size;
    }

    ret = open_image_jpeg_enc(img_data, img_size, buffer, size, img->width,
                              img->height, quality, img->type == IMG_FMT_Y);
    if(ret != OPEN_IMAGE_RET_OK)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_encode_invalid_3, P_INFO,
                      "jpeg encode failed");
        if(img->type == IMG_FMT_RGB565)
        {
            free(img_data);
        }
        ret = OPEN_IMAGE_CODEC_FAILED;
    }

    if(img->type == IMG_FMT_RGB565)
    {
        free(img_data);
    }

    return ret;
}

uint8_t *open_image_get_data(OpImg_t *img) { return img->data; }

uint32_t open_image_get_size(OpImg_t *img) { return img->size; }

void open_image_get_resolution(OpImg_t *img, uint32_t *width, uint32_t *height)
{
    *width = img->width;
    *height = img->height;
}

int open_image_show(OpImg_t *img, bool rotate)
{
#ifndef FEATURE_HAL_SCREEN_ENABLE
    return OPEN_IMAGE_NOT_SUPPORT;
#else
    uint32_t display_width = 0;
    uint32_t display_height = 0;
    bool scale_image = false;
    if(rotate)
    {
        display_width = LCD_HEIGHT;
        display_height = LCD_WIDTH;
    }
    else
    {
        display_width = LCD_WIDTH;
        display_height = LCD_HEIGHT;
    }

    int ret = 0;
    if(img == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_show_invalid_0, P_INFO,
                      "Invalid image pointer");
        return OPEN_IMAGE_INVLID_PARAMETERS;
    }
    if(img->type != IMG_FMT_RGB565)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_show_invalid_1, P_INFO,
                      "Invalid image format");
        return OPEN_IMAGE_NOT_SUPPORT;
    }

    if((img->width <= display_width) && (img->height <= display_height))
    {
#ifdef FEATURE_SUBSYS_DISPLAY_ENABLE
        DisplayRegion_t region = {0};
        region.x = (display_width - img->width) / 2;
        region.y = (display_height - img->height) / 2;
        region.width = img->width;
        region.height = img->height;
        displayWriteData(img->data, img->size, &region);
#else
        ScrWriteParam_t info = {0};
        info.start_x = (display_width - img->width) / 2;
        info.start_y = (display_height - img->height) / 2;
        info.width = img->width;
        info.height = img->height;
        info.data = open_image_get_data(img);
        info.size = open_image_get_size(img);
        Device_write("dev:/lcd", &info, sizeof(ScrWriteParam_t));
#endif
    }
    else
    {
        uint32_t show_width = 0;
        uint32_t show_height = 0;
        if(img->width * display_height > img->height * display_width)
        {
            show_width = display_width;
            show_height = (img->height * display_width) / img->width;
        }
        else
        {
            show_height = display_height;
            show_width = (img->width * display_height) / img->height;
        }
        if((show_width != img->width) || (show_height != img->height))
        {
            scale_image = true;
        }
        OpImg_t *show_img = NULL;
        if(scale_image)
        {
            show_img = open_image_create(show_width, show_height,
                                         IMG_FMT_RGB565, NULL, 0);
            if(show_img == NULL)
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_show_invalid_2,
                              P_INFO, "show image create failed");
                return OPEN_IMAGE_MALLOC_ERR;
            }
            CropInfo_t crop_info = {0};
            crop_info.x = 0;
            crop_info.y = 0;
            crop_info.width = img->width;
            crop_info.height = img->height;
            ret = open_image_scale(img, show_img, &crop_info);
            if(ret != 0)
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_show_invalid_3,
                              P_INFO, "scale show image failed");
                return OPEN_IMAGE_CODEC_FAILED;
            }
        }
        else
        {
            show_img = img;
        }

#ifdef FEATURE_SUBSYS_DISPLAY_ENABLE
        DisplayRegion_t region = {0};
        region.x = (display_width - show_width) / 2;
        region.y = (display_height - show_height) / 2;
        region.width = show_width;
        region.height = show_height;
        displayWriteData(open_image_get_data(show_img),
                         open_image_get_size(show_img), &region);
#else
        ScrWriteParam_t info = {0};
        info.start_x = (display_width - show_width) / 2;
        info.start_y = (display_height - show_height) / 2;
        info.width = show_width;
        info.height = show_height;
        info.data = open_image_get_data(show_img);
        info.size = open_image_get_size(show_img);
        Device_write("dev:/lcd", &info, sizeof(ScrWriteParam_t));
#endif
        if(scale_image)
        {
            open_image_destroy(show_img);
        }
    }

    return OPEN_IMAGE_RET_OK;
#endif
}

int open_image_get_jpgfile_info(char *filename, JpegInfo_t *info)
{
    void *jpg_codec = NULL;
    JPEG_INFO jpeg_info;
    uint8_t *jpeg_data = NULL;
    uint32_t jpeg_size = 0;
    FILE *file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_get_jpgfile_info_failed0,
                      P_INFO, "file open %s failed", filename);
        return OPEN_IMAGE_FILE_OPEN_FAILED;
    }
    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    jpeg_size = file_stat.st_size;
    jpeg_data = malloc(jpeg_size);
    if(jpeg_data == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_image_get_jpgfile_info_failed1,
                      P_INFO, "jpeg data allocate failed");
        return OPEN_IMAGE_MALLOC_ERR;
    }
    file_fread(jpeg_data, jpeg_size, 1, file);
    file_fclose(file);

    jpg_codec = JpegD_Create();
    if(JpegD_DecodeInfo(jpg_codec, jpeg_data, jpeg_size, &jpeg_info) != 0)
    {
        JpegD_Destroy(jpg_codec);
        free(jpeg_data);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    info->width = jpeg_info.uWidth;
    info->height = jpeg_info.uHeight;
    JpegD_Destroy(jpg_codec);
    free(jpeg_data);
    return OPEN_IMAGE_RET_OK;
}

int open_image_get_jpgdata_info(uint8_t *buffer, uint32_t size,
                                JpegInfo_t *info)
{
    void *jpg_codec = NULL;
    JPEG_INFO jpeg_info;
    jpg_codec = JpegD_Create();
    if(JpegD_DecodeInfo(jpg_codec, buffer, size, &jpeg_info) != 0)
    {
        JpegD_Destroy(jpg_codec);
        return OPEN_IMAGE_CODEC_FAILED;
    }
    info->width = jpeg_info.uWidth;
    info->height = jpeg_info.uHeight;
    JpegD_Destroy(jpg_codec);
    return OPEN_IMAGE_RET_OK;
}
