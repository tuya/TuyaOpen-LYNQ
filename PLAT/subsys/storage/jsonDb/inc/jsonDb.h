#ifndef __JSON_DB_H__
#define __JSON_DB_H__


#include <stdint.h>
#include "jsonCommon.h"


int32_t jsonDbAddItem(char *fileName, char *typeFileName, void *item);
int32_t jsonDbDeleteItem(char *fileName, char *typeFileName, uint32_t id);
int32_t jsonDbUpdateItem(char *fileName, char *typeFileName, uint32_t id, void *itemNew);
int32_t jsonDbGetTotalCount(char *fileName, char *typeFileName);
int32_t jsonDbGetAllItems(char *fileName, char *typeFileName, void **items);
int32_t jsonDbGetItemById(char *fileName, char *typeFileName, uint32_t id, void **item);
int32_t jsonDbGetMatchedItemsByStr(char *fileName, char *typeFileName, char *matchStr, void **items, uint32_t **ids);
int32_t jsonDbGetTypeNumber(char *typeFileName);
int32_t jsonDbGetTypeName(char *typeFileName, uint32_t id, char **name);
void    jsonDbFree(void **object);

#endif
