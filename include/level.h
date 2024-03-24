#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include <stdlib.h>

// Walls

#define W_H L'\u2500'
#define W_V L'\u2502'

#define W_NW L'\u250C'
#define W_NE L'\u2510'
#define W_SW L'\u2514'
#define W_SE L'\u2518'

#define W_N L'\u2534'
#define W_E L'\u251C'
#define W_S L'\u252C'
#define W_W L'\u2524'
#define W_X L'\u253C'

static const int WALLS[11] =
  { W_H, W_V, W_NW, W_NE, W_SW, W_SE, W_N, W_E, W_S, W_W, W_X };

// Doors
#define D_H L'\u2501'
#define D_V L'\u2503'

#define FLOOR L'\u00B7'

#define ROOM(r) int r##x = (r)->x, r##y = (r)->y, r##w = (r)->w, r##h = (r)->h

typedef enum {
  T_EMPTY = 0,
  T_FLOOR = 1,
  T_WALL = 1 << 1,
  T_DOOR = 1 << 2,
} Tile;

typedef struct room {
  struct room *next; // TODO llist? :'D
  int x;
  int y;
  int w;
  int h;
  bool is_rect;
  Tile *tiles;
  // TODO walls
} Room;

typedef struct {
  Room *rooms;
  int   rooms_num;
} Level;

bool room_is_wall(const Room *room, int x, int y);
bool room_is_floor(const Room *room, int x, int y);
Room * get_room(const Level *lvl, int x, int y);

Level * lvl_build();

#endif
