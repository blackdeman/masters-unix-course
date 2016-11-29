#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "io.h"

#define MAX 1000000000
#define MAX_ATTEMPTS 32

void PrintUsage(const char* prog) {
    fprintf(stderr, "=== number guessing client ===\n");
    fprintf(stderr, "Usage: %s UNIX_SOCKET_PATH \n\n", prog);
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Wrong number of parameters!\n");
	PrintUsage(argv[0]);
	return 1;
    }

    char* socketPath = argv[1];

    int socketfd, len;
    struct sockaddr_un remote;

    if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    fprintf(stderr, "Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    if (strlen(socketPath) >= sizeof(remote.sun_path)) {
        fprintf(stderr, "path '%s' is too long for UNIX domain socket\n", socketPath);
        return 1;
    }
    strcpy(remote.sun_path, socketPath);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(socketfd, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        return 1;
    }

    fprintf(stderr, "Connected.\n");

    uint32_t left = 0, right = MAX, guess = 0, guessNetwork;
    int attempt = 1, retCode = 1;

    char result = ' ';

    for (;;) {

	if (result == '=') {
	    retCode = 0;
	    break;
	}

	if (attempt > MAX_ATTEMPTS) {
	    fprintf(stderr, "limit of attempts reached, exit...\n");
	    break;
	}

	if (result == '<') {
	    right = guess - 1;
	} else if (result == '>') {
	    left = guess + 1;
	}
	
	guess = (right + left) / 2;

	guessNetwork = htonl(guess);

        if (!SendAll(socketfd, (char*)&guessNetwork, sizeof(guessNetwork))) {
            break;
        }

        if (!RecvAll(socketfd, &result, sizeof(result))) {
	    break;
	}

        attempt++;

	fprintf(stderr, "* attempt #%d: guessed %"PRIu32", recieved %c\n", attempt, guess, result);
    }

    if (!retCode) {
	printf("secret number is %"PRIu32"\n", guess);
    }

    if (close(socketfd) == -1) {
        perror("close socket");
        return 1;
    }

    return retCode;
}
