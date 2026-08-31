#ifdef FEATURE_SUBSYS_APP_MANAGER_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mem_map.h"
#include "cache.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_OPENSDK_ENABLE
#include "opensdk.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#include "appmanager.h"

#define APP_AUTO_RUN_DB         "D:/appAutoRunDb.json"
#define APP_AUTO_RUN_TPL        "D:/appAutoRunDb.type"

#define APP_INSTALL_RUN_DB      "appListDb.json" 
#define APP_INSTALL_RUN_TPL     "appListDb.type" 

#define APP_INSTALL_LIST_SIZE       32
#define APP_NAME_LEN_MAX            32
#define APP_PATH_LEN_MAX            32
#define APP_HEAD_LENGTH             sizeof(AppHeadT)


extern const uint32_t psyscall_index;
typedef void (*appMain)(uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);

typedef struct {
    uint32_t got_len;    
    uint32_t got_address; 
    uint32_t got_value;
    uint32_t sym_index;
} GotItemT;
typedef struct
{
    char     appName[8];
    uint32_t appSize;
    uint8_t  reserved0[4];  // Reserved for APP Type and Version
    uint8_t  reserved1[16]; // Reserved for Customer and CRC

    uint32_t textOffset;
    uint32_t textSize;
    uint32_t textRelocation;

    uint32_t dataOffset;
    uint32_t dataSize;
    uint32_t dataRelocation;

    uint32_t resourceOffset;
    uint32_t resourceSize;
    uint32_t resourceRelocation;

    uint8_t  reserved2[12]; // Reserved for 16-byte alignment
} AppHeadT;

typedef struct
{
    char     name[APP_NAME_LEN_MAX + 1];
    char     path[APP_PATH_LEN_MAX + 1];
    uint32_t address;
} AppInstallListT;

typedef struct _app_auto_run_db
{
    char name[8];
    char path[32];
    char ver[4];
} app_autorun_db;

typedef struct _app_install_db
{
    char name[8];
    char path[32];
    char ver[4];
} app_install_db;

static AppInstallListT gAppInstallList[APP_INSTALL_LIST_SIZE] = {0};


int32_t appInstall(char *name, uint32_t location)
{
    int32_t   retVal  = -1;
    char     *path    = (char *)location;

    if ((name == NULL) || (strlen(name) > APP_NAME_LEN_MAX))
    {
        SYSLOG_ERR("name error.\r\n");
        goto labEnd;
    }

    for (uint32_t i=0; i<APP_INSTALL_LIST_SIZE; i++)
    {
        if (strlen(gAppInstallList[i].name) == 0)
        {
            memcpy(gAppInstallList[i].name, name, strlen(name));
            if ((location & 0xFF800000) == AP_FLASH_XIP_ADDR)
            {
                SYSLOG_INFO("APP location: 0x%X\r\n", location);
                gAppInstallList[i].address = location;
            }
            else if ((path != NULL) && (strlen(path) <= APP_PATH_LEN_MAX) && (pathPrefixIsValid(path) == true))
            {
                SYSLOG_INFO("APP path: %s\r\n", path);
                memcpy(gAppInstallList[i].path, path, strlen(path));
            }
            else
            {
                SYSLOG_ERR("location error\r\n");
                goto labEnd;
            }
            break;
        }
    }

    retVal = 0;

labEnd:
    return retVal;
}

int32_t appUninstall(char *name)
{
    int32_t retVal = -1;

    if (name == NULL)
    {
        SYSLOG_ERR("name is NULL\r\n");
        goto labEnd;
    }

    for (uint32_t i=0; i<APP_INSTALL_LIST_SIZE; i++)
    {
        if ((strlen(name) == strlen(gAppInstallList[i].name)) && (memcmp(name, gAppInstallList[i].name, strlen(name)) == 0))
        {
            memset(&gAppInstallList[i], 0, sizeof(gAppInstallList[i]));
            retVal = 0;
            break;
        }
    }

labEnd:
    return retVal;
}

