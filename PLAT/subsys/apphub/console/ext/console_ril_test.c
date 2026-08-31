/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_ril_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-12-4
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "string.h"
#include "at_api.h"

extern int skip_atoi(const char **s);

static int32_t atRespCallback(UINT8 chanId, const CHAR *pStr, UINT32 strLen, void *pArg)
{
    printf("%s\r\n", pStr);
    return 0;
}

extern const char *getConsolePrompt(void);
int cmd_ril(int argc, char **argv)
{

    char *at_cmd = argv[1];
    // ril AT+CREG?
    if (argc == 2)
    {
        atRilAtCmdReq(at_cmd, strlen(at_cmd), atRespCallback, NULL, 0);
    }
    else
    {
        rt_kprintlnf("Usage: ril [atcmd]\r\n");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_ril, ril, ril test);

#endif // FEATURE_SUBSYS_CONSOLE_ENABLE