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
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonCommon.h"
#endif
#include "watchMenu.h"
#include "ui.h"

#define INI_KEY_WATCH_MENU          "watchMenu"


static uint32_t  gWatchMenuConfig     = 0;
char            *gWatchMenuFilePath[] = {"D:/watchMenu_cn.json", "D:/watchMenu_en.json"};

uint32_t gFirstMenuCount                                              = 0;
uint32_t gFirstMenuId[FIRST_MENU_COUNT_MAX]                           = {0};
char     gFirstMenuName[FIRST_MENU_COUNT_MAX][MENU_NAME_LENGTH_MAX]   = {0};
char     gFirstMenuImage[FIRST_MENU_COUNT_MAX][IMGGE_PATH_LENGTH_MAX] = {0};

uint32_t gSecondMenuCount[FIRST_MENU_COUNT_MAX]                                               = {0};
uint32_t gSecondMenuId[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX]                           = {0};
char     gSecondMenuName[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][MENU_NAME_LENGTH_MAX]   = {0};
char     gSecondMenuImage[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][IMGGE_PATH_LENGTH_MAX] = {0};

uint32_t gThirdMenuCount[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX]                                              = {0};
uint32_t gThirdMenuId[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][THIRD_MENU_COUNT_MAX]                           = {0};
char     gThirdMenuName[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][THIRD_MENU_COUNT_MAX][MENU_NAME_LENGTH_MAX]   = {0};
// char     gThirdMenuImage[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][THIRD_MENU_COUNT_MAX][IMGGE_PATH_LENGTH_MAX] = {0};


