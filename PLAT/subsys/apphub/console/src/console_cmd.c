/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718 
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include <ctype.h>
#include "reset.h"
#include "packet.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "time.h"
#include "storage.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "bsp.h"
#include "packet.h"

#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

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
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_PCM_ENABLE
#include "pcm.h"
#endif
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#endif

#ifdef FEATURE_SUBSYS_SYSINFO_ENABLE
#include "sysinfo.h"
#endif

extern int skip_atoi(const char **s);

#ifdef FEATURE_SUBSYS_FATFS_ENABLE
void ffInfoPrint(int argc, char **argv)
{
    INT32   errCode;
    CHAR printfBuf[256] = {0};
    CHAR *pcWriteBuffer = printfBuf;
    UINT32 fsFileCount = 0;
    DIR dir;
    FILINFO info;
//     lfs_status_t status[2] = {0};
    if(getConsoleDir()== DISK_E)
    {
        errCode = f_opendir(&dir, "0:");  
        if(errCode)
        {
            rt_kprintf("\n\r f_opendir open error");
        }
        rt_kprintf("\n\r file name\t size \r\n");
    }
    while(true)
    {
        errCode = f_readdir(&dir, &info);

        if(errCode == FR_OK && info.fname[0] == 0){
            break;
        }
        // rt_kprintf("\n\r clust:%d;sect:%d;fn:%s \r\n",dir.clust,dir.sect,dir.fn);
        rt_kprintf("\n\r %s  %d B", info.fname, info.fsize);
    }

    errCode = f_closedir(&dir);
    if(errCode){
    }
    
}
MSH_CMD_EXPORT_ALIAS(ffInfoPrint, fls, print file info);
#endif

#ifdef FEATURE_SUBSYS_FATFS_ENABLE
void ffCat(int argc, char **argv)
{
    uint32_t   size      = 0;
    uint32_t   read_size      = 0;
    char *fileName = 0;
    FIL catFile   = {0};
    FILINFO info;
    uint8_t    *buffer = NULL;

    fileName = argv[1];
    if(f_open(&catFile,fileName,FA_READ) == FR_OK)
    {
        f_stat(fileName,&info);
        size = info.fsize;
        rt_kprintf("\n\r fileName:%s\n\r size:%d \r\n",fileName,info.fsize);
        buffer = malloc(size+1);
        f_read(&catFile, buffer, size,&read_size);
        buffer[size]=0;
        rt_kprintf("%s",buffer);
        f_close(&catFile);
        free(buffer);
    }else{
        rt_kprintf("\n\r fileName:%s not exist !!",fileName);
    }
    return;
}
MSH_CMD_EXPORT_ALIAS(ffCat, fcat, cat fat);
#endif

#ifdef FEATURE_SUBSYS_FATFS_ENABLE
void ffCatx(int argc, char **argv)
{
    uint32_t   size      = 0;
    uint32_t   read_size      = 0;
    char *fileName = 0;
    FIL catFile   = {0};
    FILINFO info;
    uint8_t    *buffer = NULL;

    fileName = argv[1];
    if(f_open(&catFile,fileName,LFS_O_RDONLY) == LFS_ERR_OK)
    {
        f_stat(fileName,&info);
        size = info.fsize;
        rt_kprintf("\n\r fileName:%s\n\r size:%d \r\n",fileName,info.fsize);
        buffer = malloc(size);
        f_read(&catFile, buffer, size,&read_size);
        for(int i=0;i<size;i++)
            rt_kprintf("0x%02X ",buffer[i]);
        //rt_kprintf("%s",buffer);
        f_close(&catFile);
        free(buffer);
    }else{
        rt_kprintf("\n\r fileName:%s not exist !!\n\r",fileName);
    }

    return;
}
MSH_CMD_EXPORT_ALIAS(ffCatx, fcatx, cat fat);
#endif

bool checkPathPrefix(char *path)
{
    bool retVal = false;
    if (!((path[0] == 'c') || (path[0] == 'C') || (path[0] == 'd') || (path[0] == 'D') || (path[0] == 'e') || (path[0] == 'E')))
    {
        goto labelEnd;
    }
    if (!((path[1] == ':') && (path[2] == '/')))
    {
        goto labelEnd;
    }
    retVal = true;
labelEnd:
    return retVal;
}

