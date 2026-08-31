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

#include <stdbool.h>
#include <stdint.h>

typedef enum RawFmt_
{
    IMG_FMT_RGB565,
    IMG_FMT_YUV422S,  // equal YUYV
    IMG_FMT_Y,
    IMG_FMT_MAX
} RawFmt_t;

typedef enum
{
    OPEN_IMAGE_TYPE_RAW,
    OPEN_IMAGE_TYPE_BMP,
    OPEN_IMAGE_TYPE_JPEG,
    OPEN_IMAGE_TYPE_PNG,
    OPEN_IMAGE_TYPE_MAX
} OPEN_IMAGE_TYPE;

typedef struct open_image_info
{
    OPEN_IMAGE_TYPE type;
    RawFmt_t fmt;
} OPEN_IMAGE_INFO;

typedef enum
{
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

typedef enum
{
    OPEN_IMAGE_RET_OK,                   // 成功
    OPEN_IMAGE_NOT_SUPPORT = -1,         // 暂不支持
    OPEN_IMAGE_MALLOC_ERR = -2,          // 内存分配失败
    OPEN_IMAGE_FILE_NOT_FOUND_ERR = -3,  // 文件未找到
    OPEN_IMAGE_INVLID_PARAMETERS = -4,   // 无效参数
    OPEN_IMAGE_INVLID_PROC_INFO = -5,    // 处理信息不足
    OPEN_IMAGE_CODEC_FAILED = -6,        // 编解码错误
    OPEN_IMAGE_OUT_OF_MEMORY = -7,       // 内存不足
    OPEN_IMAGE_FILE_OPEN_FAILED = -8,    // 文件无法打开
    OPEN_IMAGE_SIZE_ERR = -9,            // 数据长度不匹配
} OPEN_IMAGE_ERR;

typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} CropInfo_t;

typedef struct OpImg_
{
    uint8_t *data;
    uint32_t size;
    RawFmt_t type;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    bool is_alloc;
} OpImg_t;

typedef struct JpegInfo_
{
    uint32_t width;
    uint32_t height;
} JpegInfo_t;

/**
 * @brief 创建一个新的图像对象。
 *
 * 该函数分配内存并初始化一个 `OpImg_t`
 * 结构体，用于表示一个图像对象。处理完成后，需要调用open_image_destroy手动释放图像资源
 *
 * @param width 图像的宽度，单位为像素。
 * @param height 图像的高度，单位为像素。
 * @param fmt 图像格式。
 * @param data
 * 图像数据指针。如果为空，则内部申请内存。如果不为空，图像内存在外部管理，销毁时不会处理内存。
 * @param size 图像数据大小。
 * @return OpImg_t* 指向新创建的图像对象的指针，如果内存分配失败则返回 NULL。
 */
OpImg_t *open_image_create(uint32_t width, uint32_t height, RawFmt_t fmt,
                           uint8_t *data, uint32_t data_size);

/**
 * @brief 销毁一个图像对象。
 *
 * 该函数释放 `OpImg_t` 结构体及其关联的图像数据所占用的内存。
 *
 * @param img 指向要销毁的 `OpImg_t` 结构体的指针。
 */
void open_image_destroy(OpImg_t *img);

/**
 * @brief 从文件中读取图像数据。
 *
 * 该函数打开指定的图像文件，读取图像数据并填充到一个新的 `OpImg_t`
 * 结构体中。处理完成后，需要调用open_image_destroy手动释放图像资源
 *
 * @param filename
 * 要读取的图像文件的路径。默认通过扩展名指定保存图像文件的格式。可以输入info强制指定格式。
 * @param img 指向 `OpImg_t`
 * 结构体的指针，读取到的图像数据。注意图像尺寸要超过文件本身，否则读取会失败。
 * @param info 指向 `OPEN_IMAGE_INFO`
 * 结构体的指针，用于存储图像的类型信息。如果为空，则从文件名中获取保存文件类型。如果不为空，以该信息为准，无论文件名是什么扩展名。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。参考OPEN_IMAGE_ERR
 */
int open_image_read(char *filename, OpImg_t *img, OPEN_IMAGE_INFO *info);

/**
 * @brief 将图像数据保存到文件中。
 *
 * 该函数将 `OpImg_t` 结构体中存储的图像数据保存到指定的文件中。
 *
 * @param filename
 * 要保存的图像文件的路径。通过文件名指定保存格式。
 * @param img 指向 `OpImg_t` 结构体的指针，包含要保存的图像数据。
 * @param info 指向 `OPEN_IMAGE_INFO`
 * 结构体的指针，指定保存信息，如果为空，则从文件名中获取保存文件类型。如果不为空，以该信息为准，无论文件名是什么扩展名。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。参考OPEN_IMAGE_ERR
 */
int open_image_save(char *filename, OpImg_t *img, OPEN_IMAGE_INFO *info);

/**
 * @brief 在指定屏幕上显示图像。
 *
 * 该函数将 `OpImg_t` 结构体中存储的图像数据显示在指定 ID
 * 的屏幕上。只支持RGB565格式图像。
 *
 * @param img 指向 `OpImg_t` 结构体的指针，包含要显示的图像数据。
 * @param rotate 是否旋转图像，true表示旋转90度，false表示不旋转。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。参考OPEN_IMAGE_ERR
 */
int open_image_show(OpImg_t *img, bool rotate);

