#ifndef  __FEATURE_H__
#define  __FEATURE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "ps_sms_if.h"


#define RING_VOLUME_INDEX_MAX   9
#define CALL_VOLUME_INDEX_MAX   9


typedef int32_t (*sub_func_init)(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
typedef int32_t (*sub_func_proc)(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);

typedef struct __sub_func_t
{
	int32_t index;
	char    name[32];
    sub_func_init init;
    sub_func_proc proc;
} sub_func;

typedef struct _call_list_db
{
    char number[40];
    char time[40];
    char flag;
} call_list_db;

typedef struct _contact_db
{
    char number[40];
    char name[40];
} contact_db;

typedef struct _display_db
{
    char name[40];
    char value[40];
} display_db;

typedef struct _profile_db
{
    char name[40];
    char value[40];
} profile_db;

typedef struct _tts_db
{
    char name[40];
    char value[40];
} tts_db;

typedef struct _presetSms_db
{
    char data[40];
} presetSms_db;

typedef struct _smsInbox_db
{
    char number[40];
    char time[40];
    char data[PSIL_SMS_MAX_TXT_SIZE + 1];
} smsInbox_db;

typedef struct _smsOutbox_db
{
    char number[40];
    char time[40];
    char data[40];
} smsOutbox_db;

int32_t cipherHandler(char *cipher);
uint8_t getCurrentWallPaper(void);
void loadWallPaper(void);
void saveWallPaper(char *wallPaper);
uint8_t getCurrentBacklight(void);
void loadBacklight(void);
void saveBacklight(char *backlight);
void loadCallVolume(void);
void saveCallVolume(char *speakerVolume);
uint8_t adjustCallVolume(bool plus);
void loadRingVolume(void);
void saveRingVolume(char *ringVolume);
void loadRingSource(void);
void saveRingSource(char *ringSource);
int get_db_item_length(int index_id);
void get_db_item(int index_id,int index,char **data);
void feature_menu_loader(char *fileName);

#ifdef __cplusplus
}
#endif
#endif 