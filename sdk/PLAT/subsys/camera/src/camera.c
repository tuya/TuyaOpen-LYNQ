/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    
 * Description:  
 * History:      Rev1.0   
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CAMERA_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "camera.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include DEBUG_LOG_HEADER_FILE

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_cam.h"
#endif

#include "cameraDrv.h"
#include "lcdDrv.h"
#include "lspi.h"
#include "slpman.h"
#include "sctdef.h"

typedef struct
{
    uint32_t buffer;
    uint32_t value;
} QueueCameraT;

#define QUEUE_SIZE_CAMERA             100

PLAT_FPSRAM_ZI_CUST __ALIGNED(8) uint8_t  photo_buf[320*240*2];
PLAT_FPSRAM_ZI_CUST __ALIGNED(8) CameraBuf_t cam_fb[3];
static volatile bool      isTransferDone = false;
static osMessageQueueId_t gCameraQueue   = NULL;
static osMutexId_t        gCameraMutex   = NULL;
static bool               gNeedStop      = false;
static camera_status_t    gCameraStatus = CAMERA_STATUS_IDLE;

#define CAM_DMA_FINISHED_TRIGGER_FLAG    OPEN_CAM_IOCTL_MAX+(0x1)
#define CAM_FRAME_START_TRIGGER_FLAG     OPEN_CAM_IOCTL_MAX+(0x2)
#define CAM_FRAME_SAVE_TRIGGER_FLAG     OPEN_CAM_IOCTL_MAX+(0x3)

int cam_shoot_flag = 0;
int cam_data_rdy = 0;

// static void camUspUsrCb(uint32_t cspiStatus)    //frame end interrupt
// {
//     if(cspiStatus & ICL_STATS_FRAME_END_Msk)
//     {
//         ECPLAT_PRINTF(UNILOG_OPEN_HAL,CAMIRQ1, P_INFO, "frame end :0x%x",camGetCspiStats(CSPI_1));
//         cspiEndIntEnable(CSPI_INT_DISABLE);
//         camRecv(cam_fb);
//         QueueCameraT queue = {0};
//         queue.buffer = CAM_FRAME_START_TRIGGER_FLAG;
//         osMessageQueuePut(gCameraQueue, &queue, 0, 0);
//     }
// }

static void camRecvCb(uint32_t event)   //dma recv callback
{
    if(event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL,CAMREC1, P_INFO, "dma transfer complete:0x%x",event);
        isTransferDone = true;
        QueueCameraT queue = {0};
        queue.buffer = CAM_DMA_FINISHED_TRIGGER_FLAG;
        osMessageQueuePut(gCameraQueue, &queue, 0, 0);
        // if (cam_shoot_flag ==1 )
        // {
        //     // close cspi to lspi
        //     cspi2LspiEnable(0);

        //     // lspi rst and clear fifo
        //     lspiRstAndClearFifo();

        //     ECPLAT_PRINTF(UNILOG_OPEN_HAL,CAMREC2, P_INFO, "dma transfer complete,:0x%x",gNeedStop);
        //     QueueCameraT queue = {0};
        //     queue.buffer = CAM_FRAME_SAVE_TRIGGER_FLAG;
        //     osMessageQueuePut(gCameraQueue, &queue, 0, 0);
        // }        
    }
}

static void sCameraInit(void)
{
    gCameraQueue = osMessageQueueNew(QUEUE_SIZE_CAMERA, sizeof(uint32_t), NULL);
    if (gCameraQueue == NULL)
    {
        SYSLOG_ERR("Failed to create queue for gCameraQueue.\r\n");
        return;
    }

    gCameraMutex = osMutexNew(NULL);
    if (gCameraMutex == NULL)
    {
        SYSLOG_ERR("Failed to create mutex for gCameraMutex.\r\n");
        return;
    }

    GPR_swReset(RST_FCLK_USP1);
    // slpManNormalIOVoltSet(IOVOLT_3_00V);
    camInit(cam_fb, NULL, camRecvCb,NULL);
    // camRegisterIRQ(CSPI_1, camIrg);
    // camRegCfg();
    // camStartStop(CSPI_START);
    
    // camStartStop(CSPI_STOP);
}

