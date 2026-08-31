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
#include "test1_mp3.h"
#include "test2_mp3.h"
#include "test_pcm.h"
//#include "test_amr.h"


static int audio_init = 0;

int test_play_status = 0;

int test_record_status = 0;
int test_record_size = 0;
uint8_t test_record_data[100*1024] = {0};


int pad_hook()
{
	ol_gpio_config_struct config = { 0 };
	 
	int pin_num = 5;
    memset(&config,0x0,sizeof(config));
    config.gpio_func = OL_GPIO_FUNC0;
    config.gpio_dir = OL_GPIO_OUTPUT;

    ol_pin_config(pin_num, &config);
	
	return pin_num;
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

void test_audio_play(void)
{
    OL_LOG_INFO("test audio play");

    if(1)
    {
        test_play_status = 1;
        ol_audioPlayAmrData(test_record_data, test_record_size, test_play_cb);
        //ol_audioPlayAmrData(test_amr_data, sizeof(test_amr_data), test_play_cb);
    }
    else
    {
        test_play_status = 1;
        audioPlayAmr("C:/a.amr", test_play_cb);
    }

    while(1 == test_play_status)
    {
        osDelay(500);
    }
}

void test_audio_pcm_play(void)
{
    OL_LOG_INFO("test pcm play");
    static int test_op = 0;

    if(0 == test_op)
    {
        test_op = 1;
        test_play_status = 1;
        //ol_audioPlayPcmData(test_record_data, test_record_size, SAMPLERATE_16K,test_play_cb, 0);
        ol_audioPlayPcmData(test_pcm_buff1, sizeof(test_pcm_buff1), SAMPLERATE_16K, test_play_cb, 1);
        ol_audioPlayPcmData(test_pcm_buff2, sizeof(test_pcm_buff2), SAMPLERATE_16K, test_play_cb, 0);
        while(1 == test_play_status)
        {
            osDelay(50);
        }
    }
    else
    {
        test_op = 0;
        test_play_status = 1;
        ol_audioPlayPcmData(test_pcm_buff1, sizeof(test_pcm_buff1), SAMPLERATE_16K, test_play_cb, 0);
        while(1 == test_play_status)
        {
            osDelay(50);
        }

        test_play_status = 1;
        ol_audioPlayPcmData(test_pcm_buff2, sizeof(test_pcm_buff2), SAMPLERATE_16K, test_play_cb, 0);
        while(1 == test_play_status)
        {
            osDelay(50);
        }

    }
}

void test_audio_record(void)
{
    RecordParamT recordParam = {0};

    OL_LOG_INFO("test audio record");

    if(1)
    {
        memset(test_record_data, 0x0, sizeof(test_record_data));
        test_record_status = 1;
        test_record_size = 0;
        recordParam.time = 0;
        recordParam.codec = AUDIO_RECORD_CODEC_AMR;
        ol_audioRecord(recordParam, test_record_cb);

        while(test_record_status)
        {
            osDelay(500);
        }

        ol_audioRecordStop();
    }
    else
    {
        recordParam.time = 5;  //record time(second) //0:record continuously
        recordParam.codec = AUDIO_RECORD_CODEC_AMR;
        EC_audioRecord("C:/a.amr", recordParam, NULL);
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
#if 0
    else
    {
        recordParam.time = 10;  //record time(second) //0:record continuously
        recordParam.codec = AUDIO_RECORD_CODEC_PCM;
        EC_audioRecord("C:/a.wav", recordParam, NULL);
    }
#endif
}


void test_mp3_play(void)
{
    int size = 0;

    OL_LOG_INFO("test play mp3");
#if 1
    test_play_status = 1;
    size = sizeof(test_mp3_data1);
    ol_audioPlayMp3Data(test_mp3_data1, size, test_play_cb);
    while(1 == test_play_status)
    {
        osDelay(500);
    }

    osDelay(1000);

    test_play_status = 1;
    size = sizeof(test_mp3_data2);
    ol_audioPlayMp3Data(test_mp3_data2, size, test_play_cb);
    while(1 == test_play_status)
    {
        osDelay(500);
    }

    osDelay(1000);

    OL_LOG_PRINTF("test play MP3 file");
    audioPlayMp3("C:/low_power.mp3", NULL, false, false);
#endif
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
    test_mp3_play();

    //osDelay(1000);
    //test_audio_record();

    //osDelay(1000);
    //test_audio_play();

   // osDelay(1000);
  //  test_audio_pcm_record();

   // osDelay(1000);
  //  test_audio_pcm_play();

    OL_LOG_INFO("test audio demo END");
}

