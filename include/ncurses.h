// UTF-8 support
#define _XOPEN_SOURCE_EXTENDED

#include <ncursesw/curses.h>

// Sleep
#include <unistd.h>

#define SMALL_SLEEP() usleep(100000)
#define SLEEP() usleep(250000)
