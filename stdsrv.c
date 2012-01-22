#include "comms.h"
#include <strings.h>
#include <unistd.h>

int main(int argc, char **argv) {
	char buf[1024];
	int r, client, sock;

	if (argc < 3) {
		printf("usage: %s [tcp|unix] [port|path]\n", argv[0]);
		return -1;
	}

	sock = strcasecmp(argv[1], "tcp")? init_unix_listen(argv[2])
					 : init_tcp_listen(atoi(argv[2]));


	while ((client = accept(sock, NULL, NULL)) > -1) {
		switch(fork()) {
		case 0:
			while ((r = read(client, buf, sizeof(buf))) > -1) {
				write(1, buf, r);
				fflush(stdout);
			}
			perror("read");
			return 0;
		case -1:
			perror("fork");
			break;
		default:
			while (fgets(buf, sizeof(buf), stdin)) {
				r = write(client, buf, strlen(buf));
				if (r < 0) perror("write");
			}
			close(client);
		}
	}
}
