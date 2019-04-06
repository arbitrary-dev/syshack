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

typedef struct {
  char ch;
  int pos_x;
  int pos_y;
} char_t;

void
move_ch(char_t *c, int x, int y) {
  int cx = c->pos_x + x;
  int cy = c->pos_y + y;

  if (cx < 0 || cx >= COLS || cy < 0 || cy >= LINES) {
    beep();
    return;
  }

  move(c->pos_y, c->pos_x);
  addch(' ');

  move(cy, cx);
  addch('@');
  refresh();

  c->pos_x = cx;
  c->pos_y = cy;
}

int
main(int argc, char *argv[])
{
  init();

  bool done = FALSE;
  int ch;

  char_t player;
  player.ch = '@';
  player.pos_x = COLS / 2;
  player.pos_y = LINES / 2;
  move_ch(&player, 0, 0);

  while (!done && (ch = wgetch(stdscr)) > 0) {
    switch (ch) {
      case 'q':
        done = TRUE;
        break;

      case 'h':
        move_ch(&player, -1, 0);
        break;

      case 'l':
        move_ch(&player, 1, 0);
        break;

      case 'j':
        move_ch(&player, 0, 1);
        break;

      case 'k':
        move_ch(&player, 0, -1);
        break;
    }
  }

  refresh();
  endwin();

  exit(0);
}
