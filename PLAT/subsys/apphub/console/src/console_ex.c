/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718 
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "time.h"
#include "storage.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"
#include "console.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#include "msh.h"

struct console_shell *gMsh = NULL;

extern char *getConsoleDirPath();

const char *getConsolePrompt(void)
{
    if (getConsoleMode())
        return "\033[32m>>>";
    else
    {
        // if(getConsoleDir()== DISK_C)
        //     return "C:\\>";
        // else if(getConsoleDir()== DISK_D)
        //     return "D:\\>";
        // else if(getConsoleDir()== DISK_E)
        //     return "E:\\>";
        // char *result = (char *)malloc(50);
        // snprintf(result, sizeof(result), "%s/%s", getConsoleDirPath(), ">");
        // return result;
        return (const char *)getConsoleDirPath();
    }
    return "\033[34m";
}

#ifdef MSH_USING_HISTORY
void mshPullHistory(struct console_shell *shell)
{
    rt_kprintf("\033[2K\r");
    rt_kprintf("%s%s", getConsolePrompt(), gMsh->line);
}

void mshPushHistory(struct console_shell *shell)
{
    if (gMsh->linePosition != 0){
        /* push history */
        if (gMsh->historyCount >= MSH_HISTORY_NUM)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&gMsh->cmdHistory[MSH_HISTORY_NUM - 1], gMsh->line, MSH_CMD_LENMAX))
            {
                /* move history */
                int index;
                for (index = 0; index < MSH_HISTORY_NUM - 1; index ++)
                {
                    memcpy(&gMsh->cmdHistory[index][0],
                           &gMsh->cmdHistory[index + 1][0], MSH_CMD_LENMAX);
                }
                memset(&gMsh->cmdHistory[index][0], 0, MSH_CMD_LENMAX);
                memcpy(&gMsh->cmdHistory[index][0], gMsh->line, gMsh->linePosition);
                /* it's the maximum history */
                gMsh->historyCount = MSH_HISTORY_NUM;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (gMsh->historyCount == 0 || memcmp(&gMsh->cmdHistory[gMsh->historyCount - 1], gMsh->line, MSH_CMD_LENMAX))
            {
                gMsh->currentHistory = gMsh->historyCount;
                memset(&gMsh->cmdHistory[gMsh->historyCount][0], 0, MSH_CMD_LENMAX);
                memcpy(&gMsh->cmdHistory[gMsh->historyCount][0], gMsh->line, gMsh->linePosition);
                /* increase count and set current history position */
                gMsh->historyCount ++;
            }
        }
    }
    gMsh->currentHistory = gMsh->historyCount;
}
#endif
// * up key  : 0x1b 0x5b 0x41
// * down key: 0x1b 0x5b 0x42
// * right key:0x1b 0x5b 0x43
// * left key: 0x1b 0x5b 0x44