static void sCameraStart(void)
{
    isTransferDone = false;
    GPR_swReset(RST_FCLK_USP1);
    lspiRstAndClearFifo();

extern lcdDrvFunc_t* lcdDev;

	lcdIoCtrl_t lcdIo;
	lcdIo.previewModeSel = CAM_PREVIEW_SET_AUTO;
	lcdIoCtrl(lcdDev, lcdIo);
	
    uint8_t regAddr = 0x36;
	uint8_t regData = 0xa0;
	lcdConfigReg(lcdDev, regAddr, &regData, 1);

    lcdSetWindow(lcdDev, 0, 0, 320-1, 240-1);

    camPreview(lcdDev, startPreview);

    cspi2LspiEnable(1);

    camGpioPulseCfg(20, 0, 5);
    camGpioPulse(0, 5, 1000, 0, false);  

    // memset(cam_fb, 0, sizeof(cam_fb));
    camRecv(cam_fb[0].data);
    camRegCfg();
    camStartStop(CSPI_START);
}

static void sCameraStop(void)
{
    extern lcdDrvFunc_t* lcdDev;
    camPreview(lcdDev, stopPreview);
    camStartStop(CSPI_STOP);
    // cspi2LspiEnable(0);
    // lspiRstAndClearFifo();
}

void subCameraTask(void *arg)
{
    uint32_t     timeout = 0;
    uint32_t     stats   = 0;
    QueueCameraT queue   = {0};

#ifdef USE_DECODE_LIB
    Initial_Decoder();
#endif
    sCameraInit();
    
    gCameraStatus = CAMERA_STATUS_IDLE;

    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gCameraQueue, &queue, 0, osWaitForever) == osOK)
        {   
            ECPLAT_PRINTF(UNILOG_OPEN_HAL,subCameraTask1, P_INFO, "queue.buffer:0x%x",queue.buffer);
            switch (queue.buffer)  
            {
            case OPEN_CAM_IOCTL_START:
                SYSLOG_INFO("camera start\r\n");
                osMutexAcquire(gCameraMutex, osWaitForever);
                sCameraStart(); 
                gCameraStatus = CAMERA_STATUS_STARTED;
                osMutexRelease(gCameraMutex);
                // osDelay(200);
                break;
            case OPEN_CAM_IOCTL_STOP:
                SYSLOG_INFO("camera stop\r\n");
                osMutexAcquire(gCameraMutex, osWaitForever);
                sCameraStop();
                gCameraStatus = CAMERA_STATUS_STOPPED;
                osMutexRelease(gCameraMutex);
                // osDelay(200);
                break;
            case OPEN_CAM_IOCTL_GET_MODE:
                // Handle get mode command
                break;
            case OPEN_CAM_IOCTL_GET_BUSY:
                // Handle get busy command
                break;
            case OPEN_CAM_IOCTL_SET_RESOLUTION:
                // Handle set resolution command
                break;
            case OPEN_CAM_IOCTL_GET_RESOLUTION:
                // Handle get resolution command
                break;
            case OPEN_CAM_IOCTL_SET_FRAMERATE:
                // Handle set framerate command
                break;
            case OPEN_CAM_IOCTL_GET_FRAMERATE:
                // Handle get framerate command
                break;
            case OPEN_CAM_IOCTL_SET_BRIGHTNESS:
                // Handle set brightness command
                break;
            case OPEN_CAM_IOCTL_SET_CONTRAST:
                // Handle set contrast command
                break;
            case OPEN_CAM_IOCTL_SET_SATURATION:
                // Handle set saturation command
                break;
            case OPEN_CAM_IOCTL_SET_SHARPNESS:
                // Handle set sharpness command
                break;
            case OPEN_CAM_IOCTL_SET_WB:
                // Handle set white balance command
                break;
            case OPEN_CAM_IOCTL_SET_EXPOSURE:
                // Handle set exposure command
                break;
            case OPEN_CAM_IOCTL_SET_AE:
                // Handle set ae command
                break;
            case OPEN_CAM_IOCTL_SET_AWB:
                // Handle set awb command
                break;
            case OPEN_CAM_IOCTL_SET_MIRROR:
                // Handle set mirror command
                break;
            case OPEN_CAM_IOCTL_GET_MIRROR:
                // Handle get mirror command
                break;
            case OPEN_CAM_IOCTL_SET_FLIP:
                // Handle set flip command
                break;
            case OPEN_CAM_IOCTL_GET_FLIP:
                // Handle get flip command
                break;
            case OPEN_CAM_IOCTL_SET_ROTATE:
                // Handle set rotate command
                break;
            case OPEN_CAM_IOCTL_GET_ROTATE:
                // Handle get rotate command
                break;
            case OPEN_CAM_IOCTL_SET_COLOR_MODE:
                // Handle set color mode command
                break;
            case OPEN_CAM_IOCTL_GET_COLOR_MODE:
                // Handle get color mode command
                break;
            case OPEN_CAM_IOCTL_START_PREVIEW:
                SYSLOG_INFO("camera start preview\n");
                osMutexAcquire(gCameraMutex, osWaitForever);
                sCameraStart(); 
                gCameraStatus = CAMERA_STATUS_STARTED;
                osMutexRelease(gCameraMutex);
                break;
            case OPEN_CAM_IOCTL_STOP_PREVIEW:
                SYSLOG_INFO("camera stop preview\n");
                osMutexAcquire(gCameraMutex, osWaitForever);
                sCameraStop();
                gCameraStatus = CAMERA_STATUS_STOPPED;
                osMutexRelease(gCameraMutex);
                break;
            case OPEN_CAM_IOCTL_REC_START:
                // Handle start rec command
                break;
            case OPEN_CAM_IOCTL_REC_STOP:
                // Handle stop rec command
                break;
            case OPEN_CAM_IOCTL_REC_FILE_FORMAT:
                // Handle rec file format command
                break;
            case OPEN_CAM_IOCTL_SHOOT_START:
                // sCameraStop();
                SYSLOG_INFO("OPEN_CAM_IOCTL_SHOOT_START\n");
                SYSLOG_INFO("photo_buf addr:0x%x\r\n",photo_buf);
                cam_data_rdy = 0;
                // cam_shoot_flag = 1;
                // cspiEndIntEnable(CSPI_INT_ENABLE);
                // Handle start shoot command
                int posIndex = camPicTake();
                if(posIndex >= 0)
                {
                    memcpy(photo_buf,cam_fb[posIndex].data,sizeof(cam_fb[posIndex].data));
                    camPicGive(posIndex);
                    cam_data_rdy =2;
                }
                break;
            case OPEN_CAM_IOCTL_SHOOT_STOP:
                // Handle stop shoot command
                break;
            case OPEN_CAM_IOCTL_SHOOT_FILE_FORMAT:
                // Handle shoot file format command
                break;           
            case CAM_DMA_FINISHED_TRIGGER_FLAG:
                // Handle shoot file format command
                // SYSLOG_INFO("CAM_DMA_FINISHED_TRIGGER_FLAG\r\n");
                // cam_data_rdy = 1;
                // cspiEndIntEnable(CSPI_INT_ENABLE);
                break;  
            case CAM_FRAME_SAVE_TRIGGER_FLAG:
                SYSLOG_INFO("CAM_FRAME_SAVE_TRIGGER_FLAG\r\n");
                // Handle shoot file format command
                if(cam_data_rdy && cam_shoot_flag ==1)
                {
                    memcpy(photo_buf, cam_fb, sizeof(cam_fb));
                    printf("photo_buf copy over\r\n");
                    cam_shoot_flag = 0;
                    cam_data_rdy = 2;
                }
                break;   
            case CAM_FRAME_START_TRIGGER_FLAG:
                SYSLOG_INFO("CAM_FRAME_START_TRIGGER_FLAG\r\n");
                break;   
            default:
                break;
            }
        }
    }
}