int32_t appRun(char *name)
{
    int32_t   retVal  = -1;
    FILE     *file    = NULL;
    AppHeadT  appHead = {0};

    SYSLOG_INFO("appRun\r\n");
    if (name == NULL)
    {
        SYSLOG_ERR("name is NULL\r\n");
        goto labEnd;
    }

    for (uint32_t i=0; i<APP_INSTALL_LIST_SIZE; i++)
    {
        if ((strlen(name) == strlen(gAppInstallList[i].name)) && (memcmp(name, gAppInstallList[i].name, strlen(name)) == 0))
        {
            if (gAppInstallList[i].address != 0) // ROM
            {
                memcpy(&appHead, (void *)gAppInstallList[i].address, sizeof(appHead));
                if (appHead.textRelocation != gAppInstallList[i].address + appHead.textOffset)
                {
                    memcpy((void *)appHead.textRelocation, (void *)(gAppInstallList[i].address + appHead.textOffset), appHead.textSize);
                }
            }
            else if (strlen(gAppInstallList[i].path) > 0) // FS
            {
                file = file_fopen(gAppInstallList[i].path, "r");
                if (file == NULL)
                {
                    printf("Failed to open the file %s.\r\n", gAppInstallList[i].path);
                    goto labEnd;
                }
                file_fread((void *)&appHead, sizeof(appHead), 1, file);
                memcpy((void *)appHead.textRelocation, (void *)appHead.textOffset, appHead.textSize);
            }
            else
            {
                SYSLOG_ERR("APP location error\r\n");
                goto labEnd;
            }

            SYSLOG_INFO("APP text relocation: 0x%X\r\n", appHead.textRelocation);
            ((appMain)(appHead.textRelocation + 1))((uint32_t)(&psyscall_index), appHead.textRelocation, 0, 0);
            retVal = 0;
            break;
        }
    }

labEnd:
    return retVal;
}

uint32_t ramspace[256*1024];
uint32_t got_addr = 0;

int32_t appRun_ram_run()
{
    uint32_t  address = 0;
    uint32_t  got = 0;
    address = (uint32_t)ramspace;
    got = got_addr;
    SYSLOG_INFO("ram address:0x%x\r\n",address);
    SYSLOG_INFO("got_addr:0x%x\r\n",got);
    DisableICache();
    EnableICache();
    ((appMain)(address + 1))((uint32_t)(&psyscall_index), address, got, 0);

    return 0;
}


int32_t appRun_ram(char *name)
{
    int32_t   retVal  = -1;
    uint32_t  address = 0;
    int32_t   got_num  = 0;
    uint32_t  got_address = 0;
    uint32_t  got_val = 0;
    uint32_t  got_len = 0;
    uint32_t  got_type = 0;

    FILE     *file    = NULL;
    AppHeadT  appHead = {0};

    SYSLOG_INFO("appRun_ram\r\n");
    SYSLOG_INFO("app name: %s\r\n", name);
    if (name == NULL)
    {
        SYSLOG_ERR("name is NULL\r\n");
        goto labEnd;
    }

    file = file_fopen(name, "r");
    if (file == NULL)
    {
        printf("Failed to open the file %s.\r\n", name);
        goto labEnd;
    }
    file_fread((void *)&appHead, 1,sizeof(appHead), file);
    SYSLOG_INFO("APP name: %s\r\n", appHead.appName);
    SYSLOG_INFO("APP appSize: %d\r\n", appHead.appSize);

    SYSLOG_INFO("APP text offset: 0x%X\r\n", appHead.textOffset);
    SYSLOG_INFO("APP text size: %d\r\n", appHead.textSize);
    SYSLOG_INFO("APP data offset: 0x%X\r\n", appHead.dataOffset);
    SYSLOG_INFO("APP data size: %d\r\n", appHead.dataSize);
    SYSLOG_INFO("APP data relocation: 0x%x\r\n", appHead.dataRelocation);

    got_num = appHead.dataSize/16;
    SYSLOG_INFO("got_num: %d\r\n", got_num);

    if(appHead.textSize > 0)
    {
        // address = malloc(appHead.textSize);
        address = (uint32_t)ramspace;
        if (address == 0)
        {
            SYSLOG_ERR("malloc error\r\n");
            goto labEnd;
        }else
        {
            file_fread((void *)address, appHead.textSize, 1, file);
            SYSLOG_INFO("APP text address: 0x%X\r\n", address);
            if(appHead.dataSize > 0)
            {
                file_fread((void *)(address+appHead.textSize), appHead.dataSize, 1, file);
                SYSLOG_INFO("APP data address: 0x%X\r\n", address+appHead.textSize);
            }
            if(got_num > 0)
            {
                got_addr = appHead.dataRelocation+address;
                for(int i=0;i<got_num;i++)
                {
                    got_len = *(uint32_t*)(address+appHead.textSize+16*i);
                    got_val = (address+appHead.textSize+16*i+4);
                    got_address = (address+appHead.textSize+16*i+8);
                    got_type = *(uint32_t*)(address+appHead.textSize+16*i+12);
                    SYSLOG_INFO("got_len: 0x%X\r\n", got_len);
                    SYSLOG_INFO("got_address: 0x%X\r\n", got_address);
                    SYSLOG_INFO("got_address data: 0x%X\r\n", *(uint32_t*)got_address);
                    SYSLOG_INFO("got_val: 0x%X\r\n", got_val);
                    SYSLOG_INFO("got_val data: 0x%X\r\n", *(uint32_t*)got_val);
                    SYSLOG_INFO("got_val data offset: 0x%X\r\n", address+*(uint32_t*)got_val);
                    SYSLOG_INFO("got_val data real data: 0x%X\r\n", *(uint32_t*)(address+*(uint32_t*)got_val));
                    SYSLOG_INFO("org data addr:0x%x\r\n",(address+*(uint32_t*)got_address))
                    SYSLOG_INFO("org data:0x%x\r\n",*(uint32_t*)(address+*(uint32_t*)got_address))
                    SYSLOG_INFO("got_type: %d\r\n", got_type);
                    if(got_type == 22)
                    {
                        SYSLOG_INFO("function\r\n");
                        *(uint32_t*)(address+*(uint32_t*)got_address) = (address+*(uint32_t*)got_val) | 1;
                        SYSLOG_INFO("change to data:%x\r\n",*(uint32_t*)(address+*(uint32_t*)got_address))
                    }
                    else
                    {
                        SYSLOG_INFO("data\r\n");
                        *(uint32_t*)(address+*(uint32_t*)got_address) = address+*(uint32_t*)got_val;
                        SYSLOG_INFO("change to data:%x\r\n",*(uint32_t*)(address+*(uint32_t*)got_address))
                    }
                    // *(uint32_t*)got_address = address+*(uint32_t*)got_address;
                    // *(uint32_t*)(address+*(uint32_t*)got_val) = address+*(uint32_t*)(address+*(uint32_t*)got_val);
                    
                }
            }
        }
    }
    SYSLOG_INFO("got_addr:0x%x\r\n",got_addr);
    DisableICache();
    EnableICache();
    // ((appMain)(address + 1))((uint32_t)(&psyscall_index), address, 0, 0);
    // free(address);
    retVal = 0;

labEnd:
    return retVal;
}

