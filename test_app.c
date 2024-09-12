#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#define DEVICE_PATH "/dev/Seminar_Final"
#define IOCTL_SET_CMD _IOW('T', 1, struct game_command)
#define IOCTL_GET_CMD _IOR('T', 2, struct game_command)

struct game_command {
    int command;
    int value;
};

char board[9] = {'1','2','3','4','5','6','7','8','9'};

void print_board() {
    printf("\n %c | %c | %c \n", board[0], board[1], board[2]);
    printf("---|---|---\n");
    printf(" %c | %c | %c \n", board[3], board[4], board[5]);
    printf("---|---|---\n");
    printf(" %c | %c | %c \n\n", board[6], board[7], board[8]);
}

int check_winner(char board[9]) {
    int win_conditions[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, 
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, 
        {0, 4, 8}, {2, 4, 6}
    };

    for (int i = 0; i < 8; i++) {
        if (board[win_conditions[i][0]] == board[win_conditions[i][1]] &&
            board[win_conditions[i][1]] == board[win_conditions[i][2]]) {
            return 1; // Current player wins
        }
    }

    for (int i = 0; i < 9; i++) {
        if (board[i] != 'X' && board[i] != 'O') {
            return 0; // Game continues
        }
    }

    return 2; // Draw
}

int main(int argc, char *argv[]) {
    int device = open(DEVICE_PATH, O_RDWR); // Open the device with read/write access
    if (device < 0) {
        perror("Failed to open the device");
        return -1;
    }

    srand(time(NULL)); // Seed the random number generator with the current time

    int player = 1, position, running = 1, status, num_players;
    struct game_command cmd;

    while (running) {
        printf("How many players (1 or 2): ");
        if (scanf("%d", &num_players) != 1) {
            printf("Invalid input! Please enter a number.\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue;
        }

        if (num_players != 1 && num_players != 2) {
            printf("Invalid number of players!\n");
            continue;
        }

        while (running) {
            print_board();
            if (num_players == 1 && player == 2) {
                // Computer move
                do {
                    position = rand() % 9 + 1;
                } while (board[position - 1] == 'X' || board[position - 1] == 'O');
                printf("Computer (Player %d) chose position %d\n", player, position);
            } else {
                printf("Player %d's turn. Enter the position (1-9): ", player);
                if (scanf("%d", &position) != 1) {
                    printf("Invalid input! Please enter a number.\n");
                    while (getchar() != '\n'); // Clear input buffer
                    continue;
                }

                if (position < 1 || position > 9 || board[position - 1] == 'X' || board[position - 1] == 'O') {
                    printf("Invalid position! Please try again.\n");
                    continue;
                }
            }

            board[position - 1] = (player == 1) ? 'X' : 'O';
            cmd.command = position;
            cmd.value = player;

            if (ioctl(device, IOCTL_SET_CMD, &cmd) < 0) {
                perror("Failed to send IOCTL command");
                close(device);
                return -1;
            }

            status = check_winner(board);
            if (status == 1) {
                print_board();
                printf("Player %d wins!\n", player);
                break;
            } else if (status == 2) {
                print_board();
                printf("The game is a draw.\n");
                break;
            }

            player = (player == 1) ? 2 : 1;
        }

        break;
    }

    close(device);
    return 0;
}