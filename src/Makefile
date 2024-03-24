CFLAGS = -g
CC = gcc

all:
	flex lex.l
	$(CC) $(CFLAGS) -o alpha al.c

clean:
	rm -f alpha lex.yy.c