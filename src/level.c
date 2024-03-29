#include <stdio.h>
#include <stdlib.h>

#include "level.h"

static Room *
mk_room_rect(int x, int y, int w, int h)
{
	Room *r = calloc(1, sizeof(*r));

	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;

	r->is_rect   = TRUE;
	r->connected = FALSE;

	return r;
}

static Tile
room_get_tile(const Room *room, int x, int y)
{
	ROOM(room);
	if (x < roomx || x >= roomx + roomw || y < roomy || y >= roomy + roomh) {
		return T_EMPTY;
	} else if (!room->tiles) {
		// clang-format off
		return x > roomx && x < roomx + roomw - 1
		    && y > roomy && y < roomy + roomh - 1
			? T_FLOOR
			: T_WALL;
		// clang-format on
	}
	return room->tiles[(y - roomy) * roomw + x - roomx];
}

static void
room_set_tile(Room *room, int x, int y, Tile tile)
{
	ROOM(room);
	if (x < roomx || x >= roomx + roomw || y < roomy || y >= roomy + roomh) {
		return;
	} else if (!room->tiles) {
		room->tiles = calloc(roomw * roomh, sizeof(Tile));
		for (int i = 0; i < roomw * roomh; ++i) {
			int nx = roomx + i % roomw;
			int ny = roomy + i / roomw;
			// clang-format off
			room->tiles[i / roomw * roomw + i % roomw]
				= nx > roomx && nx < roomx + roomw - 1
				  && ny > roomy && ny < roomy + roomh - 1
					? T_FLOOR
					: T_WALL;
			// clang-format on
		}
	}
	room->tiles[(y - roomy) * roomw + x - roomx] = tile;
}

Room *
get_room(const Level *lvl, int x, int y)
{
	if (x < 0 || x >= COLS || y < 0 || y >= LINES) {
		return NULL;
	}
	for (Room *r = lvl->rooms; r; r = r->next) {
		ROOM(r);
		if (x >= rx && x < rx + rw && y >= ry && y < ry + rh) {
			if (r->is_rect) {
				return r;
			}
			Tile t = room_get_tile(r, x, y);
			if (t != T_EMPTY) {
				return r;
			}
		}
	}
	return NULL;
}

bool
room_is_floor(const Room *room, int x, int y)
{
	return room_get_tile(room, x, y) == T_FLOOR;
}

bool
room_is_wall(const Room *room, int x, int y)
{
	return room_get_tile(room, x, y) == T_WALL;
}

static bool
is_floor(const Level *lvl, int x, int y)
{
	Room *r = get_room(lvl, x, y);
	return r && room_is_floor(r, x, y);
}

static void
render_row(int x, int y, int w, wchar_t start, wchar_t middle, wchar_t end)
{
	move(y, x);
	wchar_t row[w + 1];
	row[w] = L'\0';
	for (int i = 0; i < w; ++i) {
		wchar_t ch;
		if (i == 0) {
			ch = start;
		} else if (i == w - 1) {
			ch = end;
		} else {
			ch = middle;
		}
		row[i] = ch;
	}
	addwstr(row);
}

static bool
cell_pattern(const Room *room, int i, int pat, int x, int y)
{
	Tile t = room_get_tile(room, x, y);
	if (pat & (1 << i) && !(t & (T_WALL | T_DOOR))) {
		return FALSE;
	} else if (pat & (1 << i + 8) && !(t & T_FLOOR)) {
		return FALSE;
	}
	return TRUE;
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
room_pattern(const Room *room, int pat, int x, int y)
{
	// clang-format off
	return CHK(7, x - 1, y - 1) && CHK(6, x, y - 1) && CHK(5, x + 1, y - 1)
	    && CHK(4, x - 1, y    ) /*       :P      */ && CHK(3, x + 1, y    )
	    && CHK(2, x - 1, y + 1) && CHK(1, x, y + 1) && CHK(0, x + 1, y + 1);
	// clang-format on
}

#define CHK_PATTERN(pat) room_pattern(room, pat, x, y)

void
room_render(const Room *room)
{
	ROOM(room);
	if (!room->tiles) {
		for (int j = 0; j < roomh; ++j) {
			wchar_t start, middle, end;
			if (j == 0) {
				start  = W_NW;
				middle = W_H;
				end    = W_NE;
			} else if (j == roomh - 1) {
				start  = W_SW;
				middle = W_H;
				end    = W_SE;
			} else {
				start  = W_V;
				middle = FLOOR;
				end    = W_V;
			}
			render_row(roomx, roomy + j, roomw, start, middle, end);
		}
		// mvprintw(roomy + 1, roomx + 1, "%dx%d", roomw, roomh);
	} else {
		for (int i = 0; i < roomw * roomh; ++i) {
			int  x = roomx + i % roomw;
			int  y = roomy + i / roomw;
			Tile t = room_get_tile(room, x, y);

			wchar_t ch;

			switch (t) {
			case T_FLOOR:
				ch = FLOOR;
				break;

			case T_DOOR:
				// clang-format off
				if (room_is_wall(room, x - 1, y)
				 && room_is_wall(room, x + 1, y))
				{
					ch = D_H;
				} else {
					ch = D_V;
				}
				break;
				// clang-format on

			case T_WALL:
				// clang-format off
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
				} else if (CHK_PATTERN(0x4010) || CHK_PATTERN(0x4008)
				        || CHK_PATTERN(0x0210) || CHK_PATTERN(0x0208))
				{
					ch = W_H;
				} else if (CHK_PATTERN(0x1040) || CHK_PATTERN(0x1002)
				        || CHK_PATTERN(0x0840) || CHK_PATTERN(0x0802))
				{
					ch = W_V;
				} else {
					ch = WALLS[rand() % 11]; // FIXME pick one and persist
				}
				break;
				// clang-format on

			default:
				ch = '\0';
			}

			if (ch) {
				cchar_t cch = { A_NORMAL, { ch, L'\0' } };
				mvadd_wch(roomy + i / roomw, roomx + i % roomw, &cch);
			}
		}
	}
}

