#include "open_anim.h"
#include "mm_video_if.h"
#include "open_image.h"
#include "cmsis_os2.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include DEBUG_LOG_HEADER_FILE

#define DEFAULT_MAX_GIF_FRAME_CNT (64)

static int parse_gif_info(OpAnim_t* anim)
{
    if(!anim || !anim->data || anim->size < 13)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, parse_gif_info_fail0, P_INFO,
                      "invalid pointer");
        return -1;
    }

    if(anim->data[0] != 'G' || anim->data[1] != 'I' || anim->data[2] != 'F')
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, parse_gif_info_fail1, P_INFO,
                      "not a gif file");
        return -1;
    }

    anim->width = (uint32_t)anim->data[6] | ((uint32_t)anim->data[7] << 8);
    anim->height = (uint32_t)anim->data[8] | ((uint32_t)anim->data[9] << 8);
    anim->stride = anim->width * 4;

    anim->frame_cnt = 0;
    anim->delay = malloc(DEFAULT_MAX_GIF_FRAME_CNT *
                         sizeof(uint32_t));  // 先分配64个，后续可能需要扩展
    if(!anim->delay)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, parse_gif_info_fail2, P_INFO,
                      "malloc failed");
        return -2;
    }

    uint32_t delay_index = 0;
    uint32_t pos = 13;  // 跳过GIF头部（13字节）

    // 跳过全局颜色表
    if(anim->data[10] & 0x80)
    {
        int gct_size = 3 * (1 << ((anim->data[10] & 0x07) + 1));
        pos += gct_size;
    }

    while(pos + 1 < anim->size)
    {
        uint8_t block_type = anim->data[pos];
        if(block_type == 0x2C)
        {
            anim->frame_cnt++;
            if(delay_index >= DEFAULT_MAX_GIF_FRAME_CNT)
            {

                uint32_t* new_delay =
                    realloc(anim->delay, (delay_index + 32) * sizeof(uint32_t));
                if(!new_delay)
                {
                    break;
                }
                anim->delay = new_delay;
            }

            // 检查是否有局部颜色表
            if(pos + 9 < anim->size && (anim->data[pos + 9] & 0x80))
            {
                int lct_size = 3 * (1 << ((anim->data[pos + 9] & 0x07) + 1));
                pos += lct_size;
            }

            // 跳转到图像数据
            pos += 10;  // 图像描述符大小

            // 跳过LZW最小码大小
            if(pos < anim->size)
            {
                pos++;  // LZW最小码大小

                // 跳过图像数据块
                while(pos < anim->size)
                {
                    uint8_t block_size = anim->data[pos];
                    pos += block_size + 1;
                    if(block_size == 0) break;
                }
            }
        }
        else if(block_type == 0x21)  // 扩展块
        {
            if(pos + 1 < anim->size)
            {
                uint8_t extension_type = anim->data[pos + 1];

                if(extension_type == 0xF9)  // 图形控制扩展
                {
                    // 标准图形控制扩展块大小为4字节
                    if(pos + 8 <
                       anim->size)  // 2字节标识 + 6字节数据（含块大小）
                    {
                        uint8_t block_size = anim->data[pos + 2];
                        if(block_size == 4)  // 标准图形控制扩展块大小为4
                        {
                            // 延迟时间在偏移pos+4和pos+5处（单位10ms）
                            uint16_t delay_cs =
                                (uint16_t)anim->data[pos + 4] |
                                ((uint16_t)anim->data[pos + 5] << 8);
                            uint32_t delay_ms = delay_cs * 10;  // 转换为毫秒

                            // 将延迟与当前帧关联
                            // 注意：图形控制块可能在图像描述符之前出现
                            // 这里假设延迟属于下一帧
                            if(delay_index < DEFAULT_MAX_GIF_FRAME_CNT)
                            {
                                anim->delay[delay_index] = delay_ms;
                                delay_index++;
                            }
                        }
                    }

                    // 跳过这个扩展块
                    pos += 2;  // 扩展块标识和类型

                    // 跳过块大小和数据
                    if(pos < anim->size)
                    {
                        uint8_t block_size = anim->data[pos];
                        pos += block_size + 1;  // +1 for block size byte

                        // 跳过可能的后续子块（通常是结束符0）
                        while(pos < anim->size)
                        {
                            block_size = anim->data[pos];
                            if(block_size == 0)
                            {
                                pos++;
                                break;
                            }
                            pos += block_size + 1;
                        }
                    }
                }
                else
                {
                    // 跳过其他扩展块
                    pos += 2;  // 扩展块标识和类型

                    // 跳过所有子块
                    while(pos < anim->size)
                    {
                        uint8_t block_size = anim->data[pos];
                        pos += block_size + 1;
                        if(block_size == 0) break;
                    }
                }
            }
            else
            {
                break;
            }
        }
        else if(block_type == 0x3B)  // 文件结束符
        {
            break;
        }
        else
        {
            // 未知块，尝试跳过
            pos++;
        }
    }

    // 确保帧数至少为1
    anim->frame_cnt = anim->frame_cnt > 0 ? anim->frame_cnt : 1;

    // 重新分配delay数组到实际大小
    if(delay_index > 0 && delay_index < DEFAULT_MAX_GIF_FRAME_CNT)
    {
        uint32_t* new_delay =
            realloc(anim->delay, anim->frame_cnt * sizeof(uint32_t));
        if(new_delay)
        {
            anim->delay = new_delay;
        }
    }

    return 0;
}

