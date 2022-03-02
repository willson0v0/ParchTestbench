#include <../header/basic_io.h>
#include <../header/syscall.h>
#include <../header/type.h>

static char digits[] = "0123456789ABCDEF";

u64 strlen(char *s)
{
    char *p = s;
    while (*p)
        p++;
    return p - s;
}

void putc(int fd, char c)
{
    tb_write(fd, &c, 1);
}

void puts(int fd, char *s)
{
    tb_write(fd, s, strlen(s));
}

static void put_int(int fd, i64 num, int base, int sign)
{
    char buf[30];
    int i, neg;
    u64 unsigned_num;

    neg = 0;
    if (sign && num < 0)
    {
        neg = 1;
        unsigned_num = -num;
    }
    else
    {
        unsigned_num = num;
    }

    char *ptr = &buf[29];
    *ptr = '\0';
    ptr--;
    do
    {
        *(ptr--) = digits[unsigned_num % base];
    } while ((unsigned_num /= base) != 0);

    if (neg)
        *(ptr--) = '-';

    puts(fd, ptr + 1);
}

int vfprintf(int fd, char *format, va_list arg)
{
    char state = 0;
    for (int i = 0; format[i]; i++)
    {
        int c = format[i] & 0xff;
        if (state == 0)
        {
            if (c == '%')
            {
                state = '%';
            }
            else
            {
                putc(fd, c);
            }
        }
        else if (state == '%')
        {
            if (c == 'd')
            {
                put_int(fd, va_arg(arg, i64), 10, 1);
            }
            else if (c == 'u')
            {
                put_int(fd, va_arg(arg, u64), 10, 0);
            }
            else if (c == 'x')
            {
                put_int(fd, va_arg(arg, i64), 16, 0);
            }
            else if (c == 'p')
            {
                puts(fd, "0x");
                put_int(fd, va_arg(arg, u64), 16, 0);
            }
            else if (c == 's')
            {
                char *s = va_arg(arg, char *);
                if (s == 0)
                    s = "(null)";
                puts(fd, s);
            }
            else if (c == 'c')
            {
                putc(fd, va_arg(arg, u32));
            }
            else if (c == '%')
            {
                putc(fd, c);
            }
            else
            {
                // Unknown % sequence.  Print it to draw attention.
                putc(fd, '%');
                putc(fd, c);
            }
            state = 0;
        }
    }
}

int tb_printf(char *format, ...)
{
    va_list arg;
    int done;
    va_start(arg, format);
    done = vfprintf(1, format, arg);
    va_end(arg);

    return done;
}

char getc_noecho()
{
    char buf;
    tb_read(0, &buf, 1);
    return buf;
}

char tb_getc()
{
    char res = getc_noecho();
    tb_write(1, &res, 1);
    return res;
}

u64 tb_getline(char *buf, u64 length)
{
    char *p = buf;
    if (length == 0)
        return 0;
    while (p - buf < length - 1)
    {
        char c = tb_getc();
        if (c != '\n')
            *(p++) = c;
        else
            break;
    }
    *p = 0;
    return p - buf;
}