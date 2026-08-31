/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_reg_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-12-5
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdlib.h>
#include "rtthread.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include <stdio.h>
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "console_ex.h"

extern int skip_atoi(const char **s);

int hexCharToDecimal(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    return 0;
}

uint32_t hexStringToDecimal(const char *hexStr)
{
    uint32_t result = 0;
    while (*hexStr)
    {
        result = result * 16 + hexCharToDecimal(*hexStr);
        hexStr++;
    }
    return result;
}

bool starts_with_0x(const char **str)
{
    if (str == NULL || *str == NULL || (*str)[0] == '\0' || (*str)[1] == '\0')
    {
        return false;
    }

    if ((*str)[0] == '0' && ((*str)[1] == 'x' || (*str)[1] == 'X'))
    {
        return true;
    }

    return false;
}

int cmd_reg(int argc, char **argv)
{

    char *cmd = argv[1];
    // reg write 0x4F040000 0x1ffffede
    // reg write 1325662208 536870622
    if (strcmp(cmd, "write") == 0)
    {
        char *addr = argv[2];
        char *val = argv[3];

        if (starts_with_0x((const char **)&addr) && starts_with_0x((const char **)&val))
        {
            addr += 2;
            val += 2;

            *((uint32_t *)hexStringToDecimal(addr)) = hexStringToDecimal(val);
        }
        else
        {
            uint32_t addr_val = (uint32_t)skip_atoi((const char **)&addr);
            uint32_t val_val = (uint32_t)skip_atoi((const char **)&val);

            *((uint32_t *)addr_val) = val_val;
        }
    }
    // reg read 0x00400000
    // reg read 1325662208
    else if (strcmp(cmd, "read") == 0)
    {
        char *addr = argv[2];
        if (starts_with_0x((const char **)&addr))
        {
            addr += 2;
            uint32_t val = *((uint32_t *)hexStringToDecimal(addr));
            rt_kprintlnf("0x%08x", val);
        }
        else
        {
            uint32_t addr_val = *((uint32_t *)skip_atoi((const char **)&addr));
            rt_kprintlnf("0x%08x", addr_val);
        }
    }
    // reg dump c:/123.bin 0x800000 0x1000
    else if (strcmp(cmd, "dump") == 0)
    {
        char *file = argv[2];
        char *addr = argv[3];
        char *len = argv[4];

        if (starts_with_0x((const char **)&addr) && starts_with_0x((const char **)&len))
        {
            addr += 2;
            len += 2;
            uint32_t *p = (uint32_t *)malloc(hexStringToDecimal(len));
            if (p == NULL)
            {
                rt_kprintlnf("malloc failed\n");
                return -1;
            }

            memcpy(p, (void *)hexStringToDecimal(addr), hexStringToDecimal(len));
            FILE *fp = file_fopen((const char *)file, "wb");
            if (fp == NULL)
            {
                rt_kprintlnf("open file %s failed\n", file);
                return -1;
            }
            file_fwrite(p, hexStringToDecimal(len), 1, fp);
            file_fclose(fp);
            free(p);
        }
    }
    // reg load c:/123.bin 0x800000
    else if (strcmp(cmd, "load") == 0)
    {
        char *file = argv[2];
        char *addr = argv[3];

        if (starts_with_0x((const char **)&addr))
        {
            addr += 2;
            FILE *fp = file_fopen(file, "rb");
            if (fp == NULL)
            {
                rt_kprintlnf("open file %s failed\n", file);
                return -1;
            }

            uint32_t *p = (uint32_t *)hexStringToDecimal(addr);
            file_fread(p, hexStringToDecimal(addr), 1, fp);
            file_fclose(fp);
        }
    }
    else
    {
        rt_kprintlnf("Usage: reg [options]\r\n");
        rt_kprintlnf("    %-10s - write register", "write");
        rt_kprintlnf("    %-10s - read register", "read");
        rt_kprintlnf("    %-10s - dump register to file", "dump");
        rt_kprintlnf("    %-10s - load file to register", "load");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_reg, reg, reg test);

#endif // FEATURE_SUBSYS_CONSOLE_ENABLE

// 1 read aonr.txt
// aon1 0x4F040000
// aon2 0x4F040004

// 2 write aonw.txt
// aon 0x4F040000 0x1ffffede