#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__


#include <stdint.h>
#include <stdbool.h>


int32_t appRun(char *name);
int32_t appInstall(char *name, uint32_t location);


#define APP_NAME_LENGTH_MAX         8
#define APP_PATH_LENGTH_MAX         32
#define APP_VERSION_LENGTH_MAX      4
#define APP_AUTO_START_LENGTH_MAX   2


typedef struct
{
    char name[APP_NAME_LENGTH_MAX];
    char path[APP_PATH_LENGTH_MAX];
    char version[APP_VERSION_LENGTH_MAX];
    bool autoStart;
    bool running;
} AppDetailT;


int32_t appManagerInit(void);
int32_t appManagerStartApp(char *name);
int32_t appManagerStopApp(char *name);
int32_t appManagerInstallApp(char *path, bool autoStart);
int32_t appManagerUninstallApp(char *name);
int32_t appManagerSetAppAutoStart(char *name, bool autoStart);
int32_t appManagerGetAppByName(char *name, AppDetailT *appDetail);
int32_t appManagerGetAppsByType(uint8_t type, AppDetailT **appDetail);

#endif
