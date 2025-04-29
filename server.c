#include "csapp.h"
#include <stdint.h>

#define BOARD_SIZE 20

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

void send_board(int connfd, char board[][BOARD_SIZE]) {
    char msg_type = 'B';
    Rio_writen(connfd, &msg_type, 1);
    Rio_writen(connfd, board, BOARD_SIZE * BOARD_SIZE);
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
    memset(board, ' ', sizeof(board));
    int current_player = 0;
    int game_over = 0;
    int winner = -1;

    while (!game_over) {
        send_board(connfd1, board);
        send_board(connfd2, board);

        int connfd_current = current_player ? connfd2 : connfd1;
        char turn_msg = 'T';
        Rio_writen(connfd_current, &turn_msg, 1);

        int row_net, col_net;
        Rio_readn(connfd_current, &row_net, sizeof(row_net));
        Rio_readn(connfd_current, &col_net, sizeof(col_net));
        int row = ntohl(row_net);
        int col = ntohl(col_net);

        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE || board[row][col] != ' ') {
            fprintf(stderr, "Invalid move. Closing.\n");
            break;
        }

        board[row][col] = current_player ? 'O' : 'X';

        if (check_win(board, row, col, board[row][col])) {
            winner = current_player;
            game_over = 1;
        } else {
            int is_draw = 1;
            for (int i = 0; i < BOARD_SIZE; i++)
                for (int j = 0; j < BOARD_SIZE; j++)
                    if (board[i][j] == ' ') is_draw = 0;
            if (is_draw) game_over = 1;
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

    Close(connfd1);
    Close(connfd2);
    Close(listenfd);
    return 0;
}