void
room_clean(const Room *room)
{
	ROOM(room);
	if (!room->tiles) {
		for (int i = 0; i < roomw; ++i) {
			for (int j = 0; j < roomh; ++j) {
				mvaddch(roomy + j, roomx + i, ' ');
			}
		}
	} else {
		for (int i = 0; i < roomw * roomh; ++i) {
			int  x = roomx + i % roomw;
			int  y = roomy + i / roomw;
			Tile t = room_get_tile(room, x, y);

			if (t) {
				mvaddch(y, x, ' ');
			}
		}
	}
}

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

static void
lvl_destroy_room(Level *level, Room *room)
{
	if (level->rooms == room) {
		level->rooms = room->next;
	} else {
		Room *prev = level->rooms;
		while (prev && prev->next != room) {
			prev = prev->next;
		}
		if (prev && prev->next == room) {
			prev->next = room->next;
		}
	}
	free(room->tiles);
	room->tiles = NULL;
	free(room);
}

static void
merge_rooms(Level *lvl, Room *r1, Room *r2)
{
	attron(COLOR_PAIR(2));
	room_render(r1);
	room_render(r2);
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
			int  nx = x + i % w;
			int  ny = y + i / w;
			Tile t  = room_get_tile(r1, nx, ny);

			temp_ts[i / w * w + i % w] = t;
		}
		if (r1->tiles) {
			free(r1->tiles);
		}
		r1->tiles   = temp_ts;
		r1->is_rect = FALSE;
	}

	r1->x = x;
	r1->y = y;
	r1->w = w;
	r1->h = h;

	for (int i = 0; i < w * h; ++i) {
		int  nx = x + i % w;
		int  ny = y + i / w;
		Tile t  = room_get_tile(r2, nx, ny);
		if (t != T_EMPTY) {
			r1->tiles[i / w * w + i % w] = t;
		}
	}

	for (int i = 0; i < w * h; ++i) {
		int  nx = x + i % w;
		int  ny = y + i / w;
		Tile t  = room_get_tile(r1, nx, ny);
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
	room_render(r1);
	// mvprintw(y+1, x+1, "%dx%d w%d h%d", x, y, w, h);
	attroff(COLOR_PAIR(3));
	refresh();
	SMALL_SLEEP();

	lvl_destroy_room(lvl, r2);
	lvl->rooms_num -= 1;
}

