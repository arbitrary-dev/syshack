#include <stdbool.h>

#include "llist.h"
#include "character.h"
#include "level.h"

typedef struct {
  bool is_wall;
  Node *top;
  Room *room;
} Cell;

typedef struct {
  bool done;
  Character *player;
  Character *droid;
  Room *current_room;
  Node *render_queue;
} Context;
