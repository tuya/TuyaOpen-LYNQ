/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#define _GNU_SOURCE 1
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "bsp_custom.h"
#include "cmips.h"
#include "cmisim.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "networkmgr.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "queue.h"
#include "semphr.h"
#include "slpman.h"
#include "storage.h"
#include "string.h"
#include "task.h"
#include "time.h"

#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "bsp.h"
#include "console.h"
#include "console_ex.h"
#include "console_file.h"
#include "console_hal.h"
#include "packet.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "ff.h"
#endif
#if defined(FEATURE_SUBSYS_AUDIO_ENABLE) || defined(FEATURE_SUBSYS_MEDIA_ENABLE)
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#else
#include "media.h"
#endif
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_PCM_ENABLE
#include "pcm.h"
#endif
#endif

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
#include "openplayer.h"
#endif

#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
#include "openrecorder.h"
#endif

#ifdef FEATURE_SUBSYS_XFAI_ENABLE
#include "xfai_service.h"
#endif
#include "console_cmd.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#if defined(FEATURE_SUBSYS_AUDIO_ENABLE) || defined(FEATURE_SUBSYS_MEDIA_ENABLE)

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
openPlayer gOpenPlayHandler = NULL;
#endif
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
openRecorder gOpenRecordHandler = NULL;
#endif

void playAudioByName(char *name)
{
    // rt_kprintf("\n\rplay:%s\n\r",name);
    if(strstr(name, ".mp3"))
    {
#ifdef FEATURE_SUBSYS_MP3_ENABLE
        audioPlayMp3(name, NULL, true, true);
#endif
    }
    else if(strstr(name, ".wav") != NULL)
    {
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_WAV_ENABLE
        audioPlayWav(name, NULL);
#endif
    }
    else if(strstr(name, ".amr") != NULL)
    {
        audioPlayAmr(name, NULL);
    }
}

void stopPlayAudioByName(char *name)
{
#ifndef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
    if(strstr(name, ".mp3"))
    {
#ifdef FEATURE_SUBSYS_MP3_ENABLE
        audioStopPlay();
#endif
    }
#else
    // openPlayPause(gOpenPlayHandler,true);
    openPlayStop(gOpenPlayHandler);
#endif
}

#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
#include "record.h"

void consoleAudRecordCallback(void *userData, int32_t result)
{
    rt_kprintf("\r\nrecord result: %d", result);

    
}

void startRecordByName(char *name, int time)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    openRecorderConfigT param = {0};
    memset(&param, 0x00, sizeof(openRecorderConfigT));
    param.recordParam.recordTime = time;
    gOpenRecordHandler = openRecorderCreate(&param);
    openRecorderSetCallback(gOpenRecordHandler, consoleAudRecordCallback, NULL);
    openRecorderStart(gOpenRecordHandler, (void *)name);
#else
    RecordParamT recordParam = {0};
    recordParam.time = time * 1000;

    audioRecordAmr(name, recordParam, NULL);
#endif
}
#endif

void consoleAudPlayCallback(void *userData, int32_t result)
{
    rt_kprintf("\r\n");
    rt_kprintf("\r\nplay result: %d", result);

    
}
void consolePlayAudio(int argc, char **argv)
{
    char folder[128];
    memset(folder, 0, sizeof(folder));
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
    openPlayerConfigT opParam = {0};
    opParam.playParam.sampleRate =
        SAMPLERATE_16K;  // default,it will change when decode if not pcm
#endif
    if(strcasestr(argv[1], "http://") || strcasestr(argv[1], "https://"))
    {
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
        opParam.playParam.store = AUDIO_PLAY_STREAM;
        opParam.playParam.srcLen = strlen(argv[1]);
        gOpenPlayHandler = openPlayCreate(&opParam);
        openPlaySetCallback(gOpenPlayHandler, consoleAudPlayCallback, NULL);
        openPlay(gOpenPlayHandler, (void *)argv[1]);
#else
        playAudioByName(argv[1]);
#endif
    }
    else if(fileNameToDir(argv[1], folder))
    {
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
        opParam.playParam.store = AUDIO_PLAY_FILE;
        opParam.playParam.sampleRate = 1;
        gOpenPlayHandler = openPlayCreate(&opParam);
        openPlaySetCallback(gOpenPlayHandler, consoleAudPlayCallback, NULL);
        openPlay(gOpenPlayHandler, (void *)folder);
#else
        playAudioByName(folder);
#endif
    }
    else
    {
        rt_kprintf("\n\rinvalid path:%s", argv[1]);
    }
    
}
MSH_CMD_EXPORT_ALIAS(consolePlayAudio, play, play audio);

