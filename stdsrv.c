#include "comms.h"
#include <strings.h>
#include <unistd.h>
#include <poll.h>

int usage(char **argv) {
	printf("usage: %s tcp|unix portnum|/path/to/socket\n", argv[0]);
	exit(-1);
}

int main(int argc, char **argv) {
	char buf[1024];
	int r, client, sock;
	int FDS = 2;
	struct pollfd *fds;

	if (argc < 3) {
		usage(argv);
	}

	fds = malloc(FDS * sizeof(struct pollfd));
	sock = strcasecmp(argv[1], "tcp")?
	(strcasecmp(argv[1], "unix")? usage(argv) : init_unix_listen(argv[2]))
	 : init_tcp_listen(atoi(argv[2]));

	fds[0].fd = sock;
	fds[1].fd = 0;
	fds[0].events = POLLIN | POLLHUP;
	fds[1].events = POLLIN | POLLHUP;

	while (poll(fds, FDS, -1)) {
		if (fds[0].revents & POLLIN) {
			for (r = 2; r < FDS; r++)
				if (fds[r].fd == -1)
					break;
			if (r == FDS) {
				FDS++;
				fds = realloc(fds, FDS * sizeof(struct pollfd));
			}
			client = accept(sock, NULL, NULL);
			if (client > 0) {
				fds[r].fd = client;
				fds[r].events = POLLIN | POLLHUP;
			} else {
				perror ("accept");
			}
		}

		if (fds[1].revents & POLLIN) {
			if (fgets(buf, sizeof(buf), stdin) == NULL) {
				continue;
			}

			for (client = 2; client < FDS; client++) {
				if (fds[client].fd > 0) {
					r = write(fds[client].fd, buf, strlen(buf));
					if (r < 0) {
						perror("write");
						close(fds[client].fd);
						fds[client].fd = -1;
					}
				}
			}
		}

		for (client = 2; client < FDS; client++) {
			if (fds[client].fd == -1)
				continue;
			if (fds[client].revents & POLLIN) {
				r = read(fds[client].fd, buf, sizeof(buf));
				// ctrl+d pressed
				if (buf[0] == '\x04') {
					close (fds[client].fd);
					fds[client].fd = -1;
					continue;
				}

				if (r < 0) {
					perror("read");
					close (fds[client].fd);
					fds[client].fd = -1;
					continue;
				}
				write(1, buf, r);
				fflush(stdout);
			}
		}

		for (r = 2; r < FDS; r++)
			if (fds[r].fd != -1 && fds[r].revents & POLLHUP) {
				close (fds[r].fd);
				fds[r].fd = -1;
			}
	}

	perror("poll");
	close(client);
	return -1;
}