static uint8_t* read_file_by_filename(const char* filename, uint32_t* size)
{
    if(filename == NULL || size == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, read_file_by_filename_failed0, P_INFO,
                      "invalid pointer");
        return NULL;
    }
    FILE* file = file_fopen(filename, "rb");
    if(file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, read_file_by_filename_failed1, P_INFO,
                      "open file %s failed", filename);
        return NULL;
    }

    struct stat file_stat = {0};
    file_fstat((int)file, &file_stat);
    *size = file_stat.st_size;
    uint8_t* data = malloc(*size);

    if(data == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, read_file_by_filename_failed2, P_INFO,
                      "alloc memory failed: size:%d", *size);
        file_fclose(file);
        return NULL;
    }
    file_fread(data, *size, 1, file);
    file_fclose(file);
    return data;
}

OpAnim_t* open_anim_create_by_file(const char* filename)
{
    OpAnim_t* anim = NULL;
    if(filename == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_file_failed0,
                      P_INFO, "invalid pointer");
        return NULL;
    }
    uint32_t anim_size = 0;
    uint8_t* data = read_file_by_filename(filename, &anim_size);
    if(data == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_file_failed1,
                      P_INFO, "read file %s failed", filename);
        return NULL;
    }
    anim = open_anim_create_by_data(data, anim_size, false);
    if(anim == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_file_failed2,
                      P_INFO, "create anim failed");
        return NULL;
    }
    anim->is_alloc = true;
    return anim;
}

OpAnim_t* open_anim_create_by_data(uint8_t* data, uint32_t size, bool is_alloc)
{
    OpAnim_t* anim = NULL;
    if(data == NULL || size == 0)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_data_failed0,
                      P_INFO, "invalid pointer");
        return NULL;
    }
    anim = malloc(sizeof(OpAnim_t));
    if(anim == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_data_failed1,
                      P_INFO, "allocate memory failed");
        return NULL;
    }
    memset(anim, 0, sizeof(OpAnim_t));
    if(is_alloc)
    {
        anim->data = malloc(size);
        if(anim->data == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_data_failed2,
                          P_INFO, "allocate memory failed");
            open_anim_destroy(anim);
            return NULL;
        }
        memcpy(anim->data, data, size);
    }
    else
    {
        anim->data = data;
        anim->size = size;
    }
    anim->is_alloc = is_alloc;
    anim->type = ANIM_TYPE_GIF;
    anim->dec = GifD_Create();
    GIF_INFO info = {0};
    if(GifD_DecodeInfo(anim->dec, anim->data, anim->size, &info) != 0)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_create_by_data_failed4,
                      P_INFO, "get gif info failed");
        open_anim_destroy(anim);
        return NULL;
    }
    anim->width = info.uWidth;
    anim->height = info.uHeight;
    return anim;
}

int open_anim_destroy(OpAnim_t* anim)
{
    if(anim == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_destroy_failed0, P_INFO,
                      "invalid pointer");
        return -1;
    }
    if(anim->is_alloc)
    {
        if(anim->data)
        {
            free(anim->data);
            anim->data = NULL;
        }
    }
    if(anim->delay)
    {
        free(anim->delay);
        anim->delay = NULL;
    }
    if(anim->dec)
    {
        GifD_Destroy(anim->dec);
        anim->dec = NULL;
    }
    free(anim);
    return 0;
}

