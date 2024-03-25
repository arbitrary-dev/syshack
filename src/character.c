#include <stdlib.h>

#include "character.h"

Character *
ch_create(char symbol, int hp, State state, int x, int y)
{
	Character *c = malloc(sizeof(Character));

	c->base.type = CHARACTER;
	c->symbol    = symbol;
	c->hp        = hp;
	c->state     = state;
	c->x         = x;
	c->y         = y;

	return c;
}
