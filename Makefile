CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

all: test_stream test_lz10 test_rle

test_stream: tests/stream.c src/stream.c
	$(CC) $(CFLAGS) -o test_stream tests/stream.c src/stream.c

test_lz10: tests/lz10.c src/lz10.c src/stream.c
	$(CC) $(CFLAGS) -o test_lz10 tests/lz10.c src/lz10.c src/stream.c

test_rle: tests/rle.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_rle tests/rle.c src/rle.c src/stream.c

clean:
	rm -f test_stream test_lz10 test_rle
