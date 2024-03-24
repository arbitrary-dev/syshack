#ifndef CHARACTER_H
#define CHARACTER_H

#include "misc.h"

typedef enum {
  PLAYER,
  WANDER,
  FIGHT,
  FLIGHT,
  DEAD,
  HIGHLIGHT,
} State;

typedef struct {
  Object base;
  char   symbol;
  int x;
  int y;
  int hp;
  State  state;
} Character;

Character * ch_create(char, int, State, int, int);

#endif
