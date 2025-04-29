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

void display_board(char board[][BOARD_SIZE]) {
    printf("\nCurrent Board State:\n");
    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf(" %2d", i);
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

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    int connfd = Open_clientfd(argv[1], argv[2]);
    char symbol;
    Rio_readn(connfd, &symbol, 1);
    printf("You are %c\n", symbol);
    printf("You have %d seconds to make each move\n", MOVE_TIMEOUT);

    while (1) {
        char msg_type;
        Rio_readn(connfd, &msg_type, 1);

        if (msg_type == 'B') {
            char board[BOARD_SIZE][BOARD_SIZE];
            Rio_readn(connfd, board, BOARD_SIZE * BOARD_SIZE);
            display_board(board);
        } else if (msg_type == 'T') {
            while (1) {  // Loop until a valid move is made
                printf(ANSI_COLOR_YELLOW "Your move (row col): " ANSI_COLOR_RESET);
                int row, col;
                scanf("%d %d", &row, &col);
                
                // Basic input validation
                if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
                    printf(ANSI_COLOR_RED "Invalid position! Please enter numbers between 0 and %d\n" ANSI_COLOR_RESET, 
                           BOARD_SIZE - 1);
                    continue;
                }
                
                int row_net = htonl(row);
                int col_net = htonl(col);
                Rio_writen(connfd, &row_net, sizeof(row_net));
                Rio_writen(connfd, &col_net, sizeof(col_net));
                break;  // Exit the loop after sending the move
            }
        } else if (msg_type == 'G') {
            int winner_net;
            Rio_readn(connfd, &winner_net, sizeof(winner_net));
            int winner = ntohl(winner_net);
            if (winner == -1)
                printf(ANSI_COLOR_YELLOW "Game ended in a draw!\n" ANSI_COLOR_RESET);
            else if ((winner == 0 && symbol == 'X') || (winner == 1 && symbol == 'O'))
                printf(ANSI_COLOR_GREEN "Congratulations! You win!\n" ANSI_COLOR_RESET);
            else if (winner == -3)
                printf(ANSI_COLOR_RED "You lost due to timeout!\n" ANSI_COLOR_RESET);
            else
                printf(ANSI_COLOR_RED "You lose!\n" ANSI_COLOR_RESET);
            break;
        }
    }

    Close(connfd);
    return 0;
}