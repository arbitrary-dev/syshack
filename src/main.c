#include <assert.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "level.h"
#include "main.h"
#include "misc.h"

static Cell   **map = NULL;
static Context *ctx = NULL;

static void
init(void)
{
	setlocale(LC_ALL, "");
	initscr();
	keypad(stdscr, TRUE); // interpret special keys, like KEY_UP
	cbreak();
	noecho();
	nonl();
	curs_set(0);

	use_default_colors();
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_BLACK);

	srand(time(NULL));
}

bool
is_blocked(const Cell *c)
{
	if (c->is_wall) {
		return true;
	} else if (!c->top) {
		return false;
	}

	Object *o = c->top->value;

	switch (o->type) {
	case CHARACTER:
		return ((Character *) o)->state != DEAD;
	default:
		return false;
	}
}

void
ch_render(Character *c)
{
	switch (c->state) {
	case DEAD:
		attron(COLOR_PAIR(1));
		break;
	default:
		break;
	}

	mvaddch(c->y, c->x, c->symbol);

	switch (c->state) {
	case DEAD:
		attroff(COLOR_PAIR(1));
		break;
	default:
		break;
	}
}

void
cell_render(size_t x, size_t y)
{
	if (x < 0 || y < 0 || x >= COLS || y >= LINES) {
		return;
	}
	Node *n = map[x][y].top;
	if (!n) {
		mvaddch(y, x, ' ');
		return;
	}

	Object *o = n->value;
	switch (o->type) {
	case ITEM:
		mvaddch(y, x, '.');
		break;
	case CHARACTER:
		ch_render((Character *) o);
		break;
	}
}

static void
ch_move(Character *c, int dx, int dy, bool bypass_block)
{
	// Source
	int sx = c->x;
	int sy = c->y;
	assert(sx >= 0 && sy >= 0 && sx < COLS && sy < LINES);
	Cell *s = &map[sx][sy];

	// Target
	int tx = sx + dx;
	int ty = sy + dy;
	assert(tx >= 0 && ty >= 0 && tx < COLS && ty < LINES);
	Cell *t = &map[tx][ty];

	// TODO remove bypass
	if (!bypass_block && is_blocked(t)) {
		return;
	}

	Node *n = s->top; // FIXME find the actual Node containing Character c
	s->top  = n->next;
	free(n);

	cell_render(sx, sy);

	if (c == ctx->player && t->room && t->room != ctx->current_room) {
		attron(COLOR_PAIR(4) | A_BOLD);
		room_render(ctx->current_room);
		attroff(COLOR_PAIR(4) | A_BOLD);

		ctx->current_room = t->room;

		room_render(t->room);

		ch_render(ctx->droid);
	}

	t->top = l_prepend(t->top, c);
	c->x   = tx;
	c->y   = ty;
	ch_render(c);
}

static void
ch_move_towards(Character *c, Character *to)
{
	int dx = (to->x - c->x > 0) - (to->x - c->x < 0);
	int dy = (to->y - c->y > 0) - (to->y - c->y < 0);
	ch_move(c, dx, dy, false);
}

void
ctx_render_enqueued_snapshots()
{
	Node *i;
	while (i = ctx->render_queue) {
		Snap *snap = i->value;
		mvadd_wchstr(snap->y, snap->x, snap->text);
		free(snap->text);
		snap->text = NULL;
		free(snap);

		ctx->render_queue = i->next;

		i->value = NULL;
		i->next  = NULL;
		free(i);
	}
}

void
ctx_enqueue_region_snapshot(size_t x, size_t y, size_t L)
{
	cchar_t *prev = calloc(L + 1, sizeof(cchar_t));
	mvin_wchnstr(y, x, prev, L);

	Snap *snap = malloc(sizeof(Snap));

	snap->x    = x;
	snap->y    = y;
	snap->text = prev;

	ctx->render_queue = l_prepend(ctx->render_queue, snap);
}

void
render_text(size_t x, size_t y, const char *str)
{
	ctx_enqueue_region_snapshot(x, y, strlen(str));
	mvprintw(y, x, str); // FIXME wraps on edges
	refresh();
}

