# TCP Omok Term Project

This project implements a two-player Omok game using TCP sockets in C on Linux.
The server accepts two clients, manages the 15x15 board, validates moves, and
announces win/draw/disconnect results.

## Build

```sh
make
```

## Run

Start the server:

```sh
./omok_server 9090
```

Open two other terminals and connect two clients:

```sh
./omok_client 127.0.0.1 9090
```

Players enter row and column numbers from 1 to 15:

```text
8 8
8 9
quit
```

## Demo Checklist

- Show the server waiting for two TCP clients.
- Show BLACK and WHITE assignment.
- Demonstrate normal moves, invalid move handling, and board updates.
- Complete a five-in-a-row win or show the disconnect handling.
- Explain that the server owns all game state and clients only send move requests.
