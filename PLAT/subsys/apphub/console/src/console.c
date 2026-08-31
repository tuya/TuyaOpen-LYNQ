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
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"

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

#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

#include "console.h"
#include "console_hal.h"
#include "console_ex.h"
#include "console_cmd.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
static TaskHandle_t gConsoleMshHdl = NULL;
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
static TaskHandle_t gConsolePikaHdl = NULL;
#endif

#ifdef FEATURE_SUBSYS_MICROPYTHON_ENABLE
#include "parse.h"
#endif
#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "msh.h"
#endif
#ifdef FEATURE_SUBSYS_MIPYTHON_ENABLE
#include "pyexec.h"
extern int py_init();
extern int py_deinit();
extern void mpy_do_str(const char *src, mp_parse_input_kind_t input_kind);
#endif

EventGroupHandle_t gConsoleEventGroup = NULL;


const char COLOR_RESET[] = "\033[0m";
const char COLOR_BLACK[] = "\033[30m"; /* Black */
const char COLOR_RED[] = "\033[31m"; /* Red */
const char COLOR_GREEN[] = "\033[32m"; /* Green */
const char COLOR_YELLOW[] = "\033[33m"; /* Yellow */
const char COLOR_BLUE[] = "\033[34m"; /* Blue */
const char COLOR_MAGENTA[] = "\033[35m"; /* Magenta */
const char COLOR_CYAN[] = "\033[36m"; /* Cyan */
const char COLOR_WHITE[] = "\033[37m"; /* White */
const char COLOR_BOLDBLACK[] = "\033[1m\033[30m"; /* Bold Black */
const char COLOR_BOLDRED[] = "\033[1m\033[31m"; /* Bold Red */
const char COLOR_BOLDGREEN[] = "\033[1m\033[32m"; /* Bold Green */
const char COLOR_BOLDYELLOW[] = "\033[1m\033[33m"; /* Bold Yellow */
const char COLOR_BOLDBLUE[] = "\033[1m\033[34m"; /* Bold Blue */
const char COLOR_BOLDMAGENTA[] = "\033[1m\033[35m"; /* Bold Magenta */
const char COLOR_BOLDCYAN[] = "\033[1m\033[36m"; /* Bold Cyan */
const char COLOR_BOLDWHITE[] = "\033[1m\033[37m"; /* Bold White */

static uint8_t gConsoleMode = 0;
 
uint8_t getConsoleMode(void)
{
    return gConsoleMode;
}
uint8_t setConsoleMode(uint8_t mode)
{
    gConsoleMode = mode;

    return 0;
}

#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
static osMessageQueueId_t pikaQueue  = NULL;
static char *pythonFile = NULL;
static void consolePikaTask(void *arg)
{ 
    PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
    if(arg){
        obj_run(pikaMain,arg);
        vTaskDelete(NULL);
        return;
    }
    else{
        rt_kprintf("\r\n%s-------------- python --------------\r\n%s",COLOR_GREEN,getConsolePrompt());
        // extern unsigned char pikaModules_py_a[];
        // obj_linkLibrary(pikaMain, pikaModules_py_a);
        // obj_runModule((PikaObj*)pikaMain, "main");
        // rt_kprintf("\r\n%s",getConsolePrompt()); 
    }
    char cmdBuffer[MSH_CMD_LENMAX];
    pikaQueue = osMessageQueueNew(1, MSH_CMD_LENMAX, NULL);
    if (pikaQueue == NULL){
        rt_kprintf("\n\rconsole_pika_task init error");
        return;
    }
    while(1)
    {
        memset(cmdBuffer, 0, sizeof(cmdBuffer));
        if((osMessageQueueGet(pikaQueue, cmdBuffer, 0, osWaitForever) == osOK))
        {
            // rt_kprintf("\n\rpika:%s",cmdBuffer);
            obj_run(pikaMain, cmdBuffer);
            if(strstr(cmdBuffer,"exit()")){
                // vTaskResume(gConsoleMshHdl);
                // rt_kprintf("\n\rpika exit:%s",cmdBuffer);
                consoleMshInit();
            }
        }
        osDelay(10);
    }
    vTaskDelete(NULL);
}

