#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#endif


extern int32_t ui_powerOffCharging_set(fpui_powerOffCharging_t *powerOffCharging);


int32_t showPowerOffCharging(void)
{
    fpui_powerOffCharging_t powerOffCharging = {.batteryLevel = getBatteryPercentage()};
    return ui_powerOffCharging_set(&powerOffCharging);
}
