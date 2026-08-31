#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "rtthread.h"

#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */
#define     RT_PRINTF_PRECISION

int divide(long *n, int base)
{
    int res;
    res = (int)(((unsigned long)*n) % base);
    *n = (long)(((unsigned long)*n) / base);
    return res;
}

static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
#ifdef RT_PRINTF_PRECISION
                          int   precision,
#endif /* RT_PRINTF_PRECISION */
                          int   type)
{
    char c, sign;
    char tmp[32];
    int precision_bak = precision;
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    int i, size;
    size = s;
    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;
    c = (type & ZEROPAD) ? '0' : ' ';
    /* get sign */
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
        }
        else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }
#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base == 2 || base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
#endif /* RT_PRINTF_SPECIAL */
    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
    {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }
#ifdef RT_PRINTF_PRECISION
    if (i > precision)
        precision = i;
    size -= precision;
#else
    size -= i;
#endif /* RT_PRINTF_PRECISION */
    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
            size--;

        while (size-- > 0)
        {
            if (buf < end)
                *buf = ' ';
            ++ buf;
        }
    }
    if (sign)
    {
        if (buf < end)
        {
            *buf = sign;
        }
        -- size;
        ++ buf;
    }
    /* no align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf < end)
                *buf = c;
            ++ buf;
        }
    }

#ifdef RT_PRINTF_PRECISION
    while (i < precision--)
    {
        if (buf < end)
            *buf = '0';
        ++ buf;
    }
#endif /* RT_PRINTF_PRECISION */
    /* put number in the temporary buffer */
    while (i-- > 0 && (precision_bak != 0))
    {
        if (buf < end)
            *buf = tmp[i];
        ++ buf;
    }
    while (size-- > 0)
    {
        if (buf < end)
            *buf = ' ';
        ++ buf;
    }
    return buf;
}



#define _ISDIGIT(c)  ((unsigned)((c) - '0') < 10)

int skip_atoi(const char **s)
{
    int i = 0;
    while (_ISDIGIT(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

int rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args)
{
    rt_uint32_t num;
    int i, len;
    char *str, *end, c;
    const char *s;
    rt_uint8_t base;            /* the base of number */
    rt_uint8_t flags;           /* flags to print number */
    rt_uint8_t qualifier;       /* 'h', 'l', or 'L' for integer fields */
    rt_int32_t field_width;     /* width of output field */
#ifdef RT_PRINTF_PRECISION
    int precision;      /* min. # of digits for integers and max for a string */
#endif /* RT_PRINTF_PRECISION */
    str = buf;
    end = buf + size;
    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end  = ((char *) - 1);
        size = end - buf;
    }
    for (; *fmt ; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str < end)
                *str = *fmt;
            ++ str;
            continue;
        }
        /* process flags */
        flags = 0;
        while (1)
        {
            /* skips the first '%' also */
            ++ fmt;
            if (*fmt == '-') flags |= LEFT;
            else if (*fmt == '+') flags |= PLUS;
            else if (*fmt == ' ') flags |= SPACE;
            else if (*fmt == '#') flags |= SPECIAL;
            else if (*fmt == '0') flags |= ZEROPAD;
            else break;
        }
        /* get field width */
        field_width = -1;
        if (_ISDIGIT(*fmt)) field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++ fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }
#ifdef RT_PRINTF_PRECISION
        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++ fmt;
            if (_ISDIGIT(*fmt)) precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++ fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0) precision = 0;
        }
#endif /* RT_PRINTF_PRECISION */
        /* get the conversion qualifier */
        qualifier = 0;
        if (*fmt == 'h' || *fmt == 'l')
        {
            qualifier = *fmt;
            ++ fmt;
        }
        /* the default base */
        base = 10;
        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str < end) *str = ' ';
                    ++ str;
                }
            }
            /* get character */
            c = (rt_uint8_t)va_arg(args, int);
            if (str < end) *str = c;
            ++ str;
            /* put width */
            while (--field_width > 0)
            {
                if (str < end) *str = ' ';
                ++ str;
            }
            continue;
        case 's':
            s = va_arg(args, char *);
            if (!s) s = "(NULL)";
            for (len = 0; (len != field_width) && (s[len] != '\0'); len++);
#ifdef RT_PRINTF_PRECISION
            if (precision > 0 && len > precision) len = precision;
#endif /* RT_PRINTF_PRECISION */
            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str < end) *str = ' ';
                    ++ str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str < end) *str = *s;
                ++ str;
                ++ s;
            }

            while (len < field_width--)
            {
                if (str < end) *str = ' ';
                ++ str;
            }
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
                flags |= ZEROPAD;
            }
#ifdef RT_PRINTF_PRECISION
            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, precision, flags);
#else
            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, flags);
#endif /* RT_PRINTF_PRECISION */
            continue;
        case '%':
            if (str < end) *str = '%';
            ++ str;
            continue;
        /* integer number formats - set up the flags and "break" */
        case 'b':
            base = 2;
            break;
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (str < end) *str = '%';
            ++ str;

            if (*fmt)
            {
                if (str < end) *str = *fmt;
                ++ str;
            }
            else
            {
                -- fmt;
            }
            continue;
        }
        if (qualifier == 'l')
        {
            num = va_arg(args, rt_uint32_t);
            if (flags & SIGN) num = (rt_int32_t)num;
        }
        else if (qualifier == 'h')
        {
            num = (rt_uint16_t)va_arg(args, int);
            if (flags & SIGN) num = (rt_int16_t)num;
        }
        else
        {
            num = va_arg(args, rt_uint32_t);
            if (flags & SIGN) num = (rt_int32_t)num;
        }
#ifdef RT_PRINTF_PRECISION
        str = print_number(str, end, num, base, field_width, precision, flags);
#else
        str = print_number(str, end, num, base, field_width, flags);
#endif /* RT_PRINTF_PRECISION */
    }
    if (size > 0)
    {
        if (str < end) *str = '\0';
        else
        {
            end[-1] = '\0';
        }
    }
    return str - buf;
}


int rt_kprintf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];
    va_start(args, fmt);
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
    #ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
    extern void ecConsolePutStr(const char* str);
    ecConsolePutStr(rt_log_buf);
    #endif
    va_end(args);
    return length;
}

int rt_kprintlnf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];
    va_start(args, fmt);
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
    #ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
    extern void ecConsolePutStr(const char* str);
    ecConsolePutStr(rt_log_buf);
    ecConsolePutStr("\r\n");
    #endif
    va_end(args);
    return length;
}
