#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "io.h"

#define MAX 1000000000

void PrintUsage(const char* prog) {
    fprintf(stderr, "=== number guessing client ===\n");
    fprintf(stderr, "Usage: %s UNIX_SOCKET_PATH \n\n", prog);
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Wrong number of parameters\n");
	PrintUsage(argv[0]);
	exit(1);
    }

    char* socketPath = argv[1];

    int socketfd, len;
    struct sockaddr_un remote;

    if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    fprintf(stderr, "Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, socketPath);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(socketfd, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    fprintf(stderr, "Connected.\n");

    uint32_t left = 0, right = MAX, guess = (right + left) / 2;
    uint32_t guessNetwork;

    char result = ' ';

   while (result != '=') {

	if (result == '<') {
	    right = guess - 1;
	} else
	if (result == '>') {
	    left = guess + 1;
	}
	
	guess = (right + left) / 2;

	guessNetwork = htonl(guess);

        if (!SendAll(socketfd, (char*)&guessNetwork, sizeof(guessNetwork))) {
            fprintf(stderr, "Send failed\n");
            exit(1);
        }

        if (!RecvAll(socketfd, &result, sizeof(result))) {
	    fprintf(stderr, "Recieve failed\n");
	    exit(1);
	}

	fprintf(stderr, "Guessed %d, recieved %c\n", guess, result);
    }

    printf("Secret number is %d\n", guess);

    close(socketfd);

    return 0;
}
