#include <time.h>
#include <stdlib.h>

#include <ncursesw/curses.h>
#include <locale.h>

typedef enum {
  CHARACTER,
  ITEM,
} item_type_t;

typedef struct Item {
  item_type_t type;
  void *value;
  struct Item *next;
} item_t;

typedef struct {
  item_t *first;
} cell_t;

static cell_t **map = NULL;

typedef enum {
  PLAYER,
  WANDER,
  FLIGHT,
  FIGHT,
  DEAD,
} state_t;

typedef struct {
  char    ch;
  int     pos_x;
  int     pos_y;
  state_t state;
} char_t;

typedef struct {
  char_t *player;
  char_t *droid;
} context_t;

static context_t *ctx = NULL;

static void
init(void)
{
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  nonl();
  curs_set(0);

  use_default_colors();
  start_color();

  srand(time(NULL));
}

bool
is_blocked(int x, int y) {
  if (x < 0 || x >= COLS || y < 0 || y >= LINES)
    return true;

  item_t *i = map[x][y].first;
  return !(i == NULL || ((char_t *) i->value)->state == DEAD);
}

void
ch_move(char_t *c, int x, int y) {
  int cx = c->pos_x + x;
  int cy = c->pos_y + y;

  if (is_blocked(cx, cy)) {
    beep();
    return;
  }

  mvaddch(c->pos_y, c->pos_x, ' ');
  map[c->pos_x][c->pos_y].first = NULL;

  mvaddch(cy, cx, c->ch);
  item_t *ci = malloc(sizeof(item_t *));
  ci->type = CHARACTER;
  ci->value = c;
  map[cx][cy].first = ci;

  refresh();

  c->pos_x = cx;
  c->pos_y = cy;
}

void
ch_attack(char_t *c, int x, int y) {
  init_pair(1, COLOR_WHITE, COLOR_RED);
  int ay = c->pos_y + y;
  int ax = c->pos_x + x;
  char_t *d = ctx->droid;
  if (d->pos_x == ax && d->pos_y == ay) {
    d->state = DEAD;
    attron(COLOR_PAIR(1));
    mvaddch(ay, ax, 'x');
    attroff(COLOR_PAIR(1));
  }
}

void
move_droid(char_t *d) {
  int dx = d->pos_x + rand() % 3 - 1;
  int dy = d->pos_y + rand() % 3 - 1;

  if (is_blocked(dx, dy))
    return;

  mvaddch(d->pos_y, d->pos_x, ' ');
  map[d->pos_x][d->pos_y].first = NULL;

  mvaddch(dy, dx, d->ch);
  item_t *di = malloc(sizeof(item_t *));
  di->type = CHARACTER;
  di->value = d;
  map[dx][dy].first = di;

  refresh();

  d->pos_x = dx;
  d->pos_y = dy;
}

void
do_attack(char_t *player) {
  int px = player->pos_x;
  int py = player->pos_y;

  mvprintw(py - 1, px + 1, "Where?");
  char ch = getch();
  mvprintw(py - 1, px + 1, "      ");

  switch (ch) {
    case 'h':
      ch_attack(player, -1, 0);
      break;

    case 'l':
      ch_attack(player, 1, 0);
      break;

    case 'j':
      ch_attack(player, 0, 1);
      break;

    case 'k':
      ch_attack(player, 0, -1);
      break;

    case 'y':
      ch_attack(player, -1, -1);
      break;

    case 'u':
      ch_attack(player, 1, -1);
      break;

    case 'b':
      ch_attack(player, -1, 1);
      break;

    case 'n':
      ch_attack(player, 1, 1);
      break;
  }
}

int
main(int argc, char *argv[])
{
  init();

  ctx = malloc(sizeof(context_t *));
  map = malloc(COLS * sizeof(cell_t *));
  for (int i = 0; i < COLS; ++i) {
    map[i] = malloc(LINES * sizeof(cell_t *));
    for (int j = 0; j < LINES; ++j)
      map[i][j].first = NULL;
  }

  char_t player;
  player.ch = '@';
  player.state = PLAYER;
  player.pos_x = COLS / 2;
  player.pos_y = LINES / 2;
  ch_move(&player, 0, 0);
  ctx->player = &player;

  char_t droid;
  droid.ch = 'd';
  droid.state = WANDER;
  droid.pos_x = COLS / 2 + 1;
  droid.pos_y = LINES / 2 + 1;
  ch_move(&droid, 0, 0);
  ctx->droid = &droid;

  int ch;
  bool done = false;

  while (!done && (ch = getch()) > 0) {
    switch (ch) {
      case 'q':
        done = TRUE;
        break;

      case 'h':
        ch_move(&player, -1, 0);
        break;

      case 'l':
        ch_move(&player, 1, 0);
        break;

      case 'j':
        ch_move(&player, 0, 1);
        break;

      case 'k':
        ch_move(&player, 0, -1);
        break;

      case 'y':
        ch_move(&player, -1, -1);
        break;

      case 'u':
        ch_move(&player, 1, -1);
        break;

      case 'b':
        ch_move(&player, -1, 1);
        break;

      case 'n':
        ch_move(&player, 1, 1);
        break;

      case 'a':
        do_attack(&player);
    }

    if (droid.state == WANDER)
      move_droid(&droid);
  }

  refresh();
  endwin();

  exit(0);
}
