#include <string.h>
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "charge.h"
#ifdef FEATURE_DRIVER_KEYPAD_ENABLE
#include "keypad.h"
#include "kpc.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_MEDIA_ENABLE
#include "media.h"
#endif
#include "bsp_custom.h"
#include DEBUG_LOG_HEADER_FILE
#include "plat_config.h"
#ifdef FEATURE_SUBSYS_APPHUB_ENABLE
#include "app.h"
#include "apphub.h"
#endif

#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#include "feature.h"

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
#include "volumeManager.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#include "cJSON.h"
#include "phoneMenu.h"
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "devicemanager.h"
#include "api_scr.h"
#endif

#define CIPHER_SYSLOG_LEVEL_LENGTH  8
#define CIPHER_SYSLOG_LEVEL_PREFIX  "*#123"
#define CIPHER_SYSLOG_LEVEL_SUFFIX  "#*"


MenuInfoT  feature_menuInfo = {0};
char feature_lv0_menu_buf[10][MENU_STR_LEN_MAX+1] = {0};
char feature_lv1_menu_buf[10][MENU_STR_LEN_MAX+1] = {0};
char feature_lv2_menu_buf[10][MENU_STR_LEN_MAX+1] = {0};

int32_t feature_lv0_func_id_buf[10] = {0};
int32_t feature_lv1_func_id_buf[10] = {0};
int32_t feature_lv2_func_id_buf[10] = {0};

extern sub_func sub_func_table[];
extern int max_sub_func_num;

