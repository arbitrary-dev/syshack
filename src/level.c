#include "ncurses.h"
#include "level.h"

static Room *
mk_room_rect(x, y, w, h)
int x, y, w, h;
{
  Room *r = calloc(1, sizeof(*r));
  r->x = x;
  r->y = y;
  r->w = w;
  r->h = h;
  r->is_rect = TRUE;
  return r;
}

static Room *
get_room(lvl, x, y)
const Level *lvl;
int x, y;
{
  if (x < 0 || x >= COLS ||
      y < 0 || y >= LINES)
    return NULL;
  for (Room *r = lvl->rooms; r; r = r->next) {
    ROOM(r);
    if (x >= rx && x < rx + rw &&
        y >= ry && y < ry + rh)
      return r;
  }
  return NULL;
}

static Tile
room_get_tile(room, x, y)
const Room *room;
int x, y;
{
  ROOM(room);
  if (x < roomx || x >= roomx + roomw ||
      y < roomy || y >= roomy + roomh)
    return T_EMPTY;
  if (room->is_rect)
    return x > roomx && x < roomx + roomw - 1 &&
           y > roomy && y < roomy + roomh - 1
           ? T_FLOOR
           : T_WALL;
  return room->tiles[(y - roomy) * roomw + x - roomx];
}

static bool
room_is_floor(room, x, y)
const Room *room;
int x, y;
{
  return room_get_tile(room, x, y) == T_FLOOR;
}

static bool
is_floor(lvl, x, y)
const Level *lvl;
int x, y;
{
  Room *r = get_room(lvl, x, y);
  if (!r)
    return false;
  return room_is_floor(r, x, y);
}

static void
render_row(x, y, w, start, middle, end)
int x, y, w;
wchar_t start, middle, end;
{
  move(y, x);
  wchar_t row[w + 1];
  row[w] = L'\0';
  for (int i = 0; i < w; ++i) {
    wchar_t ch;
    if (i == 0)
      ch = start;
    else if (i == w - 1)
      ch = end;
    else
      ch = middle;
    row[i] = ch;
  }
  addwstr(row);
}

// #define X(x, y, c, t, f) mvinch((y), (x)) == (c) ? (t) : (f)

static void
render_room(level, room)
const Level *level;
const Room *room;
{
  ROOM(room);
  if (room->is_rect) {
    for (int j = 0; j < roomh; ++j) {
      wchar_t start, middle, end;
      if (j == 0) {
        start = W_NW;
        middle = W_H;
        end = W_NE;
      } else if (j == roomh - 1) {
        start = W_SW;
        middle = W_H;
        end = W_SE;
      } else {
        start = W_V;
        middle = FLOOR;
        end = W_V;
      }
      render_row(roomx, roomy + j, roomw, start, middle, end);
    }
    mvprintw(roomy + 1, roomx + 1, "%dx%d", roomw, roomh);
  } else {
    Tile *ts = room->tiles;
    for (int i = 0; i < roomw * roomh; ++i) {
      Tile t = ts[i / roomw * roomw + i % roomw];
      wchar_t ch;
      switch(t) {
        case T_FLOOR:
          ch = FLOOR;
          break;
        case T_WALL:
          ch = 'W';
          break;
        default:
          ch = '\0';
      }
      if (ch) {
        cchar_t cch = { A_NORMAL, { ch, L'\0' }};
        mvadd_wch(roomy + i / roomw, roomx + i % roomw, &cch);
      }
    }
  }
}

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

Level *
lvl_build()
{
  Level *lvl = calloc(1, sizeof(*lvl));
  int failed_attempts = 0;

  // Create random rooms

  while (/*lvl->rooms_num < 10 &&*/ failed_attempts < 100) {
    int w = 5 + rand() % 5;
    int h = 4 + rand() % 3;
    int x = 1 + rand() % (COLS - w - 2);
    int y = 1 + rand() % (LINES - h - 2);

    bool vacant = TRUE;
    for (int i = x; vacant && i < x + w; ++i)
      for (int j = y; vacant && j < y + h; ++j)
        vacant = !is_floor(lvl, i, j);
    if (!vacant) {
      ++failed_attempts;
      continue;
    }
    failed_attempts = 0;
    Room *r = mk_room_rect(x, y, w, h);
    r->next = lvl->rooms;
    lvl->rooms = r;
    lvl->rooms_num += 1;
    mvprintw(0, 0, "Rooms created: %d", lvl->rooms_num);
    render_room(lvl, r);
    refresh();
    // SMALL_SLEEP();
  }

  // Merge close ones

  for (Room *r = lvl->rooms; r; r = r->next) {
    Room *mr = NULL;
    ROOM(r);
    for (
      int ix = rx + 1, iy = ry - 1;
      ix < rx + rw - 1 && (mr = get_room(lvl, ix, iy));
      ix += mr ? mr->w : 1
    ) {
      attron(COLOR_PAIR(2));
      render_room(lvl, r);
      render_room(lvl, mr);
      attroff(COLOR_PAIR(2));
      refresh();
      SLEEP();

      ROOM(mr);
      int x = min(rx, mrx);
      int y = min(ry, mry);
      int w = x - max(rx + rw, mrx + mrw);
      int h = y - max(ry + rh, mry + mrh);
      Room *neu = mk_room_rect(x, y, w, h);
      neu->is_rect = FALSE;
      Tile *ts = neu->tiles = calloc(w * h, sizeof(*ts));
      for (int i = 0; i < w * h; ++i) {
        int nx = x + i % w;
        int ny = y + i / w;
        Tile t = room_get_tile(r, nx, ny);
        if (!t)
          t = room_get_tile(mr, nx, ny);
        ts[i / w * w + i % w] = t;
      }
      attron(COLOR_PAIR(3));
      render_room(lvl, neu);
      attroff(COLOR_PAIR(3));
      refresh();
      SLEEP();

      // TODO free r & mr
    }
  }

  return lvl;
}
