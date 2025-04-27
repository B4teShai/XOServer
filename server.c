#include "xo.h"

void init_game(game_state_t *game) {
    memset(game->board, ' ', BOARD_SIZE * BOARD_SIZE);
    game->current_player = 1;
    game->game_over = 0;
    game->winner = 0;
}

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

int check_winner(game_state_t *game, int row, int col) {
    char player = game->board[row][col];
    int count;
    
    // Check horizontal
    count = 1;
    for (int i = col - 1; i >= 0 && game->board[row][i] == player; i--) count++;
    for (int i = col + 1; i < BOARD_SIZE && game->board[row][i] == player; i++) count++;
    if (count >= WIN_CONDITION) return 1;
    
    // Check vertical
    count = 1;
    for (int i = row - 1; i >= 0 && game->board[i][col] == player; i--) count++;
    for (int i = row + 1; i < BOARD_SIZE && game->board[i][col] == player; i++) count++;
    if (count >= WIN_CONDITION) return 1;
    
    // Check diagonal (top-left to bottom-right)
    count = 1;
    for (int i = 1; row - i >= 0 && col - i >= 0 && game->board[row-i][col-i] == player; i++) count++;
    for (int i = 1; row + i < BOARD_SIZE && col + i < BOARD_SIZE && game->board[row+i][col+i] == player; i++) count++;
    if (count >= WIN_CONDITION) return 1;
    
    // Check diagonal (top-right to bottom-left)
    count = 1;
    for (int i = 1; row - i >= 0 && col + i < BOARD_SIZE && game->board[row-i][col+i] == player; i++) count++;
    for (int i = 1; row + i < BOARD_SIZE && col - i >= 0 && game->board[row+i][col-i] == player; i++) count++;
    if (count >= WIN_CONDITION) return 1;
    
    return 0;
}

int make_move(game_state_t *game, int row, int col, int player) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return 0;
    }
    if (game->board[row][col] != ' ') {
        return 0;
    }
    
    game->board[row][col] = (player == 1) ? 'X' : 'O';
    if (check_winner(game, row, col)) {
        game->game_over = 1;
        game->winner = player;
        return 1;
    }
    
    game->current_player = (player == 1) ? 2 : 1;
    return 1;
}

void handle_client(int connfd, int player_num, game_state_t *game) {
    message_t msg;
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    
    while (!game->game_over) {
        if (game->current_player == player_num) {
            // Send current game state
            Rio_writen(connfd, game, sizeof(game_state_t));
            
            // Wait for move
            if (Rio_readnb(&rio, &msg, sizeof(message_t)) <= 0) {
                printf("Client %d disconnected\n", player_num);
                game->game_over = 1;
                break;
            }
            
            if (msg.type == MSG_MOVE) {
                if (make_move(game, msg.row, msg.col, player_num)) {
                    print_board(game);
                    if (game->game_over) {
                        msg.type = MSG_GAME_OVER;
                        sprintf(msg.message, "Player %d wins!", player_num);
                        Rio_writen(connfd, &msg, sizeof(message_t));
                        break;
                    }
                } else {
                    msg.type = MSG_ERROR;
                    strcpy(msg.message, "Invalid move");
                    Rio_writen(connfd, &msg, sizeof(message_t));
                }
            }
        } else {
            // Send current game state
            Rio_writen(connfd, game, sizeof(game_state_t));
            
            // Wait for other player's move
            if (Rio_readnb(&rio, &msg, sizeof(message_t)) <= 0) {
                printf("Client %d disconnected\n", player_num);
                game->game_over = 1;
                break;
            }
        }
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    game_state_t game;
    int player_count = 0;
    int client_fds[2] = {-1, -1};
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);
    init_game(&game);
    
    printf("Server started on port %s\n", argv[1]);
    printf("Waiting for players...\n");
    
    while (player_count < 2) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        
        client_fds[player_count] = connfd;
        player_count++;
        
        if (player_count == 2) {
            printf("Game starting!\n");
            // Create threads for each client
            pthread_t tid1, tid2;
            Pthread_create(&tid1, NULL, (void *(*)(void *))handle_client, 
                         (void *)(long)client_fds[0], 1, &game);
            Pthread_create(&tid2, NULL, (void *(*)(void *))handle_client, 
                         (void *)(long)client_fds[1], 2, &game);
            
            Pthread_join(tid1, NULL);
            Pthread_join(tid2, NULL);
        }
    }
    
    Close(listenfd);
    Close(client_fds[0]);
    Close(client_fds[1]);
    exit(0);
} 