void app_install_add(char* path)
{
    app_install_db db = {0};
    AppHeadT  appHead = {0};
    FILE     *file    = NULL;

    memcpy(path,db.path,sizeof(db.path));
    file = file_fopen(path, "r");
    if (file == NULL)
    {
        printf("Failed to open the file %s.\r\n", path);
    }
    file_fread((void *)&appHead, sizeof(appHead), 1, file);
    memcpy(appHead.appName, &db.name, sizeof(appHead.appName));
    memcpy(appHead.reserved0, &db.ver, sizeof(appHead.reserved0));
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbAddItem(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL,&db);
#endif
}

void app_install_del(int index)
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbDeleteItem(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL,index);
#endif
}

void app_install_query(int index,app_install_db *db)
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL,index, (void **)&db);
#endif
}

void app_install_list()
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    app_install_db db ={0};
    //read auto run list
    int count = jsonDbGetTotalCount(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL);
    for (size_t i = 0; i < count; i++)
    {
            if ((jsonDbGetItemById(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL, i, (void **)&db) == 1))
            {
                SYSLOG_INFO("app auto run (%d): %s: %s\r\n", i, db.name, db.path);
                // run apps
            }
    }
#endif
}


void app_auto_run_add(char* path)
{
    app_autorun_db db = {0};
    AppHeadT  appHead = {0};
    FILE     *file    = NULL;

    memcpy(path,db.path,sizeof(db.path));
    file = file_fopen(path, "r");
    if (file == NULL)
    {
        printf("Failed to open the file %s.\r\n", path);
    }
    file_fread((void *)&appHead, sizeof(appHead), 1, file);
    memcpy(appHead.appName, &db.name, sizeof(appHead.appName));
    memcpy(appHead.reserved0, &db.ver, sizeof(appHead.reserved0));
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbAddItem(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL,&db);
#endif
}

void app_auto_run_del(int index)
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbDeleteItem(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL,index);
#endif
}

void app_auto_run_query(int index,app_autorun_db *db)
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbGetItemById(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL,index, (void **)&db);
#endif
}

void app_auto_run_list()
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    app_autorun_db db ={0};
    //read auto run list
    int count = jsonDbGetTotalCount(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL);
    for (size_t i = 0; i < count; i++)
    {
            if ((jsonDbGetItemById(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL, i, (void **)&db) == 1))
            {
                SYSLOG_INFO("app auto run (%d): %s: %s\r\n", i, db.name, db.path);
                // run apps
            }
    }
