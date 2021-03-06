#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#include "ncurses.h"
#include "llist.h"
#include "level.h"

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
  FIGHT,
  FLIGHT,
  DEAD,
} State;

typedef struct {
  Object base;
  char   symbol;
  int x;
  int y;
  int hp;
  State  state;
} Character;

typedef struct {
  bool done;
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

  init_pair(1, COLOR_WHITE, COLOR_RED);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);

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
  if (c->state == DEAD)
    attron(COLOR_PAIR(1));
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
ch_move(Character *c, int dx, int dy) {
  // Source
  int sx = c->x;
  int sy = c->y;

  // Target
  int tx = sx + dx;
  int ty = sy + dy;

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

void
ch_move_towards(Character *c, Character *to) {
  int dx = (to->x - c->x > 0) - (to->x - c->x < 0);
  int dy = (to->y - c->y > 0) - (to->y - c->y < 0);
  ch_move(c, dx, dy);
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
ch_attack_side(Character *c, int dx, int dy) {
  int ay = c->y + dy;
  int ax = c->x + dx;

  Character *t = (map[ax][ay].top) ? map[ax][ay].top->value : NULL;

  int damage = 0;
  if (t && t->x == ax && t->y == ay && t->state != DEAD && (damage = rand() % 5)) {
    if ((t->hp -= damage) <= 0) {
      bool is_player = t->state == PLAYER;
      t->state = DEAD;
      if (is_player) {
        ch_render(t);
        attron(COLOR_PAIR(2));
        render_text(t->x + 1, t->y - 1, "WASTED!");
        SLEEP();
        SLEEP();
        SLEEP();
        SLEEP();
        ctx->done = TRUE;
      }
    } else {
      State s = t->state;
      t->state = DEAD;
      ch_render(t);

      char str[4];
      sprintf(str, "-%d", damage);
      attron(COLOR_PAIR(2));
      render_text(t->x + 1, t->y - 1, str);
      attroff(COLOR_PAIR(2));

      SMALL_SLEEP();
      switch (s) {
        case WANDER:
        case FIGHT:
          if (t->hp > 5)
            t->state = FIGHT;
          else
            t->state = FLIGHT;
          break;

        default:
            t->state = s;
      }
      ch_render(t);
      refresh();

      SLEEP();
      ctx_render_enqueued();
      refresh();
    }
    ch_render(t);
  } else {
    render_text(c->x + 1, c->y - 1, "Miss!");
    SLEEP();
  }
}

void
ch_attack_char(Character *attacker, Character *target) {
  int dx = target->x - attacker->x;
  int dy = target->y - attacker->y;
  ch_attack_side(attacker, dx, dy);
}

static bool
is_near(Character *c1, Character *c2) {
  return abs(c1->x - c2->x) <= 1 && abs(c1->y - c2->y) <= 1;
}

void
move_droid() {
  Character *d = ctx->droid;
  switch (d->state) {
    case FIGHT: {
      Character *p = ctx->player;
      if (is_near(d, p))
        ch_attack_char(d, p);
      else
        ch_move_towards(d, p);
      break;
    }
    default:
      ch_move(d, rand() % 3 - 1, rand() % 3 - 1);
  }
}

void
do_attack(Character *player) {
  int px = player->x;
  int py = player->y;

  // FIXME out of screen
  render_text(px + 1, py - 1, "Where?");
  int ch = getch();
  ctx_render_enqueued();

  switch (ch) {
    case 'h':
      ch_attack_side(player, -1, 0);
      break;

    case 'l':
      ch_attack_side(player, 1, 0);
      break;

    case 'j':
      ch_attack_side(player, 0, 1);
      break;

    case 'k':
      ch_attack_side(player, 0, -1);
      break;

    case 'y':
      ch_attack_side(player, -1, -1);
      break;

    case 'u':
      ch_attack_side(player, 1, -1);
      break;

    case 'b':
      ch_attack_side(player, -1, 1);
      break;

    case 'n':
      ch_attack_side(player, 1, 1);
      break;

    default:
      render_text(px + 1, py - 1, "What?!");
      SLEEP();
  }
}

static Character *
ch_create(char symbol, int hp, State state, int x, int y) {
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

  /*Level *lvl = */lvl_build();

  /*
  for (int i = 0; i < 16 * 8; ++i) {
    wchar_t str[2] = { L'\u2500' + i, L'\0' };
    mvprintw(i / 16, (i % 16) * 8,"%ls %x", str, 0x2500 + i);
  }

  cchar_t wch = { A_NORMAL | COLOR_PAIR(2), { L'\u2534', L'\0' }};
  mvadd_wch(LINES - 1, COLS - 1, &wch);
  */

  ctx = calloc(1, sizeof(*ctx));

  map = calloc(COLS, sizeof(Cell *));
  for (int i = 0; i < COLS; ++i)
    map[i] = calloc(LINES, sizeof(Cell *));

  Character *player = ch_create('@', 15, PLAYER, COLS / 2, LINES / 2);
  ctx->player = player;

  Character *droid = ch_create('d', 15, WANDER, COLS / 2 + 1, LINES / 2 + 1);
  ctx->droid = droid;

  int ch;
  while (!ctx->done && (ch = getch()) > 0) {
    switch (ch) {
      case 'q':
        ctx->done = TRUE;
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

    if (ctx->done)
      break;

    ctx_render_enqueued();

    if (droid->state != DEAD)
      move_droid();

    refresh();
  }

  clear();
  mvaddstr(LINES / 2, COLS / 2 - 4, "THE END!");
  refresh();
  SLEEP();
  SLEEP();
  SLEEP();
  SLEEP();
  endwin();
  exit(0);
}
