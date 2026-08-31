#ifdef USE_DECODE_LIB
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "hal_cam.h"
#include "camera.h"

#define THREAD_STACK_SIZE_SCAN_DECODE               (300 * 1024)
#define QUEUE_SIZE_SCAN_DECODE_REQUEST              5
#define QUEUE_SIZE_SCAN_DECODE_RESPONSE             5


typedef enum
{
    SCAN_REQUEST_START = 0,
    SCAN_REQUEST_STOP,
    SCAN_REQUEST_DECODE,
} ScanDecodeRequestT;

typedef struct
{
    uint32_t command;
} QueueScanDecodeRequestT;

typedef struct
{
    int32_t  result;
    uint8_t *buffer;
    uint32_t length;
} QueueScanDecodeResponseT;


static osThreadId_t       gScanDecodeThread        = NULL;
static osMessageQueueId_t gScanDecodeQueueRequest  = NULL;
static osMessageQueueId_t gScanDecodeQueueResponse = NULL;


extern int GetDecoderResult(unsigned char *result);
extern int Decoding_Image(unsigned char *img_buffer, int width, int height);


int32_t scanAndDecode(uint8_t *buffer, uint32_t *size, uint32_t timeout)
{
    int32_t                   retVal        = -1;
    QueueScanDecodeRequestT  queueRequest  = {.command = SCAN_REQUEST_DECODE};
    QueueScanDecodeResponseT queueResponse = {0};

    if ((buffer == NULL) || (size == NULL) || (*size <= 0))
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, scanDecodeStart5, P_DEBUG, "Param error.");
        goto labelEnd;
    }

    retVal = osMessageQueuePut(gScanDecodeQueueRequest, &queueRequest, 0, timeout);
    if (retVal != osOK)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, scanDecodeStart10, P_DEBUG, "retVal=%d, %p", retVal,gScanDecodeQueueRequest);
        goto labelEnd;
    }

    retVal = osMessageQueueGet(gScanDecodeQueueResponse, &queueResponse, 0, timeout);
    if ((retVal != osOK) || (queueResponse.result != 0))
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, scanDecodeStart15, P_DEBUG, "retVal=%d, result=%d", retVal, queueResponse.result);
        retVal = -1;
        goto labelEnd;
    }

    if (queueResponse.length > *size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, scanDecodeStart20, P_DEBUG, "size is too small: size=%d, length=%d", size, queueResponse.length);
        retVal = -1;
        goto labelEnd;
    }

    memcpy(buffer, queueResponse.buffer, queueResponse.length);
    *size = queueResponse.length;

labelEnd:
    return retVal;
}

static void threadScanDecode(void *argument)
{
    int32_t                  resVal          = -1;
    QueueScanDecodeRequestT  queueRequest    = {0};
    QueueScanDecodeResponseT queueResponse   = {0};
    uint8_t                  decodeData[256] = {0};
    CamImg_t img = {0};
    int ret = 0;
    if (gScanDecodeQueueRequest == NULL)
    {
        gScanDecodeQueueRequest = osMessageQueueNew(QUEUE_SIZE_SCAN_DECODE_REQUEST, sizeof(queueRequest), NULL);
        if (gScanDecodeQueueRequest == NULL)
        {
            SYSLOG_EMERG("Failed to create queue for gScanDecodeQueueRequest.\r\n");
            goto labelEnd;
        }
    }
    if (gScanDecodeQueueResponse == NULL)
    {
        gScanDecodeQueueResponse = osMessageQueueNew(QUEUE_SIZE_SCAN_DECODE_RESPONSE, sizeof(queueResponse), NULL);
        if (gScanDecodeQueueResponse == NULL)
        {
            SYSLOG_EMERG("Failed to create queue for gScanDecodeQueueResponse.\r\n");
            goto labelEnd;
        }
    }

    while (1)
    {
        resVal = -1;
        memset(&queueRequest,  0, sizeof(queueRequest));
        memset(&queueResponse, 0, sizeof(queueResponse));
        if (osMessageQueueGet(gScanDecodeQueueRequest, &queueRequest, 0, osWaitForever) == osOK)
        {
            SYSLOG_DEBUG("command=%d\r\n", queueRequest.command);
            switch (queueRequest.command)
            {
                case SCAN_REQUEST_DECODE:
                    ret = cameraGetBuf(&img, 100);
                    if(ret != 0)
                    {
                        SYSLOG_DEBUG("get buffer failed\r\n");
                        break;
                    }
                    resVal = Decoding_Image(img.addr, img.width, img.height);
                    if (resVal > 0)
                    {
                        memset(decodeData, 0, sizeof(decodeData));
                        GetDecoderResult(decodeData);
                        SYSLOG_DEBUG("Decode success: %s\r\n", decodeData);
                    }
                    else
                    {
                        SYSLOG_DEBUG("Decode fail: resVal=%d\r\n", resVal);
                    }
                    cameraReleaseBuf(&img);
                    queueResponse.result = (resVal > 0) ? 0 : -1;
                    queueResponse.length = resVal;
                    queueResponse.buffer = decodeData;
                    resVal = osMessageQueuePut(gScanDecodeQueueResponse, &queueResponse, 0, osWaitForever);
                    if (resVal != osOK)
                    {
                        SYSLOG_DEBUG("Failed to send response: resVal=%d\r\n", resVal);
                    }
                    break;

                default:
                    SYSLOG_DEBUG("Unknown command: command=%d\r\n", queueRequest.command);
                    break;
            }
        }
    }

labelEnd:
    osThreadExit();
}

int32_t scanDecodeInit(void)
{
    osThreadAttr_t threadAttr = {0};
    if (gScanDecodeThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadScanDecode";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_SCAN_DECODE;
        gScanDecodeThread = osThreadNew(threadScanDecode, NULL, &threadAttr);
        if (gScanDecodeThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for scanDecode.\r\n");
        }
    }

    return ((gScanDecodeThread != NULL) ? 0 : -1);
}
#endif
