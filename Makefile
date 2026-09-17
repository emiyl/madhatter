CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

all: test_stream test_lz10

test_stream: tests/test_stream.c src/mh_stream.c
	$(CC) $(CFLAGS) -o test_stream tests/test_stream.c src/mh_stream.c

test_lz10: tests/test_lz10.c src/mh_lz10.c src/mh_stream.c
	$(CC) $(CFLAGS) -o test_lz10 tests/test_lz10.c src/mh_lz10.c src/mh_stream.c

clean:
	rm -f test_stream test_lz10