void consoleStopPlayAudio(int argc, char **argv)
{
    char folder[128];
    memset(folder, 0, sizeof(folder));

    if(argc == 1)
        stopPlayAudioByName(NULL);
    else if(strcasestr(argv[1], "http://") || strcasestr(argv[1], "https://"))
    {
        stopPlayAudioByName(argv[1]);
    }
    else if(fileNameToDir(argv[1], folder))
    {
        stopPlayAudioByName(folder);
    }
    else
    {
        rt_kprintf("\n\rinvalid path:%s", argv[1]);
    }
    
}
MSH_CMD_EXPORT_ALIAS(consoleStopPlayAudio, stop, stop audio);

void consolePausePlayAudio(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    openPlayPause(gOpenPlayHandler, true);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consolePausePlayAudio, pause, pause audio);

#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
void consoleRecord(int argc, char **argv)
{
    int time = 0;
    if(argc > 2) time = atoi(argv[2]);
    rt_kprintf("\n\rrecord time is: %d ", time);

#ifndef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    if(!checkPathPrefix(argv[1]))
    {
        rt_kprintf("\n\rinvalid path");
    }
    else
    {
        startRecordByName(argv[1], time);
    }
#else
    char *path = NULL;
    if(argc >= 2) path = argv[1];
    startRecordByName(path, time);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consoleRecord, record, record audio);

void consoleRecordStop(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    openRecorderStop(gOpenRecordHandler);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consoleRecordStop, recStop, stop record);

void consoleRecordPause(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    openRecorderPause(gOpenRecordHandler);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consoleRecordPause, recPause, pause record);

void consoleRecordResume(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    openRecorderResume(gOpenRecordHandler);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consoleRecordResume, recResume, Resume record);

void consoleRecordGetTime(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    rt_kprintf("\n\r%d", openRecorderGetRecTime(gOpenRecordHandler));
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consoleRecordGetTime, recGetTime, get record time);

#endif

#ifdef FEATURE_SUBSYS_TTS_ENABLE
void consoleAudPlayTTsCallback(void *userData, int32_t result)
{
    if(userData) free(userData);
    rt_kprintf("\r\n");
    rt_kprintf("\r\nplay result: %d", result);

    
}
void consolePlayTts(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
    openPlayerConfigT opParam = {0};
    opParam.playParam.store = AUDIO_PLAY_STRING;
    opParam.playParam.codec = AUDIO_PLAY_CODEC_TYPE_TTS;
    if(argv[1] == NULL)
    {
        
        return;
    }

    char *ttsData = (char *)malloc(strlen(argv[1]) + 1);
    EC_ASSERT(ttsData, 0, 0, 0);
    memset(ttsData, 0x00, strlen(argv[1]) + 1);
    memcpy(ttsData, argv[1], strlen(argv[1]));
    opParam.playParam.srcLen = strlen(argv[1]);
    gOpenPlayHandler = openPlayCreate(&opParam);
    openPlaySetCallback(gOpenPlayHandler, consoleAudPlayCallback,
                        (void *)ttsData);
    openPlay(gOpenPlayHandler, (void *)ttsData);
#else
    audioPlayTts(argv[1], NULL);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consolePlayTts, tts, play tts);
