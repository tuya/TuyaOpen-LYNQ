/*
 * tkl_audio.c
 * Copyright (C) 2024 cc <cc@tuya>
 *
 * Distributed under terms of the MIT license.
 */
#include "tuya_iot_config.h"

#if defined(ENABLE_CATX_PLAYER) && ENABLE_CATX_PLAYER == 1

#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portmacro.h"
#include "portable.h"

#include "ol_gpio_api.h"
#include "record.h"
#include "hal_i2s.h"
#include "slpman.h"
#include "slpman.h"
#include "ec_ring.h"
#include "medDataHandle.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "audio.h"
#include "audPcm.h"

#include "tuya_cloud_types.h"
#include "tuya_ringbuf.h"
#include "tkl_audio.h"
#include "tkl_gpio.h"
#include "tkl_semaphore.h"
#include "tkl_system.h"
#include "tkl_output.h"
#include "tkl_mutex.h"
#include "tkl_thread.h"
#include "tkl_queue.h"
#include "volume_ctl.h"

/* Match audPcm.c cache sizing for medInitDataCache / drain logic (G SDK may omit these) */
#ifndef MED_PCM_FRAME_SIZE
#define MED_PCM_FRAME_SIZE              (1920)
#endif
#ifndef PCM_DATA_CACHE_PRE_FILL_SIZE
#define PCM_DATA_CACHE_PRE_FILL_SIZE    (16 * 1024)
#endif
#ifndef PCM_DATA_CACHE_SIZE
#define PCM_DATA_CACHE_SIZE             (PCM_DATA_CACHE_PRE_FILL_SIZE + MED_PCM_FRAME_SIZE * 3)
#endif

#define USE_PA_PIN_CTRL  0
#if USE_PA_PIN_CTRL == 1
#define TKL_AUDIO_PA_GPIO_NUM  21
#define TKL_AUDIO_PA_ENABLE() ol_pin_set_level(TKL_AUDIO_PA_GPIO_NUM, OL_GPIO_LEVEL_LOW)
#define TKL_AUDIO_PA_DISABLE() ol_pin_set_level(TKL_AUDIO_PA_GPIO_NUM, OL_GPIO_LEVEL_HIGH)
static int audio_pa_init(void)
{
    int ret;
    ol_gpio_config_struct cfg;
    
    memset(&cfg, 0, sizeof(cfg));
    cfg.gpio_func = OL_GPIO_FUNC4;
    cfg.gpio_dir = OL_GPIO_OUTPUT;

    ret = ol_pin_config(TKL_AUDIO_PA_GPIO_NUM, &cfg);
    if (ret < 0) {
        LOGE("init audio PA failed(ret=%d)", ret);
    }
     
    return ret;
}
#else
#define TKL_AUDIO_PA_ENABLE() 
#define TKL_AUDIO_PA_DISABLE()
#endif

#undef AUDIO_PLAY_ORIGINAL_MODE_ENABLE
#define AUDIO_PLAY_ORIGINAL_MODE_ENABLE 0

#define TKL_READ_8K_PCM_FRAME_SIZE 320 /* 8KHz sample * 20ms * 8bits =  320 bytes */
#define TKL_READ_8K_PCM_FRAME_SIZE_PER_SEC ((1000 / 20) * TKL_READ_8K_PCM_FRAME_SIZE)

#define TKL_READ_16K_PCM_FRAME_SIZE 640/* 16KHz sample * 20ms * 16bits =  640 bytes */
#define TKL_READ_16K_PCM_FRAME_SIZE_PER_SEC ((1000 / 20) * TKL_READ_16K_PCM_FRAME_SIZE)

#define GET_TKL_AUDIO_8K_PCM_CACHE_SIZE(sec) ((sec) * TKL_READ_8K_PCM_FRAME_SIZE_PER_SEC)
#define GET_TKL_AUDIO_16K_PCM_CACHE_SIZE(sec) ((sec) * TKL_READ_16K_PCM_FRAME_SIZE_PER_SEC)

#define AUIO_READ_CACHE_BUFFER_SIZE (1*640)

#ifdef ASSERT
#undef ASSERT
#endif

