#include <stdlib.h>

#include <curses.h>
#include <locale.h>

static void
init(void)
{
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  nonl();
  curs_set(0);
}

int
main(int argc, char *argv[])
{
  init();

  bool done = FALSE;
  int ch;

  int pos_x = COLS / 2;
  int pos_y = LINES / 2;

  wmove(stdscr, pos_y, pos_x);
  waddch(stdscr, '@');
  wrefresh(stdscr);

  while (!done && (ch = wgetch(stdscr)) > 0) {
    switch (ch) {
      case 'q':
        done = TRUE;
        break;

      case 'h':
        wmove(stdscr, pos_y, pos_x);
        waddch(stdscr, ' ');
        pos_x -= 1;
        break;

      case 'l':
        wmove(stdscr, pos_y, pos_x);
        waddch(stdscr, ' ');
        pos_x += 1;
        break;

      case 'j':
        wmove(stdscr, pos_y, pos_x);
        waddch(stdscr, ' ');
        pos_y += 1;
        break;

      case 'k':
        wmove(stdscr, pos_y, pos_x);
        waddch(stdscr, ' ');
        pos_y -= 1;
        break;
    }

    wmove(stdscr, pos_y, pos_x);
    waddch(stdscr, '@');
    wrefresh(stdscr);
  }

  wrefresh(stdscr);
  endwin();

  exit(0);
}
