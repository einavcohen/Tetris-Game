
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>

// settings
#define BOARD_HEIGHT 20
#define BOARD_WIDTH 20
#define INIT_X 10
#define INIT_Y 1
#define ERROR "Error"

// define all keys
#define LEFT 'a'
#define RIGHT 'd'
#define DOWN 's'
#define ROTATE 'w'
#define QUIT 'q'

//definde enums below

enum Rotations {
    HORIZONTAL,
    VERTICAL,
    NUMBER_OF_ROTATIONS
};

typedef struct LineShape {
    int center_x;
    int center_y;
    enum Rotations rotations;
} LineShape;

enum boolean {
    FALSE,
    TRUE
};

//initialize_shape- func that init the line shape
int initialize_shape(LineShape *line_shape, int force_reset) {
    if (force_reset || (line_shape->rotations == HORIZONTAL
                        && line_shape->center_y >= BOARD_HEIGHT - 1
                        || line_shape->rotations == VERTICAL
                           && line_shape->center_y >= BOARD_HEIGHT - 2)) {
        line_shape->center_x = INIT_X;
        line_shape->center_y = INIT_Y;
        line_shape->rotations = VERTICAL;
    }
}

// Global Variable
LineShape lineShape;

// display_board - function that prints the board
void display_board() {
    int i;
    int j;
    int centerX = lineShape.center_x;
    int centerY = lineShape.center_y;

    //clearing the screen from other boards
    system("clear");

    write(STDOUT_FILENO, "\n", 1);
    for (i = 0; i < BOARD_HEIGHT; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            if (j == 0 || j == BOARD_WIDTH - 1 || i == BOARD_HEIGHT - 1) {
                write(STDOUT_FILENO, "*", 1);
            } else if (
                    (lineShape.rotations == HORIZONTAL && i == centerY && 2 > abs((j - centerX)))
                    || (lineShape.rotations == VERTICAL && j == centerX
                        && 2 > abs(i - centerY))) {
                write(STDOUT_FILENO, "-", 1);
            } else {
                write(STDOUT_FILENO, " ", 1);
            }
        }
        write(STDOUT_FILENO, "\n", 1); //next line
    }
}

//handle_key- function that moves the player any direction he choose
//according the pressed key
void handle_key(char pressed_key) {
    if (pressed_key == LEFT) {
        if (lineShape.rotations == HORIZONTAL && lineShape.center_x > 2
            || lineShape.rotations == VERTICAL && lineShape.center_x > 1)
            lineShape.center_x--;
    } else if (pressed_key == DOWN) {
        lineShape.center_y++;
    } else if (pressed_key == RIGHT) {
        if (lineShape.rotations == HORIZONTAL
            && lineShape.center_x < BOARD_WIDTH - 3
            || lineShape.rotations == VERTICAL
               && lineShape.center_x < BOARD_WIDTH - 2)
            lineShape.center_x++;
    } else if (pressed_key == ROTATE) {
        lineShape.rotations =
                (lineShape.rotations + 1) % NUMBER_OF_ROTATIONS;
        if (lineShape.rotations == HORIZONTAL && lineShape.center_x >=
                                                   BOARD_WIDTH - 3) {
            lineShape.center_x--;
        } else if (lineShape.rotations == HORIZONTAL && lineShape.center_x <= 2) {
            lineShape.center_x++;
        }
    }
    initialize_shape(&lineShape, FALSE);
}

//handle_key_signal-function that handle signals from ex51.c
//reading chars from the pipe instead of stdin
void handle_key_signal() {
    char key = 0;
    if (scanf("%c", &key) == EOF) {
        write(STDERR_FILENO, ERROR, sizeof(ERROR));
        exit(1);
    }
    if (key == QUIT) {
        exit(0);
    }
    handle_key(key);
    display_board();
}

//handle_alarm_signal function is called every 1 sec
void handle_alarm_signal() {
    alarm(1);
    lineShape.center_y++;
    initialize_shape(&lineShape, FALSE);
    display_board();
}

int main() {
    initialize_shape(&lineShape, TRUE);
    signal(SIGALRM, handle_alarm_signal);
    alarm(1);
    signal(SIGUSR2, handle_key_signal);
    while (1) {
        pause();
    }
}
