#include "comms.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		char buf[1024];
		int r, sock = init_connection(argv[i]);
		if (sock == -1) {
			printf("usage: sock [address]\n");
			continue;
		}
		switch(fork()) {
		case 0:
			while ((r = read(sock, buf, sizeof(buf))) > 0) {
				write(1, buf, r);
				fflush(stdout);
			}
			perror("read");
			close(sock);
			return 0;
		case -1:
			perror("fork");
			break;
		default:
			while (fgets(buf, sizeof(buf), stdin)) {
				r = write(sock, buf, strlen(buf));
				if (r <= 0) {
					perror("write");
					break;
				}
			}
			close(sock);
		}
	}
}
