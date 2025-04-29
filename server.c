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

// Forward declaration of check_win function
int check_win(char board[][BOARD_SIZE], int row, int col, char player);

// Pattern definitions for advanced win checking
typedef struct {
    int pattern[5][2];  // Pattern coordinates relative to center
    int weight;         // Pattern weight for scoring
} Pattern;

// Define winning patterns - only 5 in a row patterns
const Pattern WIN_PATTERNS[] = {
    // Horizontal
    {{{0,0}, {0,1}, {0,2}, {0,3}, {0,4}}, 100},
    // Vertical
    {{{0,0}, {1,0}, {2,0}, {3,0}, {4,0}}, 100},
    // Diagonal (top-left to bottom-right)
    {{{0,0}, {1,1}, {2,2}, {3,3}, {4,4}}, 100},
    // Diagonal (top-right to bottom-left)
    {{{0,0}, {1,-1}, {2,-2}, {3,-3}, {4,-4}}, 100}
};

// Enhanced win checking with pattern recognition - only 5 in a row
int check_win_enhanced(char board[][BOARD_SIZE], int row, int col, char player) {
    // First check for immediate win using original algorithm
    if (check_win(board, row, col, player)) return 1;
    
    // Check for 5 in a row patterns
    for (int i = 0; i < sizeof(WIN_PATTERNS)/sizeof(Pattern); i++) {
        int matches = 0;
        
        // Check pattern in all possible positions around the last move
        for (int start_row = row - 4; start_row <= row; start_row++) {
            for (int start_col = col - 4; start_col <= col; start_col++) {
                matches = 0;
                
                // Check each position in the pattern
                for (int p = 0; p < 5; p++) {
                    int check_row = start_row + WIN_PATTERNS[i].pattern[p][0];
                    int check_col = start_col + WIN_PATTERNS[i].pattern[p][1];
                    
                    if (check_row >= 0 && check_row < BOARD_SIZE && 
                        check_col >= 0 && check_col < BOARD_SIZE) {
                        if (board[check_row][check_col] == player) {
                            matches++;
                        }
                    }
                }
                
                // If we have 5 matches, it's a winning position
                if (matches == 5) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Move validation using switch-case
typedef enum {
    MOVE_VALID,
    MOVE_OUT_OF_BOUNDS,
    MOVE_OCCUPIED,
    MOVE_INVALID
} MoveValidationResult;

MoveValidationResult validate_move_enhanced(char board[][BOARD_SIZE], int row, int col, char *error_msg) {
    switch(1) {
        case 1: // Check bounds
            if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
                sprintf(error_msg, "Position (%d,%d) is out of bounds!", row, col);
                return MOVE_OUT_OF_BOUNDS;
            }
            // Fall through
            
        case 2: // Check if occupied
            if (board[row][col] != ' ') {
                sprintf(error_msg, "Position (%d,%d) is already occupied!", row, col);
                return MOVE_OCCUPIED;
            }
            // Fall through
            
        case 3: // Additional validation rules can be added here
            return MOVE_VALID;
            
        default:
            return MOVE_INVALID;
    }
}

// Pattern-based strategy analyzer - simplified to only consider 5 in a row
int analyze_position(char board[][BOARD_SIZE], int row, int col, char player) {
    int score = 0;
    char opponent = (player == 'X') ? 'O' : 'X';
    
    // Check all patterns
    for (int i = 0; i < sizeof(WIN_PATTERNS)/sizeof(Pattern); i++) {
        for (int start_row = row - 4; start_row <= row; start_row++) {
            for (int start_col = col - 4; start_col <= col; start_col++) {
                int player_count = 0;
                int opponent_count = 0;
                int empty_count = 0;
                
                // Count pieces in pattern
                for (int p = 0; p < 5; p++) {
                    int check_row = start_row + WIN_PATTERNS[i].pattern[p][0];
                    int check_col = start_col + WIN_PATTERNS[i].pattern[p][1];
                    
                    if (check_row >= 0 && check_row < BOARD_SIZE && 
                        check_col >= 0 && check_col < BOARD_SIZE) {
                        if (board[check_row][check_col] == player) {
                            player_count++;
                        } else if (board[check_row][check_col] == opponent) {
                            opponent_count++;
                        } else if (board[check_row][check_col] == ' ') {
                            empty_count++;
                        }
                    }
                }
                
                // Score the pattern - only consider 5 in a row
                if (player_count == 5) score += 1000;
                else if (player_count == 4 && empty_count == 1) score += 100;
                else if (opponent_count == 4 && empty_count == 1) score += 50;
            }
        }
    }
    return score;
}

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
        if (count >= 5) return 1;  // Only win with 5 in a row
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
    int move_analysis[2] = {0, 0};  // Track move quality for each player

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

        MoveValidationResult validation_result = validate_move_enhanced(board, row, col, error_msg);
        if (validation_result != MOVE_VALID) {
            fprintf(stderr, "Invalid move: %s\n", error_msg);
            printf("Player %c made an invalid move at (%d,%d), please try again\n", 
                   current_player ? 'O' : 'X', row, col);
            continue;
        }

        // Analyze the move before making it
        int move_score = analyze_position(board, row, col, current_player ? 'O' : 'X');
        move_analysis[current_player] += move_score;
        
        // Make the move
        board[row][col] = current_player ? 'O' : 'X';
        stats[current_player].moves_made++;

        printf("Player %c made a move at position (%d, %d) with score %d\n", 
               current_player ? 'O' : 'X', row, col, move_score);

        if (check_win_enhanced(board, row, col, board[row][col])) {
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

    // Print final game statistics with enhanced analysis
    printf("\nGame Statistics:\n");
    printf("Player X: %d moves, Score: %d, Move Quality: %d\n", 
           stats[0].moves_made, stats[0].score, move_analysis[0]);
    printf("Player O: %d moves, Score: %d, Move Quality: %d\n", 
           stats[1].moves_made, stats[1].score, move_analysis[1]);
    
    // Print move quality comparison
    if (move_analysis[0] > move_analysis[1]) {
        printf("Player X played more strategically (higher move quality)\n");
    } else if (move_analysis[1] > move_analysis[0]) {
        printf("Player O played more strategically (higher move quality)\n");
    } else {
        printf("Both players showed similar strategic play\n");
    }

    Close(connfd1);
    Close(connfd2);
    Close(listenfd);
    return 0;
}