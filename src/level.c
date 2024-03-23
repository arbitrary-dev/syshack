#include "ncurses.h"
#include "level.h"
#include <ncursesw/curses.h>
#include <stdio.h>

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

static Tile
room_get_tile(room, x, y)
const Room *room;
int x, y;
{
  ROOM(room);
  if (x < roomx || x >= roomx + roomw ||
      y < roomy || y >= roomy + roomh)
    return T_EMPTY;
  if (!room->tiles)
    return x > roomx && x < roomx + roomw - 1 &&
           y > roomy && y < roomy + roomh - 1
           ? T_FLOOR
           : T_WALL;
  return room->tiles[(y - roomy) * roomw + x - roomx];
}

static void
room_set_tile(room, x, y, tile)
Room *room;
int x, y;
Tile tile;
{
  ROOM(room);
  if (x < roomx || x >= roomx + roomw ||
      y < roomy || y >= roomy + roomh)
    return;
  if (!room->tiles)
    room->tiles = calloc(roomw * roomh, sizeof(*room->tiles));
  room->tiles[(y - roomy) * roomw + x - roomx] = tile;
}

Room *
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
        y >= ry && y < ry + rh) {
      if (r->is_rect)
        return r;
      Tile t = room_get_tile(r, x, y);
      if (t != T_EMPTY)
        return r;
    }
  }
  return NULL;
}

static bool
room_is_floor(room, x, y)
const Room *room;
int x, y;
{
  return room_get_tile(room, x, y) == T_FLOOR;
}

