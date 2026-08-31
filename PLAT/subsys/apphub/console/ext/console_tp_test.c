/****************************************************************************
 *
 * Copy right:   2025-, Copyrigths of EigenComm Ltd.
 * File name:    console_tp_test.c
 * Description:  
 * History:      Rev1.0   2025-1-17
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rtthread.h"
static uint32_t tpInput=0;
static int16_t tpPos[4]={0};
extern int skip_atoi(const char **s);
extern void tp_irq_set(uint32_t val);
int tp_scan_test(uint32_t usrId, int16_t *pos)
{
    if (pos == NULL || tpInput == 0)
    {
        return 0;
    }
    tp_irq_set(tpInput);
    pos[0] = tpPos[0];
    pos[1] = tpPos[1];
    tpInput = 0;
    return 1;
}

extern const char *getConsolePrompt(void);
int cmd_indev_tp(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        rt_kprintf("Usage: tp xPos[0-240] yPos[0-320]\r\n");
        return -1;
    }
    tpPos[0] = atoi(argv[1]);
    tpPos[1] = atoi(argv[2]);
    if (tpPos[0] < 0 || tpPos[0] > 240 || tpPos[1] < 0 || tpPos[1] > 320)
    {
        rt_kprintf("Error: Invalid coordinates. xPos[0-240] yPos[0-320]\r\n");
        return -1;
    }
    tpInput += 1;
    return 0;
}

MSH_CMD_EXPORT_ALIAS(cmd_indev_tp, tp, tp test);

#endif