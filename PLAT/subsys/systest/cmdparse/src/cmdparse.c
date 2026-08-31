#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "bsp_custom.h"
#include "ps_lib_api.h"
#include "cmips.h"
#include "slpman.h"
#include "cJSON.h"
#include "cmdparse.h"
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
#include "input.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
#include "powermanager.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_scr.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_CMDPARSE                  (2 * 1024)
#define QUEUE_SIZE_CMDPARSE_RECV                    50
#define QUEUE_SIZE_CMDPARSE_RESULT                  50


typedef struct
{
    void *buffer;
} CmdParseRecvT;


#ifdef THREAD_STATIC
static StaticTask_t       gCmdparseThreadCbMem                                = {0};
static uint8_t            gCmdparseThreadStackMem[THREAD_STACK_SIZE_CMDPARSE] = {0};
#endif
static osThreadId_t       gCmdParseThread      = NULL;
static osMessageQueueId_t gCmdParseRecvQueue   = NULL;
static osMessageQueueId_t gCmdParseResultQueue = NULL;


/* JSON Example
{
  "name": "string",
  "param1": int32_t,
  "param2": int32_t,
  "param3": "string"
}
*/

static void cmdParseDelete(void)
{
    if (gCmdParseRecvQueue != NULL)
    {
        osMessageQueueDelete(gCmdParseRecvQueue);
        gCmdParseRecvQueue = NULL;
    }

    if (gCmdParseResultQueue != NULL)
    {
        osMessageQueueDelete(gCmdParseResultQueue);
        gCmdParseResultQueue = NULL;
    }

    if (gCmdParseThread != NULL)
    {
        if (osThreadGetId() != gCmdParseThread)
        {
            osThreadTerminate(gCmdParseThread);
            gCmdParseThread = NULL;
        }
        else
        {
            gCmdParseThread = NULL;
#if 0 // Service Manager
            osThreadExit();
#else
            Service_stop("service:/threadCmdParse");
#endif
        }
    }
}

static int32_t cmdParseJsonParse(char *json, CmdParseResultT *result)
{
    int32_t  retVal = -1;
    uint32_t length = 0;
    cJSON    *root  = NULL;
    cJSON    *node  = NULL;

    if (json == NULL)
    {
        SYSLOG_ERR("json is NULL.\r\n");
        goto labelEnd;
    }

    if (result == NULL)
    {
        SYSLOG_ERR("result is NULL.\r\n");
        goto labelEnd;
    }

    root = cJSON_Parse(json);
    if (root == NULL)
    {
        SYSLOG_ERR("Failed to parse the JSON string.\r\n");
        goto labelEnd;
    }

    node = cJSON_GetObjectItem(root, "name");
    if ((node == NULL)
     || (node->type != cJSON_String)
     || (strlen(node->valuestring) == 0)
     || (strlen(node->valuestring) > CMDPARSE_NAME_LEN_MAX))
    {
        SYSLOG_ERR("The \"name\" item is incorrect.\r\n");
        goto labelEnd;
    }

    memset(result, 0, sizeof(CmdParseResultT));
    memcpy(result->name, node->valuestring, strlen(node->valuestring));

    node = cJSON_GetObjectItem(root, "param1");
    if ((node != NULL) && (node->type == cJSON_Number))
    {
        result->param1 = node->valueint;
    }

    node = cJSON_GetObjectItem(root, "param2");
    if ((node != NULL) && (node->type == cJSON_Number))
    {
        result->param2 = node->valueint;
    }

    node = cJSON_GetObjectItem(root, "param3");
    if ((node != NULL) && (node->type == cJSON_String))
    {
        length = strlen(node->valuestring);
        if (length > 0)
        {
            result->param3 = malloc(length + 1);
            if (result->param3 != NULL)
            {
                memset(result->param3, 0, length + 1);
                memcpy(result->param3, node->valuestring, length);
            }
        }
    }

    retVal = 0;

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    return retVal;
}

