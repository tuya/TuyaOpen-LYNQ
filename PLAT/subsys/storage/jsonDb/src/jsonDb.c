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
#include "jsonDb.h"


#define TYPE_CHAR               "char"


typedef struct
{
    char     key[32];
    uint32_t valueType;
    uint32_t valueSize;
} JsonDbItemInfoT;

enum
{
    GET_ALL_ITEMS = 0,
    GET_ITEM_COUNT,
    GET_ITEM_BY_ID,
    GET_ITEMS_BY_STRING
};


int32_t getItemInfo(char *fileName, JsonDbItemInfoT **itemInfo)
{
    int32_t      retVal   = -1;
    FILE        *file     = NULL;
    struct stat  buf      = {0};
    char        *buffer   = NULL;
    bool         opened   = false;
    char        *posBegin = NULL;
    char        *posEnd   = NULL;
    uint32_t     index    = 0;
    uint32_t     size     = 0;
    uint32_t     length   = 0;

    if ((fileName == NULL) || (itemInfo == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    file = file_fopen(fileName, "r");
    if (file == NULL)
    {
        SYSLOG_ERR("Failed to open the file \"%s\".\r\n", fileName);
        goto labelEnd;
    }
    opened = true;

    if ((file_fstat((int)file, &buf) != 0) || (buf.st_size == 0))
    {
        SYSLOG_ERR("Failed to get size of the file \"%s\" or the file \"%s\" is empty.\r\n", fileName, fileName);
        goto labelEnd;
    }

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    buffer = pvPortMalloc_Psram(buf.st_size + 1);
#else
    buffer = malloc(buf.st_size + 1);
#endif
    if (buffer == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for buffer\r\n", buf.st_size + 1);
        goto labelEnd;
    }

    memset(buffer, 0, buf.st_size + 1);
    file_fread((void *)buffer, buf.st_size, 1, file);

    posBegin = buffer;
    while (posBegin != NULL)
    {
        posEnd = strstr(posBegin, "\r\n");
        if (posEnd == NULL)
        {
            size += (strlen(posBegin) == 0) ? 0 : 1;
            break;
        }

        if ((posEnd - posBegin) > 0)
        {
            size++;
        }

        posBegin = posEnd + strlen("\r\n");
    }

    if (size == 0)
    {
        SYSLOG_ERR("There is no valid item information.\r\n");
        goto labelEnd;
    }

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    *itemInfo = pvPortMalloc_Psram(sizeof(JsonDbItemInfoT) * size);
#else
    *itemInfo = malloc(sizeof(JsonDbItemInfoT) * size);
#endif
    if (*itemInfo == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for *itemInfo.\r\n", sizeof(JsonDbItemInfoT) * size);
        goto labelEnd;
    }
    memset(*itemInfo, 0, sizeof(JsonDbItemInfoT) * size);

    posBegin = buffer;
    while (posBegin != NULL)
    {
        posEnd = strstr(posBegin, "\r\n");
        if (posEnd == NULL)
        {
            length = strlen(posBegin);
        }
        else
        {
            length = posEnd - posBegin;
        }

        if (length > 0)
        {
            for (uint32_t i=0; i<length; i++)
            {
                if (posBegin[i] != ':')
                {
                    (*itemInfo)[index].key[i] = posBegin[i];
                }
                else
                {
                    if (memcmp(&posBegin[++i], TYPE_CHAR, strlen(TYPE_CHAR)) == 0)
                    {
                        (*itemInfo)[index].valueType = cJSON_String;
                    }
                    else
                    {
                        SYSLOG_ERR("Unsupport value type\r\n");
                        goto labelEnd;
                    }

                    (*itemInfo)[index].valueSize = atoi(&posBegin[i + strlen(TYPE_CHAR) + 1]);
                    if ((*itemInfo)[index].valueSize == 0)
                    {
                        SYSLOG_ERR("Value size is 0.\r\n");
                        goto labelEnd;
                    }

                    break;
                }
            }
            index++;
        }

        if (posEnd == NULL)
        {
            break;
        }
        posBegin = posEnd + strlen("\r\n");
    }

    retVal = 0;

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
    if ((retVal != 0) && (*itemInfo != NULL))
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(*itemInfo);
#else
        free(*itemInfo);
#endif
        *itemInfo = NULL;
    }
    else
    {
        retVal = index;
    }
    return retVal;
}

int32_t jsonDbGetTypeNumber(char *typeFileName)
{
    JsonDbItemInfoT *itemInfo = NULL;
    int32_t          count    = 0;

    count = getItemInfo(typeFileName, &itemInfo);
    if (itemInfo != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(itemInfo);
#else
        free(itemInfo);
#endif
        itemInfo = NULL;
    }

    return count;
}

int32_t jsonDbGetTypeName(char *typeFileName, uint32_t id, char **name)
{
    int32_t          retVal   = -1;
    int32_t          count    = 0;
    uint32_t         length   = 0;
    JsonDbItemInfoT *itemInfo = NULL;

    if (name == NULL)
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    count = getItemInfo(typeFileName, &itemInfo);
    if ((count <= 0) || (itemInfo == NULL))
    {
        SYSLOG_ERR("Get item info error.\r\n");
        goto labelEnd;
    }

    if (id >= count)
    {
        SYSLOG_ERR("Id error.\r\n");
        goto labelEnd;
    }

    length = strlen(itemInfo[id].key);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    *name = pvPortMalloc_Psram(length + 1);
#else
    *name  = malloc(length + 1);
#endif
    if (*name == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for *name.\r\n", length + 1);
        goto labelEnd;
    }
    memset(*name, 0, length + 1);

    memcpy(*name, itemInfo[id].key, length);
    retVal = itemInfo[id].valueSize;

labelEnd:
    if (itemInfo != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(itemInfo);
#else
        free(itemInfo);
#endif
        itemInfo = NULL;
    }

    return retVal;
}

static int32_t saveJsonDb(char *fileName, cJSON *root)
{
    int32_t  retVal  = -1;
    FILE    *file    = NULL;
    char    *jsonStr = NULL;

    if ((fileName == NULL) || (root == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    jsonStr = cJSON_PrintUnformatted(root);
    if (jsonStr == NULL)
    {
        SYSLOG_ERR("Failed to get JSON string.\r\n");
        goto labelEnd;
    }

    file = file_fopen(fileName, "w");
    if (file == NULL)
    {
        SYSLOG_ERR("Failed to open the file \"%s\".\r\n", fileName);
        goto labelEnd;
    }

    file_fwrite(jsonStr, strlen(jsonStr), 1, file);
    file_fclose(file);
    retVal = 0;

labelEnd:
    if (jsonStr != NULL)
    {
        cJSON_free(jsonStr);
        jsonStr = NULL;
    }
    return retVal;
}

static int32_t getRootAndArrayInJsonDb(char *fileName, cJSON **root, cJSON **array)
{
    int32_t retVal = -1;

    if ((fileName == NULL) || (root == NULL) || (array == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    *root = jsonGetRoot(fileName);
    if (*root == NULL)
    {
        SYSLOG_ERR("Failed to read JSON DB.\r\n");
        goto labelEnd;
    }

    *array = cJSON_GetObjectItem(*root, "nextLevel");
    if ((*array == NULL) || ((*array)->type != cJSON_Array))
    {
        SYSLOG_ERR("JSON error.\r\n");
        goto labelEnd;
    }

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t jsonDbAddItem(char *fileName, char *typeFileName, void *item)
{
    int32_t           retVal   = -1;
    JsonDbItemInfoT  *itemInfo = NULL;
    int32_t           count    = 0;
    cJSON           **node     = NULL;
    cJSON            *root     = NULL;
    cJSON            *array    = NULL;
    uint32_t          len      = 0;
    cJSON            *object   = NULL;

    if ((fileName == NULL) || (typeFileName == NULL) || (item == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    count = getItemInfo(typeFileName, &itemInfo);
    if ((count <=0 ) || (itemInfo == NULL))
    {
        SYSLOG_ERR("Failed to get item info.\r\n");
        goto labelEnd;
    }

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    node = pvPortMalloc_Psram(sizeof(cJSON *) * count);
#else
    node = malloc(sizeof(cJSON *) * count);
#endif
    if (node == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for node\r\n", sizeof(cJSON) * count);
        goto labelEnd;
    }

    SYSLOG_DEBUG("filename： %s\r\n", fileName);
    retVal = getRootAndArrayInJsonDb(fileName, &root, &array);
    if ((retVal != 0) || (root == NULL) || (array == NULL))
    {
        SYSLOG_ERR("Failed to get root and array in JSON DB.\r\n");
        goto labelEnd;
    }

    object = cJSON_CreateObject();
    if (object == NULL)
    {
        SYSLOG_ERR("Failed to create object.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<count; i++)
    {
        node[i] = cJSON_CreateString(item + len);
        if (node[i] == NULL)
        {
            SYSLOG_ERR("Failed to create string.\r\n");
            goto labelEnd;
        }
        cJSON_AddItemToObject(object, itemInfo[i].key, node[i]);
        len += itemInfo[i].valueSize;
    }

    cJSON_AddItemToArray(array, object);
    saveJsonDb(fileName, root);

    retVal = 0;

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    if (itemInfo != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(itemInfo);
#else
        free(itemInfo);
#endif
        itemInfo = NULL;
    }
    if (node != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(node);
#else
        free(node);
#endif
        node = NULL;
    }
    return retVal;
}

int32_t jsonDbDeleteItem(char *fileName, char *typeFileName, uint32_t id)
{
    int32_t           retVal   = -1;
    JsonDbItemInfoT  *itemInfo = NULL;
    int32_t           count    = 0;
    cJSON           **node     = NULL;
    cJSON            *root     = NULL;
    cJSON            *array    = NULL;

    if ((fileName == NULL) || (typeFileName == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    count = getItemInfo(typeFileName, &itemInfo);
    if ((count <=0 ) || (itemInfo == NULL))
    {
        SYSLOG_ERR("Failed to get item info.\r\n");
        goto labelEnd;
    }

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    node = pvPortMalloc_Psram(sizeof(cJSON) * count);
#else
    node = malloc(sizeof(cJSON) * count);
#endif
    if (node == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for node\r\n", sizeof(cJSON) * count);
        goto labelEnd;
    }

    SYSLOG_DEBUG("filename： %s\r\n", fileName);
    retVal = getRootAndArrayInJsonDb(fileName, &root, &array);
    if ((retVal != 0) || (root == NULL) || (array == NULL))
    {
        SYSLOG_ERR("Failed to get root and array in JSON DB.\r\n");
        goto labelEnd;
    }

    cJSON_DeleteItemFromArray(array, id);
    saveJsonDb(fileName, root);

    retVal = 0;

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    if (itemInfo != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(itemInfo);
#else
        free(itemInfo);
#endif
        itemInfo = NULL;
    }
    if (node != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(node);
#else
        free(node);
#endif
        node = NULL;
    }
    return retVal;
}

int32_t jsonDbUpdateItem(char *fileName, char *typeFileName, uint32_t id, void *itemNew)
{
    int32_t           retVal   = -1;
    JsonDbItemInfoT  *itemInfo = NULL;
    int32_t           count    = 0;
    cJSON           **node     = NULL;
    cJSON            *root     = NULL;
    cJSON            *array    = NULL;
    uint32_t          len      = 0;
    cJSON            *object   = NULL;

    if ((fileName == NULL) || (typeFileName == NULL) || (itemNew == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    count = getItemInfo(typeFileName, &itemInfo);
    if ((count <=0 ) || (itemInfo == NULL))
    {
        SYSLOG_ERR("Failed to get item info.\r\n");
        goto labelEnd;
    }

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    node = pvPortMalloc_Psram(sizeof(cJSON) * count);
#else
    node = malloc(sizeof(cJSON) * count);
#endif
    if (node == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for node\r\n", sizeof(cJSON) * count);
        goto labelEnd;
    }

    SYSLOG_DEBUG("filename： %s\r\n", fileName);
    retVal = getRootAndArrayInJsonDb(fileName, &root, &array);
    if ((retVal != 0) || (root == NULL) || (array == NULL))
    {
        SYSLOG_ERR("Failed to get root and array in JSON DB.\r\n");
        goto labelEnd;
    }

    object = cJSON_CreateObject();
    if (object == NULL)
    {
        SYSLOG_ERR("Failed to create object.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<count; i++)
    {
        node[i] = cJSON_CreateString(itemNew + len);
        if (node[i] == NULL)
        {
            SYSLOG_ERR("Failed to create string.\r\n");
            goto labelEnd;
        }
        cJSON_AddItemToObject(object, itemInfo[i].key, node[i]);
        len += itemInfo[i].valueSize;
    }

    cJSON_ReplaceItemInArray(array, id, object);
    saveJsonDb(fileName, root);

    retVal = 0;

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    if (itemInfo != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(itemInfo);
#else
        free(itemInfo);
#endif
        itemInfo = NULL;
    }
    if (node != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(node);
#else
        free(node);
#endif
        node = NULL;
    }
    return retVal;
}

static int32_t getItems(char *fileName, char *typeFileName, uint8_t type, char *matchStr, int32_t matchId, void **items, uint32_t **ids)
{
    int32_t          retVal    = -1;
    JsonDbItemInfoT *itemInfo  = NULL;
    int32_t          count     = 0;
    cJSON           *root      = NULL;
    cJSON           *array     = NULL;
    cJSON           *arrayItem = NULL;
    cJSON           *node      = NULL;
    int32_t          size      = 0;
    uint32_t         length    = 0;
    uint32_t         len       = 0;
    uint32_t         index     = 0;
    bool             matched   = false;
    uint32_t         itemsSize = 0;

    if ((fileName == NULL) || (typeFileName == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    count = getItemInfo(typeFileName, &itemInfo);
    if ((count <=0 ) || (itemInfo == NULL))
    {
        SYSLOG_ERR("Failed to get item info.\r\n");
        goto labelEnd;
    }

    retVal = getRootAndArrayInJsonDb(fileName, &root, &array);
    if ((retVal != 0) || (root == NULL) || (array == NULL))
    {
        SYSLOG_ERR("Failed to get root and array in JSON DB.\r\n");
        goto labelEnd;
    }

    size = cJSON_GetArraySize(array);
    if (size <= 0)
    {
        SYSLOG_DEBUG("The array in the JSON DB is empty.\r\n");
        goto labelEnd;
    }

    if (type == GET_ITEM_COUNT)
    {
        retVal = size;
        goto labelEnd;
    }

    for (uint32_t i=0; i<count; i++)
    {
        length += itemInfo[i].valueSize;
    }

    if (items == NULL)
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    itemsSize = (type == GET_ITEM_BY_ID) ? length : (length * size);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    *items = pvPortMalloc_Psram(itemsSize);
#else
    *items = malloc(itemsSize);
#endif
    if (*items == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for *items\r\n", itemsSize);
        goto labelEnd;
    }
    memset(*items, 0, itemsSize);

    if (type == GET_ITEM_BY_ID)
    {
        arrayItem = cJSON_GetArrayItem(array, matchId);
        if (arrayItem != NULL)
        {
            len = 0;
            for (uint32_t j=0; j<count; j++)
            {
                node = cJSON_GetObjectItem(arrayItem, itemInfo[j].key);
                if ((node != NULL) && (node->type == cJSON_String))
                {
                    memcpy(((uint8_t *)(*items) + len), node->valuestring, MIN(strlen(node->valuestring), itemInfo[j].valueSize - 1));
                    len += itemInfo[j].valueSize;
                }
                else
                {
                    SYSLOG_ERR("Json item error.\r\n");
                    goto labelEnd;
                }
            }
        }
        else
        {
            SYSLOG_ERR("Json item error.\r\n");
            goto labelEnd;
        }

        index  = 1;
        retVal = 0;
        goto labelEnd;
    }

    if (ids != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        *ids = pvPortMalloc_Psram(sizeof(uint32_t) * size);
#else
        *ids = malloc(sizeof(uint32_t) * size);
#endif
        if (*ids == NULL)
        {
            SYSLOG_ERR("Failed to malloc %d bytes for *ids\r\n", sizeof(uint32_t) * size);
            goto labelEnd;
        }
        memset(*ids, 0, sizeof(uint32_t) * size);
    }

    for (uint32_t i=0; i<size; i++)
    {
        arrayItem = cJSON_GetArrayItem(array, i);
        if (arrayItem != NULL)
        {
            matched = false;
            if ((matchStr != NULL) && (strlen(matchStr) > 0))
            {
                for (uint32_t j=0; j<count; j++)
                {
                    node = cJSON_GetObjectItem(arrayItem, itemInfo[j].key);
                    if ((node != NULL) && (node->type == cJSON_String))
                    {
                        if ((strlen(node->valuestring) >= strlen(matchStr)) && (strstr(node->valuestring, matchStr) != NULL))
                        {
                            matched = true;
                            break;
                        }
                    }
                    else
                    {
                        SYSLOG_ERR("Json item error.\r\n");
                        goto labelEnd;
                    }
                }
            }
            else
            {
                matched = true;
            }

            if (matched != true)
            {
                continue;
            }

            len = 0;
            for (uint32_t j=0; j<count; j++)
            {
                node = cJSON_GetObjectItem(arrayItem, itemInfo[j].key);
                if ((node != NULL) && (node->type == cJSON_String))
                {
                    memcpy(((uint8_t *)(*items) + index * length + len), node->valuestring, MIN(strlen(node->valuestring), itemInfo[j].valueSize - 1));
                    len += itemInfo[j].valueSize;
                    if ((ids != NULL) && (*ids != NULL))
                    {
                        *((uint32_t *)(*ids) + index) = i;
                    }
                }
                else
                {
                    SYSLOG_ERR("Json item error.\r\n");
                    goto labelEnd;
                }
            }
            index++;
        }
    }

    retVal = 0;

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    if (itemInfo != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(itemInfo);
#else
        free(itemInfo);
#endif
        itemInfo = NULL;
    }
    if ((retVal != 0) || (index == 0))
    {
        if (itemInfo != NULL)
        {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(*items);
#else
            free(*items);
#endif
            *items = NULL;
        }
        if (itemInfo != NULL)
        {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(*ids);
#else
            free(*ids);
#endif
            *ids = NULL;
        }
    }
    if (retVal == 0)
    {
        retVal = index;
    }

    return retVal;
}

int32_t jsonDbGetItemById(char *fileName, char *typeFileName, uint32_t id, void **item)
{
    return getItems(fileName, typeFileName, GET_ITEM_BY_ID, NULL, id, item, NULL);
}

int32_t jsonDbGetMatchedItemsByStr(char *fileName, char *typeFileName, char *matchStr, void **items, uint32_t **ids)
{
    return getItems(fileName, typeFileName, GET_ITEMS_BY_STRING, matchStr, 0, items, ids);
}

int32_t jsonDbGetAllItems(char *fileName, char *typeFileName, void **items)
{
    return getItems(fileName, typeFileName, GET_ALL_ITEMS, NULL, 0, items, NULL);
}

int32_t jsonDbGetTotalCount(char *fileName, char *typeFileName)
{
    return getItems(fileName, typeFileName, GET_ITEM_COUNT, NULL, 0, NULL, NULL);
}

#ifdef JSON_DB_TEST
typedef struct
{
    char name[40];
    char number[40];
} JsonDbTestT;

int32_t jsonDbTestPrint(char *fileName, char *typeFileName)
{
    int32_t      retVal  = -1;
    cJSON       *root    = NULL;
    char        *jsonStr = NULL;
    JsonDbTestT *items   = NULL;
    uint32_t    *ids     = NULL;
    uint32_t     count   = 0;

    if ((fileName == NULL) || (typeFileName == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    root = jsonGetRoot(fileName);
    if (root == NULL)
    {
        SYSLOG_ERR("Failed to read JSON DB.\r\n");
        goto labelEnd;
    }

    jsonStr = cJSON_PrintUnformatted(root);
    if (jsonStr == NULL)
    {
        SYSLOG_ERR("JSON error.\r\n");
        goto labelEnd;
    }

    SYSLOG_INFO("jsonStr: %s\r\n", jsonStr);

    count = jsonDbGetAllItems(fileName, typeFileName, &items);
    SYSLOG_INFO("All items (count=%d):\r\n", count);
    if ((count != 0) && (items != NULL))
    {
        for (uint32_t i=0; i<count; i++)
        {
            printf("\t%s: %s\r\n", items[i].name, items[i].number);
        }
    }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    vPortFree_Psram(items);
#else
    free(items);
#endif
    items = NULL;

    count = jsonDbGetMatchedItemsByStr(fileName, typeFileName, "23", (void **)&items, &ids);
    SYSLOG_INFO("Matched \"23\" items (count=%d):\r\n", count);
    if ((count != 0) && (items != NULL) && (ids != NULL))
    {
        for (uint32_t i=0; i<count; i++)
        {
            printf("\t%s: %s (%d)\r\n", items[i].name, items[i].number, ids[i]);
        }
    }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    vPortFree_Psram(items);
#else
    free(items);
#endif
    items = NULL;
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    vPortFree_Psram(ids);
#else
    free(ids);
#endif
    ids = NULL;

    count = jsonDbGetTotalCount(fileName, typeFileName);
    SYSLOG_INFO("Get total count: %d\r\n", count);
    if (count > 0)
    {
        for (uint32_t i=0; i<count; i++)
        {
            if ((jsonDbGetItemById(fileName, typeFileName, i, (void **)&items) == 1) && (items != NULL))
            {
                SYSLOG_INFO("Item (%d): %s: %s\r\n", i, items->name, items->number);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(items);
#else
                free(items);
#endif
                items = NULL;
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

void jsonDbTest(char *fileName, char *typeFileName)
{
    jsonDbTestPrint(fileName, typeFileName);

    SYSLOG_INFO("添加 张三、李四和王五\r\n");
    char contact0[2][32] = {"张三", "123"};
    char contact1[2][32] = {"李四", "456"};
    char contact2[2][32] = {"王五", "789"};
    jsonDbAddItem(fileName, typeFileName, contact0);
    jsonDbAddItem(fileName, typeFileName, contact1);
    jsonDbAddItem(fileName, typeFileName, contact2);
    jsonDbTestPrint(fileName, typeFileName);

    SYSLOG_INFO("改为 李四 为 李23四: 654\r\n");
    char contactNew[2][32] = {"李23四", "654"};
    jsonDbUpdateItem(fileName, typeFileName, 1, contactNew);
    jsonDbTestPrint(fileName, typeFileName);

    SYSLOG_INFO("删除 李23四\r\n");
    jsonDbDeleteItem(fileName, typeFileName, 1);
    jsonDbTestPrint(fileName, typeFileName);

    SYSLOG_INFO("删除 张三\r\n");
    jsonDbDeleteItem(fileName, typeFileName, 0);
    jsonDbTestPrint(fileName, typeFileName);

    SYSLOG_INFO("删除 王五\r\n");
    jsonDbDeleteItem(fileName, typeFileName, 0);
    jsonDbTestPrint(fileName, typeFileName);


    uint32_t  length = 0;
    char     *name   = NULL;
    uint32_t  count  = jsonDbGetTypeNumber(typeFileName);
    SYSLOG_INFO("Type file: count=%d\r\n", count);
    for (uint32_t i=0; i<count; i++)
    {
        length = jsonDbGetTypeName(typeFileName, i, &name);
        if (name != NULL)
        {
            printf("\t%s: %d\r\n", name, length);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(name);
#else
            free(name);
#endif
            name = NULL;
        }
    }
}
#endif

void jsonDbFree(void **object)
{
    if ((object != NULL) && (*object != NULL))
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(*object);
#else
        free(*object);
#endif
        *object = NULL;
    }
}