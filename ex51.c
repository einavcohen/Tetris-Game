//Einav Cohen 318506615

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

//define all keys
#define LEFT 'a'
#define RIGHT 'd'
#define DOWN 's'
#define ROTATE 'w'
#define QUIT 'q'

#define ERROR "Error"
#define TETRISOUT "./draw.out"

enum boolean {
    FAILURE = -1,
    SUCCESS
};

char get_char() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

void failure(char *msg, int *pipe_to_close) {
    close(pipe_to_close[0]);
    close(pipe_to_close[1]);
    write(STDERR_FILENO, msg, sizeof(msg));
}

int execute_game(int pipe[2]) {
    pid_t pid;
    if ((pid = fork()) == -1) {
        failure(ERROR, pipe);
        return FAILURE;
    }

    if (pid > 0) {
        // Father
        close(pipe[0]);
        return pid;
    } else {
        // Child
        if (close(pipe[1]) == -1) {
            failure(ERROR, pipe);
            return FAILURE;
        }
        if (dup2(pipe[0], STDIN_FILENO) == FAILURE) {
            failure(ERROR, pipe);
            return FAILURE;
        }
        char *argv[] = {TETRISOUT, NULL};
        execvp(argv[0], argv);
        failure(ERROR, pipe);
        return FAILURE;
    }
}

int main() {
    int myPipe[2];
    if (pipe(myPipe) == -1) {
        write(STDERR_FILENO, ERROR, sizeof(ERROR));
        exit(1);
    }

    int child_pid;
    if ((child_pid = execute_game(myPipe)) == FAILURE) {
        failure(ERROR, myPipe);
        exit(1);
    }
    char pressed_key;
    while (1) {
        pressed_key = get_char();
        if (write(myPipe[1], &pressed_key, sizeof(char)) == FAILURE) {
            failure(ERROR, myPipe);
            exit(1);
        }
        if (kill(child_pid, SIGUSR2) == FAILURE) {
            failure(ERROR, myPipe);
            exit(1);
        }
        if (pressed_key == QUIT) {
            break;
        }
    }
    if (close(myPipe[1]) == FAILURE) {
        write(STDERR_FILENO, ERROR, sizeof(ERROR));
        exit(1);
    }
    if (kill(child_pid, SIGKILL) == FAILURE) {
        write(STDERR_FILENO, ERROR, sizeof(ERROR));
        exit(1);
    }
}