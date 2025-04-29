#include "csapp.h"
#include <stdint.h>
#include <time.h>

#define BOARD_SIZE 20
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define MOVE_TIMEOUT 30  // seconds per move

typedef struct {
    int score;
    int moves_made;
    time_t last_move_time;
} PlayerStats;

int check_win(char board[][BOARD_SIZE], int row, int col, char player) {
    int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {-1, 1}};
    for (int d = 0; d < 4; d++) {
        int dx = directions[d][0];
        int dy = directions[d][1];
        int count = 1;
        int x = row + dx, y = col + dy;
        while (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == player) {
            count++;
            x += dx;
            y += dy;
        }
        x = row - dx;
        y = col - dy;
        while (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == player) {
            count++;
            x -= dx;
            y -= dy;
        }
        if (count >= 5) return 1;
    }
    return 0;
}

void send_board(int connfd, char board[][BOARD_SIZE], PlayerStats *stats) {
    char msg_type = 'B';
    Rio_writen(connfd, &msg_type, 1);
    Rio_writen(connfd, board, BOARD_SIZE * BOARD_SIZE);
    
    // Print the board on server side with colors and stats
    printf("\nCurrent Board State (Move #%d):\n", stats[0].moves_made + stats[1].moves_made);
    printf("Scores - X: %d, O: %d\n", stats[0].score, stats[1].score);
    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
    }
    printf("\n");
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 'X') {
                printf(ANSI_COLOR_RED " X " ANSI_COLOR_RESET);
            } else if (board[i][j] == 'O') {
                printf(ANSI_COLOR_BLUE " O " ANSI_COLOR_RESET);
            } else {
                printf(" . ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

int validate_move(char board[][BOARD_SIZE], int row, int col, char *error_msg) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        sprintf(error_msg, "Position (%d,%d) is out of bounds!", row, col);
        return 0;
    }
    if (board[row][col] != ' ') {
        sprintf(error_msg, "Position (%d,%d) is already occupied!", row, col);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    int listenfd = Open_listenfd(argv[1]);
    printf("Server listening on port %s\n", argv[1]);

    int connfd1 = Accept(listenfd, NULL, NULL);
    printf("Client 1 connected. Assigned X.\n");
    Rio_writen(connfd1, "X", 1);

    int connfd2 = Accept(listenfd, NULL, NULL);
    printf("Client 2 connected. Assigned O.\n");
    Rio_writen(connfd2, "O", 1);

    char board[BOARD_SIZE][BOARD_SIZE];
    memset(board, ' ', BOARD_SIZE * BOARD_SIZE);
 
    PlayerStats stats[2] = {{0, 0, 0}, {0, 0, 0}};
    int current_player = 0;
    int game_over = 0;
    int winner = -1;
    char error_msg[100];

    while (!game_over) {
        send_board(connfd1, board, stats);
        send_board(connfd2, board, stats);

        int connfd_current = current_player ? connfd2 : connfd1;
        char turn_msg = 'T';
        Rio_writen(connfd_current, &turn_msg, 1);

        // Start move timer
        stats[current_player].last_move_time = time(NULL);

        int row_net, col_net;
        Rio_readn(connfd_current, &row_net, sizeof(row_net));
        Rio_readn(connfd_current, &col_net, sizeof(col_net));
        int row = ntohl(row_net);
        int col = ntohl(col_net);

        // Check for timeout
        time_t current_time = time(NULL);
        if (current_time - stats[current_player].last_move_time > MOVE_TIMEOUT) {
            printf("Player %c timed out!\n", current_player ? 'O' : 'X');
            winner = !current_player;  // Other player wins by timeout
            game_over = 1;
            stats[!current_player].score += 1;
            break;
        }

        if (!validate_move(board, row, col, error_msg)) {
            fprintf(stderr, "Invalid move: %s\n", error_msg);
            printf("Player %c made an invalid move at (%d,%d), please try again\n", 
                   current_player ? 'O' : 'X', row, col);
            continue;  // Skip the rest of the loop and ask for a new move
        }

        board[row][col] = current_player ? 'O' : 'X';
        stats[current_player].moves_made++;

        printf("Player %c made a move at position (%d, %d)\n", 
               current_player ? 'O' : 'X', row, col);

        if (check_win(board, row, col, board[row][col])) {
            winner = current_player;
            game_over = 1;
            stats[current_player].score += 1;
            printf("Player %c wins!\n", current_player ? 'O' : 'X');
        } else {
            int is_draw = 1;
            for (int i = 0; i < BOARD_SIZE; i++)
                for (int j = 0; j < BOARD_SIZE; j++)
                    if (board[i][j] == ' ') is_draw = 0;
            if (is_draw) {
                game_over = 1;
                printf("Game ended in a draw!\n");
            }
        }

        if (game_over) {
            char game_over_msg = 'G';
            int winner_net = htonl(winner);
            Rio_writen(connfd1, &game_over_msg, 1);
            Rio_writen(connfd1, &winner_net, sizeof(winner_net));
            Rio_writen(connfd2, &game_over_msg, 1);
            Rio_writen(connfd2, &winner_net, sizeof(winner_net));
        }

        current_player = !current_player;
    }

    // Print final game statistics
    printf("\nGame Statistics:\n");
    printf("Player X: %d moves, Score: %d\n", stats[0].moves_made, stats[0].score);
    printf("Player O: %d moves, Score: %d\n", stats[1].moves_made, stats[1].score);

    Close(connfd1);
    Close(connfd2);
    Close(listenfd);
    return 0;
}