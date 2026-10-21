#if 1 

#include <stdbool.h>
#include <stdio.h>

#include "slpman.h"
#include "ec_ring.h"
#include "medDataHandle.h"

#include "cmsis_os2.h"
#include "ol_log.h"

#define MED_PCM_FRAME_SIZE					(1920)
#define PCM_DATA_CACHE_PRE_FILL_SIZE		(16*1024)	
#define PCM_DATA_CACHE_SIZE 				(PCM_DATA_CACHE_PRE_FILL_SIZE + MED_PCM_FRAME_SIZE*3)

static bool gWakeup = false;
static ecRingT *pcmCache = NULL;

static void restoreCallback(void *p_data, slpManLpState state)
{
    gWakeup = true;
}

int audio_play_init(void)
{
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerInit();
#endif

    // audioRegisterToneCb(audioToneCallback);
    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);
    // slpManApplyPlatVoteHandle("audio", &vote);
    pcmCache = medInitDataCache(PCM_DATA_CACHE_SIZE, 0);
    return 0;
}

int audio_play_start(uint16_t samplerate, uint8_t bitWidth)
{
    rawDataHanderParam_T dataHandleParam = {0};
    dataHandleParam.field.samplerate = samplerate;
	dataHandleParam.field.envType = MED_DATA_ENV_TYPE_LOCAL;
	dataHandleParam.field.bitWidth = bitWidth;
	medDataHandleStart(&dataHandleParam);
    return 0;
}

int audio_play_put_frame(uint8_t *pcmData, uint32_t size)
{
    OL_LOG_INFO("audio_play_put_frame size = %d", size);
    uint32_t write_len = 0;
    int len;
    int buffer_space_len = 0;
    while(write_len < size) {
        buffer_space_len = medGetDataCacheAvlbSize();
        len = size - write_len > buffer_space_len ? buffer_space_len : size- write_len;
        if(len) {
            len = xEcRingWriteEx(pcmCache, pcmData + write_len, len);
            write_len += len;
        } 

        osDelay(50);
        OL_LOG_INFO("buffer_space_len = %d, write_len = %d, size = %d", buffer_space_len, write_len, size);
    }
    return 0;
}

int audio_play_stop(void)
{
    medDataHandleStop();
    return 0;
}

#endif


#include "record.h"
#include "hal_i2s.h"

static int test_record_size = 0;
static uint8_t test_record_data[100*1024] = {0};
static int test_record_status = 0;

static void test_record_cb(char * r_data, uint32_t r_size)
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

void test_audio_pcm_record(void)
{
    RecordParamT recordParam = {0};
    OL_LOG_INFO("test pcm record");
    if(1)
    {
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
}

#include "test_pcm.h"
void test_audio_pcm_play(void)
{
    OL_LOG_INFO("test pcm play");
    OL_LOG_INFO("test pcm size :%d", test_record_size);

    audio_play_start(SAMPLERATE_16K, 16);
    audio_play_put_frame(audio_test_pcm_buffer, sizeof(audio_test_pcm_buffer));

    {
        osDelay(10000);
    }
    audio_play_stop();
    OL_LOG_INFO("test pcm stop");
}
#include "ol_gpio_api.h"

int pad_hook()
{
    // 低电平开启PA
    ol_gpio_config_struct cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.gpio_func = OL_GPIO_FUNC4;
    cfg.gpio_dir = OL_GPIO_OUTPUT;
    ol_pin_config(21, &cfg);
    return 21;
}


void audio_demo(void)
{
    OL_LOG_INFO("test audio demo");
    //使能PA
    ol_gpio_config_struct config;
    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC4;
    config.gpio_dir = OL_GPIO_OUTPUT;
    ol_pin_config(21,&config);
    ol_pin_set_level(21, OL_GPIO_LEVEL_LOW);
    osDelay(1000);

    recordInit();
    audio_play_init();

    // osDelay(1000);
    // test_audio_pcm_record();

    osDelay(1000);
    test_audio_pcm_play();

    osDelay(1000);
    test_audio_pcm_play();

    OL_LOG_INFO("test audio demo END");
}