/**
 * @brief 对图像进行裁剪操作。
 *
 * 该函数根据 `CropInfo_t`
 * 结构体中指定的区域，从输入图像中裁剪出一部分图像数据并缩放到输出图像中。
 * 只支持RGB565格式图像。
 *
 * @param in_img 指向输入 `OpImg_t` 结构体的指针，包含原始图像数据。
 * @param out_img 指向输出 `OpImg_t`
 * 结构体的指针，裁剪后的图像会缩放到并存储的图像数据。
 * @param info 指向 `CropInfo_t` 结构体的指针，包含从输入图像裁剪区域的信息。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。参考OPEN_IMAGE_ERR
 */
int open_image_scale(OpImg_t *in_img, OpImg_t *out_img, CropInfo_t *info);

/**
 * @brief 对图像进行旋转操作。
 *
 * 该函数将输入图像数据旋转90度或者镜像，并将结果存储到输出图像中。
 * 只支持RGB565格式图像。
 * @param in_img 指向输入 `OpImg_t` 结构体的指针，包含原始图像数据。
 * @param out_img 指向输出 `OpImg_t`
 * 结构体的指针，旋转或者镜像后的图像将存储在此。
 * @param mirror 镜像操作，true表示镜像，false表示不镜像。
 * @param rotate 旋转操作，true表示旋转90度，false表示不旋转。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。参考OPEN_IMAGE_ERR
 */
int open_image_rotate_mirror(OpImg_t *in_img, OpImg_t *out_img, bool mirror,
                             bool rotate);

/**
 * @brief 对输入图像进行格式转换，并将结果存储到输出图像中。
 *
 * 该函数将输入图像 `in_img` 的格式转换为 `out_img` 所指定的格式，
 * 转换后的图像数据会存储在 `out_img` 中。`out_img` 需提前分配好内存。
 *
 * @param in_img 指向输入 `OpImg_t` 结构体的指针，包含原始图像数据。
 * @param out_img 指向输出 `OpImg_t` 结构体的指针，存储格式转换后的图像数据。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。参考OPEN_IMAGE_ERR
 */
int open_image_conv_fmt(OpImg_t *in_img, OpImg_t *out_img);

/**
 * @brief 解码图像数据。
 *
 * 该函数对输入的图像缓冲区中的数据进行解码，并返回一个新的 `OpImg_t` 结构体。
 *
 * @param img 指向 `OpImg_t` 结构体的指针，解码的后的图像对象。
 * @param buffer 指向包含编码图像数据的缓冲区的指针。
 * @param size 指向 `uint32_t`
 * 类型的指针，用于存储输入缓冲区的大小，函数返回时可能会更新为实际使用的大小。
 * @param info `OPEN_IMAGE_INFO` 结构体，包含图像的类型信息。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。
 */
int open_image_decode(OpImg_t *img, uint8_t *buffer, uint32_t *size,
                      OPEN_IMAGE_INFO *info);

/**
 * @brief 编码图像数据。
 *
 * 该函数将 `OpImg_t`
 * 结构体中存储的图像数据进行编码，并将结果存储到指定的缓冲区中。
 *
 * @param img 指向 `OpImg_t` 结构体的指针，包含要编码的图像数据。
 * @param buffer 指向用于存储编码后图像数据的缓冲区的指针。
 * @param size 指向 `uint32_t`
 * 类型的指针，用于存储缓冲区的大小，函数返回时会更新为实际编码后的数据大小。
 * @param info `OPEN_IMAGE_INFO` 结构体，包含图像的类型信息。
 * @param quality 编码质量，范围为 0-100，值越大表示质量越好。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。
 */
int open_image_encode(OpImg_t *img, uint8_t *buffer, uint32_t *size,
                      OPEN_IMAGE_INFO *info, uint8_t quality);

/**
 * @brief 获取JPEG文件的编码信息。
 *
 * 该函数用于获取JPEG文件的分辨率等编码信息
 *
 * @param file 指向JPEG文件路径的字符串指针。
 * @param info 指向JpegInfo_t结构体的指针，用于存储获取到的JPEG编码信息。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。
 */
int open_image_get_jpgfile_info(char* file, JpegInfo_t *info);

/**
 * @brief 获取JPEG数据的编码信息。
 *
 * 该函数用于获取JPEG数据的分辨率等编码信息
 *
 * @param buffer 指向JPEG数据的指针。
 * @param size 指向JPEG数据大小的指针。
 * @param info 指向JpegInfo_t结构体的指针，用于存储获取到的JPEG编码信息。
 * @return int 操作结果，成功返回 0，失败返回非 0 值。
 */
int open_image_get_jpgdata_info(uint8_t *buffer, uint32_t size, JpegInfo_t *info);

/**
 * @brief 获取图像对象的数据指针。
 *
 * 该函数返回指定图像对象中存储图像数据的指针。
 *
 * @param img 指向 `OpImg_t` 结构体的指针，代表要操作的图像对象。
 * @return uint8_t* 指向图像数据的指针，如果 `img` 为 NULL 则返回 NULL。
 */
uint8_t *open_image_get_data(OpImg_t *img);

/**
 * @brief 获取图像对象的数据大小。
 *
 * 该函数返回指定图像对象中存储的图像数据的大小。
 *
 * @param img 指向 `OpImg_t` 结构体的指针，代表要操作的图像对象。
 * @return uint32_t 图像数据的大小（字节），如果 `img` 为 NULL 则返回 0。
 */
uint32_t open_image_get_size(OpImg_t *img);

/**
 * @brief 获取图像对象的分辨率。
 *
 * @param img 指向 `OpImg_t` 结构体的指针，代表要操作的图像对象
 * @param *width, *height 指向 `uint32_t` 类型的指针，用于存储图像的宽度和高度。
 */
void open_image_get_resolution(OpImg_t *img, uint32_t *width, uint32_t *height);

#endif  //__OPEN_IMAGE_H__