int fileNameToDir(char *name, char *dir)
{
    FILE *file = NULL;
    char folder[80] = {0};
    if(checkPathPrefix(name)==false){
        if(getConsoleDir()== DISK_C)
            memcpy(folder,"C:/",strlen("C:/"));
        else if(getConsoleDir()== DISK_D)
            memcpy(folder,"D:/",strlen("D:/"));
        else if(getConsoleDir()== DISK_E)
            memcpy(folder,"E:/",strlen("E:/")); 
        strcat(folder, name);
        // rt_kprintf("\n\rcat folder:%s\n\r",folder);
    }
    else {
        memcpy(folder,name,strlen(name));
        // rt_kprintf("\n\rright folder:%s\n\r",folder);
    } 
    file = file_fopen(folder, "r+");
    if(file != NULL){
        file_fclose(file);
        memcpy(dir,folder,strlen(folder));
        // rt_kprintf("\n\rdir:%s\n\r",dir);
        return strlen(folder);
    }
    return 0;    
}

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void lfsCat(int argc, char **argv)
{
    uint32_t   size      = 0;
    char *fileName = 0;
    FILE *catFile   = NULL;
    struct stat buf = {0};
    uint8_t    *buffer = NULL;

    fileName = argv[1];
    catFile = file_fopen(fileName, "r");
    if(catFile != NULL)
    {
        file_fstat((int)catFile, &buf);
        size = buf.st_size;
        rt_kprintf("\n\r fileName:%s\n\r size:%d \r\n",fileName,size);
        buffer = malloc(size + 1);
        file_fread(buffer, size, 1, catFile);
        buffer[size] = '\0';
        rt_kprintf("%s",buffer);
        file_fclose(catFile);
        free(buffer);
    }else{
        rt_kprintf("\n\r fileName:%s not exist !!",fileName);
    }
    return;
}
MSH_CMD_EXPORT_ALIAS(lfsCat, cat, cat lfs);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void lfsCatx(int argc, char **argv)
{
    uint32_t   size      = 0;
    char *fileName = 0;
    FILE *catFile   = NULL;
    struct stat buf = {0};
    uint8_t    *buffer = NULL;

    fileName = argv[1];
    catFile = file_fopen(fileName, "r");
    if(catFile != NULL)
    {
        file_fstat((int)catFile, &buf);
        size = buf.st_size;
        rt_kprintf("\n\r fileName:%s\n\r size:%d \r\n",fileName,size);
        buffer = malloc(size);
        file_fread(buffer, size, 1, catFile);
        for(int i=0;i<size;i++)
            rt_kprintf("0x%02X ",buffer[i]);
        //rt_kprintf("%s",buffer);
        file_fclose(catFile);
        free(buffer);
    }else{
        rt_kprintf("\n\r fileName:%s not exist !!\n\r",fileName);
    }

    return;
}
MSH_CMD_EXPORT_ALIAS(lfsCatx, catx, cat lfs);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void lfsEcho(int argc, char **argv)
{
    char *fileName = 0;
    FILE *echo_file = NULL;
    fileName = argv[3];

    if(strcmp(argv[2],">>") == 0)
    {
        //append current file
        rt_kprintf("\n\r append current file\n\r");
        echo_file = file_fopen(fileName, "a+");
        if(echo_file != NULL)
        {
            rt_kprintf("\n\r fileName:%s\n\r",fileName);
            rt_kprintf("\n\r content : %s\n\r,length : %d \r\n",argv[1],strlen(argv[1]));
            file_fwrite(argv[1],strlen(argv[1]), 1, echo_file);
            file_fclose(echo_file);
        }
        else{
            rt_kprintf("\n\r fileName:%s open fail\n\r",fileName);
        }  
    }
    if(strcmp(argv[2],">") == 0)
    {
        //create new file
        rt_kprintf("\n\r create new file\n\r");
        echo_file = file_fopen(fileName, "w+");
        if(echo_file != NULL)
        {
            rt_kprintf("\n\r fileName:%s\n\r",fileName);
            rt_kprintf("\n\r content : %s\n\r,length : %d \r\n",argv[1],strlen(argv[1]));
            file_fwrite(argv[1],strlen(argv[1]), 1, echo_file);
            file_fclose(echo_file);
        }else{
            rt_kprintf("\n\r fileName:%s open fail",fileName);
        }
    }
    return;
}
MSH_CMD_EXPORT_ALIAS(lfsEcho, echo, echo lfs);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void lfsRm(int argc, char **argv)
{
    char *fileName = 0;
    fileName = argv[1];

    int ret = remove(fileName);
    if (ret == 0)
    {
        rt_kprintf("fileName:%s already delete\n\r", fileName);
    }
    else 
    {
        rt_kprintf("fileName:%s delete fail\n\r", fileName);
    }
    return;
}
MSH_CMD_EXPORT_ALIAS(lfsRm, rm, rm file in lfs);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
extern char *getConsoleDirPath();
void lfsInfoPrint(int argc, char **argv)
{
    INT32   errCode;
    CHAR printfBuf[256] = {0};
    CHAR *pcWriteBuffer = printfBuf;
    DIR *dir = NULL;
    struct lfs_info *info = NULL;
    // lfs_status_t status[2] = {0};
    uint8_t index = (getConsoleDir() == DISK_C) ? 0 : 1;
    // if(getConsoleDir()== DISK_C)
    //     dir = opendir("C:/");
    // else if(getConsoleDir()== DISK_D)
    //     dir = opendir("D:/");
    // else if(getConsoleDir()== DISK_E)
    //     dir = opendir("E:/");
    dir = opendir((const char *)getConsoleDirPath());
    // errCode = LFS_dirOpen(&dir, "/"); 
    if(dir == NULL)
    {
        rt_kprintf("\n\rprint_lfs_info open error");
        return;
    }
    rt_kprintf("\n\rfile name\tsize\r\n");
    while(true)
    {
        info = (struct lfs_info *)readdir(dir);
        if(info == NULL){
            break;
        }
        snprintf(pcWriteBuffer, 256, "\n\r%s\t%uB", info->name, info->size);
        rt_kprintf("%s",pcWriteBuffer);
    }
    if(pcWriteBuffer != printfBuf){
        
    }
    errCode = closedir(dir);
    if(errCode){
    }
    snprintf(printfBuf, 256, "Totoal:%dK; Free:%dK", getFsTotalSize(index) / 1024, getFsFreeSize(index) / 1024);
    rt_kprintf("\n\r%s\r\n",printfBuf);
}
MSH_CMD_EXPORT_ALIAS(lfsInfoPrint, ls, print file info);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
extern void setConsoleDirPath(char *path);
// cd c:/a/b/c
void lfsDir(int argc, char **argv)
{
    char *path = argv[1];
    char driver = tolower(path[0]);
    if (driver == 'c')
        setConsoleDir(DISK_C);
    else if (driver == 'd')
        setConsoleDir(DISK_D);
    else if (driver == 'e')
        setConsoleDir(DISK_E);
    setConsoleDirPath(path);
}
MSH_CMD_EXPORT_ALIAS(lfsDir, cd, change directory);
#endif

