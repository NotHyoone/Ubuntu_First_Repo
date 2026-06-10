CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -O2

.PHONY: all clean

all: omok_server omok_client

omok_server: omok_server.c
	$(CC) $(CFLAGS) omok_server.c -o omok_server

omok_client: omok_client.c
	$(CC) $(CFLAGS) omok_client.c -o omok_client

clean:
	rm -f omok_server omok_client
