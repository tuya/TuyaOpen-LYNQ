/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_audio_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-09-26
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#include "openplayer.h"
openPlayer gOpenPlayHandler = NULL;

#include "openrecorder.h"
openRecorder gOpenRecordHandler = NULL;

#include "audio.h"
#include "systime.h"
#include "rtthread.h"

extern int skip_atoi(const char **s);

static void playCallback(void)
{
	rt_kprintlnf("PLAY COMPLETE");
	// openPlaySetCallback(gOpenPlayHandler, NULL, NULL);
	// openPlayDestory(gOpenPlayHandler);
}

int cmd_play(int argc, char **argv)
{
	char *sub_cmd = argv[1];
	// audio play d:/test.mp3
	// audio play http://xxx.xxx/xxx.mp3
	if (strcmp(sub_cmd, "play") == 0)
	{
		openPlayerConfigT opParam = {0};
		opParam.playParam.sampleRate = SAMPLERATE_16K; // default,it will change when decode if not pcm
		opParam.playParam.store = AUDIO_PLAY_FILE;
		gOpenPlayHandler = openPlayCreate(&opParam);
		openPlaySetCallback(gOpenPlayHandler, playCallback, NULL);
		if (openPlay(gOpenPlayHandler, (void *)argv[2]) != 0)
		{
			openPlayDestory(gOpenPlayHandler);
			rt_kprintlnf("PLAY FAIL");
			
			return 1;
		}
	}
	// audio stop_play
	else if (strcmp(sub_cmd, "stop_play") == 0)
	{
		openPlayStop(gOpenPlayHandler);
	}
	// audio record d:/record.amr 3
	else if (strcmp(sub_cmd, "record") == 0)
	{
		char *recordFileName = argv[2];
		int recordTime = skip_atoi(&argv[3]);
		openRecorderConfigT param = {0};
		memset(&param, 0x00, sizeof(openRecorderConfigT));
		param.recordParam.recordTime = recordTime;
		gOpenRecordHandler = openRecorderCreate(&param);
		if (openRecorderStart(gOpenRecordHandler, recordFileName) != 0)
		{
			rt_kprintlnf("RECORD FAIL");
			
			return 1;
		}
	}
	// audio stop_record
	else if (strcmp(sub_cmd, "stop_record") == 0)
	{
		openRecorderStop(gOpenRecordHandler);
	}
	// audio pause_record
	else if (strcmp(sub_cmd, "pause_record") == 0)
	{
		openRecorderPause(gOpenRecordHandler);
	}
	// audio resume_record
	else if (strcmp(sub_cmd, "resume_record") == 0)
	{
		if (openRecorderResume(gOpenRecordHandler) != 0)
		{
			rt_kprintlnf("RESUME_RECORD FAIL");
			
			return 1;
		}
	}
	// audio get_record_time
	else if (strcmp(sub_cmd, "get_record_time") == 0)
	{
		rt_kprintlnf("%d", openRecorderGetRecTime(gOpenRecordHandler));
	}
	else
	{
		rt_kprintlnf("Usage: audio [options]");
		rt_kprintlnf("[options]:");
		rt_kprintlnf("    %-20s - play audio", "play");
		rt_kprintlnf("    %-20s - stop play audio", "stop_play");
		rt_kprintlnf("    %-20s - record voice", "record");
		rt_kprintlnf("    %-20s - stop record voice", "stop_record");
		rt_kprintlnf("    %-20s - pause record voice", "pause_record");
		rt_kprintlnf("    %-20s - resume record voice", "resume_record");
		rt_kprintlnf("    %-20s - get record voice time", "get_record_time");
	}
	
	return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_play, audio, audio test);

#endif