static int32_t jsonStringParse(char *json)
{
    int32_t  retVal      = -1;
    cJSON   *root        = NULL;
    cJSON   *item        = NULL;
    cJSON   *arrayFirst  = NULL;
    cJSON   *arraySecond = NULL;
    cJSON   *arrayThird  = NULL;

    if (json == NULL)
    {
        SYSLOG_ERR("json is NULL.\r\n");
        goto labelEnd;
    }

    root = cJSON_Parse(json);
    if (root == NULL)
    {
        SYSLOG_ERR("Failed to parse the JSON string.\r\n");
        goto labelEnd;
    }

    memset(gFirstMenuId,     0, sizeof(gFirstMenuId));
    memset(gFirstMenuName,   0, sizeof(gFirstMenuName));
    memset(gFirstMenuImage,  0, sizeof(gFirstMenuImage));

    memset(gSecondMenuCount, 0, sizeof(gSecondMenuCount));
    memset(gSecondMenuId,    0, sizeof(gSecondMenuId));
    memset(gSecondMenuName,  0, sizeof(gSecondMenuName));
    memset(gSecondMenuImage, 0, sizeof(gSecondMenuImage));

    arrayFirst      = cJSON_GetObjectItem(root, "nextLevel");
    gFirstMenuCount = cJSON_GetArraySize(arrayFirst);
    SYSLOG_INFO("[%s] gFirstMenuCount=%d\r\n", cJSON_GetObjectItem(root, "name")->valuestring, gFirstMenuCount);
    
    for(uint32_t i=0; i<gFirstMenuCount; i++)
    {
        item            = cJSON_GetArrayItem(arrayFirst, i);
        gFirstMenuId[i] = cJSON_GetObjectItem(item, "id")->valueint;
        strcpy(gFirstMenuName[i],  cJSON_GetObjectItem(item, "name")->valuestring);
        strcpy(gFirstMenuImage[i], cJSON_GetObjectItem(item, "image")->valuestring);

        #ifdef WATCH_MENU_TEST
        LV_LOG_WARN("%d-%d-%s,%s",i,gFirstMenuId[i],gFirstMenuName[i],gFirstMenuImage[i]);
        SYSLOG_DEBUG("menu[%d] content:\r\n%s\r\n", i, cJSON_Print(item));
        SYSLOG_DEBUG("gFirstMenuId[%d]=%d\r\n",     i, gFirstMenuId[i]);
        SYSLOG_DEBUG("gFirstMenuName[%d]=%s\r\n",   i, gFirstMenuName[i]);
        SYSLOG_DEBUG("gFirstMenuImage[%d]=%s\r\n",  i, gFirstMenuImage[i]);
        #endif
        arraySecond = cJSON_GetObjectItem(item, "nextLevel");
        if (arraySecond != NULL)
        {
            gSecondMenuCount[i] = cJSON_GetArraySize(arraySecond);
            SYSLOG_INFO("gSecondMenuCount[%d]=%d\r\n", i, gSecondMenuCount[i]);
            for (uint32_t j=0; j<gSecondMenuCount[i]; j++)
            {
                item                = cJSON_GetArrayItem(arraySecond, j);
                gSecondMenuId[i][j] = cJSON_GetObjectItem(item, "id")->valueint;
                strcpy(gSecondMenuName[i][j],  cJSON_GetObjectItem(item, "name")->valuestring);
                strcpy(gSecondMenuImage[i][j], cJSON_GetObjectItem(item, "image")->valuestring);
                #ifdef WATCH_MENU_TEST
                SYSLOG_DEBUG("menu[%d][%d] content:\r\n%s\r\n", i, j, cJSON_Print(item));
                SYSLOG_DEBUG("gSecondMenuId[%d][%d]=%d\r\n",    i, j, gSecondMenuId[i][j]);
                SYSLOG_DEBUG("gSecondMenuName[%d][%d]=%s\r\n",  i, j, gSecondMenuName[i][j]);
                SYSLOG_DEBUG("gSecondMenuImage[%d][%d]=%s\r\n", i, j, gSecondMenuImage[i][j]);
                #endif
                arrayThird = cJSON_GetObjectItem(item, "nextLevel");
                if (arrayThird != NULL)
                {
                    gThirdMenuCount[i][j] = cJSON_GetArraySize(arrayThird);
                    SYSLOG_INFO("gThirdMenuCount[%d][%d]=%d\r\n", i, j, gThirdMenuCount[i][j]);
                    for (uint32_t k=0; k<gThirdMenuCount[i][j]; k++)
                    {
                        item                  = cJSON_GetArrayItem(arrayThird, k);
                        gThirdMenuId[i][j][k] = cJSON_GetObjectItem(item, "id")->valueint;
                        strcpy(gThirdMenuName[i][j][k],  cJSON_GetObjectItem(item, "name")->valuestring);
                        // strcpy(gThirdMenuImage[i][j][k], cJSON_GetObjectItem(item, "image")->valuestring);
                        #ifdef WATCH_MENU_TEST
                        SYSLOG_DEBUG("menu[%d][%d][%d] content:\r\n%s\r\n", i, j, k, cJSON_Print(item));
                        SYSLOG_DEBUG("gThirdMenuId[%d][%d][%d]=%d\r\n",    i, j, k, gThirdMenuId[i][j][k]);
                        SYSLOG_DEBUG("gThirdMenuName[%d][%d][%d]=%s\r\n",  i, j, k, gThirdMenuName[i][j][k]);
                        // SYSLOG_DEBUG("gThirdMenuImage[%d][%d][%d]=%s\r\n", i, j, k, gThirdMenuImage[i][j][k]);
                        #endif
                    }
                }
            }
        }
    }

#ifdef JSON_DB_TEST
    cJSON *node = cJSON_GetObjectItem(root, "name");
    if ((node != NULL) && (node->type == cJSON_String)
     && (strlen(node->valuestring) == strlen("手表菜单"))
     && (memcmp(node->valuestring, "手表菜单", strlen("手表菜单")) == 0))
    {
        node = cJSON_GetObjectItem(root, "nextLevel");
        if ((node != NULL) && (node->type == cJSON_Array))
        {
            uint32_t   count    = 0;
            MenuNameT *menuList = NULL;
            count = getNextLevelMenu(node, &menuList);
            if ((count > 0) && (menuList != NULL))
            {
                SYSLOG_INFO("First level menu (count=%d):\r\n", count);
                for (uint32_t i=0; i<count; i++)
                {
                    printf("\t%s\r\n", menuList[i].name);
                }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(menuList);
#else
                free(menuList);
#endif
                menuList = NULL;
            }
        }
    }
#endif

    retVal = 0;

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    return retVal;
}