#endif
}

void app_auto_run()
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    app_autorun_db db ={0};
    //read auto run list
    int count = jsonDbGetTotalCount(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL);
    for (size_t i = 0; i < count; i++)
    {
            if ((jsonDbGetItemById(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL, i, (void **)&db) == 1))
            {
                SYSLOG_INFO("app auto run (%d): %s: %s\r\n", i, db.name, db.path);
                // run apps
                appRun(db.path);
            }
    }
#endif
}

void appManagerRunAutoApp(void)
{
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    app_autorun_db *db = NULL;

    if ((jsonDbGetItemById(APP_AUTO_RUN_DB, APP_AUTO_RUN_TPL, 0, (void **)&db) == 1))
    {
        if (appRun_ram(db->path) == 0)
        {
            SYSLOG_INFO("Run Auto APP 0: name=%s, patn=%s, ver=%s\r\n", db->name, db->path, db->ver);
            appRun_ram_run();
        }
    }

    if (db != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(db);
#else
        free(db);
#endif
        db = NULL;
    }
#endif
}





#define INSTALLED_APP_JSON          "D:/installedApp.json"
#define INSTALLED_APP_TYPE          "D:/installedApp.type"
#define APP_SIZE_MAX                10240
#define APP_THREAD_COUNT_MAX        32
#define APP_LOAD_COUNT_MAX          10


typedef enum
{
    ALL_APP = 0,
    AUTO_START_APP,
    RUNNING_APP,
    APP_TYPE_MAX
} AppTypeT;

typedef struct
{
    char name[APP_NAME_LENGTH_MAX];
    char path[APP_PATH_LENGTH_MAX];
    char version[APP_VERSION_LENGTH_MAX];
    char autoStart[APP_AUTO_START_LENGTH_MAX];
} AppPackageT;

typedef struct
{
    char     name[APP_NAME_LENGTH_MAX];
    bool     autoStart;
    bool     running;
    uint32_t address[APP_SIZE_MAX];
    uint32_t got;
    uint32_t threadId[APP_THREAD_COUNT_MAX];
} AppLoadT;


AppLoadT appLoad[APP_LOAD_COUNT_MAX] = {0};


static int32_t startApp(uint32_t index)
{
    int32_t retVal = -1;

    if (index >= APP_LOAD_COUNT_MAX)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    if (appLoad[index].running != true)
    {
        SYSLOG_DEBUG("Start app: name=%s, address=0x%X\r\n", appLoad[index].name, (uint32_t)appLoad[index].address);
        ((appMain)((uint32_t)appLoad[index].address + 1))((uint32_t)(&psyscall_index), (uint32_t)appLoad[index].address, appLoad[index].got, (uint32_t)appLoad[index].threadId);
        appLoad[index].running = true;
    }

    retVal = 0;

labelEnd:
    return retVal;
}

