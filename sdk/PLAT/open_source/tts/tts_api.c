/****************************************************************************
 *
 * Description:  speak demo entry source file
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "osasys.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "slpman.h"

#include "ol_log.h"
#include "tts_api.h"


#define TTS_THREAD_STACK_SIZE     (4 * 1024)
#define TTS_QUEUE_SIZE            64

static osMessageQueueId_t sg_tts_queue = NULL;

extern int tts_data_convert(char* p_text, uint32_t p_length, tts_decode_enum decode);


static void p_thread_tts(void* arg)
{
    tts_queue_t tts_q = {0};
    uint8_t tts_slp_handle = 0xFF;

    sg_tts_queue = osMessageQueueNew(TTS_QUEUE_SIZE, sizeof(tts_q), NULL);
    if(NULL == sg_tts_queue)
    {
        OL_LOG_ERROR("Failed to create queue for sg_speak_queue.\r\n");
        return;
    }

    slpManApplyPlatVoteHandle("tts_slp_handle", &tts_slp_handle);

    while(1)
    {
        slpManPlatVoteEnableSleep(tts_slp_handle, SLP_SLP1_STATE);
        memset(&tts_q, 0, sizeof(tts_q));
        if(osOK == osMessageQueueGet(sg_tts_queue, &tts_q, 0, osWaitForever))
        {
            slpManPlatVoteDisableSleep(tts_slp_handle, SLP_SLP1_STATE);

            tts_data_convert(tts_q.p_buffer, tts_q.p_length, tts_q.decode);
            free(tts_q.p_buffer);
            tts_q.p_buffer = NULL;
        }
    }
}

void tts_play(char* p_text, uint32_t p_length, tts_decode_enum decode)
{
    tts_queue_t tts_q = {0};

    if(NULL != p_text)
    {
        tts_q.decode = decode;
        tts_q.p_length = p_length;
        tts_q.p_buffer = malloc(p_length + 1);
        if(NULL != tts_q.p_buffer)
        {
            memset(tts_q.p_buffer, 0, p_length + 1);
            memcpy(tts_q.p_buffer, p_text, p_length);
            osMessageQueuePut(sg_tts_queue, &tts_q, 0, osWaitForever);
        }
    }
}

void tts_init(void)
{
    osThreadAttr_t thread_attr = {0};

    memset(&thread_attr, 0, sizeof(thread_attr));
    thread_attr.name       = "thread_tts";
    thread_attr.stack_size = TTS_THREAD_STACK_SIZE;
    thread_attr.priority   = osPriorityAboveNormal;
    osThreadNew(p_thread_tts, NULL, &thread_attr);
}