int32_t getNextLevelMenu(cJSON *root, MenuNameT **menu)
{
    int32_t    retVal    = -1;
    int32_t    count     = 0;
    cJSON     *arrayItem = NULL;
    cJSON     *node      = NULL;
    MenuNameT *menuList  = NULL;

    if (cJSON_IsArray(root) != true)
    {
        SYSLOG_ERR("There is no menu in next level.\r\n");
        goto labelEnd;
    }

    count = cJSON_GetArraySize(root);
    if (count <= 0)
    {
        SYSLOG_ERR("The menu count is error: %d\r\n", count);
        goto labelEnd;
    }

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    menuList = pvPortMalloc_Psram(sizeof(MenuNameT) * count);
#else
    menuList = malloc(sizeof(MenuNameT) * count);
#endif
    if (menuList == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for menuList\r\n", sizeof(MenuNameT) * count);
        goto labelEnd;
    }

    retVal = 0;
    memset(menuList, 0, sizeof(MenuNameT) * count);
    for (uint32_t i=0; i<count; i++)
    {
        arrayItem = cJSON_GetArrayItem(root, i);
        if (arrayItem != NULL)
        {
            node = cJSON_GetObjectItem(arrayItem, "name");
            if ((node != NULL) && (node->type == cJSON_String))
            {
                memcpy(menuList[i].name, node->valuestring, MIN(strlen(node->valuestring), sizeof(menuList[i].name) - 1));
                retVal++;
            }
            else
            {
                SYSLOG_ERR("The menu has no name.\r\n");
            }
        }
    }

    if (retVal == 0)
    {
        SYSLOG_ERR("There is no menu.\r\n");
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(menuList);
#else
        free(menuList);
#endif
        menuList = NULL;
    }
    else
    {
        *menu = menuList;
    }

labelEnd:
    return retVal;
}