void
ch_render_text(const Character *ch, const char *str)
{
	// By default text rendered in upper right corner
	int x = ch->x + 1;
	int y = ch->y - 1;

	size_t L = strlen(str);

	// Now check for other characters around
	const Character *och = (ch == ctx->player) ? ctx->droid : ctx->player;

	int ox = och->x;
	int oy = och->y;

	if (oy == y && ox >= x - 1 && ox <= x + L) {
		x -= L + 1;
		y += 2;
	}

	// Now check window boundaries
	if (y < 0) {
		y = ch->y + 1;
	} else if (y >= LINES) {
		y = ch->y - 1;
	}

	if (x < 0) {
		x = ch->x + 1;
	} else if (x + L > COLS) {
		x = ch->x - L;
	}

	render_text(x, y, str);
}

static void
ch_attack_side(Character *c, int dx, int dy)
{
	int ay = c->y + dy;
	int ax = c->x + dx;

	if (ax < 0 || ay < 0 || ax >= COLS || ay >= LINES) {
		return;
	}

	Object *o = (map[ax][ay].top) ? map[ax][ay].top->value : NULL;

	if (!o) {
		if (map[ax][ay].is_wall) {
			ch_render_text(c, "The wall?");
			SLEEP();
			SLEEP();
		}
		return;
	}

	switch (o->type) {
	case CHARACTER:
	{
		// TODO refactor to a function
		Character *t = (Character *) o;

		int damage = 0;

		if (t && t->x == ax && t->y == ay && t->state != DEAD
		    && (damage = rand() % 5))
		{

			if ((t->hp -= damage) <= 0) {
				bool is_player = t->state == PLAYER;

				t->state = DEAD;

				if (is_player) {
					ch_render(t);
					attron(COLOR_PAIR(2));
					ch_render_text(t, "WASTED!");
					SLEEP();
					SLEEP();
					SLEEP();
					SLEEP();
					ctx->done = TRUE;
				}
			} else {
				State s = t->state;

				t->state = DEAD;

				ch_render(t);

				char str[4];
				sprintf(str, "-%d", damage);
				attron(COLOR_PAIR(2));
				ch_render_text(t, str);
				attroff(COLOR_PAIR(2));

				SMALL_SLEEP();
				switch (s) {
				case WANDER:
				case FIGHT:
					if (t->hp > 5) {
						t->state = FIGHT;
					} else {
						t->state = FLIGHT;
					}
					break;

				default:
					t->state = s;
				}
				ch_render(t);
				refresh();

				SLEEP();
				ctx_render_enqueued_snapshots();
				refresh();
			}
			ch_render(t);
		} else {
			ch_render_text(c, "Miss!");
			SLEEP();
		}
	} break;

	default:
		break;
	}
}

static void
ch_attack_char(Character *attacker, Character *target)
{
	int dx = target->x - attacker->x;
	int dy = target->y - attacker->y;
	ch_attack_side(attacker, dx, dy);
}

static bool
is_near(Character *c1, Character *c2)
{
	return abs(c1->x - c2->x) <= 1 && abs(c1->y - c2->y) <= 1;
}

void
move_droid()
{
	Character *d = ctx->droid;
	switch (d->state) {
	case FIGHT:
	{
		Character *p = ctx->player;
		if (is_near(d, p)) {
			ch_attack_char(d, p);
		} else {
			ch_move_towards(d, p);
		}
		break;
	}
	default:
		ch_move(d, rand() % 3 - 1, rand() % 3 - 1, false);
	}
}

void
do_attack(Character *player)
{
	ch_render_text(player, "Where?");
	int ch = getch();
	ctx_render_enqueued_snapshots();

	switch (ch) {
	case 'h':
	case KEY_LEFT:
		ch_attack_side(player, -1, 0);
		break;

	case 'l':
	case KEY_RIGHT:
		ch_attack_side(player, 1, 0);
		break;

	case 'j':
	case KEY_DOWN:
		ch_attack_side(player, 0, 1);
		break;

	case 'k':
	case KEY_UP:
		ch_attack_side(player, 0, -1);
		break;

	case 'y':
		ch_attack_side(player, -1, -1);
		break;

	case 'u':
		ch_attack_side(player, 1, -1);
		break;

	case 'b':
		ch_attack_side(player, -1, 1);
		break;

	case 'n':
		ch_attack_side(player, 1, 1);
		break;

	default:
		ch_render_text(player, "What?!");
		SLEEP();
	}
}

static Character *
make_player(Room *room, int x, int y)
{
	assert(x >= 0 && y >= 0 && x < COLS && y < LINES);

	Character *ch = ch_create('@', 15, PLAYER, x, y);

	ctx->player       = ch;
	ctx->current_room = room;

	Cell *c = &map[x][y];

	c->top = l_prepend(c->top, ch);

	ch_render(ch);

	return ch;
}