void consolePikaRun(int argc, char **argv)
{
    if(argc > 1){
        char folder[80] = {0}; 
        rt_kprintf("\r\n%s-------------- python --------------\r\n%s",COLOR_GREEN,getConsolePrompt());
        // rt_kprintf("\n\rpython %d:%s\n\r",argc,argv[1]);
        if(pythonFile == NULL){
            pythonFile = calloc(1,CONSOLE_FILE_SIZE);
            if (pythonFile == NULL){
                rt_kprintf("Failed to malloc %d for python heap.\r\n",CONSOLE_FILE_SIZE);
            }
        }
        // memset(pythonFile, 0, CONSOLE_FILE_SIZE);  
        // memset(folder, 0, sizeof(folder));  
        if(fileNameToDir(argv[1],folder))
        {
            consoleFileRead((char *)folder, pythonFile, CONSOLE_FILE_SIZE);
            if(gConsolePikaHdl == NULL){
                xTaskCreate(consolePikaTask, "python_file", TASK_SIZE_CONSOLE, pythonFile, osPriorityNormal, &gConsolePikaHdl);
                // vTaskDelete(gConsolePikaHdl);
                // gConsolePikaHdl = NULL;
            }
        }
    }
    else{
        setConsoleMode(C_MODE_PY);
        consoleCharReset();
        if(gConsolePikaHdl == NULL){
            xTaskCreate(consolePikaTask, "console_pika", TASK_SIZE_CONSOLE, NULL, osPriorityNormal, &gConsolePikaHdl);
        }
    }
}

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
MSH_CMD_EXPORT_ALIAS(consolePikaRun, pikapython, Run Python Script);
#endif


void consolePikaStop(void)
{
    if(gConsolePikaHdl){
       vTaskDelete(gConsolePikaHdl);
    }
    if(pythonFile){
        free(pythonFile);
    }
    if(gConsoleMode){
        setConsoleMode(C_MODE_MSH);;
        mem_pool_deinit();
    }
}

#endif

#ifdef FEATURE_SUBSYS_MICROPYTHON_ENABLE
// python
// python d:/test.py
void consoleMicropythonRun(int argc, char **argv)
{
    if (argc == 1)
    {
        py_init();
        setConsoleMode(C_MODE_PY);
    }
    else if (argc == 2)
    {
        char folder[80] = {0};
        if (fileNameToDir(argv[1], folder))
        {
            py_init();
            // printf("exe file:%s\r\n",folder);
            pyexec_file(folder);
            // char *content = (char *)malloc(CONSOLE_FILE_SIZE);
            // consoleFileRead(folder, content, CONSOLE_FILE_SIZE);
            // mpy_do_str(content, MP_PARSE_FILE_INPUT);
            // free(content);
#if MICROPY_PY_MICROPYTHON_MEM_INFO
            // mp_micropython_mem_info(0, NULL);
#endif
            py_deinit();
        }
    }
}
MSH_CMD_EXPORT_ALIAS(consoleMicropythonRun, python, Run Python Script);
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
static osMessageQueueId_t mshQueue  = NULL;
static void consoleMshTask(void *arg)
{
    char *mshBuffer = malloc(MSH_CMD_LENMAX);
    mshQueue = osMessageQueueNew(1, MSH_CMD_LENMAX, NULL);
    if (mshQueue == NULL || mshBuffer == NULL){
        // rt_kprintf("\n\rconsole_msh_task error");
        // ECPLAT_PRINTF(UNILOG_PLA_MSH, mshQueue, P_VALUE, "%d",MSH_CMD_LENMAX);
        while(1) osDelay(10);
    }
    while(1)
    {
        memset(mshBuffer, 0, MSH_CMD_LENMAX);
        if((osMessageQueueGet(mshQueue, mshBuffer, 0, osWaitForever) == osOK))
        {
            // ECPLAT_PRINTF(UNILOG_PLA_MSH, consoleMshTask, P_VALUE, "%d:%s",strlen(mshBuffer),mshBuffer);
            msh_exec(mshBuffer, strlen(mshBuffer));
        }
        osDelay(10);
    }
    free(mshBuffer);
    vTaskDelete(NULL);
}

