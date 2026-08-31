#if 1
#include "../ui.h"

lv_obj_t *ui_Charging = NULL;
lv_obj_t *ui_Charging_Logo = NULL;
#define  OUTLINE_W    80 //电池图标宽度
#define  OUTLINE_H    40 //电池图标高度


// void ui_event_Charging( lv_event_t * e) 
// {
//     lv_event_code_t event_code = lv_event_get_code(e);
//     lv_obj_t * target = lv_event_get_target(e);
// }

void lv_anim_cb(void* p, int32_t v)
{
    //实现变色效果，电池电量低于20% 红色
    static int32_t cnt;
    if (cnt >= OUTLINE_W * 0.2 && v < OUTLINE_W * 0.2)
    {
        lv_obj_set_style_bg_color(p, lv_color_hex(0xff0000), 0);
    }
    else if (v >= OUTLINE_W * 0.2 && cnt < OUTLINE_W * 0.2)
    {
        lv_obj_set_style_bg_color(p, lv_color_hex(0xff00), 0);
    }
    cnt = v;

    //修改电量颜色obj宽度
    lv_obj_set_width(p, v);

    //修改电池百分比
    lv_obj_t *text = lv_obj_get_child(lv_obj_get_parent(p), -1);
    lv_label_set_text_fmt(text, "%d", v*100/(OUTLINE_W-4));
}


void ui_Charging_screen_init(void)
{
    ui_Charging = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Charging, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_Charging, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Charging, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_t* outline = lv_obj_create(ui_Charging);

    //设置border和pading
    lv_obj_set_style_border_width(outline, 2, 0);
    lv_obj_set_style_pad_all(outline, 0, 0);

    //设置圆角
    lv_obj_set_style_radius(outline, 8, 0);

    //关闭滚动条
    lv_obj_clear_flag(outline, LV_OBJ_FLAG_SCROLLABLE);

    //设置宽高
    lv_obj_set_size(outline, OUTLINE_W, OUTLINE_H);

    //居中对齐
    lv_obj_align(outline, LV_ALIGN_CENTER, 0, 0);

    //电池电量填充obj
    lv_obj_t* pad = lv_obj_create(outline);

   
    //设置outline
    lv_obj_set_style_outline_width(pad, 0, 0);
    lv_obj_set_style_outline_pad(pad, 0, 0);
    lv_obj_set_style_border_width(pad, 0, 0);
    //设置背景色
    lv_obj_set_style_bg_color(pad, lv_color_hex(0xff0000), 0);

    //设置宽高
    lv_obj_set_size(pad, OUTLINE_W, OUTLINE_H-4);
    lv_obj_set_style_border_width(pad, 0, 0);

    //设置圆角
    lv_obj_set_style_radius(pad, 8, 0);

    //居中对齐
    lv_obj_align(pad, LV_ALIGN_LEFT_MID, 0, 0);

    //关闭滚动条
    lv_obj_clear_flag(pad, LV_OBJ_FLAG_SCROLLABLE);

    //电池百分比
    lv_obj_t* label = lv_label_create(outline);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    //设置动画， 模仿电池电量变化
    lv_anim_t a;
    lv_anim_init(&a);

     /*Set the "animator" function*/
    lv_anim_set_exec_cb(&a, lv_anim_cb);

    /*Set the "animator" function*/
    lv_anim_set_var(&a, pad);

    /*Length of the animation [ms]*/
    lv_anim_set_time(&a, 10000);

    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&a, 0, OUTLINE_W-4);

     /*Time to wait before starting the animation [ms]*/
    lv_anim_set_delay(&a, 1000);

    /*Play the animation backward too with this duration. Default is 0 (disabled) [ms]*/
    lv_anim_set_playback_time(&a, 0);

    /*Delay before playback. Default is 0 (disabled) [ms]*/
    lv_anim_set_playback_delay(&a, 0);

    /*Number of repetitions. Default is 1.  LV_ANIM_REPEAT_INFINIT for infinite repetition*/
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

    /*Delay before repeat. Default is 0 (disabled) [ms]*/
    lv_anim_set_repeat_delay(&a, 1000);

    /* START THE ANIMATION
     *------------------*/
    lv_anim_start(&a);                             /*Start the animation*/
    // lv_obj_add_event_cb(ui_Charging, ui_event_Charging, LV_EVENT_ALL, NULL);
}
#endif