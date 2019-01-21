LIBS = -lncurses

CC = gcc
CFLAGS = -g -O2

LINK = $(CC)
LDFLAGS = $(CFLAGS) $(LIBS)

sys-hack: build/main.o
	$(LINK) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) -o $@ -c $(CFLAGS) $^