static int32_t loadApp(AppPackageT *appPackage)
{
    int32_t   retVal     = -1;
    int32_t   index      = -1;
    uint32_t  address    = 0;
    FILE     *file       = NULL;
    AppHeadT  appHead    = {0};
    uint32_t  gotCount   = 0;
    uint32_t  gotLength  = 0;
    uint32_t  gotValue   = 0;
    uint32_t  gotAddress = 0;
    uint32_t  gotType    = 0;

    if (appPackage == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<APP_LOAD_COUNT_MAX; i++)
    {
        if (strcmp(appLoad[i].name, appPackage->name) == 0)
        {
            retVal = 0;
            goto labelEnd;
        }
        if ((index == -1) && (strlen(appLoad[i].name) == 0))
        {
            index = i;
        }
    }

    if (index == -1)
    {
        SYSLOG_DEBUG("Exceeded the maximum loading quantity.\r\n");
        goto labelEnd;
    }

    address = (uint32_t)appLoad[index].address;
    memcpy(appLoad[index].name, appPackage->name, MIN(APP_NAME_LENGTH_MAX - 1, strlen(appPackage->name)));
    appLoad[index].autoStart = (appPackage->autoStart[0] == '0') ? false : true;
    // SYSLOG_DEBUG("name=%s, path=%s, version=%s, autoStart=%s\r\n", appPackage->name, appPackage->path, appPackage->version, appPackage->autoStart);
    SYSLOG_DEBUG("[%d]: name=%s, autoStart=%d, address=0x%X\r\n", index, appLoad[index].name, appLoad[index].autoStart, address);

    file = file_fopen(appPackage->path, "r");
    if (file == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file %s\r\n" , appPackage->path);
        goto labelEnd;
    }

    if (file_fread((void *)&appHead, 1, sizeof(appHead), file) != sizeof(appHead))
    {
        SYSLOG_DEBUG("Failed to read the file %s\r\n", appPackage->path);
        goto labelEnd;
    }

    if (file_fread((void *)address, 1, appHead.textSize, file) != appHead.textSize)
    {
        SYSLOG_DEBUG("Failed to read the file %s\r\n", appPackage->path);
        goto labelEnd;
    }

    if (appHead.dataSize > 0)
    {
        if (file_fread((void *)(address + appHead.textSize), 1, appHead.dataSize, file) != appHead.dataSize)
        {
            SYSLOG_DEBUG("Failed to read the file %s\r\n", appPackage->path);
            goto labelEnd;
        }
    }

    appLoad[index].got = appHead.dataRelocation + address;
    gotCount           = appHead.dataSize / 16;

    for (uint32_t i=0; i<gotCount; i++)
    {
        gotLength  = gotLength;
        gotLength  = *(uint32_t *)(address + appHead.textSize + 16 * i);
        gotValue   =               address + appHead.textSize + 16 * i + 4;
        gotAddress =               address + appHead.textSize + 16 * i + 8;
        gotType    = *(uint32_t *)(address + appHead.textSize + 16 * i + 12);
        if (gotType == 22)
        {
            *(uint32_t *)(address + *(uint32_t *)gotAddress) = (address + *(uint32_t *)gotValue) | 1;
        }
        else
        {
            *(uint32_t *)(address + *(uint32_t *)gotAddress) = address + *(uint32_t *)gotValue;
        }
    }

    retVal = index;

labelEnd:
    if (file != NULL)
    {
        file_fclose(file);
        file = NULL;
    }
    return retVal;
}

