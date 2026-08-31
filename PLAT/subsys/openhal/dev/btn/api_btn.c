/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_btn.c
 * Description:  ec7xx openHAL BTN entry source file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_Common.h"
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "api_comm.h"

//数量上限
#define EC_DEV_BTN_INDEX_LIMIT      (20)

// #ifdef BSP_MINIDKB_1V1
//此表后续会移至DDK FLASH存储区域
// static uint8_t BtnStartupList[EC_DEV_BTN_INDEX_LIMIT] = {
//     0x0, 0x0, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
// };
// #endif


/**
  \fn          
  \brief     所有BTN上电初始化为配置状态  
  \return
*/
int api_btn_startup(void* para)
{
    for(int i=0;i<EC_DEV_BTN_INDEX_LIMIT;i++)
    {

    }

    return 0;
}