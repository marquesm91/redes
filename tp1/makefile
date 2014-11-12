CC=g++
CFLAGS=-W -Wall -ansi -pedantic -I.

all: encode decode

encode: encode.c
	$(CC) -o encode encode.c $(CFLAGS)
decode: decode.c
	$(CC) -o decode decode.c $(CFLAGS)
clean:
	rm -rf *.o