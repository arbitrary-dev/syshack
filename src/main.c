#include <curses.h>
#include <locale.h>

static void
init(void)
{
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
}

int
main(int argc, char *argv[])
{
  init();
}
