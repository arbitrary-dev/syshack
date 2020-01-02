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
room_render(level, room)
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

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

static void
lvl_destroy_room(level, room)
Level *level;
Room *room;
{
  if (level->rooms == room) {
    level->rooms = room->next;
  } else {
    Room *prev = level->rooms;
    while (prev && prev->next != room)
      prev = prev->next;
    if (prev && prev->next == room)
      prev->next = room->next;
  }
  free(room->tiles);
  room->tiles = NULL;
  free(room);
}

static void
merge_rooms(lvl, r1, r2)
Level *lvl;
Room *r1, *r2;
{
  attron(COLOR_PAIR(2));
  room_render(lvl, r1);
  room_render(lvl, r2);
  attroff(COLOR_PAIR(2));
  refresh();
  SLEEP();

  ROOM(r1);
  ROOM(r2);
  int x = min(r1x, r2x);
  int y = min(r1y, r2y);
  int w = max(r1x + r1w, r2x + r2w) - x;
  int h = max(r1y + r1h, r2y + r2h) - y;

  if (!r1->tiles || r1w != w || r1h != h) {
    Tile *temp_ts = calloc(w * h, sizeof(*temp_ts));
    for (int i = 0; i < w * h; ++i) {
      int nx = x + i % w;
      int ny = y + i / w;
      Tile t = room_get_tile(r1, nx, ny);
      temp_ts[i / w * w + i % w] = t;
    }
    if (r1->tiles)
      free(r1->tiles);
    r1->tiles = temp_ts;
    r1->is_rect = FALSE;
  }

  r1->x = x;
  r1->y = y;
  r1->w = w;
  r1->h = h;

  for (int i = 0; i < w * h; ++i) {
    int nx = x + i % w;
    int ny = y + i / w;
    Tile t = room_get_tile(r2, nx, ny);
    if (t != T_EMPTY)
      r1->tiles[i / w * w + i % w] = t;
  }

  attron(COLOR_PAIR(3));
  room_render(lvl, r1);
  mvprintw(y+1, x+1, "%dx%d w%d h%d", x, y, w, h);
  attroff(COLOR_PAIR(3));
  refresh();
  SLEEP();

  lvl_destroy_room(lvl, r2);
}

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
    room_render(lvl, r);
    refresh();
    // SMALL_SLEEP();
  }

  // Merge close ones

  for (Room *r = lvl->rooms; r; r = r->next) {
    Room *mr = NULL;
    ROOM(r);

    // Scan north
    for (
      int ix = rx + 1, iy = ry - 1, skip = 1;
      ix < rx + rw - 1;
    ) {
      mr = get_room(lvl, ix, iy);
      if (mr && mr != r) {
        ix = mr->x + mr->w;
        merge_rooms(lvl, r, mr);
      } else {
        ++ix;
      }
    }

    // Scan east
    for (
      int ix = rx + rw, iy = ry + 1;
      iy < ry + rh - 1;
    ) {
      mr = get_room(lvl, ix, iy);
      if (mr && mr != r) {
        iy = mr->y + mr->h;
        merge_rooms(lvl, r, mr);
      } else {
        ++iy;
      }
    }

    // Scan south
    for (
      int ix = rx + 1, iy = ry + rh;
      ix < rx + rw - 1;
    ) {
      mr = get_room(lvl, ix, iy);
      if (mr && mr != r) {
        ix = mr->x + mr->w;
        merge_rooms(lvl, r, mr);
      } else {
        ++ix;
      }
    }

    // Scan west
    for (
      int ix = rx - 1, iy = ry + 1;
      iy < ry + rh - 1;
    ) {
      mr = get_room(lvl, ix, iy);
      if (mr && mr != r) {
        iy = mr->y + mr->h;
        merge_rooms(lvl, r, mr);
      } else {
        ++iy;
      }
    }
  }

  return lvl;
}
