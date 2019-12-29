#ifndef LEVEL_H
#define LEVEL_H

#include <stdlib.h>

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

#define FLOOR L'\u00B7'

#define ROOM(r) int r##x = (r)->x, r##y = (r)->y, r##w = (r)->w, r##h = (r)->h

typedef enum {
  T_EMPTY = 0,
  T_WALL,
  T_FLOOR,
  T_DOOR,
} Tile;

typedef struct room {
  struct room *next;
  int x;
  int y;
  int w;
  int h;
  bool is_rect;
  Tile *tiles;
} Room;

typedef struct {
  Room *rooms;
  int   rooms_num;
} Level;

Level * lvl_build();

#endif
