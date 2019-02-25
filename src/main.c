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

typedef struct Character {
  int pos_x;
  int pos_y;
} char_t;

void
move_char(char_t *c, int dx, int dy)
{
  int x = c->pos_x;
  int y = c->pos_y;

  wmove(stdscr, y, x);
  waddch(stdscr, ' ');

  x += dx;
  if (x < 0)
    x = 0;
  else if (x >= COLS)
    x = COLS - 1;

  y += dy;
  if (y < 0)
    y = 0;
  else if (y >= LINES)
    y = LINES - 1;

  wmove(stdscr, y, x);
  waddch(stdscr, '@');

  c->pos_x = x;
  c->pos_y = y;
}

int
main(int argc, char *argv[])
{
  init();

  bool done = FALSE;
  int ch;

  char_t *player = malloc(sizeof(char_t));
  player->pos_x = COLS / 2;
  player->pos_y = LINES / 2;

  move_char(player, 0, 0);
  wrefresh(stdscr);

  while (!done && (ch = wgetch(stdscr)) > 0) {
    switch (ch) {
      case 'q':
        done = TRUE;
        break;

      case 'h':
        move_char(player, -1, 0);
        break;

      case 'l':
        move_char(player, 1, 0);
        break;

      case 'j':
        move_char(player, 0, 1);
        break;

      case 'k':
        move_char(player, 0, -1);
        break;
    }

    wrefresh(stdscr);
  }

  free(player);

  wrefresh(stdscr);
  endwin();

  exit(0);
}
