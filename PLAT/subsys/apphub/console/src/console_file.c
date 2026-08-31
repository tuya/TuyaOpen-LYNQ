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
#include "console.h"
#include "bsp.h"
#include "console_hal.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
// static osMessageQueueId_t gRunPythonQueue = NULL;
// static TaskHandle_t gConsoleFileHdl = NULL;
// static char *gPythonFile = NULL;

static uint8_t gConsoleDir = 0;

char gConsoleDirPath[50] = "C:/";

uint8_t getConsoleDir(void)
{
    return gConsoleDir;
}
void setConsoleDir(uint8_t disk)
{
    gConsoleDir = disk;
}

void setConsoleDirPath(char *path)
{
    strcpy(gConsoleDirPath, path);
}

char *getConsoleDirPath()
{
    return gConsoleDirPath;
}

int consoleFileWrite( char *path, char *str,int length)
{
    FILE *file = NULL;
    if (str != NULL){       
        file = file_fopen(path, "w");
        if (file != NULL){
            file_fwrite(str, length, 1, file);
            file_fclose(file);
            return length;
        }
    }
    return 0;
}
int consoleFileRead(char *path, char *str,int length)
{
    FILE *file = NULL;
    if (str != NULL){       
        file = file_fopen(path, "r");
        if (file == NULL){
            // printf("Failed to open the file \"%s\".\r\n", path);
            return 0;
        }
        file_fread(str, CONSOLE_FILE_SIZE, 1, file);
        file_fclose(file);
        return length;
    }
    return 0;
}
 
int consoleFileFilter(char *str,char *name)
{
    char  *head   = NULL;
    char  *tail   = NULL;
    int  length     = 0;
    int  len     = 0;
    if (str != NULL){       
        // rt_kprintf("\n\rfile[%d]:%s",strlen(str),str);    
        head = strstr(str, FILE_MARK);
        if (head == NULL){
            return 0;
        }
        // rt_kprintf("\n\rlen1 %d,file:%s",length,head);
        tail = strstr(head, TAIL_MARK);
        if (tail == NULL){
            return 0;
        }
        length = tail - head + strlen(TAIL_MARK) +1;
        head = strstr(head, "###");
        tail = strstr(head, ".py");
        len = tail - head;
        if (len > 3){
            memcpy(name, head+3, len);
            // rt_kprintf("\n\rname[%d]:%s,%s",len,name,head);
        }
        return length;
    }
    return 0;
}

#if 0
int sendConsoleFile(char *src, uint32_t timeout)
{
    int length = 0;
    if (src != NULL){
        char fileName[80]={0};
        if(length = consoleFileFilter(src,fileName)){
            rt_kprintf("file recv:%s\n\r%s",fileName,src);
            consoleFileWrite(fileName, src, length);
            // char *test_buffer = malloc(CONSOLE_FILE_SIZE);
            // consoleFileRead(cmd_buffer, test_buffer, CONSOLE_FILE_SIZE);
            // rt_kprintf("\n\rread check%d,%s",strlen(test_buffer),test_buffer);
            // free(test_buffer);
            // memset(cmd_buffer, 0, CMD_RX_SIZE_MAX);
        }else {
            rt_kprintf("file recv:%s\n\r",src);
        }
    }

    return 0;
}
#endif

#endif