static Character *
make_droid(int x, int y)
{
	assert(x >= 0 && y >= 0 && x < COLS && y < LINES);

	Character *ch = ch_create('d', 15, WANDER, x, y);

	ctx->droid = ch;

	Cell *c = &map[x][y];

	c->top = l_prepend(c->top, ch);

	ch_render(ch);

	return ch;
}

int
main(int argc, char *argv[])
{
	init();

	/*
	   for (int i = 0; i < 16 * 8; ++i) {
	   wchar_t str[2] = { L'\u2500' + i, L'\0' };
	   mvprintw(i / 16, (i % 16) * 8,"%ls %x", str, 0x2500 + i);
	   }

	   cchar_t wch = { A_NORMAL | COLOR_PAIR(2), { L'\u2534', L'\0' }};
	   mvadd_wch(LINES - 1, COLS - 1, &wch);
	   */

	ctx = malloc(sizeof(Context));

	ctx->done         = false;
	ctx->player       = NULL;
	ctx->droid        = NULL;
	ctx->current_room = NULL;
	ctx->render_queue = NULL;

	map = calloc(COLS, sizeof(Cell *));
	for (int i = 0; i < COLS; ++i) {
		map[i] = calloc(LINES, sizeof(Cell));
		for (int j = 0; j < LINES; ++j) {
			map[i][j] = (Cell) { false, NULL, NULL };
		}
	}

	Character *player = NULL, *droid = NULL;

	Level *lvl = lvl_build();
	for (Room *r = lvl->rooms; r; r = r->next) {
		ROOM(r);
		bool is_last_room = !r->next;
		for (int x = rx; x < rx + rw; ++x) {
			for (int y = ry; y < ry + rh; ++y) {
				assert(x >= 0 && y >= 0 && x < COLS && y < LINES);
				if (room_is_wall(r, x, y)) {
					map[x][y].is_wall = true;
				} else if (room_is_floor(r, x, y)) {
					if (is_last_room || rand() % 6 == 0) {
						if ((is_last_room || rand() % 2) && !player) {
							player = make_player(r, x, y);
						} else if (!droid) {
							droid = make_droid(x, y);
						}
					}
					map[x][y].room = r;
				}
			}
		}
	}

	assert(player);
	assert(droid);

	// Indicate current location
	refresh();
	SLEEP();
	SLEEP();
	SLEEP();
	for (int i = 1; i <= 6; ++i) {
		if (i % 2) {
			attron(COLOR_PAIR(3) | A_REVERSE);
		}
		room_render(ctx->current_room);
		ch_render(player);
		ch_render(droid);
		attroff(A_COLOR | A_REVERSE);
		refresh();
		SLEEP();
		SLEEP();
		SLEEP();
	}

	attron(COLOR_PAIR(4) | A_BOLD);
	for (Room *r = lvl->rooms; r; r = r->next) {
		room_render(r);
	}
	attroff(COLOR_PAIR(4) | A_BOLD);

	room_render(ctx->current_room);
	ch_render(player);
	ch_render(droid);

	int ch;
	while (!ctx->done && (ch = getch()) > 0) {
		switch (ch) {
		case 'q':
			ctx->done = TRUE;
			break;

		case 'h':
		case 'H':
		case KEY_LEFT:
			ch_move(player, -1, 0, ch == 'H');
			break;

		case 'l':
		case 'L':
		case KEY_RIGHT:
			ch_move(player, 1, 0, ch == 'L');
			break;

		case 'j':
		case 'J':
		case KEY_DOWN:
			ch_move(player, 0, 1, ch == 'J');
			break;

		case 'k':
		case 'K':
		case KEY_UP:
			ch_move(player, 0, -1, ch == 'K');
			break;

		case 'y':
			ch_move(player, -1, -1, false);
			break;

		case 'u':
			ch_move(player, 1, -1, false);
			break;

		case 'b':
			ch_move(player, -1, 1, false);
			break;

		case 'n':
			ch_move(player, 1, 1, false);
			break;

		case 'a':
			do_attack(player);
			break;
		}

		if (ctx->done) {
			break;
		}

		ctx_render_enqueued_snapshots();

		if (droid->state != DEAD) {
			move_droid();
		}

		refresh();
	}

	clear();
	mvaddstr(LINES / 2, COLS / 2 - 4, "THE END!");
	refresh();
	SLEEP();
	SLEEP();
	SLEEP();
	SLEEP();
	endwin();
	exit(0);
}
