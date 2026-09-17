CC = cc
AR = ar
ARFLAGS = rcs
CFLAGS = -std=c11 -Wall -Wextra -Iinclude
PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include/madhatter
LIBDIR ?= $(PREFIX)/lib

SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)
TARGET = libmadhatter.a

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

src/%.o: src/%.c include/*.h
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -d $(INCLUDEDIR)
	install -d $(LIBDIR)
	install -m 0644 include/*.h $(INCLUDEDIR)/
	install -m 0644 $(TARGET) $(LIBDIR)/$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
