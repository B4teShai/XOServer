#include "xo.h"

void print_board(game_state_t *game) {
    printf("\n  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
    }
    printf("\n");
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf(" %c ", game->board[i][j]);
            if (j < BOARD_SIZE - 1) printf("|");
        }
        printf("\n");
        if (i < BOARD_SIZE - 1) {
            printf("   ");
            for (int j = 0; j < BOARD_SIZE; j++) {
                printf("---");
                if (j < BOARD_SIZE - 1) printf("+");
            }
            printf("\n");
        }
    }
}

int main(int argc, char **argv) {
    int clientfd;
    rio_t rio;
    game_state_t game;
    message_t msg;
    char *host, *port;
    
    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    
    host = argv[1];
    port = argv[2];
    
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);
    
    printf("Connected to server\n");
    printf("Waiting for game to start...\n");
    
    while (1) {
        // Receive game state
        if (Rio_readnb(&rio, &game, sizeof(game_state_t)) <= 0) {
            printf("Server disconnected\n");
            break;
        }
        
        print_board(&game);
        
        if (game.game_over) {
            if (game.winner > 0) {
                printf("Game Over! Player %d wins!\n", game.winner);
            } else {
                printf("Game Over! Server disconnected\n");
            }
            break;
        }
        
        if (game.current_player == 1) {
            printf("Your turn (X)\n");
            printf("Enter row and column (0-%d): ", BOARD_SIZE - 1);
            
            msg.type = MSG_MOVE;
            if (scanf("%d %d", &msg.row, &msg.col) != 2) {
                printf("Invalid input\n");
                continue;
            }
            
            Rio_writen(clientfd, &msg, sizeof(message_t));
            
            // Wait for server response
            if (Rio_readnb(&rio, &msg, sizeof(message_t)) <= 0) {
                printf("Server disconnected\n");
                break;
            }
            
            if (msg.type == MSG_ERROR) {
                printf("Error: %s\n", msg.message);
            }
        } else {
            printf("Waiting for opponent's move...\n");
        }
    }
    
    Close(clientfd);
    exit(0);
} 