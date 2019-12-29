#include <stdlib.h>

#include "ncurses.h"
#include "level.h"

static Room *
mk_room(x, y, w, h)
int x, y, w, h;
{
  Room *r = calloc(1, sizeof(*r));
  r->x = x;
  r->y = y;
  r->w = w;
  r->h = h;
  return r;
}

static Room *
get_room(lvl, x, y)
const Level *lvl;
int x, y;
{
  for (Room *r = lvl->rooms; r; r = r->next) {
    int rx = r->x, ry = r->y, rw = r->w, rh = r->h;
    if (x >= rx && x < rx + rw &&
        y >= ry && y < ry + rh)
      return r;
  }
  return NULL;
}

Level *
lvl_build()
{
  Level *lvl = calloc(1, sizeof(*lvl));
  int failed_attempts = 0;

  while (/*lvl->rooms_num < 10 &&*/ failed_attempts < 100) {
    int w = 3 + rand() % 6;
    int h = 3 + rand() % 3;
    int x = 1 + rand() % (COLS - w - 2);
    int y = 1 + rand() % (LINES - h - 2);

    bool vacant = TRUE;
    for (int i = x - 1; vacant && i <= x + w; ++i)
      for (int j = y - 1; vacant && j <= y + h; ++j)
        vacant = !get_room(lvl, i, j);
    if (!vacant) {
      ++failed_attempts;
      continue;
    }
    failed_attempts = 0;
    Room *r = mk_room(x, y, w, h);
    r->next = lvl->rooms;
    lvl->rooms = r;
    lvl->rooms_num += 1;
    mvprintw(0, 0, "Rooms created: %d", lvl->rooms_num);
    refresh();
  }

  return lvl;
}

void
lvl_render(level)
const Level *level;
{
  for (Room *r = level->rooms; r; r = r->next)
    for (int j = r->y; j < r->y + r->h; ++j) {
      move(j, r->x);
      wchar_t row[r->w + 1];
      row[r->w] = L'\0';
      for (int i = 0; i < r->w; ++i)
        row[i] = L'\u00B7';
      addwstr(row);
    }
  refresh();
}
