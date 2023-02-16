CC = gcc

INCLUDE_DIR = include
SRC_DIR     = src

OBJS = $(SRC_DIR)/input.o $(SRC_DIR)/bb.o
MAIN = envio

CFLAGS = -Wall -Wextra -Wpedantic -g -I$(INCLUDE_DIR)
LDLIBS = -lm

all: $(MAIN)

$(MAIN): $(OBJS)

clean:
	@ rm -f $(MAIN) $(OBJS)

.PHONY: all clean