#endif

#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_PCM_ENABLE
void consolePlayPcm(int argc, char **argv)
{
    uint8_t data00h = 0;
    int16_t *dataSin = NULL;
    uint32_t lengthSin = 0;

    if((strlen(argv[1]) == strlen(PCM_PARAM_00H)) &&
       (memcmp(argv[1], PCM_PARAM_00H, strlen(PCM_PARAM_00H)) == 0))
    {
        audioPlayPcm(&data00h, sizeof(data00h), atoi(argv[2]));
    }
    else if((strlen(argv[1]) == strlen(PCM_PARAM_SIN)) &&
            (memcmp(argv[1], PCM_PARAM_SIN, strlen(PCM_PARAM_SIN)) == 0))
    {
        lengthSin = pcmSinGet(atoi(argv[3]), atoi(argv[2]), &dataSin);
        if(dataSin != NULL)
        {
            audioPlayPcm((uint8_t *)dataSin, lengthSin, atoi(argv[2]));
            free(dataSin);
            dataSin = NULL;
        }
    }
    else if((strlen(argv[1]) == strlen(PCM_PARAM_END)) &&
            (memcmp(argv[1], PCM_PARAM_END, strlen(PCM_PARAM_END)) == 0))
    {
        pcmEndPlay();
    }
    else
    {
        rt_kprintf("Unsupported pcm param.\r\n");
    }

    
}
MSH_CMD_EXPORT_ALIAS(consolePlayPcm, pcm, play pcm);
#endif

#ifdef FEATURE_SUBSYS_XFAI_ENABLE
static XfaiServer xfai_server = NULL;
void consoleXfaiAECSave(int argc, char **argv)
{
    XfaiAecCfg_t cfg = {0};
    char folder[256] = {0};
    openPlayerConfigT opParam = {0};
    if(!xfai_server)
    {
        xfai_server = xfai_server_create(NULL);
    }
    cfg.write_mode = 0;
    cfg.model = 1;
    cfg.mic_num = 1;
    xfai_server_aec_set_cfg(xfai_server, &cfg);
    xfai_server_start_record_aec_file(xfai_server, "d:/xfei_record.pcm", 10);
    rt_kprintf("\n\rXunfei AI AEC start record, please wait...\n\r");
    opParam.playParam.store = AUDIO_PLAY_FILE;
    opParam.playParam.sampleRate = 1;
    gOpenPlayHandler = openPlayCreate(&opParam);
    openPlaySetCallback(gOpenPlayHandler, consoleAudPlayCallback, NULL);
    openPlay(gOpenPlayHandler, "D:/xhls.mp3");
    osDelay(10000);  // wait for 10 seconds
    xfai_server_stop_record_aec_file(xfai_server);
    rt_kprintf("\n\rXunfei AI AEC stop record\r\n");
    
}
MSH_CMD_EXPORT_ALIAS(consoleXfaiAECSave, xfaiaecsave, Xunfei AI AEC save files);

void console_xfai_warmup(const char *warm_param, void *user_param)
{
    rt_kprintf("\n\rXunfei warmup : %s\r\n", warm_param);
}

void console_xfai_wakeup(const char *wake_param, void *user_param)
{
    rt_kprintf("\n\rXunfei wakeup : %s\r\n", wake_param);
}

void consoleXfaiStart(int argc, char **argv)
{
    XfaiWakeCfg_t wakeCfg = {0};
    if(!xfai_server)
    {
        xfai_server = xfai_server_create(NULL);
    }
    wakeCfg.use_siranbi = 0;
    wakeCfg.threshold = 500;
    xfai_server_start(xfai_server, console_xfai_warmup, NULL,
                      console_xfai_wakeup, NULL);
    //xfai_server_wakeup_set_cfg(xfai_server, &wakeCfg);
}
MSH_CMD_EXPORT_ALIAS(consoleXfaiStart, xfaistart, Xunfei AI start);

