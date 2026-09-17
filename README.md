# Madhatter

Madhatter is a C11 library for reading, writing, and decompressing Nintendo-style binary asset formats for earlier Professor Layton games for the Nintendo DS, ported from the original [Python-based project](https://github.com/bullbin/madhatter).

It includes support for:

- low-level buffer/stream helpers
- LZ10 decompression/compression
- RLE decompression
- Huffman decoding/encoding helpers
- automatic file compression detection
- generic asset and archive handling
- LaytonPack and LaytonPack2 archive formats
- DLZ containers and several concrete record types
- chapter/event/story metadata structures

The project is designed to be built as a static library and consumed from C code through a single public header.

---

## Requirements

- C compiler compatible with C11
- GNU make
- POSIX shell / macOS/Linux environment

Tested in this project with:

```bash
cc -std=c11 -Wall -Wextra -Iinclude
```

---

## Installation

Clone the repository:

```bash
git clone https://github.com/emiyl/madhatter
cd madhatter
```

Build the static library:

```bash
make
```

This produces:

```bash
libmadhatter.a
```

Install to a system prefix:

```bash
sudo make install
```

By default this installs to:

- `/usr/local/include/madhatter/*.h`
- `/usr/local/lib/libmadhatter.a`

This includes the umbrella header and all public component headers used by it under a dedicated `madhatter` include directory.

You can override the installation prefix:

```bash
make install PREFIX=/opt/madhatter
```

This installs to:

```bash
/opt/madhatter/include/madhatter/*.h
/opt/madhatter/lib/libmadhatter.a
```

To remove generated build artifacts:

```bash
make clean
```

---

## Using the library

Include the umbrella header from the installed include directory:

```c
#include "madhatter/madhatter.h"
```

Then compile your program against the library:

```bash
cc your_program.c -I/usr/local/include -L/usr/local/lib -lmadhatter -o your_program
```

If using a custom prefix:

```bash
cc your_program.c -I/opt/madhatter/include -L/opt/madhatter/lib -lmadhatter -o your_program
```

---

## Example: LZ10 decompression

```c
#include <stdio.h>
#include <stdlib.h>
#include "madhatter.h"

int main(void) {
    const uint8_t src[] = { /* compressed bytes */ };
    mh_buffer out = {0};

    if (mh_lz10_decompress(src, sizeof(src), &out) != 0) {
        fprintf(stderr, "LZ10 decompression failed\n");
        return 1;
    }

    printf("decompressed %zu bytes\n", out.len);
    mh_buffer_free(&out);
    return 0;
}
```

---

## Example: loading an asset from bytes

```c
#include <stdio.h>
#include <stdlib.h>
#include "madhatter.h"

int main(void) {
    const uint8_t data[] = { /* raw file bytes */ };
    mh_asset asset = {0};
    mh_buffer out = {0};

    if (mh_asset_init_from_bytes(&asset, data, sizeof(data)) != 0) {
        fprintf(stderr, "failed to initialize asset\n");
        return 1;
    }

    if (mh_asset_decompress(&asset, &out) != 0) {
        fprintf(stderr, "decompression failed\n");
        mh_asset_free(&asset);
        return 1;
    }

    printf("decompressed asset length: %zu\n", out.len);

    mh_buffer_free(&out);
    mh_asset_free(&asset);
    return 0;
}
```

---

## Example: archive usage

```c
#include <stdio.h>
#include <stdlib.h>
#include "madhatter.h"

int main(void) {
    mh_archive archive = {0};
    const uint8_t payload[] = { 0x01, 0x02, 0x03, 0x04 };

    if (mh_archive_init(&archive) != 0) {
        fprintf(stderr, "archive init failed\n");
        return 1;
    }

    if (mh_archive_add_file(&archive, "test.bin", payload, sizeof(payload)) != 0) {
        fprintf(stderr, "failed to add archive entry\n");
        mh_archive_free(&archive);
        return 1;
    }

    mh_archive_entry *entry = mh_archive_get(&archive, "test.bin");
    if (entry != NULL) {
        printf("entry found: %s (%zu bytes)\n", entry->name, entry->asset.len);
    }

    mh_archive_free(&archive);
    return 0;
}
```

---

## Library layout

The public API is grouped under the common include file, and the implementation is split into these areas:

- core streams: `stream.h`, `stream.c`
- compression: `lz10.h`, `rle.h`, `huffman.h`
- file detection: `file.h`, `file.c`
- asset/archive wrappers: `asset.h`, `archive.h`
- archive formats: `layton_pack.h`, `layton_pack2.h`
- DLZ and data records: `dlz.h`, `nazo.h`, `goal_info.h`, `event_*`, `story_*`, `submap_info.h`, `time_definition.h`, `chapter_info.h`

The umbrella header exposes the library in a single include point:

```c
#include "madhatter.h"
```

---

## Build targets

The Makefile supports:

```bash
make        # build libmadhatter.a
make clean  # remove object files and library
make install # install header + library
```

The default installation prefix can be overridden with `PREFIX`.