int open_anim_get_frame_cnt(OpAnim_t* anim)
{
    if(anim == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_cnt_failed0,
                      P_INFO, "invalid pointer");
        return -1;
    }
    if(anim->frame_cnt == 0)
    {
        if(parse_gif_info(anim) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_cnt_failed1,
                          P_INFO, "parse anim info failed");
            open_anim_destroy(anim);
            return -1;
        }
    }
    return anim->frame_cnt;
}

uint32_t open_anim_get_delay(OpAnim_t* anim, uint32_t frame_idx)
{
    if(anim == NULL || frame_idx >= anim->frame_cnt)
    {
        return 0;
    }
    if(anim->frame_cnt == 0)
    {
        if(parse_gif_info(anim) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_delay_failed0,
                          P_INFO, "parse anim info failed");
            open_anim_destroy(anim);
            return -1;
        }
    }
    return anim->delay[frame_idx];
}

int open_anim_get_frame(OpAnim_t* anim, uint8_t* frame, uint32_t buff_size,
                        uint8_t* last_frame, uint32_t* frame_delay_in_ms)
{
    VIDEO_IMAGE_BUF ibuf;
    if(anim == NULL || frame == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_failed0, P_INFO,
                      "invalid pointer");
        return -1;
    }
    if(anim->type != ANIM_TYPE_GIF)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_failed1, P_INFO,
                      "only support gif anim");
    }
    if(buff_size < anim->width * anim->height * 2)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_failed2, P_INFO,
                      "buff_size is too small");
        return -1;
    }
    if(!anim->set_canvas)
    {
        ibuf.eFmt = VIDEO_COLOR_FMT_RGB565;
        ibuf.uWidth = anim->width;
        ibuf.uHeight = anim->height;
        ibuf.pData[0] = frame;
        if(GifD_SetCanvas(anim->dec, &ibuf) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_failed3,
                          P_INFO, "set canvas failed");
            return -1;
        }
        anim->set_canvas = true;
    }
    unsigned int is_last = 0;
    unsigned int frame_delay = 0;
    if(GifD_DecodeImage(anim->dec, NULL, &frame_delay, &is_last) != 0)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_get_frame_failed4, P_INFO,
                      "decode image failed");
        return -1;
    }
    *last_frame = is_last != 0 ? 1 : 0;
    *frame_delay_in_ms = frame_delay;
    return 0;
}

int open_anim_get_frame_size(OpAnim_t* anim, uint32_t* width, uint32_t* height)
{
    if(anim == NULL || width == NULL || height == NULL)
    {
        return -1;
    }
    *width = anim->width;
    *height = anim->height;
    return 0;
}

int open_anim_play(char* filename)
{
    if(!filename)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_play_failed0, P_INFO,
                      "invalid filename");
        return -1;
    }
    OpAnim_t* anim = open_anim_create_by_file(filename);
    if(anim == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_play_failed1, P_INFO,
                      "create anim failed");
        return -1;
    }

    uint32_t width, height;
    if(open_anim_get_frame_size(anim, &width, &height) != 0)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_play_failed2, P_INFO,
                      "get frame size failed");
        open_anim_destroy(anim);
        return -1;
    }
    uint8_t* frame = malloc(width * height * 2);
    if(frame == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_play_failed3, P_INFO,
                      "allocate memory failed");
        open_anim_destroy(anim);
        return -1;
    }
    int count = 512;  // 最多显示512帧
    while((count--) > 0)
    {
        uint8_t last_frame = 0;
        uint32_t delay = 0;
        if(open_anim_get_frame(anim, frame, width * height * 2, &last_frame,
                               &delay) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_play_failed4, P_INFO,
                          "get frame failed");
            open_anim_destroy(anim);
            free(frame);
            return -1;
        }
        OpImg_t* img = open_image_create(width, height, IMG_FMT_RGB565, frame,
                                         width * height * 2);
        if(img == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, open_anim_play_failed5, P_INFO,
                          "create image failed");
            open_anim_destroy(anim);
            free(frame);
            return -1;
        }
        open_image_show(img, false);
        open_image_destroy(img);
        osDelay(delay);
        if(last_frame != 0)
        {
            break;
        }
    }
    free(frame);
    open_anim_destroy(anim);
    return 0;
}
