#include "csapp.h"
#include <stdint.h>

#define BOARD_SIZE 20

void display_board(char board[][BOARD_SIZE]) {
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++)
            printf("%c ", board[i][j]);
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    int connfd = Open_clientfd(argv[1], argv[2]);
    char symbol;
    Rio_readn(connfd, &symbol, 1);
    printf("You are %c\n", symbol);

    while (1) {
        char msg_type;
        Rio_readn(connfd, &msg_type, 1);

        if (msg_type == 'B') {
            char board[BOARD_SIZE][BOARD_SIZE];
            Rio_readn(connfd, board, BOARD_SIZE * BOARD_SIZE);
            display_board(board);
        } else if (msg_type == 'T') {
            printf("Your move (row col): ");
            int row, col;
            scanf("%d %d", &row, &col);
            int row_net = htonl(row);
            int col_net = htonl(col);
            Rio_writen(connfd, &row_net, sizeof(row_net));
            Rio_writen(connfd, &col_net, sizeof(col_net));
        } else if (msg_type == 'G') {
            int winner_net;
            Rio_readn(connfd, &winner_net, sizeof(winner_net));
            int winner = ntohl(winner_net);
            if (winner == -1)
                printf("Draw!\n");
            else if ((winner == 0 && symbol == 'X') || (winner == 1 && symbol == 'O'))
                printf("You win!\n");
            else
                printf("You lose!\n");
            break;
        }
    }

    Close(connfd);
    return 0;
}