// mkdir c:/test/images
int lfsMkdir(int argc, char **argv)
{
    char *path = argv[1];
    // 复制原始路径到临时缓冲区
    char *temp = strdup(path);
    if (!temp)
    {
        printf("strdup fail\r\n");
        return -1;
    }

    // 分割路径为token数组
    char *tokens[20];
    int num_tokens = 0;
    char *token = strtok(temp, "/");
    while (token && num_tokens < 20)
    {
        tokens[num_tokens++] = token;
        token = strtok(NULL, "/");
    }

    // 逐步构建并输出路径
    if (num_tokens > 0)
    {
        char buffer[1024]; // 足够大的缓冲区
        strcpy(buffer, tokens[0]);
        for (int i = 1; i < num_tokens; ++i)
        {
            strcat(buffer, "/");
            strcat(buffer, tokens[i]);
            mkdir(buffer, 0777);
        }
    }
    free(temp);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(lfsMkdir, mkdir, make directory in lfs);

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void consoleHalReboot(int argc, char **argv)
{
    ResetECSystemReset();
}
MSH_CMD_EXPORT_ALIAS(consoleHalReboot, reboot, reboot);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
int consoleTasksInfo(int argc, char **argv)
{
    char buffer[1024];
    vTaskGetRunTimeStats_ec(buffer);
    rt_kprintf("\n\rtask\t\tcount\t\trate\r\n");
    rt_kprintf(buffer);
    return 0;
}
MSH_CMD_EXPORT_ALIAS(consoleTasksInfo, ps, tasks status);
#endif

uint8_t gConsoleShareMem[sizeof(FilePacketDataT)]={0};
int gConsoleShadowMemReadLen=0;
int gConsoleShareMemFlag=0;

void needShadowMem(int size)
{
    gConsoleShadowMemReadLen = size;
    gConsoleShareMemFlag = 0;
}

int waitShadowMem()
{
   return gConsoleShareMemFlag;
}

void writeShadowMem(uint8_t *buf)
{
    memcpy(gConsoleShareMem,buf,gConsoleShadowMemReadLen);
    gConsoleShareMemFlag = 1;
}

void read_shadow_mem(uint8_t *buf)
{
    memcpy(buf,gConsoleShareMem,gConsoleShadowMemReadLen);
    gConsoleShadowMemReadLen =0;
    gConsoleShareMemFlag = 0;
}

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
extern void eConsoleSendData(uint8_t *data,int len);
void lfs_send(int argc, char **argv)
{
    char *fileName = 0;
    FILE *send_file   = NULL;
    struct stat buf = {0};
    uint32_t   size      = 0;
    int pakCount = 0;
    int pakLeave = 0;
    uint8_t pdHeader[sizeof(FilePacketHdrT)]={0};
    FilePacketHdrT *pfHdr = (FilePacketHdrT *)pdHeader;

    uint8_t pdData[sizeof(FilePacketDataT)]={0};
    FilePacketDataT *pPdData = (FilePacketDataT *)pdData;

    uint8_t pkAck[sizeof(PacketAckT)]={0};
    // PacketAckT *ack=pkAck;
    // int x=0;

    uint8_t *buffer = NULL;
    // uint8_t *temp = NULL;

    fileName = argv[1];

    send_file = file_fopen(fileName, "r");
    if(send_file != NULL)
    {
        file_fstat((int)send_file, &buf);
        size = buf.st_size;
        // rt_kprintf("\n\r fileName:%s\n\r size:%d \r\n",fileName,size);
        pakCount = size/MAX_PAK_DATA_SIZE;
        pakLeave = size%MAX_PAK_DATA_SIZE;

        // send header
        pfHdr->fileSize = size;
        memcpy(pfHdr->fileName,fileName,strlen(fileName));
        pfHdr->packetSize = sizeof(FilePacketDataT);
        pfHdr->fileCounter = pakCount;

        eConsoleSendData(pdHeader,sizeof(FilePacketHdrT));
        //uart_send();
        //uart_get();
        // ec_console_get_data(ack,sizeof(PacketAckT));
        needShadowMem(8);
        for(;;)
        {
            osDelay(5);
            if(waitShadowMem() ==1)
                break;
        }
        read_shadow_mem(pkAck);

        osDelay(5);

        //send data
        buffer = malloc(MAX_PAK_SIZE);
        for(int i=0;i<=pakCount;i++)
        {
            
            memset(pdData,0,sizeof(FilePacketDataT));
            if(i==pakCount)
            {
                file_fread(buffer, pakLeave, 1, send_file);
                pPdData->packetCounter = i;
                extern uint32_t packageEncode(uint8_t * scr, uint8_t * buf, uint32_t lens);
                packageEncode(buffer,pPdData->packetData,pakLeave);
                eConsoleSendData(pdData,sizeof(FilePacketDataT));
                // for(x=0;x<sizeof(pPdData);x++)
                //     pikaPlatformPutChar(temp[x]);
                 //uart_send();
                needShadowMem(8);
                for(;;)
                {
                    osDelay(5);
                    if(waitShadowMem() ==1)
                        break;
                }
                read_shadow_mem(pkAck);
            }
            else
            {
                file_fread(buffer, MAX_PAK_DATA_SIZE, 1, send_file);
                pPdData->packetCounter = i;
                packageEncode(buffer,pPdData->packetData,MAX_PAK_DATA_SIZE);
                eConsoleSendData(pdData,sizeof(FilePacketDataT));
                needShadowMem(8);
                for(;;)
                {
                    osDelay(5);
                    if(waitShadowMem() ==1)
                        break;
                }
                read_shadow_mem(pkAck);
            }
            //uart_get();
        }
        //rt_kprintf("%s",buffer);
        file_fclose(send_file);
        free(buffer);
    }else{
        rt_kprintf("\n\r fileName:%s not exist !!\n\r",fileName);
    }

    // rt_kprintf("\n\r fileName:%s send already\n\r",fileName);
    return;
}


MSH_CMD_EXPORT_ALIAS(lfs_send, send, send file in lfs);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void lfsGet(int argc, char **argv)
{
    char *fileName = 0;
    // uint32_t fileSize = 0;
    int pakCount = 0;
    int pakLeave = 0;
    uint32_t packetSize = 0;
    FILE *getFile   = NULL;

    uint8_t pkAck[sizeof(PacketAckT)]={0};
    PacketAckT *ack=(PacketAckT *)pkAck;

    uint8_t pdHeader[sizeof(FilePacketHdrT)]={0};
    FilePacketHdrT *pfHdr = (FilePacketHdrT *)pdHeader;

    uint8_t pdData[sizeof(FilePacketDataT)]={0};
    FilePacketDataT *pPdData = (FilePacketDataT *)pdData;
    uint8_t *pRealFileData = 0;

    fileName = argv[1];
    
    //start ack
    ack->packetCounter = 0;
    ack->packetResult = 1;
    eConsoleSendData(pkAck,sizeof(PacketAckT));

    //get header
    needShadowMem(sizeof(FilePacketHdrT));
    for(;;)
    {
        osDelay(5);
        if(waitShadowMem() ==1)
            break;
    }
    read_shadow_mem(pdHeader);
    osDelay(5);
    
    packetSize = pfHdr->packetSize-sizeof(PacketHdrT)-4;    //-4 mean uint32_t packetCounter;
    pakCount = pfHdr->fileCounter;
    pakLeave = pfHdr->fileSize % packetSize;

    // ack
    ack->packetCounter = 0;
    ack->packetResult = 1;
    eConsoleSendData(pkAck,sizeof(PacketAckT));

    getFile = file_fopen(fileName, "w+");
    if(getFile != NULL)
    {
        for(int i=0;i<=pakCount;i++)
        {
            // get file data
            needShadowMem(sizeof(FilePacketDataT));
            for(;;)
            {
                osDelay(5);
                if(waitShadowMem() ==1)
                    break;
            }
            read_shadow_mem(pdData);
            osDelay(5);
            pRealFileData = pPdData->packetData+sizeof(PacketHdrT);
            if( i < pakCount)
            {   
                file_fwrite(pRealFileData,packetSize, 1, getFile);

                
            }else{
                file_fwrite(pRealFileData,pakLeave, 1, getFile);
                osDelay(1);     
            }

            //data ack
            ack->packetCounter = 0;
            ack->packetResult = 1;
            eConsoleSendData(pkAck,sizeof(PacketAckT));
        }

    }else{
        rt_kprintf("\n\r fileName:%s open fail\n\r",fileName);
    }

    file_fclose(getFile);
    // rt_kprintf("\n\r fileName:%s already delete\n\r",fileName);
    return;
}

MSH_CMD_EXPORT_ALIAS(lfsGet, get, get file from console);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
void debugSwitch(int argc, char **argv)
{
#ifdef FEATURE_SUBSYS_MODE_ENABLE
    if(debugTypeGet() >= 3)
    {
    #ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
        cmdparseInit();
        #ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
            systestInit();
        #endif
        debugSet(DEBUG_JSON);
    #endif
    }
#endif
}
MSH_CMD_EXPORT_ALIAS(debugSwitch, dbgswi, get file from console);
#endif

#ifdef VOLTE_DEMO
extern int32_t setPhoneNumber(char *number);
void updatePhoneNumber(int argc, char **argv)
{
    setPhoneNumber(argv[1]);
}
MSH_CMD_EXPORT_ALIAS(updatePhoneNumber, phoneNumber, Update phone number of volte_demo);
#endif

#ifdef FEATURE_SUBSYS_G711_ENABLE
#include "g711.h"
void g711Test(int argc, char **argv)
{
    char pathG711[128] = {0};
    char pathWav[128]  = {0};

    memset(pathG711, 0, sizeof(pathG711));
    snprintf(pathG711, sizeof(pathG711), "%s.g711", argv[1]);
    g711Encode(argv[1], pathG711);

    memset(pathWav, 0, sizeof(pathWav));
    snprintf(pathWav, sizeof(pathWav), "%s.wav", pathG711);
    g711Decode(pathG711, pathWav);

}
MSH_CMD_EXPORT_ALIAS(g711Test, g711Test, WAV to G711 and G711 to WAV);
#endif

#ifdef FEATURE_SUBSYS_SYSINFO_ENABLE
// get_sys_info 0
void get_sys_info(int argc, char **argv)
{
    char info[50];
    getSysinfo(skip_atoi((const char **)&argv[1]), info);
    printf("%s\r\n", info);
}
MSH_CMD_EXPORT_ALIAS(get_sys_info, get_sys_info, get sys info);
#endif

#endif