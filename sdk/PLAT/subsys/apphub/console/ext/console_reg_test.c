/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_reg_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-12-5
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"

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

        if (starts_with_0x(&addr) && starts_with_0x(&val))
        {
            addr += 2;
            val += 2;

            *((uint32_t *)hexStringToDecimal(addr)) = hexStringToDecimal(val);
        }
        else
        {
            uint32_t addr_val = (uint32_t)skip_atoi(&addr);
            uint32_t val_val = (uint32_t)skip_atoi(&val);

            *((uint32_t *)addr_val) = val_val;
        }
    }
    // reg read 0x00400000
    // reg read 1325662208
    else if (strcmp(cmd, "read") == 0)
    {
        char *addr = argv[2];
        if (starts_with_0x(&addr))
        {
            addr += 2;
            uint32_t val = *((uint32_t *)hexStringToDecimal(addr));
            rt_kprintlnf("0x%08x", val);
        }
        else
        {
            uint32_t addr_val = *((uint32_t *)skip_atoi(&addr));
            rt_kprintlnf("0x%08x", addr_val);
        }
    }
    else
    {
        rt_kprintlnf("Usage: reg [options]\r\n");
        rt_kprintlnf("    %-10s - write register", "write");
        rt_kprintlnf("    %-10s - read register", "read");
    }
    rt_kprintf("\r\n%s", getConsolePrompt());
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_reg, reg, reg test);

#endif // FEATURE_SUBSYS_CONSOLE_ENABLE

// 1 read aonr.txt
// aon1 0x4F040000
// aon2 0x4F040004

// 2 write aonw.txt
// aon 0x4F040000 0x1ffffede