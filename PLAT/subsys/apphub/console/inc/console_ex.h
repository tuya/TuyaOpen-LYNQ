
#ifndef __CONSOLE_EX_H__
#define __CONSOLE_EX_H__
#ifdef __cplusplus
extern "C" {
#endif

#define MSH_USING_HISTORY
#define MSH_CMD_LENMAX          128
#define MSH_HISTORY_NUM         5

enum CharStatT
{
    WAIT_NORM_CHAR,
    WAIT_SPEC_CHAR,
    WAIT_FUNC_CHAR,
};

struct console_shell
{
    enum CharStatT stat;
    uint8_t echoMode: 1;
    uint8_t promptMode: 1;
    char line[MSH_CMD_LENMAX + 1];
    uint16_t linePosition;
    uint16_t lineCurpos;
#ifdef MSH_USING_HISTORY
    uint16_t currentHistory;
    uint16_t historyCount;
    char cmdHistory[MSH_HISTORY_NUM][MSH_CMD_LENMAX];
#endif
};

void mshPullHistory(struct console_shell *shell);
void mshPushHistory(struct console_shell *shell);
int consoleExInit(void);
int consoleExDeinit(void);
int consoleChar2Cmd(int ch, char *cmd);

void consoleCharReset(void);
const char *getConsolePrompt(void);
#ifdef __cplusplus
}
#endif
#endif
