CC = gcc

CFLAGS = -Wall -g

all: run

run:
	$(CC) -o main scanner.c charcode.c error.c reader.c token.c 
	./main
clean:
	rm -f main