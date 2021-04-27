# set the compiler
CC := gcc

# general compiler flags
CFLAGS := --std=c99 -Wall -Wextra -pedantic

ifeq ($(OS),Windows_NT)
	CFLAGS += -IC:\libsdl\include -LC:\libsdl\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_net
	EXEC := server.exe
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CFLAGS += `sdl2-config --libs --cflags` -lSDL2_net
		EXEC := server.out
	endif
	ifeq ($(UNAME_S),Darwin)
		CFLAGS += `sdl2-config --libs --cflags` -lSDL2_net
		EXEC := server.out
	endif
endif

$(EXEC): main.o server.o
	$(CC) *.o $(CFLAGS) -o $(EXEC)

main.o: main.c
	$(CC) main.c -c $(CFLAGS)

server.o: server.c
	$(CC) server.c -c $(CFLAGS)

clean:
	rm -f $(EXEC) *.o

.PHONY: clean