void add_db_item(int index_id,void *data)
{
    char pathModel[40] = {0};
    char pathType[40]  = {0};

    SYSLOG_DEBUG("add_db_item \r\n");
    SYSLOG_DEBUG("index_id %d\r\n",index_id);
    SYSLOG_DEBUG("data %s\r\n",data);
    SYSLOG_DEBUG("max_sub_func_num: %d\r\n",max_sub_func_num);

    int func_id = -1;
    for(int i=0;i<max_sub_func_num;i++)
    {
        if(sub_func_table[i].index==index_id)
        {
            func_id = i;
            break;
        }
    }
    SYSLOG_DEBUG("func_id %d\r\n",func_id);

    if(func_id<0)
    {
        SYSLOG_DEBUG("func_id error\r\n");
        return;
    }

    memset(pathModel, 0, sizeof(pathModel));
    snprintf(pathModel, sizeof(pathModel), "D:/%s.json", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathModel: %s\r\n",pathModel);

    memset(pathType, 0, sizeof(pathType));
    snprintf(pathType, sizeof(pathType), "D:/%s.type", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathType: %s\r\n",pathType);

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbAddItem(pathModel, pathType, data);
#endif
}

void del_db_item(int index_id,int index)
{
    SYSLOG_DEBUG("del_db_item \r\n");
    SYSLOG_DEBUG("index_id %d\r\n",index_id);
    SYSLOG_DEBUG("index %d\r\n",index);
    SYSLOG_DEBUG("max_sub_func_num: %d\r\n",max_sub_func_num);

    char pathModel[40] = {0};
    char pathType[40]  = {0};

    int func_id = -1;
    for(int i=0;i<max_sub_func_num;i++)
    {
        if(sub_func_table[i].index==index_id)
        {
            func_id = i;
            break;
        }
    }
    
    SYSLOG_DEBUG("func_id %d\r\n",func_id);
    if(func_id<0)
    {
        SYSLOG_DEBUG("func_id error\r\n");
        return;
    }

    memset(pathModel, 0, sizeof(pathModel));
    snprintf(pathModel, sizeof(pathModel), "D:/%s.json", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathModel: %s\r\n",pathModel);

    memset(pathType, 0, sizeof(pathType));
    snprintf(pathType, sizeof(pathType), "D:/%s.type", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathType: %s\r\n",pathType);

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbDeleteItem(pathModel, pathType, index);
#endif
}

void get_db_item(int index_id,int index,char **data)
{
    SYSLOG_DEBUG("get_db_item \r\n");
    SYSLOG_DEBUG("index_id %d\r\n",index_id);
    SYSLOG_DEBUG("index %d\r\n",index);
    SYSLOG_DEBUG("max_sub_func_num: %d\r\n",max_sub_func_num);

    char pathModel[40] = {0};
    char pathType[40]  = {0};

    int func_id = -1;
    for(int i=0;i<max_sub_func_num;i++)
    {
        if(sub_func_table[i].index==index_id)
        {
            func_id = i;
            break;
        }
    }
    
    SYSLOG_DEBUG("func_id %d\r\n",func_id);
    if(func_id<0)
    {
        SYSLOG_DEBUG("func_id error\r\n");
        return;
    }

    memset(pathModel, 0, sizeof(pathModel));
    snprintf(pathModel, sizeof(pathModel), "D:/%s.json", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathModel: %s\r\n",pathModel);

    memset(pathType, 0, sizeof(pathType));
    snprintf(pathType, sizeof(pathType), "D:/%s.type", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathType: %s\r\n",pathType);

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    uint32_t size = jsonDbGetTotalCount(pathModel, pathType);
    uint32_t length = 0;
    uint32_t len    = 0;
    char *name = NULL;
    uint32_t typeSize = jsonDbGetTypeNumber(pathType);
    if(index<size && index>=0)
    {
        length = 0;
        // jsonDbGetItemById(pathModel, pathType, index, &data);
        if(jsonDbGetItemById(pathModel, pathType, index, (void **)data)==1 && (*data != NULL))
        {
            for (uint32_t i=0; i<typeSize; i++)
            {
                len = jsonDbGetTypeName(pathType, i, &name);
                SYSLOG_DEBUG("Type len:%d\r\n", len);
                SYSLOG_DEBUG("name:%s\r\n", name);
                SYSLOG_DEBUG("data:%s\r\n", &((*data)[length]));
                length += len;
                if (name != NULL)
                {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                    vPortFree_Psram(name);
#else
                    free(name);
#endif
                    name = NULL;
                }
            }
            SYSLOG_DEBUG("\r\n");
        }

    }
#endif
}

int get_db_item_length(int index_id)
{
    SYSLOG_DEBUG("get_db_item_length \r\n");
    SYSLOG_DEBUG("index_id %d\r\n",index_id);
    SYSLOG_DEBUG("max_sub_func_num: %d\r\n",max_sub_func_num);

    char pathModel[40] = {0};
    char pathType[40]  = {0};

    int func_id = -1;
    for(int i=0;i<max_sub_func_num;i++)
    {
        if(sub_func_table[i].index==index_id)
        {
            func_id = i;
            break;
        }
    }
    
    SYSLOG_DEBUG("func_id %d\r\n",func_id);
    if(func_id<0)
    {
        SYSLOG_DEBUG("func_id error\r\n");
        return 0;
    }

    memset(pathModel, 0, sizeof(pathModel));
    snprintf(pathModel, sizeof(pathModel), "D:/%s.json", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathModel: %s\r\n",pathModel);

    memset(pathType, 0, sizeof(pathType));
    snprintf(pathType, sizeof(pathType), "D:/%s.type", sub_func_table[func_id].name);
    SYSLOG_DEBUG("pathType: %s\r\n",pathType);

    uint32_t size = 0;
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    size = jsonDbGetTotalCount(pathModel, pathType);
#endif
    SYSLOG_DEBUG("total count = %d\r\n", size);

    return size;
}

int32_t cipherHandler(char *cipher)
{
    int32_t retVal = -1;
    uint8_t level  = 0;

    if ((cipher == NULL) || (strlen(cipher) != CIPHER_SYSLOG_LEVEL_LENGTH))
    {
        goto labelEnd;
    }

    level = cipher[strlen(CIPHER_SYSLOG_LEVEL_PREFIX)] - '0';
    if ((memcmp(cipher, CIPHER_SYSLOG_LEVEL_PREFIX, strlen(CIPHER_SYSLOG_LEVEL_PREFIX)) == 0)
     && (level >= SL_DEBUG)
     && (level <= SL_NO)
     && (memcmp(&cipher[strlen(CIPHER_SYSLOG_LEVEL_PREFIX) + 1], CIPHER_SYSLOG_LEVEL_SUFFIX, strlen(CIPHER_SYSLOG_LEVEL_SUFFIX)) == 0))
    {
        SYSLOG_EMERG("Set syslog level: %d (cipher: %s).\r\n", level, cipher);
        syslogSetLevel(level);
        SYSLOG_EMERG("Set syslog level: %d (cipher: %s).\r\n", level, cipher);
    }

    retVal = 0;

labelEnd:
    return retVal;
}


void feature_menu_loader(char *fileName)
{
    MenuQueryT menuQueue =
    {
        .level      = 0,
        .startIndex = 0,
        .count      = -1,
        .type       = "name"
    };
    memset(&feature_menuInfo, 0, sizeof(feature_menuInfo));
    phoneMenuGetInfo(fileName, &feature_menuInfo);
#if 0
    SYSLOG_DEBUG("totalLevel=%d\r\n", feature_menuInfo.totalLevel);
    SYSLOG_DEBUG("level0Count=%d\r\n", feature_menuInfo.level0Count);
    for (uint32_t i=0; i<10; i++)
    {
        SYSLOG_DEBUG("level1Count[%d]=%d\r\n", i, feature_menuInfo.level1Count[i]);
        for (uint32_t j=0; j<10; j++)
        {
            SYSLOG_DEBUG("level2Count[%d][%d]=%d\r\n", i, j, feature_menuInfo.level2Count[i][j]);
        }
    }
#endif

    memset(feature_lv0_menu_buf, 0, sizeof(feature_lv0_menu_buf));
    int count = phoneMenuQuery(fileName, &menuQueue, feature_lv0_menu_buf);
    if (count > 0)
    {
#if 0
        SYSLOG_DEBUG("Leve1:\r\n");
        for (uint32_t i=0; i<count; i++)
        {
            SYSLOG_DEBUG("feature_lv0_menu_buf(%d): %s\r\n", i, &feature_lv0_menu_buf[i][0]);
        }
#endif
    }

    memset(menuQueue.type, 0, sizeof(menuQueue.type));
    strcpy(menuQueue.type, "id");
    SYSLOG_DEBUG("menuQueue.type: %s\r\n",menuQueue.type);
    int id_count = phoneMenuQuery(fileName, &menuQueue, feature_lv0_func_id_buf);
    if (id_count > 0)
    {
#if 0
        SYSLOG_DEBUG("Leve1 id:\r\n");
        for (uint32_t i=0; i<id_count; i++)
        {
            SYSLOG_DEBUG("feature_lv0_func_id_buf(%d): %d\r\n", i, feature_lv0_func_id_buf[i]);
        }
#endif
    }
}

static uint8_t gWallPaper = 0;
uint8_t getCurrentWallPaper(void)
{
    return gWallPaper;
}

void loadWallPaper(void)
{
    char *wallPaper = NULL;

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById("D:/displayDb.json", "D:/displayDb.type", 0, (void **)&wallPaper);
#endif
    if (wallPaper != NULL)
    {
        gWallPaper = atoi(&wallPaper[40]);
        SYSLOG_DEBUG("wallPaper: %s, %s, %d\r\n", wallPaper, &wallPaper[40], gWallPaper);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(wallPaper);
#else
        free(wallPaper);
#endif
        wallPaper = NULL;
    }
}

void saveWallPaper(char *wallPaper)
{
    gWallPaper = atoi(&wallPaper[40]);
    SYSLOG_DEBUG("wallPaper: %s, %s, %d\r\n", wallPaper, &wallPaper[40], gWallPaper);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbUpdateItem("D:/displayDb.json", "D:/displayDb.type", 0, wallPaper);
#endif
}

static uint8_t       gBacklight = 3;
static const uint8_t gBacklightList[6] = {15, 30, 45, 60, 75, 100};
uint8_t getCurrentBacklight(void)
{
    return gBacklightList[gBacklight];
}

void loadBacklight(void)
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    char *backlight = NULL;

    jsonDbGetItemById("D:/displayDb.json", "D:/displayDb.type", 1, (void **)&backlight);
    if (backlight != NULL)
    {
        gBacklight = atoi(&backlight[40]);
        SYSLOG_DEBUG("backlight: %s=%s\r\n", backlight, &backlight[40]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(backlight);
#else
        free(backlight);
#endif
        backlight = NULL;
    }
    SYSLOG_DEBUG("backlight: [%d]=%d\r\n", gBacklight, gBacklightList[gBacklight]);
    Device_ioctl("dev:/lcd", OPEN_SCREEN_BACKLIGHT_SET, (void *)&gBacklightList[gBacklight]);
#endif
}

void saveBacklight(char *backlight)
{
    gBacklight = atoi(&backlight[40]);
    SYSLOG_DEBUG("backlight: %s=%s, [%d]=%d\r\n", backlight, &backlight[40], gBacklight, gBacklightList[gBacklight]);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbUpdateItem("D:/displayDb.json", "D:/displayDb.type", 1, backlight);
#endif
    Device_ioctl("dev:/lcd", OPEN_SCREEN_BACKLIGHT_SET, (void *)&gBacklightList[gBacklight]);
}


static const char *gRingFiles[2] = {"D:/s1imy05_imy.mp3", "D:/s1imy06_imy.mp3"};
void loadRingSource(void)
{
    uint8_t  index      = 0;
    char    *ringSource = NULL;

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById("D:/profileDb.json", "D:/profileDb.type", 0, (void **)&ringSource);
#endif
    if (ringSource != NULL)
    {
        index = atoi(&ringSource[40]);
        SYSLOG_DEBUG("ringSource: %s=%s\r\n", ringSource, &ringSource[40]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(ringSource);
#else
        free(ringSource);
#endif
        ringSource = NULL;
    }

    SYSLOG_DEBUG("ringSource: gRingFiles[%d]=%d\r\n", index, gRingFiles[index]);
    audioToneSrcSet(gRingFiles[index]);
}

void saveRingSource(char *ringSource)
{
    uint8_t index = atoi(&ringSource[40]);
    SYSLOG_DEBUG("ringSource: %s=%s, gRingFiles[%d]=%s\r\n", ringSource, &ringSource[40], index, gRingFiles[index]);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbUpdateItem("D:/profileDb.json", "D:/profileDb.type", 0, ringSource);
#endif
    audioToneSrcSet(gRingFiles[index]);
}

static uint8_t volumeLevelToPercentage(uint8_t level)
{
    uint8_t volume[] = {0, 30, 40, 50, 55, 59, 63, 67, 71, 75};

    if (level >= sizeof(volume))
    {
        level = sizeof(volume) - 1;
    }

    return volume[level];
}

void loadRingVolume(void)
{
    uint8_t  index      = 0;
    char    *ringVolume = NULL;

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById("D:/profileDb.json", "D:/profileDb.type", 1, (void **)&ringVolume);
#endif
    if (ringVolume != NULL)
    {
        index = atoi(&ringVolume[40]);
        SYSLOG_DEBUG("ringVolume: %s=%s\r\n", ringVolume, &ringVolume[40]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(ringVolume);
#else
        free(ringVolume);
#endif
        ringVolume = NULL;
    }

#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerSet("ring", volumeLevelToPercentage(index));
#endif
}

void saveRingVolume(char *ringVolume)
{
    uint8_t index = atoi(&ringVolume[40]);
    SYSLOG_DEBUG("ringVolume: %s=%s\r\n", ringVolume, &ringVolume[40]);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbUpdateItem("D:/profileDb.json", "D:/profileDb.type", 1, ringVolume);
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerSet("ring", volumeLevelToPercentage(index));
#endif
}

void loadCallVolume(void)
{
    uint8_t  index      = 0;
    char    *callVolume = NULL;

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById("D:/profileDb.json", "D:/profileDb.type", 2, (void **)&callVolume);
#endif
    if (callVolume != NULL)
    {
        index = atoi(&callVolume[40]);
        SYSLOG_DEBUG("callVolume: %s, %s, %d\r\n", callVolume, &callVolume[40], index);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(callVolume);
#else
        free(callVolume);
#endif
        callVolume = NULL;
    }

#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerSet("call", volumeLevelToPercentage(index));
#endif
}

void saveCallVolume(char *callVolume)
{
    uint8_t index = atoi(&callVolume[40]);
    SYSLOG_DEBUG("callVolume: %s, %s, %d\r\n", callVolume, &callVolume[40], index);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbUpdateItem("D:/profileDb.json", "D:/profileDb.type", 2, callVolume);
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerSet("call", volumeLevelToPercentage(index));
#endif
}

uint8_t adjustCallVolume(bool plus)
{
    uint8_t  index      = 5;
    char    *callVolume = NULL;

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById("D:/profileDb.json", "D:/profileDb.type", 2, (void **)&callVolume);
#endif
    if (callVolume != NULL)
    {
        index = atoi(&callVolume[40]);
        SYSLOG_DEBUG("callVolume(old): %s, %s, %d\r\n", callVolume, &callVolume[40], index);
    }

    if (plus == true)
    {
        if (index < CALL_VOLUME_INDEX_MAX)
        {
            index++;
        }
    }
    else
    {
        if (index > 0)
        {
            index--;
        }
    }
    memset(&callVolume[40], 0, 40);
    snprintf(&callVolume[40], 40, "%d", index);
    SYSLOG_DEBUG("callVolume(new): %s, %s, %d\r\n", callVolume, &callVolume[40], index);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbUpdateItem("D:/profileDb.json", "D:/profileDb.type", 2, callVolume);
#endif

#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerSet("call", volumeLevelToPercentage(index));
#endif

    if (callVolume != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(callVolume);
#else
        free(callVolume);
#endif
        callVolume = NULL;
    }

    return index;
}
