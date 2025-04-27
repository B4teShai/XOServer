#ifndef XO_H
#define XO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"

#define BOARD_SIZE 20
#define WIN_CONDITION 5
#define XO_MAXLINE 1024

// Game state
typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];
    int current_player;  // 1 for X, 2 for O
    int game_over;
    int winner;
} game_state_t;

// Message types
#define MSG_MOVE 1
#define MSG_GAME_OVER 2
#define MSG_ERROR 3

// Message structure
typedef struct {
    int type;
    int row;
    int col;
    char message[XO_MAXLINE];
} message_t;

// Function prototypes
void init_game(game_state_t *game);
int check_winner(game_state_t *game, int row, int col);
void print_board(game_state_t *game);
int make_move(game_state_t *game, int row, int col, int player);

#endif 