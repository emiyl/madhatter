CC = cc
AR = ar
ARFLAGS = rcs
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)
TARGET = libmadhatter.a

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

src/%.o: src/%.c include/*.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
