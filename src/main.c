#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <ncursesw/curses.h>
#include <locale.h>

#include "llist.h"

#define SMALL_SLEEP() usleep(100000)
#define SLEEP() usleep(250000)

typedef enum {
  CHARACTER,
  ITEM,
} Type;

typedef struct {
  Type type;
} Object;

typedef struct {
  Node *top;
} Cell;

static Cell **map = NULL;

typedef enum {
  PLAYER,
  WANDER,
  DEAD,
} State;

typedef struct {
  Object base;

  char   symbol;
  size_t x;
  size_t y;
  short  hp;
  State  state;
} Character;

typedef struct {
  Character *player;
  Character *droid;
  Node *render_queue;
} Context;

static Context *ctx = NULL;

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

  Node *n = map[x][y].top;

  if (!n)
    return false;

  Object *o = n->value;
  if (o->type == CHARACTER)
    return ((Character *) o)->state != DEAD;

  return false;
}

void
ch_render(Character *c) {
  if (c->state == DEAD) {
    init_pair(1, COLOR_WHITE, COLOR_RED);
    attron(COLOR_PAIR(1));
  }
  mvaddch(c->y, c->x, c->symbol);
  if (c->state == DEAD)
    attroff(COLOR_PAIR(1));
}

void
cell_render(size_t x, size_t y) {
  Node *n = map[x][y].top;
  if (!n) {
    mvaddch(y, x, ' ');
    return;
  }
  Object *o = n->value;
  if (o->type == ITEM)
    mvaddch(y, x, '.');
  else if (o->type == CHARACTER)
    ch_render((Character *) o);
}

void
ch_move(Character *c, int x, int y) {
  // Source
  int sx = c->x;
  int sy = c->y;

  // Target
  int tx = sx + x;
  int ty = sy + y;

  if (is_blocked(tx, ty))
    return;

  Node *n = map[sx][sy].top;
  assert(n);

  map[sx][sy].top = n->next;
  cell_render(sx, sy);

  map[tx][ty].top = l_prepend(map[tx][ty].top, n);
  c->x = tx;
  c->y = ty;
  ch_render(c);
}

typedef struct {
  size_t x1;
  size_t y1;
  size_t x2;
  size_t y2;
} Rect;

void
render_region(const Rect *r) {
  for (int j = r->y1; j <= r->y2; ++j)
    for (int i = r->x1; i <= r->x2; ++i)
      cell_render(i, j);
}

void
ctx_render_enqueued() {
  Node *i = ctx->render_queue;
  while (i) {
    Rect *rect = i->value;
    render_region(rect);
    free(rect);
    Node *next = i->next;
    free(i);
    i = next;
  }
  ctx->render_queue = NULL;
}

void
ctx_enqueue_rendering(size_t x1, size_t y1, size_t x2, size_t y2) {
  Rect *rect = malloc(sizeof(*rect));
  rect->x1 = x1;
  rect->y1 = y1;
  rect->x2 = x2;
  rect->y2 = y2;
  Node *new_n = calloc(1, sizeof(*new_n));
  new_n->value = rect;
  ctx->render_queue = l_append(ctx->render_queue, new_n);
}

void
render_text(size_t x, size_t y, const char *str) {
  mvprintw(y, x, str);
  refresh();
  ctx_enqueue_rendering(x, y, x + strlen(str), y);
}

void
ch_attack(Character *c, int x, int y) {
  int ay = c->y + y;
  int ax = c->x + x;
  Character *d = ctx->droid;
  int damage = 0;
  if (d->x == ax && d->y == ay && d->state != DEAD && (damage = rand() % 5)) {
    if ((d->hp -= damage) <= 0) {
      d->state = DEAD;
    } else {
      d->state = DEAD;
      ch_render(d);

      char str[3];
      sprintf(str, "-%d", damage);
      render_text(c->x + 1, c->y - 1, str);

      SMALL_SLEEP();
      d->state = WANDER;
      ch_render(d);
      refresh();

      SLEEP();
    }
    ch_render(d);
  } else {
    render_text(c->x + 1, c->y - 1, "Miss!");
    SLEEP();
  }
}

void
move_droid(Character *d) {
  ch_move(d, rand() % 3 - 1, rand() % 3 - 1);
}

void
do_attack(Character *player) {
  int px = player->x;
  int py = player->y;

  render_text(px + 1, py - 1, "Where?");
  int ch = getch();
  ctx_render_enqueued();

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

    default:
      render_text(px + 1, py - 1, "What?!");
      SLEEP();
  }
}

static Character *
ch_create(char symbol, uint8_t hp, State state, uint8_t x, uint8_t y) {
  Character *c = calloc(1, sizeof(*c));
  c->base.type = CHARACTER;
  c->symbol = symbol;
  c->hp = hp;
  c->state = state;
  c->x = x;
  c->y = y;

  Node *n = calloc(1, sizeof(*n));
  n->value = c;

  map[x][y].top = l_prepend(map[x][y].top, n);
  ch_render(c);

  return c;
}

int
main(int argc, char *argv[]) {
  init();

  ctx = calloc(1, sizeof(*ctx));

  map = calloc(COLS, sizeof(Cell *));
  for (int i = 0; i < COLS; ++i)
    map[i] = calloc(LINES, sizeof(Cell *));

  Character *player = ch_create('@', 15, PLAYER, COLS / 2, LINES / 2);
  ctx->player = player;

  Character *droid = ch_create('d', 15, WANDER, COLS / 2 + 1, LINES / 2 + 1);
  ctx->droid = droid;

  int ch;
  bool done = false;

  while (!done && (ch = getch()) > 0) {
    switch (ch) {
      case 'q':
        done = TRUE;
        break;

      case 'h':
        ch_move(player, -1, 0);
        break;

      case 'l':
        ch_move(player, 1, 0);
        break;

      case 'j':
        ch_move(player, 0, 1);
        break;

      case 'k':
        ch_move(player, 0, -1);
        break;

      case 'y':
        ch_move(player, -1, -1);
        break;

      case 'u':
        ch_move(player, 1, -1);
        break;

      case 'b':
        ch_move(player, -1, 1);
        break;

      case 'n':
        ch_move(player, 1, 1);
        break;

      case 'a':
        do_attack(player);
        break;
    }

    if (done)
      break;

    ctx_render_enqueued();

    if (droid->state == WANDER)
      move_droid(droid);

    refresh();
  }

  clear();
  mvaddstr(LINES / 2, COLS / 2 - 4, "THE END!");
  refresh();
  endwin();
  exit(0);
}
