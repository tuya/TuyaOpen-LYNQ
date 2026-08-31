#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "bsp_custom.h"
#include "ps_lib_api.h"
#include "cmips.h"
#include "slpman.h"
#include "cJSON.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "jsonCommon.h"


char *jsonGetString(char *buffer)
{
    char     *head   = NULL;
    char     *tail   = NULL;
    char     *json   = NULL;
    uint32_t  length = 0;

    if (buffer == NULL)
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    head = strstr(buffer, JSON_PREFIX_STR);
    if (head == NULL)
    {
        SYSLOG_ERR("Can't find the JSON string (Make sure the JSON string is compressed): buffer=%s\r\n", buffer);
        goto labelEnd;
    }

    tail   = strstr(head, JSON_POSTFIX_STR);
    length = (tail == NULL) ? strlen(head) : (tail - head);

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    json = pvPortMalloc_Psram(length + 1);
#else
    json = malloc(length + 1);
#endif
    if (json == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for json.\r\n", length + 1);
        goto labelEnd;
    }

    memset(json, 0,    length + 1);
    memcpy(json, head, length);

labelEnd:
    return json;
}

cJSON *jsonGetRoot(char *filename)
{
    cJSON       *retVal  = NULL;
    FILE        *file    = NULL;
    struct stat  buf     = {0};
    bool         opened  = false;
    uint32_t     size    = 0;
    char        *buffer  = NULL;
    char        *jsonStr = NULL;

    if (filename == NULL)
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    file = file_fopen(filename, "r");
    if (file == NULL)
    {
        SYSLOG_ERR("Failed to open the file \"%s\".\r\n", filename);
        goto labelEnd;
    }
    opened = true;

    if ((file_fstat((int)file, &buf) != 0) || (buf.st_size == 0))
    {
        SYSLOG_ERR("Failed to get size of the file \"%s\" or the file \"%s\" is empty.\r\n", filename, filename);
        goto labelEnd;
    }
    size = buf.st_size;

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    buffer = pvPortMalloc_Psram(size + 1);
#else
    buffer = malloc(size + 1);
#endif
    if (buffer == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for buffer\r\n", size + 1);
        goto labelEnd;
    }

    memset(buffer, 0, size + 1);
    file_fread((void *)buffer, size, 1, file);

    jsonStr = jsonGetString(buffer);
    if (jsonStr == NULL)
    {
        SYSLOG_ERR("Failed to get the JSON string.\r\n");
        goto labelEnd;
    }

    retVal = cJSON_Parse(jsonStr);
    if (retVal == NULL)
    {
        SYSLOG_ERR("Failed to parse the JSON string.\r\n");
        goto labelEnd;
    }

labelEnd:
    if (opened == true)
    {
        file_fclose(file);
    }
    if (buffer != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(buffer);
#else
        free(buffer);
#endif
        buffer = NULL;
    }
    if (jsonStr != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(jsonStr);
#else
        free(jsonStr);
#endif
        jsonStr = NULL;
    }

    return retVal;
}
