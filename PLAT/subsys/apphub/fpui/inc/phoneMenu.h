#ifndef __PHONE_MENU_H__
#define __PHONE_MENU_H__


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


int32_t phoneMenuRead(char *filename);
int32_t getNextLevelMenu(cJSON *root, MenuNameT **menu);
int32_t phoneMenuGetInfo(char *fileName, MenuInfoT *menuInfo);
int32_t phoneMenuQuery(char *fileName, MenuQueryT *menuQuery, void *buffer);


#endif