void sendConsoleCmd(char *msg, uint32_t timeout)
{
    if (msg != NULL){
        if(gConsoleMode){
            #ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE	
            if(gConsolePikaHdl != NULL){
                osMessageQueuePut(pikaQueue, msg, 0, timeout);
            }
            #endif
#ifdef FEATURE_SUBSYS_MICROPYTHON_ENABLE
            if (strstr(msg, "exit()"))
            {
                py_deinit();
            }
            else
            {
                mpy_do_str(msg, MP_PARSE_SINGLE_INPUT);
            }
#endif
        }
        else if(gConsoleMshHdl != NULL){
            osMessageQueuePut(mshQueue, msg, 0, timeout);
        }
        memset(msg, 0, strlen(msg));
    }
}


// StaticTask_t             subsysConsoleTask;
// uint8_t                  subsys_console_task_stack[TASK_SIZE_CONSOLE];
void consoleMshInit(void)
{
    gConsoleMode = 0;
    osDelay(100);
    consoleCharReset();
    rt_kprintf("\r\n-------------- mshell --------------\r\n%s",getConsolePrompt());
    if(gConsoleMshHdl == NULL)
    {
        xTaskCreate(consoleMshTask, "console_msh", TASK_SIZE_CONSOLE, NULL, osPriorityNormal, &gConsoleMshHdl);
        if (gConsoleMshHdl == NULL)
        {
            printf("Failed to create thread for gConsoleMshHdl.\r\n");
            return;
        }


            // osThreadAttr_t taskAttr;

            // memset(&taskAttr,0,sizeof(taskAttr));
            // memset(subsys_console_task_stack, 0xA5,TASK_SIZE_CONSOLE);
            // taskAttr.name = "console";
            // taskAttr.stack_mem = subsys_console_task_stack;
            // taskAttr.stack_size = TASK_SIZE_CONSOLE;
            // taskAttr.priority = osPriorityNormal;
            // taskAttr.cb_mem = &subsysConsoleTask;//task control block
            // taskAttr.cb_size = sizeof(StaticTask_t);//size of task control block

            // gConsoleMshHdl = osThreadNew(consoleMshTask, NULL, &taskAttr);
            // if (gConsoleMshHdl == NULL)
            // {
            //     printf("Failed to create thread for gConsoleMshHdl.\r\n");
            //     return;
            // }
    }
    else vTaskResume(gConsoleMshHdl);
}
#endif

void consoleTaskInit(void)
{
    if(consoleHalInit()){
        if (gConsoleEventGroup == NULL)
        {
            gConsoleEventGroup = xEventGroupCreate();
            if (gConsoleEventGroup == NULL)
            {
                printf("Failed to create event for gConsoleEventGroup.\r\n");
                return;
            }
        }
        #ifdef FEATURE_SUBSYS_FINSH_ENABLE
        consoleMshInit();
        #endif
    }
}

void consoleTaskDeinit(void)
{
    setConsoleDir(DISK_NONE);
    rt_kprintf("\r\n--- exit ---%s\r\n",COLOR_WHITE);
    
    #ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE	
    consolePikaStop();
    #endif
    #ifdef FEATURE_SUBSYS_FINSH_ENABLE
    if(gConsoleMshHdl != NULL){
        vTaskDelete(gConsoleMshHdl);
    }
    #endif
    consoleHalDeinit();
}


#ifdef FEATURE_SUBSYS_FINSH_ENABLE
MSH_CMD_EXPORT_ALIAS(consoleTaskDeinit, exit, exit msh console);
#endif
#endif