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
  int     x;
  int     y;
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

  if (i == NULL)
    return false;

  if (i->type == CHARACTER)
    return ((char_t *) i->value)->state != DEAD;

  return false;
}

void
ch_render(char_t *c) {
  if (c->state == DEAD) {
    init_pair(1, COLOR_WHITE, COLOR_RED);
    attron(COLOR_PAIR(1));
  }
  mvaddch(c->y, c->x, c->ch);
  if (c->state == DEAD)
    attroff(COLOR_PAIR(1));
}

void
item_render(item_t *item, int x, int y) {
  if (item == NULL)
    mvaddch(y, x, ' ');
  else if (item->type == ITEM)
    mvaddch(y, x, '.');
  else if (item->type == CHARACTER)
    ch_render((char_t *) item->value);
}

void
ch_move(char_t *c, int x, int y) {
  int cx = c->x + x;
  int cy = c->y + y;

  if (is_blocked(cx, cy))
    return;

  item_t *ci = map[c->x][c->y].first;
  if (ci == NULL) {
    ci = malloc(sizeof(item_t *));
    ci->type = CHARACTER;
    ci->value = c;
  }
  item_t *pi = NULL; // previous
  while (ci != NULL && ci->value != c) {
    pi = ci;
    ci = ci->next;
  }

  item_render(pi, c->x, c->y);
  if (pi == NULL)
    map[c->x][c->y].first = NULL;
  else
    pi->next = NULL;

  item_t *last = map[cx][cy].first;
  if (last == NULL) {
    map[cx][cy].first = ci;
  } else {
    while (last != NULL && last->next != NULL)
      last = last->next;
    last->next = ci;
  }
  c->x = cx;
  c->y = cy;
  ch_render(c);

  refresh();
}

void
render_region(int x, int y, int w) {
  for (int i = 0; i < w; ++i)
    item_render(map[x + i][y].first, x + i, y);
}

void
ch_attack(char_t *c, int x, int y) {
  int ay = c->y + y;
  int ax = c->x + x;
  char_t *d = ctx->droid;
  if (d->x == ax && d->y == ay && d->state != DEAD) {
    d->state = DEAD;
    ch_render(d);
  } else {
    mvprintw(c->y - 1, c->x + 1, "Miss!");
    getch();
    render_region(c->x + 1, c->y - 1, 5);
  }
}

void
move_droid(char_t *d) {
  ch_move(d, rand() % 3 - 1, rand() % 3 - 1);
}

void
do_attack(char_t *player) {
  int px = player->x;
  int py = player->y;

  mvprintw(py - 1, px + 1, "Where?");
  char ch = getch();
  render_region(px + 1, py - 1, 6);

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
  player.x = COLS / 2;
  player.y = LINES / 2;
  ch_move(&player, 0, 0);
  ctx->player = &player;

  char_t droid;
  droid.ch = 'd';
  droid.state = WANDER;
  droid.x = COLS / 2 + 1;
  droid.y = LINES / 2 + 1;
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