#define TKL_AUDIO_DBG_EN 1
#if TKL_AUDIO_DBG_EN
#define LOGI(fmt, args...) tkl_log_output("%s:%d " fmt "\n", "tkl_audio", __LINE__, ##args)
#define LOGD(fmt, args...) tkl_log_output("%s:%d " fmt "\n", "tkl_audio", __LINE__, ##args)
#define LOGE(fmt, args...) tkl_log_output("%s:%d " fmt "\n", "tkl_audio", __LINE__, ##args)
#define ASSERT(condition) do { if (!(condition)) {LOGE("ASSERT: %s", #condition);}} while (0)
#else
#define LOGD(fmt, args...) do{}while(0)
#define LOGI(fmt, args...) tkl_log_output("%s:%d " fmt "\n", "tkl_audio", __LINE__, ##args)
#define LOGE(fmt, args...) tkl_log_output("%s:%d " fmt "\n", "tkl_audio", __LINE__, ##args)
#define ASSERT(condition)
#endif

static uint8_t audio_sleep_vote = 0xFF;
#define TKL_AUDIO_SLEEP_INIT()  slpManApplyPlatVoteHandle("audio", &audio_sleep_vote)
#define TKL_AUDIO_SLEEP_ENABLE() slpManPlatVoteEnableSleep(audio_sleep_vote, SLP_SLP1_STATE)
#define TKL_AUDIO_SLEEP_DISABLE() slpManPlatVoteDisableSleep(audio_sleep_vote, SLP_SLP1_STATE)

#define TKL_AUDIO_MUTEX_LOCK_EN 0
#if TKL_AUDIO_MUTEX_LOCK_EN
static TKL_MUTEX_HANDLE audio_lock = NULL;
#define AUDIO_MUTEX_LOCK_INIT() do {                     \
    if (NULL == audio_lock) {                                \
        if (OPRT_OK != tkl_mutex_create_init(&audio_lock)) { \
            LOGE("audio create mutex lock failed");          \
        }                                                    \
    } \
}while(0)
#define AUDIO_MUTEX_LOCK() tkl_mutex_lock(audio_lock)
#define AUDIO_MUTEX_UNLOCK() tkl_mutex_unlock(audio_lock)
#define AUDIO_MUTEX_RELEASE() tkl_mutex_release(audio_lock)
#else
#define AUDIO_MUTEX_LOCK_INIT() do{}while(0)
#define AUDIO_MUTEX_LOCK()      do{}while(0)
#define AUDIO_MUTEX_UNLOCK()    do{}while(0)
#define AUDIO_MUTEX_RELEASE()   do{}while(0)
#endif

#define TKL_AUDIO_SEMAPHORE_EN 0

static rawDataHanderParam_T tkl_ao_params;
static uint8_t tkl_ai_state = 0, tkl_ao_state = 0;
static TKL_FRAME_PUT_CB user_audio_read_handler = NULL;
static RecordParamT tkl_record_params;
static int audio_read_size = 0;
static uint8_t audio_read_buffer[AUIO_READ_CACHE_BUFFER_SIZE] = { 0 };
#if TKL_AUDIO_SEMAPHORE_EN == 1
static  TKL_SEM_HANDLE tkl_audio_play_sem = NULL;
#endif

#if !AUDIO_PLAY_ORIGINAL_MODE_ENABLE 
ecRingT *audio_play_cache = NULL;
#endif

void audio_play_complete_handler(int32_t status)
{
    // LOGD("===status %d", status); 
#if TKL_AUDIO_SEMAPHORE_EN   
    if (tkl_audio_play_sem) {
        tkl_semaphore_post(tkl_audio_play_sem);
    }
#endif    
}

static void audio_read_handler(char *buffer, uint32_t size)
{
    TKL_AUDIO_FRAME_INFO_T audio_frame;
    uint32_t audio_read_buffer_size = sizeof(audio_read_buffer);

    // LOGD("read_buffer_size %d size %d audio_read_size %d", audio_read_buffer_size, size, audio_read_size);
    if ((audio_read_buffer_size - audio_read_size) >= size) {
        memcpy(&audio_read_buffer[audio_read_size], buffer, size);
        audio_read_size += size;

        if (audio_read_buffer_size == audio_read_size) {
            if (user_audio_read_handler) {
                memset(&audio_frame, 0, sizeof(audio_frame));
                audio_frame.buf_size = audio_read_size;
                audio_frame.codectype = TKL_CODEC_AUDIO_PCM;
                audio_frame.datebits =  TKL_AUDIO_DATABITS_16;
                audio_frame.sample = tkl_record_params.samplerate == SAMPLERATE_8K ? TKL_AUDIO_SAMPLE_8K : TKL_AUDIO_SAMPLE_16K;
                audio_frame.pbuf = (CHAR_T *)&audio_read_buffer;
                user_audio_read_handler(&audio_frame);
            }
            audio_read_size = 0;
        }
    } else {
        ASSERT(0);
    }
}

