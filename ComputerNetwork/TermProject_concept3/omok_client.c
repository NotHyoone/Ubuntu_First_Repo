/*
 * TCP Omok client for the Computer Networks term project.
 * AI assistance notice: the initial structure and helper functions were drafted
 * with OpenAI Codex and should be disclosed in the submitted report/code.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_SIZE 256

static int send_all(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t sent = send(fd, buf, len, 0);
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (sent == 0) {
            return -1;
        }
        buf += sent;
        len -= (size_t)sent;
    }
    return 0;
}

static int send_line(int fd, const char *line)
{
    char buf[LINE_SIZE];
    int written = snprintf(buf, sizeof(buf), "%s\n", line);
    if (written < 0 || written >= (int)sizeof(buf)) {
        return -1;
    }
    return send_all(fd, buf, (size_t)written);
}

static int recv_line(int fd, char *buf, size_t size)
{
    size_t used = 0;

    while (used + 1 < size) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return 0;
        }
        if (ch == '\n') {
            break;
        }
        if (ch != '\r') {
            buf[used++] = ch;
        }
    }
    buf[used] = '\0';
    return 1;
}

static int connect_to_server(const char *ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", ip);
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }
    return fd;
}

static int prompt_and_send_move(int fd)
{
    char input[LINE_SIZE];
    char msg[LINE_SIZE];
    int row;
    int col;

    while (1) {
        printf("Your move (row col, 1-15) or quit: ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            send_line(fd, "QUIT");
            return -1;
        }
        input[strcspn(input, "\r\n")] = '\0';

        if (strcmp(input, "quit") == 0 || strcmp(input, "QUIT") == 0) {
            send_line(fd, "QUIT");
            return -1;
        }
        if (sscanf(input, "%d %d", &row, &col) == 2) {
            snprintf(msg, sizeof(msg), "MOVE %d %d", row, col);
            if (send_line(fd, msg) < 0) {
                perror("send");
                return -1;
            }
            return 0;
        }

        printf("Invalid input. Example: 8 8\n");
    }
}

static void print_result(const char *line, const char *my_color)
{
    if (strstr(line, "DRAW") != NULL) {
        printf("Game result: draw.\n");
        return;
    }
    if (strstr(line, my_color) != NULL) {
        printf("Game result: you win. (%s)\n", line + 7);
    } else {
        printf("Game result: you lose. (%s)\n", line + 7);
    }
}

static void run_client(int fd)
{
    char line[LINE_SIZE];
    char my_color[16] = "";

    while (1) {
        int status = recv_line(fd, line, sizeof(line));
        if (status <= 0) {
            printf("Disconnected from server.\n");
            break;
        }

        if (strncmp(line, "WELCOME ", 8) == 0) {
            if (strcmp(line + 8, "BLACK") == 0) {
                strcpy(my_color, "BLACK");
            } else if (strcmp(line + 8, "WHITE") == 0) {
                strcpy(my_color, "WHITE");
            } else {
                strcpy(my_color, "UNKNOWN");
            }
            printf("You are %s.\n", my_color);
        } else if (strncmp(line, "MESSAGE ", 8) == 0) {
            printf("%s\n", line + 8);
        } else if (strncmp(line, "ERROR ", 6) == 0) {
            printf("Error: %s\n", line + 6);
        } else if (strcmp(line, "BOARD_BEGIN") == 0) {
            while (recv_line(fd, line, sizeof(line)) > 0) {
                if (strcmp(line, "BOARD_END") == 0) {
                    break;
                }
                printf("%s\n", line);
            }
        } else if (strncmp(line, "TURN ", 5) == 0) {
            const char *turn = line + 5;
            if (strcmp(turn, my_color) == 0) {
                if (prompt_and_send_move(fd) < 0) {
                    break;
                }
            } else {
                printf("Waiting for %s...\n", turn);
            }
        } else if (strncmp(line, "RESULT ", 7) == 0) {
            print_result(line, my_color);
            break;
        } else {
            printf("Server: %s\n", line);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server-ip> <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Port must be between 1 and 65535.\n");
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    int fd = connect_to_server(argv[1], port);
    if (fd < 0) {
        return 1;
    }

    run_client(fd);
    close(fd);
    return 0;
}
