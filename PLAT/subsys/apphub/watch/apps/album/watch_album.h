#ifndef __WATCH_ALBUM_H__
#define __WATCH_ALBUM_H__
#include "lvgl.h"

#define MAX_FILE_COUNT (64)
#define MAX_PATH_LENGTH (64)
#define MAX_ALBUM_SHOW_COUNT (4)
#define ALBUM_PREVIEW_WIDTH (96)
#define ALBUM_PREVIEW_HEIGHT (96)
#define TSK_NAME_ALBUM "album"

typedef struct
{
    uint8_t cur_page;
    uint8_t max_page;
    uint8_t cur_file;
} AlbumPage_t;

typedef struct
{
    uint32_t file_cnt;
    char file_path[MAX_FILE_COUNT][MAX_PATH_LENGTH];
} FileList_t;

extern lv_obj_t *ui_photoAlbumScreen;
extern lv_obj_t *ui_photoListView;
extern lv_obj_t *ui_photoThumbnail[MAX_ALBUM_SHOW_COUNT];
extern lv_obj_t *ui_photoAlbumPreview;

void ui_watch_album_init(void);
void ui_watch_album_destroy(void);
#endif