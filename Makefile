CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

all: test_stream test_lz10 test_rle test_huffman test_file test_asset test_archive test_layton_pack test_layton_pack2 test_dlz test_nazo test_nazo_list test_goal_info test_herbtea_event test_event_descriptor_bank

test_stream: tests/stream.c src/stream.c
	$(CC) $(CFLAGS) -o test_stream tests/stream.c src/stream.c

test_lz10: tests/lz10.c src/lz10.c src/stream.c
	$(CC) $(CFLAGS) -o test_lz10 tests/lz10.c src/lz10.c src/stream.c

test_rle: tests/rle.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_rle tests/rle.c src/rle.c src/stream.c

test_huffman: tests/huffman.c src/huffman.c src/stream.c
	$(CC) $(CFLAGS) -o test_huffman tests/huffman.c src/huffman.c src/stream.c

test_file: tests/file.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_file tests/file.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c

test_asset: tests/asset.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_asset tests/asset.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c

test_archive: tests/archive.c src/archive.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_archive tests/archive.c src/archive.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c

test_layton_pack: tests/layton_pack.c src/layton_pack.c src/archive.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_layton_pack tests/layton_pack.c src/layton_pack.c src/archive.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c

test_layton_pack2: tests/layton_pack2.c src/layton_pack2.c src/archive.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c
	$(CC) $(CFLAGS) -o test_layton_pack2 tests/layton_pack2.c src/layton_pack2.c src/archive.c src/asset.c src/file.c src/huffman.c src/lz10.c src/rle.c src/stream.c

test_dlz: tests/dlz.c src/dlz.c src/stream.c
	$(CC) $(CFLAGS) -o test_dlz tests/dlz.c src/dlz.c src/stream.c

test_nazo: tests/nazo.c src/nazo.c src/stream.c
	$(CC) $(CFLAGS) -o test_nazo tests/nazo.c src/nazo.c src/stream.c

test_nazo_list: tests/nazo_list.c src/nazo_list.c src/stream.c
	$(CC) $(CFLAGS) -o test_nazo_list tests/nazo_list.c src/nazo_list.c src/stream.c

test_goal_info: tests/goal_info.c src/goal_info.c src/stream.c
	$(CC) $(CFLAGS) -o test_goal_info tests/goal_info.c src/goal_info.c src/stream.c

test_herbtea_event: tests/herbtea_event.c src/herbtea_event.c src/stream.c
	$(CC) $(CFLAGS) -o test_herbtea_event tests/herbtea_event.c src/herbtea_event.c src/stream.c

test_event_descriptor_bank: tests/event_descriptor_bank.c src/event_descriptor_bank.c src/stream.c
	$(CC) $(CFLAGS) -o test_event_descriptor_bank tests/event_descriptor_bank.c src/event_descriptor_bank.c src/stream.c

clean:
	rm -f test_stream test_lz10 test_rle test_huffman test_file test_asset test_archive test_layton_pack test_layton_pack2 test_dlz test_nazo test_nazo_list test_goal_info test_herbtea_event test_event_descriptor_bank