void subCameraInit()
{
    osThreadAttr_t taskAttr;

    memset(&taskAttr,0,sizeof(taskAttr));
    memset(subsys_camera_task_stack, 0xA5,SUBSYS_CAMERA_TASK_STACK_SIZE);
    taskAttr.name = "camera";
    taskAttr.stack_mem = subsys_camera_task_stack;
    taskAttr.stack_size = SUBSYS_CAMERA_TASK_STACK_SIZE;
    taskAttr.priority = osPriorityNormal;
    taskAttr.cb_mem = &subsys_camera_task;//task control block
    taskAttr.cb_size = sizeof(StaticTask_t);//size of task control block

    osThreadNew(subCameraTask, NULL, &taskAttr);
}

void cameraStart(void)
{
    QueueCameraT queue = {0};
    if(gCameraStatus == CAMERA_STATUS_IDLE||gCameraStatus == CAMERA_STATUS_STOPPED)
    {
        queue.buffer = OPEN_CAM_IOCTL_START;
        gNeedStop = false;
        gCameraStatus = CAMERA_STATUS_STARTING;
        osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
    }
}

void cameraStop(void)
{
    QueueCameraT queue = {0};

    if(gCameraStatus == CAMERA_STATUS_STARTED)
    {
        queue.buffer = OPEN_CAM_IOCTL_STOP;
        cam_data_rdy = 0;
        gNeedStop = true;
        gCameraStatus = CAMERA_STATUS_STOPPING;
        osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
    }
}

