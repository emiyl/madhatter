CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

all: test

test: tests/test_lz10.c src/madhatter.c
	$(CC) $(CFLAGS) -o test_lz10 tests/test_lz10.c src/madhatter.c

clean:
	rm -f test_lz10
