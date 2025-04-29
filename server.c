#include "csapp.h"
#include <stdint.h>
#include <time.h>

#define BOARD_SIZE 20
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define MOVE_TIMEOUT 30  // нэг хөдөлгөөн хийх хугацаа

typedef struct {
    int score;
    int moves_made;
    time_t last_move_time;
} PlayerStats;

// check_win функцийн
int check_win(char board[][BOARD_SIZE], int row, int col, char player);

typedef struct {
    int pattern[5][2];  // Төвтэй харьцуулсан  координатууд
    int weight;         // Оноо авах загварын ж
} Pattern;

// Зөвхөн 5 дараалсан ялах загваруудыг тодорхойлох
const Pattern WIN_PATTERNS[] = {
    // Хэвтээ
    {{{0,0}, {0,1}, {0,2}, {0,3}, {0,4}}, 100},
    // Босоо
    {{{0,0}, {1,0}, {2,0}, {3,0}, {4,0}}, 100},
    // Диагональ (дээд зүүнээс доод баруун руу)
    {{{0,0}, {1,1}, {2,2}, {3,3}, {4,4}}, 100},
    // Диагональ (дээд баруунаас доод зүүн рүү)
    {{{0,0}, {1,-1}, {2,-2}, {3,-3}, {4,-4}}, 100}
};

// Загвар таних 
int check_win_enhanced(char board[][BOARD_SIZE], int row, int col, char player) {
    // Эхлээд анхны тодорхойлсон алгоритмыг ашиглан шууд ялалтыг шалгах
    if (check_win(board, row, col, player)) return 1;
    
    // 5 дараалсан загваруудыг шалгах
    for (int i = 0; i < sizeof(WIN_PATTERNS)/sizeof(Pattern); i++) {
        int matches = 0;
        
        // Сүүлийн хөдөлгөөний эргэн тойронд бүх боломжит байрлалд загварыг шалгах
        for (int start_row = row - 4; start_row <= row; start_row++) {
            for (int start_col = col - 4; start_col <= col; start_col++) {
                matches = 0;
                
                // Загвар дахь байрлал бүрийг шалгах
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
                
                // Хэрэв 5 таарч байвал ялах байрлал
                if (matches == 5) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// хөдөлгөөний хүчинтэй эсэх
typedef enum {
    MOVE_VALID,
    MOVE_OUT_OF_BOUNDS,
    MOVE_OCCUPIED,
    MOVE_INVALID
} MoveValidationResult;

MoveValidationResult validate_move_enhanced(char board[][BOARD_SIZE], int row, int col, char *error_msg) {
    switch(1) {
        case 1: // Хүрээг шалгах
            if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
                sprintf(error_msg, "Position (%d,%d) is out of bounds!", row, col);
                return MOVE_OUT_OF_BOUNDS;
            }
            
        case 2: // Аль хэдийн ашиглаглсан эсэх
            if (board[row][col] != ' ') {
                sprintf(error_msg, "Position (%d,%d) is already occupied!", row, col);
                return MOVE_OCCUPIED;
            }
            
        case 3: // Хүчинтэй
            return MOVE_VALID;
            
        default:
            return MOVE_INVALID;
    }
}

int analyze_position(char board[][BOARD_SIZE], int row, int col, char player) {
    int score = 0;
    char opponent = (player == 'X') ? 'O' : 'X';
    
    // Бүх загваруудыг шалгах
    for (int i = 0; i < sizeof(WIN_PATTERNS)/sizeof(Pattern); i++) {
        for (int start_row = row - 4; start_row <= row; start_row++) {
            for (int start_col = col - 4; start_col <= col; start_col++) {
                int player_count = 0;
                int opponent_count = 0;
                int empty_count = 0;
                
                // Загвар дахь хэсгүүдийг тоолох
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
                
                // Загварт оноо өгөх
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
        if (count >= 5) return 1;  // Зөвхөн 5 дараалсан
    }
    return 0;
}

void send_board(int connfd, char board[][BOARD_SIZE], PlayerStats *stats) {
    char msg_type = 'B';
    Rio_writen(connfd, &msg_type, 1);
    Rio_writen(connfd, board, BOARD_SIZE * BOARD_SIZE);
    
    // XO самбарыг хэвлэх
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
    int move_analysis[2] = {0, 0};  // Тоглогч бүрийн хөдөлгөөний чанар

    while (!game_over) {
        send_board(connfd1, board, stats);
        send_board(connfd2, board, stats);

        int connfd_current = current_player ? connfd2 : connfd1;
        char turn_msg = 'T';
        Rio_writen(connfd_current, &turn_msg, 1);

        // Хөдөлгөөний хугацааг эхлүүлэх
        stats[current_player].last_move_time = time(NULL);

        int row_net, col_net;
        Rio_readn(connfd_current, &row_net, sizeof(row_net));
        Rio_readn(connfd_current, &col_net, sizeof(col_net));
        int row = ntohl(row_net);
        int col = ntohl(col_net);

        // Хугацааны шалгалт
        time_t current_time = time(NULL);
        if (current_time - stats[current_player].last_move_time > MOVE_TIMEOUT) {
            printf("Player %c timed out!\n", current_player ? 'O' : 'X');
            winner = !current_player;  // Бусад тоглогч хугацааны дагуу ялна
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

        // Хөдөлгөөнийг хийхээс өмнө шинжлэх
        int move_score = analyze_position(board, row, col, current_player ? 'O' : 'X');
        move_analysis[current_player] += move_score;
        
        // Хөдөлгөөнийг хийх
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

    // эцсийн тоглоомын статистик
    printf("\nGame Statistics:\n");
    printf("Player X: %d moves, Score: %d, Move Quality: %d\n", 
           stats[0].moves_made, stats[0].score, move_analysis[0]);
    printf("Player O: %d moves, Score: %d, Move Quality: %d\n", 
           stats[1].moves_made, stats[1].score, move_analysis[1]);
    
    // Хөдөлгөөний чанарын харьцуулалт
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