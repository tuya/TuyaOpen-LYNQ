#ifdef FEATURE_SUBSYS_APP_MANAGER_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mem_map.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_OPENSDK_ENABLE
#include "opensdk.h"
#endif

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif

#define APP_AUTO_RUN_DB         "appAutoRunDb.json" 
#define APP_AUTO_RUN_TPL        "appAutoRunDb.type" 

#define APP_INSTALL_RUN_DB      "appListDb.json" 
#define APP_INSTALL_RUN_TPL     "appListDb.type" 

#define APP_INSTALL_LIST_SIZE       32
#define APP_NAME_LEN_MAX            32
#define APP_PATH_LEN_MAX            32
#define APP_HEAD_LENGTH             sizeof(AppHeadT)


extern const uint32_t psyscall_index;
typedef void (*appMain)(uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);

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
    uint32_t  address = 0;
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
                memcpy(&appHead, gAppInstallList[i].address, sizeof(appHead));
                if (appHead.textRelocation != gAppInstallList[i].address + appHead.textOffset)
                {
                    memcpy(appHead.textRelocation, gAppInstallList[i].address + appHead.textOffset, appHead.textSize);
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
                memcpy(appHead.textRelocation, appHead.textOffset, appHead.textSize);
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

void app_install_add(char* path)
{
    app_install_db db = {0};
    AppHeadT  appHead = {0};
    FILE     *file    = NULL;

    memcpy(path,&db.path,sizeof(path));
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
    jsonDbGetItemById(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL,index,&db);
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
            if ((jsonDbGetItemById(APP_INSTALL_RUN_DB,APP_INSTALL_RUN_TPL, i, &db) == 1))
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

    memcpy(path,&db.path,sizeof(path));
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
    jsonDbGetItemById(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL,index,&db);
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
            if ((jsonDbGetItemById(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL, i, &db) == 1))
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
            if ((jsonDbGetItemById(APP_AUTO_RUN_DB,APP_AUTO_RUN_TPL, i, &db) == 1))
            {
                SYSLOG_INFO("app auto run (%d): %s: %s\r\n", i, db.name, db.path);
                // run apps
                appRun(db.path);
            }
    }
#endif
}

#endif
