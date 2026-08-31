#ifndef __WATCH_MENU_H__
#define __WATCH_MENU_H__


#include <stdint.h>
#include "cJSON.h"


#define MENU_STR_LEN_MAX            32
#define MENU_ID_LEN                 4


typedef struct
{
    char name[MENU_STR_LEN_MAX + 1];
} MenuNameT;

typedef struct
{
    int8_t totalLevel;
    int8_t level0Count;
    int8_t level1Count[10];
    int8_t level2Count[10][10];
} MenuInfoT;

typedef struct
{
    int8_t level;
    int8_t level0;
    int8_t level1;
    int8_t startIndex;
    int8_t count;
    char   type[8]; // name:只返回name, id:只返回id, 否则:返回name和id
} MenuQueryT;


#define FIRST_MENU_COUNT_MAX        12
#define SECOND_MENU_COUNT_MAX       7
#define THIRD_MENU_COUNT_MAX        1
#define MENU_NAME_LENGTH_MAX        60
#define IMGGE_PATH_LENGTH_MAX       35


extern uint32_t gFirstMenuCount;
extern uint32_t gFirstMenuId[FIRST_MENU_COUNT_MAX];
extern char     gFirstMenuName[FIRST_MENU_COUNT_MAX][MENU_NAME_LENGTH_MAX];
extern char     gFirstMenuImage[FIRST_MENU_COUNT_MAX][IMGGE_PATH_LENGTH_MAX];

extern uint32_t gSecondMenuCount[FIRST_MENU_COUNT_MAX];
extern uint32_t gSecondMenuId[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX];
extern char     gSecondMenuName[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][MENU_NAME_LENGTH_MAX];
extern char     gSecondMenuImage[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][IMGGE_PATH_LENGTH_MAX];

extern uint32_t gThirdMenuCount[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX];
extern uint32_t gThirdMenuId[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][THIRD_MENU_COUNT_MAX];
extern char     gThirdMenuName[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][THIRD_MENU_COUNT_MAX][MENU_NAME_LENGTH_MAX];
// extern char     gThirdMenuImage[FIRST_MENU_COUNT_MAX][SECOND_MENU_COUNT_MAX][THIRD_MENU_COUNT_MAX][IMGGE_PATH_LENGTH_MAX];

extern char    *gWatchMenuFilePath[];

int32_t watchMenuRead(char *filename);
int32_t getNextLevelMenu(cJSON *root, MenuNameT **menu);
int32_t watchMenuGetInfo(char *fileName, MenuInfoT *menuInfo);
int32_t watchMenuQuery(char *fileName, MenuQueryT *menuQuery, void *buffer);
int32_t watchMenuFilePathSave(char *path);
char   *watchMenuFilePathGet(void);


#endif
