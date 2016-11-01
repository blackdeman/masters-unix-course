#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void printUsage() {
    fprintf(stderr, "Usage\n");
}

int main(int argc, char *argv[]) {

    // parsing arguments

    if (argc < 2) {
        perror("Argument list is empty.");
        printUsage();
        return 2;
    }

    char* commands[argc];

    int commandsCount = 0;
    int curCommandStart = -1;

    int i, j;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "|") == 0) {
            if (curCommandStart < 0 || i == argc - 1) {
                perror("Empty command found.");
                printUsage();
                return 2;
            } else {
                commands[i - 1] = NULL;
                curCommandStart = -1;
            }
        } else {
            if (curCommandStart < 0) {
                curCommandStart = i;

                commandsCount++;
            }
            commands[i - 1] = argv[i];
        }
    }
    commands[argc - 1] = NULL;

    int commandStartIndex = 0;
    int commandStartIndexes[commandsCount];
    commandStartIndexes[commandStartIndex++] = 0;
    for (i = 1; i < argc; ++i) {
        if (commands[i] == NULL) {
            commandStartIndexes[commandStartIndex++] = i + 1;
        }
    }

    // running commands

    int pipesCount = commandsCount - 1;
    int pipes[pipesCount][2];

    pid_t children[commandsCount];

    for (i = 0; i < pipesCount; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("Failed to create pipe");
            return 1;
        }
    }

    for (i = 0; i < commandsCount; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            char** args = commands + commandStartIndexes[i];

            if (i != commandsCount - 1) {
                close(pipes[i][0]);
                dup2(pipes[i][1], 1);
            }
            execvp(args[0], args);
        }

        if (commandsCount != 1) {
            close(pipes[i][1]);
            dup2(pipes[i][0], 0);
        }

        children[i] = pid;
    }

    for (i = 0; i < commandsCount - 1; ++i)
        for (j = 0; j < 2; ++j) {
            close(pipes[i][j]);
        }

    int exitCode = 0;
    int status;
    for (i = 0; i < commandsCount; ++i) {
        waitpid(children[i], &status, 0);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                exitCode = 1;
            }
        }
    }

    return exitCode;
}