#endif

#ifdef FEATURE_SUBSYS_MED_PCM_ENABLE
void consolePlayPcmStr(int argc, char **argv)
{
    openPlayerConfigT opParam = {0};
    opParam.playParam.sampleRate = SAMPLERATE_16K;
    opParam.playParam.srcLen = 58464;
    opParam.playParam.store = AUDIO_PLAY_STRING;
    opParam.playParam.codec = AUDIO_PLAY_CODEC_TYPE_PCM;
#ifndef BSP_AIKIT
    gOpenPlayHandler = openPlayCreate(&opParam);
    extern const uint8_t callAlertRing16k[];
    openPlay(gOpenPlayHandler, (void *)callAlertRing16k);
#endif
    
}
MSH_CMD_EXPORT_ALIAS(consolePlayPcmStr, pcmStr, play pcm string demo);
#endif

#ifdef FEATURE_SUBSYS_MED_WAV_ENABLE
void consolePlayStr(int argc, char **argv)
{
    medSrcT *medStringSrc = NULL;
    uint32_t codec = atoi(argv[1]);
    if(codec < AUDIO_PLAY_CODEC_TYPE_PCM || codec > AUDIO_PLAY_CODEC_TYPE_WAV)
        return;
    medStringSrc = medFileToString(argv[2]);
    if(!medStringSrc) return;
    openPlayerConfigT opParam = {0};
    opParam.playParam.sampleRate = SAMPLERATE_16K;  // used for pcm default
    opParam.playParam.srcLen = medStringSrc->srcLen;
    opParam.playParam.store = AUDIO_PLAY_STRING;
    opParam.playParam.codec = codec;
    gOpenPlayHandler = openPlayCreate(&opParam);

    openPlay(gOpenPlayHandler, (void *)medStringSrc->src);

    
}
MSH_CMD_EXPORT_ALIAS(consolePlayStr, playString, play media by string demo);
#endif
#ifdef SPEAKER_APP
void consoleChangeVolume(int argc, char **argv)
{
    if((strlen(argv[1]) == strlen(VOLUME_PLUS_VPRESS_SHORT)) &&
       (memcmp(argv[1], VOLUME_PLUS_VPRESS_SHORT,
               strlen(VOLUME_PLUS_VPRESS_SHORT)) == 0))
    {
        audioAdjustVolume(ACTION_VOLUME_PLUS_SHORT);
    }
    else if((strlen(argv[1]) == strlen(VOLUME_PLUS_VPRESS_LONG)) &&
            (memcmp(argv[1], VOLUME_PLUS_VPRESS_LONG,
                    strlen(VOLUME_PLUS_VPRESS_LONG)) == 0))
    {
        audioAdjustVolume(ACTION_VOLUME_PLUS_LONG);
    }
    else if((strlen(argv[1]) == strlen(VOLUME_MINUS_VPRESS_SHORT)) &&
            (memcmp(argv[1], VOLUME_MINUS_VPRESS_SHORT,
                    strlen(VOLUME_MINUS_VPRESS_SHORT)) == 0))
    {
        audioAdjustVolume(ACTION_VOLUME_MINUS_SHORT);
    }
    else if((strlen(argv[1]) == strlen(VOLUME_MINUS_VPRESS_LONG)) &&
            (memcmp(argv[1], VOLUME_MINUS_VPRESS_LONG,
                    strlen(VOLUME_MINUS_VPRESS_LONG)) == 0))
    {
        audioAdjustVolume(ACTION_VOLUME_MINUS_LONG);
    }
    else
    {
        rt_kprintf("Unsupported virtual volume.\r\n");
    }

    
}
MSH_CMD_EXPORT_ALIAS(consoleChangeVolume, volume, change volume);
#endif
#endif
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_SOFTCODEC_ENABLE
int16_t gPcmBuf[1024 * 20];
int consolePlayPwm(int argc, char **argv)
{
    uint32_t rate = atoi(argv[3]);
    uint32_t freq = atoi(argv[1]);
    uint32_t swi = atoi(argv[2]);
    uint32_t shift = atoi(argv[4]);
    uint32_t basefreq = atoi(argv[5]);
    uint32_t count = rate / freq;
    uint32_t length = count * 2;
    FILE *pcmFile = NULL;
    struct stat buf = {0};
    uint32_t size = 0;
    char *fileName = "c:/NetConnetOk.wav";
    fileName = argv[1];

    // printf("consolePlayPwm freq:%s;swi:%s",argv[1],argv[2]);
    printf("consolePlayPwm freq:%d;swi:%d;rate:%d", freq, swi, rate);
    pcmFile = file_fopen(fileName, "r");
    if(pcmFile != NULL)
    {
        file_fstat((int)pcmFile, &buf);
        size = buf.st_size;

        rt_kprintf("\n\r fileName:%s\n\r size:%d \r\n", fileName, size);
        if(size > 1024 * 20) size = 1024 * 40;

        file_fread(gPcmBuf, 1, size, pcmFile);

        for(int i = 0; i < 20; i++) rt_kprintf("0x%02X ", gPcmBuf[i]);
        // rt_kprintf("%s",buffer);
        file_fclose(pcmFile);
    }
    else
    {
        rt_kprintf("\n\r fileName:%s not exist !!\n\r", fileName);
    }

    pwmdaInit(rate, basefreq, shift);

    // data prepare
    // for (uint32_t i=0; i<count; i++)
    // {
    //     gPcmBuf[i] = (uint16_t)(400 * (sin(2 * 3.14 * i / count)+1));
    //     printf("%04X | ", gPcmBuf[i]);
    //     // gPcmBuf[i] = (int16_t)(1024 * sin(2 * 3.14 * i / count));
    //     // printf("%02X %02X | ", (gPcmBuf[i] & 0xFF), (((gPcmBuf[i] >> 8) &
    //     0xFF)));
    // }
    pwmdaSetData(gPcmBuf, size, shift);

    // play
    if(swi == 1) pwmdaStart();

    if(swi == 0) pwmdaStop();

    return 0;
}
MSH_CMD_EXPORT_ALIAS(consolePlayPwm, playpwm, tasks status);
#endif
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_SOFTCODEC_ENABLE
int consolePlayPwms(int argc, char **argv)
{
    uint32_t rate = atoi(argv[3]);
    uint32_t freq = atoi(argv[1]);
    uint32_t swi = atoi(argv[2]);
    uint32_t shift = atoi(argv[4]);
    uint32_t basefreq = atoi(argv[5]);
    uint32_t count = rate / freq;
    uint32_t length = count * 2000;
    lfs_file_t pcmFile = {0};
    int wavehdrLen = 44;
    uint32_t size = wavehdrLen + length;

    pwmdaInit(rate, basefreq, shift);

    // data prepare
    for(uint32_t i = wavehdrLen / 2; i < size / 2; i++)
    {
        gPcmBuf[i] = (int16_t)(32768 * sin(2 * 3.1416 * i / count));
        // printf("%d | ", gPcmBuf[i]);
        // gPcmBuf[i] = (int16_t)(1024 * sin(2 * 3.14 * i / count));
        // printf("%02X %02X | ", (gPcmBuf[i] & 0xFF), (((gPcmBuf[i] >> 8) &
        // 0xFF)));
    }
    pwmdaSetData(gPcmBuf, size, shift);

    // play
    if(swi == 1) pwmdaStart();

    if(swi == 0) pwmdaStop();

    return 0;
}
MSH_CMD_EXPORT_ALIAS(consolePlayPwms, playpwms, tasks status);
#endif
#endif

#endif