CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

all: test_stream test_lz10 test_rle test_huffman test_file test_asset test_archive test_layton_pack test_layton_pack2 test_dlz test_nazo test_nazo_list test_goal_info test_herbtea_event test_event_descriptor_bank test_event_base_list test_event_info_list test_story_select_list test_submap_info

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

test_event_base_list: tests/event_base_list.c src/event_base_list.c src/stream.c
	$(CC) $(CFLAGS) -o test_event_base_list tests/event_base_list.c src/event_base_list.c src/stream.c

test_event_info_list: tests/event_info_list.c src/event_info_list.c src/stream.c
	$(CC) $(CFLAGS) -o test_event_info_list tests/event_info_list.c src/event_info_list.c src/stream.c

test_story_select_list: tests/story_select_list.c src/story_select_list.c src/stream.c
	$(CC) $(CFLAGS) -o test_story_select_list tests/story_select_list.c src/story_select_list.c src/stream.c

test_submap_info: tests/submap_info.c src/submap_info.c src/stream.c
	$(CC) $(CFLAGS) -o test_submap_info tests/submap_info.c src/submap_info.c src/stream.c

clean:
	rm -f test_stream test_lz10 test_rle test_huffman test_file test_asset test_archive test_layton_pack test_layton_pack2 test_dlz test_nazo test_nazo_list test_goal_info test_herbtea_event test_event_descriptor_bank test_event_base_list test_event_info_list test_story_select_list test_submap_info