int consoleChar2Cmd(int ch,char* cmd)
{
    int len = 0;
    if (ch < 0) return 0;
    if(gMsh == NULL){
        rt_kprintf("msh not inited\n\r");
        return 0;
    } 
    if (ch == 0x1b){
        gMsh->stat = WAIT_SPEC_CHAR;
        return 0;
    }
    else if (gMsh->stat == WAIT_SPEC_CHAR){
        if (ch == 0x5b){
            gMsh->stat = WAIT_FUNC_CHAR;  
            return 0; 
        }
        gMsh->stat = WAIT_NORM_CHAR; 
    }
    else if (gMsh->stat == WAIT_FUNC_CHAR){
        gMsh->stat = WAIT_NORM_CHAR;
        if (ch == 0x41) /* up key */
        {
            #ifdef MSH_USING_HISTORY
            if (gMsh->currentHistory > 0)
                gMsh->currentHistory --;
            else{
                gMsh->currentHistory = 0;
                return 0;
            }
            /* copy the history command */
            memcpy(gMsh->line, &gMsh->cmdHistory[gMsh->currentHistory][0], MSH_CMD_LENMAX);
            gMsh->lineCurpos = gMsh->linePosition = (uint16_t)strlen(gMsh->line);
            mshPullHistory(gMsh);
            #endif
            return 0;
        }
        else if (ch == 0x42) /* down key */
        {
            #ifdef MSH_USING_HISTORY
            /* next history */
            if (gMsh->currentHistory < gMsh->historyCount - 1)
                gMsh->currentHistory ++;
            else{
                /* set to the end of history */
                if (gMsh->historyCount != 0)
                    gMsh->currentHistory = gMsh->historyCount - 1;
                else
                    return 0;
            }
            memcpy(gMsh->line, &gMsh->cmdHistory[gMsh->currentHistory][0], MSH_CMD_LENMAX);
            gMsh->lineCurpos = gMsh->linePosition = (uint16_t)strlen(gMsh->line);
            mshPullHistory(gMsh);
            #endif
            return 0;
        }
        else if (ch == 0x44) /* left key */
        {
            if (gMsh->lineCurpos){
                rt_kprintf("\b");
                gMsh->lineCurpos --;
            }
            return 0;
        }
        else if (ch == 0x43) /* right key */
        {
            if (gMsh->lineCurpos < gMsh->linePosition){
                rt_kprintf("%c", gMsh->line[gMsh->lineCurpos]);
                gMsh->lineCurpos ++;
            }
            return 0;
        }
        return 0;
    }
    /* received null or error */
    if (ch == '\0' || ch == 0xFF) return 0;
    /* handle tab key */
    else if(ch == '\t'){
        /* move the cursor to the beginning of line */
        for (int i = 0; i < gMsh->lineCurpos; i++) rt_kprintf("\b");
        /* auto complete */
        msh_auto_complete(&gMsh->line[0]);
        /* re-calculate position */
        gMsh->lineCurpos = gMsh->linePosition = (uint16_t)strlen(gMsh->line);
        return 0;
    }
    /* handle backspace key */
    else if (ch == 0x7f || ch == 0x08){
        /* note that gMsh->lineCurpos >= 0 */
        if (gMsh->lineCurpos == 0) return 0;
        gMsh->linePosition--;
        gMsh->lineCurpos--;
        if (gMsh->linePosition > gMsh->lineCurpos){
            int i;
            memmove(&gMsh->line[gMsh->lineCurpos],
                        &gMsh->line[gMsh->lineCurpos + 1],
                        gMsh->linePosition - gMsh->lineCurpos);
            gMsh->line[gMsh->linePosition] = 0;
            rt_kprintf("\b%s  \b", &gMsh->line[gMsh->lineCurpos]);
            /* move the cursor to the origin position */
            for (i = gMsh->lineCurpos; i <= gMsh->linePosition; i++)
                rt_kprintf("\b");
        }
        else{
            rt_kprintf("\b \b");
            gMsh->line[gMsh->linePosition] = 0;
        }
        return 0;
    }
    /* handle end of line, break */
    if (ch == '\r' || ch == '\n'){
        if (gMsh->linePosition == 0){
            rt_kprintf("\n\r%s >",getConsolePrompt());
            return 0;
        }
        #ifdef MSH_USING_HISTORY
        mshPushHistory(gMsh);
        #endif
        if(gMsh->echoMode) rt_kprintf("\n\r%s >",getConsolePrompt());
        memcpy(cmd, gMsh->line, gMsh->linePosition);
        memset(gMsh->line, 0, sizeof(gMsh->line));
        len = gMsh->lineCurpos-1;
        gMsh->lineCurpos = gMsh->linePosition = 0;
        // return (gMsh->lineCurpos-1);
        return len;
    }
    /* normal character */
    if (gMsh->lineCurpos < gMsh->linePosition){
        int i;
        memmove(&gMsh->line[gMsh->lineCurpos + 1],
                    &gMsh->line[gMsh->lineCurpos],
                    gMsh->linePosition - gMsh->lineCurpos);
        gMsh->line[gMsh->lineCurpos] = ch;
        if (gMsh->echoMode)
            rt_kprintf("%s", &gMsh->line[gMsh->lineCurpos]);

        /* move the cursor to new position */
        for (i = gMsh->lineCurpos; i < gMsh->linePosition; i++)
            rt_kprintf("\b");
    }
    else{
        gMsh->line[gMsh->linePosition] = ch;
        if (gMsh->echoMode)
            rt_kprintf("%c", ch);
    }
    gMsh->linePosition ++;
    gMsh->lineCurpos++;
    if (gMsh->linePosition >= MSH_CMD_LENMAX){
        /* clear command line */
        gMsh->linePosition = 0;
        gMsh->lineCurpos = 0;
    }
    return 0;
} 
 
void consoleCharReset(void)
{
    if (gMsh != NULL){
        gMsh->linePosition = 0;
        gMsh->lineCurpos = 0;
        memset(gMsh->line, 0, sizeof(gMsh->line));
    }
}
 
extern void finsh_system_function_init(const void *begin, const void *end);
int consoleExInit(void)
{
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    finsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
    gMsh = (struct console_shell *)calloc(1, sizeof(struct console_shell));
    if (gMsh == NULL){
        rt_kprintf("no memory for consoleHalTask\n\r");
        return 0;
    }
    gMsh->echoMode = 0;
    gMsh->promptMode = 1;
    return 1;
}

 
int consoleExDeinit(void)
{
    free(gMsh);
    return 0;
}
#endif