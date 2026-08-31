
#include <rtthread.h>
#include <string.h>
#include <rtdef.h>
#include "cmsis_os2.h"
#include "console_ex.h"
#ifdef RT_USING_FINSH

#ifndef FINSH_ARG_MAX
#define FINSH_ARG_MAX    8
#endif /* FINSH_ARG_MAX */

#include "msh.h"
#include "shell.h"
#ifdef RT_USING_MODULE
#include <dlmodule.h>
#endif /* RT_USING_MODULE */

typedef int (*cmd_function_t)(int argc, char **argv);

void rt_assert_handler(const char *ex_string, const char *func, rt_size_t line)
{
//    volatile char dummy = 0;
//     if (rt_assert_hook == RT_NULL)
//     {
// #ifdef RT_USING_MODULE
//         if (dlmodule_self())
//         {
//             /* close assertion module */
//             dlmodule_exit(-1);
//         }
//         else
// #endif /*RT_USING_MODULE*/
//         {
//             rt_kprintf("(%s) assertion failed at function:%s, line number:%d \n", ex_string, func, line);
//             while (dummy == 0);
//         }
//     }
//     else
//     {
//         rt_assert_hook(ex_string, func, line);
//     }
}

char *rt_strncpy(char *dst, const char *src, rt_size_t n)
{
    if (n != 0)
    {
        char *d = dst;
        const char *s = src;
        do
        {
            if ((*d++ = *s++) == 0)
            {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                {
                    *d++ = 0;
                }

                break;
            }
        } while (--n != 0);
    }

    return (dst);
}
int msh_help(int argc, char **argv)
{
    rt_kprintf("commands:\n\r");
    {
        struct finsh_syscall *index;
        for (index = _syscall_table_begin;
                index < _syscall_table_end;
                FINSH_NEXT_SYSCALL(index)){
#if defined(FINSH_USING_DESCRIPTION) && defined(FINSH_USING_SYMTAB)
            rt_kprintf("%-16s - %s\n\r", index->name, index->desc);
#else
            rt_kprintf("%s ", index->name);
#endif
        }
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(msh_help, help, shell help.);

#ifdef MSH_USING_BUILT_IN_COMMANDS
void rt_memory_info(rt_size_t *total,
                            rt_size_t *used,
                            rt_size_t *max_used)
{

}
// RTM_EXPORT(rt_memory_info);
rt_tick_t rt_tick_get(void)
{
    /* return the global tick */
    return osKernelGetTickCount();
}
// RTM_EXPORT(rt_tick_get);

#ifdef RT_USING_HEAP
int cmd_free(int argc, char **argv)
{
#ifdef RT_USING_MEMHEAP_AS_HEAP
    extern void list_memheap(void);
    list_memheap();
#else
    rt_size_t total = 0, used = 0, max_used = 0;

    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("total    : %d\n\r", total);
    rt_kprintf("used     : %d\n\r", used);
    rt_kprintf("maximum  : %d\n\r", max_used);
    rt_kprintf("available: %d\n\r", total - used);
#endif
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_free, free, Show the memory usage in the system.);
#endif /* RT_USING_HEAP */
#endif /* MSH_USING_BUILT_IN_COMMANDS */

static int msh_split(char *cmd, rt_size_t length, char *argv[FINSH_ARG_MAX])
{
    char *ptr;
    rt_size_t position;
    rt_size_t argc;
    rt_size_t i;

    ptr = cmd;
    position = 0;
    argc = 0;

    while (position < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        if (argc >= FINSH_ARG_MAX)
        {
            rt_kprintf("Too many args ! We only Use:\n\r");
            for (i = 0; i < argc; i++)
            {
                rt_kprintf("%s ", argv[i]);
            }
            rt_kprintf("\n\r");
            break;
        }

        if (position >= length) break;
        /* handle string */
        if (*ptr == '"')
        {
            ptr ++;
            position ++;
            argv[argc] = ptr;
            argc ++;
            /* skip this string */
            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++;
                        position ++;
                    }
                }
                ptr ++;
                position ++;
            }
            if (position >= length) break;

            /* skip '"' */
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++;
                position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}

static cmd_function_t msh_get_cmd(char *cmd, int size)
{
    struct finsh_syscall *index;
    cmd_function_t cmd_func = RT_NULL;

    for (index = _syscall_table_begin;
            index < _syscall_table_end;
            FINSH_NEXT_SYSCALL(index))
    {
        if (strncmp(index->name, cmd, size) == 0 &&
                index->name[size] == '\0')
        {
            cmd_func = (cmd_function_t)index->func;
            break;
        }
    }

    return cmd_func;
}

static int _msh_exec_cmd(char *cmd, rt_size_t length, int *retp)
{
    int argc;
    rt_size_t cmd0_size = 0;
    cmd_function_t cmd_func;
    char *argv[FINSH_ARG_MAX];

    RT_ASSERT(cmd);
    RT_ASSERT(retp);

    /* find the size of first command */
    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && cmd0_size < length)
        cmd0_size ++;
    if (cmd0_size == 0)
        return -RT_ERROR;

    cmd_func = msh_get_cmd(cmd, cmd0_size);
    if (cmd_func == RT_NULL)
        return -RT_ERROR;

    /* split arguments */
    memset(argv, 0x00, sizeof(argv));
    argc = msh_split(cmd, length, argv);
    if (argc == 0)
        return -RT_ERROR;

    /* exec this command */
    *retp = cmd_func(argc, argv);
    return 0;
}

int msh_exec(char *cmd, rt_size_t length)
{
    int cmd_ret;
    /* strim the beginning of command */
    while ((length > 0) && (*cmd  == ' ' || *cmd == '\t'))
    {
        cmd++;
        length--;
    }
    if (length == 0)
        return 0;

    /* Exec sequence:
     * 1. built-in command
     * 2. module(if enabled)
     */
    if (_msh_exec_cmd(cmd, length, &cmd_ret) == 0)
    {
        return cmd_ret;
    }

    /* truncate the cmd at the first space. */
    {
        char *tcmd;
        tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0')
        {
            tcmd++;
        }
        *tcmd = '\0';
    }
    rt_kprintf("%s: command not found.\n\r", cmd);
    return -1;
}

static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;
    while ((*str != 0) && (*str2 != 0) && (*str == *str2))
    {
        str ++;
        str2 ++;
    }
    return (str - str1);
}

void msh_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct finsh_syscall *index;
    min_length = 0;
    name_ptr = RT_NULL;
    if (*prefix == '\0')
    {
        msh_help(0, RT_NULL);
        return;
    }

    /* checks in internal command */
    {
        for (index = _syscall_table_begin; index < _syscall_table_end; FINSH_NEXT_SYSCALL(index))
        {
            /* skip finsh shell function */
            cmd_name = (const char *) index->name;
            if (strncmp(prefix, cmd_name, strlen(prefix)) == 0)
            {
                if (min_length == 0)
                {
                    /* set name_ptr */
                    name_ptr = cmd_name;
                    /* set initial length */
                    min_length = strlen(name_ptr);
                }

                length = str_common(name_ptr, cmd_name);
                if (length < min_length)
                    min_length = length;

                rt_kprintf("%s\n\r", cmd_name);
            }
        }
    }

    /* auto complete string */
    if (name_ptr != NULL)
    {
        rt_strncpy(prefix, name_ptr, min_length);
    }

    return ;
}
#endif /* RT_USING_FINSH */
