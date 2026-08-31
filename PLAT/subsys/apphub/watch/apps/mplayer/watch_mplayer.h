#ifndef __WATCH_MPLAYER_H__
#define __WATCH_MPLAYER_H__
#include "lvgl.h"
#define UI_IMG_EXIT_PNG	"D:/ui_img_exit_png.png"
#define UI_IMG_LAST_PNG	"D:/ui_img_last_png.png"
#define UI_IMG_MUSIC_PLAYER_PNG	"D:/ui_img_music_play_png.png"
#define UI_IMG_MUSIC_FILE_PNG	"D:/ui_img_music_file_png.png"
#define UI_IMG_NEXT_PNG	"D:/ui_img_next_png.png"
#define UI_IMG_PAUSE_PNG	"D:/ui_img_pause_png.png"
#define UI_IMG_PLAY_PNG	"D:/ui_img_play_png.png"

extern lv_obj_t* ui_scrFileList;
extern lv_obj_t* ui_scrMusicPlayer;
extern lv_obj_t* ui_ibExit;
extern lv_obj_t* ui_ibPlay;
extern lv_obj_t* ui_ibLast;
extern lv_obj_t* ui_ibNext;
extern lv_obj_t* ui_labCurTime;
extern lv_obj_t* ui_labTotalTime;
extern lv_obj_t* ui_sliTime;
extern lv_obj_t* ui_imgMusicPlay;
extern lv_obj_t* ui_lbMusicTitle;

void ui_mplayer_init(void);
void ui_mplayer_destroy(void);
#endif