int32_t watchMenuRead(char *filename)
{
    int32_t      retVal = -1;
    FILE        *file   = NULL;
    struct stat  buf    = {0};
    bool         opened = false;
    uint32_t     size   = 0;
    char        *buffer = NULL;
    char        *json   = NULL;

    SYSLOG_ERR("Open the file \"%s\".\r\n", filename);
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

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    json = jsonGetString(buffer);
#endif
    if (json != NULL)
    {
        jsonStringParse(json);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(json);
#else
        free(json);
#endif
        json = NULL;
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

    return retVal;
}

int32_t watchMenuGetInfo(char *fileName, MenuInfoT *menuInfo)
{
    int32_t  retVal   = -1;
    cJSON   *root     = NULL;
    cJSON   *array[3] = {0};
    cJSON   *level[3] = {0};

    if ((fileName == NULL) || (menuInfo == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    root = jsonGetRoot(fileName);
#endif
    if (root == NULL)
    {
        SYSLOG_ERR("Failed to get JSON root: %s\r\n", fileName);
        goto labelEnd;
    }

    array[0] = cJSON_GetObjectItem(root, "nextLevel");
    if ((array[0] != NULL) && (array[0]->type == cJSON_Array))
    {
        menuInfo->totalLevel = 1;
        menuInfo->level0Count = cJSON_GetArraySize(array[0]);
        for (uint32_t i=0; i<menuInfo->level0Count; i++)
        {
            level[0] = cJSON_GetArrayItem(array[0], i);
            if (level[0] != NULL)
            {
                array[1] = cJSON_GetObjectItem(level[0], "nextLevel");
                if ((array[1] != NULL) && (array[1]->type == cJSON_Array))
                {
                    menuInfo->totalLevel = 2;
                    menuInfo->level1Count[i] = cJSON_GetArraySize(array[1]);
                    for (uint32_t j=0; j<menuInfo->level1Count[i]; j++)
                    {
                        level[1] = cJSON_GetArrayItem(array[1], j);
                        if (level[1] != NULL)
                        {
                            array[2] = cJSON_GetObjectItem(level[1], "nextLevel");
                            if ((array[2] != NULL) && (array[2]->type == cJSON_Array))
                            {
                                menuInfo->totalLevel = 3;
                                menuInfo->level2Count[i][j] = cJSON_GetArraySize(array[2]);
                            }
                        }
                    }
                }
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

static int32_t queryArrayDetails(cJSON *node, MenuQueryT *menuQuery, void *buffer)
{
    int32_t   retVal     = -1;
    cJSON    *array      = NULL;
    cJSON    *level      = NULL;
    cJSON    *item       = NULL;
    uint32_t  totalCount = 0;
    uint32_t  count      = 0;
    uint32_t  endIndex   = 0;
    uint32_t  index      = 0;
    uint32_t  length     = 0;

    if ((node == NULL) || (menuQuery == NULL) || (buffer == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

    array = cJSON_GetObjectItem(node, "nextLevel");
    if ((array != NULL) && (array->type == cJSON_Array))
    {
        totalCount = cJSON_GetArraySize(array);
        if (menuQuery->startIndex >= totalCount)
        {
            SYSLOG_ERR("startIndex must be less than totalCount: startIndex=%d, totalCount=%d\r\n", menuQuery->startIndex, totalCount);
            goto labelEnd;
        }

        if (menuQuery->count == -1)
        {
            count    = totalCount - menuQuery->startIndex;
            endIndex = totalCount;
        }
        else
        {
            if (menuQuery->startIndex + menuQuery->count > totalCount)
            {
                SYSLOG_ERR("startIndex plus count must be less than or equal to totalCount: startIndex=%d, count=%d, totalCount=%d\r\n", menuQuery->startIndex, menuQuery->count, totalCount);
                goto labelEnd;
            }

            count    = menuQuery->count;
            endIndex = menuQuery->startIndex + menuQuery->count;
        }

        for (uint32_t i=menuQuery->startIndex; i<endIndex; i++)
        {
            level = cJSON_GetArrayItem(array, i);
            if (level != NULL)
            {
                if ((strlen(menuQuery->type) == strlen("name")) && (memcmp(menuQuery->type, "name", strlen("name")) == 0))
                {
                    item = cJSON_GetObjectItem(level, "name");
                    if ((item != NULL) && (item->type == cJSON_String))
                    {
                        index  = i * (MENU_STR_LEN_MAX + 1);
                        length = (strlen(item->valuestring) < MENU_STR_LEN_MAX) ? strlen(item->valuestring) : MENU_STR_LEN_MAX;
                        memset(&buffer[index], 0,                 MENU_STR_LEN_MAX + 1);
                        memcpy(&buffer[index], item->valuestring, length);
                    }
                    else
                    {
                        SYSLOG_ERR("Menu item error: %d\r\n", i);
                        goto labelEnd;
                    }
                }
                else if ((strlen(menuQuery->type) == strlen("id")) && (memcmp(menuQuery->type, "id", strlen("id")) == 0))
                {
                    item = cJSON_GetObjectItem(level, "id");
                    if ((item != NULL) && (item->type == cJSON_Number) && (sizeof(item->valueint) <= MENU_ID_LEN))
                    {
                        index  = i * MENU_ID_LEN;
                        memset(&buffer[index], 0,               MENU_ID_LEN);
                        memcpy(&buffer[index], &item->valueint, sizeof(item->valueint));
                        SYSLOG_ERR("item->valueint: %d\r\n", item->valueint);
                    }
                    else
                    {
                        SYSLOG_ERR("Menu item error: %d\r\n", i);
                        index  = i * MENU_ID_LEN;
                        memset(&buffer[index], 0,               MENU_ID_LEN);
                        // goto labelEnd;
                    }
                }
                else
                {
                    item = cJSON_GetObjectItem(level, "name");
                    if ((item != NULL) && (item->type == cJSON_String))
                    {
                        index  = i * (MENU_STR_LEN_MAX + 1 + MENU_ID_LEN);
                        length = (strlen(item->valuestring) < MENU_STR_LEN_MAX) ? strlen(item->valuestring) : MENU_STR_LEN_MAX;
                        memset(&buffer[index], 0,                 MENU_STR_LEN_MAX + 1);
                        memcpy(&buffer[index], item->valuestring, length);
                    }
                    else
                    {
                        SYSLOG_ERR("Menu item error: %d\r\n", i);
                        goto labelEnd;
                    }

                    item = cJSON_GetObjectItem(level, "id");
                    if ((item != NULL) && (item->type == cJSON_Number) && (sizeof(item->valueint) <= MENU_ID_LEN))
                    {
                        index  = i * (MENU_STR_LEN_MAX + 1 + MENU_ID_LEN) + (MENU_STR_LEN_MAX + 1);
                        memset(&buffer[index], 0,               MENU_ID_LEN);
                        memcpy(&buffer[index], &item->valueint, sizeof(item->valueint));
                    }
                    else
                    {
                        SYSLOG_ERR("Menu item error: %d\r\n", i);
                        goto labelEnd;
                    }
                }
            }
            else
            {
                SYSLOG_ERR("Menu item name error: %d\r\n", i);
                goto labelEnd;
            }
        }
    }

    retVal = count;

labelEnd:
    return retVal;
}

int32_t watchMenuQuery(char *fileName, MenuQueryT *menuQuery, void *buffer)
{
    int32_t   retVal      = -1;
    cJSON    *root        = NULL;
    cJSON    *array[2]    = {0};
    cJSON    *level[2]    = {0};
    uint32_t  level0Count = 0;
    uint32_t  level1Count = 0;

    if ((fileName == NULL) || (menuQuery == NULL) || (buffer == NULL))
    {
        SYSLOG_ERR("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    root = jsonGetRoot(fileName);
#endif
    if (root == NULL)
    {
        SYSLOG_ERR("Failed to get JSON root: %s\r\n", fileName);
        goto labelEnd;
    }

    if (menuQuery->level == 0)
    {
        retVal = queryArrayDetails(root, menuQuery, buffer);
        if (retVal < 0)
        {
            SYSLOG_ERR("Failed to query array details.\r\n");
            goto labelEnd;
        }
    }
    else if (menuQuery->level == 1)
    {
        array[0] = cJSON_GetObjectItem(root, "nextLevel");
        if ((array[0] != NULL) && (array[0]->type == cJSON_Array))
        {
            level0Count = cJSON_GetArraySize(array[0]);
            if (menuQuery->level0 >= level0Count)
            {
                SYSLOG_ERR("level0 must be less than level0Count: level0=%d, level0Count=%d\r\n", menuQuery->level0, level0Count);
                goto labelEnd;
            }

            level[0] = cJSON_GetArrayItem(array[0], menuQuery->level0);

            retVal = queryArrayDetails(level[0], menuQuery, buffer);
            if (retVal < 0)
            {
                SYSLOG_ERR("Failed to query array details.\r\n");
                goto labelEnd;
            }
        }
    }
    else if (menuQuery->level == 2)
    {
        array[0] = cJSON_GetObjectItem(root, "nextLevel");
        if ((array[0] != NULL) && (array[0]->type == cJSON_Array))
        {
            level0Count = cJSON_GetArraySize(array[0]);
            if (menuQuery->level0 >= level0Count)
            {
                SYSLOG_ERR("level0 must be less than level0Count: level0=%d, level0Count=%d\r\n", menuQuery->level0, level0Count);
                goto labelEnd;
            }

            level[0] = cJSON_GetArrayItem(array[0], menuQuery->level0);

            array[1] = cJSON_GetObjectItem(level[0], "nextLevel");

            if ((array[1] != NULL) && (array[1]->type == cJSON_Array))
            {
                level1Count = cJSON_GetArraySize(array[1]);
                if (menuQuery->level1 >= level1Count)
                {
                    SYSLOG_ERR("level1 must be less than level1Count: level1=%d, level1Count=%d\r\n", menuQuery->level1, level1Count);
                    goto labelEnd;
                }

                level[1] = cJSON_GetArrayItem(array[1], menuQuery->level1);

                retVal = queryArrayDetails(level[1], menuQuery, buffer);
                if (retVal < 0)
                {
                    SYSLOG_ERR("Failed to query array details.\r\n");
                    goto labelEnd;
                }
            }
        }
    }
    else
    {
        SYSLOG_ERR("level error.\r\n");
        goto labelEnd;
    }

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return retVal;
}

int32_t watchMenuFilePathSave(char *path)
{
    int32_t  retVal = -1;
    uint32_t index  = 0;
    uint32_t size   = sizeof(gWatchMenuFilePath) / sizeof(gWatchMenuFilePath[0]);

    if ((path != NULL) && ((strcmp(path, gWatchMenuFilePath[gWatchMenuConfig]) != 0)))
    {
        for (index=0; index<size; index++)
        {
            if (strcmp(path, gWatchMenuFilePath[index]) == 0)
            {
                break;
            }
        }

        if (index < size)
        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            retVal = iniKeyValueWrite(DEFAULT_INFO, INI_KEY_WATCH_MENU, INI_VALUE_INT, (void *)&index);
#endif
            if (retVal == 0)
            {
                gWatchMenuConfig = index;
            }
        }
    }

    return retVal;
}

char *watchMenuFilePathGet(void)
{
    uint32_t  watchMenuConfig = gWatchMenuConfig;
    uint32_t *value           = NULL;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    value = iniKeyValueRead(DEFAULT_INFO, INI_KEY_WATCH_MENU, INI_VALUE_INT);
#endif
    if (value != NULL)
    {
        watchMenuConfig = *value;
        free(value);
        value = NULL;
    }
    else
    {
        iniKeyValueWrite(DEFAULT_INFO, INI_KEY_WATCH_MENU, INI_VALUE_INT, (void *)&watchMenuConfig);
    }

    if ((watchMenuConfig >= 0) && (watchMenuConfig < (sizeof(gWatchMenuFilePath) / sizeof(gWatchMenuFilePath[0]))))
    {
        gWatchMenuConfig = watchMenuConfig;
    }

    return gWatchMenuFilePath[watchMenuConfig];
}

#ifdef WATCH_MENU_TEST
void watchMenuTest(char *fileName)
{
    MenuInfoT  menuInfo = {0};
    int32_t    count    = 0;
    char       buffer[(MENU_STR_LEN_MAX + 1 + MENU_ID_LEN) * 10] = {0};
    MenuQueryT menuQuery =
    {
        .level      = 0,
        .startIndex = 0,
        .count      = -1,
        .type       = "name"
    };
    MenuQueryT menuQueryId =
    {
        .level      = 0,
        .startIndex = 0,
        .count      = -1,
        .type       = "id"
    };
    MenuQueryT menuQueryNameId =
    {
        .level      = 0,
        .startIndex = 0,
        .count      = -1
    };
    MenuQueryT menuQuery1 =
    {
        .level      = 1,
        .level0     = 3,
        .startIndex = 0,
        .count      = -1,
        .type       = "name"
    };
    MenuQueryT menuQuery2 =
    {
        .level      = 2,
        .level0     = 3,
        .level1     = 1,
        .startIndex = 0,
        .count      = -1,
        .type       = "name"
    };

    if (fileName != NULL)
    {
        watchMenuGetInfo(fileName, &menuInfo);
        printf("totalLevel=%d\r\n", menuInfo.totalLevel);
        printf("level0Count=%d\r\n", menuInfo.level0Count);
        for (uint32_t i=0; i<10; i++)
        {
            printf("level1Count[%d]=%d\r\n", i, menuInfo.level1Count[i]);
            for (uint32_t j=0; j<10; j++)
            {
                printf("level2Count[%d][%d]=%d\r\n", i, j, menuInfo.level2Count[i][j]);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = watchMenuQuery(fileName, &menuQuery, buffer);
        if (count > 0)
        {
            printf("Leve1:\r\n");
            for (uint32_t i=0; i<count; i++)
            {
                printf("buffer(%d): %s\r\n", i, &buffer[i * (MENU_STR_LEN_MAX + 1)]);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = watchMenuQuery(fileName, &menuQueryId, buffer);
        if (count > 0)
        {
            uint32_t id = 0;
            if (sizeof(id) >= MENU_ID_LEN)
            {
                printf("Leve1(id):\r\n");
                for (uint32_t i=0; i<count; i++)
                {
                    id = 0;
                    memcpy(&id, &buffer[i * MENU_ID_LEN], MENU_ID_LEN);
                    printf("buffer(%d): %d\r\n", i, id);
                }
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = watchMenuQuery(fileName, &menuQueryNameId, buffer);
        if (count > 0)
        {
            uint32_t id = 0;
            if (sizeof(id) >= MENU_ID_LEN)
            {
                printf("Leve1(name&id):\r\n");
                for (uint32_t i=0; i<count; i++)
                {
                    id = 0;
                    memcpy(&id, &buffer[i * (MENU_STR_LEN_MAX + 1 + MENU_ID_LEN) + (MENU_STR_LEN_MAX + 1)], MENU_ID_LEN);
                    printf("buffer(%d): %s, %d\r\n", i, &buffer[i * (MENU_STR_LEN_MAX + 1 + MENU_ID_LEN)], id);
                }
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = watchMenuQuery(fileName, &menuQuery1, buffer);
        if (count > 0)
        {
            printf("Leve2:\r\n");
            for (uint32_t i=0; i<count; i++)
            {
                printf("buffer(%d): %s\r\n", i, &buffer[i * (MENU_STR_LEN_MAX + 1)]);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = watchMenuQuery(fileName, &menuQuery2, buffer);
        if (count > 0)
        {
            printf("Leve3:\r\n");
            for (uint32_t i=0; i<count; i++)
            {
                printf("buffer(%d): %s\r\n", i, &buffer[i * (MENU_STR_LEN_MAX + 1)]);
            }
        }
    }
}
#endif
