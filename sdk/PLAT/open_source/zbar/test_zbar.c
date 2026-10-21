#include <stdio.h>
#include <string.h>

#include "zbar.h"
#include "test_img_buff.h"

/* adapted from v4l2 spec */
#define fourcc(a, b, c, d)                      \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) |     \
     ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

unsigned char rgb[8*8*3] = {
    255, 255, 255,  176, 238, 176,   94, 220,  94,   60, 213,  60,
     60, 213,  60,   94, 220,  94,  176, 238, 176,  255, 255, 255,
    176, 238, 176,   46, 210,  46,   10, 102,  10,   17, 204,  17,
     17, 204,  17,   10, 102,  10,   46, 210,  46,  176, 238, 176,
     94, 220,  94,   19, 204,  19,    9, 102,   9,   17, 204,  17,
     17, 204,  17,    9, 102,   9,   19, 204,  19,   94, 220,  94,
     60, 213,  60,   17, 204,  17,    9, 102,   9,   17, 204,  17,
     17, 204,  17,    9, 102,   9,   17, 204,  17,   60, 213,  60,
     60, 213,  60,   17, 204,  17,   17, 204,  17,   17, 204,  17,
     17, 204,  17,   17, 204,  17,   17, 204,  17,   60, 213,  60,
     94, 220,  94,   10, 102,  10,   17, 204,  17,   17, 204,  17,
     17, 204,  17,   17, 204,  17,   10, 102,  10,   94, 220,  94,
    176, 238, 176,   46, 210,  46,   10, 102,  10,    9, 102,   9,
      9, 102,   9,   10, 102,  10,   46, 210,  46,  176, 238, 176,
    255, 255, 255,  176, 238, 176,   94, 220,  94,   60, 213,  60,
     60, 213,  60,   94, 220,  94,  176, 238, 176,  255, 255, 255,
};



void test_zbar(void* raw, int width, int height)
{
    zbar_image_scanner_t *scanner = NULL;
    int ret = 0;

    zbar_log("------ test_zbar ------\r\n");

    /* create a reader */
    scanner = zbar_image_scanner_create();
    if(NULL == scanner)
    {
        zbar_log("zbar_image_scanner_create fail NULL, return \r\n");
        return;
    }

    /* configure the reader */
    ret = zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
    if(0 != ret)
    {
        zbar_log("zbar_image_scanner_set_config fail, ret = %d \r\n", ret);
        return;
    }

    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    if(NULL == image)
    {
        zbar_log("zbar_image_create fail NULL, return \r\n");
        return;
    }
    zbar_image_set_format(image, fourcc('Y','8','0','0'));
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

    /* scan the image for barcodes */
    ret = zbar_scan_image(scanner, image);
    zbar_log("zbar_scan_image ret = %d \r\n", ret);

    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol))
    {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        if(NULL != data)
        {
            zbar_log("decoded: %s \r\n", zbar_get_symbol_name(typ));
            zbar_log("symbol len[%d] data: %s \r\n", strlen(data), data);
        }
    }

    /* clean up */
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);

    zbar_log("------ test_zbar end ------\r\n");
}   

void test_rgb(void)
{

    zbar_log("------ test_rgb ------\r\n");

    zbar_image_t *img = zbar_image_create();
    zbar_image_set_size(img, 640, 480);
    zbar_image_set_format(img, fourcc('R','G','B','3'));
    zbar_image_set_data(img, rgb, sizeof(rgb), NULL);

    zbar_log("------ zbar_image_convert ------\r\n");

    zbar_image_t *convert_img = zbar_image_convert(img, fourcc('Y','8','0','0'));
    if(NULL != convert_img)
    {
        zbar_log("convert_img: %d x %d (%lx) %08lx\n",
                zbar_image_get_width(convert_img),
                zbar_image_get_height(convert_img),
                zbar_image_get_data_length(convert_img),
                zbar_image_get_format(convert_img));osDelay(1000);

        test_zbar((void* )zbar_image_get_data(convert_img), zbar_image_get_width(convert_img), zbar_image_get_height(convert_img));
    }

    zbar_log("------ test_rgb end------\r\n");osDelay(1000);

    zbar_image_destroy(img);
}

void zbar_demo(void)
{
    test_zbar((void* )data_buf, 280, 280);
}

