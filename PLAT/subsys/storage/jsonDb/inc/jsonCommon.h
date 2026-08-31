#ifndef __JSON_COMMON_H__
#define __JSON_COMMON_H__


#include <stdint.h>
#include "cJSON.h"


#define JSON_PREFIX_STR         "{\"name\""
#define JSON_POSTFIX_STR        "\r"
#ifndef MIN
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#endif


char  *jsonGetString(char *buffer);
cJSON *jsonGetRoot(char *filename);


#endif
