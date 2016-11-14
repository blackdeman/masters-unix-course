#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void printUsage() {
    fprintf(stderr, "\nUsage: pipeline command1 | command2 | ... | commandN\n");
}

void cleanup(char** commands, int* commandStartIndexes, int** pipes, int pipesCount, pid_t* children) {
    free(commands);

    free(commandStartIndexes);

    for (int i = 0; i < pipesCount; ++i) {
        free(pipes[i]);
    }
    free(pipes);

    free(children);
}

int main(int argc, char *argv[]) {

    // parsing arguments

    if (argc < 2) {
        fprintf(stderr, "Argument list is empty.");
        printUsage();
        return 2;
    }

    char** commands = (char**) malloc(argc * sizeof(char*));
    if (!commands) {
        perror("malloc");
        return 1;
    }

    int commandsCount = 0;
    int curCommandStart = -1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "|") == 0) {
            if (curCommandStart < 0 || i == argc - 1) {
                fprintf(stderr, "Empty command found.");
                printUsage();
                free(commands);
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
    int* commandStartIndexes = (int*)malloc(commandsCount * sizeof(int));
    if (!commandStartIndexes) {
        perror("malloc");
        free(commands);
        return 1;
    }
    commandStartIndexes[commandStartIndex++] = 0;
    for (int i = 1; i < argc; ++i) {
        if (commands[i] == NULL && i != argc - 1) {
            commandStartIndexes[commandStartIndex++] = i + 1;
        }
    }

    // running commands

    pid_t* children = (pid_t*)malloc(commandsCount * sizeof(pid_t));
    if (!children) {
        perror("malloc");
        free(commands);
        free(commandStartIndexes);
        return 1;
    }

    int pipesCount = commandsCount - 1;
    int** pipes = (int**)malloc(pipesCount * sizeof(int*));

    for (int i = 0; i < pipesCount; ++i) {
        pipes[i] = (int*)malloc(2 * sizeof(int));
        if (!pipes[i]) {
            perror("malloc");
            free(commands);
            free(commandStartIndexes);
            free(children);
            return 1;
        }
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
            return 1;
        }
    }

    for (int i = 0; i < commandsCount; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            char** args = commands + commandStartIndexes[i];

            if (i != commandsCount - 1) {
                if (close(pipes[i][0]) == -1) {
                    perror("close");
                    cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
                    return 1;
                }
                if (dup2(pipes[i][1], 1) == -1) {
                    perror("dup2");
                    cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
                    return 1;
                }
            }
            if (execvp(args[0], args) == -1) {
               perror("execvp");
               cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
               return 1;
	    }
        }

        if (commandsCount != 1 && i < pipesCount) {
            if (close(pipes[i][1]) == -1) {
                perror("close");
                cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
                return 1;
            }
            if (dup2(pipes[i][0], 0) == -1) {
                perror("pipe");
                cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
                return 1;
            }
        }

        children[i] = pid;
    }

    int exitCode = 0;
    int status;
    for (int i = 0; i < commandsCount; ++i) {
        if (waitpid(children[i], &status, 0) == -1) {
            perror("waitpid");
            cleanup(commands, commandStartIndexes, pipes, pipesCount, children);
            return 1;
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                exitCode = 1;
            }
        }
    }

    cleanup(commands, commandStartIndexes, pipes, pipesCount, children);

    return exitCode;
}
