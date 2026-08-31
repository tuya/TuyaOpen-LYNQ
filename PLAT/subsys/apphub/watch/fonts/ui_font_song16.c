
#include "lvgl.h"
#include "merged.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif


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


static x_header_t __g_xbf_hd = {
    .min = 0x0020,
    .max = 0xffe5,
    .bpp = 2,
};

static uint8_t gFontBuffer[800 * 1024] = {0};


uint32_t lv_font_extern_init(void)
{
    uint32_t  retVal = 0;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    FILE     *file   = NULL;
#endif

    memset(gFontBuffer, 0, sizeof(gFontBuffer));
    for (uint32_t i=0; i<TOTAL_BIN_NUM; i++)
    {
        if (strstr(LV_FONT_EXTERN_16, ext_bin_desc[i].path) != NULL)
        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            file = file_fopen(LV_FONT_EXTERN_16, "r");
            if (file != NULL)
            {
                file_fread(gFontBuffer, ext_bin_desc[i].size, 1, file);
                file_fclose(file);

                __g_xbf_hd.min = ext_bin_desc[i].width;
                __g_xbf_hd.max = ext_bin_desc[i].height;
                SYSLOG_DEBUG("%s, size=%d, min=0x%X, max=0x%X\r\n", LV_FONT_EXTERN_16, ext_bin_desc[i].size, __g_xbf_hd.min,__g_xbf_hd.max);

                retVal = ext_bin_desc[i].size;
            }
#endif
            break;
        }
    }

    if (retVal == 0)
    {
        SYSLOG_DEBUG("Failed to read font %s\r\n", LV_FONT_EXTERN_16);
    }

    return retVal;
}

static uint8_t *__user_font_getdata(int offset, int size){
    return (uint8_t *)(gFontBuffer + offset);
}

static const uint8_t * __user_font_get_bitmap(const lv_font_t * font, uint32_t unicode_letter) {
    if( unicode_letter>__g_xbf_hd.max || unicode_letter<__g_xbf_hd.min ) {
        return NULL;
    }
    uint32_t unicode_offset = sizeof(x_header_t)+(unicode_letter-__g_xbf_hd.min)*4;
    uint32_t *p_pos = (uint32_t *)__user_font_getdata(unicode_offset, 4);
    if( p_pos[0] != 0 ) {
        uint32_t pos = p_pos[0];
        glyph_dsc_t * gdsc = (glyph_dsc_t*)__user_font_getdata(pos, sizeof(glyph_dsc_t));
        return __user_font_getdata(pos+sizeof(glyph_dsc_t), gdsc->box_w*gdsc->box_h*__g_xbf_hd.bpp/8);
    }
    return NULL;
}


static bool __user_font_get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next) {
    if( unicode_letter>__g_xbf_hd.max || unicode_letter<__g_xbf_hd.min ) {
        return NULL;
    }
    uint32_t unicode_offset = sizeof(x_header_t)+(unicode_letter-__g_xbf_hd.min)*4;
    uint32_t *p_pos = (uint32_t *)__user_font_getdata(unicode_offset, 4);
    if( p_pos[0] != 0 ) {
        glyph_dsc_t * gdsc = (glyph_dsc_t*)__user_font_getdata(p_pos[0], sizeof(glyph_dsc_t));
        dsc_out->adv_w = gdsc->adv_w;
        dsc_out->box_h = gdsc->box_h;
        dsc_out->box_w = gdsc->box_w;
        dsc_out->ofs_x = gdsc->ofs_x;
        dsc_out->ofs_y = gdsc->ofs_y;
        dsc_out->bpp   = __g_xbf_hd.bpp;
        return true;
    }
    return false;
}

const lv_font_t ui_font_song16 = {
    .get_glyph_bitmap = __user_font_get_bitmap,
    .get_glyph_dsc = __user_font_get_glyph_dsc,
    .line_height = 16,
    .base_line = 0,
};

