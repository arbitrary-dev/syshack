#ifndef LEVEL_H
#define LEVEL_H

typedef struct room {
  struct room *next;
  int x;
  int y;
  int w;
  int h;
} Room;

typedef struct {
  Room *rooms;
  int   rooms_num;
} Level;

Level * lvl_build();
void lvl_render();

#endif