/**
* @brief ai init
* 
* @param[in] pconfig: audio config
* @param[in] count: count of pconfig
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_init(TKL_AUDIO_CONFIG_T *pconfig, INT32_T count)
{
    if(NULL == pconfig) {
        return OPRT_INVALID_PARM;
    }
    if(pconfig->codectype != TKL_CODEC_AUDIO_PCM) {
        return OPRT_NOT_SUPPORTED;
    }
    if(pconfig->sample != TKL_AUDIO_SAMPLE_8K && pconfig->sample != TKL_AUDIO_SAMPLE_16K) {
        return OPRT_NOT_SUPPORTED;
    }
    memset(&tkl_record_params, 0, sizeof(tkl_record_params));
    tkl_record_params.codec = AUDIO_RECORD_CODEC_PCM;
    tkl_record_params.samplerate = pconfig->sample == TKL_AUDIO_SAMPLE_8K ? SAMPLERATE_8K : SAMPLERATE_16K;
    tkl_record_params.time = 0;                         /* always record unit stop it by user */
    user_audio_read_handler = pconfig->put_cb;

    LOGI("ai init sample %d succ", tkl_record_params.samplerate);
    return OPRT_OK;
}

/**
* @brief ai start
* 
* @param[in] card: card number
* @param[in] chn: channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_start(INT32_T card, TKL_AI_CHN_E chn)
{
    if (tkl_ai_state) {
        return OPRT_OK;
    }
    
    if (recordInit() < 0) {
        LOGE("recordInit failed");
        return OPRT_COM_ERROR;
    }
    
    if (ol_audioRecord(tkl_record_params, audio_read_handler) < 0) {
        LOGE("ol_audioRecord failed");
        return OPRT_COM_ERROR;
    }

    tkl_ai_state = 1;
    LOGI("ai start succ");
    return OPRT_OK;
}

/**
* @brief ai get frame
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[out] pframe: audio frame, pframe->pbuf allocated by upper layer application
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_get_frame(INT32_T card, TKL_AI_CHN_E chn, TKL_AUDIO_FRAME_INFO_T *pframe)
{
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief ai stop
* 
* @param[in] card: card number
* @param[in] chn: channel number
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_stop(INT32_T card, TKL_AI_CHN_E chn)
{
    if(!tkl_ai_state)
        return OPRT_OK;
    ol_audioRecordStop();
    tkl_ai_state = 0;
    LOGI("ai stop");
    return OPRT_OK;
}

/**
* @brief ai uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_uninit(VOID)
{
    return OPRT_OK;
}

/**
* @brief ai set mic volume
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] vol: mic volume,[0, 100]
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_set_vol(INT32_T card, TKL_AI_CHN_E chn, INT32_T vol)
{
    return codec_mic_set_volume(MIC_TYPE_AUDIO, vol);
}

OPERATE_RET tkl_ai_get_vol(INT32_T card, TKL_AI_CHN_E chn, INT32_T *vol)
{
    return codec_mic_get_volume(MIC_TYPE_AUDIO, vol);
}

/**
* @brief ao init
* 
* @param[in] pconfig: audio config
* @param[in] count: config count
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_init(TKL_AUDIO_CONFIG_T *pconfig, INT32_T count, VOID **handle)
{
    if(NULL == pconfig) {
        return OPRT_INVALID_PARM;
    }
    if(pconfig->codectype != TKL_CODEC_AUDIO_PCM) {                                                                                                     
        return OPRT_NOT_SUPPORTED;
    }
    if(pconfig->sample != TKL_AUDIO_SAMPLE_8K && pconfig->sample != TKL_AUDIO_SAMPLE_16K) {
        return OPRT_NOT_SUPPORTED;
    }

    memset(&tkl_ao_params, 0, sizeof(tkl_ao_params));
    tkl_ao_params.field.bitWidth = 0;                   //默认16bit
    tkl_ao_params.field.samplerate = pconfig->sample == TKL_AUDIO_SAMPLE_8K ? SAMPLERATE_8K : SAMPLERATE_16K;                  
    tkl_ao_params.field.envType = MED_DATA_ENV_TYPE_LOCAL;

    AUDIO_MUTEX_LOCK_INIT();
#if USE_PA_PIN_CTRL == 1
    if (audio_pa_init() < 0) {
        AUDIO_MUTEX_RELEASE();
        return OPRT_COM_ERROR;
    }
#else
    tkl_audio_pa_pin_set(pconfig->spk_gpio, pconfig->spk_gpio_polarity);
#endif
#if TKL_AUDIO_SEMAPHORE_EN
    if (NULL == tkl_audio_play_sem) {
        ret = tkl_semaphore_create_init(&tkl_audio_play_sem, 0, 1);
        if ((ret < 0) || (NULL == tkl_audio_play_sem)) {
            AUDIO_MUTEX_RELEASE();
            return OPRT_COM_ERROR;
        }
    }
#endif
    TKL_AUDIO_PA_ENABLE();
    LOGI("ao init sample:%d pa_pin:%d", tkl_ao_params.field.samplerate, pconfig->spk_gpio);
    return OPRT_OK;
}

/**
* @brief ao start
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[out] handle: handle of start
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_start(INT32_T card, TKL_AO_CHN_E chn, VOID *handle)
{
    if (tkl_ao_state) {
        return OPRT_OK;
    }
    LOGI("tkl_ao_start");
#if AUDIO_PLAY_ORIGINAL_MODE_ENABLE     
    if (audioInit() < 0) {
        return OPRT_COM_ERROR;
    }
#else
    TKL_AUDIO_SLEEP_INIT();

    audio_play_cache = medInitDataCache(PCM_DATA_CACHE_SIZE, 0);        //仅首次调用，底层才会create ringbuffer
    if (NULL == audio_play_cache) {
        LOGE("create audio play data cache failed");
        return OPRT_COM_ERROR;
    }
#endif
    TKL_AUDIO_SLEEP_DISABLE();
    tkl_ao_state = 1;
    return OPRT_OK;
}

/**
* @brief ao output frame
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] handle: handle of start
* @param[in] pframe: output frame
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_put_frame(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, TKL_AUDIO_FRAME_INFO_T *pframe)
{
    ASSERT(NULL != pframe);
    ASSERT(NULL != pframe->pbuf);

    LOGD("tkl_ao_put_frame: %d %d %d", pframe->used_size, tkl_ao_params.field.samplerate, medGetDataCacheAvlbSize());
#if !AUDIO_PLAY_ORIGINAL_MODE_ENABLE     
    uint32_t cache_free_size;
    uint32_t buff_size;
    uint32_t write_size = 0;
    uint32_t pick_size;
    CHAR_T *pbuf;
#endif

    if (!tkl_ao_state) {
        return OPRT_COM_ERROR;
    }

    AUDIO_MUTEX_LOCK();
    if(medDataHandleStateGet() != MED_DATA_HDL_STA_START) {
        LOGI("medDataHandleStart");
        medDataHandleStart(&tkl_ao_params);
    }

#if AUDIO_PLAY_ORIGINAL_MODE_ENABLE             
    ol_audioPlayPcmData((uint8_t *)pframe->pbuf, pframe->buf_size, tkl_ao_params.field.samplerate, audio_play_complete_handler, 0);
#else
    pbuf = pframe->pbuf;
    buff_size = pframe->used_size;
    while (write_size < buff_size) {
        if(medDataHandleStateGet() != MED_DATA_HDL_STA_START) {
            LOGI("medDataHandleStateGet not start");
            break;
        }

        pick_size = ((buff_size - write_size) > MED_PCM_FRAME_SIZE) ? MED_PCM_FRAME_SIZE : (buff_size - write_size);
        cache_free_size = medGetDataCacheAvlbSize();
        if (cache_free_size < pick_size) {
            tkl_system_sleep(40);
            // LOGD("no enough cache memory(cache_free_size=%d %d)", cache_free_size, medGetDataCacheAvlbSize());
            continue;
        }
        pick_size = xEcRingWriteEx(audio_play_cache, (uint8_t *)(pbuf + write_size), pick_size);
        write_size += pick_size;
    }
    TKL_AUDIO_SLEEP_ENABLE();
#endif

#if TKL_AUDIO_SEMAPHORE_EN
    if (tkl_audio_play_sem) {
        tkl_semaphore_wait(tkl_audio_play_sem, osWaitForever);
    }
#endif

    AUDIO_MUTEX_UNLOCK();
    return OPRT_OK;
}

/**
* @brief ao stop
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] handle: handle of start
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/ 
OPERATE_RET tkl_ao_stop(INT32_T card, TKL_AO_CHN_E chn, VOID *handle)
{
    LOGI("tkl_ao_stop %d", tkl_ao_state);
    if(!tkl_ao_state)
        return OPRT_OK;
#if AUDIO_PLAY_ORIGINAL_MODE_ENABLE    
    audPcmPlayStop();
#else
    // Wait for the ring buffer to drain before stopping so that
    // audio fed just before stop() is fully played out.
    // PCM_DATA_CACHE_SIZE ≈ 22144; buffer is empty when avlb returns near full.
    int drain_wait = 0;
    while (medGetDataCacheAvlbSize() < (PCM_DATA_CACHE_SIZE - MED_PCM_FRAME_SIZE)
           && drain_wait < 150) {
        tkl_system_sleep(20);
        drain_wait++;
    }
    medDataHandleStop();                        // stop，底层不释放ringbuffer
#endif
    tkl_ao_state = 0;
    LOGI("tkl_ao_stop %d", tkl_ao_state);
    return OPRT_OK;
}

/**
* @brief ao uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_uninit(VOID *handle)
{
#if TKL_AUDIO_SEMAPHORE_EN    
    if (tkl_audio_play_sem) {
        tkl_semaphore_release(tkl_audio_play_sem);
        tkl_audio_play_sem = NULL;
    }
#endif    
    memset(&tkl_ao_params, 0, sizeof(tkl_ao_params));
    AUDIO_MUTEX_RELEASE();
    return OPRT_OK;
}

/**
* @brief ao set volume
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] vol: mic volume,[0, 100]
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_set_vol(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, INT32_T vol)
{
    return codec_volume_set(VOLUME_TYPE_AUDIO, vol);
}

/**
* @brief ao get volume
* 
* @param[in] card: card number
* @param[in] chn: channel number
* @param[in] vol: mic volume,[0, 100]
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ao_get_vol(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, INT32_T *vol)
{
    return codec_volume_get(VOLUME_TYPE_AUDIO, vol);
}

/**
* @brief audio input detect start
* 
* @param[in] card: card number
* @param[in] type: detect type
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_detect_start(INT32_T card, TKL_MEDIA_DETECT_TYPE_E type)
{
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief audio input detect stop
* 
* @param[in] card: card number
* @param[in] type: detect type
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_detect_stop(INT32_T card, TKL_MEDIA_DETECT_TYPE_E type)
{
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief audio detect get result
* 
* @param[in] card: card number
* @param[in] type: detect type
* @param[out] presult: audio detect result
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_ai_detect_get_result(INT32_T card, TKL_MEDIA_DETECT_TYPE_E type, TKL_AUDIO_DETECT_RESULT_T *presult)
{
    return OPRT_NOT_SUPPORTED;
}

// tuya接口 pcm音频流播放测试
#if 0
#include "tkl_thread.h"
// #include "tkl_audio_test.h"
#include "test_pcm.h"

static TKL_THREAD_HANDLE pcm_stream_play_thread = NULL;

void pcm_stream_play_test(void *arg)
{
    tkl_system_sleep(5000);
    TKL_AUDIO_CONFIG_T pconfig = {
        .spk_sample = TKL_AUDIO_SAMPLE_16K,
        .datebits = TKL_AUDIO_DATABITS_16,
        .sample = TKL_AUDIO_SAMPLE_16K,
        .codectype = TKL_CODEC_AUDIO_PCM,
        .spk_gpio = 21,
        .spk_gpio_polarity = false
    };
    int ret = tkl_ao_init(&pconfig, 1, NULL);
    if(ret) {
        return ;
    }

    tkl_ao_start(0, 0, NULL);
    TKL_AUDIO_FRAME_INFO_T pframe;
    // memset(&pframe, 0, sizeof(pframe));
    // pframe.used_size = sizeof(audio_test_pcm_buffer);
    // pframe.codectype = TKL_CODEC_AUDIO_PCM;
    // pframe.pbuf = audio_test_pcm_buffer;
    // tkl_ao_put_frame(0, 0, NULL, &pframe);

    int send_buffer_size = 0;
    while(send_buffer_size < sizeof(audio_test_pcm_buffer)) {
        memset(&pframe, 0, sizeof(pframe));
        pframe.used_size = sizeof(audio_test_pcm_buffer) - send_buffer_size > 4096 ? 4096 : sizeof(audio_test_pcm_buffer) - send_buffer_size;
        pframe.codectype = TKL_CODEC_AUDIO_PCM;
        pframe.pbuf = audio_test_pcm_buffer + send_buffer_size;
        tkl_ao_put_frame(0, 0, NULL, &pframe);
        send_buffer_size += pframe.used_size;
    }

    tkl_system_sleep(2000);
    tkl_ao_stop(0, 0, NULL);

    while(1)
        tkl_system_sleep(20000);

    return ;
}

void tkl_player_stream_play_test(void)
{
    tkl_thread_create(&pcm_stream_play_thread, "pcm_stream_play_thread", 10*1024, osPriorityNormal, pcm_stream_play_test, NULL);
}

#endif

//录音接口测试
#if 0
#include "tkl_thread.h"

static TKL_THREAD_HANDLE pcm_stream_record_thread = NULL;

#define MAX_RECORD_LEN (32 * 1024 * 10)
static uint32_t record_data_len = 0;
static uint8_t *record_buffer = NULL;
static TKL_SEM_HANDLE record_sem = NULL;

static int record_cb(TKL_AUDIO_FRAME_INFO_T *pframe)
{
    LOGI("record_cb %d %d",record_data_len, pframe->buf_size);
    int len = MAX_RECORD_LEN - record_data_len;
    len = len > pframe->buf_size ? pframe->buf_size : len;
    if(len) {
        memcpy(record_buffer+record_data_len, pframe->pbuf, len);
        record_data_len += pframe->buf_size;
        return 0;
    } else {
        tkl_semaphore_post(record_sem);
        return -1;
    }
}

void test_stream_record_task(void *arg)
{
    tkl_system_sleep(5000);
    record_buffer = tkl_system_malloc(MAX_RECORD_LEN);
    if(!record_buffer) {
        LOGE("tkl_system_malloc failed");
        return;
    }
    tkl_semaphore_create_init(&record_sem, 0, 1);
    TKL_AUDIO_CONFIG_T record_pconfig = {
        .sample = TKL_AUDIO_DATABITS_16,
        .codectype = TKL_CODEC_AUDIO_PCM,
        .put_cb = record_cb,
    };
    OPERATE_RET ret = tkl_ai_init(&record_pconfig, 1);
    if(ret) 
        return;
    ret = tkl_ai_start(0, 0);
    if(ret) 
        return;

    tkl_semaphore_wait(record_sem, -1);
    tkl_ai_stop(0, 0);
    tkl_system_sleep(50);

    TKL_AUDIO_CONFIG_T pconfig = {
        .spk_sample = 16000,
        .spk_gpio = 21,
        .spk_gpio_polarity = false
    };
    ret = tkl_ao_init(&pconfig, 1, NULL);
    if(ret) {
        return ;
    }

    tkl_ao_start(0, 0, NULL);
    TKL_AUDIO_FRAME_INFO_T pframe;
    pframe.used_size = record_data_len;
    pframe.pbuf = (CHAR_T *)record_buffer;
    pframe.datebits = TKL_AUDIO_DATABITS_16;
    pframe.sample = TKL_AUDIO_SAMPLE_16K;
    pframe.codectype = TKL_CODEC_AUDIO_PCM;
    tkl_ao_put_frame(0, 0, NULL, &pframe);

    LOGD("record data player over");
    while(1)
        tkl_system_sleep(20000);

    return ;
}

void tkl_audio_record_test(void)
{
    tkl_thread_create(&pcm_stream_record_thread, "pcm_stream_record_thread", 10*1024, osPriorityNormal, test_stream_record_task, NULL);
}
#endif
#endif
