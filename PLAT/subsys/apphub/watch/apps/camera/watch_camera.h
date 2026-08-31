#ifndef __WATCH_CAMERA_H__
#define __WATCH_CAMERA_H__
#include "lvgl.h"

#define TSK_NAME_CAMERA "cam"

extern lv_obj_t *ui_cameraScreen;
extern lv_obj_t *ui_cameraPanel;
extern lv_obj_t *ui_cameraCapture;
extern lv_obj_t *ui_cameraTitle;
extern lv_obj_t *ui_cameraIcon;
extern lv_obj_t *ui_cameraPreview;
extern lv_obj_t *ui_cameraButton;
extern lv_obj_t *ui_cameraButtonLabel;

void ui_watch_camera_init(void);
void ui_watch_camera_destroy(void);

#endif