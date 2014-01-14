#include "comms.h"
#include <strings.h>
#include <unistd.h>
#include <poll.h>

int main(int argc, char **argv) {
	char buf[1024];
	int r, client, sock;
	struct pollfd fds[2];

	if (argc < 3) {
		printf("usage: %s [tcp|unix] [port|path]\n", argv[0]);
		return -1;
	}

	sock = strcasecmp(argv[1], "tcp")? init_unix_listen(argv[2])
					 : init_tcp_listen(atoi(argv[2]));

	if ((client = accept(sock, NULL, NULL)) > -1) {
		fds[0].fd = client;
		fds[1].fd = 0;
		fds[0].events = POLLIN | POLLHUP;
		fds[1].events = POLLIN | POLLHUP;

		while (poll(fds, 2, -1)) {
			if (fds[0].revents & POLLIN) {
				r = read(client, buf, sizeof(buf));
				// ctrl+d pressed
				if (buf[0] == '\x04') {
					close (client);
					return 0;
				}

				if (r < 0) {
					perror("read");
					continue;
				}
				write(1, buf, r);
				fflush(stdout);
			}

			if (fds[1].revents & POLLIN) {
				if (fgets(buf, sizeof(buf), stdin) == NULL) {
					close (client);
					return 0;
				}

				r = write(client, buf, strlen(buf));
				if (r < 0)
					perror("write");
			}

			for (r = 0; r < 2; r++)
				if (fds[r].revents & POLLHUP) {
					close (client);
					return 0;
				}
		}

		perror("poll");
		close(client);
		return -1;
	}

	perror ("accept");
	return -1;
}
