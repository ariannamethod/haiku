# haiku — a small alien in one C file. libc + -lm, nothing else.
CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
LDLIBS  ?= -lm

haiku: haiku.c
	$(CC) $(CFLAGS) haiku.c $(LDLIBS) -o haiku

clean:
	rm -f haiku haiku.state haiku.state.tmp

.PHONY: clean
