/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    open_anim.h
 * Description:  open animation header file
 * History:      Rev1.0   2026-02-25
 *
 ****************************************************************************/
#ifndef __OPEN_ANIM_H__
#define __OPEN_ANIM_H__

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 动画类型枚举
 */
typedef enum AnimType_
{
    ANIM_TYPE_GIF, /**< GIF动画类型 */
    ANIM_TYPE_MAX  /**< 动画类型最大值，用于边界检查 */
} AnimType_t;

/**
 * @brief 动画结构体
 */
typedef struct OpAnim_
{
    uint8_t* data;      /**< 动画数据指针 */
    uint32_t size;      /**< 动画数据大小（字节） */
    AnimType_t type;    /**< 动画类型 */
    uint32_t width;     /**< 动画宽度（像素） */
    uint32_t height;    /**< 动画高度（像素） */
    uint32_t stride;    /**< 动画行步长（字节） */
    uint32_t frame_cnt; /**< 动画帧数 */
    uint32_t* delay;    /**< 每帧延迟数组（毫秒） */
    bool is_alloc;      /**< 数据是否由动画模块分配 */
    void* dec;          /**< 解码器指针 */
    bool set_canvas;    /**< 是否设置了画布 */
} OpAnim_t;

/**
 * @brief 从文件中创建动画
 * @param filename 动画文件路径
 * @return 成功返回动画结构体指针，失败返回NULL
 */
OpAnim_t* open_anim_create_by_file(const char* filename);

/**
 * @brief 从内存中创建动画
 * @param data 动画数据指针
 * @param size 动画数据大小（字节）
 * @param is_alloc 数据是否由动画模块分配,
 * 如果is_alloc为true时，该函数会重新分配一块内存将数据拷贝到该内存中。
 * 如果is_alloc为false，那么data指针必须指向一个有效的动画数据块，
 * 该函数不会对data进行任何内存管理，也不会对数据进行拷贝。
 * @return 成功返回动画结构体指针，失败返回NULL
 */
OpAnim_t* open_anim_create_by_data(uint8_t* data, uint32_t size, bool is_alloc);

/**
 * @brief 销毁动画
 * @param anim 动画结构体指针
 * @return 成功返回0，失败返回错误码
 */
int open_anim_destroy(OpAnim_t* anim);

/**
 * @brief 获取动画的帧数
 * @param anim 动画结构体指针
 * @return 成功返回帧数，失败返回错误码
 * @note 该函数需要遍历GIF文件，大文件耗时会较长。如果只是播放可以不进行调用。open_anim_get_frame会返回最后一帧信息。
 */
int open_anim_get_frame_cnt(OpAnim_t* anim);

/**
 * @brief 获取指定帧的延迟
 * @param anim 动画结构体指针
 * @param frame_idx 帧索引
 * @return 返回指定帧的延迟（毫秒）
 * @note 该函数需要遍历GIF文件，大文件耗时会较长。如果只是播放可以不进行调用。open_anim_get_frame会返回当前帧delay信息。
 */
uint32_t open_anim_get_delay(OpAnim_t* anim, uint32_t frame_idx);

/**
 * @brief 获取帧的数据
 * @param anim 动画结构体指针
 * @param frame 输出参数，用于存储帧数据的缓冲区
 * @param buff_size 缓冲区大小（字节）
 * @param last_frame 输出参数，为1则为最后一帧，为0则为非最后一帧
 * @param frame_delay_in_ms 输出参数，用于当前帧的播放延迟（毫秒）
 * @return 成功返回0，失败返回错误码
 */
int open_anim_get_frame(OpAnim_t* anim, uint8_t* frame, uint32_t buff_size, uint8_t *last_frame, uint32_t* frame_delay_in_ms);

/**
 * @brief 获取动画帧的大小
 * @param anim 动画结构体指针
 * @param width 输出参数，用于存储帧宽度（像素）
 * @param height 输出参数，用于存储帧高度（像素）
 * @return 成功返回0，失败返回错误码
 */
int open_anim_get_frame_size(OpAnim_t* anim, uint32_t* width, uint32_t* height);

/**
 * @brief 播放动画
 * @param filename 动画文件路径
 * @return 成功返回0，失败返回错误码
 */
int open_anim_play(char* filename);

#endif  //__OPEN_ANIM_H__