static int32_t getAppList(AppPackageT **appPackage)
{
    int32_t retVal = -1;
    int32_t count  = 0;

    if (appPackage == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    count = jsonDbGetAllItems(INSTALLED_APP_JSON, INSTALLED_APP_TYPE, (void **)appPackage);
    if ((count <= 0) || (*appPackage == NULL))
    {
        goto labelEnd;
    }
#endif

    retVal = count;

labelEnd:
    return retVal;
}

static int32_t startAutoApp(void)
{
    int32_t      retVal     = 0;
    AppPackageT *appPackage = NULL;
    int32_t      count      = -1;
    int32_t      index      = -1;

    count = getAppList(&appPackage);
    if ((count <= 0) || (appPackage == NULL))
    {
        SYSLOG_DEBUG("There are no installed apps.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<count; i++)
    {
        if (appPackage[i].autoStart[0] != '0')
        {
            index = loadApp(&appPackage[i]);
            if (index < 0)
            {
                SYSLOG_DEBUG("Failed to load the %s app.\r\n", appPackage[i].name);
                continue;
            }

            if (startApp(index) != 0)
            {
                SYSLOG_DEBUG("Failed to start the %s app.\r\n", appPackage[i].name);
                continue;
            }

            retVal++;
        }
    }

labelEnd:
    if (appPackage != NULL)
    {
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
        jsonDbFree((void **)&appPackage);
#endif
    }
    return retVal;
}

int32_t appManagerInit(void)
{
    memset(appLoad, 0, sizeof(appLoad));

    if (startAutoApp() == 0)
    {
        SYSLOG_DEBUG("There are no auto-start apps.\r\n");
    }

    return 0;
}

static int32_t getAppPackage(char *name, AppPackageT *appPackage)
{
    int32_t      retVal  = -1;
    int32_t      count   = -1;
    AppPackageT *package = NULL;

    if ((name == NULL) || (appPackage == NULL))
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    count = getAppList(&package);
    if ((count <= 0) || (package == NULL))
    {
        goto labelEnd;
    }

    for (uint32_t i=0; i<count; i++)
    {
        if (strcmp(package[i].name, name) == 0)
        {
            memcpy(appPackage, &package[i], sizeof(AppPackageT));
            retVal = i;
            goto labelEnd;
        }
    }

labelEnd:
    if (package != NULL)
    {
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
        jsonDbFree((void **)&package);
#endif
    }
    return retVal;
}

static int32_t getAppLoadIndex(char *name)
{
    int32_t retVal = -1;

    if (name == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<APP_LOAD_COUNT_MAX; i++)
    {
        if (strcmp(appLoad[i].name, name) == 0)
        {
            retVal = i;
            goto labelEnd;
        }
    }

labelEnd:
    return retVal;
}

int32_t appManagerStartApp(char *name)
{
    int32_t     retVal     = -1;
    int32_t     index      = -1;
    AppPackageT appPackage = {0};

    if (name == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    index = getAppLoadIndex(name);
    if (index < 0)
    {
        if (getAppPackage(name, &appPackage) < 0)
        {
            SYSLOG_DEBUG("The %s app is not installed.\r\n", name);
            goto labelEnd;
        }

        index = loadApp(&appPackage);
        if (index < 0)
        {
            SYSLOG_DEBUG("Failed to load the %s app.\r\n", name);
            goto labelEnd;
        }
    }

    if (appLoad[index].running == true)
    {
        SYSLOG_DEBUG("The %s app is already running.\r\n", name);
        goto labelEnd;
    }

    retVal = startApp(index);
    if (retVal != 0)
    {
        SYSLOG_DEBUG("Failed to start the %s app.\r\n", name);
        goto labelEnd;
    }

labelEnd:
    return retVal;
}

static int32_t stopApp(uint32_t index)
{
    int32_t retVal = -1;

    if (index >= APP_LOAD_COUNT_MAX)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    if (appLoad[index].running == true)
    {
        for (uint32_t i=0; i<APP_THREAD_COUNT_MAX; i++)
        {
            if (appLoad[index].threadId[i] != 0)
            {
                if ((osThreadTerminate((osThreadId_t)appLoad[index].threadId[i]) != osOK)
                 && (osThreadGetState( (osThreadId_t)appLoad[index].threadId[i]) != osThreadTerminated)
                 && (osThreadTerminate((osThreadId_t)appLoad[index].threadId[i]) != osOK))
                {
                    SYSLOG_DEBUG("Failed to stop the %s thread.\r\n", osThreadGetName((osThreadId_t)appLoad[index].threadId[i]));
                    goto labelEnd;

                }
                appLoad[index].threadId[i] = 0;
            }
        }
        appLoad[index].running = false;
    }

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t appManagerStopApp(char *name)
{
    int32_t retVal = -1;
    int32_t index  = -1;

    if (name == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    index = getAppLoadIndex(name);
    if (index < 0)
    {
        SYSLOG_DEBUG("The %s app is not run.\r\n", name);
        goto labelEnd;
    }

    if (appLoad[index].running != true)
    {
        SYSLOG_DEBUG("The %s app is not running.\r\n", name);
        goto labelEnd;
    }

    if (stopApp(index) != 0)
    {
        SYSLOG_DEBUG("Failed to stop the %s app.\r\n", name);
        goto labelEnd;
    }

labelEnd:
    return retVal;
}

int32_t appManagerInstallApp(char *path, bool autoStart)
{
    int32_t      retVal     = -1;
    FILE        *file       = NULL;
    AppHeadT     appHead    = {0};
    AppPackageT  appPackage = {0};

    if (path == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    file = file_fopen(path, "r");
    if (file == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file %s\r\n" , path);
        goto labelEnd;
    }

    if (file_fread((void *)&appHead, 1, sizeof(appHead), file) != sizeof(appHead))
    {
        SYSLOG_DEBUG("Failed to read the file %s\r\n", path);
        goto labelEnd;
    }

    if (getAppPackage(appHead.appName, &appPackage) >= 0)
    {
        SYSLOG_DEBUG("The %s app is already installed.\r\n", appHead.appName);
        goto labelEnd;
    }

    memset(&appPackage, 0, sizeof(appPackage));
    memcpy(appPackage.name,    appHead.appName,   MIN(APP_NAME_LENGTH_MAX - 1,    strlen(appHead.appName)));
    memcpy(appPackage.path,    path,              MIN(APP_PATH_LENGTH_MAX - 1,    strlen(path)));
    memcpy(appPackage.version, appHead.reserved0, MIN(APP_VERSION_LENGTH_MAX - 1, strlen((char *)appHead.reserved0)));
    appPackage.autoStart[0] = (autoStart == false) ? '0' : '1';

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbAddItem(INSTALLED_APP_JSON, INSTALLED_APP_TYPE, (void *)&appPackage);
    if (retVal != 0)
    {
        SYSLOG_DEBUG("Failed to install the %s app.\r\n", path);
        goto labelEnd;
    }
#endif

    // SYSLOG_DEBUG("name=%s, path=%s, version=%s, autoStart=%d\r\n", appHead.appName, path, appHead.reserved0, autoStart);
    SYSLOG_DEBUG("name=%s, path=%s, version=%s, autoStart=%s\r\n", appPackage.name, appPackage.path, appPackage.version, appPackage.autoStart);

labelEnd:
    if (file != NULL)
    {
        file_fclose(file);
        file = NULL;
    }
    return retVal;
}

int32_t appManagerUninstallApp(char *name)
{
    int32_t     retVal     = -1;
    int32_t     index      = -1;
    AppPackageT appPackage = {0};

    if (name == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    index = getAppLoadIndex(name);
    if (index >= 0)
    {
        if (stopApp(index) != 0)
        {
            SYSLOG_DEBUG("Failed to stop the %s app.\r\n", name);
            goto labelEnd;
        }
        memset(&appLoad[index], 0, sizeof(AppLoadT));
    }

    index = getAppPackage(name, &appPackage);
    if ((index < 0))
    {
        SYSLOG_DEBUG("The %s app is not installed.\r\n", name);
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbDeleteItem(INSTALLED_APP_JSON, INSTALLED_APP_TYPE, index);
    if (retVal != 0)
    {
        SYSLOG_DEBUG("Failed to uninstall the %s app.\r\n", name);
        goto labelEnd;
    }
#endif

    SYSLOG_DEBUG("name=%s, path=%s, version=%s, autoStart=%s\r\n", appPackage.name, appPackage.path, appPackage.version, appPackage.autoStart);

labelEnd:
    return retVal;
}

int32_t appManagerSetAppAutoStart(char *name, bool autoStart)
{
    int32_t     retVal     = -1;
    int32_t     index      = -1;
    AppPackageT appPackage = {0};

    if (name == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    index = getAppPackage(name, &appPackage);
    if (index < 0)
    {
        SYSLOG_DEBUG("The %s app is not installed.\r\n", name);
        goto labelEnd;
    }

    if (((autoStart == false) && (appPackage.autoStart[0] == '0')) || ((autoStart == true) && (appPackage.autoStart[0] == '1')))
    {
        SYSLOG_DEBUG("The %s app is already %sauto-start.\r\n", name, (autoStart == false) ? "non-" : "");
        retVal = 0;
        goto labelEnd;
    }

    appPackage.autoStart[0] = (autoStart == false) ? '0' : '1';

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbUpdateItem(INSTALLED_APP_JSON, INSTALLED_APP_TYPE, index, (void *)&appPackage);
    if (retVal != 0)
    {
        SYSLOG_DEBUG("Failed to set the auto-start status of the %s app.\r\n", name);
        goto labelEnd;
    }
#endif

    SYSLOG_DEBUG("name=%s, path=%s, version=%s, autoStart=%s\r\n", appPackage.name, appPackage.path, appPackage.version, appPackage.autoStart);

labelEnd:
    return retVal;
}

int32_t appManagerGetAppByName(char *name, AppDetailT *appDetail)
{
    int32_t     retVal     = -1;
    int32_t     index      = -1;
    AppPackageT appPackage = {0};

    if ((name == NULL) || (appDetail == NULL))
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    if (getAppPackage(name, &appPackage) < 0)
    {
        SYSLOG_DEBUG("No app named %s\r\n", name);
        goto labelEnd;
    }

    memset(appDetail, 0, sizeof(AppDetailT));
    memcpy(appDetail->name,    appPackage.name,    MIN(APP_NAME_LENGTH_MAX - 1,    strlen(appPackage.name)));
    memcpy(appDetail->path,    appPackage.path,    MIN(APP_PATH_LENGTH_MAX - 1,    strlen(appPackage.path)));
    memcpy(appDetail->version, appPackage.version, MIN(APP_VERSION_LENGTH_MAX - 1, strlen(appPackage.version)));
    appDetail->autoStart = (appPackage.autoStart[0] == '0') ? false : true;

    index = getAppLoadIndex(appDetail->name);
    if (index >= 0)
    {
        appDetail->running = appLoad[index].running;
    }

    SYSLOG_DEBUG("name=%s, path=%s, version=%s, autoStart=%d, running=%d\r\n", appDetail->name, appDetail->path, appDetail->version, appDetail->autoStart, appDetail->running);

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t appManagerGetAppsByType(uint8_t type, AppDetailT **appDetail)
{
    int32_t      retVal     = 0;
    AppPackageT *appPackage = NULL;
    AppDetailT  *detail     = NULL;
    int32_t      count      = -1;
    int32_t      index      = -1;
    uint32_t     j          = 0;

    if ((type >= APP_TYPE_MAX) || (appDetail == NULL))
    {
        retVal = -1;
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    count = getAppList(&appPackage);
    if ((count <= 0) || (appPackage == NULL))
    {
        SYSLOG_DEBUG("There are no installed apps.\r\n");
        goto labelEnd;
    }

    detail = malloc(sizeof(AppDetailT) * count);
    if (detail == NULL)
    {
        retVal = -1;
        SYSLOG_DEBUG("Failed to malloc %s bytes for detail.\r\n");
        goto labelEnd;
    }

    memset(detail, 0, sizeof(AppDetailT) * count);

    SYSLOG_DEBUG("type=%d\r\n", type);
    switch (type)
    {
        case ALL_APP:
            for (uint32_t i=0; i<count; i++)
            {
                memcpy(detail[i].name,    appPackage[i].name,    MIN(APP_NAME_LENGTH_MAX - 1,    strlen(appPackage[i].name)));
                memcpy(detail[i].path,    appPackage[i].path,    MIN(APP_PATH_LENGTH_MAX - 1,    strlen(appPackage[i].path)));
                memcpy(detail[i].version, appPackage[i].version, MIN(APP_VERSION_LENGTH_MAX - 1, strlen(appPackage[i].version)));
                detail[i].autoStart = (appPackage[i].autoStart[0] == '0') ? false : true;

                index = getAppLoadIndex(detail[i].name);
                if (index >= 0)
                {
                    detail[i].running = appLoad[index].running;
                }
                SYSLOG_DEBUG("[%d]: name=%s, path=%s, version=%s, autoStart=%d, running=%d\r\n", i, detail[i].name, detail[i].path, detail[i].version, detail[i].autoStart, detail[i].running);
            }
            retVal = count;
            break;

        case AUTO_START_APP:
            j = 0;
            for (uint32_t i=0; i<count; i++)
            {
                if (appPackage[i].autoStart[0] != '0')
                {
                    memcpy(detail[j].name,    appPackage[i].name,    MIN(APP_NAME_LENGTH_MAX - 1,    strlen(appPackage[i].name)));
                    memcpy(detail[j].path,    appPackage[i].path,    MIN(APP_PATH_LENGTH_MAX - 1,    strlen(appPackage[i].path)));
                    memcpy(detail[j].version, appPackage[i].version, MIN(APP_VERSION_LENGTH_MAX - 1, strlen(appPackage[i].version)));
                    detail[j].autoStart = true;

                    index = getAppLoadIndex(detail[j].name);
                    if (index >= 0)
                    {
                        detail[j].running = appLoad[index].running;
                    }

                    SYSLOG_DEBUG("[%d]: name=%s, path=%s, version=%s, autoStart=%d, running=%d\r\n", j, detail[j].name, detail[j].path, detail[j].version, detail[j].autoStart, detail[j].running);
                    j++;
                }
            }
            retVal = j;
            break;

        case RUNNING_APP:
            j = 0;
            for (uint32_t i=0; i<count; i++)
            {
                index = getAppLoadIndex(appPackage[i].name);
                if (index >= 0)
                {
                    memcpy(detail[j].name,    appPackage[i].name,    MIN(APP_NAME_LENGTH_MAX - 1,    strlen(appPackage[i].name)));
                    memcpy(detail[j].path,    appPackage[i].path,    MIN(APP_PATH_LENGTH_MAX - 1,    strlen(appPackage[i].path)));
                    memcpy(detail[j].version, appPackage[i].version, MIN(APP_VERSION_LENGTH_MAX - 1, strlen(appPackage[i].version)));
                    detail[j].autoStart = (appPackage[i].autoStart[0] == '0') ? false : true;
                    detail[j].running   = true;

                    SYSLOG_DEBUG("[%d]: name=%s, path=%s, version=%s, autoStart=%d, running=%d\r\n", j, detail[j].name, detail[j].path, detail[j].version, detail[j].autoStart, detail[j].running);
                    j++;
                }
            }
            retVal = j;
            break;

        default:
            break;
    }

    if (retVal == 0)
    {
        SYSLOG_DEBUG("There are no apps of the specified type.\r\n");
        if (detail != NULL)
        {
            free(detail);
            detail = NULL;
        }
    }

    *appDetail = detail;

labelEnd:
    if (appPackage != NULL)
    {
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
        jsonDbFree((void **)&appPackage);
#endif
    }
    return retVal;
}

#endif
