#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"

#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
#include "doubao_Engine.h"
#endif
#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "finsh.h"
#include "rtthread.h"

void consoleRtcStart(int argc, char **argv)
{	
#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
	doubao_rtc_proc(AI_ENG_START_VOICE_CHART_FLAG);
#endif
    // rt_kprintf("\n\r%s >",getConsolePrompt());
}


void consoleRtcStop(int argc, char **argv)
{	
#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
    doubao_rtc_proc(AI_ENG_STOP_VOICE_CHART_FLAG);
#endif
    // rt_kprintf("\n\r%s >",getConsolePrompt());
}

void consoleRtcUpdate(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
    doubao_rtc_proc(AI_ENG_UPDATE_VOICE_CHART_FLAG);
#endif
    // rt_kprintf("\n\r%s >",getConsolePrompt());
}


MSH_CMD_EXPORT_ALIAS(consoleRtcStart, rtcStart, ai rtc start);

MSH_CMD_EXPORT_ALIAS(consoleRtcStop, rtcStop, ai rtc stop);

MSH_CMD_EXPORT_ALIAS(consoleRtcUpdate, rtcUpdate, ai rtc interrupt);


#endif

#endif/*FEATURE_SUBSYS_CONSOLE_ENABLE*/

