/*
*---------------------------------------------------------------
*                        Lvgl Font Tool                         
*                                                               
* 注:使用unicode编码                                              
* 注:本字体文件由Lvgl Font Tool V0.4 生成                          
* 作者:阿里(qq:617622104)                                         
*---------------------------------------------------------------
*/


#include "lvgl.h"
#include "merged.h"
#include "ui.h"
#include "storage.h"


typedef struct{
    uint16_t min;
    uint16_t max;
    uint8_t  bpp;
    uint8_t  reserved[3];
}x_header_t;
typedef struct{
    uint32_t pos;
}x_table_t;
typedef struct{
    uint8_t adv_w;
    uint8_t box_w;
    uint8_t box_h;
    int8_t  ofs_x;
    int8_t  ofs_y;
    uint8_t r;
}glyph_dsc_t;


static x_header_t g_xbf_hd = {
    .min = 0x0020,
    .max = 0xFFE5,
    .bpp = 1,
};

PLAT_FPSRAM_ZI_CUST uint8_t g_font_buf[1024*850];
uint32_t lv_font_extern_init(void)
{
    memset(g_font_buf,0,sizeof(g_font_buf));
    for (uint8_t i=0; i<TOTAL_BIN_NUM; i++){
        if(strstr(LV_FONT_EXTERN_36,ext_bin_desc[i].path) != NULL){
            FILE * bareFile = file_fopen(LV_FONT_EXTERN_36, "r");
            if(bareFile != NULL){
                file_fread(g_font_buf, ext_bin_desc[i].size, 1, bareFile);
                file_fclose(bareFile);
                g_xbf_hd.min = ext_bin_desc[i].width;
                g_xbf_hd.max = ext_bin_desc[i].height;
                UI_LOG("[%X,%X],%d,%s",g_xbf_hd.min,g_xbf_hd.max,ext_bin_desc[i].size,LV_FONT_EXTERN_36);
            }
            return ext_bin_desc[i].size;
        }
    }
    UI_ERR("fail read font %s",LV_FONT_EXTERN_36);
    return 0;
}

// static uint8_t __g_font_buf[80];
static uint8_t *__user_font_getdata(int offset, int size){
    // spiFlashRead(offset,__g_font_buf, size);
    // return __g_font_buf;
    return (uint8_t*)(g_font_buf+offset);
}


static const uint8_t * __user_font_get_bitmap(const lv_font_t * font, uint32_t unicode_letter) {
    if( unicode_letter>g_xbf_hd.max || unicode_letter<g_xbf_hd.min ) {
        return NULL;
    }
    uint32_t unicode_offset = sizeof(x_header_t)+(unicode_letter-g_xbf_hd.min)*4;
    uint32_t *p_pos = (uint32_t *)__user_font_getdata(unicode_offset, 4);
    if( p_pos[0] != 0 ) {
        uint32_t pos = p_pos[0];
        glyph_dsc_t * gdsc = (glyph_dsc_t*)__user_font_getdata(pos, sizeof(glyph_dsc_t));
        return __user_font_getdata(pos+sizeof(glyph_dsc_t), gdsc->box_w*gdsc->box_h*g_xbf_hd.bpp/8);
    }
    return NULL;
}


static bool __user_font_get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next) {
    if( unicode_letter>g_xbf_hd.max || unicode_letter<g_xbf_hd.min ) {
        return NULL;
    }
    uint32_t unicode_offset = sizeof(x_header_t)+(unicode_letter-g_xbf_hd.min)*4;
    uint32_t *p_pos = (uint32_t *)__user_font_getdata(unicode_offset, 4);
    if( p_pos[0] != 0 ) {
        glyph_dsc_t * gdsc = (glyph_dsc_t*)__user_font_getdata(p_pos[0], sizeof(glyph_dsc_t));
        dsc_out->adv_w = gdsc->adv_w;
        dsc_out->box_h = gdsc->box_h;
        dsc_out->box_w = gdsc->box_w;
        dsc_out->ofs_x = gdsc->ofs_x;
        dsc_out->ofs_y = gdsc->ofs_y;
        dsc_out->bpp  = g_xbf_hd.bpp;
        return true;
    }
    return false;
}


//Source Han Sans CN Light,,-1
//字模高度：36
//XBF字体,外部bin文件
const lv_font_t free36 = {
    .get_glyph_bitmap = __user_font_get_bitmap,
    .get_glyph_dsc = __user_font_get_glyph_dsc,
    .line_height = 36,
    .base_line = 0,
};

