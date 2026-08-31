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
#include "phoneMenu.h"


// #define PHONE_MENU_DEBUG


char menu_name[10][16] = {0};
int menu_max_num = 0;
int menu_func[10] = {0};
char menu_image[10][30] = {0};

char next_menu_name[10][15][16] = {0};
int next_menu_func[10][15] = {0};
int next_menu_max_num[10] = {0};

static int32_t jsonStringParse(char *json)
{
    int32_t  retVal = -1;
    cJSON    *root  = NULL;
#ifdef JSON_DB_TEST
    cJSON    *node  = NULL;
#endif

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

    memset(menu_name, 0, sizeof(menu_name));
    memset(menu_image, 0, sizeof(menu_image));
    memset(next_menu_name, 0, sizeof(next_menu_name));
    cJSON *menu_json = cJSON_GetArrayItem(root, 1);
    menu_max_num = cJSON_GetArraySize(menu_json);
#ifdef PHONE_MENU_DEBUG
    SYSLOG_INFO("array_items: %d\r\n", menu_max_num);
#endif
    for(int i=0;i<menu_max_num;i++)
    {
        cJSON *item = cJSON_GetArrayItem(menu_json, i);
#ifdef PHONE_MENU_DEBUG
        SYSLOG_INFO("array_items %d,name: %s\r\n",i, cJSON_Print(item));
#endif
        cJSON *name =cJSON_GetObjectItem(item,"name");
        cJSON *image =cJSON_GetObjectItem(item,"image");
        cJSON *func_id =cJSON_GetObjectItem(item,"id");
        cJSON *next =cJSON_GetObjectItem(item,"nextLevel");

#ifdef PHONE_MENU_DEBUG
        SYSLOG_INFO("array_items %d, name: %s, image: %s, func: %s\r\n",i, cJSON_Print(name), cJSON_Print(image), cJSON_Print(func_id));
#endif
        memcpy(menu_name[i], cJSON_Print(name)+1, strlen(cJSON_Print(name))-2);
        memcpy(menu_image[i], cJSON_Print(image)+1, strlen(cJSON_Print(image))-2);
        menu_func[i] = atoi(cJSON_Print(func_id));
        if(next != NULL)
        {
            int next_num = cJSON_GetArraySize(next);
            next_menu_max_num[i] = next_num;
#ifdef PHONE_MENU_DEBUG
            SYSLOG_INFO("next_num %d\r\n",next_num);
#endif
            for (int j = 0; j < next_num; j++)
            {
                cJSON *next_item = cJSON_GetArrayItem(next, j);
                cJSON *next_name =cJSON_GetObjectItem(next_item,"name");
                cJSON *next_id =cJSON_GetObjectItem(next_item,"id");

#ifdef PHONE_MENU_DEBUG
                SYSLOG_INFO("next_item %d,next_name: %s next_id:%s\r\n",j, cJSON_Print(next_name),cJSON_Print(next_id));
#endif
                memcpy(next_menu_name[i][j], cJSON_Print(next_name)+1, strlen(cJSON_Print(next_name))-2);
                next_menu_func[i][j] = atoi(cJSON_Print(next_id));
            }
            
        }


    }

#ifdef JSON_DB_TEST
    node = cJSON_GetObjectItem(root, "name");
    if ((node != NULL) && (node->type == cJSON_String)
     && (strlen(node->valuestring) == strlen("手机菜单"))
     && (memcmp(node->valuestring, "手机菜单", strlen("手机菜单")) == 0))
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

int32_t phoneMenuRead(char *filename)
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

int32_t phoneMenuGetInfo(char *fileName, MenuInfoT *menuInfo)
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
                        memset((uint8_t *)buffer + index, 0,                 MENU_STR_LEN_MAX + 1);
                        memcpy((uint8_t *)buffer + index, item->valuestring, length);
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
                        memset((uint8_t *)buffer + index, 0,               MENU_ID_LEN);
                        memcpy((uint8_t *)buffer + index, &item->valueint, sizeof(item->valueint));
                        SYSLOG_ERR("item->valueint: %d\r\n", item->valueint);
                    }
                    else
                    {
                        SYSLOG_ERR("Menu item error: %d\r\n", i);
                        index  = i * MENU_ID_LEN;
                        memset((uint8_t *)buffer + index, 0,               MENU_ID_LEN);
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
                        memset((uint8_t *)buffer + index, 0,                 MENU_STR_LEN_MAX + 1);
                        memcpy((uint8_t *)buffer + index, item->valuestring, length);
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
                        memset((uint8_t *)buffer + index, 0,               MENU_ID_LEN);
                        memcpy((uint8_t *)buffer + index, &item->valueint, sizeof(item->valueint));
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

int32_t phoneMenuQuery(char *fileName, MenuQueryT *menuQuery, void *buffer)
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

#ifdef PHONE_MENU_TEST
void phoneMenuTest(char *fileName)
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
        phoneMenuGetInfo(fileName, &menuInfo);
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
        count = phoneMenuQuery(fileName, &menuQuery, buffer);
        if (count > 0)
        {
            printf("Leve1:\r\n");
            for (uint32_t i=0; i<count; i++)
            {
                printf("buffer(%d): %s\r\n", i, &buffer[i * (MENU_STR_LEN_MAX + 1)]);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = phoneMenuQuery(fileName, &menuQueryId, buffer);
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
        count = phoneMenuQuery(fileName, &menuQueryNameId, buffer);
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
        count = phoneMenuQuery(fileName, &menuQuery1, buffer);
        if (count > 0)
        {
            printf("Leve2:\r\n");
            for (uint32_t i=0; i<count; i++)
            {
                printf("buffer(%d): %s\r\n", i, &buffer[i * (MENU_STR_LEN_MAX + 1)]);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        count = phoneMenuQuery(fileName, &menuQuery2, buffer);
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
