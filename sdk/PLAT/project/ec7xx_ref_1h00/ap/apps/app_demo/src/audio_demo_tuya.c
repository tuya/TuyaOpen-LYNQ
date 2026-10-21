/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "ostask.h"
#include "os_common.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "audio.h"
#include "record.h"
#include "ol_gpio_api.h"
#include "test_pcm.h"

static int audio_init = 0;
int test_play_status = 0;
int test_record_status = 0;
int test_record_size = 0;
uint8_t test_record_data[100*1024] = {0};

int pad_hook(void)
{
    // 低电平开启PA
    ol_gpio_config_struct cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.gpio_func = OL_GPIO_FUNC4;
    cfg.gpio_dir = OL_GPIO_OUTPUT;
    ol_pin_config(21, &cfg);
    return 21;
}

void test_play_cb(void)
{
    OL_LOG_INFO("test play cb");
    test_play_status = 0;
}

void test_record_cb(char * r_data, uint32_t r_size)
{
    OL_LOG_INFO("test record cb data size = %d, record_size = %d", r_size, test_record_size);

    if((sizeof(test_record_data) - test_record_size) > r_size)
    {
        memcpy(&test_record_data[test_record_size], r_data, r_size);
        test_record_size += r_size;
    }
    else
    {
        test_record_status = 0;
    }
}

void test_audio_pcm_play(void)
{
    OL_LOG_INFO("test pcm play");
    test_play_status = 1;
    ol_audioPlayPcmData(test_record_data, test_record_size, SAMPLERATE_16K,test_play_cb, 1);
    ol_audioPlayPcmData(audio_test_pcm_buffer, sizeof(audio_test_pcm_buffer), SAMPLERATE_16K, test_play_cb, 1);
    ol_audioPlayPcmData(audio_test_pcm_buffer, sizeof(audio_test_pcm_buffer), SAMPLERATE_16K, test_play_cb, 0);
    while(1 == test_play_status)
    {
        osDelay(50);
    }
}

void test_audio_pcm_record(void)
{
    RecordParamT recordParam = {0};
    OL_LOG_INFO("test pcm record");
    memset(test_record_data, 0x0, sizeof(test_record_data));
    test_record_status = 1;
    test_record_size = 0;
    recordParam.time = 10; //record time(second) //0:record continuously
    recordParam.samplerate = SAMPLERATE_16K;
    recordParam.codec = AUDIO_RECORD_CODEC_PCM;
    ol_audioRecord(recordParam, test_record_cb);
    while(test_record_status)
    {
        osDelay(500);
    }
    ol_audioRecordStop();
}

void audio_demo(void)
{
    OL_LOG_INFO("test audio demo");

    if(0 == audio_init)
    {
        subMediaInit();
        audio_init = 1;
    }

    osDelay(1000);
    test_audio_pcm_record();

    osDelay(1000);
    test_audio_pcm_play();

    OL_LOG_INFO("test audio demo END");
}

