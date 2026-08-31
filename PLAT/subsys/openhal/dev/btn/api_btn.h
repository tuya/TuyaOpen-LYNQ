/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_btn.h
 * Description:  ec7xx openHAL BTN entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#ifndef _API_BTN_H_
#define _API_BTN_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"

typedef void ( *btn_isr_t)(void);

int api_btn_startup(void* para);
#ifdef __cplusplus
}
#endif
#endif /* _API_BTN_H_ */