Level *
lvl_build()
{
	Level *lvl = malloc(sizeof(Level));

	int failed_attempts = 0;

	// Create random rooms
	while (/*lvl->rooms_num < 10 &&*/ failed_attempts < 100) {
		int w = 5 + rand() % 5;
		int h = 4 + rand() % 3;
		int x = 1 + rand() % (COLS - w - 2);
		int y = 1 + rand() % (LINES - h - 2);

		bool vacant = TRUE;
		for (int i = x; vacant && i < x + w; ++i) {
			for (int j = y; vacant && j < y + h; ++j) {
				vacant = !is_floor(lvl, i, j);
			}
		}
		if (!vacant) {
			++failed_attempts;
			continue;
		}
		failed_attempts = 0;
		Room *r         = mk_room_rect(x, y, w, h);
		r->next         = lvl->rooms;
		lvl->rooms      = r;
		lvl->rooms_num += 1;
		mvprintw(0, 0, "Rooms created: %d", lvl->rooms_num);
		room_render(r);
		refresh();
		SMALL_SLEEP();
	}

	// Merge close ones
	for (Room *r = lvl->rooms; r; r = r->next) {
		Room *mr = NULL;
		ROOM(r);

		// Scan north
		for (int ix = rx + 1, iy = ry - 1; ix < rx + rw - 1;) {
			mr = get_room(lvl, ix, iy);
			if (mr && mr != r && room_is_floor(mr, ix, iy)) {
				ix = mr->x + mr->w;
				merge_rooms(lvl, r, mr);
			} else {
				++ix;
			}
		}

		// Scan east
		for (int ix = rx + rw, iy = ry + 1; iy < ry + rh - 1;) {
			mr = get_room(lvl, ix, iy);
			if (mr && mr != r && room_is_floor(mr, ix, iy)) {
				iy = mr->y + mr->h;
				merge_rooms(lvl, r, mr);
			} else {
				++iy;
			}
		}

		// Scan south
		for (int ix = rx + 1, iy = ry + rh; ix < rx + rw - 1;) {
			mr = get_room(lvl, ix, iy);
			if (mr && mr != r && room_is_floor(mr, ix, iy)) {
				ix = mr->x + mr->w;
				merge_rooms(lvl, r, mr);
			} else {
				++ix;
			}
		}

		// Scan west
		for (int ix = rx - 1, iy = ry + 1; iy < ry + rh - 1;) {
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

	// Magic offsets matrices
	// clang-format off
	int omx[49]
		= { 0,  0,  1,  1,  1,  0, -1, -1, -1,  0,  1,  2,  2,  2,  2,  2,  1,
		    0, -1, -2, -2, -2, -2, -2, -1,  0,  1,  2,  3,  3,  3,  3,  3,  3,
		    3,  2,  1,  0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1 };
	int omy[49]
		= { 0, -1, -1,  0,  1,  1,  1,  0, -1, -2, -2, -2, -1,  0,  1,  2,  2,
		    2,  2,  2,  1,  0, -1, -2, -2, -3, -3, -3, -3, -2, -1,  0,  1,  2,
		    3,  3,  3,  3,  3,  3,  3,  2,  1,  0, -1, -2, -3, -3, -3 };
	// clang-format on

	// Connect rooms
	for (Room *r = lvl->rooms; r; r = r->next) {
		int shift = 0;
		ROOM(r);

		do {
			bool invalid_shift = FALSE;

			int ox = omx[shift];
			int oy = omy[shift];

			if (shift) {
				// Check if no floor tile in r2, otherwise invalid shift
				for (int i = 0; i < rw * rh; ++i) {
					int x = rx + i % rw;
					int y = ry + i / rw;
					if (room_get_tile(r, x, y) == T_EMPTY) {
						continue;
					}

					// Shifted coordinates for a second room
					int sx = x + ox;
					int sy = y + oy;

					if (sx < 0 || sx >= COLS || sy < 0 || sy >= LINES
					    || rx + ox < 0 || rx + rw - 1 + ox >= COLS
					    || ry + oy < 0 || ry + rh - 1 + oy >= LINES)
					{
						invalid_shift = TRUE;
						break;
					}

					Room *r2 = get_room(lvl, sx, sy);

					if (r2 && r != r2 && room_is_floor(r2, sx, sy)) {
						invalid_shift = TRUE;
						break;
					}
				}

				if (invalid_shift) {
					continue;
				}
			}

			for (int i = 0; i < rw * rh; ++i) {
				int x = rx + i % rw;
				int y = ry + i / rw;
				if (!room_is_wall(r, x, y)) {
					continue;
				}

				// Shifted coordinates for a second room
				int sx = x + ox;
				int sy = y + oy;

				Room *r2;

				// clang-format off
				if ((
					room_is_wall(r, x, y - 1) && room_is_wall(r, x, y + 1)
					&& ((
						room_is_floor(r, x + 1, y)
						&& (r2 = get_room(lvl, sx - 1, sy)) && r != r2
						&& room_is_floor(r2, sx - 1, sy)
					) || (
						room_is_floor(r, x - 1, y)
						&& (r2 = get_room(lvl, sx + 1, sy)) && r != r2
						&& room_is_floor(r2, sx + 1, sy)
					))
				) || (
					room_is_wall(r, x - 1, y) && room_is_wall(r, x + 1, y)
					&& ((
						room_is_floor(r, x, y + 1)
						&& (r2 = get_room(lvl, sx, sy - 1)) && r != r2
						&& room_is_floor(r2, sx, sy - 1)
					) || (
						room_is_floor(r, x, y - 1)
						&& (r2 = get_room(lvl, sx, sy + 1)) && r != r2
						&& room_is_floor(r2, sx, sy + 1)
					))
				))
				// clang-format on
				{
					// TODO remember these in an array and then pick up one
					// or two (on a different wall) randomly.
					room_set_tile(r, x, y, T_DOOR);
					room_set_tile(r2, sx, sy, T_DOOR);

					r->connected  = TRUE;
					r2->connected = TRUE;

					attron(COLOR_PAIR(3));
					room_render(r2);
					attroff(A_COLOR);
				}
			}

			// Shift the room
			if (r->connected && shift) {
				room_clean(r);
				r->x += ox;
				r->y += oy;
			}

			if (r->connected) {
				attron(COLOR_PAIR(3));
			} else {
				attron(COLOR_PAIR(2));
			}
			room_render(r);
			attroff(A_COLOR);
			if (r->connected && shift) {
				refresh();
				SMALL_SLEEP();
			}
		} while (++shift < 49 && !r->connected);
	}

	refresh();

	return lvl;
}
