/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "ol_lbs_api.h"

void lbs_demo(void)
{
    int ret = 0;
    char lbs_info[128];
    char key[30] = {"0"};//input the Gaode key requested by the customer

    memset(lbs_info, 0, sizeof(lbs_info));
    ret = ol_lbs_gdhttp_info(key, lbs_info);
    OL_LOG_INFO("gd lbs test: ret = %d, info = %s",ret, lbs_info);
}

