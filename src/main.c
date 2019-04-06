#include <time.h>
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

  srand(time(NULL));
}

typedef enum {
  PLAYER,
  WANDER,
  FLIGHT,
  FIGHT,
} state_t;

typedef struct {
  char    ch;
  int     pos_x;
  int     pos_y;
  state_t state;
} char_t;

bool
is_blocked(int x, int y) {
  return x < 0 || x >= COLS || y < 0 || y >= LINES;
}

void
move_ch(char_t *c, int x, int y) {
  int cx = c->pos_x + x;
  int cy = c->pos_y + y;

  if (is_blocked(cx, cy)) {
    beep();
    return;
  }

  mvaddch(c->pos_y, c->pos_x, ' ');
  mvaddch(cy, cx, c->ch);
  refresh();

  c->pos_x = cx;
  c->pos_y = cy;
}

void
move_droid(char_t *d) {
  int dx = d->pos_x + rand() % 3 - 1;
  int dy = d->pos_y + rand() % 3 - 1;

  if (is_blocked(dx, dy))
    return;

  mvaddch(d->pos_y, d->pos_x, ' ');
  mvaddch(dy, dx, d->ch);
  refresh();

  d->pos_x = dx;
  d->pos_y = dy;
}

int
main(int argc, char *argv[])
{
  init();

  char_t player;
  player.ch = '@';
  player.state = PLAYER;
  player.pos_x = COLS / 2;
  player.pos_y = LINES / 2;
  move_ch(&player, 0, 0);

  char_t droid;
  droid.ch = 'd';
  droid.state = WANDER;
  droid.pos_x = COLS / 2 + 1;
  droid.pos_y = LINES / 2 + 1;
  move_ch(&droid, 0, 0);

  int ch;
  bool done = false;

  while (!done && (ch = getch()) > 0) {
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

    move_droid(&droid);
  }

  refresh();
  endwin();

  exit(0);
}
