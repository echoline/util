#include "comms.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		char buf[1024];
		int r, sock = init_connection(argv[i]);
		if (sock == -1)
			continue;
		switch(fork()) {
		case 0:
			while ((r = read(sock, buf, sizeof(buf))) > 0) {
				buf[r] = '\0';
				printf("%s", buf);
				fflush(stdout);
			}
			perror("read");
			return 0;
		case -1:
			perror("fork");
			break;
		default:
			while (fgets(buf, sizeof(buf), stdin)) {
				r = write(sock, buf, strlen(buf));
				if (r < 0) perror("write");
			}
			close(sock);
		}
	}
}
