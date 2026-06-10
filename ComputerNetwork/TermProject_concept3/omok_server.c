/*
 * TCP Omok server for the Computer Networks term project.
 * AI assistance notice: the initial structure and helper functions were drafted
 * with OpenAI Codex and should be disclosed in the submitted report/code.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BOARD_SIZE 15   // 오목판 가로 == 15, 세로 == 15
#define LINE_SIZE 256   // 클라이언트와 주고받는 메시지의 최대 길이 (개행 포함)

// 돌의 상태를 나타내는 열거형
typedef enum {
    EMPTY = 0,  // 빈 칸
    BLACK = 1,  // BLACK 돌
    WHITE = 2   // WHITE 돌
} Stone;

static int board[BOARD_SIZE][BOARD_SIZE];   // 15x15 오목판 상태 저장 2차원 배열

// 돌의 이름을 반환하는 함수
static const char *stone_name(int stone)
{
    return stone == BLACK ? "BLACK" : "WHITE";  // 돌이 BLACK이면 "BLACK", 그렇지 않으면 "WHITE" 반환
}

// 돌의 심볼을 반환하는 함수
static char stone_symbol(int stone)
{
    if (stone == BLACK) {   // BLACK 돌이면 'X' 반환
        return 'X';
    }
    if (stone == WHITE) {   // WHITE 돌이면 'O' 반환
        return 'O';
    }
    return '.';
}

// 모든 데이터를 전송할 때까지 반복
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

static int send_line(int fd, const char *fmt, ...)
{
    char line[LINE_SIZE];
    va_list ap;

    va_start(ap, fmt);
    int written = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    if (written < 0 || written >= (int)sizeof(line)) {
        return -1;
    }

    size_t len = strlen(line);
    if (len + 1 >= sizeof(line)) {
        return -1;
    }
    line[len] = '\n';
    line[len + 1] = '\0';
    return send_all(fd, line, len + 1);
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

// 모든 클라이언트에게 같은 메시지를 보냄
static void broadcast_line(int black_fd, int white_fd, const char *fmt, ...)
{
    char line[LINE_SIZE];
    va_list ap;

    va_start(ap, fmt);  // 가변 인자 처리
    vsnprintf(line, sizeof(line), fmt, ap); // 메시지 포맷팅
    va_end(ap); // 가변 인자 처리 종료

    send_line(black_fd, "%s", line);    // BLACK 플레이어에게 메시지 전송
    send_line(white_fd, "%s", line);    // WHITE 플레이어에게 메시지 전송
}

// 현재 보드 상태를 특정 클라이언트에게 전송
static void send_board_to(int fd)
{
    send_line(fd, "BOARD_BEGIN");
    send_line(fd, "    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15");
    for (int row = 0; row < BOARD_SIZE; row++) {
        char line[LINE_SIZE];
        int offset = snprintf(line, sizeof(line), "%2d ", row + 1);
        for (int col = 0; col < BOARD_SIZE; col++) {
            offset += snprintf(line + offset, sizeof(line) - (size_t)offset,
                               " %c ", stone_symbol(board[row][col]));
        }
        send_line(fd, "%s", line);
    }
    send_line(fd, "BOARD_END");
}

// 모든 클라이언트에게 현재 보드 상태를 전송
static void broadcast_board(int black_fd, int white_fd)
{
    send_board_to(black_fd);
    send_board_to(white_fd);
}

// 보드 범위 내에 있는지 확인
static int in_range(int row, int col)
{
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

// 특정 방향으로 연속된 돌의 개수를 세는 함수
static int count_direction(int row, int col, int dr, int dc, int stone)
{
    int count = 0;
    row += dr;
    col += dc;
    while (in_range(row, col) && board[row][col] == stone) {
        count++;
        row += dr;
        col += dc;
    }
    return count;
}

// 현재 위치에서 돌이 5개 이상 연속되어 있는지 확인
static int is_win(int row, int col, int stone)
{
    const int dirs[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1}
    };

    for (int i = 0; i < 4; i++) {
        int total = 1;
        total += count_direction(row, col, dirs[i][0], dirs[i][1], stone);
        total += count_direction(row, col, -dirs[i][0], -dirs[i][1], stone);
        if (total >= 5) {
            return 1;
        }
    }
    return 0;
}

// 보드가 가득 찼는지 확인
static int is_board_full(void)
{
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col] == EMPTY) {
                return 0;
            }
        }
    }
    return 1;
}

// 서버 소켓 생성 및 설정
static int create_server_socket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);    // TCP 소켓 생성
    
    // 소켓 생성 실패 시 에러 처리
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    // SO_REUSEADDR 옵션 설정: 서버가 종료된 후에도 소켓을 재사용할 수 있도록 함
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;  // IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // 모든 인터페이스에서 연결 허용
    addr.sin_port = htons((uint16_t)port);  // 포트 번호 설정

    // 소켓에 주소 바인딩
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); // 바인딩 실패 시 에러 처리
        close(server_fd);   // 소켓 닫기
        return -1;
    }
    // 클라이언트 연결 대기
    if (listen(server_fd, 2) < 0) {
        perror("listen");   // 연결 실패 시 에러 처리
        close(server_fd);   // 소켓 닫기
        return -1;
    }
    return server_fd;   // 성공적으로 서버 소켓 생성 및 설정 완료
}

// 클라이언트 연결 수락 및 플레이어 색상 할당
static int accept_player(int server_fd, const char *color)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);  // 클라이언트 연결 수락
    // 연결 수락 실패 시 에러 처리
    if (client_fd < 0) {
        perror("accept"); // 클라이언트 연결 실패 시 에러 처리
        return -1;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));  // 클라이언트 IP 주소 문자열로 변환
    printf("%s player connected from %s:%d\n", color, ip, ntohs(client_addr.sin_port)); // 연결된 클라이언트 정보 출력
    return client_fd;
}

// 클라이언트로부터 MOVE 명령을 받아 처리
static int parse_move(const char *line, int *row, int *col)
{
    int r;
    int c;

    if (sscanf(line, "MOVE %d %d", &r, &c) == 2) {
        *row = r - 1;
        *col = c - 1;
        return 1;
    }
    return 0;
}

// 게임 진행 루프
static void run_game(int black_fd, int white_fd)
{
    int turn = BLACK;
    char line[LINE_SIZE];

    memset(board, 0, sizeof(board));
    send_line(black_fd, "WELCOME BLACK");
    send_line(white_fd, "WELCOME WHITE");
    broadcast_line(black_fd, white_fd, "MESSAGE Game start. X=BLACK, O=WHITE.");
    broadcast_board(black_fd, white_fd);

    while (1) {
        int current_fd = turn == BLACK ? black_fd : white_fd;
        int other_fd = turn == BLACK ? white_fd : black_fd;
        int row;
        int col;

        broadcast_line(black_fd, white_fd, "TURN %s", stone_name(turn));

        int status = recv_line(current_fd, line, sizeof(line));
        if (status <= 0 || strcmp(line, "QUIT") == 0) {
            send_line(other_fd, "RESULT %s_WIN opponent_disconnected", stone_name(turn == BLACK ? WHITE : BLACK));
            printf("%s disconnected. Game ended.\n", stone_name(turn));
            break;
        }

        if (!parse_move(line, &row, &col)) {
            send_line(current_fd, "ERROR Use: row col  (example: 8 8)");
            continue;
        }
        if (!in_range(row, col)) {
            send_line(current_fd, "ERROR Move must be inside 1..15.");
            continue;
        }
        if (board[row][col] != EMPTY) {
            send_line(current_fd, "ERROR That position is already occupied.");
            continue;
        }

        board[row][col] = turn;
        broadcast_line(black_fd, white_fd, "MESSAGE %s placed at row %d col %d.",
                       stone_name(turn), row + 1, col + 1);
        broadcast_board(black_fd, white_fd);

        if (is_win(row, col, turn)) {
            broadcast_line(black_fd, white_fd, "RESULT %s_WIN", stone_name(turn));
            printf("%s wins.\n", stone_name(turn));
            break;
        }
        if (is_board_full()) {
            broadcast_line(black_fd, white_fd, "RESULT DRAW");
            printf("Draw.\n");
            break;
        }

        turn = turn == BLACK ? WHITE : BLACK;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Port must be between 1 and 65535.\n");
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    int server_fd = create_server_socket(port);
    if (server_fd < 0) {
        return 1;
    }

    printf("TCP Omok server listening on port %d\n", port);
    printf("Waiting for BLACK player...\n");
    int black_fd = accept_player(server_fd, "BLACK");
    if (black_fd < 0) {
        close(server_fd);
        return 1;
    }

    send_line(black_fd, "MESSAGE Connected as BLACK. Waiting for WHITE player...");
    printf("Waiting for WHITE player...\n");
    int white_fd = accept_player(server_fd, "WHITE");
    if (white_fd < 0) {
        close(black_fd);
        close(server_fd);
        return 1;
    }

    run_game(black_fd, white_fd);

    close(black_fd);
    close(white_fd);
    close(server_fd);
    return 0;
}