bool
room_is_wall(room, x, y)
const Room *room;
int x, y;
{
  return room_get_tile(room, x, y) == T_WALL;
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

static bool
cell_pattern(room, i, pat, x, y)
const Room *room;
const int i, pat, x, y;
{
  Tile t = room_get_tile(room, x, y);
  if (pat & (1 << i) && !(t & (T_WALL | T_DOOR)))
    return false;
  if (pat & (1 << i + 8) && !(t & T_FLOOR))
    return false;
  return true;
}

#define CHK(i, x, y) cell_pattern(room, i, pat, x, y)

/* `pat` hex describes the following patterns:
 *
 * 0x8258 =
 * 100 00 010  010 11 000
 * floor-----  walls-----
 *
 *    N           N
 *   100         010
 * W 0 0 E     W 1 1 E
 *   010         000
 *    S           S
 */
static bool
room_pattern(room, pat, x, y)
const Room *room;
const int pat, x, y;
{
  return
    CHK(1, x    , y + 1) &&
    CHK(6, x    , y - 1) &&
    CHK(3, x + 1, y    ) &&
    CHK(4, x - 1, y    ) &&

    CHK(0, x + 1, y + 1) &&
    CHK(7, x - 1, y - 1) &&
    CHK(2, x - 1, y + 1) &&
    CHK(5, x + 1, y - 1);
}

#define CHK_PATTERN(pat) room_pattern(room, pat, x, y)

static void
room_render(level, room)
const Level *level;
const Room *room;
{
  ROOM(room);
  if (!room->tiles) {
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
    for (int i = 0; i < roomw * roomh; ++i) {
      int x = roomx + i % roomw;
      int y = roomy + i / roomw;
      Tile t = room_get_tile(room, x, y);
      wchar_t ch;
      switch(t) {

        case T_FLOOR:
          ch = FLOOR;
          break;

        case T_DOOR:
          if (room_is_wall(room, x - 1, y) && room_is_wall(room, x + 1, y))
            ch = D_H;
          else
            ch = D_V;
          break;

        case T_WALL: {
          if (CHK_PATTERN(0x815A) || CHK_PATTERN(0x245A)) {
            ch = W_X;
          } else if (CHK_PATTERN(0x8258) || CHK_PATTERN(0x2258)) {
            ch = W_N;
          } else if (CHK_PATTERN(0x304A) || CHK_PATTERN(0x114A)) {
            ch = W_E;
          } else if (CHK_PATTERN(0x441A) || CHK_PATTERN(0x411A)) {
            ch = W_S;
          } else if (CHK_PATTERN(0x8852) || CHK_PATTERN(0x0C52)) {
            ch = W_W;
          } else if (CHK_PATTERN(0xE01F) || CHK_PATTERN(0x07F8)) {
            ch = W_H;
          } else if (CHK_PATTERN(0x946B) || CHK_PATTERN(0x29D6)) {
            ch = W_V;
          } else if (CHK_PATTERN(0x1248) || CHK_PATTERN(0x2048)) {
            ch = W_SW;
          } else if (CHK_PATTERN(0x500A) || CHK_PATTERN(0x010A)) {
            ch = W_NW;
          } else if (CHK_PATTERN(0x4812) || CHK_PATTERN(0x0412)) {
            ch = W_NE;
          } else if (CHK_PATTERN(0x0A50) || CHK_PATTERN(0x8050)) {
            ch = W_SE;
          } else if (CHK_PATTERN(0x4010) || CHK_PATTERN(0x4008) ||
                     CHK_PATTERN(0x0210) || CHK_PATTERN(0x0208)) {
            ch = W_H;
          } else if (CHK_PATTERN(0x1040) || CHK_PATTERN(0x1002) ||
                     CHK_PATTERN(0x0840) || CHK_PATTERN(0x0802)) {
            ch = W_V;
          } else {
            ch = WALLS[rand() % 11];
          }
          break;
        }

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
  SMALL_SLEEP();

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

  for (int i = 0; i < w * h; ++i) {
    int nx = x + i % w;
    int ny = y + i / w;
    Tile t = room_get_tile(r1, nx, ny);
    if (t == T_WALL) {
      Tile tw = room_get_tile(r1, nx - 1, ny);
      Tile te = room_get_tile(r1, nx + 1, ny);
      if (tw == T_FLOOR && te == T_FLOOR) {
        room_set_tile(r1, nx, ny, T_FLOOR);
        continue;
      }

      Tile tn = room_get_tile(r1, nx, ny - 1);
      Tile ts = room_get_tile(r1, nx, ny + 1);
      if (tn == T_FLOOR && ts == T_FLOOR) {
        r1->tiles[i / w * w + i % w] = T_FLOOR;
        continue;
      }
    }
  }

  attron(COLOR_PAIR(3));
  room_render(lvl, r1);
  //mvprintw(y+1, x+1, "%dx%d w%d h%d", x, y, w, h);
  attroff(COLOR_PAIR(3));
  refresh();
  SMALL_SLEEP();

  lvl_destroy_room(lvl, r2);
  lvl->rooms_num -= 1;
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
      int ix = rx + 1, iy = ry - 1;
      ix < rx + rw - 1;
    ) {
      mr = get_room(lvl, ix, iy);
      if (mr && mr != r && room_is_floor(mr, ix, iy)) {
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
      if (mr && mr != r && room_is_floor(mr, ix, iy)) {
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
      if (mr && mr != r && room_is_floor(mr, ix, iy)) {
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
      if (mr && mr != r && room_is_floor(mr, ix, iy)) {
        iy = mr->y + mr->h;
        merge_rooms(lvl, r, mr);
      } else {
        ++iy;
      }
    }
  }
  mvprintw(1, 0, "Rooms after merge: %d", lvl->rooms_num);

  // Connect rooms
  for (Room *r = lvl->rooms; r; r = r->next) {
    ROOM(r);
    for (int i = 0; i < rw * rh; ++i) {
      int x = rx + i % rw;
      int y = ry + i / rw;
      if (!room_is_wall(r, x, y))
        continue;
      Room *r2;
      // TODO move rooms around to maximize doorable perimiter
      if ((room_is_wall(r, x, y - 1) && room_is_wall(r, x, y + 1)
          && ((room_is_floor(r, x + 1, y) && (r2 = get_room(lvl, x - 1, y))
               && r != r2 && room_is_floor(r2, x - 1, y))
           || (room_is_floor(r, x - 1, y) && (r2 = get_room(lvl, x + 1, y))
               && r != r2 && room_is_floor(r2, x + 1, y))))
       || (room_is_wall(r, x - 1, y) && room_is_wall(r, x + 1, y)
         && ((room_is_floor(r, x, y + 1) && (r2 = get_room(lvl, x, y - 1))
              && r != r2 && room_is_floor(r2, x, y - 1))
          || (room_is_floor(r, x, y - 1) && (r2 = get_room(lvl, x, y + 1))
              && r != r2 && room_is_floor(r2, x, y + 1))))) {
        // TODO remember these in an array and then pick up one
        // or two (on a different wall) randomly.
        room_set_tile(r, x, y, T_DOOR);
        room_set_tile(r2, x, y, T_DOOR);

        room_render(lvl, r);
        room_render(lvl, r2);
      }
    }
  }
  refresh();

  return lvl;
}
