CC=gcc
CFLAGS= -g -Wall
EXE=so-cpp

all: build

build: so-cpp.c
	$(CC) $^ $(CFLAGS) -o $(EXE)

clean:
	rm -f $(EXE)

.PHONY: clean run
