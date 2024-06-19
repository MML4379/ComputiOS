#include <stdio.h>

int putchar(int ic) {
#if defined(__is_libk)
    char c = (char) ic;
    // terminal_write(&c, sizeof(c));
#else
    // TODO: implement stdio and the write system call
#endif
    return ic;
}