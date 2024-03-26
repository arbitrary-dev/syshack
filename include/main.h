#include <stdbool.h>

#include "character.h"
#include "level.h"
#include "llist.h"
#include "ncurses.h"

typedef struct {
	bool  is_wall;
	Node *top;
	Room *room;
} Cell;

typedef struct {
	bool       done;
	Character *player;
	Character *droid;
	Room      *current_room;
	Node      *render_queue;
} Context;

typedef struct {
	size_t   x;
	size_t   y;
	cchar_t *text;
} Snap;
