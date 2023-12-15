CC = gcc
CFLAGS = -lraylib -lm -lpthread -lopengl32 -lgdi32 -lwinmm -Wall -Wextra
SOURCES = ./src/*.c

all: game run clean

game:
	$(CC) $(SOURCES) $(CFLAGS) -o game

run:
	./game

clean:
	rm game