static char *cmdParseJsonGet(char **pos)
{
    char     *head  = NULL;
    char     *tail  = NULL;
    char     *json  = NULL;
    uint32_t length = 0;

    if ((pos == NULL) || (*pos == NULL))
    {
        SYSLOG_ERR("%s\r\n", (pos == NULL) ? "pos is NULL." : "*pos is NULL");
        goto labelEnd;
    }

    head = strstr(*pos, CMDPARSE_PREFIX_STR);
    if (head == NULL)
    {
        *pos = NULL;
        goto labelEnd;
    }

    tail   = strstr(head, CMDPARSE_POSTFIX_STR);
    length = (tail == NULL) ? strlen(head) : (tail - head);

    json = malloc(length + 1);
    if (json == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for json.\r\n", length + 1);
        goto labelEnd;
    }

    memset(json, 0,    length + 1);
    memcpy(json, head, length);

    *pos = (tail == NULL) ? NULL : (tail + strlen(CMDPARSE_POSTFIX_STR));

labelEnd:
    return json;
}

static void threadCmdParse(void *argument)
{
    CmdParseRecvT   cmdParseRecv   = {0};
    CmdParseResultT cmdParseResult = {0};
    char            *head          = NULL;
    char            *json          = NULL;

    gCmdParseRecvQueue = osMessageQueueNew(QUEUE_SIZE_CMDPARSE_RECV, sizeof(cmdParseRecv), NULL);
    if (gCmdParseRecvQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gCmdParseRecvQueue.\r\n");
        goto labelEnd;
    }

    gCmdParseResultQueue = osMessageQueueNew(QUEUE_SIZE_CMDPARSE_RESULT, sizeof(cmdParseResult), NULL);
    if (gCmdParseResultQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gCmdParseResultQueue.\r\n");
        goto labelEnd;
    }

    while (1)
    {
        memset(&cmdParseRecv, 0, sizeof(cmdParseRecv));
        if (osMessageQueueGet(gCmdParseRecvQueue, &cmdParseRecv, 0, osWaitForever) == osOK)
        {
            if (cmdParseRecv.buffer != NULL)
            {
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
                if ((powerManagerGetBacklightState() == false) || (powerManagerGetPowerState() != POWER_STATE_WORKING))
                {
                    SYSLOG_DEBUG("cmdParseRecv=%s\r\n", cmdParseRecv.buffer);
                    continue;
                }
                powerManagerResetBacklightTimer();
#endif

                head = cmdParseRecv.buffer;
                while (head != NULL)
                {
                    json = cmdParseJsonGet(&head);
                    if (json != NULL)
                    {
                        if (cmdParseJsonParse(json, &cmdParseResult) == 0)
                        {
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
                            if (systestProcess(cmdParseResult.name, (SystestParamT *)(&(cmdParseResult.param1))) != 0)
#endif
                            {
                                osMessageQueuePut(gCmdParseResultQueue, &cmdParseResult, 0, osWaitForever);
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
                                inputNotify();
#endif
                            }
                        }
                        free(json);
                        json = NULL;
                    }
                }

                free(cmdParseRecv.buffer);
                cmdParseRecv.buffer = NULL;
            }
        }
    }

labelEnd:
    cmdParseDelete();
}

void cmdparseInit(void)
{
    osThreadAttr_t threadAttr = {0};

    if (gCmdParseThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadCmdParse";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_CMDPARSE;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gCmdparseThreadStackMem;
        threadAttr.cb_mem     = &gCmdparseThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
        gCmdParseThread = osThreadNew(threadCmdParse, NULL, &threadAttr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
        Service_reg(serviceName, threadCmdParse, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gCmdParseThread = (osThreadId_t)Service_start(serviceName);
#endif
        if (gCmdParseThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for gCmdParseThread.\r\n");
        }
    }
}

int32_t cmdparseQueuePut(void *data, uint32_t length)
{
    int32_t       retVal       = -1;
    CmdParseRecvT cmdParseRecv = {0};

    if ((data == NULL) || (length == 0))
    {
        SYSLOG_ERR("%s\r\n", (data == NULL) ? "data is NULL." : "length is 0.");
        goto labelEnd;
    }

    cmdParseRecv.buffer = malloc(length + 1);
    if (cmdParseRecv.buffer == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for cmdParseRecv.buffer.\r\n", length + 1);
        goto labelEnd;
    }
    else
    {
        memset(cmdParseRecv.buffer, 0,    length + 1);
        memcpy(cmdParseRecv.buffer, data, length);
        osMessageQueuePut(gCmdParseRecvQueue, &cmdParseRecv, 0, osWaitForever);
    }

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t cmdParseResultGet(CmdParseResultT *result, uint32_t timeout)
{
    return osMessageQueueGet(gCmdParseResultQueue, result, 0, timeout);
}