void cameraStartPreview(void *callback_func)
{
    QueueCameraT queue = {0};
    if(gCameraStatus == CAMERA_STATUS_IDLE||gCameraStatus == CAMERA_STATUS_STOPPED)
    {
        queue.buffer = OPEN_CAM_IOCTL_START;
        gNeedStop = false;
        gCameraStatus = CAMERA_STATUS_STARTING;
        osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
    }
}

void cameraStopPreview(void)
{
    QueueCameraT queue = {0};
    if(gCameraStatus == CAMERA_STATUS_STARTED)
    {
        queue.buffer = OPEN_CAM_IOCTL_STOP;
        gNeedStop = true;
        gCameraStatus = CAMERA_STATUS_STOPPING;
        osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
    }
}

void cameraGetFrame(void *callback_func)
{

}

camera_status_t cameraGetStatus(void)
{
    return gCameraStatus;
}

void cameraGetSettings(void)
{

}

void cameraSetSettings(void)
{

}

void cameraGetHWInfo(void)
{

}

void cameraStartRecord(void *callback_func)
{
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_REC_START;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraStopRecord(void)
{
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_REC_STOP;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraStartShoot(void *callback_func)
{
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SHOOT_START;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraStopShoot(void)  //unused
{
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SHOOT_STOP;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraQRDecode(void *callback_func)
{

}

void cameraSetMirror(uint32_t mirror) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_MIRROR;
    queue.value = mirror;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraSetRotation(uint32_t rotation) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_ROTATE;
    queue.value = rotation;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraSetFlip(uint32_t flip) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_FLIP;
    queue.value = flip;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraSetContrast(uint32_t contrast) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_CONTRAST;
    queue.value = contrast;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraSetSaturation(uint32_t saturation) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_SATURATION;
    queue.value = saturation;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraSetSharpness(uint32_t sharpness) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_SHARPNESS;
    queue.value = sharpness;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}

void cameraSetWhiteBalance(uint32_t whiteBalance) {
    QueueCameraT queue = {0};
    queue.buffer = OPEN_CAM_IOCTL_SET_WB;
    queue.value = whiteBalance;
    osMessageQueuePut(gCameraQueue, &queue, 0, osWaitForever);
}





#endif
