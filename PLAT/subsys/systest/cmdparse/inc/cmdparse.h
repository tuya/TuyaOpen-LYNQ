#ifndef __CMDPARSE_H__
#define __CMDPARSE_H__


#include <stdint.h>



#define CMDPARSE_PREFIX_STR         "{\"name\""
#define CMDPARSE_POSTFIX_STR        "\r"
#define CMDPARSE_NAME_LEN_MAX       100


typedef struct
{
    char    name[CMDPARSE_NAME_LEN_MAX + 1];
    int32_t param1;
    int32_t param2;
    char    *param3;
} CmdParseResultT;


void    cmdparseInit(void);
int32_t cmdparseQueuePut(void *data, uint32_t length);
int32_t cmdParseResultGet(CmdParseResultT *result, uint32_t timeout);


#endif
